# Omarchy / Arch preparation

The target investigated here runs `linux-omarchy 7.2.5-3`, with matching headers, GCC 16.2.1, mkinitcpio, and Limine. The patched module has compiled successfully and passed kernel-identity and dependency comparisons. It is now staged in the normal UKI with a stock-image fallback; runtime validation is pending a reboot. Do not assume a Bazzite binary is compatible.

## Matching source

Use the official packaging revision for the installed package. For `7.2.5-3`, the inspected revision is:

`7b11c97603dd9d751d803746560ee51640709725`

in [omacom/omarchy-pkgs](https://github.com/omacom/omarchy-pkgs/tree/7b11c97603dd9d751d803746560ee51640709725/pkgbuilds/linux-omarchy).

Read the PKGBUILD, then use `makepkg --nobuild --nodeps` as a regular user to fetch, verify, extract, and prepare its sources. Import the package's declared public signing keys into a dedicated GPG home if necessary. Preserve checksum and signature verification. The recipe applies distribution patches before the D700 patches.

Build the module with this repository's script against the installed `linux-omarchy-headers`.

## Installation and boot images

An Arch-family installation can use a kernel-specific module under `/usr/lib/modules/<release>/updates/`, followed by `depmod` and initramfs regeneration. Verify that `modinfo -k <release> -n amdgpu` resolves to the intended override and that its kernel identity matches.

Omarchy's mkinitcpio wrapper explicitly warns that `mkinitcpio -P` alone does not update Limine entries. Use the distribution's `limine-mkinitcpio` / `limine-update` workflow appropriate to its configured unified-kernel images. Inspect the local Limine configuration, preserve the existing boot image as a fallback, and verify the generated image before rebooting.

## Validated staging procedure

The inspected installation uses `/boot/EFI/Linux/omarchy_linux-omarchy.efi`, with its normal encrypted-root command line embedded in the UKI. Before changing anything, the stock UKI and `/boot/limine.conf` were backed up to a new root-only directory under `/var/lib/d700macpro`.

A second byte-identical stock UKI was kept at `/boot/EFI/d700macpro/stock-linux-omarchy.efi`. The supported tool added its recovery entry:

```bash
sudo limine-entry-tool --add-efi 'D700 stock driver fallback' \
  /boot/EFI/d700macpro/stock-linux-omarchy.efi \
  --comment 'Original driver and boot image retained for recovery' --priority 10
```

The normal Omarchy entry has priority 50 in this installation. The menu tree and `default_entry` were checked to ensure the recovery entry did not become the default. For a UKI without an embedded command line, use a recovery entry that supplies the appropriate command line instead of assuming a plain EFI entry is sufficient.

After checking the artifact's exact kernel identity and dependencies:

```bash
kernel_release=$(uname -r)
sudo install -D -m 0644 artifacts/amdgpu.ko \
  /usr/lib/modules/$kernel_release/updates/d700macpro/amdgpu.ko
sudo depmod -a "$kernel_release"
modinfo -k "$kernel_release" -F filename amdgpu
sudo limine-mkinitcpio linux-omarchy
```

`modinfo` must resolve to the new override. The stock compressed module is left untouched. Regeneration updates the normal UKI and its Limine integrity hash. It does not reboot or reload the currently running driver.

The generated UKI's `.initrd` section was extracted into a private temporary directory. `lsinitcpio --extract` and `modinfo -b <extracted-root> -k <release>` confirmed that its module lookup selected the patched driver, and its bytes matched the built artifact exactly. The normal UKI's BLAKE2 hash matched its boot entry, and the stock recovery image still matched the backup.

The running boot ID was unchanged throughout staging. Clock behavior must still be verified after rebooting into the staged image.

## Restore stock

Remove only the override installed by this procedure, run `depmod` for that kernel, then regenerate with `limine-mkinitcpio linux-omarchy`. The recovery menu entry can boot the preserved stock UKI if the normal entry fails. Keep backups until the restored boot is verified.

No automatic installer is supplied here for an unverified Limine layout. Root access is needed to stage system files; building does not require it. Kernel updates require a matching rebuilt module or an upstream-fixed kernel. Keep signature enforcement unchanged.
