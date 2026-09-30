#!/usr/bin/env python3
"""Steam's library artwork for the game as a non-Steam shortcut: the
portrait capsule, the wide capsule, the hero banner and the logo that sits
on it (linux/steam/add-to-steam.py puts them in Steam's grid folder); and
the 3DS HOME Menu's banner for the CIA (3ds/banner.png).

Pixel art drawn from code, nothing from the ROM: the title's infinity mark
and its CYBERWORLD ENDLESS lettering (the same shapes scene_title.c draws),
over the net's platforms in the site's PET colours. Each picture is drawn
small and scaled by a whole number. Run it after changing the drawing; the
PNGs are committed.

    python3 tools/steam_art.py
"""
import math
import os
import random

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'linux', 'steam')

# the site's and the icon's colours
SKY_TOP = (4, 12, 30)
SKY_BOTTOM = (12, 58, 88)
GRID = (20, 64, 96)
GREEN = (8, 189, 115)
GREEN_HI = (74, 231, 115)
GREEN_LO = (0, 123, 74)
CYAN_HI = (107, 222, 255)
CYAN = (66, 198, 231)
TEAL = (33, 115, 140)
NAVY = (16, 82, 107)
EDGE = (33, 74, 82)
INK = (10, 22, 34)
WHITE = (247, 255, 247)
PLATE_HI = (198, 255, 140)
PLATE = (0, 189, 74)
PLATE_LO = (0, 140, 56)
GOLD_HI = (255, 247, 165)
GOLD = (255, 214, 16)
ORANGE = (247, 165, 0)
GOLD_LO = (198, 90, 0)

# ---- the title's shapes (scene_title.c) ----

GLYPHS = {
    'C': ('.#####.', '##...##', '##.....', '##.....', '##.....', '##.....', '##.....', '##...##', '.#####.'),
    'Y': ('##...##', '##...##', '##...##', '.##.##.', '..###..', '..###..', '..###..', '..###..', '..###..'),
    'B': ('######.', '##...##', '##...##', '##...##', '######.', '##...##', '##...##', '##...##', '######.'),
    'E': ('#######', '##.....', '##.....', '##.....', '######.', '##.....', '##.....', '##.....', '#######'),
    'R': ('######.', '##...##', '##...##', '##...##', '######.', '##.##..', '##..##.', '##...##', '##...##'),
    'W': ('##...##', '##...##', '##...##', '##.#.##', '##.#.##', '##.#.##', '##.#.##', '#######', '.##.##.'),
    'O': ('.#####.', '##...##', '##...##', '##...##', '##...##', '##...##', '##...##', '##...##', '.#####.'),
    'L': ('##.....', '##.....', '##.....', '##.....', '##.....', '##.....', '##.....', '##.....', '#######'),
    'D': ('######.', '##...##', '##...##', '##...##', '##...##', '##...##', '##...##', '##...##', '######.'),
    'N': ('##...##', '###..##', '####.##', '##.####', '##..###', '##...##', '##...##', '##...##', '##...##'),
    'S': ('.######', '##.....', '##.....', '##.....', '.#####.', '.....##', '.....##', '.....##', '######.'),
}
GLYPH_H, GLYPH_ADV, SPACE_ADV, LEAN = 9, 8, 5, 3


def text_mask(s):
    """The lettering as rows of 0/1, leaning like the logo."""
    w = (GLYPH_H - 1) // LEAN + 2 + sum(SPACE_ADV if c == ' ' else GLYPH_ADV for c in s) - 1
    m = [[0] * w for _ in range(GLYPH_H)]
    x = 0
    for c in s:
        if c == ' ':
            x += SPACE_ADV
            continue
        for r, row in enumerate(GLYPHS[c]):
            for k, p in enumerate(row):
                if p == '#':
                    m[r][x + k + (GLYPH_H - 1 - r) // LEAN] = 1
        x += GLYPH_ADV
    return m


def infinity_mask(scale):
    """The infinity mark: a lemniscate as a thick stroke, slanted, the stroke
    from the upper right to the lower left crossing over the other (2 marks
    the gap it leaves in the one beneath)."""
    a, b, rad, slant = 23.8 * scale, 40.8 * scale, 3.6 * scale, 0.42
    n = 360
    seg = [[(a * math.cos(t) / (1 + math.sin(t) ** 2), b * math.sin(t) * math.cos(t) / (1 + math.sin(t) ** 2))
            for t in (math.pi * (s + i / n) for i in range(n + 1))] for s in (0, 1)]
    w, h = int(a * 2 + rad * 2 + b * 0.4 * slant + 8), int(b * 0.8 + rad * 2 + 6)
    m = [[0] * w for _ in range(h)]
    for r in range(h):
        for c in range(w):
            py = r - h / 2
            px = c - w / 2 + py * slant
            d = [min((px - x) ** 2 + (py - y) ** 2 for x, y in seg[s]) for s in (0, 1)]
            if d[0] <= rad * rad:
                m[r][c] = 1
            elif d[1] <= rad * rad:
                m[r][c] = 2 if d[0] <= (rad + 1.2) ** 2 and abs(px) < 2 * rad + 2 else 1
    rows = [r for r in range(h) if any(m[r])]
    cols = [c for c in range(w) if any(m[r][c] for r in range(h))]
    return [row[cols[0]:cols[-1] + 1] for row in m[rows[0]:rows[-1] + 1]]


def stamp(im, m, x0, y0, fill, rings):
    """Draws mask m at (x0, y0): its rows in the fill colours from top to
    bottom, then an outline and rings around it (2 in the mask: outline)."""
    h, w = len(m), len(m[0])
    at = lambda r, c: m[r][c] if 0 <= r < h and 0 <= c < w else 0
    reach = len(rings)
    px = im.load()
    for r in range(-reach, h + reach):
        for c in range(-reach, w + reach):
            X, Y = x0 + c, y0 + r
            if not (0 <= X < im.width and 0 <= Y < im.height):
                continue
            v = at(r, c)
            if v == 2:
                px[X, Y] = rings[0] + (255,)
            elif v == 1:
                px[X, Y] = fill[r * len(fill) // h] + (255,)
            else:
                d = min((max(abs(dr), abs(dc)) for dr in range(-reach, reach + 1) for dc in range(-reach, reach + 1)
                         if at(r + dr, c + dc)), default=reach + 1)
                if d <= reach:
                    px[X, Y] = rings[d - 1] + (255,)


INF_FILL = [GOLD_HI, GOLD_HI, GOLD, GOLD, GOLD, ORANGE, ORANGE, GOLD_LO]
TEXT_FILL = [PLATE_LO, PLATE, PLATE_HI, WHITE, WHITE, WHITE, PLATE_HI, PLATE, PLATE_LO]
RINGS = [INK, EDGE]


def size(m):
    return len(m[0]), len(m)


def wordmark(im, x, y, inf_scale, stacked):
    """The infinity mark and CYBERWORLD / ENDLESS: side by side, or the
    lettering under the mark; (x, y) is the middle of the whole."""
    inf = infinity_mask(inf_scale)
    top, bottom = text_mask('CYBERWORLD'), text_mask('ENDLESS')
    (iw, ih), (tw, th), (bw, _) = size(inf), size(top), size(bottom)
    gap = 3
    if stacked:
        total = ih + 6 + th * 2 + gap
        iy = y - total // 2
        stamp(im, inf, x - iw // 2, iy, INF_FILL, RINGS)
        ty = iy + ih + 6
        stamp(im, top, x - tw // 2, ty, TEXT_FILL, RINGS)
        stamp(im, bottom, x - bw // 2, ty + th + gap, TEXT_FILL, RINGS)
    else:
        total = iw + 7 + tw
        ix = x - total // 2
        stamp(im, inf, ix, y - ih // 2, INF_FILL, RINGS)
        tx = ix + iw + 7
        ty = y - (th * 2 + gap) // 2
        stamp(im, top, tx, ty, TEXT_FILL, RINGS)
        stamp(im, bottom, tx + 2, ty + th + gap, TEXT_FILL, RINGS)


# ---- the net ----

BAYER = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]


def sky(w, h, top=SKY_TOP, bottom=SKY_BOTTOM):
    """A dithered gradient with the net's grid faint over it."""
    im = Image.new('RGBA', (w, h))
    px = im.load()
    steps = 6
    for y in range(h):
        for x in range(w):
            f = y / max(h - 1, 1) * steps
            k = int(f) + (1 if (f - int(f)) * 16 > BAYER[y % 4][x % 4] else 0)
            t = min(k, steps) / steps
            c = tuple(round(a + (b - a) * t) for a, b in zip(top, bottom))
            # the grid: diagonal lines at the panels' slant, every 16 across
            if (x // 2 + y) % 16 == 0 or (x // 2 - y) % 16 == 0:
                c = tuple(round(v + (g - v) * 0.55) for v, g in zip(c, GRID))
            px[x, y] = c + (255,)
    return im


def platform(im, x0, y0, cols, rows, depth):
    """An isometric platform of cols x rows panels (16 x 8 pixels each), its
    back corner at (x0, y0): the PET's green panels, a light rim along the
    back edges, and sides `depth` pixels deep."""
    px = im.load()

    def uv(x, y):
        a, b = (x - x0) / 8, (y - y0) / 4
        return (a + b) / 2, (b - a) / 2

    bottom_x = x0 + (cols - rows) * 8
    x_lo, x_hi = x0 - rows * 8 - 1, x0 + cols * 8 + 1
    y_lo, y_hi = y0 - 1, y0 + (cols + rows) * 4 + depth + 2
    for y in range(max(y_lo, 0), min(y_hi, im.height)):
        for x in range(max(x_lo, 0), min(x_hi, im.width)):
            u, v = uv(x + 0.5, y + 0.5)
            if 0 <= u < cols and 0 <= v < rows:
                # the top: panels with a light edge up-left and a dark one down-right
                fu, fv = u % 1, v % 1
                if u < 0.13 or v < 0.13:
                    c = CYAN_HI
                elif fu < 0.1 or fv < 0.1:
                    c = GREEN_HI
                elif fu > 0.9 or fv > 0.9:
                    c = GREEN_LO
                else:
                    c = GREEN
                px[x, y] = c + (255,)
                continue
            # the sides: the top shape pushed down, left face and right face
            for d in range(1, depth + 1):
                u2, v2 = uv(x + 0.5, y + 0.5 - d)
                if 0 <= u2 < cols and 0 <= v2 < rows:
                    left = x < bottom_x
                    c = (TEAL if d > 1 else CYAN) if left else (NAVY if d > 1 else TEAL)
                    if d == depth:
                        c = EDGE
                    px[x, y] = c + (255,)
                    break


def sparkles(im, n, seed, box=None):
    """Data specks rising: short vertical streaks and crosses of light."""
    rnd = random.Random(seed)
    px = im.load()
    x_lo, y_lo, x_hi, y_hi = box or (0, 0, im.width, im.height)
    for _ in range(n):
        x, y = rnd.randrange(x_lo, x_hi), rnd.randrange(y_lo, y_hi)
        kind = rnd.random()
        if kind < 0.5:
            for k in range(rnd.randint(2, 5)):
                if 0 <= y + k < im.height:
                    px[x, y + k] = (CYAN_HI if k == 0 else CYAN if k < 2 else TEAL) + (255,)
        elif kind < 0.8:
            for dx, dy in ((0, 0), (1, 0), (-1, 0), (0, 1), (0, -1)):
                if 0 <= x + dx < im.width and 0 <= y + dy < im.height:
                    px[x + dx, y + dy] = (WHITE if (dx, dy) == (0, 0) else CYAN) + (255,)
        else:
            px[x, y] = CYAN_HI + (255,)


def net(im, ox, oy, pieces, seed, specks=True):
    """A net in one isometric grid: platforms (U, V, cols, rows) and the
    one-panel ways between them (U, V, length, 'u' or 'v'), in panels from
    (ox, oy), drawn back to front, then specks over it all."""
    shapes = []
    for p in pieces:
        if len(p) == 4 and p[3] in ('u', 'v'):
            u, v, n, axis = p
            shapes.append((u, v, n if axis == 'u' else 1, 1 if axis == 'u' else n, 3))
        else:
            u, v, cols, rows = p
            shapes.append((u, v, cols, rows, 5))
    for u, v, cols, rows, depth in sorted(shapes, key=lambda s: (s[0] + s[1], s[4])):
        platform(im, ox + (u - v) * 8, oy + (u + v) * 4, cols, rows, depth)
    if specks:
        sparkles(im, im.width * im.height // 300, seed)


# ---- the pictures ----

def capsule():
    """600 x 900: the portrait capsule of the library's shelves (base 120 x 180, 5x)."""
    im = sky(120, 180)
    # a way climbing from the bottom towards the mark
    net(im, 52, 104, [(0, 0, 2, 2), (2, 0, 3, 'u'), (5, -1, 3, 3), (6, 2, 3, 'v'), (5, 5, 3, 2),
                      (1, 2, 4, 'v'), (0, 6, 2, 2), (-4, 0, 2, 2), (-2, 0, 2, 'u'), (-4, 2, 3, 'v'),
                      (-5, 5, 3, 3), (-2, 6, 2, 'u'), (2, 8, 3, 3), (8, 5, 2, 2), (8, 7, 3, 'v')], 1)
    wordmark(im, 60, 56, 1.05, True)
    return im, 5


def wide():
    """920 x 430: the wide capsule of Recent Games and the grid (base 184 x 86, 5x)."""
    im = sky(184, 86)
    net(im, 92, 44, [(-9, -1, 2, 2), (-7, 0, 3, 'u'), (-4, -1, 3, 3), (-3, 2, 3, 'v'), (-4, 5, 2, 2),
                     (4, -6, 2, 2), (5, -4, 4, 'v'), (4, 0, 3, 3), (7, 1, 3, 'u'), (10, 0, 2, 3),
                     (0, 6, 3, 2), (-2, 6, 2, 'u')], 2)
    wordmark(im, 92, 38, 0.8, False)
    return im, 5


def hero():
    """1920 x 620: the banner behind the game's page (base 384 x 124, 5x); no
    lettering, the logo goes over it at its lower left, so the net stands to
    the right and a faint mark rises behind it."""
    base = (14, 70, 100)
    im = sky(384, 124, SKY_TOP, base)
    inf = infinity_mask(2.2)
    faint = Image.new('RGBA', im.size, (0, 0, 0, 0))
    stamp(faint, inf, 292 - len(inf[0]) // 2, 8, [(34, 104, 136)], [(22, 76, 108)])
    faint.putalpha(faint.getchannel('A').point(lambda a: a * 80 // 255))
    im.alpha_composite(faint)
    net(im, 262, 26, [(0, 0, 3, 3), (3, 1, 3, 'u'), (6, 0, 4, 3), (1, 3, 4, 'v'), (0, 7, 3, 2),
                      (-6, -1, 2, 2), (-4, 0, 4, 'u'), (8, 3, 3, 'v'), (7, 6, 4, 4), (3, 7, 4, 'u'),
                      (-3, 7, 3, 'u'), (-6, 6, 3, 3), (-9, 2, 2, 2), (-8, 4, 2, 'v'), (11, 1, 2, 'u'),
                      (13, 0, 2, 2), (11, 10, 2, 2), (9, 10, 2, 'u')], 3)
    return im, 5


def logo():
    """The logo over the hero: transparent, 1200 wide (base 150, 8x)."""
    inf = infinity_mask(0.8)
    iw, ih = size(inf)
    tw, th = size(text_mask('CYBERWORLD'))
    w, h = iw + 7 + tw + 4, max(ih, th * 2 + 3) + 4
    im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    wordmark(im, w // 2, h // 2, 0.8, False)
    return im, 8


def banner():
    """256 x 128: the banner the 3DS HOME Menu shows over the game's icon
    (base 128 x 64, 2x): the wide capsule's net under the wordmark."""
    im = sky(128, 64)
    net(im, 64, 30, [(-6, -1, 2, 2), (-4, 0, 3, 'u'), (-1, -1, 3, 3), (0, 2, 2, 'v'), (-1, 4, 2, 2),
                     (3, -5, 2, 2), (4, -3, 3, 'v'), (4, 0, 2, 2), (6, 0, 2, 'u')], 4)
    wordmark(im, 64, 26, 0.6, False)
    return im, 2


def main():
    os.makedirs(OUT, exist_ok=True)
    for name, make, out in (('capsule', capsule, OUT), ('wide', wide, OUT), ('hero', hero, OUT), ('logo', logo, OUT),
                            ('banner', banner, os.path.join(ROOT, '3ds'))):
        im, k = make()
        im = im.resize((im.width * k, im.height * k), Image.NEAREST)
        path = os.path.join(out, name + '.png')
        im.save(path, optimize=True)
        print(f'{path}: {im.width}x{im.height}')


if __name__ == '__main__':
    main()
