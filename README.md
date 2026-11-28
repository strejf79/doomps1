# doomps1

From-scratch **PlayStation 1** Doom-*style* engine by Johannes Brorsson.

**Not a port of Doom. Not PSXDOOM-RE. Not psyq.** Clean-room C99 engine targeting real hardware and DuckStation.

| | |
|---|---|
| Toolchain | **PSn00bSDK + Nugget** (`mipsel-none-elf`) |
| Video | **PAL 320×256**, dither off |
| License | **Proprietary / All Rights Reserved** — see `LICENSE` |
| Canonical branch | **`engine`** |

## Current milestone — M1

Two connected rooms you can walk through with Doom-like tank + strafe controls.

- Portal/sector graph (hand-authored test map)  
- Flat 1-quad floors & ceilings  
- Linedef wall quads (GTE → OT)  
- Double-buffered ordering table  
- Audio stub  

## Quick start

```bash
./scripts/setup-psn00bsdk.sh
source third_party/psn00bsdk/env.sh
make
# DuckStation → open build/doomps1.cue (BIOS required on your machine)
```

Full instructions: [docs/build-and-run.md](docs/build-and-run.md).

## Layout

```
include/engine/   public headers
src/platform/     boot / VSync
src/render/       GPU OT + GTE world submit
src/world/        sectors, portals, collision, two-room map
src/player/       tank + strafe movement
src/pad/          BIOS pad poll
src/audio/        no-op stub
tools/            WAD cooker stub (host, later)
```
