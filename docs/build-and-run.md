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
2. **CMake ≥ 3.21** and **Ninja** (bundled with the SDK zip is fine).
3. **DuckStation** with a **PS1 BIOS** already configured (not redistributed here).

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

## Run in DuckStation

1. Open DuckStation → BIOS set (Settings → BIOS).
2. Prefer **PAL** console / region (framebuffer is **320×256**).
3. File → Boot CD Image → `build/doomps1.cue`.
4. Enable TTY if you want the boot `printf`.

**Expected (M1.5):** first-person walk between two rooms — **warm red** vs **cool cyan** static Gouraud tint; soft blend at the portal edge.

| Input | Action |
|-------|--------|
| D-pad Up/Down | Forward / back |
| D-pad Left/Right | Tank turn |
| L1 / R1 (or L2 / R2) | Strafe |

On-screen font shows sector, light RGB, vis count, position. Audio is silent (stub).

## Milestone notes

- **M0:** clear + hello triangle  
- **M1:** two rooms, portal flood, collision, GTE + double OT, dither off  
- **M1.5:** per-sector RGB static lighting + vertex Gouraud (`POLY_G4`; `POLY_GT*` with atlases later)  
- FreeDoom / WAD cooker: next milestone
