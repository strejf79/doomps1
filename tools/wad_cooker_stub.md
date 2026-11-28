# WAD → intermediate → `.map.bin` cooker

**Status (M2):** working host tool `tools/cook_map.py`.

## Pipeline

```bash
# Fixture
python3 tools/cook_map.py --json maps/tworoom.json \
  -o assets/maps/tworoom.map.bin --embed src/resources/map_embedded.c

# FreeDoom (download IWAD separately; not committed)
python3 tools/cook_map.py --wad path/to/freedoom1.wad --map E1M1 \
  --scale 8 --max-sectors 10 \
  -o assets/maps/freedoom_e1m1.map.bin --embed src/resources/map_embedded.c
```

Then `make` on the console target. Runtime loads the embedded blob via `mapbin_load`.

## What it does

1. Parse Doom IWAD/PWAD lumps (VERTEXES, LINEDEFS, SIDEDEFS, SECTORS, THINGS).  
2. Flood sectors from player start (type 1) up to `--max-sectors`.  
3. Emit engine walls (1-/2-sided → solid/portal) + AABB floors + static RGB from lightlevel × palette.  
4. Scale Doom units (`--scale`) for GTE range.  
5. Write little-endian `.map.bin` (`DPM1`) and optional C embed.

## Limits (honest)

- Floors are **AABB quads**, not full sector polygons — fine for bring-up, not final FreeDoom fidelity.  
- No textures / `POLY_GT*` atlases yet.  
- Subset flood may drop distant rooms.  
- FreeDoom WADs are **not** vendored in-repo (license/size); CI/dev machines cook locally.

## Next

True portalized meshes, tex.bin, and failure-on-budget in the cooker.
