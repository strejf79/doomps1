#!/usr/bin/env python3
"""Generate a 64×64 4bpp brick TIM for wall texturing (host tool)."""
from __future__ import annotations

import struct
import sys
from pathlib import Path


def rgb15(r: int, g: int, b: int) -> int:
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def main() -> int:
    out = Path(sys.argv[1] if len(sys.argv) > 1 else "assets/tex/wall64.tim")
    w = h = 64
    clut = [
        rgb15(40, 28, 24),
        rgb15(150, 70, 50),
        rgb15(170, 85, 60),
        rgb15(120, 55, 40),
        rgb15(90, 90, 100),
    ] + [0] * 11

    pixels = [[0] * w for _ in range(h)]
    for y in range(h):
        row = y // 8
        yoff = y % 8
        for x in range(w):
            xshift = 16 if (row & 1) else 0
            xx = (x + xshift) % w
            xoff = xx % 16
            if yoff == 0 or xoff == 0:
                pixels[y][x] = 0
            else:
                pixels[y][x] = 1 + ((xoff + yoff + row) % 3)

    pix_bytes = bytearray()
    for y in range(h):
        for x in range(0, w, 2):
            lo = pixels[y][x] & 0xF
            hi = pixels[y][x + 1] & 0xF
            pix_bytes.append(lo | (hi << 4))

    mode = 0x08  # 4bpp + CLUT
    clut_data = b"".join(struct.pack("<H", c) for c in clut)
    clut_body = struct.pack("<IHHHH", 12 + len(clut_data), 0, 480, 16, 1) + clut_data
    pix_body = struct.pack("<IHHHH", 12 + len(pix_bytes), 320, 0, w // 4, h) + pix_bytes
    tim = struct.pack("<II", 0x10, mode) + clut_body + pix_body
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(tim)
    print(f"wrote {out} ({len(tim)} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
