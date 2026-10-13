"""0.12.0's trailer, one Battle Network, in Battle Network 5's manner rather
than the first trailer's 2003 advert: a liberation mission. The net's
panels stand dark around the picture and turn blue as the shots go by, the
darkness takes them back for the DarkChips, and the logo frees them all.
The shots run in a field monitor with its readouts in the margins, the
words are typed and scanned in as a briefing's, a cut flips the picture's
panels over, and the dark shots glitch purple.

Its shots are whole frames of the game running from the developer's ROMs,
BN6's and BN5 Team Colonel's beside it, at 4x: the release pictures' own
scripted runs (build.py's SCREENSHOTS and CLIPS) and a few of this file's.
Everything else is drawn here; its music is v0_12_0_music.py's, an
original anthem in the manner of BN5's title.

    python3 build.py host && python3 tools/trailer.py 0.12.0 [--keep]
"""
import math
import os
import random
import shutil
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer as kit  # noqa: E402
from trailer import ROOT, from_clips, from_shots  # noqa: E402
from trailers import v0_12_0_music as music  # noqa: E402

VERSION = '0.12.0'
PLAY_LABEL = 'WATCH THE 0.12.0 TRAILER'


# (a BN6 layer and a BN6 battle, BN6's ROM alone, as most players have it)
PAGED = '100:,' + '4:A,6:,' * 30
BN6_ONLY = ('bn6-map', 'bn6-battle')
SCENES = {
    'oran': from_shots('bn5-oran', 420, 700),
    'nebula': from_shots('bn5-nebula', 424, 620),
    'undernet': from_shots('bn5-undernet', 420, 700),
    'scilab': from_shots('bn5-scilab', 420, 700),
    'bn6-map': (['--scene', 'emu', '--net-biome', '0', '--run-depth', '3', '--seed', '3', '--dev', 'quiet'], {},
                PAGED + '300:', 420, 700),
    'bn6-battle': (['--scene', 'emu', '--net-biome', '0', '--run-depth', '5', '--seed', '3', '--dev', 'quiet'],
                   {'CYBERWORLD_AUTOPILOT': '1'}, '300:battle', 380, 1000),
    'viruses': from_shots('bn5-viruses', 640, 940),
    'lava': from_shots('bn5-lava', 640, 940),
    'sea': from_shots('bn5-sea', 700, 1000),
    'toadman': from_shots('bn5-toadman', 1000, 2620),
    'knightman': from_shots('bn5-guardian', 1100, 2120),
    'unite': from_clips('bn5-one-net'),
    'chaos': from_shots('bn5-chaos', 1000, 1300),
    'flame': from_shots('bn5-dark-flame', 600, 1500),
    'darkinvs': from_clips('bn5-darkinvs'),
    'darkplus': from_shots('bn5-darkplus', 1380, 1700),
    'visitor': from_shots('bn5-visitor', 480, 700),
    'darksoul': from_shots('bn5-darksoul', 640, 900),
}


def scenes():
    return SCENES


# ---- the look: a liberation mission ----

FPS, W, H = 30, 1280, 720
BAR = 50                     # frames a bar at 144 beats a minute
BEAT = BAR / 4               # (12.5: the beats' frames alternate 12 and 13)
BARS = music.BARS
FRAMES = BARS * BAR + 78     # (the last chord's ring, then black)
POSTER = 18 * BAR + 34       # the logo over the freed net

HEAVY = kit.HEAVY
NARROW = '/usr/share/fonts/opentype/urw-base35/NimbusSansNarrow-Bold.otf'
MONO = '/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf'

INK = (3, 6, 16)
CYAN = (86, 218, 255)
AMBER = (255, 186, 52)
WHITE = (238, 246, 255)
VIOLET = (196, 84, 255)
RED = (255, 70, 70)

MX, MY, MW, MH = 160, 40, 960, 640     # the monitor: the game at 4x


def F(bar, beat=0):
    return round(bar * BAR + beat * BEAT)


def font(path, size):
    return ImageFont.truetype(path, size)


def lerp(a, b, t):
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def ease(t):
    t = max(0.0, min(1.0, t))
    return 1 - (1 - t) ** 3


# ---- the net's panels: dark, freed, and the flip between ----

PW, PH = 112, 56
FLIP = 6


def panel(fill, edge, inner):
    im = Image.new('RGBA', (PW, PH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)

    def diamond(k):
        return [(PW / 2, 1 + k), (PW - 2 - 2 * k, PH / 2), (PW / 2, PH - 2 - k), (1 + 2 * k, PH / 2)]
    d.polygon(diamond(0), fill=edge)
    d.polygon(diamond(3), fill=fill)
    d.polygon(diamond(11), outline=inner)
    d.line([(PW / 2 - 14, 9), (PW / 2, 4), (PW / 2 + 14, 9)], fill=lerp(edge, (255, 255, 255), 0.35), width=1)
    return im


DARK = panel((20, 7, 34), (84, 30, 132), (52, 18, 82))
DARK_PULSE = panel((30, 10, 50), (140, 56, 210), (84, 30, 130))
FREE = panel((8, 34, 70), (52, 162, 220), (24, 84, 140))
FREE_PULSE = panel((12, 48, 94), (130, 224, 255), (52, 140, 200))


def flip_frames():
    out = {}
    for to in (False, True):
        for k in range(FLIP):
            p = (k + 0.5) / FLIP
            src = (FREE if to else DARK) if p >= 0.5 else (DARK if to else FREE)
            h = max(2, round(PH * abs(math.cos(math.pi * p))))
            im = src.resize((PW, h), Image.BILINEAR)
            if 0.3 < p < 0.7:
                ImageDraw.Draw(im).line([(10, h // 2), (PW - 10, h // 2)], fill=(200, 245, 255, 255), width=2)
            out[(to, k)] = im
    return out


class Field:
    """The net around the monitor: each panel freed while its rank is under
    the freed share, so the share's rise frees them from the middle out
    and its fall darkens them from the edges in."""

    def __init__(self):
        rnd = random.Random(12)
        self.flip = flip_frames()
        cells = []
        for row in range(-2, H // (PH // 2) + 3):
            for col in range(-1, W // PW + 2):
                x = col * PW + (PW // 2 if row % 2 else 0) - PW // 2
                y = row * (PH // 2) - PH // 2
                d = math.hypot((x + PW / 2 - W / 2) / W, (y + PH / 2 - H / 2) / H * 1.3)
                cells.append([x, y, d + rnd.uniform(0, 0.22), False, -99, rnd.random()])
        order = sorted(range(len(cells)), key=lambda i: cells[i][2])
        for rank, i in enumerate(order):
            cells[i][2] = rank / len(order)
        self.cells = cells

    def update(self, fr, freed):
        for c in self.cells:
            want = c[2] < freed
            if want != c[3]:
                c[3], c[4] = want, fr

    def draw(self, im, fr, pulse=False, shown=1.0):
        for x, y, rank, state, since, appear in self.cells:
            if appear > shown:
                continue
            k = fr - since
            if k < FLIP:
                spr = self.flip[(state, k)]
            elif state:
                spr = FREE_PULSE if pulse else FREE
            else:
                spr = DARK_PULSE if pulse else DARK
            im.paste(spr, (x, y + (PH - spr.height) // 2), spr)


# the freed share: (frame, share), straight between
FREED = [(0, 0.0), (F(4), 0.0), (F(4) + 10, 0.24), (F(11, 3), 0.7), (F(12), 0.7), (F(12) + 14, 0.03),
         (F(14), 0.03), (F(14) + 10, 0.46), (F(17, 3), 0.9), (F(18), 0.9), (F(18) + 26, 1.01)]


def freed_at(fr):
    for (f0, v0), (f1, v1) in zip(FREED, FREED[1:]):
        if f0 <= fr < f1:
            return v0 + (v1 - v0) * (fr - f0) / (f1 - f0)
    return FREED[-1][1]


# ---- text ----

def text_image(text, path, size, color, track=0, stroke=0):
    """A line of text on transparency, its letters `track` apart; and each
    letter's right edge, for typing it out."""
    f = font(path, size)
    asc, desc = f.getmetrics()
    xs, x = [], 0
    for ch in text:
        x += f.getlength(ch) + track
        xs.append(round(x))
    im = Image.new('RGBA', (max(1, round(x)) + 2 * stroke + 4, asc + desc + 2 * stroke), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    x = stroke
    for ch in text:
        d.text((x, stroke), ch, font=f, fill=color, stroke_width=stroke, stroke_fill=INK)
        x += f.getlength(ch) + track
    return im, xs


class Typed:
    """Text typed out: `rate` letters a frame, a cursor block after them."""

    def __init__(self, text, path, size, color, track=0, rate=1.5, cursor=None):
        self.im, self.xs = text_image(text, path, size, color, track)
        self.rate, self.cursor, self.n = rate, cursor, len(text)

    def draw(self, im, x, y, k, alpha=1.0):
        if k < 0:
            return
        shown = min(self.n, int(k * self.rate) + 1)
        w = self.xs[shown - 1] if shown else 0
        part = self.im.crop((0, 0, max(1, w), self.im.height))
        if alpha < 1:
            part.putalpha(part.getchannel('A').point(lambda a: int(a * alpha)))
        im.paste(part, (x, y), part)
        if self.cursor and (shown < self.n or (k // 8) % 2 == 0) and alpha >= 1:
            ch = self.im.height
            ImageDraw.Draw(im).rectangle([x + w + 4, y + ch * 0.18, x + w + 4 + ch * 0.45, y + ch * 0.86],
                                         fill=self.cursor)


def plate(im, box, color=INK, alpha=200):
    x0, y0, x1, y1 = [round(v) for v in box]
    if x1 <= x0 or y1 <= y0:
        return
    im.paste(color, (x0, y0, x1, y1), Image.new('L', (x1 - x0, y1 - y0), alpha))


class Words:
    """A briefing's lower third (or upper): its kicker in amber, the
    headline typed, the caption under it, on a band that opens from the
    left and closes to a line."""

    def __init__(self, bar, beat, beats, kicker, head, caption='', where='low', dark=False):
        self.start, self.end = F(bar, beat), F(bar, beat + beats)
        self.where, self.dark = where, dark
        accent = VIOLET if dark else CYAN
        self.accent = accent
        self.kicker = Typed(kicker, NARROW, 24, AMBER if not dark else (255, 150, 230), track=3, rate=3)
        self.head = Typed(head, HEAVY, 70, WHITE, track=2, rate=1.2, cursor=accent)
        self.caption = None
        for size in range(25, 15, -1):
            if not caption:
                break
            self.caption = Typed(caption, NARROW, size, lerp(accent, WHITE, 0.55), track=1, rate=4)
            if self.caption.im.width <= MW - 140:
                break

    def draw(self, im, fr):
        k, n = fr - self.start, self.end - self.start
        if not 0 <= k < n:
            return
        y0 = MY + 470 if self.where == 'low' else MY + 18
        h = 150 if self.caption else 116
        close = max(0.0, (k - (n - 5)) / 5)
        open_ = ease(k / 6)
        x1 = MX + 40 + (MW - 80) * open_
        cy, half = y0 + h / 2, h / 2 * (1 - ease(close))
        plate(im, (MX + 40, cy - half, x1, cy + half), INK, 210)
        d = ImageDraw.Draw(im)
        d.rectangle([MX + 40, cy - half, MX + 46, cy + half], fill=self.accent)
        d.line([(MX + 40, cy - half), (x1, cy - half)], fill=self.accent, width=2)
        if close > 0:
            return
        self.kicker.draw(im, MX + 72, y0 + 12, k - 3)
        self.head.draw(im, MX + 68, y0 + 36, k - 6)
        if self.caption:
            self.caption.draw(im, MX + 72, y0 + 114, k - 14)


# ---- the monitor and its readouts ----

def monitor_frame(color):
    im = Image.new('RGBA', (MW + 40, MH + 40), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    x0, y0, x1, y1 = 16, 16, 23 + MW, 23 + MH
    d.rectangle([x0, y0, x1, y1], outline=color + (120,), width=1)
    arm = 38
    for cx, cy, sx, sy in ((x0, y0, 1, 1), (x1, y0, -1, 1), (x0, y1, 1, -1), (x1, y1, -1, -1)):
        d.line([(cx, cy), (cx + sx * arm, cy)], fill=color + (255,), width=4)
        d.line([(cx, cy), (cx, cy + sy * arm)], fill=color + (255,), width=4)
    for i in range(1, 32):
        x = x0 + i * (x1 - x0) / 32
        tall = 7 if i % 4 == 0 else 3
        d.line([(x, y0 - 2 - tall), (x, y0 - 3)], fill=color + (140,), width=1)
        d.line([(x, y1 + 3), (x, y1 + 2 + tall)], fill=color + (140,), width=1)
    return im


def scanlines():
    m = Image.new('L', (MW, MH), 0)
    d = ImageDraw.Draw(m)
    for y in range(3, MH, 4):
        d.line([(0, y), (MW, y)], fill=34)
    return m


class Hud:
    """The margins: the freed share and its meter on the left; on the
    right which net the shot is from, a radar and the signal."""

    def __init__(self):
        self.frames = {c: monitor_frame(c) for c in (CYAN, VIOLET)}
        self.lines = scanlines()
        self.label = {c: text_image('LIBERATED', NARROW, 21, c, track=3)[0] for c in (CYAN, VIOLET)}
        self.dark_label = text_image('DARKNESS', NARROW, 21, VIOLET, track=3)[0]
        self.nets = {(n, on): self.badge(n, on) for n in ('BN6', 'BN5') for on in (False, True)}
        self.small = {t: text_image(t, NARROW, 18, lerp(CYAN, INK, 0.3), track=3)[0]
                      for t in ('NET', 'RADAR', 'SIGNAL', 'CH 0.12.0', 'REC')}

    def badge(self, name, on):
        color = CYAN if name == 'BN6' else AMBER
        im = Image.new('RGBA', (60, 34), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        if on:
            d.rectangle([0, 0, 59, 33], fill=color + (255,))
        d.rectangle([0, 0, 59, 33], outline=color + (255 if on else 90,), width=2)
        f = font(NARROW, 24)
        d.text((30, 17), name, font=f, fill=(INK + (255,)) if on else color + (110,), anchor='mm')
        return im

    def draw(self, im, fr, freed, nets, dark, tab):
        accent = VIOLET if dark else CYAN
        frame = self.frames[accent]
        im.paste(frame, (MX - 20, MY - 20), frame)
        d = ImageDraw.Draw(im)
        # the left margin: the share freed
        plate(im, (14, 58, 146, 676), INK, 150)
        label = self.dark_label if dark else self.label[accent]
        im.paste(label, (26, 70), label)
        pct = f'{round(100 * min(freed, 1))}%'
        d.text((24, 96), pct, font=font(HEAVY, 46), fill=WHITE)
        top, bottom = 170, 650
        d.rectangle([28, top, 44, bottom], outline=accent, width=1)
        fill_y = bottom - (bottom - top) * min(1, freed)
        if fill_y < bottom - 2:
            d.rectangle([30, fill_y, 42, bottom - 2], fill=accent)
        for i in range(11):
            y = top + i * (bottom - top) / 10
            d.line([(48, y), (58 if i % 5 == 0 else 53, y)], fill=accent, width=1)
        for i, y in enumerate(range(186, 640, 26)):
            on = (i * 7 + fr // 4) % 9 < 5
            d.rectangle([70, y, 70 + (52 if i % 3 else 30), y + 3], fill=lerp(accent, INK, 0.25 if on else 0.75))
        # the right margin: the nets, the radar, the signal
        plate(im, (1134, 58, 1266, 676), INK, 150)
        im.paste(self.small['NET'], (1146, 66), self.small['NET'])
        for i, n in enumerate(('BN6', 'BN5')):
            b = self.nets[(n, n in nets)]
            im.paste(b, (1140 + 64 * i, 92), b)
        im.paste(self.small['RADAR'], (1146, 146), self.small['RADAR'])
        cx, cy, r = 1200, 240, 54
        for rr in (r, r * 0.6):
            d.ellipse([cx - rr, cy - rr, cx + rr, cy + rr], outline=lerp(accent, INK, 0.3), width=1)
        d.line([(cx - r, cy), (cx + r, cy)], fill=lerp(accent, INK, 0.6))
        d.line([(cx, cy - r), (cx, cy + r)], fill=lerp(accent, INK, 0.6))
        a = fr * 0.16
        sweep = Image.new('RGBA', (2 * r + 2, 2 * r + 2), (0, 0, 0, 0))
        sd = ImageDraw.Draw(sweep)
        for i in range(12):
            sd.pieslice([0, 0, 2 * r, 2 * r], math.degrees(a) - 4 * (i + 1), math.degrees(a) - 4 * i,
                        fill=accent + (120 - i * 10,))
        im.paste(sweep, (cx - r, cy - r), sweep)
        for bx, by in ((0.5, -0.3), (-0.4, 0.45), (0.15, 0.6), (-0.6, -0.5)):
            ang = math.atan2(by, bx)
            since = (a - ang) % (2 * math.pi)
            glow = max(0.0, 1 - since / 2.5)
            if glow > 0:
                px, py = cx + bx * r, cy + by * r
                d.ellipse([px - 4, py - 4, px + 4, py + 4], fill=lerp(INK, WHITE if not dark else VIOLET, glow))
        im.paste(self.small['SIGNAL'], (1146, 312), self.small['SIGNAL'])
        for i in range(6):
            level = 0.55 + 0.45 * math.sin(fr * 0.21 + i * 1.3)
            hgt = 10 + i * 9
            on = i < 3 + 3 * level
            d.rectangle([1148 + i * 18, 400 - hgt, 1160 + i * 18, 400], fill=lerp(accent, INK, 0.15 if on else 0.75))
        for i, y in enumerate(range(430, 640, 22)):
            v = (fr * 3 + i * 37) % 97
            d.text((1146, y), f'{i:02d} {v:02d}.{(v * 7) % 10}', font=font(MONO, 15), fill=lerp(accent, INK, 0.45))
        im.paste(self.small['CH 0.12.0'], (1146, 648), self.small['CH 0.12.0'])
        # the top margin: the feed's tab, and REC with the time
        if tab:
            t = tab_image(tab, accent)
            im.paste(t, (MX - 4, 2), t)
        if (fr // 15) % 2 == 0:
            d.ellipse([1004, 12, 1018, 26], fill=RED)
        im.paste(self.small['REC'], (1024, 10), self.small['REC'])
        secs = fr / FPS
        d.text((1062, 11), f'{int(secs // 60):02d}:{int(secs) % 60:02d}:{fr % FPS:02d}', font=font(MONO, 17),
               fill=lerp(accent, WHITE, 0.4))


_tabs = {}


def tab_image(text, color):
    key = (text, color)
    if key not in _tabs:
        t, _ = text_image(text, NARROW, 21, color, track=2)
        im = Image.new('RGBA', (t.width + 34, 30), INK + (230,))
        d = ImageDraw.Draw(im)
        d.rectangle([0, 0, 5, 29], fill=color + (255,))
        im.paste(t, (18, 3), t)
        _tabs[key] = im
    return _tabs[key]


# ---- the pictures ----

def game(scene, i):
    return kit.shot(scene, i)


def at4(pic):
    return pic.resize((MW, MH), Image.NEAREST)


class Shot:
    """What the monitor shows from `start`: a scene at 4x ('full'), BN6's
    and BN5's halves side by side ('split'), four at 2x ('mosaic') or a
    screenshot ('still'); its tab and nets, and whether it is dark."""

    def __init__(self, bar, beat, view, tab, nets, dark=False, cut='flip'):
        self.start = F(bar, beat)
        self.view, self.tab, self.nets, self.dark, self.cut = view, tab, nets, dark, cut

    def picture(self, fr):
        k = fr - self.start
        kind = self.view[0]
        if kind == 'full':
            _, scene, first = self.view
            return at4(game(scene, first + max(0, k)))
        if kind == 'still':
            return at4(kit.shot('still', self.view[1]))
        if kind == 'split':
            _, left, lf, right, rf = self.view
            out = at4(game(left, lf + max(0, k)))
            out.paste(at4(game(right, rf + max(0, k))).crop((MW // 2, 0, MW, MH)), (MW // 2, 0))
            d = ImageDraw.Draw(out)
            glow = 0.6 + 0.4 * math.sin(fr * 0.4)
            d.rectangle([MW // 2 - 3, 0, MW // 2 + 2, MH], fill=lerp(CYAN, WHITE, glow))
            return out
        if kind == 'mosaic':
            out = Image.new('RGB', (MW, MH), INK)
            for i, (scene, first) in enumerate(self.view[1]):
                pic = game(scene, first + max(0, k)).resize((480, 320), Image.NEAREST)
                out.paste(pic, ((i % 2) * 480, (i // 2) * 320))
            d = ImageDraw.Draw(out)
            d.line([(480, 0), (480, MH)], fill=INK, width=4)
            d.line([(0, 320), (MW, 320)], fill=INK, width=4)
            for i, label in enumerate(self.view[2]):
                t = tab_image(label, AMBER)
                out.paste(t, ((i % 2) * 480 + 10, (i // 2) * 320 + 10), t)
            return out
        raise ValueError(kind)


def flip_cut(old, new, k, speed):
    """The picture's panels flipping over from the old shot to the new, in
    a wave from the top left; None once done."""
    cols, rows, cell = 12, 8, 80
    if k >= (cols + rows) * speed + 4:
        return None
    out = Image.new('RGB', (MW, MH), INK)
    d = ImageDraw.Draw(out)
    for cy in range(rows):
        for cx in range(cols):
            p = (k - (cx + cy) * speed) / 4
            box = (cx * cell, cy * cell, cx * cell + cell, cy * cell + cell)
            if p <= 0:
                out.paste(old.crop(box), box[:2])
            elif p >= 1:
                out.paste(new.crop(box), box[:2])
            else:
                src = old if p < 0.5 else new
                h = max(1, round(cell * abs(1 - 2 * p)))
                piece = src.crop(box).resize((cell, h), Image.NEAREST)
                out.paste(piece, (box[0], box[1] + (cell - h) // 2))
                if 0.3 < p < 0.7:
                    d.line([(box[0] + 4, box[1] + cell // 2), (box[2] - 4, box[1] + cell // 2)], fill=CYAN, width=2)
    return out


def glitch(img, fr, strength):
    """Darkness over a picture: a violet grade, slices torn sideways, the
    colours split."""
    rnd = random.Random(fr * 7919 + 3)
    tint = ImageChops.multiply(img, Image.new('RGB', img.size, (172, 104, 255)))
    out = Image.blend(img, tint, 0.8)
    for _ in range(int(2 + 9 * strength)):
        y = rnd.randrange(0, MH - 8)
        h = rnd.randint(3, 36)
        dx = round(rnd.uniform(-46, 46) * strength)
        band = out.crop((0, y, MW, y + h))
        out.paste(band, (dx, y))
    s = round(2 + 7 * strength)
    r, g, b = out.split()
    return Image.merge('RGB', (ImageChops.offset(r, s, 0), g, ImageChops.offset(b, -s, 0)))


# ---- the timeline ----

BN5_AREA = 'BN5  ·  ACDC AREA'
SHOTS = [
    Shot(3, 0, ('split', 'bn6-battle', 150, 'viruses', 36), 'BN6  |  BN5  ·  ONE ENGINE', ('BN6', 'BN5')),
    Shot(5, 0, ('full', 'viruses', 50), BN5_AREA, ('BN5',)),
    Shot(6, 0, ('full', 'lava', 96), 'BN5  ·  LAVA', ('BN5',)),
    Shot(6, 2, ('full', 'sea', 60), 'BN5  ·  SEA', ('BN5',), cut='fast'),
    Shot(7, 0, ('full', 'toadman', 30), 'BN5  ·  SCILAB AREA  ·  GUARDIAN', ('BN5',)),
    Shot(7, 1, ('full', 'toadman', 330), 'BN5  ·  SCILAB AREA  ·  GUARDIAN', ('BN5',), cut='fast'),
    Shot(8, 0, ('full', 'knightman', 2), 'BN5  ·  ACDC AREA  ·  GUARDIAN', ('BN5',)),
    Shot(8, 2, ('full', 'knightman', 380), 'BN5  ·  ACDC AREA  ·  GUARDIAN', ('BN5',), cut='fast'),
    Shot(9, 0, ('full', 'unite', 44), 'CUSTOM  ·  UNITE', ('BN6', 'BN5')),
    Shot(9, 2, ('full', 'unite', 144), 'SHADOWSOUL', ('BN6', 'BN5'), cut='fast'),
    Shot(10, 0, ('full', 'chaos', 64), 'KNIGHTSOUL  ·  CHAOS UNISON', ('BN6', 'BN5')),
    Shot(11, 0, ('full', 'visitor', 0), 'BN6  ·  CENTRAL AREA  ·  A VISITOR', ('BN6', 'BN5')),
    Shot(12, 0, ('full', 'flame', 330), 'BN5  ·  FLAME OF DARKNESS', ('BN5',), dark=True, cut='glitch'),
    Shot(13, 0, ('full', 'darkinvs', 140), 'DARKCHIP  ·  DARKINVS', ('BN5',), dark=True, cut='glitch'),
    Shot(13, 2, ('full', 'darkplus', 100), 'DARKCHIP  ·  DARKPLUS', ('BN5',), dark=True, cut='glitch'),
    Shot(14, 0, ('full', 'darksoul', 20), 'BN5  ·  THE DARK SOUL', ('BN5',)),
    Shot(15, 0, ('mosaic', [('oran', 0), ('scilab', 0), ('undernet', 0), ('nebula', 0)],
                 ['ORAN AREA', 'SCILAB AREA', 'UNDERNET', 'NEBULA AREA']), 'BN5  ·  ITS NET', ('BN5',)),
    Shot(16, 0, ('still', 'saves-compare'), 'TITLE  ·  SAVES', ('BN6', 'BN5')),
    Shot(16, 2, ('still', 'saves-result'), 'TITLE  ·  SAVES', ('BN6', 'BN5'), cut='fast'),
    Shot(17, 0, ('full', 'chaos', 110), 'KNIGHTSOUL', ('BN6', 'BN5'), cut='hard'),
    Shot(17, 1, ('full', 'toadman', 392), 'TOADMAN', ('BN5',), cut='hard'),
    Shot(17, 2, ('full', 'unite', 300), 'SHADOWSOUL', ('BN6', 'BN5'), cut='hard'),
    Shot(17, 3, ('full', 'bn6-battle', 190), 'BN6  ·  CENTRAL AREA', ('BN6',), cut='hard'),
]
MONITOR = (F(3), F(18))

WORDS = [
    Words(3, 0, 4, "BN5'S BATTLES, IN BN6'S", 'ONE ENGINE', '', 'top'),
    Words(5, 0, 4, BN5_AREA, 'ITS VIRUSES', "BN6'S HUD, CUSTOM SCREEN, CROSSES AND BEAST OUT"),
    Words(6, 0, 4, 'BN5  ·  ITS FIELDS', 'LAVA AND SEA', "AS PANELS OF BN6'S OWN ENGINE"),
    Words(7, 0, 8, 'BN5  ·  SIX NAVIS', 'ITS GUARDIANS', 'TOADMAN  ·  KNIGHTMAN  ·  SHADOWMAN  ·  NUMBERMAN  ·  TOMAHAWKMAN  ·  COLONEL'),
    Words(9, 0, 4, 'DELETE ONE, KEEP HIS SOUL', 'UNITE!', "ON BN6'S CUSTOM SCREEN, FOR THREE TURNS", 'top'),
    Words(10, 0, 4, "BY BN5'S OWN RULE", 'CHAOS UNISON', 'A SOUL AND ITS DARKCHIP AT ONCE'),
    Words(11, 0, 4, 'FROM THE SECOND ACT', 'THE NETS MEET', "ONE BATTLE IN FOUR BRINGS THE OTHER NET'S VIRUS"),
    Words(12, 0, 4, "BN5'S FLAMES OF DARKNESS", 'DARKCHIPS', 'THEY COME WHILE MEGAMAN IS WORRIED', 'top', dark=True),
    Words(13, 0, 4, 'DARKINVS  ·  DARKPLUS', 'AT A PRICE', '20 MAX HP FOR EACH BATTLE ONE IS USED IN', dark=True),
    Words(14, 0, 4, 'MISS A CHAOS UNISON', 'THE DARK SOUL', 'AND HE STANDS ON THE OTHER SIDE'),
    Words(15, 0, 4, "BN5'S NET", 'SIX AREAS', 'ACDC  ·  ORAN  ·  SCILAB  ·  END AREA  ·  UNDERNET  ·  NEBULA'),
    Words(16, 0, 4, 'L ON THE TITLE', 'SAVES THAT TRAVEL', 'ONE FILE  ·  PC, PHONE, HANDHELD, NEW 3DS, BROWSER'),
]

# the intro: four feeds of BN5's net (bar 1), then BN6's beside BN5's (bar 2)
FEED_AT = [(150, 28), (650, 28), (150, 372), (650, 372)]
FEEDS_BN5 = [('oran', 0, 'BN5  ·  ORAN AREA'), ('scilab', 0, 'BN5  ·  SCILAB AREA'),
             ('undernet', 0, 'BN5  ·  UNDERNET'), ('nebula', 0, 'BN5  ·  NEBULA AREA')]
FEEDS_TWO = [('bn6-map', 0, 'BN6  ·  CENTRAL AREA'), ('oran', 50, 'BN5  ·  ORAN AREA'),
             ('bn6-battle', 150, 'BN6  ·  A BATTLE'), ('viruses', 6, 'BN5  ·  A BATTLE')]


def feed(im, scene, first, label, at, k, fr):
    """A feed at 2x popping in: its height opening, static fading."""
    if k < 0:
        return
    x, y = at
    pic = game(scene, first + k).resize((480, 320), Image.NEAREST)
    color = CYAN if label.startswith('BN6') else AMBER
    t = ease(k / 5)
    h = max(2, round(320 * t))
    if k < 6:
        noise = Image.effect_noise((120, 80), 90).resize((480, 320), Image.NEAREST).convert('RGB')
        pic = Image.blend(pic, noise, 0.8 * (1 - k / 6))
    pic = pic.crop((0, (320 - h) // 2, 480, (320 - h) // 2 + h))
    im.paste(pic, (x, y + (320 - h) // 2))
    d = ImageDraw.Draw(im)
    d.rectangle([x - 3, y + (320 - h) // 2 - 3, x + 482, y + (320 - h) // 2 + h + 2], outline=color, width=2)
    if k >= 4:
        tab = tab_image(label, color)
        im.paste(tab, (x + 8, y + 8), tab)


class Intro:
    def __init__(self):
        self.lines = [Typed('> INCOMING TRANSMISSION', MONO, 36, CYAN, rate=1.2, cursor=CYAN),
                      Typed('> BATTLE NETWORK 5  ·  TEAM COLONEL', MONO, 36, AMBER, rate=1.4, cursor=AMBER)]
        self.two = Typed('TWO NETS', HEAVY, 92, WHITE, track=4, rate=1.0, cursor=CYAN)

    def draw(self, im, fr):
        if fr < F(1):
            self.lines[0].draw(im, 170, 290, fr - 4)
            self.lines[1].draw(im, 170, 350, fr - F(0, 2))
            return
        feeds = FEEDS_BN5 if fr < F(2) else FEEDS_TWO
        for i, (scene, first, label) in enumerate(feeds):
            if fr < F(2):
                k = fr - F(1, i)
            elif scene == 'oran':
                k = fr - F(1, 0)
            else:
                k = fr - F(2, 0 if i % 2 == 0 else 0.5)
            feed(im, scene, first, label, FEED_AT[i], k, fr)
        if fr >= F(2):
            k = fr - F(2)
            plate(im, (0, 300, W * ease(k / 6), 420), INK, 225)
            ImageDraw.Draw(im).line([(0, 300), (W * ease(k / 6), 300)], fill=CYAN, width=2)
            self.two.draw(im, (W - self.two.im.width) // 2, 306, k - 4)


class Title:
    """Bar 4: the name, scanned in from the top over the split."""

    def __init__(self):
        self.kicker, _ = text_image('VERSION 0.12.0', NARROW, 30, AMBER, track=6)
        self.name, _ = text_image('ONE BATTLE NETWORK', HEAVY, 82, WHITE, track=3)
        self.caption, _ = text_image("BN5 TEAM COLONEL'S NET, IN BN6'S OWN ENGINE", NARROW, 30, lerp(CYAN, WHITE, 0.5), track=2)

    def draw(self, im, fr):
        k = fr - F(4)
        if not 0 <= k < BAR:
            return
        plate(im, (MX, MY, MX + MW, MY + MH), INK, 150)
        top, bottom = MY + 170, MY + 420
        scan = top + (bottom - top) * ease(k / 9)
        for img, y in ((self.kicker, MY + 182), (self.name, MY + 226), (self.caption, MY + 340)):
            x = (W - img.width) // 2
            visible = max(0, min(img.height, round(scan - y)))
            if visible:
                part = img.crop((0, 0, img.width, visible))
                im.paste(part, (x, y), part)
        if k < 10:
            ImageDraw.Draw(im).line([(MX, scan), (MX + MW, scan)], fill=lerp(CYAN, WHITE, 0.6), width=3)


class Ending:
    """Bar 18, the logo over the freed net; bar 19, the end card, a line on
    each of the last three chords."""

    def __init__(self):
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        self.logo = lg.resize((lg.width * 4 // 5, lg.height * 4 // 5), Image.LANCZOS)
        self.small_logo = lg.resize((lg.width * 9 // 20, lg.height * 9 // 20), Image.LANCZOS)
        self.sub = Typed('VERSION 0.12.0   ·   ONE BATTLE NETWORK', NARROW, 36, AMBER, track=5, rate=2.5)
        self.free, _ = text_image('FREE  ·  BRING YOUR OWN ROM', HEAVY, 54, WHITE, track=2)
        self.plats, _ = text_image('WINDOWS  ·  MAC  ·  LINUX  ·  ANDROID  ·  IPHONE  ·  NEW 3DS  ·  PORTMASTER  ·  BROWSER',
                                   NARROW, 27, CYAN, track=2)
        self.url, _ = text_image('saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless', MONO, 22,
                                 lerp(CYAN, WHITE, 0.6))
        self.notice = [text_image(line, NARROW, 19, (150, 170, 200), track=1)[0] for line in (
            'Unofficial fan project, not affiliated with Capcom. Runs from your own Mega Man Battle Network 6: '
            'Cybeast Gregar (USA) ROM.',
            'Mega Man Battle Network 5: Team Colonel (USA) beside it brings its net.')]

    def draw(self, im, fr):
        k = fr - F(18)
        d = ImageDraw.Draw(im)
        if fr < F(19):
            plate(im, (0, 150, W, 500), INK, round(170 * ease(k / 8)))
            lg = self.logo
            x, y = (W - lg.width) // 2, 180
            scan = y + (lg.height + 10) * ease(k / 10)
            part = lg.crop((0, 0, lg.width, max(1, min(lg.height, round(scan - y)))))
            im.paste(part, (x, y), part)
            if k < 12:
                d.line([(x - 40, scan), (x + lg.width + 40, scan)], fill=lerp(CYAN, WHITE, 0.6), width=3)
            d.line([(W / 2 - 330 * ease((k - 8) / 8), 448), (W / 2 + 330 * ease((k - 8) / 8), 448)], fill=CYAN, width=2)
            self.sub.draw(im, (W - self.sub.im.width) // 2, 462, k - 12)
            return
        k = fr - F(19)
        plate(im, (0, 40, W, 690), INK, 200)
        lg = self.small_logo
        im.paste(lg, ((W - lg.width) // 2, 70), lg)
        for img, y, at in ((self.free, 250, F(19, 0)), (self.plats, 352, F(19, 1.5)), (self.url, 410, F(19, 3))):
            j = fr - at
            if j >= 0:
                t = ease(j / 5)
                part = img.crop((0, 0, max(1, round(img.width * t)), img.height))
                im.paste(part, ((W - img.width) // 2, y), part)
        d.line([(W / 2 - 300, 330), (W / 2 + 300, 330)], fill=lerp(CYAN, INK, 0.4), width=1)
        for i, line in enumerate(self.notice):
            im.paste(line, ((W - line.width) // 2, 626 + 26 * i), line)


def vignette():
    v = Image.new('L', (W // 8, H // 8))
    px = v.load()
    for y in range(H // 8):
        for x in range(W // 8):
            r = math.hypot((x - W / 16) / (W / 16), (y - H / 16) / (H / 16))
            px[x, y] = int(max(0, min(190, (r - 0.7) * 280)))
    return v.resize((W, H), Image.BICUBIC)


def beat_age(fr):
    i = int(fr / BEAT)
    while F(0, i + 1) <= fr:
        i += 1
    while F(0, i) > fr:
        i -= 1
    return fr - F(0, i)


# flashes: (frame, colour, frames)
FLASHES = [(F(4), WHITE, 4), (F(12), VIOLET, 5), (F(14), WHITE, 4), (F(18), WHITE, 5), (F(19), WHITE, 2),
           (F(19, 1.5), WHITE, 2), (F(19, 3), WHITE, 3)]


def monitor(im, fr, hud, freed):
    i = max(j for j, s in enumerate(SHOTS) if s.start <= fr)
    s = SHOTS[i]
    k = fr - s.start
    pic = s.picture(fr)
    if s.dark:
        pic = glitch(pic, fr, max(0.22, 1 - k / 8) + (0.6 if fr % 17 == 0 else 0))
    if i > 0 and s.cut in ('flip', 'fast'):
        prev = SHOTS[i - 1]
        old = prev.picture(fr)
        if prev.dark:
            old = glitch(old, fr, 0.3)
        cut = flip_cut(old, pic, k, 0.35 if s.cut == 'flip' else 0.12)
        if cut is not None:
            pic = cut
    elif s.cut == 'hard' and k < 4:
        # (a punch in, settling)
        z = 1 + 0.06 * (4 - k) / 4
        cw, ch = round(MW / z), round(MH / z)
        pic = pic.crop(((MW - cw) // 2, (MH - ch) // 2, (MW - cw) // 2 + cw, (MH - ch) // 2 + ch)).resize((MW, MH), Image.NEAREST)
        if k < 2:
            pic = Image.blend(pic, Image.new('RGB', (MW, MH), WHITE), 0.5 - 0.25 * k)
    pic.paste(INK, (0, 0), hud.lines)
    k3 = fr - F(3)
    if k3 < 8:
        h = max(2, round(MH * ease(k3 / 8)))
        pic = pic.crop((0, (MH - h) // 2, MW, (MH - h) // 2 + h))
        im.paste(pic, (MX, MY + (MH - h) // 2))
    else:
        im.paste(pic, (MX, MY))
    return s


def compose(only=None):
    frames_dir = os.path.join(kit.WORK, 'frames')
    if only is None:
        shutil.rmtree(frames_dir, ignore_errors=True)
    os.makedirs(frames_dir, exist_ok=True)
    field, hud, intro, title, ending = Field(), Hud(), Intro(), Title(), Ending()
    vig = vignette()
    for fr in range(FRAMES):
        bar = fr // BAR
        freed = freed_at(fr)
        field.update(fr, freed)
        if only is not None and fr not in only:
            continue
        im = Image.new('RGB', (W, H), INK)
        pulse = (4 <= bar < 12 or bar >= 14) and beat_age(fr) < 3
        field.draw(im, fr, pulse, 1.0 if fr >= F(1) else fr / F(1))
        if fr < F(3):
            intro.draw(im, fr)
        elif fr < F(18):
            s = monitor(im, fr, hud, freed)
            title.draw(im, fr)
            for w in WORDS:
                w.draw(im, fr)
            hud.draw(im, fr, freed, s.nets, s.dark, s.tab)
        else:
            ending.draw(im, fr)
        for at, color, n in FLASHES:
            if 0 <= fr - at < n:
                im = Image.blend(im, Image.new('RGB', (W, H), color), 0.7 * (1 - (fr - at) / n))
        im.paste(INK, (0, 0), vig)
        end = FRAMES - 22
        if fr > end:
            im = Image.blend(im, Image.new('RGB', (W, H), (0, 0, 0)), min(1.0, (fr - end) / 20))
        im.save(os.path.join(frames_dir, f'{fr:05d}.png'), compress_level=1)
        if fr % 100 == 0:
            print('frame', fr, '/', FRAMES)
