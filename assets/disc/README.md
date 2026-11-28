# Disc payload (plan M5 + post-M6 maps)

Files here are packaged into the CD image by `iso.xml`:

| ISO path | Source | Role |
|----------|--------|------|
| `DATA/MAP.BIN` | `freedoom_e1m1.map.bin` | Slot 0 (also embedded) |
| `DATA/MAP2.BIN` | `freedoom_e1m2.map.bin` | Slot 1 |
| `DATA/MAP3.BIN` | `freedoom_e1m3.map.bin` | Slot 2 |
| `DATA/PREFETC.TOC` | `prefetch.toc.txt` | Prefetch adjacency list |
| `DATA/SFX.BIN` | placeholder bank header | Reserved streamed ADPCM |

Recook: `./scripts/cook-freedoom-maps.sh /path/to/freedoom1.wad`

Real hardware smoke is **out of scope** in this VM. Use DuckStation + BIOS as the proxy; see `docs/disc-layout.md`.
