#!/usr/bin/env python3
"""The itch.io page's pictures (docs/ITCH.md): the cover in listings and
search, the banner over the page, the picture behind the player's Run game
button, the theme's background tile, and the screenshots at a whole scale.

The cover and banner are tools/steam_art.py's drawing (the infinity mark,
the CYBERWORLD ENDLESS lettering, the net's platforms in the site's PET
colours); --frame NAME draws the cover instead with one whole frame of the
game running from docs/screenshots in the site's screen frame (AGENTS.md:
whole frames only, never a crop). Each picture is drawn small and scaled by
a whole number.
The cover, banner, embed picture and tile are committed in docs/itch/; the screenshots are
written to .build/itch/ for the upload.

    python3 tools/itch_art.py [--frame NAME]
    python3 tools/itch_art.py --devlog [TAG ...]   # each release's devlog cover, .build/itch/covers/
"""
import argparse
import hashlib
import io
import os
import re
import subprocess
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import site_versions  # noqa: E402
import steam_art as art  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'docs', 'itch')
SHOTS_OUT = os.path.join(ROOT, '.build', 'itch')
SHOTS = os.path.join(ROOT, 'docs', 'screenshots')

# the site's screen frame (tools/site_art.py's 'screen'), outside in
SCREEN_RINGS = [(0, 123, 74), (107, 222, 255), (66, 198, 231), (66, 198, 231), (33, 115, 140)]

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
    """630 x 500, the cover with a frame (base 315 x 250, 2x): the wordmark at twice
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


def cover_art():
    """630 x 500, itch's cover (base 126 x 100, 5x): the wordmark stacked
    over the net, as the Steam capsule draws it."""
    im = art.sky(126, 100)
    art.net(im, 63, 52, [(-7, -1, 2, 2), (-5, 0, 3, 'u'), (-2, -1, 3, 3), (-1, 2, 3, 'v'), (-2, 5, 2, 2),
                         (4, -5, 2, 2), (5, -3, 3, 'v'), (4, 0, 3, 3), (7, 1, 3, 'u'), (1, 6, 3, 2),
                         (-6, 4, 3, 'u'), (-8, 3, 2, 2)], 8)
    art.wordmark(im, 63, 46, 1.0, True)
    return im, 5


def banner():
    """960 x 240, the page's banner (base 192 x 48, 5x): the wide capsule's
    net with the wordmark across it."""
    im = art.sky(192, 48)
    art.net(im, 96, 18, [(-11, -1, 2, 2), (-9, 0, 3, 'u'), (-6, -1, 3, 3), (-5, 2, 2, 'v'), (-6, 4, 2, 2),
                         (6, -6, 2, 2), (7, -4, 4, 'v'), (6, 0, 3, 3), (9, 1, 3, 'u'), (12, 0, 2, 3),
                         (1, 5, 3, 2), (-1, 5, 2, 'u')], 7)
    im.alpha_composite(logo_layer(192, 48, 96, 24, 0.8))
    return im, 5


def embed_bg(frame='central'):
    """960 x 640, behind the player's Run game button until it is clicked:
    one whole frame of the game at 4x (240 x 160), a layer of the net,
    dimmed a third towards the page's navy so the button stands out. No
    wordmark: the banner above carries it."""
    im = Image.open(os.path.join(SHOTS, frame + '.png')).convert('RGBA')
    im.alpha_composite(Image.new('RGBA', im.size, (11, 37, 50, 85)))
    return im, 4


def pictures(rel):
    """A release's notes' pictures in order, each as (commit or tag, path):
    from the release's tag, or the commit a raw address names."""
    out = []
    for url in re.findall(r'(?:src="|\]\()([^")]+\.(?:png|gif))', rel['md']):
        raw = re.match(r'https://raw\.githubusercontent\.com/[^/]+/[^/]+/([^/]+)/(.+)', url)
        if raw:
            out.append(raw.groups())
        elif '../' in url:
            out.append((rel['tag'], 'docs/' + url.split('../', 1)[1]))
    return out


def lead_picture(rel, used=()):
    """The release's cover picture, its first frame: its trailer's poster
    where its notes open with one; else the first of its pictures whose
    bytes no older release's cover took, title screens left out (Capcom's
    logo is no cover). Returns it, whether it is a trailer, and its bytes'
    SHA-1."""
    found = pictures(rel)
    trailer = bool(found) and found[0][1].endswith('trailer-play.png')
    if trailer:
        found = [(found[0][0], found[0][1].replace('trailer-play.png', 'trailer.png'))]
    candidates = [f for f in found if not re.search(r'/(title|marks)[^/]*$', f[1])] or found
    pick = None
    for ref, path in candidates:
        data = subprocess.run(['git', 'show', f'{ref}:{path}'], cwd=ROOT, capture_output=True, check=True).stdout
        digest = hashlib.sha1(data).hexdigest()
        if pick is None:
            pick = (data, digest)
        if digest not in used:
            pick = (data, digest)
            break
    im = Image.open(io.BytesIO(pick[0]))
    im.seek(0)
    return im.convert('RGBA'), trailer, pick[1]


def devlog_cover(rel, used=()):
    """1280 x 720, a release's devlog post's cover (itch: 16:9): its trailer's
    poster, or its picture (lead_picture), whole, at the largest whole scale
    the net's card holds, in the site's screen frame (rings 3 pixels each).
    Returns the cover and the picture's SHA-1."""
    im, trailer, pick = lead_picture(rel, used)
    if trailer:
        return (im.resize((1280, 720), Image.LANCZOS) if im.size != (1280, 720) else im), pick
    w, h = im.size
    k = w // 240 if w % 240 == 0 and h % 160 == 0 and w // 240 == h // 160 else 1
    im = im.resize((w // k, h // k), Image.NEAREST)
    w, h = im.size
    s = min(1216 // w, 656 // h)
    if s >= 1:
        im = im.resize((w * s, h * s), Image.NEAREST)
    else:
        d = -(-max(w, 1216) * 1 // 1216) if w * 656 >= h * 1216 else -(-h // 656)
        d = max(d, -(-w // 1216), -(-h // 656))
        im = im.resize((w // d, h // d), Image.LANCZOS)
    card = art.sky(256, 144)
    art.net(card, 128, 60, [(-15, -2, 2, 2), (-13, -1, 3, 'u'), (-10, -2, 3, 3), (10, -6, 2, 2), (11, -4, 4, 'v'),
                            (10, 0, 3, 3), (-12, 6, 3, 3), (-9, 7, 3, 'u'), (13, 5, 2, 2), (11, 6, 2, 'u')], 11)
    card = card.resize((1280, 720), Image.NEAREST)
    rings = [c for c in SCREEN_RINGS for _ in range(3)]
    framed = Image.new('RGBA', (im.width + 2 * len(rings), im.height + 2 * len(rings)))
    px = framed.load()
    for i, c in enumerate(rings):
        for x in range(i, framed.width - i):
            px[x, i] = px[x, framed.height - 1 - i] = c + (255,)
        for y in range(i, framed.height - i):
            px[i, y] = px[framed.width - 1 - i, y] = c + (255,)
    framed.paste(im, (len(rings), len(rings)))
    card.alpha_composite(framed, ((1280 - framed.width) // 2, (720 - framed.height) // 2))
    return card, pick


def tile():
    """48 x 48, the theme's background: the site's PET floor (web/assets/ui/floor.png) at 3x, as the site shows it."""
    return Image.open(os.path.join(ROOT, 'web', 'assets', 'ui', 'floor.png')).convert('RGBA'), 3


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--frame', help='a cover with this screenshot in it (docs/screenshots/NAME.png)')
    ap.add_argument('--devlog', nargs='*', metavar='TAG',
                    help="the releases' devlog covers into .build/itch/covers/ (no TAG: every release)")
    a = ap.parse_args()
    if a.devlog is not None:
        # (oldest first, each taking a picture no older cover took)
        rels = [r for r in reversed(site_versions.releases()) if r['date']]
        out = os.path.join(SHOTS_OUT, 'covers')
        os.makedirs(out, exist_ok=True)
        used = set()
        for rel in rels:
            cover, pick = devlog_cover(rel, used)
            used.add(pick)
            if a.devlog and rel['tag'] not in a.devlog:
                continue
            path = os.path.join(out, rel['tag'] + '.png')
            cover.convert('RGB').save(path, optimize=True)
            print(os.path.relpath(path, ROOT))
        return
    os.makedirs(OUT, exist_ok=True)
    for name, (im, k) in (('cover', cover(a.frame) if a.frame else cover_art()), ('banner', banner()), ('embed', embed_bg()), ('background', tile())):
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
