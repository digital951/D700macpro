# Patch provenance

## Desktop power-source correction

`0002-si-desktop-power-source.patch` adapts Timur Kristóf's September 23, 2026 patch, **“drm/amd/pm/si: Check AMD_IS_MOBILITY flag for PPSMC_SYSTEMFLAG_GPIO_DC.”** Mario Limonciello reviewed it and said he would queue it.

- [Original patch](https://www.mail-archive.com/amd-gfx@lists.freedesktop.org/msg151959.html)
- [Maintainer review](https://www.mail-archive.com/amd-gfx@lists.freedesktop.org/msg152027.html)
- [Companion AC/DC limit cleanup](https://www.mail-archive.com/amd-gfx@lists.freedesktop.org/msg151960.html), not needed for this controlled test

The desktop correction leaves the existing temperature and regulator-hot protection flags intact. It changes neither the factory frequency/voltage tables nor firmware stored on the graphics cards.

## Exact VFCT matching

`0001-vfct-exact-match-first.patch` fixes an early relaxed match hiding a later exact PCI bus match. On the tested Mac, the second GPU loaded XA-028 even though its distinct XB-028 image was present. Runtime hashes confirmed that the patch corrected the selection.

- [Change introducing relaxed VFCT bus matching](https://github.com/torvalds/linux/commit/11c141672045ffc0187aa604f2c0f597bc334fb2)
- [Related multi-GPU report, AMD issue 5762](https://gitlab.freedesktop.org/drm/amd/-/work_items/5762)

This patch retains the existing renumbered-bus fallback after the complete exact-match scan. It does not attempt to solve every ambiguous multi-GPU fallback case. The VBIOS correction alone did not resolve the low-clock issue on the tested machine.

## Matching distribution sources

- Bazzite test: [OGC source tag v7.2.4-ogc3](https://github.com/OpenGamingCollective/linux/releases/tag/v7.2.4-ogc3), installed headers/compiler for `7.2.4-ogc3.1.fc44.x86_64`.
- Omarchy preparation: [linux-omarchy packaging revision 7b11c976](https://github.com/omacom/omarchy-pkgs/tree/7b11c97603dd9d751d803746560ee51640709725/pkgbuilds/linux-omarchy), package `7.2.5-3` and its matching headers.

Do not substitute vanilla upstream source for a distribution kernel that carries additional patches.
