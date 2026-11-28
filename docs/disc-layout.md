# Disc layout, prefetch, and PAL timing (plan M5)

**Status:** scaffolding complete without real PS1 hardware.  
**Proxy:** DuckStation + host BIOS is the smoke path. Hardware timing will differ; design is fail-soft so slow seeks never stall the 30 Hz tick.

## Cue / bin image

`make` → `build/doomps1.cue` + `build/doomps1.bin` via PSn00bSDK `psn00bsdk_add_cd_image` / mkpsxiso from `iso.xml`.

| ISO9660 path | Role |
|--------------|------|
| `SYSTEM.CNF` | Boot config → `DOOMPS1.EXE` |
| `DOOMPS1.EXE` | Engine PS-EXE |
| `LICENSE` | Proprietary stub |
| `DATA/MAP.BIN` | FreeDoom E1M1 subset (embedded fail-soft too) |
| `DATA/MAP2.BIN` | FreeDoom E1M2 subset |
| `DATA/MAP3.BIN` | FreeDoom E1M3 subset |
| `DATA/PREFETC.TOC` | Prefetch adjacency list |
| `DATA/SFX.BIN` | Placeholder SFX bank header |

Volume id: `DOOMPS1_M6X`. Single Mode-2 data track. SFX currently embed in the EXE via `.VAG` incbin; `DATA/SFX.BIN` remains a disc placeholder for a future streamed bank.

### Host checks

```bash
source third_party/psn00bsdk/env.sh && make
./scripts/verify-disc-layout.sh
```

## Prefetch model

- Combat working set stays in RAM (embedded map + entity pools).
- `disc_prefetch(path, priority)` queues ISO paths; `disc_update()` warms **one** sector when the CD is idle.
- Never call blocking multi-megabyte reads from the sim tick.
- Adjacent rooms / SFX banks are hints only until streaming packs land.

## Fail-soft seek path

| Failure | Runtime behavior |
|---------|------------------|
| `CdInit` fails | Embedded assets only; game still boots |
| `CdSearchFile` miss | Log + continue; keep embedded map |
| Slow seek / timeout | Abort read, mark slot failed, keep playing |
| Truncated file | Read up to buffer cap; do not hang |

DuckStation seeks are optimistic vs real media (~150–300 KB/s + long seeks). Treat any hitch-free DuckStation run as a **necessary but not sufficient** hardware signal.

## PAL timing notes (first-class)

| Item | Lock |
|------|------|
| Framebuffer | **320×256** PAL |
| Sim tick | Fixed **30 Hz** (`time` module) |
| Display | PAL VSync; DuckStation region = PAL |
| NTSC | Secondary / later — do not ship below 320×256 |

On real hardware (future M5 exit): confirm the same `.cue` boots via PSIO/burned CDRW; measure seek stalls under combat; recalibrate prefetch priorities if door/elevator rooms hitch.

## DuckStation-as-proxy checklist

1. BIOS configured (not redistributed).
2. Console region **PAL**.
3. Boot `build/doomps1.cue`.
4. TTY: expect `disc: CdInit ok` when the CD image mounts; `disc: prefetch …` lines are optional warm reads.
5. If TTY shows `CdInit failed`, the EXE still runs from the emulated disc boot path using embedded data — investigate ISO only if boot itself fails.

## Out of scope here

- Physical PS1 / PSIO smoke  
- NTSC 320×240 ship config  
- Full `.lvl.pak` TOC streaming (stubbed via prefetch list only)
