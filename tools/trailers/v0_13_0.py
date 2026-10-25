"""0.13.0's trailer, Cyberworld Endless connects: the online Net Square,
in the manner of an online game's launch trailer rather than the last two
trailers' advert and liberation mission. Linking: several players' games
on screen at once, each a window in its operator's colour with his name,
channel and ping, joined by lines of light whose packets carry the cuts
(a window opens as a packet from the last one reaches it). The words are
the Square's own: each step's pill, its headline in a speech bubble
pointing at the game, the phrases as chat bubbles; a HUD over the top
(connecting, the channel, the operators here, the ping) and the Square's
log at the bottom. The arc: two players logging in and saying hello; the
gate in Lan's HP; the NetPass's stamps; meeting, trading, a NetBattle from
both sides, the stands; the week's DeepNet; two, four, then eight games;
the logo and a friend to bring.

Its shots are whole frames of the game running from the developer's ROMs,
BN6's and BN5 Team Colonel's beside it, at a whole scale (3x, 2x, 1x):
games in a Square of their own on a local server, several at once
(tools/trailer_online.py), each scripted by the wall clock. Everything
else is drawn here; its music is v0_13_0_music.py's, an original piece in
the manner of BN6's "Surge of Power!".

    python3 build.py linux && python3 build.py server && python3 tools/trailer.py 0.13.0 [--keep]
"""
import math
import os
import shutil
import sys

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer as kit  # noqa: E402
import trailer_online as online  # noqa: E402
from trailer import ROOT  # noqa: E402
from trailers import v0_13_0_music as music  # noqa: E402

VERSION = '0.13.0'
PLAY_LABEL = 'WATCH THE 0.13.0 TRAILER'
BN6_ONLY = ()


# ---- the footage: games in a Square of their own, at once ----

STAMPED = {'netpass.ini': 'stamps=4\n'}   # (the stamp moment seen before: no greeter's ceremony)
ARRIVE = 1232                             # (in the Square, the log-in beam: the frame, through the gate at once)


def steps_frames(steps):
    return sum(int(s.split(':')[0]) for s in steps.split(',') if s)


class Visit:
    """A game's script by the wall clock: through the gate at once, then
    its steps at seconds after the scene's first game arrived (a game that
    starts `delay` seconds later reaches each second that much sooner in
    its own frames; a visit plays at the clock's pace)."""

    def __init__(self, delay=0.0, ask=False):
        self.delay = delay
        if ask:
            # (MegaMan's question with the NetPass's strip, LEFT to Yes)
            self.s = online.PAGED + '0:place 16 72 3,110:,6:LEFT,24:,6:A,'
            self.t, self.arrive = 1200 + 146, ARRIVE + 146
        else:
            self.s = online.PAGED + '0:place 16 72 3,'
            self.t, self.arrive = 1200, ARRIVE

    def frame(self, w):
        return self.arrive + round((w - self.delay) * 60)

    def at(self, w, steps=''):
        target = self.frame(w)
        if target > self.t:
            self.s += f'{target - self.t}:,'
            self.t = target
        return self.then(steps)

    def then(self, steps):
        if steps:
            self.s += steps.rstrip(',') + ','
            self.t += steps_frames(steps)
        return self


def strip(v, page, k):
    """Item k of the emote strip's page (0 the emotes, six by two; 1 the
    phrases, two by four) sent from SELECT (issue #178, which replaced
    L's pages; the 0.13.0 cut went through them): the steps from where its
    cursor was left in this visit (`v.strip`), round the grid's ends."""
    cols, rows = (6, 2) if page == 0 else (2, 4)
    state = v.__dict__.setdefault('strip', {'page': 0, 0: 0, 1: 0})
    steps = '6:SELECT,20:,'
    if state['page'] != page:
        steps += '6:L,10:,' if page == 0 else '6:R,10:,'
    cur = state[page]
    for delta, n, ahead, back in (((k % cols) - (cur % cols), cols, 'RIGHT', 'LEFT'),
                                  ((k // cols) - (cur // cols), rows, 'DOWN', 'UP')):
        delta %= n
        steps += (f'6:{ahead},6:,' * delta) if delta <= n // 2 else (f'6:{back},6:,' * (n - delta))
    state['page'], state[page] = page, k
    return steps + '6:A,'


def say(v, k):
    """The strip's phrase k ("Hello!" 0, "Nice build!" 1)"""
    return strip(v, 1, k)


def emote(v, k):
    """The strip's emote k (Wave 0, Cheer 1)"""
    return strip(v, 0, k)


# (the eight round the plaza's landmark, in front of it on the screen:
# world x, y and facing, toward it)
RING = [(-76, 76, 1), (-60, 132, 0), (-4, 148, 7), (40, 140, 6), (-100, 40, 1), (-110, 120, 0), (-36, 190, 7),
        (-130, 176, 0)]
OPERATORS = ['Kei', 'Mira', 'Dex', 'Rio', 'Nova', 'Juno', 'Tomo', 'Sora']
DELAYS = [0, 2.5, 4, 5, 6, 7, 7.5, 8]


def meet_game(i):
    v = Visit(DELAYS[i])
    if i < 2:
        v.at(4 + 0.3 * i, say(v, 0))                # (the first two say hello as Mira arrives)
    x, y, f = RING[i]
    v.at(13, f'0:place {x} {y} {f},')
    v.at(15 + 0.6 * (i % 4), say(v, i % 2))         # (together round the landmark: the Square cheers)
    v.at(26 + 0.4 * i, emote(v, i % 2))
    v.at(40)
    first = 1150 if i < 2 else v.frame(12)
    return dict(name=OPERATORS[i].lower(), operator=OPERATORS[i], delay=DELAYS[i], script=v.s, files=STAMPED,
                first=first, last=v.t)


def gate_scene():
    """Alpha through the gate as a player goes, his first visit with the
    full pass (the greeter's stamp moment), then the Board's posts and the
    week's DeepNet (its entries seeded on the server)."""
    v = Visit(0, ask=True)
    v.at(2.6, '6:A,')          # (the greeter's first page)
    v.at(4.6, '6:A,')          # (... and "STAMP,STAMP,STAMP!": the pass opens, the stamps land)
    v.at(11, '6:A,')           # (the full pass closed)
    v.at(12.5, '0:place 124 396 1,20:,6:A,')    # (the Board, from its front)
    for k in range(10):
        v.at(14 + 2.2 * k, '6:A,')
    v.at(37)
    return dict(name='kei', operator='Kei', ask=True, script=v.s, first=1150, last=v.t)


WEEK = [('Mira', 10, True, 41), ('Dex', 9, False, 38), ('Rio', 8, False, 35), ('Nova', 7, False, 29),
        ('Juno', 6, False, 22), ('Tomo', 5, False, 25), ('Sora', 4, False, 17), ('Zed', 3, False, 11)]


def battle_scene():
    """A NetBattle from both sides (both games play it by script, charged
    shots), Dex in the stands from the referee's WANNA WATCH?, cheering,
    and the referee's word to Kei after it."""
    mira = Visit(0).at(3, '0:place -60 300 1,').at(55)
    kei = Visit(1).at(4, '0:place -44 300 5,20:,')
    for w in (5, 6.2, 7.4, 8.6, 9.8, 11):       # (his card's pages typed out, then Battle)
        kei.at(w, '6:A,')
    kei.at(40, '6:A,').at(55)                  # (back in the Square: "Alright! We won,Lan!!")
    dex = Visit(2).at(9, '0:place -428 28 3,20:,')
    for k in range(9):
        dex.at(11 + 1.4 * k, '6:A,')            # (the referee's pages, until his WANNA WATCH?'s Yes)
    for w in (25, 27, 29):
        dex.at(w, '6:L,')                       # (cheers)
    dex.at(31, '6:R,')
    for k in range(8):
        dex.at(39 + 1.4 * k, '6:A,')            # (back in the Square, the referee calls the bout)
    dex.at(55)
    hp = ['arenahp=40']
    return {'games': [
        dict(name='mira', operator='Mira', delay=0, script=mira.s, dev=['arenaboth'] + hp, files=STAMPED,
             first=mira.frame(4), last=mira.t),
        dict(name='kei', operator='Kei', delay=1, script=kei.s, dev=['arenaboth'] + hp, files=STAMPED,
             first=kei.frame(4), last=kei.t),
        dict(name='dex', operator='Dex', delay=2, script=dex.s, files=STAMPED,
             first=dex.frame(9), last=dex.t),
    ]}


# (Trade Central's two screens, as the trade dev step opens them in a run:
# the spares list, its sorts, and the chip trade's swap; docs/devtools/)
CHIPS = '250:,0:chip 1 A 1,0:chip 20 F 1,0:chip 411 M 1,0:chip 413 E 1,0:chip 21 S 1,'
TRADE_RUN = ['--scene', 'emu', '--run-depth', '2']


def online_scenes():
    return {
        'gate': {'seed': 120, 'weeks': WEEK, 'games': [gate_scene()]},
        'meet': {'seed': 120, 'games': [meet_game(i) for i in range(8)]},
        'battle': dict(seed=120, **battle_scene()),
        'trade': {'games': [
            dict(name='list', offline=True, args=TRADE_RUN, first=250, last=640,
                 script=CHIPS + '0:trade list,50:,6:DOWN,24:,6:DOWN,24:,6:DOWN,30:,6:START,44:,6:START,44:,6:UP,24:,6:UP,60:'),
            dict(name='deal', offline=True, args=TRADE_RUN, first=250, last=620,
                 script=CHIPS + '0:trade deal 20 5 411 12,60:,6:RIGHT,30:,6:A,70:,0:trade swap,200:'),
        ]},
    }


def scenes():
    """(none from the release pictures' runs: every scene is online_scenes's,
    captured by footage() below)"""
    return {}


def footage(keep):
    online.capture(kit.WORK, online_scenes(), keep)


# ---- the look: linking ----

FPS, W, H = 30, 1280, 720
BAR = round(FPS * 60 * 4 / music.BPM)     # (48 frames at 150 beats a minute)
BEAT = BAR / 4
BARS = music.BARS
FRAMES = BARS * BAR + round(music.TAIL * FPS)
POSTER = 19 * BAR + 30                    # the wall of eight players' games

GOTHIC = '/usr/share/fonts/opentype/urw-base35/URWGothic-Demi.otf'
BOOK = '/usr/share/fonts/opentype/urw-base35/URWGothic-Book.otf'
SEMI = '/usr/share/fonts/truetype/lato/Lato-Semibold.ttf'
MONO = '/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf'

INK = (5, 9, 24)
NAVY = (12, 22, 52)
DOT = (30, 50, 96)
LINK = (120, 226, 255)
WHITE = (240, 248, 255)
SOFT = (176, 206, 240)
GOLD = (255, 214, 72)
OPS = {'Kei': (66, 214, 255), 'Mira': (255, 112, 190), 'Dex': (160, 236, 90), 'Rio': (255, 168, 64),
       'Nova': (178, 140, 255), 'Juno': (255, 222, 80), 'Tomo': (80, 236, 200), 'Sora': (255, 96, 96),
       'Beta': (255, 140, 120)}


def F(bar, beat=0):
    return round(bar * BAR + beat * BEAT)


_fonts = {}


def font(path, size):
    if (path, size) not in _fonts:
        _fonts[(path, size)] = ImageFont.truetype(path, size)
    return _fonts[(path, size)]


def lerp(a, b, t):
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def ease(t):
    t = max(0.0, min(1.0, t))
    return 1 - (1 - t) ** 3


def pop(k):
    """A bubble's scale as it pops in: past its size and back."""
    if k < 0:
        return 0.0
    if k >= 6:
        return 1.0
    return [0.55, 0.85, 1.06, 1.08, 1.03, 1.0][k]


def fit(text, path, size, width, lines=2, least=14):
    """`text` wrapped to `width` in at most `lines` lines, the font shrunk
    until it does: (font, lines)."""
    for s in range(size, least - 1, -1):
        f = font(path, s)
        out, line = [], ''
        for word in text.split(' '):
            trial = (line + ' ' + word).strip()
            if f.getlength(trial) <= width or not line:
                line = trial
            else:
                out.append(line)
                line = word
        out.append(line)
        if len(out) <= lines and all(f.getlength(x) <= width for x in out):
            return f, out
    return f, out


# ---- the field: the net's dark, a dot grid, its nodes ----

def field(dark=0.0):
    im = Image.new('RGB', (W, H), INK)
    d = ImageDraw.Draw(im)
    for y in range(16, H, 32):
        for x in range(16, W, 32):
            c = lerp(DOT, INK, dark)
            d.rectangle([x - 1, y - 1, x, y], fill=c)
    v = Image.new('L', (W // 8, H // 8))
    px = v.load()
    for y in range(H // 8):
        for x in range(W // 8):
            r = math.hypot((x - W / 16) / (W / 16), (y - H / 16) / (H / 16))
            px[x, y] = int(max(0, min(200, (r - 0.55) * 260 + 120 * dark)))
    im.paste(INK, (0, 0), v.resize((W, H), Image.BICUBIC))
    return im


# ---- a player's game: a window with his name, his channel and ping ----

class Win:
    """A player's game at a whole scale, its frame in his colour. `feed`
    is [(frame, scene, sample, rate)]: from each frame on, the scene's
    footage from that sample (rate 0 holds it). It opens as a packet
    reaches it (`src`, a point the packet starts from) and closes into one."""

    def __init__(self, op, scale, x, y, feed, opens, closes=None, src=None, ping=31, label=None, head=None, pad=None):
        self.op, self.s, self.x, self.y = op, scale, x, y
        self.feed, self.opens, self.closes, self.src = feed, opens, closes, src
        self.ping, self.label = ping, label
        self.color = OPS[op]
        self.pad = pad or (6 if scale > 1 else 4)
        self.head = head or (28 if scale > 1 else 20)
        self.pw, self.ph = 240 * scale, 160 * scale
        self.box = (x - self.pad, y - self.head - self.pad, x + self.pw + self.pad, y + self.ph + self.pad)

    @property
    def anchor(self):
        """where packets meet it: its header's left end"""
        return (self.box[0] + 18, self.box[1] + self.head // 2 + self.pad // 2)

    def center(self):
        return ((self.box[0] + self.box[2]) / 2, (self.box[1] + self.box[3]) / 2)

    def source(self, fr):
        i = max(j for j, e in enumerate(self.feed) if e[0] <= fr or j == 0)
        start, scene, sample, rate = self.feed[i][:4]
        return i, scene, sample + max(0, round((fr - start) * rate))

    def who(self, fr):
        """the player whose game the window shows now (a feed entry's fifth)"""
        i = max(j for j, e in enumerate(self.feed) if e[0] <= fr or j == 0)
        return self.feed[i][4] if len(self.feed[i]) > 4 else self.op

    def picture(self, fr):
        i, scene, sample = self.source(fr)
        pic = kit.shot(scene, sample)
        return pic.resize((self.pw, self.ph), Image.NEAREST) if self.s > 1 else pic

    def shown(self, fr):
        return fr >= self.opens and (self.closes is None or fr < self.closes + 8)

    def draw(self, im, fr, dim=0.0):
        if not self.shown(fr):
            return
        k = fr - self.opens
        x0, y0, x1, y1 = self.box
        d = ImageDraw.Draw(im)
        op = self.who(fr)
        color = lerp(OPS[op], (70, 90, 130), dim)
        closing = self.closes is not None and fr >= self.closes
        c = (fr - self.closes) if closing else 0
        # the frame traced from its header's left end round both ways
        trace = 1.0 if k >= 6 else ease(k / 6)
        if closing:
            trace = 1.0 - ease(c / 7)
        if trace < 1.0:
            per = 2 * ((x1 - x0) + (y1 - y0))
            n = per * trace / 2
            pts = [(x0, y0), (x1, y0), (x1, y1), (x0, y1), (x0, y0)]
            for direction in (pts, pts[::-1]):
                left, last = n, direction[0]
                for p in direction[1:]:
                    seg = math.hypot(p[0] - last[0], p[1] - last[1])
                    if left <= 0:
                        break
                    t = min(1.0, left / seg)
                    q = (last[0] + (p[0] - last[0]) * t, last[1] + (p[1] - last[1]) * t)
                    d.line([last, q], fill=color, width=3)
                    left -= seg
                    last = p
            if closing:
                return
        if k < 4:
            return
        # the frame and the header bar
        im.paste(INK, (x0, y0, x1, y1))
        d.rounded_rectangle([x0, y0, x1, y1], radius=8, outline=color, width=3)
        hb = (x0 + 3, y0 + 3, x1 - 3, y0 + self.head + 1)
        head_open = ease((k - 4) / 4) if not closing else 1.0
        d.rectangle([hb[0], hb[1], hb[0] + (hb[2] - hb[0]) * head_open, hb[3]], fill=color)
        if head_open >= 1:
            big = self.s > 1
            f = font(GOTHIC, 20 if big else 14)
            d.text((hb[0] + 10, (hb[1] + hb[3]) // 2 + 1), (self.label or op).upper(), font=f, fill=INK, anchor='lm')
            ping = self.ping + round(4 * math.sin(fr * 0.09 + self.ping))
            info = f'CH 1  ·  {ping} ms' if big else f'{ping}ms'
            fm = font(MONO, 14 if big else 11)
            right = hb[2] - 10
            for i in range(4):
                h = 4 + 3 * i
                on = i < 2 + (fr // 9 + self.ping) % 3
                d.rectangle([right - 30 + 8 * i, hb[3] - 6 - h, right - 25 + 8 * i, hb[3] - 6],
                            fill=INK if on else lerp(color, INK, 0.45))
            d.text((right - 40, (hb[1] + hb[3]) // 2 + 1), info, font=fm, fill=INK, anchor='rm')
        # the picture: rows coming in, a scan line at each re-tune, a
        # collapse to a line as it closes
        if k < 6:
            return
        pic = self.picture(fr)
        if dim:
            pic = Image.blend(pic, Image.new('RGB', pic.size, INK), dim)
        i, _, _ = self.source(fr)
        since = fr - self.feed[i][0]
        if 0 < i and since < 5:
            old = self.picture(self.feed[i][0] - 1)
            line = round(self.ph * since / 5)
            old.paste(pic.crop((0, 0, self.pw, line)), (0, 0))
            pic = old
            ImageDraw.Draw(pic).rectangle([0, line, self.pw, line + 2 * self.s], fill=lerp(color, WHITE, 0.4))
        if closing:
            h = max(2, round(self.ph * (1 - ease(c / 4))))
            pic = pic.crop((0, (self.ph - h) // 2, self.pw, (self.ph - h) // 2 + h))
            im.paste(pic, (self.x, self.y + (self.ph - h) // 2))
            return
        reveal = (k - 6) / 4
        if reveal < 1:
            rows = Image.new('L', (self.pw, self.ph), 0)
            rd = ImageDraw.Draw(rows)
            for yy in range(0, self.ph, 2 * self.s):
                if yy / self.ph < reveal * 1.6 or (yy // (2 * self.s)) % 2 == 0 and yy / self.ph < reveal * 3:
                    rd.rectangle([0, yy, self.pw, yy + 2 * self.s - 1], fill=255)
            im.paste(pic, (self.x, self.y), rows)
        else:
            im.paste(pic, (self.x, self.y))


# ---- the lines between players: packets running along them ----

def packet(im, a, b, t, color, trail=0.35, size=5):
    """A packet at t along a to b (straight), its light trailing it."""
    d = ImageDraw.Draw(im)
    if t <= 0:
        return
    t = min(t, 1.0)
    for j in range(8):
        u = t - trail * j / 8
        if u < 0:
            break
        px, py = a[0] + (b[0] - a[0]) * u, a[1] + (b[1] - a[1]) * u
        r = size * (1 - j / 9)
        d.rectangle([px - r, py - r, px + r, py + r], fill=lerp(color, INK, j / 8))
    px, py = a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t
    d.rectangle([px - 2, py - 2, px + 2, py + 2], fill=WHITE)


def link(im, a, b, fr, color, speed=0.035, count=2, alpha=1.0, both=True):
    """A line of light between two players' games, its packets running
    both ways."""
    if alpha <= 0:
        return
    d = ImageDraw.Draw(im)
    d.line([a, b], fill=lerp(INK, lerp(color, INK, 0.35), alpha), width=2)
    for i in range(count):
        t = (fr * speed + i / count) % 1.0
        packet(im, a, b, t, lerp(INK, color, alpha), trail=0.12, size=3)
        if both:
            packet(im, b, a, (t + 0.5 / count) % 1.0, lerp(INK, color, alpha), trail=0.12, size=3)


# ---- words: a step's pill, its headline in a speech bubble, a caption ----

def pill(im, text, x, y, color, k, anchor='l'):
    if k < 0:
        return
    f = font(GOTHIC, 22)
    w = round(f.getlength(text)) + 30
    if anchor == 'c':
        x -= w // 2
    grow = ease(k / 5)
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([x, y, x + max(16, w * grow), y + 34], radius=17, fill=color)
    if grow > 0.95:
        d.text((x + 15, y + 18), text, font=f, fill=INK, anchor='lm')


def bubble(im, text, tip, side, k, size=58, width=420, lines=2, path=GOTHIC, name=None, color=None, out=None):
    """A speech bubble whose tail points at `tip` from `side` (the bubble
    'above', 'below', 'left' or 'right' of it), popping in at k = 0; a
    name tag on its top edge where given; out: frames from its close."""
    if k < 0:
        return
    scale = pop(k)
    if out is not None and out >= 0:
        scale *= max(0.0, 1 - out / 4)
    if scale <= 0.05:
        return
    f, rows = fit(text, path, size, width, lines)
    asc, desc = f.getmetrics()
    lh = round((asc + desc) * 1.02)
    tw = max(f.getlength(r) for r in rows)
    bw, bh = round(tw + 2 * 22), lh * len(rows) + 2 * 14
    tail = 16
    canvas = Image.new('RGBA', (bw + 2 * tail + 8, bh + 2 * tail + 8), (0, 0, 0, 0))
    d = ImageDraw.Draw(canvas)
    ox, oy = tail + 4, tail + 4
    d.rounded_rectangle([ox, oy, ox + bw, oy + bh], radius=min(22, bh // 2), fill=WHITE + (255,),
                        outline=(color or WHITE) + (255,), width=3)
    if side == 'above':
        tx, ty = ox + min(bw - 40, 40), oy + bh
        d.polygon([(tx, ty - 2), (tx + 22, ty - 2), (tx + 4, ty + tail)], fill=WHITE + (255,))
        tipc = (tx + 4, ty + tail)
    elif side == 'below':
        tx, ty = ox + min(bw - 40, 40), oy
        d.polygon([(tx, ty + 2), (tx + 22, ty + 2), (tx + 4, ty - tail)], fill=WHITE + (255,))
        tipc = (tx + 4, ty - tail)
    elif side == 'left':
        tx, ty = ox + bw, oy + bh // 2
        d.polygon([(tx - 2, ty - 12), (tx - 2, ty + 10), (tx + tail, ty + 4)], fill=WHITE + (255,))
        tipc = (tx + tail, ty + 4)
    else:
        tx, ty = ox, oy + bh // 2
        d.polygon([(tx + 2, ty - 12), (tx + 2, ty + 10), (tx - tail, ty + 4)], fill=WHITE + (255,))
        tipc = (tx - tail, ty + 4)
    for i, r in enumerate(rows):
        d.text((ox + 22, oy + 14 + i * lh), r, font=f, fill=INK + (255,))
    if name:
        fn = font(GOTHIC, 16)
        nw = round(fn.getlength(name)) + 18
        d.rounded_rectangle([ox + 12, oy - 13, ox + 12 + nw, oy + 9], radius=10, fill=(color or LINK) + (255,))
        d.text((ox + 21, oy - 2), name, font=fn, fill=INK + (255,), anchor='lm')
    if scale != 1.0:
        sw, sh = max(1, round(canvas.width * scale)), max(1, round(canvas.height * scale))
        canvas = canvas.resize((sw, sh), Image.BILINEAR)
        tipc = (tipc[0] * scale, tipc[1] * scale)
    im.paste(canvas, (round(tip[0] - tipc[0]), round(tip[1] - tipc[1])), canvas)


class Words:
    """A step of the arc: its pill, its headline bubble (its tail toward the
    game), its caption; at (x, y) in a column `width` wide."""

    def __init__(self, start, end, step, head, caption, x, y, width, color, side='right', size=60):
        self.start, self.end = start, end
        self.step, self.head, self.caption = step, head, caption
        self.x, self.y, self.width, self.color, self.side, self.size = x, y, width, color, side, size

    def draw(self, im, fr):
        k = fr - self.start
        if k < 0 or fr >= self.end + 4:
            return
        out = fr - self.end if fr >= self.end else None
        if self.step and out is None:
            pill(im, self.step, self.x, self.y, self.color, k)
        f, rows = fit(self.head, GOTHIC, self.size, self.width - 50, 2)
        lh = sum(f.getmetrics()) * 1.02
        bh = lh * len(rows) + 28
        top = self.y + 52
        if self.side == 'right':       # (the game to its right: the tail on the right edge)
            tw = max(f.getlength(r) for r in rows) + 44
            tip = (self.x + tw + 16 + 4, top + bh / 2 + 4)
            bubble(im, self.head, tip, 'left', k - 2, self.size, self.width - 50, 2, color=self.color, out=out)
        elif self.side == 'left':
            tip = (self.x - 16 - 4, top + bh / 2 + 4)
            bubble(im, self.head, tip, 'right', k - 2, self.size, self.width - 50, 2, color=self.color, out=out)
        else:                           # (the game below: the tail down)
            tip = (self.x + 44, top + bh + 16)
            bubble(im, self.head, tip, 'above', k - 2, self.size, self.width - 50, 2, color=self.color, out=out)
        if self.caption and out is None and k >= 8:
            d = ImageDraw.Draw(im)
            a = ease((k - 8) / 5)
            if self.side == 'above':
                # (one line under the bubble, over the games)
                fc, crow = fit(self.caption, SEMI, 25, self.width, 1, least=16)
                tw = fc.getlength(crow[0])
                box = Image.new('L', (round(tw) + 24, 36), round(200 * a))
                im.paste(INK, (self.x - 8, round(top + bh + 4)), box)
                d.text((self.x + 4, top + bh + 8), crow[0], font=fc, fill=lerp(INK, SOFT, a))
                return
            fc, crow = fit(self.caption, SEMI, 25, self.width, 3, least=15)
            yy = top + bh + 18
            for i, r in enumerate(crow):
                d.text((self.x + 4, yy + i * 31), r, font=fc, fill=lerp(INK, SOFT, a))


# ---- the HUD over the top, and the Square's log at the bottom ----

class Hud:
    def draw(self, im, fr, status, operators, dark=0.0):
        d = ImageDraw.Draw(im)
        im.paste(lerp(INK, NAVY, 0.6), (0, 0, W, 38))
        d.line([(0, 38), (W, 38)], fill=lerp(LINK, INK, 0.55 + 0.3 * dark), width=1)
        f = font(MONO, 16)
        on = status != 'CONNECTING'
        dot = GOLD if not on else (LINK if dark < 0.5 else (150, 120, 255))
        if on or (fr // 6) % 2 == 0:
            d.ellipse([18, 13, 30, 25], fill=dot)
        text = status if on else 'CONNECTING' + '.' * (1 + (fr // 5) % 3)
        d.text((40, 19), text, font=f, fill=WHITE, anchor='lm')
        x = max(236, 40 + f.getlength(text) + 40)
        d.text((x, 19), 'NET SQUARE  ·  CH 1  ·  CENTRAL AREA', font=font(MONO, 14), fill=SOFT, anchor='lm')
        # the operators here, eight pips
        x = 850
        d.text((x, 19), 'OPERATORS', font=font(MONO, 14), fill=SOFT, anchor='lm')
        for i in range(8):
            c = list(OPS.values())[i] if i < operators else lerp(NAVY, SOFT, 0.2)
            d.rounded_rectangle([x + 96 + 16 * i, 12, x + 106 + 16 * i, 26], radius=3, fill=c)
        d.text((x + 228, 19), f'{operators}/8', font=f, fill=WHITE, anchor='lm')
        # the ping, a line of the last seconds
        base = 1170
        pts = []
        for i in range(24):
            t = fr - (23 - i) * 2
            v = 34 + 5 * math.sin(t * 0.21) + 3 * math.sin(t * 0.07 + 1)
            pts.append((base + i * 3, 30 - (v - 24) * 0.8))
        if on:
            d.line(pts, fill=LINK, width=1)
            d.text((W - 14, 19), f'{round(34 + 5 * math.sin(fr * 0.21))}ms', font=font(MONO, 13), fill=WHITE, anchor='rm')


class Log:
    """The Square's events as a chat log, the newest at the bottom: (frame,
    who, words)."""

    def __init__(self, lines):
        self.lines = sorted(lines)

    def draw(self, im, fr, x, y, rows=3, alpha=1.0):
        seen = [ln for ln in self.lines if ln[0] <= fr][-rows - 1:]
        if not seen or alpha <= 0:
            return
        d = ImageDraw.Draw(im)
        f = font(MONO, 16)
        k = fr - seen[-1][0]
        slide = 22 * (1 - ease(k / 5))
        for i, (at, who, words) in enumerate(reversed(seen)):
            yy = y - i * 24 + slide
            fade = alpha * (1.0 if i == 0 else max(0.0, 1 - 0.28 * i - (0.28 * (1 - ease(k / 5)) if i == rows else 0)))
            if i > rows or fade <= 0:
                continue
            c = OPS.get(who, LINK)
            name = '>' if who == '' else who.upper()
            d.text((x, yy), name, font=f, fill=lerp(INK, c, fade))
            d.text((x + f.getlength(name) + 10, yy), words, font=f, fill=lerp(INK, WHITE, fade))


# ---- the timeline, in bars and beats ----

# where a game stands: one at 3x on the left or the right, two at 2x, four
# at 2x filling the frame, eight at 1x
L3, R3 = (62, 118), (498, 118)
TWO = [(102, 250), (698, 250)]
FOUR = [(136, 30), (672, 30), (136, 390), (672, 390)]
EIGHT = [(94 + 284 * (i % 4), 190 + 240 * (i // 4)) for i in range(8)]
HUB = (1176, 26)    # (the server, where the HUD's ping line runs)

KEI_HELLO, MIRA_HELLO = 384, 318


def windows():
    ws = []

    def add(*a, **k):
        w = Win(*a, **k)
        ws.append(w)
        return w

    # the hook: Kei logs in, Mira after him, and they say hello
    kei = add('Kei', 2, *TWO[0], [(0, 'meet-kei', 38, 1), (F(1), 'meet-kei', KEI_HELLO, 1)], -10, F(2), ping=29)
    mira = add('Mira', 2, *TWO[1], [(F(0, 1), 'meet-mira', 38, 1), (F(1), 'meet-mira', MIRA_HELLO, 1)], F(0, 1) + 2,
               F(2), src=kei.anchor, ping=36)
    # going online, the NetPass's stamps on the beats, Full pass on the bar
    gate = add('Kei', 3, *L3, [(F(2), 'gate-kei', 34, 1), (F(3) + 18, 'gate-kei', 104, 0), (F(4), 'gate-kei', 111, 1),
                               (F(5) - 6, 'gate-kei', 262, 1), (F(5, 1) - 2, 'gate-kei', 283, 1),
                               (F(5, 2) - 2, 'gate-kei', 300, 1), (F(5, 2) + 8, 'gate-kei', 310, 0),
                               (F(6) - 2, 'gate-kei', 313, 1)], F(2) + 2, F(7), src=kei.anchor, ping=33)
    # the features
    meet = add('Kei', 3, *R3, [(F(7), 'meet-kei', 756, 1), (F(8), 'meet-rio', 384, 1, 'Rio')], F(7) + 2, F(9),
               src=gate.anchor, ping=31)
    trade = add('Kei', 3, *L3, [(F(9), 'trade-list', 30, 1), (F(9, 2), 'trade-deal', 16, 1),
                                (F(10), 'trade-deal', 84, 1)], F(9) + 2, F(11), src=meet.anchor, ping=30)
    bk = add('Kei', 2, *TWO[0], [(F(11), 'battle-kei', 318, 1), (F(12), 'battle-kei', 700, 1)], F(11) + 2, F(13),
             src=trade.anchor, ping=29)
    bm = add('Mira', 2, *TWO[1], [(F(11), 'battle-mira', 318, 1), (F(12), 'battle-mira', 700, 1)], F(11) + 6, F(13),
             src=bk.anchor, ping=36)
    stands = add('Dex', 3, *R3, [(F(13), 'battle-dex', 404, 1), (F(14), 'battle-dex', 1082, 1),
                                 (F(14, 2), 'battle-dex', 1122, 1)], F(13) + 2, F(15), src=bm.anchor, ping=41)
    deep = add('Kei', 3, *L3, [(F(15), 'gate-kei', 746, 1), (F(15, 2), 'gate-kei', 826, 1), (F(16), 'gate-kei', 866, 1)],
               F(15) + 2, F(17), src=stands.anchor, ping=33)
    # the climax: two, four, eight players' games
    c2 = [add('Kei', 2, *TWO[0], [(F(17), 'meet-kei', 1050, 1)], F(17) + 2, F(18), src=deep.anchor, ping=29),
          add('Mira', 2, *TWO[1], [(F(17), 'meet-mira', 1040, 1)], F(17) + 6, F(18), src=deep.anchor, ping=36)]
    fours = [('Dex', 690), ('Rio', 704), ('Nova', 720), ('Juno', 734)]
    c4 = [add(op, 2, *FOUR[i], [(F(18), f'meet-{op.lower()}', at, 1)], F(18) + 2 + 3 * i, F(19),
              src=c2[i % 2].anchor, ping=30 + 3 * i, head=24, pad=4) for i, (op, at) in enumerate(fours)]
    eights = [('Kei', 1084), ('Mira', 1090), ('Dex', 740), ('Rio', 754), ('Nova', 770), ('Juno', 784), ('Tomo', 750),
              ('Sora', 764)]
    c8 = []
    for i, (op, at) in enumerate(eights):
        src = c4[i % 4].anchor if i < 4 else None
        w = add(op, 1, *EIGHT[i], [(F(19), f'meet-{op.lower()}', at, 1)], F(19) + 2 + 2 * i, F(21) + i,
                src=src, ping=27 + 2 * i)
        if src is None:
            w.src = c8[i - 4].anchor
        c8.append(w)
    return ws, dict(hook=(kei, mira), gate=gate, meet=meet, trade=trade, battle=(bk, bm), stands=stands, deep=deep,
                    c2=c2, c4=c4, c8=c8)


COL_R = 836        # (the words' column beside a game on the left)
COL_L = 48         # (... and beside one on the right)
WORDS = [
    Words(F(2), F(3), None, 'TAKE THE GATE', "A GATE IN LAN'S HP, BETWEEN ACTS", COL_R, 96, 400, OPS['Kei'], 'left'),
    Words(F(3), F(4), None, 'CONNECTING...', "THE OFFICIAL SERVER, OR ANYONE'S", COL_R, 96, 400, OPS['Kei'], 'left'),
    Words(F(4), F(5), None, 'LOGGED IN!', 'THE NET SQUARE, CHANNEL 1', COL_R, 96, 400, OPS['Kei'], 'left'),
    Words(F(5), F(6), None, 'EVERY STAMP', 'SQUARE · ARENA · TRADE · DEEPNET', COL_R, 96, 400, GOLD, 'left'),
    Words(F(6), F(7), None, 'FULL PASS!', 'BN6 CYBEAST GREGAR + BN5 TEAM COLONEL (USA)', COL_R, 96, 400, GOLD, 'left'),
    Words(F(7), F(8), None, 'SAY HELLO', 'PHRASES, EMOTES, CARDS: 8 OPERATORS A CHANNEL', COL_L, 96, 410, OPS['Mira'],
          'right'),
    Words(F(8), F(9), None, 'THE SQUARE CHEERS', 'SAY SOMETHING TOGETHER BY ITS LANDMARK', COL_L, 96, 410, OPS['Mira'], 'right'),
    Words(F(9), F(10), None, 'SWAP A SPARE', "ONE CHIP FOR ONE, BN5'S CHIPS TOO", COL_R, 96, 400, OPS['Rio'], 'left'),
    Words(F(10), F(11), None, 'DEAL!', 'THE CHIP COMES HOME WITH YOU', COL_R, 96, 400, OPS['Rio'], 'left'),
    Words(F(11), F(13), '05 · NETBATTLE', 'NETBATTLE A FRIEND', 'ROLLBACK: 2 FRAMES OF INPUT DELAY ON A GOOD LINK', 102, 44,
          1000, OPS['Kei'], 'above', size=50),
    Words(F(13), F(14), None, 'FROM THE STANDS', 'L CHEERS, R SHOWS THE OTHER SIDE', COL_L, 96, 410, OPS['Dex'], 'right'),
    Words(F(14), F(15), None, 'THE REFEREE CALLS IT', 'EVERY BOUT IN YOUR CHANNEL, AND ITS CHEERS', COL_L, 96, 410,
          OPS['Dex'], 'right'),
    Words(F(15), F(16), None, "THE WEEK'S DEEP NET", 'EVERY MONDAY A NEW NET, THE SAME FOR EVERY DIVER', COL_R, 96, 400,
          OPS['Nova'], 'left'),
    Words(F(16), F(17), None, 'HOW DEEP?', 'THE BOARD KEEPS THE DEEPEST, AS REPORTED', COL_R, 96, 400, OPS['Nova'], 'left'),
    Words(F(17), F(18), None, 'ONE SQUARE, EVERY SYSTEM', 'NETBATTLES ON COMPUTERS, PHONES, HANDHELDS AND THE BROWSER', 102, 24,
          1000, OPS['Kei'], 'above', size=50),
    Words(F(18) + 6, F(19), None, 'SERVERS ANYONE CAN HOST', 'ROM-FREE: ONE PYTHON FILE, OR DOCKER', 300, 262, 760,
          OPS['Rio'], 'above', size=50),
    Words(F(19), F(20), None, '8 OPERATORS A CHANNEL', '', 94, 20, 1000, OPS['Juno'], 'above', size=48),
    Words(F(20), F(21), None, 'SEE YOU IN THE SQUARE', '', 94, 20, 1000, OPS['Tomo'], 'above', size=48),
]
# (the pills that stay while a step's headlines change)
PILLS = [(F(2), F(5), '01 · GO ONLINE', OPS['Kei']), (F(5), F(7), '02 · THE NETPASS', GOLD),
         (F(7), F(9), '03 · MEET', OPS['Mira']), (F(9), F(11), '04 · TRADE', OPS['Rio']),
         (F(13), F(15), '06 · WATCH', OPS['Dex']), (F(15), F(17), '07 · DIVE', OPS['Nova'])]

# the Square's chatter in the words' column: (frame, who, words)
CHATTER = [(F(7, 1), 'Dex', 'Hello!'), (F(7, 2), 'Rio', 'Nice build!'), (F(7, 3), 'Nova', 'Hello!'),
           (F(8), 'Juno', 'Nice build!'), (F(8, 1), 'Tomo', 'Hello!'), (F(8, 2), 'Sora', 'Hello!'),
           (F(13, 1), 'Dex', 'Cheer!'), (F(13, 2), 'Dex', 'Cheer!'), (F(13, 3), 'Dex', 'Cheer!')]

LOG = Log([
    (0, 'Kei', 'logged in'), (F(0, 1) + 6, 'Mira', 'logged in'), (F(1), 'Kei', 'Hello!'), (F(1, 1), 'Mira', 'Hello!'),
    (F(2, 1), '', "Lan's HP: the gate"), (F(3, 1), '', 'calling the Net Square...'), (F(4), 'Kei', 'logged in, channel 1'),
    (F(5), '', 'NetPass: Arena stamp'), (F(5, 1), '', 'NetPass: Trade stamp'), (F(5, 2), '', 'NetPass: DeepNet stamp'),
    (F(6), '', 'NetPass: full pass'),
    (F(7), 'Dex', 'logged in'), (F(7, 2), 'Rio', 'Nice build!'), (F(8), '', 'the Square cheers!'),
    (F(9), 'Kei', 'offers FireBrn1 F'), (F(9, 2), 'Beta', 'offers MrkCan1 M'), (F(10, 2), '', 'traded: FireBrn1 F for MrkCan1 M'),
    (F(11), 'Kei', 'challenges Mira'), (F(11, 2), '', 'NetBattle: Kei vs Mira'), (F(12), '', 'rollback on, delay 2 frames'),
    (F(13), 'Dex', 'watches Kei vs Mira'), (F(13, 2), 'Dex', 'cheers!'), (F(14), '', 'the referee calls it: 3 cheers'),
    (F(15), '', "the week's Deep Net is open"), (F(16), 'Mira', 'dove to layer 10!'),
    (F(17), 'Sora', 'logged in'), (F(17, 2), 'Tomo', 'o/'), (F(18), 'Juno', '* * *'), (F(18, 2), 'Rio', 'o/'),
    (F(19), 'Nova', 'Hello!'), (F(19, 2), 'Dex', 'Nice build!'), (F(20), 'Mira', 'o/'), (F(20, 2), 'Kei', 'See you!'),
])


def operators(fr):
    if fr < F(0, 1) + 6:
        return 1
    if fr < F(2):
        return 2
    if fr < F(4):
        return 0
    if fr < F(7):
        return 1
    return 8


def status(fr):
    if fr < 4 or F(3) <= fr < F(4):
        return 'CONNECTING'
    if F(15) <= fr < F(17):
        return 'DEEP NET  ·  OCT 19 - OCT 25'
    return 'ONLINE'


# ---- the hook's title and the end card ----

class Title:
    def draw(self, im, fr):
        if fr >= F(2) + 4:
            return
        out = fr - F(2) if fr >= F(2) else None
        if out is None:
            pill(im, 'CYBERWORLD ENDLESS', W // 2, 52, LINK, fr, anchor='c')
        bubble(im, 'CONNECTS', (W // 2 - 150, 214), 'above', fr - 8, 80, 600, 1, color=LINK, out=out)


class Ending:
    def __init__(self):
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        self.logo = lg.resize((lg.width * 3 // 4, lg.height * 3 // 4), Image.LANCZOS)

    def draw(self, im, fr):
        k = fr - F(21)
        if k < 0:
            return
        d = ImageDraw.Draw(im)
        lg = self.logo
        a = ease((k - 6) / 10)
        if a > 0:
            part = lg.copy()
            part.putalpha(part.getchannel('A').point(lambda v: int(v * a)))
            im.paste(part, ((W - lg.width) // 2, 66), part)
        bubble(im, 'BRING A FRIEND.', (W // 2 - 190, 400), 'above', k - 14, 76, 760, 1, color=OPS['Mira'])
        # you and a friend, linked, a packet each way on the beat
        if k >= 24:
            a, b = (W // 2 - 330, 568), (W // 2 + 330, 568)
            f = font(GOTHIC, 22)
            for (x, y), who, c in ((a, 'YOU', OPS['Kei']), (b, 'A FRIEND', OPS['Mira'])):
                d.ellipse([x - 9, y - 9, x + 9, y + 9], fill=c)
                d.text((x + (-20 if x < W // 2 else 20), y), who, font=f, fill=c, anchor='rm' if x < W // 2 else 'lm')
            d.line([(a[0] + 14, a[1]), (b[0] - 14, b[1])], fill=lerp(LINK, INK, 0.5), width=2)
            beat = (fr - F(21)) // 12
            t = ((fr - F(21)) % 12) / 12
            pa, pb = (a[0] + 14, a[1]), (b[0] - 14, b[1])
            packet(im, pa if beat % 2 else pb, pb if beat % 2 else pa, ease(t * 1.3),
                   OPS['Kei'] if beat % 2 else OPS['Mira'], trail=0.3, size=5)
        j = fr - F(22)
        if j >= 0:
            f = font(SEMI, 30)
            line = 'MEET  ·  TRADE  ·  NETBATTLE  ·  WATCH  ·  DIVE'
            d.text((W // 2, 440), line, font=f, fill=lerp(INK, WHITE, ease(j / 5)), anchor='mm')
            f2 = font(MONO, 24)
            url = 'saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless'
            d.text((W // 2, 500), url, font=f2, fill=lerp(INK, LINK, ease((j - 4) / 5)), anchor='mm')
            small = font(SEMI, 19)
            notes = ('Unofficial fan project, not affiliated with Capcom. Free; runs from your own Mega Man Battle '
                     'Network 6: Cybeast Gregar (USA) ROM.',
                     'The full NetPass needs Mega Man Battle Network 5: Team Colonel (USA) beside it. '
                     'NetBattles on every system but the New 3DS.')
            for i, n in enumerate(notes):
                d.text((W // 2, 634 + 28 * i), n, font=small, fill=lerp(INK, (150, 172, 205), ease((j - 8) / 6)), anchor='mm')


# ---- what is drawn between the games: links, packets, chatter ----

def links(im, fr, wins, dim=0.0):
    k = wins
    # the hook's two, and the battle's: one line, packets both ways
    for a, b, fast in ((k['hook'][0], k['hook'][1], False), (k['battle'][0], k['battle'][1], True),
                       (k['c2'][0], k['c2'][1], False)):
        if a.shown(fr) and b.shown(fr) and fr >= b.opens + 10 and (a.closes is None or fr < a.closes):
            pa = (a.box[2], (a.box[1] + a.box[3]) // 2)
            pb = (b.box[0], (b.box[1] + b.box[3]) // 2)
            link(im, pa, pb, fr, LINK, speed=0.07 if fast else 0.035, count=3 if fast else 2)
    # a single game: its line up to the server
    for w in (k['gate'], k['meet'], k['trade'], k['stands'], k['deep']):
        if w.opens + 10 <= fr < w.closes:
            p = (w.box[2] - 60, w.box[1])
            hub = (HUB[0], 38)
            corner = (p[0], 60)
            alpha = 1.0 - dim
            link(im, p, corner, fr, LINK, speed=0.05, count=1, alpha=alpha, both=False)
            link(im, corner, (hub[0], 60), fr, LINK, speed=0.03, count=2, alpha=alpha, both=False)
            link(im, (hub[0], 60), hub, fr, LINK, speed=0.08, count=1, alpha=alpha, both=False)
    # four in a ring, eight in a grid
    c4 = k['c4']
    if F(18) + 14 <= fr < F(19):
        for i, j in ((0, 1), (2, 3), (0, 2), (1, 3)):
            a, b = c4[i], c4[j]
            if i + 1 == j:
                pa, pb = (a.box[2], (a.box[1] + a.box[3]) // 2), (b.box[0], (b.box[1] + b.box[3]) // 2)
            else:
                pa, pb = ((a.box[0] + a.box[2]) // 2, a.box[3]), ((b.box[0] + b.box[2]) // 2, b.box[1])
            link(im, pa, pb, fr, LINK, speed=0.06, count=2)
    c8 = k['c8']
    if F(19) + 18 <= fr < F(21):
        for i in range(8):
            a = c8[i]
            if i % 4 < 3:
                b = c8[i + 1]
                link(im, (a.box[2], (a.box[1] + a.box[3]) // 2), (b.box[0], (b.box[1] + b.box[3]) // 2), fr,
                     OPS[a.op], speed=0.06, count=2)
            if i < 4:
                b = c8[i + 4]
                link(im, ((a.box[0] + a.box[2]) // 2, a.box[3]), ((b.box[0] + b.box[2]) // 2, b.box[1]), fr,
                     OPS[b.op], speed=0.05, count=1)


def transfers(im, fr, wins):
    """The cuts: a packet from the last game to the next as it opens; at
    the end, each of the eight to the logo."""
    for w in wins:
        if w.src is not None and w.opens - 8 <= fr < w.opens + 2:
            t = ease((fr - (w.opens - 8)) / 9)
            packet(im, w.src, w.anchor, t, OPS[w.op], trail=0.5, size=6)
        if w.closes is not None and w.closes >= F(21) and w.closes <= fr < w.closes + 12:
            t = ease((fr - w.closes) / 12)
            packet(im, w.center(), (W // 2, 160), t, OPS[w.op], trail=0.4, size=5)


def chatter(im, fr):
    """The Square's phrases in the words' column, a chat's bubbles."""
    for group, x0, y0, end in (((F(7), F(9)), COL_L + 6, 420, F(9)), ((F(13), F(14)), COL_L + 6, 420, F(14))):
        lines = [c for c in CHATTER if group[0] <= c[0] < group[1] and c[0] <= fr]
        if fr >= end or not lines:
            continue
        for i, (at, who, words) in enumerate(lines[-3:]):
            y = y0 + 74 * i
            bubble(im, words, (x0 + 30, y + 64), 'above', fr - at, 30, 300, 1, path=GOTHIC, name=who.upper(),
                   color=OPS[who], out=(fr - (end - 4)) if fr >= end - 4 else None)


def ghosts(im, fr):
    """Bar 16: the places of the eight to come, dashed, packets streaming in."""
    k = fr - F(16, 3)
    if not 0 <= k or fr >= F(17) + 6:
        return
    d = ImageDraw.Draw(im)
    for i, (x, y) in enumerate(EIGHT):
        if k < 3 * i:
            continue
        x0, y0, x1, y1 = x - 4, y - 24, x + 244, y + 164
        c = lerp(INK, OPS[list(OPS)[i]], 0.45)
        for xx in range(x0, x1, 14):
            d.line([(xx, y0), (min(xx + 7, x1), y0)], fill=c, width=2)
            d.line([(xx, y1), (min(xx + 7, x1), y1)], fill=c, width=2)
        for yy in range(y0, y1, 14):
            d.line([(x0, yy), (x0, min(yy + 7, y1))], fill=c, width=2)
            d.line([(x1, yy), (x1, min(yy + 7, y1))], fill=c, width=2)


def connecting(im, fr, w):
    """Bar 3's wait while the server answers: the gate's flicker held under
    a ring of packets going round."""
    k = fr - (F(3) + 18)
    if not 0 <= k < F(4) + 5 - (F(3) + 18):
        return
    if fr >= F(4):
        # (the Square scanned in from the top as the wait ends)
        line = round(w.ph * (fr - F(4)) / 5) + 2 * w.s
        im.paste(INK, (w.x, w.y + line, w.x + w.pw, w.y + w.ph))
        return
    cx, cy = w.x + w.pw // 2, w.y + w.ph // 2
    im.paste(INK, (w.x, w.y, w.x + w.pw, w.y + w.ph))
    d = ImageDraw.Draw(im)
    r = 70
    d.ellipse([cx - r, cy - r, cx + r, cy + r], outline=lerp(LINK, INK, 0.6), width=3)
    for i in range(3):
        a = fr * 0.32 + i * 2 * math.pi / 3
        px, py = cx + r * math.cos(a), cy + r * math.sin(a)
        d.rectangle([px - 6, py - 6, px + 6, py + 6], fill=WHITE if i == 0 else LINK)
    d.text((cx, cy), 'CONNECTING', font=font(MONO, 20), fill=WHITE, anchor='mm')


# ---- the frames ----

def compose(only=None):
    frames_dir = os.path.join(kit.WORK, 'frames')
    if only is None:
        shutil.rmtree(frames_dir, ignore_errors=True)
    os.makedirs(frames_dir, exist_ok=True)
    base, deep_base = field(), field(dark=0.75)
    wins, k = windows()
    hud, title, ending = Hud(), Title(), Ending()
    for fr in range(FRAMES):
        if only is not None and fr not in only:
            continue
        dark = 1.0 if F(15) <= fr < F(16, 2) else max(0.0, 1 - (fr - F(16, 2)) / 24) if F(16, 2) <= fr < F(17) else 0.0
        im = Image.blend(base, deep_base, dark) if dark else base.copy()
        ghosts(im, fr)
        links(im, fr, k, dim=dark)
        for w in wins:
            w.draw(im, fr)
        connecting(im, fr, k['gate'])
        transfers(im, fr, wins)
        title.draw(im, fr)
        for start, end, text, color in PILLS:
            if start <= fr < end:
                x = COL_L if text.startswith(('03', '06')) else COL_R
                pill(im, text, x, 96, color, fr - start)
        for w in WORDS:
            w.draw(im, fr)
        chatter(im, fr)
        if fr < F(21):
            if not F(18) <= fr < F(19):
                hud.draw(im, fr, status(fr), operators(fr), dark)
            if F(18) <= fr < F(19):
                pass
            elif F(19) <= fr:
                LOG.draw(im, fr, 94, 688)
            elif any(w.shown(fr) and w.s == 2 for w in wins):
                LOG.draw(im, fr, 102, 688)
            else:
                x = 498 if (F(7) <= fr < F(9) or F(13) <= fr < F(15)) else 62
                LOG.draw(im, fr, x, 688)
        ending.draw(im, fr)
        end = FRAMES - 20
        if fr > end:
            im = Image.blend(im, Image.new('RGB', (W, H), (0, 0, 0)), min(1.0, (fr - end) / 18))
        im.save(os.path.join(frames_dir, f'{fr:05d}.png'), compress_level=1)
        if fr % 100 == 0:
            print('frame', fr, '/', FRAMES)
