"""0.20.0's trailer, RED SUN JOINS: BN4 Red Sun beside BN6 (CHANGELOG.md's
0.20.0): its ROM offered on the ROMs screen, its five net areas, six of
its virus families visiting battles, the tournament (an act's climax on
BN4's own bracket screen, three Navis in a row) and BN4's Navis in it,
GutsMan, ThunderMan, WindMan, FireMan, SearchMan, AquaMan and Roll; the
Glitch layer and EXTRAS in the recap. In the skill's structure (SKILL.md,
The structure): a cold open of two bars with no words, the title card,
four segments (a section card of a bar, then a shot a bar), the recap,
the stinger on the music's cadence and the end card held.

The idea: the release is a tournament, and the trailer its bracket. The
music is v0_20_0_music.py, an original piece in the manner of BN4's own
tournament theme (song 0x17). The look is BN4's bracket screen as the
project's PET draws its windows (tools/site_art.py's cyan frames on navy)
with Red Sun's red, all drawn here:
- The field is the net's navy with its grid, and low behind the bracket
  the red sun the release's game is named for, faint.
- The right column is a bracket of four entrants, one a segment: each a
  plate with its segment's picture (a whole frame, small) and its number,
  in grey until its round, framed red while it plays, gold once won, its
  line climbing to the next round in gold; the final's line reaches the
  logo at the stinger.
- The game's window holds the shot at 3x. Every cut is the bracket's line
  reaching it: a red line runs in from the bracket's side and the new
  picture is drawn behind it, right to left, the window's frame flashing
  red.
- The section label is a red ROUND tab in the top left corner; the
  caption is one line on a navy plate under the window, its red tab at
  the left.
- Type: Nimbus Sans Narrow Bold (URW's), upright, a scoreboard's
  condensed capitals; the site's address in the same face.
None of 0.19.0's devices (the LCD stripes, the pill tabs, the refresh as
the cut, the setting's spinner) nor 0.17.0's (the battle's screens, the
chip window, the Program Advance) nor 0.14.0's slanted seam.

Its shots are whole frames of the game running from the developer's ROMs:
the ROMs screen with BN4's cartridge, BN4's areas walked by the autopilot
(x15 to x19), its viruses and its Navis by the foes step in its areas'
battles, BN4's bracket screen by the bracket step (docs/devtools/
battle-on-demand.md), the setup's EXTRAS page and a Glitch warp.

    python3 build.py host && python3 tools/trailer.py 0.20.0 [--keep] [--frames A,B]
"""
import os
import shutil
import sys

from PIL import Image, ImageDraw, ImageFont, ImageOps

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer as kit  # noqa: E402
from trailer import ROOT  # noqa: E402
from trailers import v0_20_0_music as music  # noqa: E402

VERSION = '0.20.0'
PLAY_LABEL = 'WATCH THE 0.20.0 TRAILER'
BN6_ONLY = ()


# ---- the footage ----

AUTO = {'CYBERWORLD_AUTOPILOT': '1'}
PAGED = '100:,' + '4:A,6:,' * 17
BN4_NET = ['--scene', 'emu', '--seed', '4', '--dev', 'quiet,god']
# the bracket step's eight entrants, each its plate, flags and seed
# (docs/devtools/battle-on-demand.md; bn4_tournament.h's BN4_PLATE_*):
# MegaMan, GutsMan, ThunderMan, WindMan, FireMan, SearchMan, AquaMan and
# Roll, all in; then after a round, MegaMan and three Navis through
R1 = '1e4000/004001/084002/054003/014004/094005/034006/044007'
R2 = '1e4100/008001/084102/058003/014104/098005/034106/048007'


def area(biome, seed):
    """one of BN4's areas walked by the autopilot"""
    return (['--scene', 'emu', '--net-biome', biome, '--run-depth', '3', '--seed', str(seed), '--dev', 'quiet,god'],
            AUTO, None, 150, 900)


def fight(biome, foes, last=1500):
    """named foes fought in a BN4 area's battle by the autopilot"""
    return (BN4_NET + ['--net-biome', biome], AUTO, f'100:,0:foes {foes}', 150, last)


def scenes():
    return {
        'roms': (['--scene', 'launcher'], {}, None, 0, 300),
        'setup': (['--scene', 'setup'], {}, '60:,6:R,40:,6:R,60:,6:DOWN,24:,6:DOWN,24:,6:A,40:,6:DOWN,24:,6:A,80:', 0, 420),
        'x15': area('x15', 3), 'x16': area('x16', 4), 'x17': area('x17', 3), 'x18': area('x18', 5), 'x19': area('x19', 3),
        # Billy, Spikey, Shrimpy; Walla, Elmperor, CirKill (docs/devtools/dev-menu.md's ids)
        'vir-a': fight('x15', '2C0@42/2C6@53/2CC@63'),
        'vir-b': fight('x17', '2D2@42/2D8@53/2DE@63'),
        # BN4's Navis by their V1 ids (src/layer/xguardian_bn4.c, XN_BN4_FIRST_ID 0x380 + 4 a seat)
        'navi-guts': fight('x15', '384@52'), 'navi-thunder': fight('x16', '394@52'),
        'navi-wind': fight('x17', '388@52'), 'navi-fire': fight('x18', '390@52'),
        'navi-search': fight('x19', '38C@52'), 'navi-aqua': fight('x16', '3A8@52'),
        'navi-roll': fight('x15', '380@52'),
        'bracket-1': (BN4_NET + ['--net-biome', 'x15'], {}, PAGED + f'0:bracket 0 {R1},400:', 150, 560),
        'bracket-2': (BN4_NET + ['--net-biome', 'x15'], {}, PAGED + f'0:bracket 0 {R2},400:', 150, 560),
        'glitch': (['--scene', 'emu', '--run-depth', '4', '--seed', '3', '--dev', 'quiet,glitch'], {},
                   PAGED + '0:glitch,300:', 100, 500),
    }


# ---- the look ----

FPS, W, H = 30, 1280, 720
BAR = round(FPS * 60 * 4 / music.BPM)     # (50 frames at 144 beats a minute)
BEAT = BAR / 4
FRAMES = music.BARS * BAR + round(music.TAIL * FPS)

FACE = '/usr/share/fonts/opentype/urw-base35/NimbusSansNarrow-Bold.otf'

# tools/site_art.py's PET colours and Red Sun's
FRAME_HI, FRAME, FRAME_LO = (107, 222, 255), (66, 198, 231), (16, 140, 239)
NAVY, GRID = (8, 18, 46), (16, 34, 74)
WHITE, SOFT, INK = (247, 255, 247), (198, 214, 239), (6, 12, 30)
RED, RED_HI, RED_LO = (222, 36, 48), (255, 96, 72), (120, 14, 28)
SUN = (58, 14, 34)
GOLD, GOLD_LO = (255, 214, 16), (176, 120, 8)
LINE_OFF = (52, 72, 118)
PLATE, PLATE_EDGE = (14, 30, 72), (156, 206, 239)

PIC = (64, 96)                   # the game's picture at 3x: 720 x 480
CAP = (64, 606, 784, 666)        # the caption's plate
LABEL = (64, 38)
BIG = (160, 40)                  # the big window's picture at 4x
CUT = 7                          # the bracket line's run into the window, in frames
PLATES_X, PLATE_W, PLATE_H = 840, 144, 112
SEMI_X, FINAL_X = 1040, 1150


def F(bar, beat=0.0):
    return round(bar * BAR + beat * BEAT)


_fonts = {}


def font(size):
    if size not in _fonts:
        _fonts[size] = ImageFont.truetype(FACE, size)
    return _fonts[size]


def text(d, xy, words, size, fill, anchor='la', shade=None, width=None):
    """the condensed capitals, shrunk to `width` where given"""
    while width and size > 14 and font(size).getlength(words) > width:
        size -= 1
    if shade:
        d.text((xy[0] + 3, xy[1] + 3), words, font=font(size), fill=shade, anchor=anchor)
    d.text(xy, words, font=font(size), fill=fill, anchor=anchor)


def lerp(a, b, t):
    t = max(0.0, min(1.0, t))
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


_field = None


def field():
    """the net's navy and its grid, the red sun low behind the bracket"""
    global _field
    if _field is None:
        im = Image.new('RGB', (W, H), NAVY)
        d = ImageDraw.Draw(im)
        for r, c in ((420, (24, 16, 50)), (330, (40, 16, 44)), (250, SUN)):
            d.ellipse([1030 - r, 820 - r, 1030 + r, 820 + r], fill=c)
        for x in range(0, W, 40):
            d.line([(x, 0), (x, H)], fill=GRID)
        for y in range(0, H, 40):
            d.line([(0, y), (W, y)], fill=GRID)
        _field = im
    return _field.copy()


def window(d, box, lit=0.0, k=3):
    """the PET's cyan window, its frame flashed red by `lit` (0-1)"""
    x0, y0, x1, y1 = box
    hi, mid, lo = (lerp(c, t, lit) for c, t in ((FRAME_HI, RED_HI), (FRAME, RED), (FRAME_LO, RED_LO)))
    d.rectangle([x0, y0, x1, y1], fill=lo)
    d.rectangle([x0, y0, x1 - k, y1 - k], fill=hi)
    d.rectangle([x0 + k, y0 + k, x1 - k, y1 - k], fill=mid)
    d.rectangle([x0 + 3 * k, y0 + 3 * k, x1 - 3 * k, y1 - 3 * k], fill=lo)


# ---- the timeline ----

class Shot:
    """a bar's shot: its caption and its feed [(frame, scene, sample)],
    each scene played from that sample at 30 fps from that frame on"""

    def __init__(self, start, caption, feed):
        self.start, self.caption, self.feed = start, caption, feed

    def sample(self, fr):
        e = max((e for e in self.feed if e[0] <= fr), default=self.feed[0])
        return e[1], e[2] + max(0, fr - e[0])


class Segment:
    def __init__(self, card, number, label, title, thumb, shots):
        self.card, self.number, self.label, self.title, self.thumb, self.shots = card, number, label, title, thumb, shots


TITLE, SEG1, SEG2, SEG3, SEG4, RECAP, STINGER, END = F(2), F(4), F(8), F(11), F(15), F(19), F(21), F(22)


def beats(start, scenes):
    """a feed cutting on the beat: (scene, sample, beats held)"""
    feed, b = [], 0
    for scene, sample, n in scenes:
        feed.append((start + F(0, b), scene, sample))
        b += n
    return feed


def segments():
    return [
        Segment(SEG1, '1', 'BN4 RED SUN', 'RED SUN JOINS', ('x16', 60), [
            Shot(F(5), 'A THIRD ROM, BESIDE BN6', [(F(5), 'roms', 20)]),
            Shot(F(6), 'FIVE OF ITS NET AREAS', beats(F(6), (('x15', 60, 2), ('x16', 120, 2)))),
            Shot(F(7), 'IN ITS OWN TILES AND MUSIC', beats(F(7), (('x18', 80, 2), ('x19', 60, 2)))),
        ]),
        Segment(SEG2, '2', 'ITS VIRUSES', 'ITS VIRUSES VISIT', ('vir-a', 230), [
            Shot(F(9), 'SIX OF ITS VIRUS FAMILIES', [(F(9), 'vir-a', 250)]),
            Shot(F(10), 'ONE BATTLE IN FOUR, FROM ACT 2', [(F(10), 'vir-b', 230)]),
        ]),
        Segment(SEG3, '3', 'THE TOURNAMENT', 'THE TOURNAMENT', ('bracket-1', 80), [
            Shot(F(12), "BN4'S OWN BRACKET, EVERY FACE", [(F(12), 'bracket-1', 70)]),
            Shot(F(13), 'THREE NAVIS IN A ROW', [(F(13), 'bracket-2', 70)]),
            Shot(F(14), 'AN ACT\'S CLIMAX, NO HEAL BETWEEN', [(F(14), 'navi-guts', 226)]),
        ]),
        Segment(SEG4, '4', "BN4'S NAVIS", "BN4'S NAVIS", ('navi-thunder', 240), [
            Shot(F(16), 'GUTSMAN, THUNDERMAN, WINDMAN', beats(F(16), (('navi-thunder', 226, 2), ('navi-wind', 236, 2)))),
            Shot(F(17), 'FIREMAN, SEARCHMAN, AQUAMAN', beats(F(17), (('navi-fire', 236, 2), ('navi-search', 236, 1),
                                                                    ('navi-aqua', 236, 1)))),
            Shot(F(18), 'AND ROLL: SEVEN NAVIS IN ITS DRAW', [(F(18), 'navi-roll', 236)]),
        ]),
    ]


# the recap: (scene, sample, played or held), its line, at 2x (the setup's
# page held on its first line: its Tournament line's own words say what a
# build without the faces says)
TILES = [(('glitch', 140, 1), 'GLITCH LAYERS'),
         (('setup', 70, 0), 'EXTRAS: SWITCH THEM OFF')]
RECAP_CAPTION = 'AND MORE IN THE CHANGELOG'
SYSTEMS = ('PortMaster handhelds · Linux · Windows · macOS · Android',
           'iPhone and iPad · New 3DS · Switch · PS Vita · PS4 · PS5 · the browser')
SITE = 'saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless'


def where(segs, fr):
    seg = None
    for s in segs:
        if s.card <= fr:
            seg = s
    shot = None
    for sh in seg.shots:
        if sh.start <= fr:
            shot = sh
    return seg, shot


# ---- the pictures ----

def picture(scene, i, scale=3):
    im = kit.shot(scene, i)
    if scene == 'roms':
        # (the ROMs screen's line naming the data folder, a dev run's
        # path, covered in its own background)
        im = im.copy()
        ImageDraw.Draw(im).rectangle([0, 116, 239, 128], fill=im.getpixel((2, 110)))
    return im.resize((240 * scale, 160 * scale), Image.NEAREST)


_thumbs = {}


def thumb(scene, i, grey):
    key = (scene, i, grey)
    if key not in _thumbs:
        im = kit.shot(scene, i).resize((120, 80), Image.LANCZOS)
        if grey:
            im = Image.blend(ImageOps.grayscale(im).convert('RGB'), Image.new('RGB', im.size, NAVY), 0.45)
        _thumbs[key] = im
    return _thumbs[key]


def run_in(im, new, old, k, box):
    """the cut: the bracket's red line running in from the right, the new
    picture drawn behind it"""
    x, y = box
    w, h = new.size
    if old is not None:
        im.paste(old, (x, y))
    edge = round(w * (1 - min(1.0, (k + 1) / CUT)))
    im.paste(new.crop((edge, 0, w, h)), (x + edge, y))
    if k + 1 < CUT:
        d = ImageDraw.Draw(im)
        d.rectangle([x + edge - 4, y, x + edge + 3, y + h - 1], fill=RED_HI)
        d.rectangle([x + edge + 4, y, x + edge + 7, y + h - 1], fill=GOLD)


# ---- the bracket ----

def plate_y(i):
    return 96 + i * 132


def plate_state(segs, i, fr):
    """'wait', 'play' or 'won'"""
    seg = segs[i]
    if fr < seg.card:
        return 'wait'
    nxt = segs[i + 1].card if i + 1 < len(segs) else RECAP
    return 'play' if fr < nxt else 'won'


def bracket(im, d, segs, fr):
    """four plates, their lines to two semis and the final, gold as their
    rounds are won"""
    mids = []
    for i, seg in enumerate(segs):
        y = plate_y(i)
        st = plate_state(segs, i, fr)
        edge = {'wait': LINE_OFF, 'play': RED, 'won': GOLD}[st]
        if st == 'play':
            edge = lerp(RED, RED_HI, abs((fr - seg.card) % BAR - BAR / 2) / (BAR / 2))
        x0 = PLATES_X
        d.rectangle([x0 - 4, y - 4, x0 + PLATE_W + 4, y + PLATE_H + 4], fill=edge)
        d.rectangle([x0, y, x0 + PLATE_W, y + PLATE_H], fill=PLATE)
        im.paste(thumb(*seg.thumb, st == 'wait'), (x0 + 12, y + 8))
        text(d, (x0 + 12, y + 92), seg.number, 22, GOLD if st != 'wait' else SOFT, 'lm')
        text(d, (x0 + 32, y + 92), seg.label, 18, WHITE if st != 'wait' else SOFT, 'lm', width=PLATE_W - 40)
        mids.append(y + PLATE_H // 2)
    # the lines: each plate to its semi, gold once its round is won
    for pair in ((0, 1), (2, 3)):
        sy = (mids[pair[0]] + mids[pair[1]]) // 2
        for i in pair:
            won = plate_state(segs, i, fr) == 'won'
            c = GOLD if won else LINE_OFF
            d.line([(PLATES_X + PLATE_W + 4, mids[i]), (SEMI_X, mids[i])], fill=c, width=6)
            d.line([(SEMI_X, mids[i]), (SEMI_X, sy)], fill=c, width=6)
        both = all(plate_state(segs, i, fr) == 'won' for i in pair)
        d.line([(SEMI_X, sy), (FINAL_X, sy)], fill=GOLD if both else LINE_OFF, width=6)
    s0 = (mids[0] + mids[1]) // 2
    s1 = (mids[2] + mids[3]) // 2
    allwon = all(plate_state(segs, i, fr) == 'won' for i in range(4))
    d.line([(FINAL_X, s0), (FINAL_X, s1)], fill=GOLD if allwon else LINE_OFF, width=6)
    fy = (s0 + s1) // 2
    d.line([(FINAL_X, fy), (FINAL_X + 50, fy)], fill=GOLD if allwon else LINE_OFF, width=6)
    r = 16
    cx = FINAL_X + 50 + r
    d.polygon([(cx - r, fy), (cx, fy - r), (cx + r, fy), (cx, fy + r)], fill=GOLD if allwon else LINE_OFF,
              outline=WHITE if allwon else PLATE_EDGE)


# ---- the page: the segments and the recap ----

def label(d, number, words, tab=None):
    x, y = LABEL
    tab = tab or f'ROUND {number}'
    w = font(24).getlength(tab) + 28
    d.rectangle([x, y, x + w, y + 36], fill=RED)
    text(d, (x + 14, y + 18), tab, 24, WHITE, 'lm')
    text(d, (x + w + 16, y + 18), words, 24, WHITE, 'lm')


def caption(d, words):
    x0, y0, x1, y1 = CAP
    d.rectangle([x0, y0, x1, y1], fill=PLATE_EDGE)
    d.rectangle([x0 + 2, y0 + 2, x1 - 2, y1 - 2], fill=PLATE)
    d.rectangle([x0 + 2, y0 + 2, x0 + 14, y1 - 2], fill=RED)
    text(d, (x0 + 34, (y0 + y1) // 2), words, 38, WHITE, 'lm', width=x1 - x0 - 60)


def draw_page(segs, fr):
    im = field()
    d = ImageDraw.Draw(im)
    seg, shot = where(segs, fr)
    bracket(im, d, segs, fr)
    if shot is not None:
        changes = [e[0] for e in shot.feed]
        last = max(c for c in changes if c <= fr)
        k = fr - last
        lit = max(0.0, 1 - k / CUT) if k < CUT else 0.0
    else:
        k, lit = fr - seg.card, max(0.0, 1 - (fr - seg.card) / CUT)
    window(d, (PIC[0] - 12, PIC[1] - 12, PIC[0] + 720 + 11, PIC[1] + 480 + 11), lit)
    if shot is None:
        new = card_picture(seg, fr)
        old = None
        if segs.index(seg) > 0:
            prev = segs[segs.index(seg) - 1].shots[-1]
            old = picture(*prev.sample(seg.card - 1))
        run_in(im, new, old, k, PIC) if k < CUT else im.paste(new, PIC)
    else:
        new = picture(*shot.sample(fr))
        if k < CUT:
            if last == shot.start:
                idx = seg.shots.index(shot)
                old = picture(*seg.shots[idx - 1].sample(shot.start - 1)) if idx else card_picture(seg, shot.start - 1)
            else:
                old = picture(*shot.sample(last - 1))
            run_in(im, new, old, k, PIC)
        else:
            im.paste(new, PIC)
    d = ImageDraw.Draw(im)
    label(d, seg.number, seg.label)
    caption(d, shot.caption if shot is not None else '')
    return im


def card_picture(seg, fr):
    """the section card in the game's window: ROUND and its number on the
    first beat, the feature's name on the second (never two at once)"""
    card = Image.new('RGB', (720, 480), INK)
    cd = ImageDraw.Draw(card)
    for y in range(0, 480, 24):
        cd.line([(0, y), (720, y)], fill=(14, 26, 58))
    cd.rectangle([0, 150, 720, 330], fill=RED_LO)
    cd.rectangle([0, 150, 720, 156], fill=RED)
    cd.rectangle([0, 324, 720, 330], fill=RED)
    text(cd, (360, 110), f'ROUND {seg.number}', 52, GOLD, 'mm', shade=INK)
    if fr - seg.card >= BEAT:
        text(cd, (360, 240), seg.title, 84, WHITE, 'mm', shade=INK, width=660)
    return card


def draw_recap(fr):
    """the tiles at 2x in one wide window, one a beat, a line under each"""
    im = field()
    d = ImageDraw.Draw(im)
    label(d, '', f'ALSO IN {VERSION}', tab='MORE')
    k = fr - RECAP
    window(d, (52, 84, 1228, 588), max(0.0, 1 - k / CUT))
    w, h, gap = 480, 320, 64
    x0 = W // 2 - (2 * w + gap) // 2
    y0 = 118
    for i, ((scene, sample, rate), line) in enumerate(TILES):
        if fr < RECAP + i * BEAT:
            continue
        x = x0 + i * (w + gap)
        d.rectangle([x - 6, y0 - 6, x + w + 5, y0 + h + 5], fill=GOLD)
        im.paste(kit.shot(scene, sample + rate * (fr - RECAP)).resize((w, h), Image.NEAREST), (x, y0))
        text(d, (x + w / 2, y0 + h + 52), line, 36, WHITE, 'mm', shade=INK, width=w + 40)
    caption(d, RECAP_CAPTION)
    return im


# ---- the big window: the cold open, the title, the stinger, the end ----

_logo = {}


def logo(width):
    if width not in _logo:
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        _logo[width] = lg.resize((width, round(lg.height * width / lg.width)), Image.LANCZOS)
    return _logo[width]


def big_frame(lit=0.0):
    im = field()
    d = ImageDraw.Draw(im)
    window(d, (BIG[0] - 12, BIG[1] - 12, BIG[0] + 960 + 11, BIG[1] + 640 + 11), lit)
    return im, d


# the cold open: BN4's bracket with every face, then ThunderMan's fight
COLD = [(0, 'bracket-1', 66), (F(1), 'navi-thunder', 226)]


def draw_cold(fr):
    start, scene, first = max(e for e in COLD if e[0] <= fr)
    k = fr - start
    im, d = big_frame(max(0.0, 1 - k / CUT) if start else 0.0)
    new = picture(scene, first + k, 4)
    if start and k < CUT:
        run_in(im, new, picture(COLD[0][1], COLD[0][2] + start - 1, 4), k, BIG)
    else:
        im.paste(new, BIG)
    return im


def draw_title(fr):
    """the logo, RED SUN JOINS and the version on the red tab"""
    k = fr - TITLE
    im, d = big_frame(max(0.0, 1 - k / CUT))
    d.rectangle([BIG[0], BIG[1], BIG[0] + 959, BIG[1] + 639], fill=INK)
    for y in range(BIG[1], BIG[1] + 640, 24):
        d.line([(BIG[0], y), (BIG[0] + 959, y)], fill=(14, 26, 58))
    lg = logo(640)
    im.paste(lg, (W // 2 - 320, 110), lg)
    if k < CUT:
        cover = picture(COLD[1][1], COLD[1][2] + TITLE - 1 - F(1), 4)
        edge = round(960 * (1 - (k + 1) / CUT))
        im.paste(cover.crop((0, 0, edge, 640)), BIG)
        d.rectangle([BIG[0] + edge - 4, BIG[1], BIG[0] + edge + 3, BIG[1] + 639], fill=RED_HI)
    if k >= BEAT:
        text(d, (W // 2, 110 + lg.height + 76), 'RED SUN JOINS', 92, lerp(INK, WHITE, (k - BEAT + 1) / 4), 'mm',
             shade=RED_LO)
    if k >= 2 * BEAT:
        box = (W // 2 - 230, 486, W // 2 + 230, 548)
        d.rectangle(box, fill=RED)
        d.rectangle([box[0], box[3] - 6, box[2], box[3]], fill=RED_LO)
        text(d, (W // 2, 516), f'{VERSION} · OUT NOW', 44, WHITE, 'mm')
    return im


def draw_stinger(segs, fr):
    """the final: the bracket's lines from both sides meeting in the logo,
    the version under it"""
    k = fr - STINGER
    im, d = big_frame()
    d.rectangle([BIG[0], BIG[1], BIG[0] + 959, BIG[1] + 639], fill=INK)
    t = min(1.0, (k + 1) / CUT)
    cy = 330
    reach = round((W // 2 - 360 - BIG[0]) * t)
    for x0, sgn in ((BIG[0], 1), (BIG[0] + 959, -1)):
        d.line([(x0, cy), (x0 + sgn * reach, cy)], fill=GOLD, width=8)
    if k >= CUT - 2:
        lg = logo(700)
        im.paste(lg, (W // 2 - 350, cy - lg.height // 2 - 40), lg)
        text(d, (W // 2, cy + lg.height // 2 + 50), VERSION, 72, GOLD, 'mm', shade=INK)
    return im


def draw_end(fr):
    """the end card, held still: the logo, the version on the red tab, where
    to play it, the site and the ROM lines"""
    im, d = big_frame()
    d.rectangle([BIG[0], BIG[1], BIG[0] + 959, BIG[1] + 639], fill=INK)
    k = fr - END
    lg = logo(560)
    y = 76
    im.paste(lg, (W // 2 - 280, y), lg)
    y += lg.height + 40
    box = (W // 2 - 220, y, W // 2 + 220, y + 60)
    d.rectangle(box, fill=RED)
    d.rectangle([box[0], box[3] - 6, box[2], box[3]], fill=RED_LO)
    text(d, (W // 2, y + 30), f'{VERSION} · OUT NOW', 40, WHITE, 'mm')
    if k < BEAT // 2:
        return im
    y += 60 + 52
    for i, line in enumerate(SYSTEMS):
        text(d, (W // 2, y + 36 * i), line, 28, WHITE, 'mm', width=900)
    text(d, (W // 2, y + 98), SITE, 30, FRAME_HI, 'mm', width=900)
    foot = ("Needs your own Mega Man Battle Network 6: Cybeast Gregar (USA) ROM.",
            "BN5's and BN4's areas need Team Colonel (USA) and Red Sun (USA) beside it.",
            'An unofficial fan project, not affiliated with Capcom.')
    for i, line in enumerate(foot):
        text(d, (W // 2, y + 152 + 28 * i), line, 22, SOFT, 'mm', width=900)
    return im


# ---- the frame ----

def draw(segs, fr):
    if fr < TITLE:
        return draw_cold(fr)
    if fr < SEG1:
        return draw_title(fr)
    if fr < RECAP:
        return draw_page(segs, fr)
    if fr < STINGER:
        return draw_recap(fr)
    if fr < END:
        return draw_stinger(segs, fr)
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


POSTER = F(3, 3)        # (the title card held: the logo, RED SUN JOINS, 0.20.0 · OUT NOW)
