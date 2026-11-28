# Local PC handoff (DuckStation)

Cloud agents build the `.cue` here; **you** run DuckStation + BIOS on your machine. This is the supported playtest path.

## 1. Get the repo

```bash
git clone https://github.com/strejf79/doomps1.git
cd doomps1
git checkout engine
git pull
```

For textured walls before it lands on `engine`, use:

```bash
git checkout cursor/textured-walls-0fbf
git pull
```

Canonical trunk is **`engine`**. Do not merge legacy `master` (old PSXDOOM-RE tree) into it.

## 2. Toolchain

### Linux or WSL2 (recommended on Windows)

Needs: `curl`, `unzip`, `cmake` ≥ 3.21, `ninja`, `python3`.

```bash
./scripts/setup-psn00bsdk.sh
source third_party/psn00bsdk/env.sh
make
./scripts/verify-disc-layout.sh
```

### Native Windows (no WSL)

1. Install [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK/releases) **Windows** zip + Nugget `mipsel-none-elf` toolchain.
2. Set in PowerShell (adjust paths):

```powershell
$env:PSN00BSDK_LIBS = "C:\path\to\lib\libpsn00b"
$env:PATH = "C:\path\to\bin;" + $env:PATH
cmake --preset default .
cmake --build build
```

Or use the repo `Dockerfile` from WSL/Linux if native setup fights you.

## 3. DuckStation

1. Settings → BIOS → point at your PS1 BIOS (not in this repo).
2. Console region **PAL** (engine framebuffer is **320×256**).
3. File → Boot CD Image → `build/doomps1.cue` (same folder as `doomps1.bin`).
4. Optional: enable TTY / stdout for `printf` boot lines (`tex: wall64…`, `map: disc E1M1…`).

### Controls

| Input | Action |
|-------|--------|
| D-pad | Move / tank turn |
| L1 / L2 / R2 | Strafe |
| Cross / Square | Fire |
| Circle | Next weapon |
| Triangle | Use door / elevator |
| Select | Next map (E1M1→E1M2→E1M3) |

## 4. After you play

Note what breaks (black screen, no textures, wrong map, audio, crash) and paste TTY lines if any. Fixes land as PRs into `engine`.

## 5. Optional host cook

FreeDoom IWAD is **not** vendored:

```bash
./scripts/cook-freedoom-maps.sh /path/to/freedoom1.wad
make
```
