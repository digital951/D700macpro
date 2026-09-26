# Bazzite / rpm-ostree persistence

Use the matching OGC source and installed kernel headers to build the module. Keep a bootable stock deployment. Do not point the permanent default at a hand-written entry containing a fixed OSTree deployment path; that path can become stale after updates.

The tested approach stores the override under `/etc/d700macpro/overlay`, preserving the module's exact kernel-specific path, and enables managed initramfs regeneration.

The following commands assume regeneration was previously disabled. Check `rpm-ostree initramfs` first; if it is already enabled, preserve its existing arguments instead of replacing them.

```bash
kernel_release=$(uname -r)
module_target=/etc/d700macpro/overlay/usr/lib/modules/$kernel_release/kernel/drivers/gpu/drm/amd/amdgpu/amdgpu.ko
test "$(cat artifacts/kernel-version)" = "$kernel_release"
sudo install -D -m 0644 artifacts/amdgpu.ko "$module_target"
sudo rpm-ostree initramfs --enable \
  --arg=--include --arg=/etc/d700macpro/overlay --arg=/ --arg=--nostrip
```

After the transaction completes, inspect `rpm-ostree status` and the default boot entry. Extract the embedded module with `lsinitrd --file` and compare its hash with the built artifact before rebooting. After boot, verify clocks and VBIOS selection again.

This survives ordinary reboots and uses Bazzite's managed deployment paths. It is **kernel-specific**, not a promise to patch every future kernel. When the kernel version changes, the old module stays under its old directory and cannot replace the new kernel's module. Confirm that the new kernel contains the fixes or rebuild from its exact distribution source.

## Restore stock

If this setup enabled regeneration on a system where it was previously disabled:

```bash
sudo rpm-ostree initramfs --disable
```

Verify the staged stock deployment and reboot. Remove `/etc/d700macpro` only after stock boot is verified, and only if it contains this installation's files. If regeneration existed before this setup, remove only these override arguments and preserve the prior configuration.

The method does not overwrite the stock module in `/usr`. If your kernel requires signed modules, use your distribution's signing workflow; do not disable enforcement to follow this guide.
