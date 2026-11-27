# doomps1

From-scratch **PlayStation 1** Doom-*style* engine by Johannes Brorsson.

**Not a port of Doom. Not PSXDOOM-RE. Not psyq.** Clean-room C99 engine targeting real hardware and DuckStation.

| | |
|---|---|
| Toolchain | **PSn00bSDK + Nugget** (`mipsel-none-elf`) |
| Video | **PAL 320×256** |
| License | **Proprietary / All Rights Reserved** — see `LICENSE` |
| Canonical branch | **`engine`** (orphan trunk; leave legacy `master` alone) |

## M0 status

- Clear-screen + colored gouraud triangle via Ordering Table  
- VSync double-buffer loop  
- Doom-like pad poll stub (tank move/turn, shoulder strafe)  
- Audio no-op stub  

## Quick start

```bash
./scripts/setup-psn00bsdk.sh
source third_party/psn00bsdk/env.sh
make
# Open build/doomps1.cue in DuckStation (BIOS required on your machine)
```

Full instructions: [docs/build-and-run.md](docs/build-and-run.md).

## Layout

```
include/engine/   public headers (platform, render, pad, audio, time, …)
src/              boot, render OT path, pad, audio stub, main loop
docs/             build / DuckStation notes
scripts/          PSn00bSDK setup helper
Dockerfile        optional Linux SDK image
```

Later milestones add world/sectors, WAD→cooker (FreeDoom), entities, weapons, and SPU audio — see the project design plan (not duplicated here).
