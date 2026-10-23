"""0.15.0's trailer, LIBERATION: BN5's Liberation Missions in the endless
net. The release's news is an operation run phase by phase, so the cut is
a war room's map: a night-blue grid with its sectors marked, each shot a
monitor in a sector of its own, and every cut the camera panning to the
next, easing in as BN5's board camera does (a share of what is left each
frame). A readout counts the phases against the par, as the board's L view
does; for the drop it turns crimson, DARKLOID PHASE, the map with it.
Headlines in URW Gothic settle from wide tracking onto the beat, their
captions in a mono face, under the monitor where they hide nothing. The
devices the earlier trailers used (chrome type, typed briefing words,
panels freed, linked windows, a slanted seam) are not here.

Its shots are whole frames of the game running from the developer's ROMs,
BN6's and BN5 Team Colonel's beside it, at 3x (four boards at 2x): the
autopilot through seed 1's act 2 board and on to act 3's, other seeds' and
acts' boards, and home before act 2. Everything else is drawn here; its
music is v0_15_0_music.py's, an original piece in the manner of BN5's
board theme.

    python3 build.py host && python3 tools/trailer.py 0.15.0 [--keep]
"""
import os
import shutil
import sys

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer as kit  # noqa: E402
from trailer import ROOT  # noqa: E402
from trailers import v0_15_0_music as music  # noqa: E402

VERSION = '0.15.0'
PLAY_LABEL = 'WATCH THE 0.15.0 TRAILER'
BN6_ONLY = ()


# ---- the footage ----

AUTO = {'CYBERWORLD_AUTOPILOT': '1'}


def board(seed, depth, first, last):
    """the autopilot on a dark layer's board in BN5's look (both ROMs)"""
    return (['--scene', 'emu', '--run-depth', str(depth), '--seed', str(seed), '--dev', 'dark,god,onehit'],
            AUTO, None, first, last)


# seed 7's fork before act 2, its pink pad the dark way: the welcome's
# "Two ways open this time!", MegaMan away from the pad (its swirl shows
# only then), then at it, A: his reading of the way, "Part of it's gone dark."
FORK = ('40:,' + '4:A,12:,' * 4 + '160:,' + '4:A,12:,' * 6 + '0:place 120 -15 1,180:,0:place 165 -15 1,30:,6:A,'
        + '90:,4:A,60:,4:A,60:,4:A,60:,4:A,60:,4:A,200:')
FIRST = {}      # each scene's first game frame, for the samples below


def scenes():
    s = {
        'lib-fork': (['--scene', 'home', '--run-depth', '4', '--seed', '7', '--dev', 'quiet'], {}, FORK, 0, 1300),
        # seed 1's act 2 board, the autopilot through it (#214's Darkloids' phase)
        'lib-open': board(1, 5, 0, 720),
        'lib-first': board(1, 5, 1100, 2300),
        'lib-phase': board(1, 5, 2050, 2800),
        'lib-darkturn': board(1, 5, 13300, 14200),
        'lib-final': board(1, 5, 15600, 16800),
        'lib-boss': board(1, 5, 22600, 24800),
        # ... and on to act 3's
        'lib-act3': board(1, 5, 29500, 30400),
        # other boards: other seeds and acts
        'lib-b2': board(2, 5, 600, 1500),
        'lib-b3': board(3, 5, 600, 1500),
        'lib-b4': board(1, 8, 600, 1500),
        'lib-b5': board(1, 11, 600, 1500),
    }
    FIRST.update({k: v[3] for k, v in s.items()})
    return s


def at(scene, game):
    """the footage's sample showing game frame `game` of `scene` (every
    second frame kept)"""
    if not FIRST:
        scenes()
    return max(0, (game - FIRST[scene]) // 2)


# ---- the look ----

FPS, W, H = 30, 1280, 720
BAR = round(FPS * 60 * 4 / music.BPM)     # (50 frames at 144 beats a minute)
BEAT = BAR / 4
FRAMES = music.BARS * BAR + round(music.TAIL * FPS)

GOTHIC = '/usr/share/fonts/opentype/urw-base35/URWGothic-Demi.otf'
MONO = '/usr/share/fonts/truetype/ubuntu/UbuntuSansMono[wght].ttf'

NIGHT = (6, 10, 24)
GRID = (13, 27, 52)
MAJOR = (22, 46, 84)
LABEL = (54, 90, 138)
CYAN = (83, 215, 255)
AMBER = (255, 184, 64)
CRIMSON = (255, 62, 94)
WHITE = (246, 248, 255)
DIM = (140, 162, 196)
RED_NIGHT = (24, 6, 16)
RED_GRID = (52, 14, 30)
RED_MAJOR = (88, 22, 44)

CELL = 48               # the map's grid
MAJOR_EVERY = 5         # a sector's lines every 240 px
SECTOR = (1040, 720)    # a monitor's sector on the map


def F(bar, beat=0.0):
    return round(bar * BAR + beat * BEAT)


_fonts = {}


def font(path, size, wght=None):
    key = (path, size, wght)
    if key not in _fonts:
        f = ImageFont.truetype(path, size)
        if wght:
            f.set_variation_by_axes([wght])
        _fonts[key] = f
    return _fonts[key]


def mono(size, wght=500):
    return font(MONO, size, wght)


def ease(t):
    t = max(0.0, min(1.0, t))
    return 1 - (1 - t) ** 3


def lerp(a, b, t):
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def sector(col, row):
    return (col * SECTOR[0] + SECTOR[0] // 2, row * SECTOR[1] + SECTOR[1] // 2)


# ---- the map ----

def draw_map(im, cam, red):
    """The war room's map under the camera at `cam` (the world point at the
    frame's middle): a fine grid, its sectors' lines, a mark and a name at
    each sector's corner; `red` (0-1) the Darkloids' phase's tint."""
    d = ImageDraw.Draw(im)
    d.rectangle([0, 0, W, H], fill=lerp(NIGHT, RED_NIGHT, red))
    grid, major, label = lerp(GRID, RED_GRID, red), lerp(MAJOR, RED_MAJOR, red), lerp(LABEL, CRIMSON, red * 0.6)
    ox, oy = cam[0] - W / 2, cam[1] - H / 2
    x0 = int(ox // CELL) * CELL
    y0 = int(oy // CELL) * CELL
    for wx in range(x0, int(ox + W) + CELL, CELL):
        sx = round(wx - ox)
        big = (wx // CELL) % MAJOR_EVERY == 0
        d.line([(sx, 0), (sx, H)], fill=major if big else grid, width=2 if big else 1)
    for wy in range(y0, int(oy + H) + CELL, CELL):
        sy = round(wy - oy)
        big = (wy // CELL) % MAJOR_EVERY == 0
        d.line([(0, sy), (W, sy)], fill=major if big else grid, width=2 if big else 1)
    step = CELL * MAJOR_EVERY
    f = mono(13, 500)
    for wx in range(int(ox // step) * step, int(ox + W) + step, step):
        for wy in range(int(oy // step) * step, int(oy + H) + step, step):
            sx, sy = round(wx - ox), round(wy - oy)
            d.line([(sx - 6, sy), (sx + 6, sy)], fill=label, width=2)
            d.line([(sx, sy - 6), (sx, sy + 6)], fill=label, width=2)
            name = chr(65 + (wx // step) % 26) + str(wy // step + 1)
            d.text((sx + 6, sy + 5), name, font=f, fill=label)


# ---- a monitor: a game's picture in its sector ----

class Pic:
    """A game's picture at `scale`, its middle `off` from its shot's place
    on the map. `feed` is [(frame, scene, sample, rate)]: from each frame
    on, that scene's footage from that sample (rate 0 holds it); a feed's
    cut inside a shot is a scan line drawn down the picture."""

    def __init__(self, feed, scale=3, off=(0, 0), tag=None):
        self.feed, self.s, self.off, self.tag = feed, scale, off, tag
        self.pw, self.ph = 240 * scale, 160 * scale

    def source(self, fr):
        i = max([j for j, e in enumerate(self.feed) if e[0] <= fr] or [0])
        start, scene, sample, rate = self.feed[i]
        return i, scene, sample + max(0, int((fr - start) * rate))

    def picture(self, fr):
        i, scene, sample = self.source(fr)
        pic = kit.shot(scene, sample).resize((self.pw, self.ph), Image.NEAREST)
        since = fr - self.feed[i][0]
        if i > 0 and since < 5:
            old = self.picture(self.feed[i][0] - 1)
            edge = round(self.ph * ease((since + 1) / 5))
            old.paste(pic.crop((0, 0, self.pw, edge)), (0, 0))
            d = ImageDraw.Draw(old)
            d.rectangle([0, max(0, edge - 3), self.pw, edge + 2], fill=WHITE)
            pic = old
        return pic

    def draw(self, im, fr, mid, accent):
        x0 = round(mid[0] + self.off[0] - self.pw / 2)
        y0 = round(mid[1] + self.off[1] - self.ph / 2)
        if x0 > W or y0 > H or x0 + self.pw < 0 or y0 + self.ph < 0:
            return
        x1, y1 = x0 + self.pw, y0 + self.ph
        d = ImageDraw.Draw(im)
        d.rectangle([x0 + 10, y0 + 10, x1 + 10, y1 + 10], fill=(0, 0, 0))
        d.rectangle([x0 - 3, y0 - 3, x1 + 2, y1 + 2], fill=lerp(CYAN, accent, 0.5) if accent != AMBER else CYAN)
        im.paste(self.picture(fr), (x0, y0))
        b, t = 26, 4
        for cx, cy, sx, sy in ((x0 - 10, y0 - 10, 1, 1), (x1 + 9, y0 - 10, -1, 1), (x0 - 10, y1 + 9, 1, -1),
                               (x1 + 9, y1 + 9, -1, -1)):
            d.rectangle([min(cx, cx + sx * b), min(cy, cy + sy * t), max(cx, cx + sx * b), max(cy, cy + sy * t)], fill=accent)
            d.rectangle([min(cx, cx + sx * t), min(cy, cy + sy * b), max(cx, cx + sx * t), max(cy, cy + sy * b)], fill=accent)
        if self.tag:
            f = mono(17 if self.s >= 3 else 15, 600)
            d.text((x0 + 2, y0 - 18), self.tag, font=f, fill=accent, anchor='ls')


# ---- words ----

class Head:
    """A headline under the monitor, its letters settling from wide tracking
    onto the beat; its caption, in the mono face, under it."""

    def __init__(self, start, end, text, caption=None, y=640, size=52, color=WHITE, cap_color=DIM, x=W / 2,
                 width=W - 120):
        self.start, self.end, self.text, self.caption, self.y = start, end, text, caption, y
        self.size, self.color, self.cap_color, self.x, self.width = size, color, cap_color, x, width

    def draw(self, im, fr, bg):
        k = fr - self.start
        if k < 0 or fr >= self.end:
            return
        out = fr - (self.end - 4)
        a = ease(k / 7) * (1 - max(0.0, out / 4))
        d = ImageDraw.Draw(im)
        size = self.size
        f = font(GOTHIC, size)
        spread = 22 * (1 - ease(k / 8))
        while size > 24 and sum(f.getlength(c) for c in self.text) + spread * (len(self.text) - 1) > self.width:
            size -= 2
            f = font(GOTHIC, size)
        total = sum(f.getlength(c) for c in self.text) + spread * (len(self.text) - 1)
        x = self.x - total / 2
        for c in self.text:
            d.text((x, self.y), c, font=f, fill=lerp(bg, self.color, a), anchor='lm')
            x += f.getlength(c) + spread
        if self.caption:
            ca = ease((k - 4) / 6) * (1 - max(0.0, out / 4))
            cs = 21
            cf = mono(cs, 500)
            while cs > 14 and cf.getlength(self.caption) > self.width:
                cs -= 1
                cf = mono(cs, 500)
            d.text((self.x, self.y + size * 0.5 + 22), self.caption, font=cf, fill=lerp(bg, self.cap_color, ca), anchor='mm')


class Stack:
    """Words stacked in the mosaic's middle column, a line a beat."""

    def __init__(self, start, end, lines, caption, x=W / 2, y=250):
        self.start, self.end, self.lines, self.caption, self.x, self.y = start, end, lines, caption, x, y

    def draw(self, im, fr, bg):
        if not self.start <= fr < self.end:
            return
        d = ImageDraw.Draw(im)
        out = max(0.0, (fr - (self.end - 4)) / 4)
        for i, line in enumerate(self.lines):
            k = fr - self.start - i * BEAT / 2
            if k < 0:
                continue
            a = ease(k / 6) * (1 - out)
            f = font(GOTHIC, 40)
            spread = 14 * (1 - ease(k / 8))
            total = sum(f.getlength(c) for c in line) + spread * (len(line) - 1)
            x = self.x - total / 2
            for c in line:
                d.text((x, self.y + 50 * i), c, font=f, fill=lerp(bg, WHITE, a), anchor='lm')
                x += f.getlength(c) + spread
        k = fr - self.start - len(self.lines) * BEAT / 2
        if k > 0:
            a = ease(k / 6) * (1 - out)
            f = mono(16, 500)
            for j, part in enumerate(self.caption):
                d.text((self.x, self.y + 50 * len(self.lines) + 14 + 22 * j), part, font=f, fill=lerp(bg, DIM, a),
                       anchor='mm')


# ---- the readout: the phase against the par ----

# (the act 2 board's own phases where its banners show them: PLAYER PHASE 01,
# 08's final turn and time up, the Darkloids' phase before 12, cleared in 12
# against a par of 12: "Right on par!")
PHASES = [(0, 'FORK'), (F(3), None), (F(4), 1), (F(10), 8), (F(12), 'D'), (F(14), 12), (F(17), 'END')]
PAR = 12


def phase_at(fr):
    i = max(j for j, (s, _) in enumerate(PHASES) if s <= fr)
    return i, PHASES[i]


def readout(im, fr, red, where=''):
    """Top left the phase and the par, its digits rolling up on a change;
    top right the mission's name and the beat's four marks."""
    d = ImageDraw.Draw(im)
    i, (start, ph) = phase_at(fr)
    accent = lerp(AMBER, CRIMSON, red)
    label = {'D': 'DARKLOID PHASE', None: 'BRIEFING', 'END': 'MISSION CLEAR', 'FORK': 'AT THE FORK'}.get(ph, 'PHASE')
    d.text((44, 30), label, font=mono(16, 600), fill=lerp(DIM, CRIMSON, red), anchor='ls')
    shown = [p for _, p in PHASES[:i + 1] if isinstance(p, int)]
    num = shown[-1] if shown else 0
    prev = [p for _, p in PHASES[:i] if isinstance(p, int)]
    k = fr - start
    digits = mono(46, 700)
    box = Image.new('RGB', (70, 50), lerp(NIGHT, RED_NIGHT, red))
    bd = ImageDraw.Draw(box)
    roll = ease(k / 5) if prev and isinstance(ph, int) and prev[-1] != num else 1.0
    if roll < 1:
        bd.text((0, 25 - 50 * roll), f'{prev[-1]:02d}', font=digits, fill=accent, anchor='lm')
    bd.text((0, 25 + 50 * (1 - roll)), f'{num:02d}' if num else '--', font=digits, fill=accent, anchor='lm')
    im.paste(box, (42, 36))
    d.text((112, 62), f'/ PAR {PAR}', font=mono(22, 500), fill=DIM, anchor='lm')
    d.text((W - 44, 30), where or 'LIBERATION MISSION', font=mono(16, 600), fill=lerp(DIM, CRIMSON, red), anchor='rs')
    beat = int((fr % BAR) // BEAT)
    for j in range(4):
        cx, cy = W - 44 - 26 * (3 - j), 58
        lit = j == beat
        d.polygon([(cx, cy - 9), (cx + 9, cy), (cx, cy + 9), (cx - 9, cy)],
                  fill=accent if lit else lerp(NIGHT, MAJOR, 1.0), outline=accent)


# ---- the shots ----

class Shot:
    """From `start` to the next shot: the monitors and words at `place` on
    the map; the camera pans to it from the last shot's place, a share
    `pull` of what is left each frame (BN5's board camera eases so)."""

    def __init__(self, start, place, pics=(), words=(), pull=0.42, center=None, where=''):
        self.start, self.place, self.pics, self.words, self.pull = start, place, pics, words, pull
        self.center, self.where = center, where

    def draw(self, im, fr, cam, red):
        mid = (self.place[0] - cam[0] + W / 2, self.place[1] - cam[1] + H / 2)
        accent = lerp(AMBER, CRIMSON, red)
        for p in self.pics:
            p.draw(im, fr, mid, accent)

    def draw_words(self, im, fr, red):
        bg = lerp(NIGHT, RED_NIGHT, red)
        for w in self.words:
            w.draw(im, fr, bg)


def hud_hidden(fr):
    """the readout out while the camera pulls back to the four boards (bars
    8-9), faded over four frames each way"""
    if F(8) <= fr < F(10):
        return min(1.0, (fr - F(8) + 1) / 4)
    if F(10) <= fr < F(10) + 4:
        return 1 - (fr - F(10) + 1) / 4
    return 0.0


def red_at(fr):
    """the Darkloids' phase's tint: in over four frames at bar 12, out at 14"""
    if fr < F(12):
        return 0.0
    if fr < F(14):
        return ease((fr - F(12)) / 4)
    return max(0.0, 1 - (fr - F(14)) / 4)


def shots():
    S = lambda scene, n: n           # (a scene's footage sample, picked on its contact sheet)
    return [
        # home after act 1: the fork, its pink pad gone dark, MegaMan's reading of it
        Shot(0, sector(0, 0), [Pic([(0, 'lib-fork', S('lib-fork', 120), 1),
                                    (F(2), 'lib-fork', S('lib-fork', 372), 1)], tag="LAN'S HP · THE FORK")],
             [Head(F(0, 1), F(1), 'AFTER EVERY ACT, A FORK', 'Two ways on into the endless net'),
              Head(F(1, 0.5), F(2), 'ONE WAY HAS GONE DARK', "From the second act, behind one of the fork's two ways"),
              Head(F(2, 0.5), F(3), 'TAKE IT?', "It costs that layer's Net Dealer and its Mr.Prog")], where='ACT 1 · HOME'),
        # the board's opening: the camera on its way to the Darkloid
        Shot(F(3), sector(1, 0), [Pic([(F(3), 'lib-open', S('lib-open', 18), 1)], tag='LAYER 5 · GONE DARK')],
             [Head(F(3, 0.5), F(4), 'LIBERATION', 'CYBERWORLD ENDLESS 0.15.0', y=646, size=66, cap_color=AMBER)],
             where='ACT 2 · LAYER 5'),
        # PLAYER PHASE 01
        Shot(F(4), sector(1, 1), [Pic([(F(4), 'lib-open', S('lib-open', 158), 1)], tag='LAYER 5 · PLAYER PHASE')],
             [Head(F(4, 0.5), F(5), 'THE LAYER BECOMES A BOARD', "BN5's Liberation Mission, made from the layer's own floor")],
             where='ACT 2 · LAYER 5'),
        # the first panel: the walk, its battle, the win in one turn
        Shot(F(5), sector(2, 1), [Pic([(F(5), 'lib-first', S('lib-first', 20), 1),
                                       (F(6), 'lib-first', S('lib-first', 253), 1),
                                       (F(7), 'lib-first', S('lib-first', 338), 1)], tag='LAYER 5 · PANEL 25,30')],
             [Head(F(5, 0.5), F(6), 'ONE PANEL AT A TIME', 'MegaMan walks the board as BN6 walks him'),
              Head(F(6, 0.25), F(7), 'A BATTLE FOR EVERY PANEL', "BN5's own turns, in BN6's battles"),
              Head(F(7, 0.25), F(8), 'WIN IN ONE TURN', 'and the dark around the panel falls too')], where='ACT 2 · LAYER 5'),
        # other boards
        Shot(F(8), sector(3, 1), [Pic([(F(8), 'lib-b2', 0, 1)], 2, (-388, -160), 'ACT 2'),
                                  Pic([(F(8), 'lib-b3', 0, 1)], 2, (388, -160), 'ACT 2'),
                                  Pic([(F(8), 'lib-b4', 0, 1)], 2, (-388, 184), 'ACT 3'),
                                  Pic([(F(8), 'lib-b5', 0, 1)], 2, (388, 184), 'ACT 4')],
             [Stack(F(8, 0.5), F(10), ['NEVER', 'THE SAME', 'BOARD', 'TWICE'],
                    ["each run's seed", 'makes its own, its', 'par set by a planner', 'that plays it'], y=190)],
             where='ACTS 2-4'),
        # three turns, or it stays dark
        Shot(F(10), sector(3, 2), [Pic([(F(10), 'lib-final', S('lib-final', 85), 1),
                                        (F(11), 'lib-final', S('lib-final', 372), 1)], tag='LAYER 5 · PHASE 08')],
             [Head(F(10, 0.25), F(11), 'THREE TURNS TO WIN IT', 'FINAL TURN! is the last'),
              Head(F(11, 0.25), F(12), 'OR IT STAYS DARK', 'Time up leaves the panel dark')], where='ACT 2 · LAYER 5'),
        # the drop: the Darkloids' phase
        Shot(F(12), sector(2, 2), [Pic([(F(12), 'lib-darkturn', S('lib-darkturn', 130), 1),
                                        (F(12, 2.5), 'lib-darkturn', S('lib-darkturn', 176), 1),
                                        (F(13), 'lib-darkturn', S('lib-darkturn', 226), 1),
                                        # (his line read whole: the autopilot pages it at once, so it is held)
                                        (F(13) + 4, 'lib-darkturn', S('lib-darkturn', 230), 0),
                                        (F(13, 2), 'lib-darkturn', S('lib-darkturn', 262), 1)], tag='LAYER 5 · DARKLOID PHASE')],
             [Head(F(12, 0.5), F(14), 'THEN THE DARK STRIKES BACK', 'The Darkloid and every guardian take their turn')],
             pull=0.3, where='ACT 2 · LAYER 5'),
        # the climax: PLAYER PHASE 08, the Darkloid, the par
        Shot(F(14), sector(2, 3), [Pic([(F(14), 'lib-boss', S('lib-boss', 163), 1),
                                        (F(15), 'lib-boss', S('lib-boss', 345), 1),
                                        (F(16), 'lib-boss', S('lib-boss', 400), 1),
                                        (F(16, 2), 'lib-boss', S('lib-boss', 478), 1),
                                        (F(17), 'lib-boss', S('lib-boss', 640), 1)], tag='LAYER 5 · THE DARKLOID')],
             [Head(F(14, 0.25), F(15), 'EACH ACT DARKER', 'Barriers and keys from act 3, a TinHawk, then traps'),
              Head(F(15, 0.25), F(17), 'BREAK THROUGH TO THE DARKLOID', "A Navi of BN6's, waiting on his ring"),
              Head(F(17, 0.25), F(18), 'BEAT THE PAR, ARM THE RUN', 'The fewer the phases, the better the chip you carry on')],
             pull=0.55, where='ACT 2 · LAYER 5'),
        Shot(F(18), sector(3, 3), [], [], pull=0.42, center='end'),
    ]


_logo = []


def ending(im, fr):
    k = fr - F(18)
    if k < 0:
        return
    if not _logo:
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        _logo.append(lg.resize((lg.width * 3 // 4, lg.height * 3 // 4), Image.LANCZOS))
    lg = _logo[0]
    a = ease((k - 4) / 10)
    if a > 0:
        part = lg.copy()
        part.putalpha(part.getchannel('A').point(lambda v: int(v * a)))
        im.paste(part, ((W - lg.width) // 2, 100 + round(14 * (1 - a))), part)
    words = [Head(F(18, 1), FRAMES, 'LIBERATE THE NET.', None, y=440, size=56, color=WHITE),
             Head(F(18, 2), FRAMES, '', 'A WAY YOU CHOOSE  ·  A BOARD NEVER THE SAME  ·  IN BN5\'S OWN LOOK AND SOUND', y=468,
                  size=10, cap_color=AMBER),
             Head(F(19), FRAMES, '', 'saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless', y=508, size=10,
                  cap_color=CYAN)]
    notes = ('Unofficial fan project, not affiliated with Capcom. Free; runs from your own Mega Man Battle '
             'Network 6: Cybeast Gregar (USA) ROM.',
             'Liberation Missions need Mega Man Battle Network 5: Team Colonel (USA) beside it.')
    for w in words:
        w.draw(im, fr, NIGHT)
    d = ImageDraw.Draw(im)
    a = ease((fr - F(19, 1)) / 8)
    for i, n in enumerate(notes):
        d.text((W / 2, 632 + 28 * i), n, font=mono(16, 400), fill=lerp(NIGHT, (150, 156, 180), a), anchor='mm')


def camera(timeline, fr):
    """The camera's place: each shot's, reached from the last one's by a
    share of what is left each frame."""
    i = max(j for j, s in enumerate(timeline) if s.start <= fr)
    shot = timeline[i]
    if i == 0:
        return shot.place, i
    prev = camera(timeline, shot.start - 1)[0]
    k = fr - shot.start
    t = 1 - (1 - shot.pull) ** (k + 1)
    if t > 0.995:
        t = 1.0
    return (prev[0] + (shot.place[0] - prev[0]) * t, prev[1] + (shot.place[1] - prev[1]) * t), i


def compose(only=None):
    frames_dir = os.path.join(kit.WORK, 'frames')
    if only is None:
        shutil.rmtree(frames_dir, ignore_errors=True)
    os.makedirs(frames_dir, exist_ok=True)
    timeline = shots()
    for fr in range(FRAMES):
        if only is not None and fr not in only:
            continue
        red = red_at(fr)
        cam, i = camera(timeline, fr)
        im = Image.new('RGB', (W, H))
        draw_map(im, cam, red)
        shot = timeline[i]
        if i > 0 and fr - shot.start < 16:
            timeline[i - 1].draw(im, fr, cam, red)
        shot.draw(im, fr, cam, red)
        shot.draw_words(im, fr, red)
        if shot.center == 'end':
            ending(im, fr)
        hide = hud_hidden(fr)
        if hide < 1:
            top = im.copy()
            readout(top, fr, red, shot.where)
            im = Image.blend(top, im, hide) if hide else top
        if fr < 10:
            im = Image.blend(Image.new('RGB', (W, H)), im, ease(fr / 9))
        if fr > FRAMES - 18:
            im = Image.blend(im, Image.new('RGB', (W, H)), min(1.0, (fr - (FRAMES - 18)) / 16))
        im.save(os.path.join(frames_dir, f'{fr:05d}.png'), compress_level=1)
        if fr % 100 == 0:
            print('frame', fr, '/', FRAMES, flush=True)


POSTER = F(3, 3)        # (the title, the camera come to the Darkloid)
