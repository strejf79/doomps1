# doomps1

From-scratch **PlayStation 1** Doom-*style* engine by Johannes Brorsson.

**Not a port of Doom. Not PSXDOOM-RE. Not psyq.** Clean-room C99 engine targeting real hardware and DuckStation.

| | |
|---|---|
| Toolchain | **PSn00bSDK + Nugget** (`mipsel-none-elf`) |
| Video | **PAL 320×256**, dither off |
| License | **Proprietary / All Rights Reserved** — see `LICENSE` |
| Canonical branch | **`engine`** |

## Current tip

Plan milestones **M0–M6** are on `engine` (multi-map disc slots + SPU stub). Active polish: **textured brick walls** (`POLY_GT4` + 4bpp TIM).

## Local PC (you + DuckStation)

Playtest is **on your machine** (BIOS stays local). Short path:

```bash
git clone https://github.com/strejf79/doomps1.git && cd doomps1
git checkout engine && git pull
./scripts/setup-psn00bsdk.sh          # Linux / WSL2
source third_party/psn00bsdk/env.sh
make
# DuckStation PAL → build/doomps1.cue
```

Full local handoff (Windows notes, controls, TTY): **[docs/local-dev.md](docs/local-dev.md)**.  
Also: [docs/build-and-run.md](docs/build-and-run.md), [docs/disc-layout.md](docs/disc-layout.md).

Optional re-cook (host IWAD, not vendored):

```bash
./scripts/cook-freedoom-maps.sh /path/to/freedoom1.wad
make
```


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
