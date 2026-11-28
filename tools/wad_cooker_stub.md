# WAD cooker skeleton (post-M1)

Not required to load FreeDoom yet. Future host tool will:

1. Read FreeDoom IWAD/PWAD lumps  
2. Emit intermediate sector/portal mesh  
3. Cook `.map.bin` / `.tex.bin` matching `include/engine/world.h` layouts  

M1 uses the hand-authored two-room fixture in `src/world/map_tworoom.c` instead.
