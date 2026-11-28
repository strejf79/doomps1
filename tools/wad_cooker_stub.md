# WAD → `.map.bin` cooker (DPM1 v2)

```bash
python3 tools/cook_map.py --wad path/to/freedoom1.wad --map E1M1 \
  --scale 8 --max-sectors 16 \
  -o assets/maps/freedoom_e1m1.map.bin --embed src/resources/map_embedded.c
```

**Budgets (fail the cook if exceeded):** 24 sectors, 384 walls, 32 things, 12288 bytes.

**v2 adds:** things (enemies/pickups/exit), exit marker, far LOD flags, door-candidate wall flags.

FreeDoom IWAD is not vendored; cook locally.
