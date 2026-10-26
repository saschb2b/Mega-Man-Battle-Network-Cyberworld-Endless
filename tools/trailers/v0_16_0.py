"""0.16.0's trailer, FIGHT ON?: BN6's Virus Battler in the run, and the
Crosses a run holds on a dark layer's team. The release's news is a hand
the PET holds: the viruses on a BtlrCard (a KeyItem) picked from BN6's
VIRUS LIST, the Navis whose Crosses the run holds picked on the TEAM
screen. So the cut is a PET screen of its own, drawn in BN6's PET look
(docs/engine/ui.md, tools/site_art.py's colours): the band names the
shot, a list window holds the shots as its rows with the orange cursor
stepping down them on the beat, the game's window beside it shows the
row under the cursor, and the INFORMATION box says what it is. The
sections are the list's pages, turned by its green arrows; the dark
layer's two bars take the screen down a few of the GBA's brightness
steps; the rumble flashes the window's frame; the end asks the
Battler's own question, Fight on?, with the green triangle before Yes.
The screen fades up and out as BN6's do, in sixteen steps. The devices
the earlier trailers used (chrome type, typed briefing words, panels
freed, linked windows, a slanted seam, a war room's map and its pans)
are not here.

Its shots are whole frames of the game running from the developer's ROMs,
BN6's and BN5 Team Colonel's beside it: a Battler's series on seed 1's
first layer (`--dev battler=1`, the Virus Battler PR's own session:
MegaMan walking up to him, two rounds at fast forward, the pot taken),
the TEAM screen at act 3 with six Souls and SlashMan's Cross held, the
autopilot on act 2's dark board with each Cross brought (`--setup
endless,0,0,0,N`). Everything else is drawn here; the music is
v0_16_0_music.py's, an original piece in the manner of BN6's virus
battle theme, which a Battler's rounds play.

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
    }
    return s


# ---- the look: a PET screen ----
#
# Drawn at the PET's own scale, a 427 x 240 screen (the game's picture at
# 1x in it), and scaled 3x by nearest neighbour into the 1280 x 720 frame,
# so a PET pixel is three of the video's and every edge stays hard. Its
# parts and colours are BN6's PET screens' as docs/engine/ui.md measured
# them and tools/site_art.py draws the site's (the band, the cyan window,
# the information box, the orange cursor, the green choice, the page
# arrows); the words are the site's fonts (web/assets/fonts, OFL):
# Silkscreen for the band and labels, Pixelify Sans for the list and the
# information, drawn at the video's scale.

FPS, W, H = 30, 1280, 720
BAR = round(FPS * 60 * 4 / music.BPM)     # (48 frames at 150 beats a minute)
BEAT = BAR // 4
FRAMES = music.BARS * BAR + round(music.TAIL * FPS)

S = 3                               # video pixels to a PET pixel
LW, LH = 427, 240                   # the PET screen

FONTS = os.path.join(ROOT, 'web', 'assets', 'fonts')
CAPS = os.path.join(FONTS, 'silkscreen-700.woff2')
CAPS_THIN = os.path.join(FONTS, 'silkscreen-400.woff2')
CHAT = os.path.join(FONTS, 'pixelify-sans.woff2')

BAND = (0, 123, 49)
BODY = (8, 189, 115)
BODY_LINE = (74, 231, 115)
FRAME_HI, FRAME, FRAME_LO = (107, 222, 255), (66, 198, 231), (16, 140, 239)
FIELD = (16, 82, 107)
WHITE = (247, 255, 247)
SHADE = (33, 115, 140)
EDGE = (33, 74, 82)
INFO_BOX, PAGE, INK = (74, 231, 115), (247, 247, 247), (66, 57, 57)
ORANGE, ORANGE_DARK = (255, 173, 16), (214, 107, 0)
CHOICE, CHOICE_HI, CHOICE_OUT = (57, 255, 0), (115, 255, 74), (33, 165, 33)
ARROW, ARROW_HI, ARROW_DARK = (74, 198, 16), (189, 231, 132), (8, 165, 0)
CYAN_TEXT = (107, 222, 255)

BAND_H = 18
LIST = (7, 23, 161, 192)            # the list window (x0, y0, x1, y1)
GAME = (168, 23, 417, 192)          # the game's window: its picture at (173, 28)
INFO = (7, 196, 417, 236)           # the information box
ROW0, ROW_H = 43, 16                # the list's first row and its height


def F(bar, beat=0.0):
    return round(bar * BAR + beat * BEAT)


_fonts = {}


def font(path, size):
    if (path, size) not in _fonts:
        _fonts[(path, size)] = ImageFont.truetype(path, size)
    return _fonts[(path, size)]


def ease(t):
    t = max(0.0, min(1.0, t))
    return 1 - (1 - t) ** 3


def steps(t):
    """t (0-1) in the GBA's sixteen brightness steps"""
    return math.floor(max(0.0, min(1.0, t)) * 16) / 16


# ---- the shots: a row of the PET's list each ----

class Shot:
    """A row of the list: `row` its line, `head` the band's name while the
    cursor is on it, `text` the information box's, `feed` [(frame, scene,
    sample, rate)] the game's window (from each frame on, that scene's
    footage from that sample; rate 0 holds it), `hits` frames the window's
    frame flashes on (the rumble)."""

    def __init__(self, start, row, head, text, feed, hits=(), hint=''):
        self.start, self.row, self.head, self.text, self.feed = start, row, head, text, feed
        self.hits, self.hint = hits, hint

    def sample(self, fr):
        i = max([j for j, e in enumerate(self.feed) if e[0] <= fr] or [0])
        start, scene, first, rate = self.feed[i]
        return scene, first + max(0, int((fr - start) * rate))


class Page:
    """A page of the list: its name on the window's band, its shots"""

    def __init__(self, name, shots):
        self.name, self.shots = name, shots


def pages():
    return [
        Page('VIRUS BATTLER', [
            # the intro: MegaMan walks up to a Battler at a side path's end; his offer
            Shot(F(0, 2), 'The Battler', 'A VIRUS BATTLER', "A Mr.Prog in his own BN6 colour, at a side path's end",
                 [(F(0, 2), 'bv-series', 12, 1)]),
            Shot(F(2), 'His offer', "BN6'S VIRUS BATTLER", 'On the first layer of about half the acts, from the second',
                 [(F(2), 'bv-series', 86, 1)], hint='CYBERWORLD ENDLESS 0.16.0'),
            # the main section: the stake, the list, the fight, the pot
            Shot(F(4), 'The stake', 'STAKE YOUR ZENNY', '500 to 3000 by act; each win adds it again to the pot',
                 [(F(4), 'bv-series', 186, 1)]),
            Shot(F(5), 'VIRUS LIST', "YOUR BTLRCARD'S VIRUSES", 'Every Rare you delete joins it, this run and every run after',
                 [(F(5), 'bv-series', 440, 0), (F(5, 1.5), 'bv-series', 624, 1)]),
            Shot(F(6), 'Place them', 'PLACE THEM, THEN WATCH', "They fight his pair on their own; MegaMan's HP is untouched",
                 [(F(6), 'bv-series', 656, 1)]),
            Shot(F(7), 'The fight', 'THE FIGHT RUNS FAST', 'It takes no input, so fast forward can run through it',
                 [(F(7), 'bv-series', 768, 1)]),
            Shot(F(9), 'The pot', 'THE POT GROWS', 'A win adds the stake; a loss or a draw loses it all',
                 [(F(9), 'bv-series', 896, 1)]),
            Shot(F(10), 'Fight on?', 'TAKE THE POT, OR FIGHT ON?', 'He shows his next pair before you choose',
                 [(F(10), 'bv-series', 1006, 1), (F(10, 2), 'bv-series', 1062, 1)]),
            Shot(F(11), 'Cash out', 'CASH OUT', 'Two wins on a 500 stake: 1500 zenny', [(F(11), 'bv-series', 2296, 1)]),
        ]),
        # the drop: the dark layer's team; the climax: the commands, the Cross
        Page('LIBERATION TEAM', [
            Shot(F(12), 'Your Crosses', 'YOUR CROSSES JOIN THE TEAM', "On a dark layer's board, each in his own BN6 body",
                 [(F(12), 'heat', 16, 1)]),
            Shot(F(13), 'TEAM', 'PICK WHO COMES', 'Where you hold more Navis than the board has places',
                 [(F(13), 'team', 150, 1)]),
            Shot(F(14), 'Napalm', "HEATMAN'S NAPALM", 'Four ahead and both sides of the third, one battle',
                 [(F(14), 'heat', 122, 1), (F(14) + 12, 'heat', 134, 0)]),
            Shot(F(14, 2), 'MagnetBarrier', "ELECMAN'S MAGNETBARRIER", 'No harm on the board to the team till the next phase',
                 [(F(14, 2), 'elec', 45, 0), (F(14, 2) + 9, 'elec', 52, 1), (F(14, 2) + 15, 'elec', 58, 0)]),
            Shot(F(15), 'BombLiberation', "CHARGEMAN'S BOMBLIBERATION", 'He flies over the dark and frees the panel, no battle',
                 [(F(15), 'charge', 84, 0), (F(15) + 8, 'charge', 90, 1), (F(15) + 14, 'charge', 96, 0)]),
            Shot(F(15, 2), 'WideSwrd', "SLASHMAN'S WIDESWRD", 'Three across, one battle',
                 [(F(15, 2), 'slash', 148, 1), (F(15, 2) + 10, 'slash', 158, 0)]),
            Shot(F(16), 'HeatCross', 'HIS BATTLES, IN HIS CROSS', "MegaMan in HeatCross from the first turn, on HeatMan's HP",
                 [(F(16), 'heat', 327, 1)]),
        ]),
        Page('FEEL', [
            Shot(F(17), 'Rumble', 'FEEL EVERY HIT', "Rumble on a fight's beats: a hit, the charge, a delete",
                 [(F(17), 'heat', 476, 1)], hits=(F(17, 0.75), F(17, 1.25), F(17, 1.75))),
            Shot(F(17, 2), 'Triggers', "A DUALSENSE'S TRIGGERS", "On a DualSense, L's trigger clicks once the Custom gauge is full",
                 [(F(17, 2), 'heat', 500, 1)], hits=(F(17, 2.5),)),
        ]),
    ]


NEWS = ['Virus Battler', "Crosses' team", "Souls' chips", 'Rumble', 'Your Square']
END = F(18)


def where(book, fr):
    """(page, the shot under the cursor, its row) at frame fr; the shot is
    None before the first and from the end"""
    if fr >= END:
        return None, None, 0
    page, shot, row = book[0], None, 0
    for p in book:
        for i, s in enumerate(p.shots):
            if s.start <= fr:
                page, shot, row = p, s, i
    return page, shot, row


# ---- the PET's parts, at its scale ----

_floor = []


def body():
    """the PET's body under its windows: site_art.py's floor"""
    if not _floor:
        tile = Image.open(os.path.join(ROOT, 'web', 'assets', 'ui', 'floor.png')).convert('RGB')
        im = Image.new('RGB', (LW, LH), BODY)
        for x in range(0, LW, tile.width):
            for y in range(BAND_H, LH, tile.height):
                im.paste(tile, (x, y))
        _floor.append(im)
    return _floor[0].copy()


def band(d, name, hint):
    """the band across the top: the screen's name in heavy white capitals,
    two stripes running off to its right; a hint at its right end"""
    d.rectangle([0, 0, LW - 1, BAND_H - 1], fill=BAND)
    f = font(CAPS, 16)
    x1 = 8
    if name:
        d.text((8, -3), name, font=f, fill=WHITE)
        x1 = 8 + round(f.getlength(name)) + 8
    hw = round(font(CAPS_THIN, 8).getlength(hint)) + 10 if hint else 0
    for y, lead in ((6, 0), (11, 5)):
        x = x1 + lead
        d.line([(x, y + 4), (x + 4, y)], fill=BODY, width=1)
        d.line([(x + 4, y), (LW - 1 - hw, y)], fill=BODY, width=2)
    if hint:
        d.text((LW - 6, 9), hint, font=font(CAPS_THIN, 8), fill=WHITE, anchor='rm')


def window(d, box, lit=0.0):
    """the cyan window round a navy field; `lit` (0-1) its frame flashed"""
    x0, y0, x1, y1 = box
    hi, mid, lo = (lerp(c, (255, 255, 255), lit) for c in (FRAME_HI, FRAME, FRAME_LO))
    d.rectangle([x0, y0, x1, y1], fill=lo)
    d.rectangle([x0, y0, x1 - 1, y1 - 1], fill=hi)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=mid)
    d.rectangle([x0 + 4, y0 + 4, x1 - 4, y1 - 4], fill=lo)
    d.rectangle([x0 + 5, y0 + 5, x1 - 5, y1 - 5], fill=FIELD)


def lerp(a, b, t):
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def page_arrow(d, x, y, right, lit=False):
    """the green arrow where L or R turns the page"""
    body_, hi = (ARROW_HI, WHITE) if lit else (ARROW, ARROW_HI)
    pts = [(x, y), (x, y + 8), (x + 4, y + 4)] if right else [(x + 4, y), (x + 4, y + 8), (x, y + 4)]
    d.polygon(pts, fill=body_, outline=ARROW_DARK)
    d.line([pts[0], pts[2]], fill=hi)


def cursor(d, x, y, fr):
    """the orange arrow at a row's left, never still: one pixel right, back,
    two right, three video frames each (KeyItem's six of the game's)"""
    x += (0, 1, 0, 2)[(fr // 3) % 4]
    d.polygon([(x, y), (x, y + 8), (x + 5, y + 4)], fill=ORANGE, outline=ORANGE_DARK)
    d.line([(x + 1, y + 2), (x + 1, y + 6)], fill=(255, 206, 24))


def triangle(d, x, y):
    """the green triangle before the chosen answer"""
    d.polygon([(x, y), (x, y + 8), (x + 5, y + 4)], fill=CHOICE, outline=CHOICE_OUT)
    d.line([(x + 1, y + 1), (x + 1, y + 7)], fill=CHOICE_HI)


def list_window(d, name, rows, row, fr, turned=None):
    """the list: its page's name on the window's band between the page
    arrows, its rows, the cursor on `row` (None: no cursor); `turned` the
    arrow just pressed ('L' or 'R')"""
    window(d, LIST)
    x0, y0, x1, _ = LIST
    d.rectangle([x0 + 5, y0 + 5, x1 - 5, y0 + 16], fill=FRAME)
    d.line([(x0 + 5, y0 + 16), (x1 - 5, y0 + 16)], fill=FRAME_LO)
    d.text(((x0 + x1) / 2, y0 + 11), name, font=font(CAPS, 8), fill=WHITE, anchor='mm')
    page_arrow(d, x0 + 8, y0 + 7, False, turned == 'L')
    page_arrow(d, x1 - 12, y0 + 7, True, turned == 'R')
    if row is not None:
        cursor(d, x0 + 8, ROW0 + row * ROW_H + 4, fr)


def info_box(d):
    x0, y0, x1, y1 = INFO
    d.rectangle([x0, y0, x1, y1], fill=EDGE)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=INFO_BOX)
    d.text((x0 + 5, y0 + 5), 'INFORMATION', font=font(CAPS, 8), fill=WHITE, anchor='lm', stroke_width=0)
    d.rectangle([x0 + 3, y0 + 10, x1 - 3, y1 - 3], fill=PAGE)


# ---- the words, at the video's scale ----

def chat(d, xy, text, size, fill, shade=None, anchor='la', width=None):
    """Pixelify Sans, its ligatures off (its fi is no letter of the PET's),
    shrunk to `width` where given"""
    while width and size > 12 and font(CHAT, size).getlength(text) > width:
        size -= 1
    f = font(CHAT, size)
    if shade:
        d.text((xy[0] + S, xy[1] + S), text, font=f, fill=shade, anchor=anchor, features=['-liga'])
    d.text(xy, text, font=f, fill=fill, anchor=anchor, features=['-liga'])


def rows_text(d, rows):
    x0 = LIST[0]
    for i, r in enumerate(rows):
        y = (ROW0 + i * ROW_H) * S
        f = 31
        while font(CHAT, f).getlength(r) > (LIST[2] - LIST[0] - 26) * S:
            f -= 1
        chat(d, ((x0 + 19) * S, y + 2 * S), r, f, WHITE, SHADE)


def info_text(d, text):
    x0, y0, x1, y1 = INFO
    f = 33
    while font(CHAT, f).getlength(text) > (x1 - x0 - 14) * S:
        f -= 1
    chat(d, ((x0 + 8) * S, (y0 + 22) * S), text, f, INK, anchor='lm')


# ---- the end: the logo in the game's window, the question in the box ----

_logo = []


def logo():
    if not _logo:
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        w = 600
        _logo.append(lg.resize((w, round(lg.height * w / lg.width)), Image.LANCZOS))
    return _logo[0]


def ending_small(d, fr):
    """the end's PET parts: Yes and No's triangle"""
    if fr >= END + 10:
        triangle(d, INFO[0] + 8, INFO[1] + 25)


def ending_big(im, d, fr):
    """the end's words and the logo, at the video's scale"""
    k = fr - END
    gx, gy = (GAME[0] + 5) * S, (GAME[1] + 5) * S
    if k >= 4:
        lg = logo()
        im.paste(lg, (gx + (720 - lg.width) // 2, gy + 70), lg)
        chat(d, (gx + 360, gy + 270), 'saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless', 24, CYAN_TEXT,
             anchor='mm', width=680)
    if k >= 16:
        for i, n in enumerate(('Unofficial fan project, not affiliated with Capcom. Free; it runs',
                               'from your own Mega Man Battle Network 6: Cybeast Gregar (USA) ROM.',
                               'The dark boards need Mega Man Battle Network 5: Team Colonel (USA) beside it.')):
            chat(d, (gx + 360, gy + 352 + 30 * i), n, 19, WHITE, anchor='mm', width=680)
    x0, y0 = INFO[0] * S, INFO[1] * S
    if k >= 4:
        chat(d, (x0 + 8 * S, y0 + 16 * S), 'Fight on?', 33, INK, anchor='lm')
    if k >= 10:
        chat(d, (x0 + 16 * S, y0 + 29 * S), 'Yes', 33, INK, anchor='lm')
        chat(d, (x0 + 46 * S, y0 + 29 * S), 'No', 33, INK, anchor='lm')


# ---- the frame ----

def dim_at(fr):
    """the dark layer's two bars: the screen down six of the GBA's sixteen
    brightness steps, and back"""
    if F(12) <= fr < F(14):
        return min(6, (fr - F(12)) // 1 + 1) / 16
    if F(14) <= fr < F(14) + 6:
        return (5 - (fr - F(14))) / 16
    return 0.0


def turned_at(book, fr):
    """the page arrow pressed in the frames after a page turns"""
    for p in book[1:] + [None]:
        start = p.shots[0].start if p else END
        if 0 <= fr - start < 6:
            return 'R'
    return None


def draw(book, fr):
    page, shot, row = where(book, fr)
    small = body()
    d = ImageDraw.Draw(small)
    d.fontmode = '1'
    end = fr >= END
    band(d, 'CYBERWORLD ENDLESS' if end else (shot.head if shot else ''), '0.16.0' if end else (shot.hint if shot else ''))
    lit = max([1 - (fr - h) / 4 for h in (shot.hits if shot else ()) if 0 <= fr - h < 4] or [0.0])
    window(d, GAME, lit)
    if end:
        list_window(d, 'NEW IN 0.16.0', NEWS, min(len(NEWS) - 1, (fr - END) // BEAT), fr, turned_at(book, fr))
    else:
        list_window(d, page.name, [s.row for s in page.shots], row if shot else None, fr, turned_at(book, fr))
    info_box(d)
    if shot:
        scene, i = shot.sample(fr)
        small.paste(kit.shot(scene, i), (GAME[0] + 5, GAME[1] + 5))
    if end:
        ending_small(d, fr)
    im = small.resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
    d = ImageDraw.Draw(im)
    rows_text(d, NEWS if end else [s.row for s in page.shots])
    if shot:
        info_text(d, shot.text)
    if end:
        ending_big(im, d, fr)
    return im


def compose(only=None):
    frames_dir = os.path.join(kit.WORK, 'frames')
    if only is None:
        shutil.rmtree(frames_dir, ignore_errors=True)
    os.makedirs(frames_dir, exist_ok=True)
    book = pages()
    black = Image.new('RGB', (W, H))
    for fr in range(FRAMES):
        if only is not None and fr not in only:
            continue
        im = draw(book, fr)
        # BN6's fades: up from black as the screen opens, down at the end,
        # the dark layer's two bars a few steps down, in sixteen steps
        dark = max(dim_at(fr), 1 - steps((fr + 1) / 16), steps((fr - (FRAMES - 17)) / 16))
        if dark > 0:
            im = Image.blend(im, black, dark)
        im.save(os.path.join(frames_dir, f'{fr:05d}.png'), compress_level=1)
        if fr % 100 == 0:
            print('frame', fr, '/', FRAMES, flush=True)


POSTER = F(3, 2)        # (BN6'S VIRUS BATTLER on the band, his first pair named)
