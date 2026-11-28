# Disc payload (plan M5)

Files here are packaged into the CD image by `iso.xml`:

| ISO path | Source | Role |
|----------|--------|------|
| `DATA/MAP.BIN` | cooked FreeDoom subset | Optional disc-resident map (runtime still embeds a copy) |
| `DATA/PREFETC.TOC` | `prefetch.toc.txt` | Prefetch adjacency list for fail-soft seeks |
| `DATA/SFX.BIN` | placeholder bank header | Reserved for M6 ADPCM slabs on disc |

Real hardware smoke is **out of scope** in this VM. Use DuckStation + BIOS as the proxy; see `docs/disc-layout.md`.
