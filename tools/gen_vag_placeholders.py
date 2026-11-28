#!/usr/bin/env python3
# Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
"""Generate tiny mono VAGp ADPCM placeholders for plan M6 SFX/music stubs."""
from __future__ import annotations

import math
import struct
from pathlib import Path

OUT = Path(__file__).resolve().parents[1] / "assets" / "sfx"


def encode_pcm16_to_vag_blocks(samples, loop: bool = False) -> bytes:
    samples = list(samples)
    pad = (28 - (len(samples) % 28)) % 28
    samples += [0] * pad
    nblocks = len(samples) // 28
    blocks: list[bytes] = []
    for bi in range(nblocks):
        chunk = samples[bi * 28 : (bi + 1) * 28]
        peak = max(abs(s) for s in chunk) or 1
        shift = 0
        while shift < 12 and (peak >> shift) > 7:
            shift += 1
        flags = 0x01
        if loop and bi == 0:
            flags = 0x06
        if loop and bi == nblocks - 1:
            flags = 0x03
        if not loop and bi == nblocks - 1:
            flags = 0x01
        data = bytearray(16)
        data[0] = shift & 0x0F
        data[1] = flags
        for i, s in enumerate(chunk):
            nibble = max(-8, min(7, s >> shift)) if shift else max(-8, min(7, s))
            byte_i = 2 + (i // 2)
            if i % 2 == 0:
                data[byte_i] = nibble & 0x0F
            else:
                data[byte_i] |= (nibble & 0x0F) << 4
        blocks.append(bytes(data))
    if not blocks:
        blocks.append(bytes([0x00, 0x01] + [0] * 14))
    return b"".join(blocks)


def write_vag(path: Path, pcm, sample_rate: int = 22050, name: str = "sfx", loop: bool = False) -> None:
    body = encode_pcm16_to_vag_blocks(pcm, loop=loop)
    header = struct.pack(">4sIIII", b"VAGp", 0x20, 0, len(body), sample_rate)
    header += b"\0" * 12
    header += name.encode("ascii")[:16].ljust(16, b"\0")
    path.write_bytes(header + body)
    print(f"{path.name}: {path.stat().st_size} bytes loop={loop}")


def tone(freq: float, dur_s: float, rate: int = 22050, amp: int = 12000):
    n = int(rate * dur_s)
    out = []
    attack = int(rate * 0.005)
    release = int(rate * 0.02)
    for i in range(n):
        v = int(amp * math.sin(2 * math.pi * freq * (i / rate)))
        env = 1.0
        if i < attack:
            env = i / max(1, attack)
        elif i > n - release:
            env = (n - i) / max(1, release)
        out.append(int(v * env))
    return out


def noise_burst(dur_s: float, rate: int = 22050, amp: int = 8000):
    n = int(rate * dur_s)
    x = 1234567
    out = []
    for i in range(n):
        x = (1103515245 * x + 12345) & 0x7FFFFFFF
        v = int(((x / 0x7FFFFFFF) * 2 - 1) * amp)
        env = 1.0 - (i / max(1, n - 1))
        out.append(int(v * env * env))
    return out


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    write_vag(OUT / "pistol.vag", tone(880, 0.06, amp=14000), name="pistol")
    write_vag(OUT / "shotgun.vag", tone(220, 0.10, amp=16000) + noise_burst(0.05, amp=10000), name="shotgun")
    write_vag(OUT / "chaingun.vag", tone(660, 0.04, amp=12000), name="chaingun")
    write_vag(OUT / "hurt.vag", tone(140, 0.12, amp=10000), name="hurt")
    write_vag(OUT / "footstep.vag", noise_burst(0.04, amp=5000), name="foot")
    write_vag(OUT / "music_loop.vag", tone(110, 0.25, amp=2000), name="music", loop=True)
    write_vag(OUT / "silence.vag", [0] * 56, name="silence", loop=True)


if __name__ == "__main__":
    main()
