#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-only
set -euo pipefail

found=0
for device in /sys/class/drm/card[0-9]/device; do
  [[ -r "$device/vendor" && -r "$device/device" && -r "$device/subsystem_vendor" ]] || continue
  [[ $(<"$device/vendor") == 0x1002 && $(<"$device/device") == 0x6798 && $(<"$device/subsystem_vendor") == 0x106b ]] || continue
  found=1
  printf '\n%s\n' "$(basename -- "$(readlink -f "$device")")"
  for attribute in vbios_version power_dpm_force_performance_level power_dpm_state; do
    if [[ -r "$device/$attribute" ]]; then
      printf '%s: %s\n' "$attribute" "$(cat "$device/$attribute")"
    fi
  done
  for sensor in "$device"/hwmon/hwmon*; do
    [[ -r "$sensor/freq1_input" && -r "$sensor/freq2_input" && -r "$sensor/temp1_input" ]] || continue
    core=$(<"$sensor/freq1_input")
    memory=$(<"$sensor/freq2_input")
    temperature=$(<"$sensor/temp1_input")
    printf 'core: %s MHz; memory: %s MHz; temperature: %s C\n' \
      "$((core / 1000000))" "$((memory / 1000000))" "$((temperature / 1000))"
  done
done
if (( ! found )); then
  printf 'No matching Apple Tahiti XT / D700 device found.\n' >&2
  exit 1
fi
