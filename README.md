# D700macpro

Two AMDGPU fixes tested on the **2013 Mac Pro (MacPro6,1) with dual FirePro D700 GPUs**.

The investigated system remained at **300 MHz core / 150 MHz memory**, including under load. With both fixes it reached **850 MHz core / 1,370 MHz memory** automatically, accepted factory performance requests, and returned to idle normally.

## The two bugs

1. **Wrong VBIOS selected for the second GPU.** The firmware table contains separate `113-C3861XA-028` and `113-C3861XB-028` images. Relaxed PCI bus matching can select the first image for both cards. Patch 1 scans for exact matches before allowing the existing fallback.
2. **Desktop GPUs treated as battery-capable devices.** These cards advertise `HARDWAREDC` and have a 300/150 MHz battery state. Patch 2 limits the SMC's GPIO DC mode to mobile GPUs. This was the change that restored clock scaling.

The power-state change is a backport of **Timur Kristóf's September 23, 2026 upstream patch**, reviewed by Mario Limonciello. It is not an original invention of this repository. The separate VFCT patch was developed and tested during this investigation. See [provenance](docs/provenance.md).

## Results

| Ten-second offscreen shader test | VBIOS fix alone | Both fixes |
| --- | --- | --- |
| Display GPU | 654 frames, 300/150 MHz | 1,797 frames, 850/1,370 MHz |
| Other GPU | 659 frames, 300/150 MHz | 1,821 frames, 850/1,370 MHz |

That is about **2.75× more frames in this diagnostic workload**, not a promise about every game. Peak observed GPU temperature was 79°C during the short test. Automatic mode and idle recovery were verified; no new GPU errors were observed.

Validated runtime: Bazzite 44.20260921, `7.2.4-ogc3.1.fc44.x86_64`, Mesa 26.2.2. Managed Bazzite persistence has also booted successfully. The Omarchy `7.2.5-3-omarchy` module builds successfully with matching identity and dependencies; installation and runtime validation are pending.

## Use

Start with [diagnosis and validation](docs/validation.md). Prefer a distribution kernel containing the upstream corrections when available.

To build locally, obtain the **exact distribution kernel source, including its patches**, plus the matching installed headers and compiler. Then:

```bash
bash scripts/build-module.sh /path/to/matching/kernel-source ./artifacts
bash scripts/show-gpus.sh
```

The build script modifies the provided source tree. Use a dedicated source checkout. It does not install a module, modify firmware, change boot settings, or reboot.

Installation depends on the distribution:

- [Bazzite / rpm-ostree](docs/bazzite.md)
- [Omarchy / Arch and Limine](docs/omarchy.md)

**A built module is specific to one kernel release.** Do not copy the Bazzite module to Omarchy, or reuse it after a kernel update. The persistence method described here survives normal reboots on the matched kernel; a different kernel needs its own build or the upstream fix. No security enforcement is disabled by these instructions.

This repository contains source patches and build instructions, not GPU firmware images or prebuilt kernel modules.

## License

GPL-2.0-only. Upstream authorship and source links are preserved in [provenance](docs/provenance.md).
