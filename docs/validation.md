# Diagnosis and validation

## Identify the problem

Run `bash scripts/show-gpus.sh`, then repeat while a GPU workload is active. A 300/150 MHz idle reading is normal; staying there under load is the symptom.

On the investigated D700 pair:

- Each VBIOS declared `HARDWAREDC` and a 300/150 MHz battery state.
- Both controllers had already received valid performance levels up to 850/1,370 MHz.
- The live SMC system flags were 0x55, including GPIO DC mode.
- The driver classified PCI device 1002:6798 as desktop Tahiti, without `AMD_IS_MOBILITY`.
- With the desktop guard, system flags became 0x54 and clocks scaled normally.

The raw controller-memory addresses used during diagnosis were derived from the matching firmware headers. They are deliberately not provided as a universal write recipe.

## What was tested

The VFCT selection functions were tested against the actual firmware table: exact matching of both GPUs, reversed entry order, a nonmatching device ID, and the existing renumbered-bus fallback. The original code failed two of six checks; the patch passed all six with address and undefined-behavior sanitizers.

The full AMDGPU module was compiled against matching kernel headers. Its name, kernel identity, and dependencies were compared with the stock module. Its embedded initramfs copy was checked byte-for-byte before booting.

After boot:

1. Confirm each GPU loads its corresponding VBIOS; the tested pair uses XA-028 on 02:00.0 and XB-028 on 06:00.0.
2. Check automatic clocks under a short workload on each GPU.
3. Check temperature and kernel logs.
4. If testing factory performance controls, save and restore the original mode even on failure.
5. Confirm idle recovery and a normal display session.

The optional shader source in `tools/shader-load.c` reproduces the ten-second workload used for the published frame counts. It does not change clock settings. Build it on Linux with:

```bash
gcc -O2 -Wall -Wextra -Werror tools/shader-load.c \
  -Wl,-l:libEGL.so.1 -Wl,-l:libGLESv2.so.2 -o /tmp/d700-shader-load
timeout 15s env EGL_PLATFORM=surfaceless DRI_PRIME=pci-0000_06_00_0 /tmp/d700-shader-load
```

Select the actual PCI address for the GPU being tested. Watch temperature and stop if cooling is inadequate. The published run used an 80°C stop threshold. Short diagnostic tests do not establish sustained gaming or compute stability.
