"""0.16.0's trailer, FIGHT ON?: BN6's Virus Battler in the run, the
Crosses a run holds on a dark layer's team, and rumble with a
DualSense's triggers, in the skill's structure (SKILL.md, The structure):
a cold open of two bars with no words, the title card, a segment for each
headline (a section card of a bar, then a shot a bar, each one concrete
thing), the recap of the rest, the logo's stinger on the music's
cadence, and the end card held.

The look is BN6's PET (docs/engine/ui.md, tools/site_art.py's colours):
the big window, the game at 4x in one cyan window on the PET's body, for
the cold open, the title, the stinger and the end card; and the PET
screen for the segments: the band holds the section's label, a list
window holds the segment's shots with the orange cursor stepping down it
on the beat (the cut), the game's window beside it, the INFORMATION box
the caption, one line. A section opens with its number and name in its
colour in the game's window, under the page arrow just pressed. The
rumble flashes the window's frame on the hits; the screen fades up and
changes in the GBA's brightness steps; the end card stays, still. The
words are the site's Silkscreen; the address alone its Pixelify Sans.

Its shots are whole frames of the game running from the developer's ROMs,
BN6's and BN5 Team Colonel's beside it: a Battler's series on seed 1's
first layer (`--dev battler=1`, the Virus Battler PR's own session:
MegaMan walking up to him, two rounds at fast forward, the pot taken),
the TEAM screen at act 3 with six Souls and SlashMan's Cross held, the
autopilot on act 2's dark board with each Cross brought (`--setup
endless,0,0,0,N`), Options' Audio with a virtual DualSense, a Net
Dealer's shelf with a Soul held, and two of docs/screenshots (the Net
Square, a battle). Everything else is drawn here; the music is
v0_16_0_music.py's, an original piece in the manner of BN6's "Showdown"
(song 0x22), grown from the sketch the owner chose.

    python3 build.py host && python3 tools/trailer.py 0.16.0 [--keep]
"""
import math
import os
import shutil
import sys

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer as kit  # noqa: E402
from trailer import ROOT  # noqa: E402
from trailers import v0_16_0_music as music  # noqa: E402

VERSION = '0.16.0'
PLAY_LABEL = 'WATCH THE 0.16.0 TRAILER'
BN6_ONLY = ()


# ---- the footage ----

AUTO = {'CYBERWORLD_AUTOPILOT': '1'}


def play(cmds):
    """play.py's commands (press, wait, place) as `--input`'s steps"""
    out = []
    for cmd in cmds.split(';'):
        w = cmd.split()
        if not w:
            continue
        if w[0] == 'press':
            out += [f'6:{w[1]}', '6:']
        elif w[0] == 'hold':
            out.append(f'{w[2]}:{w[1]}')
        elif w[0] == 'wait':
            out.append(f'{w[1]}:')
        elif w[0] == 'place':
            out.append(f'0:place {w[1]} {w[2]} {w[3]}')
    return ','.join(out)


# seed 1's first layer with Battler 1 on it (Mettaur and Gunner, three
# rounds), 5000 zenny and ErthDrgn on the card beside Mettaur and Gunner:
# the Virus Battler PR's session vt, MegaMan walking up to him, two rounds
# at fast forward, the pot taken after the second
BATTLER = ['--scene', 'emu', '--seed', '1', '--dev', 'quiet,battler=1,zenny=5000,viruses=0x10000']
PAGE = 'wait 290; ' + 'press A; wait 12; ' * 8           # (the layer's greeting paged)
LIST = ('press A; wait 12; ' + 'press DOWN; wait 8; ' * 13 + 'press A; wait 12; press RIGHT; wait 12; press A; wait 40; '
        'press A; wait 40; press DOWN; wait 12; press A; wait 100; ')
ROUND = 'press A; wait 40; press FAST; wait 160; press FAST; wait 200; '
NEXT = 'press A; wait 90; press A; wait 90; wait 2; press DOWN; wait 10; press A; wait 90; press A; wait 420; '
WALK = PAGE + 'place 44 100 0; wait 10; hold UP+LEFT 30; wait 10; press A; '
CHAT = 'wait 70; press A; wait 68; press A; wait 88; press A; wait 70; press LEFT; wait 20; press A; wait 90; press A; wait 420; '
# (the second round's offer comes after the DOWN: its cursor stays on TAKE
# THE POT, and MegaMan takes it)
SERIES = WALK + CHAT + LIST + ROUND + NEXT + LIST + ROUND + NEXT + 'wait 400'


def board(seed, depth, cross, first, last):
    """the autopilot on a dark layer's board with a Cross brought (both ROMs)"""
    return (['--scene', 'emu', '--run-depth', str(depth), '--seed', str(seed), '--dev', 'dark,god,onehit', '--setup',
             f'endless,0,0,0,{cross}'], AUTO, None, first, last)


def scenes():
    s = {
        'bv-series': (BATTLER, {}, play(SERIES), 470, 5600),
        # act 3's board with the six Souls and SlashMan's Cross held: the
        # TEAM screen, its cursor down the list to SlashMan
        'team': (['--scene', 'emu', '--run-depth', '8', '--seed', '1', '--dev', 'dark,god,onehit,souls=0x3F', '--setup',
                  'endless,0,0,0,3'], {}, play('wait 280; ' + 'press DOWN; wait 30; ' * 7 + 'wait 160'), 200, 680),
        # act 2's board, a Cross brought: HeatMan's Napalm and his battle as
        # HeatCross, ElecMan's MagnetBarrier, ChargeMan's BombLiberation,
        # SlashMan on the board and as SlashCross
        'heat': board(1, 5, 1, 2150, 3480),
        'elec': board(1, 5, 2, 9860, 10020),
        'charge': board(1, 5, 5, 4360, 4620),
        'slash': board(1, 5, 3, 4000, 4900),
        # Options' Audio with a DualSense (a virtual one): Vibration, Trigger effects
        'options': (['--scene', 'title', '--pad', 'ps5'], {},
                    play('wait 60; press SELECT; wait 40; press DOWN; wait 20; press DOWN; wait 20; press A; wait 60; '
                         + 'press DOWN; wait 40; ' * 6 + 'wait 60'), 100, 700),
        # layer 2 of seed 3 with Colonel's Soul held: the Net Dealer's shelf,
        # its fourth chip one of the Soul's kind
        'shelf': (['--scene', 'emu', '--run-depth', '2', '--seed', '3', '--dev', 'quiet,souls=1', '--talk', 'shop:430'], {},
                  '500:' + ',4:A,96:' * 8 + ',4:A,56:' * 6 + ',6:DOWN,24:' * 2 + ',60:', 1200, 1800),
    }
    return s




# ---- the look: BN6's PET, in the skill's structure ----
#
# Two screens, both drawn in BN6's PET look (docs/engine/ui.md, the colours
# of tools/site_art.py) and scaled by nearest neighbour so every edge stays
# hard: the big window (a 320 x 180 screen at 4x, the game's picture at 4x
# in one cyan window on the PET's body) for the cold open, the title card,
# the logo's stinger and the end card; and the PET screen (427 x 240 at 3x)
# for the feature segments and the recap: the band with the section's
# label, a list window of the segment's shots with the orange cursor
# stepping down it on the beat, the game's window beside it, and the
# INFORMATION box holding the feature's caption. The words are the site's
# Silkscreen (OFL, web/assets/fonts), whose B, G, 8 and 6 stay apart, drawn
# at the video's scale inside the safe area; the site's address alone, whose
# case matters, in its Pixelify Sans.

FPS, W, H = 30, 1280, 720
BAR = round(FPS * 60 * 4 / music.BPM)     # (48 frames at 150 beats a minute)
BEAT = BAR // 4
FRAMES = music.BARS * BAR + round(music.TAIL * FPS)

FONTS = os.path.join(ROOT, 'web', 'assets', 'fonts')
CAPS = os.path.join(FONTS, 'silkscreen-700.woff2')        # (heavy: names, labels, the buttons)
CAPS_THIN = os.path.join(FONTS, 'silkscreen-400.woff2')   # (the list, the captions, the end card's lines)
PIXEL = os.path.join(FONTS, 'pixelify-sans.woff2')        # (the site's address alone: its case matters)

BAND = (0, 123, 49)
BODY = (8, 189, 115)
FRAME_HI, FRAME, FRAME_LO = (107, 222, 255), (66, 198, 231), (16, 140, 239)
FIELD = (16, 82, 107)
WHITE = (247, 255, 247)
SHADE = (33, 115, 140)
EDGE = (33, 74, 82)
INFO_BOX, PAGE, INK = (74, 231, 115), (247, 247, 247), (66, 57, 57)
ORANGE, ORANGE_DARK, ORANGE_LIT = (255, 173, 16), (214, 107, 0), (255, 206, 24)
CHOICE, CHOICE_HI, CHOICE_OUT = (57, 255, 0), (115, 255, 74), (33, 165, 33)
BUTTON, BUTTON_EDGE, BUTTON_SHADE = (16, 231, 41), (206, 222, 239), (41, 132, 148)
ARROW, ARROW_HI, ARROW_DARK = (74, 198, 16), (189, 231, 132), (8, 165, 0)
CYAN_TEXT = (107, 222, 255)
GOLD = (255, 214, 16)

SAFE_X, SAFE_Y = 64, 36               # 5% of each edge

# the PET screen, at its 427 x 240
S = 3
LW, LH = 427, 240
BAND_H = 25
LIST = (7, 28, 161, 197)
GAME = (168, 28, 417, 197)            # its picture at (173, 33)
INFO = (7, 200, 417, 238)
ROW0, ROW_H = 50, 20
# the big window, at its 320 x 180
B = 4
BW, BH = 320, 180
BIG = (35, 5, 284, 174)               # its picture at (40, 10)


def F(bar, beat=0.0):
    return round(bar * BAR + beat * BEAT)


_fonts = {}


def font(size, bold=False):
    """the site's Silkscreen, a pixel face whose B, G, 8 and 6 stay apart
    (Pixelify Sans' B read as a G at these sizes: the owner, 26 October)"""
    key = (size, bold)
    if key not in _fonts:
        _fonts[key] = ImageFont.truetype(CAPS if bold else CAPS_THIN, size)
    return _fonts[key]


def text(d, xy, words, size, fill, bold=False, anchor='la', shade=None, width=None):
    """Silkscreen's capitals, shrunk to `width` where given"""
    while width and size > 12 and font(size, bold).getlength(words) > width:
        size -= 1
    f = font(size, bold)
    if shade:
        d.text((xy[0] + 3, xy[1] + 3), words, font=f, fill=shade, anchor=anchor)
    d.text(xy, words, font=f, fill=fill, anchor=anchor)


def address(d, cx, cy, words, fill):
    """the site's address, whose case matters (Silkscreen has capitals
    only): Pixelify Sans, its B and C Silkscreen's at its height"""
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


def ease(t):
    t = max(0.0, min(1.0, t))
    return 1 - (1 - t) ** 3


def steps(t):
    """t (0-1) in the GBA's sixteen brightness steps"""
    return math.floor(max(0.0, min(1.0, t)) * 16) / 16


def lerp(a, b, t):
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


# ---- the timeline ----

class Shot:
    """One bar's shot of a segment: its row in the list, the caption in
    the INFORMATION box, `feed` [(frame, scene, sample, rate)] as 0.15.0's
    (from each frame on, that scene's footage from that sample; rate 0
    holds it), `hits` frames the window's frame flashes on (the rumble)."""

    def __init__(self, start, row, caption, feed, hits=()):
        self.start, self.row, self.caption, self.feed, self.hits = start, row, caption, feed, hits

    def sample(self, fr):
        i = max([j for j, e in enumerate(self.feed) if e[0] <= fr] or [0])
        start, scene, first, rate = self.feed[i]
        return scene, first + max(0, int((fr - start) * rate))


class Segment:
    """A feature: its section card's bar, its label (the band's, fixed
    through the segment), its colour and its shots"""

    def __init__(self, card, number, label, colour, shots):
        self.card, self.number, self.label, self.colour, self.shots = card, number, label, colour, shots


COLD = [(0, 'bv-series', 800, 1), (F(1), 'heat', 300, 1)]          # the fight's last blows; HeatCross
TITLE, STINGER, END = F(2), F(19), F(20)
RECAP = F(17)


def segments():
    return [
        Segment(F(4), '01', "BN6'S VIRUS BATTLER", GOLD, [
            Shot(F(5), 'His offer', 'Stake zenny; he names his pair', [(F(5), 'bv-series', 100, 1)]),
            Shot(F(6), 'VIRUS LIST', "Pick yours on BN6's VIRUS LIST",
                 [(F(6), 'bv-series', 440, 0), (F(6, 1.5), 'bv-series', 624, 1)]),
            Shot(F(7), 'The fight', 'They fight his pair on their own', [(F(7), 'bv-series', 778, 1)]),
            Shot(F(8), 'The pot', 'Take the pot, or fight on', [(F(8), 'bv-series', 1040, 1), (F(8, 2), 'bv-series', 1062, 1)]),
        ]),
        Segment(F(9), '02', 'YOUR CROSSES JOIN THE DARK', ORANGE, [
            Shot(F(10), 'TEAM', 'Pick who comes on the TEAM screen', [(F(10), 'team', 150, 1)]),
            Shot(F(11), 'BombLiberation', "ChargeMan's command frees a dark panel",
                 [(F(11), 'charge', 84, 0), (F(11) + 8, 'charge', 90, 1), (F(11) + 14, 'charge', 96, 0)]),
            Shot(F(12), 'HeatCross', 'His battles: MegaMan in his Cross', [(F(12), 'heat', 372, 1)]),
        ]),
        Segment(F(13), '03', 'RUMBLE AND TRIGGERS', CYAN_TEXT, [
            Shot(F(14), 'Vibration', 'Vibration: Strong, Medium, Weak or Off', [(F(14), 'options', 135, 0)]),
            Shot(F(15), 'Triggers', "A DualSense's L clicks at a full gauge", [(F(15), 'options', 165, 0)]),
            Shot(F(16), 'In a fight', 'A hit, a delete, a Cross: you feel it', [(F(16), 'heat', 450, 1)],
                 hits=(F(16, 0.5), F(16, 1.25), F(16, 2.75))),
        ]),
    ]


# the recap: (picture, its label); a count only where the CHANGELOG gives one
TILES = [(('shelf', 110), "1 in 4 finds of a held Soul's kind"),
         (('still', 'online-square'), 'Host your own Net Square'),
         (('still', 'battle'), 'Attacks rumble as they sound')]
SYSTEMS = ('PortMaster handhelds · Linux · Windows · macOS', 'Android · iPhone and iPad · New 3DS · the browser')
SITE = 'saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless'


# ---- the PET's parts ----

_floor = []


def body(w, h, top=0):
    """the PET's body: site_art.py's floor"""
    key = (w, h, top)
    for k, im in _floor:
        if k == key:
            return im.copy()
    tile = Image.open(os.path.join(ROOT, 'web', 'assets', 'ui', 'floor.png')).convert('RGB')
    im = Image.new('RGB', (w, h), BODY)
    for x in range(0, w, tile.width):
        for y in range(top, h, tile.height):
            im.paste(tile, (x, y))
    _floor.append((key, im))
    return im.copy()


def window(d, box, lit=0.0):
    """the cyan window round a navy field; `lit` (0-1) its frame flashed"""
    x0, y0, x1, y1 = box
    hi, mid, lo = (lerp(c, (255, 255, 255), lit) for c in (FRAME_HI, FRAME, FRAME_LO))
    d.rectangle([x0, y0, x1, y1], fill=lo)
    d.rectangle([x0, y0, x1 - 1, y1 - 1], fill=hi)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=mid)
    d.rectangle([x0 + 4, y0 + 4, x1 - 4, y1 - 4], fill=lo)
    d.rectangle([x0 + 5, y0 + 5, x1 - 5, y1 - 5], fill=FIELD)


def band(d, stripes_from):
    """the band across the top, two stripes running off to its right"""
    d.rectangle([0, 0, LW - 1, BAND_H - 1], fill=BAND)
    for y, lead in ((9, 0), (15, 5)):
        x = stripes_from + lead
        d.line([(x, y + 4), (x + 4, y)], fill=BODY, width=1)
        d.line([(x + 4, y), (LW - 1, y)], fill=BODY, width=2)


def page_arrow(d, x, y, right, lit=False):
    fill, hi = (ARROW_HI, WHITE) if lit else (ARROW, ARROW_HI)
    pts = [(x, y), (x, y + 8), (x + 4, y + 4)] if right else [(x + 4, y), (x + 4, y + 8), (x, y + 4)]
    d.polygon(pts, fill=fill, outline=ARROW_DARK)
    d.line([pts[0], pts[2]], fill=hi)


def cursor(d, x, y, fr):
    """the orange arrow, never still: one pixel right, back, two right"""
    x += (0, 1, 0, 2)[(fr // 3) % 4]
    d.polygon([(x, y), (x, y + 8), (x + 5, y + 4)], fill=ORANGE, outline=ORANGE_DARK)
    d.line([(x + 1, y + 2), (x + 1, y + 6)], fill=ORANGE_LIT)


def triangle(d, x, y):
    """the green triangle before the chosen answer, at its scale"""
    d.polygon([(x, y), (x, y + 8), (x + 5, y + 4)], fill=CHOICE, outline=CHOICE_OUT)
    d.line([(x + 1, y + 1), (x + 1, y + 7)], fill=CHOICE_HI)


def info_box(d):
    x0, y0, x1, y1 = INFO
    d.rectangle([x0, y0, x1, y1], fill=EDGE)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=INFO_BOX)
    d.rectangle([x0 + 3, y0 + 10, x1 - 3, y1 - 3], fill=PAGE)


def button(d, cx, cy, words, size=34):
    """the one action, NaviCust's RUN!: bright green in a light edge, a
    dark shade under it, at the video's scale"""
    f = font(size, True)
    w = f.getlength(words) + 60
    h = size + 24
    x0, y0 = round(cx - w / 2), round(cy - h / 2)
    d.rectangle([x0 + 6, y0 + 6, x0 + w + 6, y0 + h + 6], fill=BUTTON_SHADE)
    d.rectangle([x0, y0, x0 + w, y0 + h], fill=BUTTON_EDGE)
    d.rectangle([x0 + 6, y0 + 6, x0 + w - 6, y0 + h - 6], fill=BUTTON)
    text(d, (cx, cy), words, size, WHITE, True, 'mm', shade=(8, 140, 24))


# ---- the big window: the cold open, the title, the stinger, the end ----

_logo = {}


def logo(width):
    if width not in _logo:
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        _logo[width] = lg.resize((width, round(lg.height * width / lg.width)), Image.LANCZOS)
    return _logo[width]


def appear(fr, at, n=6):
    """0-1: words come up in the GBA's brightness steps over n frames"""
    return steps((fr - at + 1) / n)


def draw_big(fr):
    small = body(BW, BH)
    d = ImageDraw.Draw(small)
    window(d, BIG)
    if fr < TITLE:
        i = 0 if fr < COLD[1][0] else 1
        start, scene, first, rate = COLD[i]
        small.paste(kit.shot(scene, first + (fr - start) * rate), (BIG[0] + 5, BIG[1] + 5))
    im = small.resize((BW * B, BH * B), Image.NEAREST)
    d = ImageDraw.Draw(im)
    gx, gy = (BIG[0] + 5) * B, (BIG[1] + 5) * B          # the window's field: 960 x 640
    cx = gx + 480

    def put_logo(width, y, at):
        a = appear(fr, at)
        if a <= 0:
            return
        lg = logo(width).copy()
        lg.putalpha(lg.getchannel('A').point(lambda v: int(v * a)))
        im.paste(lg, (round(cx - lg.width / 2), y), lg)

    def say(at, xy, words, size, fill, bold=False, width=880):
        a = appear(fr, at)
        if a > 0:
            text(d, xy, words, size, lerp(FIELD, fill, a), bold, 'mm', width=width)

    if TITLE <= fr < F(4):
        put_logo(760, gy + 90, TITLE)
        say(TITLE + BEAT, (cx, gy + 360), 'FIGHT ON?', 72, WHITE, True)
        if fr >= TITLE + 2 * BEAT:
            button(d, cx, gy + 490, '0.16.0 · OUT NOW', 40)
    elif STINGER <= fr < END:
        put_logo(760, gy + 150, STINGER, )
        say(STINGER + 2, (cx, gy + 420), '0.16.0', 80, GOLD, True)
    elif fr >= END:
        put_logo(560, gy + 40, END - 6)
        button(d, cx, gy + 250, '0.16.0 · OUT NOW', 34)
        for i, line in enumerate(SYSTEMS):
            say(END + BEAT + i * 2, (cx, gy + 345 + 40 * i), line, 24, WHITE)
        a = appear(fr, END + 2 * BEAT)
        if a > 0:
            address(d, cx, gy + 450, SITE, lerp(FIELD, CYAN_TEXT, a))
        say(END + 3 * BEAT, (cx, gy + 535), "Needs your own Mega Man Battle Network 6: Cybeast Gregar (USA) ROM.", 16,
            (198, 222, 239))
        say(END + 3 * BEAT, (cx, gy + 566), 'The dark boards need Mega Man Battle Network 5: Team Colonel (USA) too.', 16,
            (198, 222, 239))
        say(END + 3 * BEAT, (cx, gy + 597), 'An unofficial fan project, not affiliated with Capcom.', 16, (198, 222, 239))
    return im


# ---- the PET screen: the segments and the recap ----

def where(segs, fr):
    """(the segment, the shot under the cursor or None on its card)"""
    seg = None
    for s in segs:
        if s.card <= fr:
            seg = s
    if seg is None:
        return None, None, -1
    shot, row = None, -1
    for i, sh in enumerate(seg.shots):
        if sh.start <= fr:
            shot, row = sh, i
    return seg, shot, row


def draw_pet(segs, fr):
    small = body(LW, LH, BAND_H)
    d = ImageDraw.Draw(small)
    recap = fr >= RECAP
    seg, shot, row = (None, None, -1) if recap else where(segs, fr)
    label = 'ALSO IN 0.16.0' if recap else seg.label
    band(d, 22 + round(font(40, True).getlength(label) / S) + 8)
    if recap:
        # the recap's one window, its tiles in it
        window(d, (LIST[0], LIST[1], GAME[2], INFO[3]))
    else:
        # the list: the segment's shots as its rows, its number on the window's band
        window(d, LIST)
        x0, y0, x1, _ = LIST
        d.rectangle([x0 + 5, y0 + 5, x1 - 5, y0 + 17], fill=FRAME)
        d.line([(x0 + 5, y0 + 17), (x1 - 5, y0 + 17)], fill=FRAME_LO)
        page_arrow(d, x0 + 8, y0 + 7, False)
        page_arrow(d, x1 - 12, y0 + 7, True, 0 <= fr - seg.card < 6)
        if shot:
            cursor(d, x0 + 8, ROW0 + row * ROW_H + 5, fr)
        lit = max([1 - (fr - h) / 4 for h in (shot.hits if shot else ()) if 0 <= fr - h < 4] or [0.0])
        window(d, GAME, lit)
        if shot:
            scene, i = shot.sample(fr)
            small.paste(kit.shot(scene, i), (GAME[0] + 5, GAME[1] + 5))
        info_box(d)
    im = small.resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
    d = ImageDraw.Draw(im)
    text(d, (SAFE_X + 2, 40), label, 40, WHITE, True, 'lm', shade=EDGE)
    if recap:
        draw_recap(im, d, fr)
        return im
    x0, y0, x1, _ = LIST
    text(d, ((x0 + x1) / 2 * S, (y0 + 11) * S), f'{seg.number} / 03', 24, WHITE, True, 'mm')
    if fr - seg.card >= 2 * BEAT:
        for i, sh in enumerate(seg.shots):
            text(d, ((x0 + 19) * S, (ROW0 + i * ROW_H + 9) * S), sh.row, 24, WHITE, anchor='lm', shade=SHADE,
                 width=(x1 - x0 - 26) * S)
    ix0, iy0, ix1, _ = INFO
    text(d, ((ix0 + 5) * S, (iy0 + 5) * S + 1), 'INFORMATION', 24, WHITE, True, 'lm')
    if shot:
        text(d, (SAFE_X + 8, (iy0 + 24) * S), shot.caption, 32, INK, anchor='lm', width=(ix1 - ix0 - 20) * S)
    else:
        # the section card: the feature's number and name in its colour, in the game's window
        gx, gy = (GAME[0] + 5) * S, (GAME[1] + 5) * S
        k = fr - seg.card
        a = appear(fr, seg.card)
        text(d, (gx + 360, gy + 170), seg.number, 96, lerp(FIELD, seg.colour, a), True, 'mm')
        words = seg.label.split(' ')
        half = (len(words) + 1) // 2
        for j, line in enumerate((' '.join(words[:half]), ' '.join(words[half:]))):
            if line and k >= 2:
                text(d, (gx + 360, gy + 290 + 60 * j), line, 48, lerp(FIELD, WHITE, appear(fr, seg.card + 2)), True, 'mm',
                     width=660)
    return im


def draw_recap(im, d, fr):
    """three tiles, a picture and its label each, one a beat"""
    w, h = 240, 160
    gap = 90
    x0 = (W - 3 * w - 2 * gap) // 2
    y0 = 230
    for i, ((scene, sample), label) in enumerate(TILES):
        at = RECAP + i * BEAT
        if fr < at:
            continue
        x = x0 + i * (w + gap)
        d.rectangle([x - 15, y0 - 15, x + w + 14, y0 + h + 14], fill=FRAME_LO)
        d.rectangle([x - 15, y0 - 15, x + w + 11, y0 + h + 11], fill=FRAME_HI)
        d.rectangle([x - 12, y0 - 12, x + w + 11, y0 + h + 11], fill=FRAME)
        d.rectangle([x - 3, y0 - 3, x + w + 2, y0 + h + 2], fill=FRAME_LO)
        im.paste(kit.shot(scene, sample), (x, y0))
        words = label.split(' ')
        half = (len(words) + 1) // 2
        for j, line in enumerate((' '.join(words[:half]), ' '.join(words[half:]))):
            text(d, (x + w / 2, y0 + h + 66 + 42 * j), line, 24, WHITE, True, 'mm', shade=SHADE, width=w + 70)


# ---- the frame ----

def compose(only=None):
    frames_dir = os.path.join(kit.WORK, 'frames')
    if only is None:
        shutil.rmtree(frames_dir, ignore_errors=True)
    os.makedirs(frames_dir, exist_ok=True)
    segs = segments()
    black = Image.new('RGB', (W, H))
    for fr in range(FRAMES):
        if only is not None and fr not in only:
            continue
        pet = F(4) <= fr < STINGER
        im = draw_pet(segs, fr) if pet else draw_big(fr)
        # BN6's fades: up from black as the screen opens, and quickly out
        # and in where the screen changes (the title to the first section,
        # the recap to the stinger), in the GBA's sixteen steps
        dark = 1 - steps((fr + 1) / 12)
        for at in (F(4), STINGER):
            if at - 4 <= fr < at:
                dark = max(dark, (fr - at + 5) / 5)
            elif at <= fr < at + 4:
                dark = max(dark, 1 - (fr - at + 1) / 5)
        if dark > 0:
            im = Image.blend(im, black, min(1.0, steps(dark)))
        im.save(os.path.join(frames_dir, f'{fr:05d}.png'), compress_level=1)
        if fr % 100 == 0:
            print('frame', fr, '/', FRAMES, flush=True)


POSTER = F(3, 3)        # (the title card held: the logo, FIGHT ON?, 0.16.0 · OUT NOW)
