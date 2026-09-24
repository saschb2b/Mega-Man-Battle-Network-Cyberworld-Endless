#!/usr/bin/env python3
"""The project site's interface frames (web/assets/ui/*.png), drawn as pixel
art in the colours of BN6's PET screens and scaled 3x by CSS border-image.

Nothing here comes from the ROM: the shapes are drawn from code, only the
colours were measured from screenshots (docs/screenshots and the PET's own
screens). Run it after changing a frame; the PNGs are committed.

    python3 tools/site_art.py
"""
import math
import os

from PIL import Image

OUT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'web', 'assets', 'ui')

# the PET's colours (Cybeast Gregar's green PET)
GREEN = (8, 189, 115)
GREEN_LINE = (74, 231, 115)
DARK_GREEN = (0, 123, 74)
CYAN = (66, 198, 231)
CYAN_HI = (107, 222, 255)
CYAN_LO = (16, 140, 239)
WHITE = (247, 255, 247)
TEAL = (33, 115, 140)
NAVY = (16, 82, 107)
SLOT = (16, 99, 107)
SLOT_EDGE = (33, 74, 82)
PLATE = (0, 189, 74)
PLATE_HI = (198, 255, 140)
PLATE_LO = (0, 140, 56)
PLATE_OUT = (33, 74, 82)
PAPER = (247, 247, 247)
METAL_HI = (198, 222, 239)
METAL = (165, 189, 214)
METAL_LO = (99, 132, 148)
CARD = (222, 222, 239)
CARD_LO = (173, 181, 198)
INK = (41, 41, 41)
EXIT = (0, 49, 74)
OLIVE = (74, 123, 66)
GOLD = (255, 214, 16)
GOLD_LO = (198, 90, 0)


def rounded(size, r, layers, fill, bevel=None):
    """A square frame of `size` pixels: rings of (thickness, colour) from the
    outside in, corners rounded by `r`, then the fill. A colour may be a pair
    (upper-left, lower-right) for a bevel."""
    im = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    for y in range(size):
        for x in range(size):
            px, py = x + 0.5, y + 0.5
            # distance inside a rounded square of side `size` and radius r
            dx = max(r - px, 0, px - (size - r))
            dy = max(r - py, 0, py - (size - r))
            if dx > 0 and dy > 0:
                d = r - math.hypot(dx, dy)
            else:
                d = min(px, py, size - px, size - py)
            if d <= 0:
                continue
            depth, colour = 0, fill
            for thick, c in layers:
                if d <= depth + thick:
                    colour = c
                    break
                depth += thick
            if isinstance(colour[0], tuple):
                colour = colour[0] if px + py < size else colour[1]
            im.putpixel((x, y), colour + (255,))
    return im


def pill(width, height, layers, fill, flat_left=False):
    """A plate whose ends come to a point (the PET's stat pills), or with a
    flat left end and a slanted right one (its menu plates)."""
    im = Image.new('RGBA', (width, height), (0, 0, 0, 0))
    mid = (height - 1) / 2
    for y in range(height):
        slope = int(abs(y - mid) + 0.5)
        left = 0 if flat_left else slope
        right = width - 1 - (y // 2 if flat_left else slope)
        for x in range(left, right + 1):
            # distance to the outline, in whole pixels
            d = min(x - left, right - x, y, height - 1 - y)
            colour = fill
            depth = 0
            for thick, c in layers:
                if d < depth + thick:
                    colour = c
                    break
                depth += thick
            if isinstance(colour[0], tuple):
                colour = colour[0] if y < mid else colour[1]
            im.putpixel((x, y), colour + (255,))
    return im


def tile():
    """The PET screen's floor: green with a light diagonal every 16 pixels."""
    im = Image.new('RGB', (16, 16), GREEN)
    for i in range(16):
        for w in range(2):
            im.putpixel(((i + w) % 16, 15 - i), GREEN_LINE)
    return im


# 12x12 icons for the menu plates and rows, drawn here. Letters pick colours.
ICON_INK = {
    'k': (33, 74, 82), 'w': WHITE, 's': CARD_LO, 'c': CYAN, 'b': CYAN_LO, 'n': NAVY,
    'y': GOLD, 'o': (247, 165, 0), 'g': GREEN_LINE, 'r': (255, 41, 33),
}
ICONS = {
    # a cursor arrow: jack in
    'play': """
..kk........
..kyk.......
..kook......
..koook.....
..kooook....
..koooook...
..koooook...
..kooook....
..koook.....
..kook......
..kyk.......
..kk........""",
    # an arrow into a tray: download
    'get': """
....kkkk....
....kyyk....
....kyyk....
..kkkyykkk..
..kyyyyyyk..
...kyyyyk...
....kyyk....
.....kk.....
kk........kk
kck......kck
kcckkkkkkcck
kkkkkkkkkkkk""",
    # a battle chip
    'chip': """
.kkkkkkkkkk.
.kwwwwwwwwk.
.kwkkkkkkwk.
.kwkbccbkwk.
.kwkcyyckwk.
.kwkbccbkwk.
.kwkkkkkkwk.
.kwwwwwwwwk.
.kwoowwsswk.
.kwwwwwwwwk.
.kkkkkkkkkk.
............""",
    # steps down: the run, layer by layer
    'layers': """
kkkkkk......
kwccck......
kbbbbk......
kkkkkkkkk...
...kwccck...
...kbbbbk...
...kkkkkkkkk
......kwccck
......kbbbbk
......kkkkkk
............
............""",
    # a monitor with code: the source
    'code': """
kkkkkkkkkkkk
knnnnnnnnnnk
kngggnnnnnnk
knnnggggnnnk
kngggggnnnnk
knnnnnnnnnnk
kkkkkkkkkkkk
....kssk....
..kkkkkkkk..
..kssssssk..
..kkkkkkkk..
............""",
    # a monitor with a picture: a desktop
    'pc': """
kkkkkkkkkkkk
kbbbbbbbbbbk
kbbbwwbbbbbk
kbbbbbbbbbbk
kggggbbggggk
kggggggggggk
kkkkkkkkkkkk
....kssk....
..kkkkkkkk..
..kssssssk..
..kkkkkkkk..
............""",
    # a handheld
    'handheld': """
..kkkkkkkk..
..kssssssk..
..kskkkksk..
..kskcbksk..
..kskbcksk..
..kskkkksk..
..kssssssk..
..kkskssrk..
..kkkssrsk..
..kskssssk..
..kssssssk..
..kkkkkkkk..""",
    # a globe: the browser
    'web': """
....kkkk....
..kkcbbckk..
.kccbccbcck.
.kbbbbbbbbk.
kccbccccbcck
kccbccccbcck
kbbbbbbbbbbk
kccbccccbcck
.kccbccbcck.
.kbbbbbbbbk.
..kkcbbckk..
....kkkk....""",
    # a letter
    'mail': """
............
............
kkkkkkkkkkkk
kskwwwwwwksk
kwskwwwwkswk
kwwskwwkswwk
kwwwskkswwwk
kwwwwsswwwwk
kwwwwwwwwwwk
kkkkkkkkkkkk
............
............""",
}


def icon(rows):
    rows = rows.strip('\n').split('\n')
    im = Image.new('RGBA', (12, 12), (0, 0, 0, 0))
    for y, row in enumerate(rows):
        row = row[:12]
        for x, ch in enumerate(row):
            if ch in ICON_INK:
                im.putpixel((x, y), ICON_INK[ch] + (255,))
    return im


def main():
    os.makedirs(OUT, exist_ok=True)
    art = {
        # a panel: dark outline, cyan, a white line, cyan, teal, then navy
        'panel': rounded(15, 4, [(1, DARK_GREEN), (2, CYAN), (1, WHITE), (1, CYAN), (1, TEAL)], NAVY),
        # the same around a picture of the game
        'screen': rounded(11, 3, [(1, DARK_GREEN), (1, CYAN_HI), (2, CYAN), (1, TEAL)], (0, 0, 0)),
        # a slot for numbers and rows, sunk into a panel
        'slot': rounded(9, 3, [(1, SLOT_EDGE)], SLOT),
        # a row lit by the cursor
        'slot-on': rounded(9, 3, [(1, GOLD_LO)], (123, 99, 16)),
        # the navigator's text box: a metal rim, a green frame, paper
        'textbox': rounded(15, 4, [(1, METAL_LO), (1, (METAL_HI, METAL)), (1, METAL), (1, DARK_GREEN), (2, GREEN_LINE)], PAPER),
        # a chip's card, as the Library shows one
        'card': rounded(11, 2, [(1, INK), (1, (PAPER, CARD_LO))], CARD),
        # the dark plate the EXIT sits on
        'exit': rounded(9, 4, [(1, OLIVE)], EXIT),
        # menu plates: flat left, slanted right; the lit one brighter
        'plate': pill(20, 16, [(1, PLATE_OUT), (1, (PLATE_HI, PLATE_LO))], PLATE, flat_left=True),
        'plate-on': pill(20, 16, [(1, PLATE_OUT), (1, ((255, 255, 200), (0, 160, 64)))], (74, 231, 115), flat_left=True),
        # a stat pill, pointed at both ends
        'pill': pill(20, 13, [(1, CYAN_LO), (1, (CYAN_HI, CYAN))], CYAN),
        'pill-gold': pill(20, 13, [(1, GOLD_LO), (1, ((255, 247, 165), (247, 165, 0)))], (255, 198, 33)),
    }
    for name, im in art.items():
        im.save(os.path.join(OUT, f'{name}.png'), optimize=True)
    for name, rows in ICONS.items():
        art[f'icon-{name}'] = icon(rows)
        art[f'icon-{name}'].save(os.path.join(OUT, f'icon-{name}.png'), optimize=True)
    tile().save(os.path.join(OUT, 'floor.png'), optimize=True)
    print('wrote', ', '.join(sorted(list(art) + ['floor'])), 'to', OUT)


if __name__ == '__main__':
    main()
