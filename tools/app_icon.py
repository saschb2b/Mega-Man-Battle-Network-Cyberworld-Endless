#!/usr/bin/env python3
"""The application icon: a 32x32 pixel-art tile in the site's PET colours
with the title's infinity mark, drawn from code (nothing from the ROM).

Writes linux/icons/<size>.png (32 to 512, whole-number scales),
windows/icon.ico (the .exe's and the installer's) and src/core/app_icon.h
(the 32x32 pixels, for the window icon). Run it after changing the drawing;
the outputs are committed.

    python3 tools/app_icon.py
"""
import math
import os

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
N = 32

OUTLINE = (33, 74, 82)
CYAN_HI = (107, 222, 255)
CYAN = (66, 198, 231)
TEAL = (33, 115, 140)
GREEN = (8, 189, 115)
GREEN_LINE = (74, 231, 115)
INK = (16, 54, 74)
GOLD_HI = (255, 247, 165)
GOLD = (255, 214, 16)
ORANGE = (247, 165, 0)


def inside(x, y, r):
    """Whether pixel (x, y) lies in the N x N square with corners rounded by r."""
    px, py = x + 0.5, y + 0.5
    dx = max(r - px, 0, px - (N - r))
    dy = max(r - py, 0, py - (N - r))
    return math.hypot(dx, dy) <= r if dx and dy else True


def depth(x, y, r):
    """How many rings in from the edge pixel (x, y) is."""
    for d in range(N // 2):
        if not inside_ring(x, y, d, r):
            return d
    return N // 2


def inside_ring(x, y, d, r):
    # the square shrunk by d pixels on each side
    px, py = x + 0.5 - d, y + 0.5 - d
    n, rr = N - 2 * d, max(r - d, 0)
    if px < 0 or py < 0 or px > n or py > n:
        return False
    dx = max(rr - px, 0, px - (n - rr))
    dy = max(rr - py, 0, py - (n - rr))
    return math.hypot(dx, dy) <= rr if dx and dy else True


def lemniscate(steps=720):
    """Points along an infinity sign centred in the tile."""
    pts = []
    for i in range(steps):
        t = 2 * math.pi * i / steps
        s = 1 + math.sin(t) ** 2
        pts.append((16 + 11 * math.cos(t) / s, 16 + 11 * math.sin(t) * math.cos(t) / s * 1.7))
    return pts


def draw():
    im = Image.new('RGBA', (N, N), (0, 0, 0, 0))
    curve = lemniscate()
    for y in range(N):
        for x in range(N):
            if not inside_ring(x, y, 0, 7):
                continue
            d = depth(x, y, 7)
            if d == 1:
                c = OUTLINE
            elif d in (2, 3):
                c = CYAN_HI if x + y < N - 1 else CYAN
            elif d == 4:
                c = TEAL
            else:
                # the PET's floor: green with a light diagonal every 8 pixels
                c = GREEN_LINE if (x + y) % 8 in (0, 1) else GREEN
            dist = min(math.hypot(x + 0.5 - cx, y + 0.5 - cy) for cx, cy in curve)
            if d >= 4 and dist < 2.6:
                c = INK
            if d >= 4 and dist < 1.5:
                c = GOLD_HI if y < 13 else GOLD if y < 18 else ORANGE
            im.putpixel((x, y), c + (255,))
    return im


def main():
    im = draw()
    out = os.path.join(ROOT, 'linux', 'icons')
    os.makedirs(out, exist_ok=True)
    for size in (32, 64, 128, 256, 512):
        im.resize((size, size), Image.NEAREST).save(os.path.join(out, f'{size}.png'), optimize=True)
    # the .exe's icon: whole-number scales, and 16 px smoothed down from 32
    ico = [im.resize((s, s), Image.NEAREST) for s in (256, 128, 64, 32)] + [im.resize((16, 16), Image.LANCZOS)]
    os.makedirs(os.path.join(ROOT, 'windows'), exist_ok=True)
    ico[0].save(os.path.join(ROOT, 'windows', 'icon.ico'), sizes=[i.size for i in ico], append_images=ico[1:])
    px = list(im.tobytes())
    lines = [', '.join(f'0x{b:02x}' for b in px[i:i + 16]) for i in range(0, len(px), 16)]
    with open(os.path.join(ROOT, 'src', 'core', 'app_icon.h'), 'w') as f:
        f.write('/* The window icon, 32x32 RGBA; drawn by tools/app_icon.py (do not edit). */\n')
        f.write('#pragma once\n#include <stdint.h>\n\n')
        f.write(f'enum {{ APP_ICON_SIZE = {N} }};\n')
        f.write(f'static const uint8_t app_icon_rgba[{len(px)}] = {{\n\t' + ',\n\t'.join(lines) + '\n};\n')
    print('wrote linux/icons, windows/icon.ico and src/core/app_icon.h')


if __name__ == '__main__':
    main()
