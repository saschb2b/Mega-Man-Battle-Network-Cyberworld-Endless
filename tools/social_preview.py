#!/usr/bin/env python3
"""build/social-preview.png: the repository's social preview (1280 x 640), for
GitHub's Settings > General > Social preview, which only takes an upload.

The site's PET floor (web/assets/ui/floor.png, 3x) behind two of the game's
screenshots at 2x (docs/screenshots: the title and a guardian's card) and
the pitch beneath them. The picture holds whole frames of the game, like the
README's screenshots: it is written to build/, not committed."""
import os
import sys

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
W, H = 1280, 640
INK = (33, 74, 82)          # the site's text edge (--edge)
WHITE = (247, 255, 247)
GOLD = (255, 214, 16)
FONT = '/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'


def font(size):
    try:
        return ImageFont.truetype(FONT, size)
    except OSError:
        sys.exit(f'{FONT} is missing (Debian and Ubuntu: fonts-dejavu-core)')


def edged(d, xy, text, f, fill):
    """Text with the site's one-pixel ink edge, two pixels at this size."""
    x, y = xy
    for dx in (-2, 0, 2):
        for dy in (-2, 0, 2):
            if dx or dy:
                d.text((x + dx, y + dy), text, font=f, fill=INK)
    d.text((x, y), text, font=f, fill=fill)


def centred(d, y, text, f, fill):
    w = d.textlength(text, font=f)
    edged(d, ((W - w) / 2, y), text, f, fill)


def main():
    floor = Image.open(os.path.join(ROOT, 'web', 'assets', 'ui', 'floor.png')).convert('RGB')
    floor = floor.resize((floor.width * 3, floor.height * 3), Image.NEAREST)
    im = Image.new('RGB', (W, H))
    for y in range(0, H, floor.height):
        for x in range(0, W, floor.width):
            im.paste(floor, (x, y))
    d = ImageDraw.Draw(im)
    shots = ['title', 'guardian']
    x = (W - (len(shots) * 480 + (len(shots) - 1) * 40)) // 2
    for name in shots:
        shot = Image.open(os.path.join(ROOT, 'docs', 'screenshots', name + '.png')).convert('RGB')
        shot = shot.resize((480, 320), Image.NEAREST)
        d.rectangle((x - 6, 36, x + 486, 362), fill=INK)
        im.paste(shot, (x, 42))
        x += 520
    centred(d, 400, 'A roguelike for Mega Man Battle Network 6', font(46), WHITE)
    centred(d, 470, 'A new net every run, on the real game from your own ROM', font(30), WHITE)
    centred(d, 540, 'PortMaster  ·  Linux  ·  browser', font(30), GOLD)
    out = os.path.join(ROOT, 'build', 'social-preview.png')
    os.makedirs(os.path.dirname(out), exist_ok=True)
    im.save(out, optimize=True)
    print(out)


if __name__ == '__main__':
    main()
