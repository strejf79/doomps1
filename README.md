# doomps1

From-scratch **PlayStation 1** Doom-*style* engine by Johannes Brorsson.

**Not a port of Doom. Not PSXDOOM-RE. Not psyq.** Clean-room C99 engine targeting real hardware and DuckStation.

| | |
|---|---|
| Toolchain | **PSn00bSDK + Nugget** (`mipsel-none-elf`) |
| Video | **PAL 320×256**, dither off |
| License | **Proprietary / All Rights Reserved** — see `LICENSE` |
| Canonical branch | **`engine`** |

## Current tip — post-M6 multi-map

Plan milestones **M0–M6** are on `engine`. Tip adds three budgeted FreeDoom subsets on disc (`MAP` / `MAP2` / `MAP3`); **Select** or **EXIT** advances the slot. SPU ADPCM SFX + music stub; disc prefetch fail-soft.

## Quick start

```bash
./scripts/setup-psn00bsdk.sh
source third_party/psn00bsdk/env.sh
make
# DuckStation → open build/doomps1.cue (BIOS required on your machine)
```

Optional re-cook (host IWAD, not vendored):

```bash
./scripts/cook-freedoom-maps.sh /path/to/freedoom1.wad
make
```

Full instructions: [docs/build-and-run.md](docs/build-and-run.md). Disc notes: [docs/disc-layout.md](docs/disc-layout.md).

## Layout

```
include/engine/   public headers
src/platform/     boot / VSync / disc
src/render/       GPU OT + GTE world submit
src/world/        sectors, portals, doors
src/player/       tank + strafe movement
src/entities/     monsters
src/weapons/      hitscan weapons
src/items/        pickups
src/audio/        SPU ADPCM + music stub
src/fx/           muzzle particles
tools/            WAD → .map.bin cooker (host)
assets/disc/      CD DATA payloads
assets/maps/      cooked FreeDoom subsets
```
