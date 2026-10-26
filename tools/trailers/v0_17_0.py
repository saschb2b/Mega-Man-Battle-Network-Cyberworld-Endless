"""0.17.0's trailer, THE RUN, SHARPENED: the run cut to its battles and
the picks between them (CHANGELOG.md's 0.17.0, from the course check of
26 October 2026, docs/game/core-loop.md), in the skill's structure
(SKILL.md, The structure): a cold open of two bars with no words, the
title card, four segments (a section card of a bar, then a shot a bar,
each one concrete thing), the recap of the rest, the stinger on the
music's cadence and the end card held.

The idea: the trailer is one virus battle. The music is v0_17_0_music.py,
an original piece in the manner of BN5's "Virus Busting" (song 0x15), the
song every layer now plays, grown from the sketch the owner chose
(sketch_bust_a, "Busting"). The look is a battle's own screens in the
project's PET style (tools/site_art.py's colours, its icons, the site's
Silkscreen), as the stills the owner chose show it:
- The frame is the battle's net, a navy field under a cyan grid drifting
  as a battle's background does. The HP box top left holds the version;
  the Custom gauge across the top holds the section's label (one place,
  small capitals) and fills through the segment, full and flashing on its
  last beat.
- A segment is the Custom screen beside the field: its feature is one of
  five battle chips (the site's icons as their art, a name, a code; the
  codes S, H, A, R and P), the CHIP SELECT row's orange brackets step onto
  it on its section card, OK lights and the chip goes into the column. On
  the card the field shows the chip large with the section's number and
  name; each shot opens out of the chip's window into the field (the
  cut). The caption sits on the plate under both, where BN draws a used
  chip's name; a count, where the CHANGELOG gives one, sits where its
  damage would.
- The Custom screen slides in from the left as BN's does when the title
  gives way to the first segment, and out again for the recap.
- The stinger is a Program Advance: the five chips in code order, S H A
  R P, close up and flash into the logo and the version.
- The end card is BN's results: DELETE TIME (the cut's own length),
  BUSTING LEVEL S, GET DATA 0.17.0 · OUT NOW, the systems, the site and the
  ROM lines, held still.
Silkscreen's S reads as its 5 at these sizes (and BN's busting levels run
1 to 10 and S), so the codes and the rank are drawn from letters of our
own (GLYPHS).

    python3 build.py host && python3 tools/trailer.py 0.17.0 [--keep] [--frames A,B]
"""
import os
import shutil
import sys

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer as kit  # noqa: E402
from trailer import ROOT  # noqa: E402
from trailers import v0_17_0_music as music  # noqa: E402

VERSION = '0.17.0'
PLAY_LABEL = 'WATCH THE 0.17.0 TRAILER'
BN6_ONLY = ()


# ---- the footage ----

def from_compare(name, first, last):
    """a scene of tools/before_after.py's, this build's side: its run, its
    input and its environment, game frames first to last"""
    import before_after
    e = next(x for x in before_after.SCENES if x[0] == name)
    argv = list(e[1])
    script = None
    if '--input' in argv:
        i = argv.index('--input')
        script = argv[i + 1]
        del argv[i:i + 2]
    return (argv, dict(e[2]), script, first, last)


# (the media branch's recipes for 0.17.0's notes, until build.py on the
# release branch has them: MegaMan's read by seed 30's layer 4's exit pad
# beside a dark warp and at Lan's HP's pink pad on seed 7; a won Server's
# offer and pick on seed 8's layer 4)
FORK_PAD = (['--scene', 'emu', '--run-depth', '4', '--seed', '30', '--dev', 'quiet'], {}, '300:,0:place 284 -300 1,110:')
SERVER_8 = ['--scene', 'emu', '--run-depth', '4', '--seed', '8', '--dev', 'god,onehit,quiet', '--talk', 'challenge:200']
SERVER_SHOTS = '6:B,6:,12:,' * 6
SERVER_PICK = ('1:,300:,' + '6:A,6:,70:,' * 3 + '6:A,6:,90:,6:LEFT,6:,10:,6:A,6:,400:,6:START,6:,10:,6:A,6:,120:,' + SERVER_SHOTS +
               '8:UP,15:,' + SERVER_SHOTS + '8:DOWN,15:,8:DOWN,15:,' + SERVER_SHOTS + '200:,60:,' + '6:A,6:,60:,' * 2 +
               '6:A,6:,200:,' + '6:A,6:,90:,' * 2)


def scenes():
    """the release pictures' runs (build.py's SCREENSHOTS and CLIPS,
    tools/before_after.py's), a scene each"""
    import build
    return {
        'fork-pad': (*FORK_PAD, 330, 470),
        # an act's first layer, seed 1's layer 4: the card, its two boxes
        'arrival': from_compare('arrival-words', 120, 420),
        'fork-portal': (['--scene', 'home', '--run-depth', '7', '--seed', '7'], {},
                        build.HP_PAGED + '0:place 160 37 0,20:,6:A,100:,6:A,90:,6:A,90:', 1380, 1560),
        'server-offer': (SERVER_8, {}, SERVER_PICK, 440, 620),
        'server-pick': (SERVER_8, {}, SERVER_PICK, 2380, 2640),
        # (its battle, the buster down each row: the cold open's, until the
        # battle floor's run)
        'server-battle': (SERVER_8, {}, SERVER_PICK, 1050, 1750),
        # the setup's Folder row stepping to Z-Canon (245), Busting (285)
        # and Lean (325); Z-Canon's and Busting's first hands
        'setup-folders': kit.from_shots('setup-folders', 200, 360),
        'hand-zcanon': kit.from_shots('hand-zcanon', 940, 1100),
        'hand-busting': kit.from_shots('hand-busting', 940, 1100),
        # ProtoMan's netbattle on seed 7's layer 8: his blow stopped,
        # MegaMan on the layer at 1 HP, Chaud's word (3000)
        'protoman': from_compare('protoman-netbattle', 2200, 3100),
    }


# ---- the look ----

FPS, W, H = 30, 1280, 720
BAR = round(FPS * 60 * 4 / music.BPM)     # (48 frames at 150 beats a minute)
BEAT = BAR // 4
FRAMES = music.BARS * BAR + round(music.TAIL * FPS)

FONTS = os.path.join(ROOT, 'web', 'assets', 'fonts')
CAPS = os.path.join(FONTS, 'silkscreen-700.woff2')
CAPS_THIN = os.path.join(FONTS, 'silkscreen-400.woff2')
PIXEL = os.path.join(FONTS, 'pixelify-sans.woff2')
UI = os.path.join(ROOT, 'web', 'assets', 'ui')

# tools/site_art.py's colours, as 0.16.0's cut took them
FRAME_HI, FRAME, FRAME_LO = (107, 222, 255), (66, 198, 231), (16, 140, 239)
FIELD = (16, 82, 107)
WHITE = (247, 255, 247)
ORANGE, ORANGE_LIT = (255, 173, 16), (255, 206, 24)
BUTTON, BUTTON_EDGE, BUTTON_SHADE = (16, 231, 41), (206, 222, 239), (41, 132, 148)
CYAN_TEXT = (107, 222, 255)
GOLD = (255, 214, 16)
SOFT = (198, 222, 239)
# the battle's net under it, and BN's plates (the HP box, a chip's name)
NET, NET_LINE, NET_DOT = (8, 30, 66), (20, 66, 118), (40, 104, 160)
PLATE, PLATE_EDGE = (16, 33, 82), (156, 206, 239)
GAUGE_IN = (8, 22, 52)

SAFE_X, SAFE_Y = 64, 36
S = 3                                      # the look drawn at 427 x 240, then 3x
LW, LH = 427, 240
HP_BOX = (7, 9, 62, 24)
GAUGE = (70, 12, 419, 22)
CUSTOM = (7, 29, 154, 198)
ART = (15, 47, 115, 107)
NAME = (15, 110, 115, 122)
COLUMN = (125, 47)                         # five slots, 19 apart
ROW = (15, 141)                            # CHIP SELECT: five slots of 16, 19 apart
PITCH = 19
OK = (113, 162, 147, 190)
GAME = (161, 29, 419, 198)                 # its picture at (170, 34)
PIC = (170, 34)
PLATE_BOX = (7, 203, 419, 236)
BIG = (160, 40)                            # the big window's picture at 4x, at video scale


def F(bar, beat=0.0):
    return round(bar * BAR + beat * BEAT)


_fonts = {}


def font(size, bold=False):
    k = (size, bold)
    if k not in _fonts:
        _fonts[k] = ImageFont.truetype(CAPS if bold else CAPS_THIN, size)
    return _fonts[k]


def text(d, xy, words, size, fill, bold=False, anchor='la', shade=None, width=None):
    """Silkscreen's capitals, shrunk to `width` where given"""
    while width and size > 12 and font(size, bold).getlength(words) > width:
        size -= 1
    if shade:
        d.text((xy[0] + 3, xy[1] + 3), words, font=font(size, bold), fill=shade, anchor=anchor)
    d.text(xy, words, font=font(size, bold), fill=fill, anchor=anchor)


def address(d, cx, cy, words, fill):
    """the site's address in Pixelify Sans (its case matters), its B and C
    Silkscreen's"""
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


# the codes and the rank in letters of our own: Silkscreen's S reads as
# its 5 (and Z as 2) at these sizes, and BN's busting levels run 1 to 10
# and S, so a code or the rank standing alone is drawn from these
GLYPHS = {
    'S': ('.###.', '#...#', '#....', '.###.', '....#', '#...#', '.###.'),
    'H': ('#...#', '#...#', '#...#', '#####', '#...#', '#...#', '#...#'),
    'A': ('.###.', '#...#', '#...#', '#####', '#...#', '#...#', '#...#'),
    'R': ('####.', '#...#', '#...#', '####.', '#.#..', '#..#.', '#...#'),
    'P': ('####.', '#...#', '#...#', '####.', '#....', '#....', '#....'),
}


def glyph(d, cx, cy, ch, px, fill, shade=None):
    """one of GLYPHS centred on (cx, cy), each of its pixels px square"""
    x0, y0 = round(cx - 2.5 * px), round(cy - 3.5 * px)
    for dx, c in ((max(1, px // 2), shade), (0, fill)):
        if c is None:
            continue
        for j, row in enumerate(GLYPHS[ch]):
            for i, on in enumerate(row):
                if on == '#':
                    d.rectangle([x0 + i * px + dx, y0 + j * px + dx, x0 + (i + 1) * px - 1 + dx,
                                 y0 + (j + 1) * px - 1 + dx], fill=c)


def lerp(a, b, t):
    t = max(0.0, min(1.0, t))
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def ease(t):
    t = max(0.0, min(1.0, t))
    return 1 - (1 - t) ** 3


_icons = {}


def icon(name, scale=1):
    k = (name, scale)
    if k not in _icons:
        im = Image.open(os.path.join(UI, name + '.png')).convert('RGBA')
        _icons[k] = im.resize((im.width * scale, im.height * scale), Image.NEAREST) if scale > 1 else im
    return _icons[k]


# ---- the battle's parts, at 427 x 240 ----

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


def plate(d, box):
    """BN's plate (the HP box, a used chip's name): navy in a light edge"""
    x0, y0, x1, y1 = box
    d.rectangle([x0, y0, x1, y1], fill=PLATE_EDGE)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=PLATE)
    d.line([(x0 + 2, y1 - 2), (x1 - 2, y1 - 2)], fill=GAUGE_IN)


def gauge(d, share, fr):
    """the Custom gauge: its fill cyan, gold and flashing when full"""
    x0, y0, x1, y1 = GAUGE
    d.rectangle([x0, y0, x1, y1], fill=PLATE_EDGE)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=GAUGE_IN)
    if share > 0:
        full = share >= 1
        c = (GOLD if (fr // 4) % 2 else ORANGE) if full else FRAME
        xe = x0 + 2 + round((x1 - x0 - 4) * min(1.0, share))
        d.rectangle([x0 + 2, y0 + 2, xe, y1 - 2], fill=c)
        d.line([(x0 + 2, y0 + 2), (xe, y0 + 2)], fill=WHITE if full else FRAME_HI)


def brackets(d, box, fr):
    """the cursor: four orange corners round a chip, breathing a pixel"""
    g = 1 + (fr // 6) % 2
    x0, y0, x1, y1 = box[0] - g, box[1] - g, box[2] + g, box[3] + g
    for cx, cy, sx, sy in ((x0, y0, 1, 1), (x1, y0, -1, 1), (x0, y1, 1, -1), (x1, y1, -1, -1)):
        d.line([(cx, cy), (cx + 4 * sx, cy)], fill=ORANGE, width=2)
        d.line([(cx, cy), (cx, cy + 4 * sy)], fill=ORANGE, width=2)
        d.point((cx + sx, cy + sy), fill=ORANGE_LIT)


def slot(d, x, y, n=16):
    d.rectangle([x, y, x + n - 1, y + n - 1], fill=GAUGE_IN, outline=FRAME_LO)


# the five chips: (art, name, code); the art the site's icons
CHIPS = [('icon-chip', 'BATTLE', 'S'), ('cards', 'PICKS', 'H'), ('icon-code', 'FOLDERS', 'A'),
         ('icon-star', 'RIVALS', 'R'), ('icon-layers', 'PLUS', 'P')]


def put_chip(im, x, y, art, scale=1):
    """a chip's art at (x, y): a site icon, or a Server's three (the site's
    card thrice, the middle one raised)"""
    if art == 'cards':
        c = icon('card', scale)
        for i, dy in enumerate((2, 0, 2)):
            im.paste(c, (x + i * 2 * scale, y + dy * scale), c)
    else:
        ic = icon(art, scale)
        im.paste(ic, (x, y), ic)


# ---- the timeline ----

class Shot:
    """One bar's shot of a segment: the caption on the plate, a count where
    its damage would be, `feed` [(frame, scene, sample, rate)] (from each
    frame on, that scene's footage from that sample; rate 0 holds it; a
    scene 'still' is a screenshot by name)"""

    def __init__(self, start, caption, feed, count='', opens=True):
        self.start, self.caption, self.feed, self.count, self.opens = start, caption, feed, count, opens

    def sample(self, fr):
        i = max([j for j, e in enumerate(self.feed) if e[0] <= fr] or [0])
        start, scene, first, rate = self.feed[i]
        if scene == 'still':
            return scene, first
        return scene, first + max(0, int((fr - start) * rate))


class Segment:
    """A feature: its section card's bar, its number and label (the
    gauge's, fixed through the segment), its chip and its shots"""

    def __init__(self, card, number, label, chip, shots):
        self.card, self.number, self.label, self.chip, self.shots = card, number, label, chip, shots


COLD = [(0, 'server-battle', 92, 1), (F(1), 'server-battle', 262, 1)]   # a battle's blows and its end, no words
TITLE, RECAP, PA, STINGER, END = F(2), F(20), F(21, 2), F(22), F(23)


def still(name):
    return [(0, 'still', name, 0)]


# (footage still to come: a marked placeholder, never another feature's picture)
STANDIN = [(0, 'standin', 0, 0)]


def segments():
    return [
        Segment(F(4), '01', 'THE BATTLE FLOOR', 0, [
            # (the Server's battle until the battle floor's run lands; the
            # End Area's to come)
            Shot(F(5), 'A BATTLE ON EVERY LAYER', [(F(5), 'server-battle', 140, 1)]),
            Shot(F(6), "BN5'S END AREA WALKS ONE WIDE", STANDIN),
            Shot(F(7), 'FEWER WORDS ON THE WAY', [(F(7), 'arrival', 30, 1)]),
        ]),
        Segment(F(8), '02', 'EVERY PICK COUNTS', 1, [
            Shot(F(9), 'A WON SERVER: PICK ONE OF', [(F(9), 'server-pick', 40, 1)], count='3'),
            Shot(F(10), 'MEGAMAN READS THE WAY AHEAD', [(F(10), 'fork-pad', 4, 1)]),
            # (the BugFrag shop's run to come)
            Shot(F(11), 'BUGFRAGS BUY PROGRAMS', STANDIN),
        ]),
        Segment(F(12), '03', 'THREE NEW FOLDERS', 2, [
            Shot(F(13), 'NEW STARTING FOLDERS', [(F(13), 'setup-folders', 8, 1), (F(13, 1.5), 'setup-folders', 26, 1),
                                                 (F(13, 3), 'setup-folders', 47, 0)], count='3'),
            Shot(F(14), 'Z-CANON: CANNONS FOR GIGACAN1', [(F(14), 'hand-zcanon', 10, 1)]),
            Shot(F(15), 'BUSTING: NO RECOVERY CHIP', [(F(15), 'hand-busting', 50, 0)]),
        ]),
        Segment(F(16), '04', 'THE RIVALS REMEMBER', 3, [
            Shot(F(17), 'GUARDIANS REMEMBER HOW IT ENDED', still('guardian-memory')),
            Shot(F(18), 'LAN STOPS THE NETBATTLE AT', [(F(18), 'protoman', 128, 1), (F(18, 2.5), 'protoman', 196, 1)],
                 count='1 HP'),
            Shot(F(19), 'THE DIVE GOES ON', [(F(19), 'protoman', 240, 0)], opens=False),
        ]),
        # the recap: the fifth chip, its tiles in the field
        Segment(RECAP, '05', f'ALSO IN {VERSION}', 4, []),
    ]


# the recap: (picture, its label); a count only where the CHANGELOG gives
# one; and its caption on the plate
TILES = [(('still', 'home-bbs-post'), ("REQUESTS ON", "BN6'S OWN BBS")),
         (('still', 'home-courier-offer'), ('THE COURIER NAMES', 'EACH ONE')),
         (('fork-portal', 70), ("LAN'S HP'S PORTALS", 'READ TOO'))]
RECAP_CAPTION = "OPT-IN STATS COUNT A RUN'S CHOICES"
SYSTEMS = ('PortMaster handhelds · Linux · Windows · macOS', 'Android · iPhone and iPad · New 3DS · the browser')
SITE = 'saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless'
OPEN = 5                                   # a shot's opening out of the chip's window, in frames
SLIDE = 6                                  # the Custom screen's slide, in frames


def where(segs, fr):
    """(the segment, its shot or None on its card)"""
    seg = None
    for s in segs:
        if s.card <= fr:
            seg = s
    shot = None
    for sh in seg.shots:
        if sh.start <= fr:
            shot = sh
    return seg, shot


# ---- the battle screen: the segments and the recap ----

def custom_window(small, d, seg, segs, fr, dx):
    """the Custom screen, moved dx: the chip's window, the column of
    chips sent, CHIP SELECT and its cursor, OK"""
    x0, y0, x1, y1 = CUSTOM
    window(d, (x0 + dx, y0, x1 + dx, y1))
    card = fr - seg.card
    art, _, _ = CHIPS[seg.chip]
    d.rectangle([ART[0] + dx, ART[1], ART[2] + dx, ART[3]], fill=lerp(FIELD, FRAME_LO, 0.35), outline=FRAME_HI)
    put_chip(small, (ART[0] + ART[2]) // 2 - 24 + dx, ART[1] + 6, art, 4)
    plate(d, (NAME[0] + dx, NAME[1], NAME[2] + dx, NAME[3]))
    k = segs.index(seg)
    sent = k + (1 if card >= 3 * BEAT else 0)
    for i in range(5):
        x, y = COLUMN[0] + dx, COLUMN[1] + 19 * i
        slot(d, x, y)
        if i < sent:
            put_chip(small, x + 2, y + 2, CHIPS[i][0])
    for i in range(5):
        x, y = ROW[0] + PITCH * i + dx, ROW[1]
        slot(d, x, y)
        if i >= sent or i == seg.chip and card < 3 * BEAT:
            put_chip(small, x + 2, y + 2, CHIPS[i][0])
    at = seg.chip if card >= BEAT or k == 0 else segs[k - 1].chip
    if card < 3 * BEAT:
        brackets(d, (ROW[0] + PITCH * at + dx, ROW[1], ROW[0] + PITCH * at + 15 + dx, ROW[1] + 15), fr)
    lit = 3 * BEAT <= card < 3 * BEAT + 6
    ox0, oy0, ox1, oy1 = OK
    d.rectangle([ox0 + 2 + dx, oy0 + 2, ox1 + 2 + dx, oy1 + 2], fill=BUTTON_SHADE)
    d.rectangle([ox0 + dx, oy0, ox1 + dx, oy1], fill=BUTTON_EDGE)
    d.rectangle([ox0 + 2 + dx, oy0 + 2, ox1 - 2 + dx, oy1 - 2], fill=WHITE if lit else BUTTON)


def custom_words(d, seg, fr, dx):
    _, name, code = CHIPS[seg.chip]
    x = dx * S
    text(d, (CUSTOM[0] * S + 22 + x, (CUSTOM[1] + 10) * S), 'CUSTOM', 18, CYAN_TEXT, True, 'lm')
    text(d, ((NAME[0] + NAME[2]) * S / 2 + x, (NAME[1] + NAME[3]) * S / 2), name, 24, WHITE, True, 'mm',
         width=(NAME[2] - NAME[0]) * S - 16)
    glyph(d, NAME[0] * S + 20 + x, (NAME[3] + 9) * S, code, 4, GOLD, GAUGE_IN)
    for i in range(5):
        glyph(d, (ROW[0] + PITCH * i + 8) * S + x, (ROW[1] + 24) * S, CHIPS[i][2], 3, GOLD, GAUGE_IN)
    lit = 3 * BEAT <= fr - seg.card < 3 * BEAT + 6
    text(d, ((OK[0] + OK[2]) * S / 2 + x, (OK[1] + OK[3]) * S / 2), 'OK', 40, BUTTON if lit else WHITE, True, 'mm',
         shade=None if lit else (8, 140, 24))


def chip_card(small, d, seg, fr):
    """the section card in the field: the chip large, its code"""
    art, _, _ = CHIPS[seg.chip]
    put_chip(small, (GAME[0] + GAME[2]) // 2 - 36, GAME[1] + 20, art, 6)


def standin():
    """the field's picture while a shot's footage is still to come"""
    im = Image.new('RGB', (240, 160), GAUGE_IN)
    d = ImageDraw.Draw(im)
    for x in range(-160, 240, 12):
        d.line([(x, 0), (x + 160, 160)], fill=PLATE)
    d.rectangle([60, 66, 180, 94], fill=GAUGE_IN, outline=FRAME_LO)
    f = ImageFont.truetype(CAPS, 8)
    d.text((120, 80), 'FOOTAGE TO COME', font=f, fill=SOFT, anchor='mm')
    return im


def opening(small, picture, fr, start):
    """the cut: the shot opening out of the chip's window into the field"""
    t = ease((fr - start + 1) / OPEN)
    x0 = round(ART[0] + (PIC[0] - ART[0]) * t)
    y0 = round(ART[1] + (PIC[1] - ART[1]) * t)
    w = round((ART[2] - ART[0]) + (240 - (ART[2] - ART[0])) * t)
    h = round((ART[3] - ART[1]) + (160 - (ART[3] - ART[1])) * t)
    if t >= 1:
        small.paste(picture, PIC)
    else:
        small.paste(picture.resize((max(1, w), max(1, h)), Image.NEAREST), (x0, y0))


def draw_battle(segs, fr):
    small = net(fr)
    d = ImageDraw.Draw(small)
    seg, shot = where(segs, fr)
    recap = seg.card == RECAP
    dx = -round((CUSTOM[2] + 4) * (1 - ease((fr - segs[0].card + 1) / SLIDE))) if fr < segs[0].card + SLIDE else 0
    plate(d, HP_BOX)
    span = (PA - RECAP - BEAT // 2) if recap else (4 * BAR - BEAT)
    gauge(d, (fr - seg.card) / span, fr)
    window(d, GAME)
    if shot is None and not recap:
        chip_card(small, d, seg, fr)
    elif shot is not None:
        scene, i = shot.sample(fr)
        pic = standin() if scene == 'standin' else kit.shot(scene, i)
        if shot.opens and fr - shot.start < OPEN:
            prev = seg.shots.index(shot) - 1
            if prev >= 0:
                before = seg.shots[prev].sample(shot.start - 1)
                small.paste(standin() if before[0] == 'standin' else kit.shot(*before), PIC)
            opening(small, pic, fr, shot.start)
        else:
            small.paste(pic, PIC)
    if dx > -(CUSTOM[2] + 4):
        custom_window(small, d, seg, segs, fr, dx)
    plate(d, PLATE_BOX)
    im = small.resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
    d = ImageDraw.Draw(im)
    text(d, ((HP_BOX[0] + HP_BOX[2]) * S / 2, (HP_BOX[1] + HP_BOX[3]) * S / 2), VERSION, 30, WHITE, True, 'mm')
    label = f'{seg.number} · {seg.label}'
    text(d, (GAUGE[0] * S + 14, (GAUGE[1] + GAUGE[3]) * S / 2 + 1), label, 22, WHITE, True, 'lm', shade=GAUGE_IN)
    if dx > -(CUSTOM[2] + 4):
        custom_words(d, seg, fr, dx)
    cy = (PLATE_BOX[1] + PLATE_BOX[3]) * S / 2
    if recap:
        draw_recap(im, d, fr)
        text(d, (PLATE_BOX[0] * S + 30, cy), RECAP_CAPTION, 40, WHITE, True, 'lm',
             width=(PLATE_BOX[2] - PLATE_BOX[0]) * S - 60)
    elif shot is None:
        # the card's words in the field, under the chip
        gx = (GAME[0] + GAME[2]) * S / 2
        a = min(1.0, (fr - seg.card + 1) / 4)
        text(d, (gx, (GAME[1] + 105) * S), seg.number, 40, lerp(FIELD, GOLD, a), True, 'mm')
        text(d, (gx, (GAME[1] + 130) * S), seg.label, 44, lerp(FIELD, WHITE, a), True, 'mm', shade=GAUGE_IN,
             width=(GAME[2] - GAME[0] - 24) * S)
    else:
        text(d, (PLATE_BOX[0] * S + 30, cy), shot.caption, 40, WHITE, True, 'lm',
             width=(PLATE_BOX[2] - PLATE_BOX[0]) * S - (260 if shot.count else 60))
        if shot.count:
            text(d, (PLATE_BOX[2] * S - 30, cy), shot.count, 48, GOLD, True, 'rm')
    return im


def draw_recap(im, d, fr):
    """three tiles in the field at 1x, a picture and its two lines each,
    one a beat"""
    w, h, gap = 240, 160, 12
    x0 = (GAME[0] + 5) * S
    y0 = (GAME[1] + 5) * S + 110
    for i, ((scene, sample), lines) in enumerate(TILES):
        if fr < RECAP + i * BEAT:
            continue
        x = x0 + i * (w + gap)
        d.rectangle([x - 3, y0 - 3, x + w + 2, y0 + h + 2], fill=FRAME_HI)
        im.paste(kit.shot(scene, sample), (x, y0))
        for j, line in enumerate(lines):
            text(d, (x + w / 2, y0 + h + 44 + 36 * j), line, 22, WHITE, True, 'mm', shade=GAUGE_IN, width=w)


# ---- the big window: the cold open, the title, the stinger, the end ----

_logo = {}


def logo(width):
    if width not in _logo:
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        _logo[width] = lg.resize((width, round(lg.height * width / lg.width)), Image.LANCZOS)
    return _logo[width]


def big_frame(fr):
    """the net at 3x and one big window, its picture's place at 4x"""
    im = net(fr).resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
    d = ImageDraw.Draw(im)
    window(d, (BIG[0] - 15, BIG[1] - 15, BIG[0] + 960 + 14, BIG[1] + 640 + 14), 3)
    return im, d


def draw_cold(fr):
    im, d = big_frame(fr)
    feed = max(e for e in COLD if e[0] <= fr)
    start, scene, first, rate = feed
    pic = kit.shot(scene, first + int((fr - start) * rate))
    im.paste(pic.resize((960, 640), Image.NEAREST), BIG)
    return im


def draw_title(fr):
    """the logo, THE RUN, SHARPENED and the version on BN's plate"""
    im, d = big_frame(fr)
    k = fr - TITLE
    lg = logo(620)
    im.paste(lg, (W // 2 - 310, 120), lg)
    if k >= BEAT:
        a = min(1.0, (k - BEAT + 1) / 4)
        text(d, (W // 2, 120 + lg.height + 70), 'THE RUN, SHARPENED', 56, lerp(FIELD, WHITE, a), True, 'mm',
             shade=GAUGE_IN)
    if k >= 2 * BEAT:
        x0, x1, y0, y1 = 330, 950, 440, 526
        d.rectangle([x0, y0, x1, y1], fill=PLATE_EDGE)
        d.rectangle([x0 + 3, y0 + 3, x1 - 3, y1 - 3], fill=PLATE)
        d.rectangle([x0 + 6, y1 - 9, x1 - 6, y1 - 6], fill=GAUGE_IN)
        text(d, (W // 2, (y0 + y1) / 2), f'{VERSION} · OUT NOW', 44, GOLD, True, 'mm', shade=GAUGE_IN,
             width=x1 - x0 - 60)
    return im


def draw_pa(fr):
    """the Program Advance's two beats: the five chips in code order, S H
    A R P, closing up"""
    im, d = big_frame(fr)
    t = ease((fr - PA + 1) / (STINGER - PA))
    gap = round(150 - 126 * t)
    x0 = W // 2 - (4 * gap + 5 * 96) // 2
    for i, (art, _, code) in enumerate(CHIPS):
        x = x0 + i * (96 + gap)
        d.rectangle([x - 6, 294, x + 102, 402], fill=FRAME_LO)
        d.rectangle([x, 300, x + 96, 396], fill=GAUGE_IN)
        put_chip(im, x + 12, 312, art, 6)
        glyph(d, x + 48, 456, code, 6, GOLD, GAUGE_IN)
    return im


def draw_stinger(fr):
    """the cadence's downbeat: a flash, then the logo and the version"""
    im, d = big_frame(fr)
    lg = logo(700)
    im.paste(lg, (W // 2 - 350, 190), lg)
    text(d, (W // 2, 190 + lg.height + 70), VERSION, 64, GOLD, True, 'mm', shade=GAUGE_IN)
    k = fr - STINGER
    if k < 6:
        return Image.blend(im, Image.new('RGB', (W, H), WHITE), 1 - k / 6)
    return im


def draw_end(fr):
    """BN's results in the PET's look, held still"""
    small = net(END)
    d = ImageDraw.Draw(small)
    window(d, (40, 8, 386, 232))
    im = small.resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
    d = ImageDraw.Draw(im)
    k = fr - END
    text(d, (40 * S + 30, 8 * S + 34), 'RESULTS', 22, CYAN_TEXT, True, 'lm')
    lg = logo(500)
    y = 74
    im.paste(lg, (W // 2 - 250, y), lg)
    y += lg.height + 44
    length = FRAMES / FPS
    rows = (('DELETE TIME', f'{int(length // 60)}:{length % 60:05.2f}'), ('BUSTING LEVEL', 'S'),
            ('GET DATA', VERSION + ' · OUT NOW'))
    for i, (key_, value) in enumerate(rows):
        if k < i * 4:
            continue
        yy = y + 56 * i
        text(d, (310, yy), key_, 30, CYAN_TEXT, True, 'lm')
        if value in GLYPHS:
            glyph(d, 950, yy - 2, value, 6, GOLD, GAUGE_IN)
        else:
            text(d, (970, yy), value, 30, GOLD if i < 2 else WHITE, True, 'rm', shade=GAUGE_IN)
        d.line([(310, yy + 26), (970, yy + 26)], fill=FRAME_LO, width=3)
    if k < BEAT:
        return im
    y += 56 * 3 + 22
    for i, line in enumerate(SYSTEMS):
        text(d, (W // 2, y + 34 * i), line, 22, WHITE, False, 'mm')
    address(d, W // 2, y + 90, SITE, CYAN_TEXT)
    foot = ("Needs your own Mega Man Battle Network 6: Cybeast Gregar (USA) ROM.",
            "BN5's areas need Mega Man Battle Network 5: Team Colonel (USA) too.",
            'An unofficial fan project, not affiliated with Capcom.')
    for i, line in enumerate(foot):
        text(d, (W // 2, y + 136 + 26 * i), line, 16, SOFT, False, 'mm')
    return im


# ---- the frame ----

def draw(segs, fr):
    if fr < TITLE:
        return draw_cold(fr)
    if fr < segs[0].card:
        return draw_title(fr)
    if fr < PA:
        return draw_battle(segs, fr)
    if fr < STINGER:
        return draw_pa(fr)
    if fr < END:
        return draw_stinger(fr)
    return draw_end(fr)


def compose(only=None):
    frames_dir = os.path.join(kit.WORK, 'frames')
    if only is None:
        shutil.rmtree(frames_dir, ignore_errors=True)
    os.makedirs(frames_dir, exist_ok=True)
    segs = segments()
    for fr in range(FRAMES):
        if only is not None and fr not in only:
            continue
        draw(segs, fr).save(os.path.join(frames_dir, f'{fr:05d}.png'), compress_level=1)
        if fr % 100 == 0:
            print('frame', fr, '/', FRAMES, flush=True)


POSTER = F(3, 3)        # (the title card held: the logo, THE RUN, SHARPENED, 0.17.0 · OUT NOW)
