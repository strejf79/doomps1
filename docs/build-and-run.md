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

**Expected (plan M4 slice):** budgeted FreeDoom subset + **doors** (Triangle), **pickups**, **pistol/shotgun/chaingun** (Circle cycles), staggered AI, exit.

| Input | Action |
|-------|--------|
| D-pad | Move / tank turn |
| L1 / L2 / R2 | Strafe |
| Cross / Square | Fire (chaingun full-auto while held) |
| Circle | Next weapon |
| Triangle | Use door / elevator |

Audio is silent (stub).

### Re-cook FreeDoom (host)

```bash
# Download FreeDoom IWAD yourself, then:
python3 tools/cook_map.py --wad path/to/freedoom1.wad --map E1M1 \
  --scale 8 --max-sectors 10 \
  -o assets/maps/freedoom_e1m1.map.bin --embed src/resources/map_embedded.c
make
```

## Milestone notes

- **M0–M1.5:** hello tri → two-room walk → colored static lighting  
- **M2:** WAD→`.map.bin` cooker + embedded FreeDoom E1M1 subset  
- **Combat:** hitscan + monster FSM + HUD  
- **Plan M3 budgeted:** cooker budgets, things/exit, LOD, muzzle FX  
- **Plan M4:** doors/elevators, pickups, 3 weapons, staggered AI  
