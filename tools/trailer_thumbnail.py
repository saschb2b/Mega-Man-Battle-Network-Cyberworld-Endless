#!/usr/bin/env python3
"""A trailer's YouTube thumbnails: up to three, for Studio's Test &
compare, each an SVG of the house layout drawn to a 1280x720 PNG
(docs/project/youtube.md, The thumbnail; the youtube-publish skill's
thumbnail.md has the rules and why).

    python3 tools/trailer_thumbnail.py [VERSION] [--frame PATH]

VERSION is the newest cut by default. It reads the cut's THUMBNAILS, a
list of up to three:

    {'hero': (scene, sample),      # a frame of the cut's footage (or ('still', NAME), a docs/screenshots picture)
     'region': (x, y, w),          # the part of that frame shown, w game pixels wide (240, 160 or 120) and 16:9
     'words': ('CLOUD', 'SAVES!'), # two lines, four words at most in all
     'lit': 1,                     # which line is gold (the other white)
     'side': 'right',              # optional: the words and the logo on the right, for a subject on the left
     'point': (x, y)}              # optional: the orange cursor points here (game pixels of the frame)

and writes .build/youtube/cyberworld-endless-VERSION-thumbnail-a.png (-b,
-c), their .svg beside them, and ...-thumbnail-check.png: each one at
1280, and as small as YouTube shows it (320 and 168 wide) on its dark
and its light page, to read before any goes up. --frame PATH takes the
hero from a picture file instead (240x160), to try a layout.

The layout is fixed (the blueprint's half): the frame's region filling
the picture at a whole scale, a navy scrim under the words on the left,
the PET's cyan frame, the logo and the version's plate top left with a
green NEW, two lines of heavy slanted type bottom left with a thick navy edge,
the bottom right kept clear for YouTube's length stamp (with 'side':
'right' the scrim, the logo and the words are mirrored, the words raised
clear of the stamp: a battle's MegaMan stands on the left). The hero, its
region, the words and the pointer are the release's.

Needs Chrome or Chromium to draw the SVG (google-chrome, chromium, or
CHROME=path). The hero is a region of a whole frame with everything
around its subject kept: never a sprite cut out (AGENTS.md, Rules).
"""
import base64
import io
import os
import shutil
import subprocess
import sys

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import pil_nearest  # noqa: F401,E402

OUT = os.path.join(ROOT, '.build', 'youtube')
W, H = 1280, 720
# the words' face: Lato Black Italic, the heavy slanted sans a thumbnail is read in at 168 wide (Silkscreen Bold's
# S, W and 4 close up at that size: "NOW" read as "NO" and a heart); the site's Silkscreen where Lato is not installed
LATO = '/usr/share/fonts/truetype/lato/Lato-BlackItalic.ttf'
SILK = os.path.join(ROOT, 'web', 'assets', 'fonts', 'silkscreen-700.woff2')
FONT, FONT_KIND, LETTER = (LATO, 'ttf', 0.66) if os.path.exists(LATO) else (SILK, 'woff2', 0.86)
LOGO = os.path.join(ROOT, 'linux', 'steam', 'logo.png')
WIDTHS = (240, 160, 120)                  # a region's width in game pixels: the frame's whole width, two thirds, half
MAX_WORDS, MAX_LETTERS = 4, 9             # in all; in a line (nine letters still stand 130 px tall)
# the PET's colours (tools/site_art.py)
FRAME_HI, FRAME, FRAME_LO = '#6bdeff', '#42c6e7', '#108cef'
NAVY, GOLD, WHITE, GREEN, ORANGE = '#081e42', '#ffd610', '#f7fff7', '#10e729', '#ffad10'


def data_uri(im_or_path, kind='png'):
    if isinstance(im_or_path, str):
        with open(im_or_path, 'rb') as f:
            raw = f.read()
    else:
        buf = io.BytesIO()
        im_or_path.save(buf, 'PNG')
        raw = buf.getvalue()
    mime = {'png': 'image/png', 'woff2': 'font/woff2', 'ttf': 'font/ttf'}[kind]
    return f'data:{mime};base64,' + base64.b64encode(raw).decode()


def check(spec):
    """the rules a thumbnail's own half must keep (thumbnail.md)"""
    x, y, w = spec['region']
    h = w * 9 // 16
    if w not in WIDTHS:
        raise SystemExit(f'thumbnail: a region is {", ".join(map(str, WIDTHS))} game pixels wide, not {w}')
    if x < 0 or y < 0 or x + w > 240 or y + h > 160:
        raise SystemExit(f'thumbnail: the region {x},{y} {w}x{h} leaves the frame')
    words = spec['words']
    if len(words) > 2 or sum(len(line.split()) for line in words) > MAX_WORDS:
        raise SystemExit(f'thumbnail: two lines and {MAX_WORDS} words at most: {words}')
    for line in words:
        if len(line) > MAX_LETTERS:
            raise SystemExit(f'thumbnail: "{line}" is over {MAX_LETTERS} letters, too small to read at 168 wide')


def hero(frame, region):
    """the frame's region at a whole scale (3840 wide), for the SVG to
    show at 1280"""
    x, y, w = region
    part = frame.convert('RGB').crop((x, y, x + w, y + w * 9 // 16))
    return part.resize((3840, 2160), Image.NEAREST)


def svg(spec, frame, version):
    x, y, w = spec['region']
    k = W / w                              # a game pixel on the thumbnail
    lines = spec['words']
    lit = spec.get('lit', len(lines) - 1)
    # the words: as large as their longest line allows in 780 px, 190 at most
    size = min(190, int(780 / (max(len(line) for line in lines) * LETTER)))
    step = round(size * 1.06)
    right = spec.get('side') == 'right'
    base = H - (96 if right else 62) - step * (len(lines) - 1)
    wx, anchor = (W - 62, 'end') if right else (52, 'start')
    words = ''
    for i, line in enumerate(lines):
        fill = GOLD if i == lit else WHITE
        yy = base + i * step
        words += (f'<text x="{wx + 10}" y="{yy + 10}" font-size="{size}" fill="#020a1c" stroke="#020a1c" stroke-width="30" '
                  f'stroke-linejoin="round" text-anchor="{anchor}">{line}</text>'
                  f'<text x="{wx}" y="{yy}" font-size="{size}" fill="{fill}" stroke="{NAVY}" stroke-width="26" '
                  f'stroke-linejoin="round" paint-order="stroke" text-anchor="{anchor}">{line}</text>')
    pointer = ''
    if spec.get('point'):
        px, py = (spec['point'][0] - x) * k, (spec['point'][1] - y) * k
        # the PET's orange cursor arrow, its point on the subject, coming from the left
        pointer = (f'<g transform="translate({px:.0f},{py:.0f})">'
                   f'<polygon points="-150,-74 -24,0 -150,74" fill="#020a1c" transform="translate(8,10)"/>'
                   f'<polygon points="-150,-74 -24,0 -150,74" fill="{ORANGE}" stroke="{NAVY}" stroke-width="14" '
                   f'stroke-linejoin="round" paint-order="stroke"/>'
                   f'<polygon points="-138,-50 -52,0 -138,-22" fill="#ffce18"/></g>')
    logo = Image.open(LOGO)
    lw = 330
    lh = round(logo.height * lw / logo.width)
    plate_y = 34 + lh + 26
    bx = W - 44 - 342 if right else 44        # the brand's left edge: the logo, the version's plate and NEW
    g0, g1 = (1, 0) if right else (0, 1)
    return f'''<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="{W}" height="{H}" viewBox="0 0 {W} {H}">
<defs>
<style>@font-face {{ font-family: "Words"; src: url("{data_uri(FONT, FONT_KIND)}"); }}
text {{ font-family: "Words"; }}</style>
<linearGradient id="left" x1="{g0}" x2="{g1}" y1="0" y2="0"><stop offset="0" stop-color="{NAVY}" stop-opacity="0.92"/>
<stop offset="0.42" stop-color="{NAVY}" stop-opacity="0.62"/><stop offset="0.7" stop-color="{NAVY}" stop-opacity="0"/></linearGradient>
<linearGradient id="down" x1="0" x2="0" y1="0" y2="1"><stop offset="0.45" stop-color="{NAVY}" stop-opacity="0"/>
<stop offset="1" stop-color="{NAVY}" stop-opacity="0.8"/></linearGradient>
</defs>
<image href="{data_uri(hero(frame, spec['region']))}" x="0" y="0" width="{W}" height="{H}" preserveAspectRatio="none"/>
<rect width="{W}" height="{H}" fill="url(#left)"/>
<rect width="{W}" height="{H}" fill="url(#down)"/>
{pointer}
<rect x="6" y="6" width="{W - 12}" height="{H - 12}" fill="none" stroke="{FRAME_LO}" stroke-width="12"/>
<rect x="9" y="9" width="{W - 18}" height="{H - 18}" fill="none" stroke="{FRAME}" stroke-width="6"/>
<rect x="4" y="4" width="{W - 8}" height="{H - 8}" fill="none" stroke="{FRAME_HI}" stroke-width="3"/>
<image href="{data_uri(LOGO)}" x="{bx}" y="34" width="{lw}" height="{lh}"/>
<rect x="{bx}" y="{plate_y}" width="206" height="58" fill="#9cceef"/><rect x="{bx + 4}" y="{plate_y + 4}" width="198" height="50" fill="#102152"/>
<text x="{bx + 103}" y="{plate_y + 42}" font-size="36" fill="{GOLD}" text-anchor="middle">{version}</text>
<rect x="{bx + 220}" y="{plate_y}" width="122" height="58" fill="#cedeef"/><rect x="{bx + 224}" y="{plate_y + 4}" width="114" height="50" fill="{GREEN}"/>
<text x="{bx + 281}" y="{plate_y + 42}" font-size="36" fill="{WHITE}" stroke="#088c18" stroke-width="6" paint-order="stroke" text-anchor="middle">NEW</text>
{words}
</svg>'''


def chrome():
    for name in (os.environ.get('CHROME'), 'google-chrome', 'chromium', 'chromium-browser'):
        if name and shutil.which(name):
            return shutil.which(name)
    raise SystemExit('trailer_thumbnail: no Chrome to draw the SVG with (set CHROME=path)')


def draw(svg_path, png_path):
    """the SVG as a 1280x720 PNG, by headless Chrome"""
    page = svg_path[:-4] + '.html'
    with open(svg_path, encoding='utf-8') as f, open(page, 'w', encoding='utf-8') as out:
        out.write('<!doctype html><meta charset="utf-8"><style>html,body{margin:0;background:#000;overflow:hidden}'
                  'svg{display:block}</style>' + f.read())
    subprocess.run([chrome(), '--headless=new', '--disable-gpu', '--hide-scrollbars', '--force-device-scale-factor=1',
                    f'--window-size={W},{H}', '--virtual-time-budget=3000', f'--screenshot={png_path}', 'file://' + page],
                   check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    os.remove(page)
    im = Image.open(png_path).convert('RGB')
    if im.size != (W, H):
        im = im.crop((0, 0, W, H))
    im.save(png_path, optimize=True)


def sheet(paths, dest):
    """each thumbnail at 1280, and at 320 and 168 wide on YouTube's dark
    and light pages: what a viewer's eye gets"""
    n = len(paths)
    out = Image.new('RGB', (W + 40 + 720, n * (H + 20) + 20), (24, 24, 24))
    for i, p in enumerate(paths):
        im = Image.open(p).convert('RGB')
        y = 20 + i * (H + 20)
        out.paste(im, (20, y))
        for j, bg in enumerate(((15, 15, 15), (255, 255, 255))):
            x0 = W + 40 + j * 360
            out.paste(Image.new('RGB', (360, H), bg), (x0, y))
            out.paste(im.resize((320, 180), Image.LANCZOS), (x0 + 20, y + 30))
            out.paste(im.resize((168, 94), Image.LANCZOS), (x0 + 20, y + 250))
            out.paste(im.resize((120, 68), Image.LANCZOS), (x0 + 20, y + 380))
    out.save(dest, optimize=True)


def main():
    args = sys.argv[1:]
    frame_path = None
    if '--frame' in args:
        i = args.index('--frame')
        frame_path = args[i + 1]
        del args[i:i + 2]
    import trailer as kit
    import trailers
    version = args[0] if args else trailers.VERSIONS[-1]
    cut = trailers.module(version)
    specs = getattr(cut, 'THUMBNAILS', None)
    if not specs:
        raise SystemExit(f'{version}: its cut has no THUMBNAILS (the poster is its thumbnail)')
    if len(specs) > 3:
        raise SystemExit('three thumbnails at most: Test & compare takes three')
    kit.WORK = os.path.join(ROOT, '.build', 'trailer-' + version)
    os.makedirs(OUT, exist_ok=True)
    made = []
    for spec, letter in zip(specs, 'abc'):
        check(spec)
        frame = Image.open(frame_path) if frame_path else kit.shot(*spec['hero'])
        dest = os.path.join(OUT, f'cyberworld-endless-{version}-thumbnail-{letter}')
        with open(dest + '.svg', 'w', encoding='utf-8') as f:
            f.write(svg(spec, frame, version))
        draw(dest + '.svg', dest + '.png')
        made.append(dest + '.png')
        print('wrote', os.path.relpath(dest + '.png', ROOT), f'({os.path.getsize(dest + ".png") // 1024} KB)')
    check_path = os.path.join(OUT, f'cyberworld-endless-{version}-thumbnail-check.png')
    sheet(made, check_path)
    print('wrote', os.path.relpath(check_path, ROOT), '(look at it: the words at 168 wide)')


if __name__ == '__main__':
    main()
