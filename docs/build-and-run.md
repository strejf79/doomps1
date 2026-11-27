# Build and run (DuckStation)

**Branch:** `engine` is the canonical from-scratch trunk. Do not merge the legacy `master` PSXDOOM-RE / psyq tree into it.

## Prerequisites

1. **PSn00bSDK + Nugget** (`mipsel-none-elf` GCC)  
   - Linux helper: `./scripts/setup-psn00bsdk.sh` then `source third_party/psn00bsdk/env.sh`  
   - Or install from [PSn00bSDK releases](https://github.com/Lameguy64/PSn00bSDK/releases) and set:
     ```bash
     export PSN00BSDK_LIBS=/path/to/lib/libpsn00b
     export PATH=/path/to/bin:$PATH
     ```
2. **CMake ≥ 3.21** and **Ninja** (or change the preset generator).
3. **DuckStation** on your machine, with a **PS1 BIOS** already configured (not redistributed here).

Optional: `docker build -t doomps1-sdk .` then `docker run --rm -v "$PWD":/src -w /src doomps1-sdk make`.

## Build

```bash
source third_party/psn00bsdk/env.sh   # if using the setup script
make
```

Artifacts under `build/`:

| File | Role |
|------|------|
| `doomps1.exe` | PS-EXE |
| `doomps1.bin` / `doomps1.cue` | CD image for DuckStation |

Equivalent CMake:

```bash
cmake --preset default .
cmake --build ./build
```

## Run in DuckStation

1. Open DuckStation → ensure BIOS is set (Settings → BIOS).
2. Prefer PAL console / region settings for this build (framebuffer is **320×256**).
3. File → Boot CD Image → select `build/doomps1.cue`.
4. Enable TTY / console logging if you want the `printf` boot line.

**Expected M0 picture:** dark blue clear, one gouraud-colored triangle centered on screen. D-pad moves it (tank forward/back + turn); L1/R1 (or L2/R2) strafe. No audio.

## What this milestone does *not* include

- FreeDoom / WAD cooker (starts after M0)
- Real SPU / music
- Any PSXDOOM-RE or psyq code

## Cloud / CI note

If the Nugget toolchain or PSn00bSDK zip cannot be installed in an environment (egress, missing release asset, etc.), sources remain compilable once `PSN00BSDK_LIBS` is valid. A missing toolchain is a setup failure, not a successful hardware run.
