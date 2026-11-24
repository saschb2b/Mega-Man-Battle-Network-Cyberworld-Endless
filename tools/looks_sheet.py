#!/usr/bin/env python3
"""MegaMan's looks' palettes as a contact sheet (docs/game/looks.md): each of
the twelve palettes' sixteen colours, read from src/core/looks.c as the game
keeps them (BGR555, shown as the GBA shows them), with its name from
looks_text.c and its index, then any game frames given, three to a row, under them.

    python3 tools/looks_sheet.py OUT.png [FRAME.png ...]

The colours are the project's own; frames are whole frames of the game
(a capture's, never a sprite cut out).
"""
import os
import re
import sys

from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SW, GAP, LABEL = 24, 2, 120


def palettes():
    src = open(os.path.join(ROOT, 'src', 'core', 'looks.c')).read()
    names = re.findall(r'"([^"]+)"', open(os.path.join(ROOT, 'src', 'core', 'looks_text.c')).read().split('{', 1)[1])
    rows = []
    for block in re.findall(r'\[LOOK_\w+\] = \{(.*?)\}', src, re.S):
        rows.append([int(x, 16) for x in re.findall(r'RGB\(0x([0-9A-Fa-f]{6})\)', block)])
    return list(zip(names, rows))


def gba(rgb):
    """24-bit RGB as BGR555 keeps it, then shown as the GBA's 5 bits widen."""
    r, g, b = (rgb >> 19) & 31, (rgb >> 11) & 31, (rgb >> 3) & 31
    return tuple(c << 3 | c >> 2 for c in (r, g, b))


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    pals = palettes()
    w = LABEL + 16 * (SW + GAP)
    h = 18 + len(pals) * (SW + GAP)
    frames = [Image.open(p).convert('RGB') for p in sys.argv[2:]]
    # (the frames three to a row under the swatches)
    cols = min(3, len(frames)) or 1
    fw = max((f.width for f in frames), default=0)
    fh = max((f.height for f in frames), default=0)
    rows = (len(frames) + cols - 1) // cols
    sheet = Image.new('RGB', (max(w, cols * (fw + GAP * 4)), h + rows * (fh + GAP * 4) + GAP * 4), (24, 24, 32))
    d = ImageDraw.Draw(sheet)
    for i in range(16):
        d.text((LABEL + i * (SW + GAP) + 6, 2), f'{i:X}', fill=(200, 200, 200))
    for row, (name, colours) in enumerate(pals):
        y = 18 + row * (SW + GAP)
        d.text((4, y + 6), f'{row:2d} {name}', fill=(230, 230, 230))
        for i, c in enumerate(colours):
            x = LABEL + i * (SW + GAP)
            d.rectangle([x, y, x + SW - 1, y + SW - 1], fill=gba(c))
    for n, f in enumerate(frames):
        sheet.paste(f, ((n % cols) * (fw + GAP * 4), h + GAP * 4 + (n // cols) * (fh + GAP * 4)))
    sheet.save(sys.argv[1])
    print(f'{sys.argv[1]}: {len(pals)} palettes, {len(frames)} frames')


if __name__ == '__main__':
    main()
