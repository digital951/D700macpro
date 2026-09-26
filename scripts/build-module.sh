#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-only
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
  printf 'Usage: %s MATCHING_KERNEL_SOURCE [OUTPUT_DIRECTORY]\n' "$0" >&2
  exit 2
fi
if (( EUID == 0 )); then
  printf 'Build as a regular user. Installation is a separate administrator step.\n' >&2
  exit 1
fi

repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
source_dir=$(realpath -- "$1")
output_dir=${2:-"$repo_dir/artifacts"}
kernel_release=$(uname -r)
headers_dir=/usr/lib/modules/$kernel_release/build
module_dir=$source_dir/drivers/gpu/drm/amd/amdgpu
build_jobs=${D700_JOBS:-8}

[[ $build_jobs =~ ^[1-9][0-9]*$ ]] || { printf 'D700_JOBS must be positive.\n' >&2; exit 2; }
case "$source_dir" in
  *[[:space:]]*) printf 'Use a source path without whitespace for Kbuild.\n' >&2; exit 2 ;;
esac
[[ -f "$headers_dir/.config" && -f "$module_dir/Makefile" ]] || {
  printf 'Matching kernel headers or complete AMDGPU source are missing.\n' >&2
  exit 1
}
grep -q '^CONFIG_CC_IS_GCC=y$' "$headers_dir/.config" || {
  printf 'This tested workflow supports GCC-built kernels only.\n' >&2
  exit 1
}

mkdir -p -- "$output_dir"
output_dir=$(realpath -- "$output_dir")
for source_patch in "$repo_dir"/patches/*.patch; do
  if git -C "$source_dir" apply --check "$source_patch"; then
    git -C "$source_dir" apply "$source_patch"
  elif git -C "$source_dir" apply --reverse --check "$source_patch" 2>/dev/null; then
    printf 'Already applied: %s\n' "$(basename -- "$source_patch")"
  else
    printf 'Patch does not match this source; inspect whether upstream already fixed it: %s\n' "$source_patch" >&2
    exit 1
  fi
done

printf 'Building for %s\n' "$kernel_release"
# BTF is optional debug metadata. Omit it consistently for this local module.
make -C "$headers_dir" M="$module_dir" \
  KCFLAGS="-I$source_dir/include/trace" CONFIG_DEBUG_INFO_BTF_MODULES= \
  -j"$build_jobs" modules 2>&1 | tee "$output_dir/build.log"

cp -- "$module_dir/amdgpu.ko" "$output_dir/amdgpu.ko"
strip --strip-debug "$output_dir/amdgpu.ko"
[[ $(modinfo -F name "$output_dir/amdgpu.ko") == amdgpu ]]
[[ $(modinfo -F vermagic "$output_dir/amdgpu.ko") == "$(modinfo -F vermagic amdgpu)" ]]
[[ $(modinfo -F depends "$output_dir/amdgpu.ko") == "$(modinfo -F depends amdgpu)" ]]
printf '%s\n' "$kernel_release" > "$output_dir/kernel-version"
(
  cd -- "$output_dir"
  sha256sum amdgpu.ko > SHA256SUMS
)
printf '\nBuilt and checked: %s/amdgpu.ko\n' "$output_dir"
printf 'No installed driver or boot configuration was changed.\n'
