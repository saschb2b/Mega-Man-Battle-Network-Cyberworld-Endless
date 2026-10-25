#!/usr/bin/env python3
"""The itch.io page's pictures (docs/ITCH.md): the cover in listings and
search, the banner over the page, the theme's background tile, and the
screenshots at a whole scale.

The cover and banner are tools/steam_art.py's drawing (the infinity mark,
the CYBERWORLD ENDLESS lettering, the net's platforms in the site's PET
colours), the cover with one whole frame of the game running from
docs/screenshots in the site's screen frame (AGENTS.md: whole frames only,
never a crop). Each picture is drawn small and scaled by a whole number.
The cover, banner and tile are committed in docs/itch/; the screenshots are
written to .build/itch/ for the upload.

    python3 tools/itch_art.py [--frame NAME]
"""
import argparse
import os
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import steam_art as art  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'docs', 'itch')
SHOTS_OUT = os.path.join(ROOT, '.build', 'itch')
SHOTS = os.path.join(ROOT, 'docs', 'screenshots')

# the site's screen frame (tools/site_art.py's 'screen'), outside in
SCREEN_RINGS = [(0, 123, 74), (107, 222, 255), (66, 198, 231), (66, 198, 231), (33, 115, 140)]

# the cover's frame by default: a battle's Custom screen against a guardian
COVER_FRAME = 'boss-custom'

# the page's screenshots, in the order itch shows them, each at 4x
SCREENSHOTS = ['net', 'boss-custom', 'guardian', 'liberation-turn', 'online-square', 'bn5-unite', 'home-news', 'setup']


def logo_layer(w, h, x, y, inf_scale):
    """The wordmark on its own transparent layer, centred on (x, y)."""
    im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    art.wordmark(im, x, y, inf_scale, False)
    return im


def framed(shot):
    """A 240x160 frame inside the site's screen rings."""
    k = len(SCREEN_RINGS)
    im = Image.new('RGBA', (shot.width + 2 * k, shot.height + 2 * k))
    px = im.load()
    for i, c in enumerate(SCREEN_RINGS):
        for x in range(i, im.width - i):
            px[x, i] = px[x, im.height - 1 - i] = c + (255,)
        for y in range(i, im.height - i):
            px[i, y] = px[im.width - 1 - i, y] = c + (255,)
    im.paste(shot.convert('RGBA'), (k, k))
    return im


def cover(frame):
    """630 x 500, itch's cover (base 315 x 250, 2x): the wordmark at twice
    the base over a whole frame of the game, the net round it."""
    im = art.sky(315, 250)
    art.net(im, 20, 168, [(0, 0, 3, 3), (3, 1, 2, 'u'), (-3, 2, 3, 'v'), (-4, 5, 2, 2), (2, 5, 2, 2)], 5)
    art.net(im, 290, 150, [(0, 0, 2, 2), (2, 0, 3, 'u'), (-1, 3, 3, 3), (-3, 1, 3, 'v'), (2, 5, 2, 2)], 6,
            specks=False)
    logo = logo_layer(144, 32, 72, 16, 0.8).resize((288, 64), Image.NEAREST)
    im.alpha_composite(logo, ((315 - 288) // 2, 4))
    shot = framed(Image.open(os.path.join(SHOTS, frame + '.png')).convert('RGB'))
    im.alpha_composite(shot, ((315 - shot.width) // 2, 72))
    return im, 2


def banner():
    """960 x 240, the page's banner (base 192 x 48, 5x): the wide capsule's
    net with the wordmark across it."""
    im = art.sky(192, 48)
    art.net(im, 96, 18, [(-11, -1, 2, 2), (-9, 0, 3, 'u'), (-6, -1, 3, 3), (-5, 2, 2, 'v'), (-6, 4, 2, 2),
                         (6, -6, 2, 2), (7, -4, 4, 'v'), (6, 0, 3, 3), (9, 1, 3, 'u'), (12, 0, 2, 3),
                         (1, 5, 3, 2), (-1, 5, 2, 'u')], 7)
    im.alpha_composite(logo_layer(192, 48, 96, 24, 0.8))
    return im, 5


def tile():
    """48 x 48, the theme's background: the site's PET floor (web/assets/ui/floor.png) at 3x, as the site shows it."""
    return Image.open(os.path.join(ROOT, 'web', 'assets', 'ui', 'floor.png')).convert('RGBA'), 3


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--frame', default=COVER_FRAME, help='the screenshot in the cover (docs/screenshots/NAME.png)')
    a = ap.parse_args()
    os.makedirs(OUT, exist_ok=True)
    for name, (im, k) in (('cover', cover(a.frame)), ('banner', banner()), ('background', tile())):
        im = im.resize((im.width * k, im.height * k), Image.NEAREST)
        path = os.path.join(OUT, name + '.png')
        im.save(path, optimize=True)
        print(f'{os.path.relpath(path, ROOT)}: {im.width}x{im.height}')
    os.makedirs(SHOTS_OUT, exist_ok=True)
    for i, name in enumerate(SCREENSHOTS, 1):
        im = Image.open(os.path.join(SHOTS, name + '.png')).convert('RGB')
        path = os.path.join(SHOTS_OUT, f'{i:02d}-{name}.png')
        im.resize((im.width * 4, im.height * 4), Image.NEAREST).save(path, optimize=True)
        print(f'{os.path.relpath(path, ROOT)}: {im.width * 4}x{im.height * 4}')


if __name__ == '__main__':
    main()
