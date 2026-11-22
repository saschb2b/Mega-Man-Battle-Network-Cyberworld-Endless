"""The house look: what every trailer from 0.22.0 on is drawn with
(.claude/skills/trailer/blueprint.md). The blueprint's fixed half lives
here, so two trailers look like one series: the slots and their bars (the
piece's FORM, house_music.py), the frame (the battle net, the version in
the HP box, the section's label in the Custom gauge filling through its
segment, the panel of the segment's shots with the PET's orange cursor,
the game's picture at 3x beside it, the caption on the plate under both),
the title card, the narrator's shots (Dad in BN6's own chat box, whole
frames at 4x), the recap's tiles, the stinger and the end card, one type
(the site's Silkscreen), the project's PET colours (tools/site_art.py's).
It is 0.17.0's look, the cut the owner approved at its first render and
named as the reference with its music.

A cut (vX_Y_Z.py) gives the free half as a Trailer: its name, its accent,
its field (what moves behind the frame), its cut (how a shot arrives), its
segments and their shots, the narrator's boxes, the cold open, the recap;
and may draw a panel, a section card or a whole segment of its own.

    T = house.Trailer(version='0.22.0', name='...', music=music, accent=..., ...)
    def scenes(): return T.scenes(own)
    def compose(only=None): T.compose(only)
    POSTER, PLAY_LABEL, VERSION = T.poster, T.play_label, T.version
"""
import os
import re
import shutil
import sys

from PIL import Image, ImageDraw, ImageFont
import pil_nearest  # noqa: F401,E402  (Pillow's nearest resize kept working: tools/pil_nearest.py)

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer as kit  # noqa: E402
from trailer import ROOT  # noqa: E402

FPS, W, H = 30, 1280, 720
FONTS = os.path.join(ROOT, 'web', 'assets', 'fonts')
CAPS = os.path.join(FONTS, 'silkscreen-700.woff2')
CAPS_THIN = os.path.join(FONTS, 'silkscreen-400.woff2')
PIXEL = os.path.join(FONTS, 'pixelify-sans.woff2')
UI = os.path.join(ROOT, 'web', 'assets', 'ui')

# tools/site_art.py's colours (BN6's PET look, docs/engine/ui.md)
FRAME_HI, FRAME, FRAME_LO = (107, 222, 255), (66, 198, 231), (16, 140, 239)
FIELD = (16, 82, 107)
WHITE = (247, 255, 247)
ORANGE, ORANGE_LIT = (255, 173, 16), (255, 206, 24)
BUTTON, BUTTON_EDGE, BUTTON_SHADE = (16, 231, 41), (206, 222, 239), (41, 132, 148)
CYAN_TEXT = (107, 222, 255)
GOLD = (255, 214, 16)
SOFT = (198, 222, 239)
NET, NET_LINE, NET_DOT = (8, 30, 66), (20, 66, 118), (40, 104, 160)
PLATE, PLATE_EDGE = (16, 33, 82), (156, 206, 239)
GAUGE_IN = (8, 22, 52)

S = 3                                      # the look drawn at 427 x 240, then 3x
LW, LH = 427, 240
HP_BOX = (7, 9, 62, 24)
GAUGE = (70, 12, 419, 22)
PANEL = (7, 29, 154, 198)
ROWS = (15, 60)                            # the panel's rows: up to five, 26 apart, 20 tall
ROW_PITCH, ROW_H, ROW_MAX = 26, 20, 5
GAME = (161, 29, 419, 198)                 # its picture at (170, 34), 3x at video scale
PIC = (170, 34)
WIDE = (7, 29, 419, 198)                   # the recap's one window
PLATE_BOX = (7, 203, 419, 236)
BIG = (160, 40)                            # the big window's picture at 4x, at video scale
SAFE_X, SAFE_Y = 64, 36
OPEN = 5                                   # a shot's arrival, in frames
SLIDE = 6                                  # the panel's slide in, in frames

SYSTEMS = ('PortMaster handhelds · Linux · Windows · macOS · Android · the browser',
           'iPhone and iPad · New 3DS · Switch · PS Vita · PSP · PS4 · PS5')
SITE = 'saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless'
FOOT = ('Needs your own Mega Man Battle Network 6: Cybeast Gregar (USA) ROM.',
        "BN5's and BN4's own need Team Colonel (USA) and Red Sun (USA) beside it.",
        'An unofficial fan project, not affiliated with Capcom.')
THANKS = 'THANKS FOR DIVING IN'

# BN6's chat box: three lines of 20 letters (src/layer/text.h, ta_wrap)
BOX_LINES, BOX_WIDTH = 3, 20


# ---- drawing: type, colour, the PET's pieces ----

_fonts = {}


def font(size, bold=False):
    k = (size, bold)
    if k not in _fonts:
        _fonts[k] = ImageFont.truetype(CAPS if bold else CAPS_THIN, size)
    return _fonts[k]


CLOSED = '4%'                              # Silkscreen Bold's 4 and % close up into blobs ("04" read as a heart)


def runs(words):
    """the words in runs of (closed, letters): the bold face's closed
    glyphs apart from the rest"""
    out = []
    for c in words:
        if out and out[-1][0] == (c in CLOSED):
            out[-1][1] += c
        else:
            out.append([c in CLOSED, c])
    return out


def put(d, xy, words, size, fill, bold, anchor):
    """the words at xy; in bold, a 4 or a % from the regular face struck
    twice, a little apart, so it stays open"""
    if not bold or not any(c in CLOSED for c in words):
        d.text(xy, words, font=font(size, bold), fill=fill, anchor=anchor)
        return
    parts = runs(words)
    widths = [font(size, not closed).getlength(part) + (size // 8 if closed else 0) for closed, part in parts]
    x = xy[0] - {'l': 0, 'm': sum(widths) / 2, 'r': sum(widths)}[anchor[0]]
    for (closed, part), w in zip(parts, widths):
        if closed:
            for dx in (size // 16, size // 16 + max(1, size // 14)):
                d.text((x + dx, xy[1]), part, font=font(size, False), fill=fill, anchor='l' + anchor[1])
        else:
            d.text((x, xy[1]), part, font=font(size, True), fill=fill, anchor='l' + anchor[1])
        x += w


def text(d, xy, words, size, fill, bold=False, anchor='la', shade=None, width=None):
    """Silkscreen's capitals, shrunk to `width` where given"""
    while width and size > 12 and font(size, bold).getlength(words) > width:
        size -= 1
    if shade:
        put(d, (xy[0] + 3, xy[1] + 3), words, size, shade, bold, anchor)
    put(d, xy, words, size, fill, bold, anchor)


def address(d, cx, cy, words, fill):
    """the site's address in Pixelify Sans (its case matters), its B and C
    Silkscreen's (0.16.0's lesson: Pixelify's B reads as a G)"""
    p = ImageFont.truetype(PIXEL, 26)
    k = ImageFont.truetype(CAPS_THIN, 27)
    pb, kb = p.getbbox('B'), k.getbbox('B')
    widths = [k.getlength(c) if c in 'BC' else p.getlength(c) for c in words]
    x = cx - sum(widths) / 2
    y = cy - (pb[1] + pb[3]) / 2
    for c, w in zip(words, widths):
        if c in 'BC':
            d.text((x, y + pb[3] - kb[3]), c, font=k, fill=fill)
        else:
            d.text((x, y), c, font=p, fill=fill, features=['-liga'])
        x += w


def lerp(a, b, t):
    t = max(0.0, min(1.0, t))
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def ease(t):
    t = max(0.0, min(1.0, t))
    return 1 - (1 - t) ** 3


_icons = {}


def icon(name, scale=1):
    """one of the site's icons (web/assets/ui), our own drawing"""
    k = (name, scale)
    if k not in _icons:
        im = Image.open(os.path.join(UI, name + '.png')).convert('RGBA')
        _icons[k] = im.resize((im.width * scale, im.height * scale), Image.NEAREST) if scale > 1 else im
    return _icons[k]


def net(fr):
    """the battle's net: a navy field, a cyan grid drifting down and right
    a pixel every other frame, a dot where its lines cross"""
    im = Image.new('RGB', (LW, LH), NET)
    d = ImageDraw.Draw(im)
    o = (fr // 2) % 16
    for x in range(-16 + o, LW, 16):
        d.line([(x, 0), (x, LH)], fill=NET_LINE)
    for y in range(-16 + o, LH, 16):
        d.line([(0, y), (LW, y)], fill=NET_LINE)
    for x in range(-16 + o, LW, 16):
        for y in range(-16 + o, LH, 16):
            d.point((x, y), fill=NET_DOT)
    return im


def window(d, box, k=1, lit=0.0):
    """the PET's cyan window round a navy field; k its pixel's size (3 at
    video scale); `lit` (0-1) its frame flashed"""
    x0, y0, x1, y1 = box
    hi, mid, lo = (lerp(c, (255, 255, 255), lit) for c in (FRAME_HI, FRAME, FRAME_LO))
    d.rectangle([x0, y0, x1, y1], fill=lo)
    d.rectangle([x0, y0, x1 - k, y1 - k], fill=hi)
    d.rectangle([x0 + k, y0 + k, x1 - k, y1 - k], fill=mid)
    d.rectangle([x0 + 4 * k, y0 + 4 * k, x1 - 4 * k, y1 - 4 * k], fill=lo)
    d.rectangle([x0 + 5 * k, y0 + 5 * k, x1 - 5 * k, y1 - 5 * k], fill=FIELD)


def plate(d, box, edge=PLATE_EDGE):
    """BN's plate (the HP box, a used chip's name): navy in a light edge"""
    x0, y0, x1, y1 = box
    d.rectangle([x0, y0, x1, y1], fill=edge)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=PLATE)
    d.line([(x0 + 2, y1 - 2), (x1 - 2, y1 - 2)], fill=GAUGE_IN)


def gauge(d, share, fr, colour):
    """the Custom gauge: its fill the cut's accent, gold and flashing when
    full"""
    x0, y0, x1, y1 = GAUGE
    d.rectangle([x0, y0, x1, y1], fill=PLATE_EDGE)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=GAUGE_IN)
    if share > 0:
        full = share >= 1
        c = (GOLD if (fr // 4) % 2 else ORANGE) if full else colour
        xe = x0 + 2 + round((x1 - x0 - 4) * min(1.0, share))
        d.rectangle([x0 + 2, y0 + 2, xe, y1 - 2], fill=c)
        d.line([(x0 + 2, y0 + 2), (xe, y0 + 2)], fill=WHITE if full else lerp(c, WHITE, 0.5))


def cursor(d, x, y, fr):
    """the PET's orange cursor arrow, its point at (x, y), nodding a pixel"""
    x += (fr // 8) % 2
    d.polygon([(x - 5, y - 4), (x, y), (x - 5, y + 4)], fill=ORANGE)
    d.line([(x - 5, y - 3), (x - 1, y)], fill=ORANGE_LIT)


_logo = {}


def logo(width):
    if width not in _logo:
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        _logo[width] = lg.resize((width, round(lg.height * width / lg.width)), Image.LANCZOS)
    return _logo[width]


def pic(scene, i):
    """a scene's frame; a card of our own where its footage is not taken
    yet (0.17.0's way: the cut is built on stand-ins, each swapped for its
    run as it lands)"""
    if scene != 'still' and not os.path.isdir(os.path.join(kit.WORK, scene)):
        im = Image.new('RGB', (240, 160), (30, 34, 60))
        d = ImageDraw.Draw(im)
        d.rectangle([4, 4, 235, 155], outline=(90, 100, 150))
        d.text((120, 70), 'FOOTAGE TO COME', font=font(12, True), fill=(160, 170, 210), anchor='mm')
        d.text((120, 90), str(scene).upper(), font=font(8), fill=(120, 130, 170), anchor='mm')
        return im
    return kit.shot(scene, i)


# ---- the narrator: Dad in BN6's own chat box ----

def wrap(box):
    """a box's lines as BN6's chat box breaks them (20 letters a line)"""
    lines, line = [], ''
    for word in box.split(' '):
        if line and len(line) + 1 + len(word) > BOX_WIDTH:
            lines.append(line)
            line = word
        else:
            line = (line + ' ' + word).strip()
    return lines + [line]


class Narration:
    """The narrator's boxes for one slot: `boxes` [(beats from the slot's
    start, the box's words)], said by Dad in BN6's own chat box (--talk
    narrate, src/director/director_dev.c) on a layer of its own, a scene
    of whole frames. The game types a letter every two frames; the cut
    plays the typing at twice its pace and then the box as it stands,
    each box from its beat. A chat stops the game under it, so what moves
    behind him is `backdrop`, other footage his box is laid over
    (frame())."""

    AT, HOLD, PAGED = 700, 260, 648        # the talk's frame, a box's frames, the arrival's words paged by

    def __init__(self, scene, boxes, backdrop=None, seed=7, area=None):
        self.scene, self.boxes, self.backdrop, self.seed, self.area = scene, boxes, backdrop, seed, area
        self._mask = None
        for _, words in boxes:
            lines = wrap(words)
            if len(lines) > BOX_LINES or max(map(len, lines)) > BOX_WIDTH:
                raise SystemExit(f'narrator: "{words}" does not fit a chat box ({BOX_LINES} lines of {BOX_WIDTH}): {lines}')

    def run(self):
        """the scene: (argv, env, script, first, last)"""
        n = len(self.boxes)
        argv = ['--scene', 'emu', '--seed', str(self.seed), '--dev', 'quiet', '--talk', f'narrate:{self.AT}']
        if self.area is not None:
            argv += ['--net-biome', str(self.area)]
        script = '200:,' + '6:A,50:,' * 8 + f'{self.AT + self.HOLD - self.PAGED}:,' + f'6:A,{self.HOLD - 6}:,' * (n - 1)
        env = {'CYBERWORLD_NARRATE': '|'.join(w for _, w in self.boxes)}
        return (argv, env, script, self.AT - 20, self.AT + n * self.HOLD - 8)

    def mask(self):
        """the chat box's place in the scene's frames: where a frame with
        the box open differs from one before it opened, row by row from
        its leftmost such pixel to its rightmost (its rounded corners
        kept), in the frame's lower half"""
        if self._mask is None:
            before = pic(self.scene, 4).convert('RGB')
            opened = pic(self.scene, (20 + 6) // 2 + 30).convert('RGB')
            m = Image.new('L', (240, 160), 0)
            a, b = before.load(), opened.load()
            for y in range(84, 160):
                xs = [x for x in range(240) if a[x, y] != b[x, y]]
                if len(xs) >= 120:
                    ImageDraw.Draw(m).line([(xs[0], y), (xs[-1], y)], fill=255)
            self._mask = m
        return self._mask

    def frame(self, k, beat):
        """the slot's frame k: the narrator's box, and behind it the
        backdrop's footage where the cut names one (`backdrop`, a feed
        [(beats, scene, sample, rate)]): the box's rows of his frame laid
        over the backdrop's frame, both whole frames of the game, so the
        picture moves while he speaks"""
        box = pic(self.scene, self.sample(k, beat))
        if not self.backdrop or not os.path.isdir(os.path.join(kit.WORK, self.scene)):
            return box
        feed = [(round(b * beat), sc, first, rate) for b, sc, first, rate in self.backdrop]
        start, scene, first, rate = max((e for e in feed if e[0] <= k), default=feed[0])
        out = pic(scene, first if scene == 'still' else first + max(0, int((k - start) * rate))).copy()
        out.paste(box, (0, 0), self.mask())
        return out

    def sample(self, k, beat):
        """the sample (every second game frame) at slot frame k, a beat
        `beat` frames"""
        i = max(j for j, (beats, _) in enumerate(self.boxes) if beats * beat <= k or j == 0)
        beats, words = self.boxes[i]
        k -= round(beats * beat)
        first = (20 + i * self.HOLD + 6) // 2
        typed = len(words) + 8
        last = (20 + (i + 1) * self.HOLD) // 2 - 6
        if k < 0:
            return max(0, first + 2 * k) if i == 0 else first
        return first + 2 * k if 2 * k < typed else min(last, first + typed + (k - typed // 2))


# ---- the timeline ----

class Shot:
    """A shot of a segment, `bars` long (1): its `tag` (a few words, the
    panel's row), its caption on the plate (one concrete line, 40 letters
    at most), a count where the release gives one, and its `feed`
    [(beats from the shot's start, scene, sample, rate)] (from each beat
    on, that scene's footage from that sample; rate 0 holds it; a scene
    'still' is a screenshot by name)."""

    def __init__(self, tag, caption, feed, count='', bars=1):
        self.tag, self.caption, self.feed, self.count, self.bars = tag, caption, feed, count, bars
        self.start = 0

    def sample(self, fr, beat):
        feed = [(self.start + round(b * beat), sc, first, rate) for b, sc, first, rate in self.feed]
        start, scene, first, rate = max((e for e in feed if e[0] <= fr), default=feed[0])
        if scene == 'still':
            return scene, first
        return scene, first + max(0, int((fr - start) * rate))


class Segment:
    """A feature: its label (the gauge's, fixed through the segment), its
    shots, an icon of the site's for its card (web/assets/ui) and, for a
    segment that draws itself, `draw(T, small, d, seg, shot, fr)` in place
    of the panel."""

    def __init__(self, label, shots, icon=None, draw=None):
        self.label, self.shots, self.icon, self.draw = label, shots, icon, draw
        self.card, self.number, self.bars = 0, '', 1 + sum(s.bars for s in shots)


class Tile:
    """One of the recap's four: a picture (scene, sample) and two lines"""

    def __init__(self, scene, sample, lines):
        self.scene, self.sample, self.lines = scene, sample, lines


def changelog(version):
    """(entries, fixes) of the CHANGELOG's section for `version`, or of
    Unreleased before its release: a count a cut says is counted here"""
    with open(os.path.join(ROOT, 'CHANGELOG.md'), encoding='utf-8') as f:
        text_ = f.read()
    parts = re.split(r'^## ', text_, flags=re.M)
    body = next((p for p in parts if p.startswith(version)), None) or next(p for p in parts if p.startswith('Unreleased'))
    entries = re.findall(r'^- \*\*(.*?)\*\*', body, flags=re.M | re.S)
    return len(entries), sum(1 for e in entries if e.startswith('Fixed'))


class Trailer:
    """A cut on the blueprint. `music` the piece (its FORM the slots);
    `cold` the cold open's feed [(beats, scene, sample, rate)]; `hello` and
    `thanks` the narrator's Narrations; `segments`; `recap` four Tiles and
    `recap_caption` (words, count); `accent` the gauge's fill and the
    numbers' colour; `field(fr)` the 427 x 240 picture behind the frame
    (the net by default); `cut` how a shot arrives: 'open' (out of its
    row in the panel), 'sweep' (drawn in from the left under a bright
    line), 'hard', or a function (small, old, new, t) of its own."""

    def __init__(self, version, name, music, cold, hello, thanks, segments, recap, recap_caption,
                 accent=FRAME, field=net, cut='open', systems=SYSTEMS, foot=FOOT, thanks_line=THANKS):
        self.version, self.name, self.music = version, name, music
        self.cold, self.hello, self.thanks, self.segments = cold, hello, thanks, segments
        self.recap, self.recap_caption = recap, recap_caption
        self.accent, self.field, self.cut = accent, field, cut
        self.systems, self.foot, self.thanks_line = systems, foot, thanks_line
        self.bar = round(FPS * 60 * 4 / music.BPM)
        self.beat = self.bar // 4
        self.frames = music.BARS * self.bar + round(music.TAIL * FPS)
        slots = {}
        segs = iter(segments)
        for slot, at, n in music.FORM:
            if slot == 'segment':
                seg = next(segs)
                if seg.bars != n:
                    raise SystemExit(f'segment "{seg.label}" is {seg.bars} bars, the piece gives it {n}')
                seg.card, seg.number = self.F(at), f'{segments.index(seg) + 1:02d}'
                t = at + 1
                for sh in seg.shots:
                    sh.start = self.F(t)
                    t += sh.bars
            else:
                slots[slot] = (self.F(at), self.F(at + n))
        if next(segs, None) is not None:
            raise SystemExit('more segments than the piece has')
        self.slots = slots
        self.poster = slots['title'][1] - self.beat
        self.play_label = f'WATCH THE {version} TRAILER'

    def F(self, bar, beat=0.0):
        return round(bar * self.bar + beat * self.beat)

    def scenes(self, own):
        """the cut's scenes with the narrator's two"""
        return dict(own, **{n.scene: n.run() for n in (self.hello, self.thanks)})

    # ---- the frame's kinds ----

    def base(self, fr):
        return self.field(fr).copy()

    def big_frame(self, fr):
        """the field at 3x and one big window, its picture's place at 4x"""
        im = self.base(fr).resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
        d = ImageDraw.Draw(im)
        window(d, (BIG[0] - 15, BIG[1] - 15, BIG[0] + 960 + 14, BIG[1] + 640 + 14), 3)
        return im, d

    def draw_cold(self, fr):
        im, _ = self.big_frame(fr)
        feed = [(round(b * self.beat), sc, first, rate) for b, sc, first, rate in self.cold]
        start, scene, first, rate = max(e for e in feed if e[0] <= fr)
        im.paste(pic(scene, first + int((fr - start) * rate)).resize((960, 640), Image.NEAREST), BIG)
        return im

    def draw_title(self, fr):
        """the logo, the release's name and the version on BN's plate"""
        im, d = self.big_frame(fr)
        k = fr - self.slots['title'][0]
        lg = logo(620)
        im.paste(lg, (W // 2 - 310, 120), lg)
        if k >= self.beat:
            a = min(1.0, (k - self.beat + 1) / 4)
            text(d, (W // 2, 120 + lg.height + 70), self.name, 56, lerp(FIELD, WHITE, a), True, 'mm', shade=GAUGE_IN,
                 width=900)
        if k >= 2 * self.beat:
            x0, x1, y0, y1 = 330, 950, 440, 526
            d.rectangle([x0, y0, x1, y1], fill=PLATE_EDGE)
            d.rectangle([x0 + 3, y0 + 3, x1 - 3, y1 - 3], fill=PLATE)
            d.rectangle([x0 + 6, y1 - 9, x1 - 6, y1 - 6], fill=GAUGE_IN)
            text(d, (W // 2, (y0 + y1) / 2), f'{self.version} · OUT NOW', 44, GOLD, True, 'mm', shade=GAUGE_IN,
                 width=x1 - x0 - 60)
        return im

    def draw_narrator(self, fr, slot, narration):
        """Dad on the line: his box in the game's own frame at 4x"""
        im, _ = self.big_frame(fr)
        k = fr - self.slots[slot][0]
        im.paste(narration.frame(k, self.beat).resize((960, 640), Image.NEAREST), BIG)
        return im

    def where(self, fr):
        """(the segment, its shot or None on its card)"""
        seg = None
        for s in self.segments:
            if s.card <= fr:
                seg = s
        shot = None
        for sh in seg.shots:
            if sh.start <= fr:
                shot = sh
        return seg, shot

    def row_box(self, i, dx=0):
        x, y = ROWS[0] + dx, ROWS[1] + ROW_PITCH * i
        return (x, y, PANEL[2] - 8 + dx, y + ROW_H)

    def panel(self, small, d, seg, shot, fr, dx):
        """the segment's panel: a row a shot, the cursor on the one
        shown, the ones shown lit; on the card the rows arrive one a beat"""
        x0, y0, x1, y1 = PANEL
        window(d, (x0 + dx, y0, x1 + dx, y1))
        now = seg.shots.index(shot) if shot else -1
        card = fr - seg.card
        for i, sh in enumerate(seg.shots[:ROW_MAX]):
            if shot is None and card < (i + 1) * self.bar // (len(seg.shots) + 1):
                continue
            box = self.row_box(i, dx)
            plate(d, box, ORANGE if i == now else PLATE_EDGE)
            if i < now:
                d.rectangle([box[2] - 12, box[1] + 7, box[2] - 6, box[3] - 7], fill=BUTTON)
            if i == now:
                cursor(d, box[0] - 1, (box[1] + box[3]) // 2, fr)

    def panel_words(self, d, seg, shot, fr, dx):
        x = dx * S
        text(d, (PANEL[0] * S + 24 + x, (PANEL[1] + 10) * S), f'IN {self.version}', 18, CYAN_TEXT, True, 'lm')
        text(d, (PANEL[0] * S + 24 + x, (PANEL[1] + 22) * S), seg.number, 34, self.accent, True, 'lm', shade=GAUGE_IN)
        now = seg.shots.index(shot) if shot else -1
        card = fr - seg.card
        for i, sh in enumerate(seg.shots[:ROW_MAX]):
            if shot is None and card < (i + 1) * self.bar // (len(seg.shots) + 1):
                continue
            box = self.row_box(i, dx)
            fill = WHITE if i == now or shot is None else SOFT if i < now else lerp(PLATE, SOFT, 0.45)
            text(d, (box[0] * S + 22, (box[1] + box[3]) * S / 2 + 1), sh.tag, 20, fill, True, 'lm',
                 width=(box[2] - box[0]) * S - (70 if i < now else 40))

    def arrive(self, small, old, new, fr, shot, seg):
        """the cut: the new picture's arrival over OPEN frames"""
        t = ease((fr - shot.start + 1) / OPEN)
        if old is not None:
            small.paste(old, PIC)
        if callable(self.cut):
            self.cut(small, old, new, t)
        elif self.cut == 'sweep' and t < 1:
            w = max(1, round(240 * t))
            small.paste(new.crop((0, 0, w, 160)), PIC)
            ImageDraw.Draw(small).line([(PIC[0] + w, PIC[1]), (PIC[0] + w, PIC[1] + 159)], fill=WHITE)
        elif self.cut == 'open' and t < 1:
            a = self.row_box(seg.shots.index(shot))
            x0 = round(a[0] + (PIC[0] - a[0]) * t)
            y0 = round(a[1] + (PIC[1] - a[1]) * t)
            w = round((a[2] - a[0]) + (240 - (a[2] - a[0])) * t)
            h = round((a[3] - a[1]) + (160 - (a[3] - a[1])) * t)
            small.paste(new.resize((max(1, w), max(1, h)), Image.NEAREST), (x0, y0))
        else:
            small.paste(new, PIC)

    def header(self, d, im_d, label, share, fr):
        """(on the look's picture d) the HP box and the gauge; (on the
        video's im_d, None on the first pass) their words"""
        if im_d is None:
            plate(d, HP_BOX)
            gauge(d, share, fr, self.accent)
        else:
            text(im_d, ((HP_BOX[0] + HP_BOX[2]) * S / 2, (HP_BOX[1] + HP_BOX[3]) * S / 2), self.version, 30, WHITE, True, 'mm')
            text(im_d, (GAUGE[0] * S + 14, (GAUGE[1] + GAUGE[3]) * S / 2 + 1), label, 22, WHITE, True, 'lm', shade=GAUGE_IN)

    def caption(self, d, words, count=''):
        """the plate's line, and its count in gold at the right, the
        line kept clear of it"""
        cy = (PLATE_BOX[1] + PLATE_BOX[3]) * S / 2
        room = (PLATE_BOX[2] - PLATE_BOX[0]) * S - 60
        if count:
            room -= font(48, True).getlength(count) + 40
            text(d, (PLATE_BOX[2] * S - 30, cy), count, 48, GOLD, True, 'rm')
        text(d, (PLATE_BOX[0] * S + 30, cy), words, 40, WHITE, True, 'lm', width=room)

    def draw_segment(self, fr):
        small = self.base(fr)
        d = ImageDraw.Draw(small)
        seg, shot = self.where(fr)
        first = self.segments[0].card
        dx = -round((PANEL[2] + 4) * (1 - ease((fr - first + 1) / SLIDE))) if fr < first + SLIDE else 0
        span = seg.bars * self.bar - self.beat
        self.header(d, None, '', (fr - seg.card) / span, fr)
        window(d, GAME)
        if shot is None and seg.icon:
            ic = icon(seg.icon, 6)
            small.paste(ic, ((GAME[0] + GAME[2] - ic.width) // 2, GAME[1] + 20), ic)
        elif shot is not None:
            new = pic(*shot.sample(fr, self.beat))
            prev = seg.shots.index(shot) - 1
            if fr - shot.start < OPEN:
                old = pic(*seg.shots[prev].sample(shot.start - 1, self.beat)) if prev >= 0 else None
                self.arrive(small, old, new, fr, shot, seg)
            else:
                small.paste(new, PIC)
        if seg.draw:
            seg.draw(self, small, d, seg, shot, fr)
        elif dx > -(PANEL[2] + 4):
            self.panel(small, d, seg, shot, fr, dx)
        plate(d, PLATE_BOX)
        im = small.resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
        d = ImageDraw.Draw(im)
        self.header(None, d, f'{seg.number} · {seg.label}', 0, fr)
        if not seg.draw and dx > -(PANEL[2] + 4):
            self.panel_words(d, seg, shot, fr, dx)
        cy = (PLATE_BOX[1] + PLATE_BOX[3]) * S / 2
        if shot is None:
            gx = (GAME[0] + GAME[2]) * S / 2
            a = min(1.0, (fr - seg.card + 1) / 4)
            y = (GAME[1] + (105 if seg.icon else 70)) * S
            text(d, (gx, y), seg.number, 40, lerp(FIELD, self.accent, a), True, 'mm')
            text(d, (gx, y + 25 * S), seg.label, 44, lerp(FIELD, WHITE, a), True, 'mm', shade=GAUGE_IN,
                 width=(GAME[2] - GAME[0] - 24) * S)
        else:
            self.caption(d, shot.caption, shot.count)
        return im

    def draw_recap(self, fr):
        """the rest of the release: four tiles at 2x, two a bar, the second
        of each bar on its third beat; the caption and its count"""
        start, end = self.slots['recap']
        small = self.base(fr)
        d = ImageDraw.Draw(small)
        self.header(d, None, '', (fr - start) / (end - start - self.beat), fr)
        window(d, WIDE)
        plate(d, PLATE_BOX)
        im = small.resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
        d = ImageDraw.Draw(im)
        self.header(None, d, f'{len(self.segments) + 1:02d} · ALSO IN {self.version}', 0, fr)
        pair = 2 * ((fr - start) // self.bar)
        for j, tile in enumerate(self.recap[pair:pair + 2]):
            if j and (fr - start) % self.bar < 2 * self.beat:
                continue
            x, y = 130 + j * 540, 118
            d.rectangle([x - 4, y - 4, x + 483, y + 323], fill=FRAME_HI)
            im.paste(pic(tile.scene, tile.sample).resize((480, 320), Image.NEAREST), (x, y))
            for i, line in enumerate(tile.lines):
                text(d, (x + 240, y + 320 + 40 + 36 * i), line, 26, WHITE, True, 'mm', shade=GAUGE_IN, width=500)
        self.caption(d, *self.recap_caption)
        return im

    def draw_stinger(self, fr):
        """the cadence's downbeat: a flash, then the logo and the version"""
        im, d = self.big_frame(fr)
        lg = logo(700)
        im.paste(lg, (W // 2 - 350, 190), lg)
        text(d, (W // 2, 190 + lg.height + 70), self.version, 64, GOLD, True, 'mm', shade=GAUGE_IN)
        k = fr - self.slots['stinger'][0]
        if k < 6:
            return Image.blend(im, Image.new('RGB', (W, H), WHITE), 1 - k / 6)
        return im

    def draw_end(self, fr):
        """the end card, held still: the logo, the thanks, the version,
        where to play it, the site and the ROM's lines"""
        end = self.slots['end'][0]
        small = self.field(end).copy()
        d = ImageDraw.Draw(small)
        window(d, (40, 8, 386, 232))
        im = small.resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
        d = ImageDraw.Draw(im)
        lg = logo(460)
        y = 64
        im.paste(lg, (W // 2 - 230, y), lg)
        y += lg.height + 46
        text(d, (W // 2, y), self.thanks_line, 44, WHITE, True, 'mm', shade=GAUGE_IN, width=900)
        y += 44
        x0, x1 = 400, 880
        d.rectangle([x0, y, x1, y + 66], fill=PLATE_EDGE)
        d.rectangle([x0 + 3, y + 3, x1 - 3, y + 63], fill=PLATE)
        text(d, (W // 2, y + 33), f'{self.version} · OUT NOW', 34, GOLD, True, 'mm', shade=GAUGE_IN, width=x1 - x0 - 40)
        y += 66 + 40
        for i, line in enumerate(self.systems):
            text(d, (W // 2, y + 34 * i), line, 22, WHITE, False, 'mm', width=940)
        address(d, W // 2, y + 34 * len(self.systems) + 22, SITE, CYAN_TEXT)
        for i, line in enumerate(self.foot):
            text(d, (W // 2, y + 34 * len(self.systems) + 68 + 26 * i), line, 16, SOFT, False, 'mm')
        return im

    # ---- the frame ----

    def draw(self, fr):
        s = self.slots
        if fr < s['title'][0]:
            return self.draw_cold(fr)
        if fr < s['title'][1]:
            return self.draw_title(fr)
        if 'hello' in s and fr < s['hello'][1]:
            return self.draw_narrator(fr, 'hello', self.hello)
        if fr < s['recap'][0]:
            return self.draw_segment(fr)
        if fr < s['recap'][1]:
            return self.draw_recap(fr)
        if 'thanks' in s and fr < s['thanks'][1]:
            return self.draw_narrator(fr, 'thanks', self.thanks)
        if fr < s['end'][0]:
            return self.draw_stinger(fr)
        return self.draw_end(fr)

    def compose(self, only=None):
        frames_dir = os.path.join(kit.WORK, 'frames')
        if only is None:
            shutil.rmtree(frames_dir, ignore_errors=True)
        os.makedirs(frames_dir, exist_ok=True)
        for fr in range(self.frames):
            if only is not None and fr not in only:
                continue
            self.draw(fr).save(os.path.join(frames_dir, f'{fr:05d}.png'), compress_level=1)
            if fr % 100 == 0:
                print('frame', fr, '/', self.frames, flush=True)
