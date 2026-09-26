# Omarchy / Arch preparation

The target investigated here runs `linux-omarchy 7.2.5-3`, with matching headers, GCC 16.2.1, mkinitcpio, and Limine. The patched module has compiled successfully and passed kernel-identity and dependency comparisons. Privileged installation and runtime validation are pending; do not assume a Bazzite binary is compatible.

## Matching source

Use the official packaging revision for the installed package. For `7.2.5-3`, the inspected revision is:

`7b11c97603dd9d751d803746560ee51640709725`

in [omacom/omarchy-pkgs](https://github.com/omacom/omarchy-pkgs/tree/7b11c97603dd9d751d803746560ee51640709725/pkgbuilds/linux-omarchy).

Read the PKGBUILD, then use `makepkg --nobuild --nodeps` as a regular user to fetch, verify, extract, and prepare its sources. Import the package's declared public signing keys into a dedicated GPG home if necessary. Preserve checksum and signature verification. The recipe applies distribution patches before the D700 patches.

Build the module with this repository's script against the installed `linux-omarchy-headers`.

## Installation and boot images

An Arch-family installation can use a kernel-specific module under `/usr/lib/modules/<release>/updates/`, followed by `depmod` and initramfs regeneration. Verify that `modinfo -k <release> -n amdgpu` resolves to the intended override and that its kernel identity matches.

Omarchy's mkinitcpio wrapper explicitly warns that `mkinitcpio -P` alone does not update Limine entries. Use the distribution's `limine-mkinitcpio` / `limine-update` workflow appropriate to its configured unified-kernel images. Inspect the local Limine configuration, preserve the existing boot image as a fallback, and verify the generated image before rebooting.

No automatic installer is supplied here for an unverified Limine layout. Root access is needed to stage system files; building does not require it. Kernel updates require a matching rebuilt module or an upstream-fixed kernel. Keep signature enforcement unchanged.
