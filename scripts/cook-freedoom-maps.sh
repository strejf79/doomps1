#!/usr/bin/env bash
# Cook FreeDoom Phase-1 subsets into assets/maps + assets/disc.
# IWAD is host-local (not vendored). Example:
#   ./scripts/cook-freedoom-maps.sh third_party/iwads/freedoom1.wad
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WAD="${1:-$ROOT/third_party/iwads/freedoom1.wad}"
if [[ ! -f "$WAD" ]]; then
  echo "missing IWAD: $WAD" >&2
  exit 1
fi
mkdir -p "$ROOT/assets/maps" "$ROOT/assets/disc"
python3 "$ROOT/tools/cook_map.py" --wad "$WAD" --map E1M1 --scale 8 --max-sectors 10 \
  -o "$ROOT/assets/maps/freedoom_e1m1.map.bin" --embed "$ROOT/src/resources/map_embedded.c"
python3 "$ROOT/tools/cook_map.py" --wad "$WAD" --map E1M2 --scale 10 --max-sectors 12 \
  -o "$ROOT/assets/maps/freedoom_e1m2.map.bin"
python3 "$ROOT/tools/cook_map.py" --wad "$WAD" --map E1M3 --scale 8 --max-sectors 12 \
  -o "$ROOT/assets/maps/freedoom_e1m3.map.bin"
cp "$ROOT/assets/maps/freedoom_e1m1.map.bin" "$ROOT/assets/disc/map.bin"
cp "$ROOT/assets/maps/freedoom_e1m2.map.bin" "$ROOT/assets/disc/map2.bin"
cp "$ROOT/assets/maps/freedoom_e1m3.map.bin" "$ROOT/assets/disc/map3.bin"
echo "disc maps ready: MAP.BIN MAP2.BIN MAP3.BIN"
