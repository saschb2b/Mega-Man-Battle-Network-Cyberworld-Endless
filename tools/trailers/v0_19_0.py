"""0.19.0's trailer, SEE IT AS YOU SET IT: Options reworked (CHANGELOG.md's
0.19.0): its pages in BN6's PET look with pill tabs, a battle beside
Display's lines that shows each change as it is made, a GBA's screen
(mGBA's colour correction and the VBA, DS and PSP colours, frame blending
to LCD ghosting, RetroArch's LCD 3x) and the sound set your way (Music
and Effects in levels, Output, mGBA's Audio Filter), in the skill's
structure (SKILL.md, The structure): a cold open of two bars with no
words, the title card, three segments (a section card of a bar, then a
shot a bar, each one concrete thing), the recap of the rest, the
stinger on the music's cadence and the end card held.

The idea: the trailer is an Options page, and the screen it sets. The
music is v0_19_0_music.py, an original piece in the manner of BN5's ACDC
Town (song 0x03), the owner's reference. The look is built from the
release's own screen in the project's PET style (tools/site_art.py's
colours, which are its own, and the site's Silkscreen):
- The field is navy, every pixel of it an LCD's three faint stripes and a
  dark line under each row: LCD 3x's own pattern, the screen the release
  lets a player set.
- The section label is Options' new tab row: a pill for each segment, the
  shown one RUN!'s green, L and R beside it, R lit as the page turns.
- The game's window holds the shot at 3x, and every cut is the screen's
  refresh: the new picture drawn in row by row from the top under a
  bright line, as an LCD draws its rows; a look turned inside a shot
  (the same battle in another screen's colours) is drawn in the same way,
  on the beat.
- The side window is the setting being turned: its name, its value
  between Options' orange arrows (lit as it turns, at the frame the
  picture changes), a pip for each of its values, the chosen one green.
- The caption is a line of Options' list: a navy plate with the orange
  cursor arrow.
- The stinger is the logo drawn in by the same refresh; the end card the
  logo and the version on the green pill, held still.
None of 0.17.0's devices (the battle's screens, the chip opening into the
field, the Program Advance) nor 0.16.0's (the list's cursor stepping as
the cut, the band, the INFORMATION box as the caption).

Its shots are whole frames of the game running from the developer's ROMs:
Options from the title (its pages turned, Display's lines and its battle,
Audio's lines), and one of BN6's virus battles taken in every look of the
GBA's screen, the same battle frame for frame; LCD 3x, which shows only
on the screen, from screen shots (footage() below).

    python3 build.py host && python3 tools/trailer.py 0.19.0 [--keep] [--frames A,B]
"""
import os
import shutil
import sys
import threading

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer as kit  # noqa: E402
from trailer import ROOT  # noqa: E402
from trailers import v0_19_0_music as music  # noqa: E402

VERSION = '0.19.0'
PLAY_LABEL = 'WATCH THE 0.19.0 TRAILER'
BN6_ONLY = ()


# ---- the footage ----

# One of BN6's virus battles on a layer, the same in every run: the
# arrival's words paged, the layer's next battle, a chip sent, then
# MegaMan stepping up and down his column and shooting a Gunner (game
# frames 1200 to 1420). The GBA's screen changes only how it is shown,
# so each look is the same battle, frame for frame.
BATTLE = ('150:,' + '4:A,6:,' * 60 + '20:,0:battle,220:,4:A,16:,4:A,16:,4:START,16:,4:A,110:,6:A,24:,'
          + '6:B,14:,6:UP,10:,6:B,14:,6:DOWN,10:,' * 6 + '200:')
NET = ['--scene', 'emu', '--dev', 'quiet', '--size', '720x480']
# Options from the title (tools/cw_routes.py's route): SELECT, its Display
# page; a press every 32 game frames, a beat of the music at 30 fps
TITLE = ['--scene', 'title', '--size', '720x480']
OPTIONS = '60:,6:SELECT,6:,40:,'
BEAT_PRESS = lambda k: f'6:{k},26:,'  # noqa: E731


def battle(*look):
    return (NET + list(look), {}, BATTLE, 1180, 1440)


def scenes():
    return {
        # the battle in each look of the GBA's screen
        'bt-off': battle(),
        'bt-gba': battle('--colour-correction', 'gba'),
        'bt-vba': battle('--colour-correction', 'vba'),
        'bt-nds': battle('--colour-correction', 'nds'),
        'bt-psp': battle('--colour-correction', 'psp'),
        'bt-lcd': battle('--interframe-blending', 'lcd_ghosting'),
        # the pages turned: Audio, Game, back to Display (from game frame 112)
        'opt-tabs': (TITLE, {}, OPTIONS + '50:,' + BEAT_PRESS('R') + BEAT_PRESS('R') + BEAT_PRESS('R') + '90:', 100, 400),
        # Display: down to Smooth motion (its preview), Colour correction
        # turned through GBA, VBA, DS, PSP and off, a beat each; Frame
        # blending through Simple, Smart, LCD and LCD fast, two beats each;
        # LCD 3x on
        'opt-display': (TITLE, {}, OPTIONS + '20:,6:DOWN,14:,6:DOWN,90:,6:DOWN,74:,' + BEAT_PRESS('RIGHT') * 5
                        + '6:DOWN,58:,' + '6:RIGHT,58:,' * 4 + '6:DOWN,58:,6:RIGHT,90:', 100, 1000),
        # Audio: Music 10 to 8, Effects 10 to 8, Output to Headphones and
        # Mono, Sound to Soft and mGBA, a beat each
        'opt-audio': (TITLE, {}, OPTIONS + '6:R,58:,' + BEAT_PRESS('LEFT') * 2 + BEAT_PRESS('DOWN') + BEAT_PRESS('LEFT') * 2
                      + BEAT_PRESS('DOWN') + BEAT_PRESS('RIGHT') * 2 + BEAT_PRESS('DOWN') + BEAT_PRESS('RIGHT') * 2 + '90:',
                      100, 800),
        # START on a changed Display page: back as it came?
        'opt-reset': (TITLE, {}, OPTIONS + '20:,6:DOWN,14:,6:DOWN,14:,6:DOWN,40:,6:RIGHT,60:,6:START,120:', 100, 420),
        # the Game page: Controls, Fast forward, Statistics
        'opt-game': (TITLE, {}, OPTIONS + '6:L,120:', 100, 300),
    }


# LCD 3x shows only on the screen (docs/engine/lcd.md), so its shots are
# the whole screen's, sixteen a run: at 3x (720 x 480) for its segment,
# with GBA's colours and LCD ghosting at 4x (960 x 640) for the cold open
SCREENS = {
    'lcd3x': (['--size', '720x480', '--lcd3x', 'on'], 1240),
    'cold-lcd': (['--size', '960x640', '--colour-correction', 'gba', '--interframe-blending', 'lcd_ghosting',
                  '--lcd3x', 'on'], 1240),
}
SCREEN_FRAMES = 64          # (every second game frame: two bars at 30 fps)


def screen_run(name, args, first, part):
    """sixteen of a scene's screen shots, game frames first + 32 * part on"""
    rel = os.path.relpath(kit.WORK, ROOT)
    data = os.path.join(kit.WORK, f'data-{name}-{part}')
    shutil.rmtree(data, ignore_errors=True)
    os.makedirs(data)
    frames = [first + 2 * (16 * part + i) for i in range(16)]
    shots = ','.join(f'{f}:/src/{rel}/{name}/s{f:05d}.bmp' for f in frames)
    code = kit.build.docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir',
                            f'/src/{rel}/data-{name}-{part}', *NET[:4], *args, '--input', BATTLE,
                            '--frames', str(frames[-1] + 1), '--screen-shot', shots,
                            mounts=[(kit.build.default_rom_dir(), '/rom:ro')])
    if code:
        sys.exit(f'capturing {name} failed')


def footage(keep):
    """the screen shots' scenes, as NNNN.png (their whole screen) beside
    the canvas scenes"""
    todo = [n for n in SCREENS if not (keep and os.path.isdir(os.path.join(kit.WORK, n))
                                       and len(os.listdir(os.path.join(kit.WORK, n))) >= SCREEN_FRAMES)]
    for name in todo:
        shutil.rmtree(os.path.join(kit.WORK, name), ignore_errors=True)
        os.makedirs(os.path.join(kit.WORK, name))
    gate = threading.Semaphore(8)

    def run(name, part):
        with gate:
            args, first = SCREENS[name]
            screen_run(name, args, first, part)
    threads = [threading.Thread(target=run, args=(n, p)) for n in todo for p in range(SCREEN_FRAMES // 16)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    for name in todo:
        out = os.path.join(kit.WORK, name)
        first = SCREENS[name][1]
        for i in range(SCREEN_FRAMES):
            p = os.path.join(out, f's{first + 2 * i:05d}.bmp')
            Image.open(p).convert('RGB').save(os.path.join(out, f'{i:04d}.png'))
            os.remove(p)
        print('captured', name)


# ---- the look ----

FPS, W, H = 30, 1280, 720
BAR = round(FPS * 60 * 4 / music.BPM)     # (64 frames at 112.5 beats a minute)
BEAT = BAR // 4
FRAMES = music.BARS * BAR + round(music.TAIL * FPS)

FONTS = os.path.join(ROOT, 'web', 'assets', 'fonts')
CAPS = os.path.join(FONTS, 'silkscreen-700.woff2')
CAPS_THIN = os.path.join(FONTS, 'silkscreen-400.woff2')
PIXEL = os.path.join(FONTS, 'pixelify-sans.woff2')

# tools/site_art.py's colours, which are the Options page's own
FRAME_HI, FRAME, FRAME_LO = (107, 222, 255), (66, 198, 231), (16, 140, 239)
FIELD = (16, 82, 107)
WHITE = (247, 255, 247)
ORANGE, ORANGE_LIT = (255, 173, 16), (255, 206, 24)
PILL_ON, PILL_EDGE = (16, 231, 41), (247, 255, 247)
PILL_OFF, PILL_OFF_LO = (66, 198, 231), (16, 140, 239)
DARK_GREEN = (0, 123, 49)
CYAN_TEXT = (107, 222, 255)
GOLD = (255, 214, 16)
SOFT = (198, 222, 239)
INK = (8, 22, 52)
PLATE, PLATE_EDGE = (16, 33, 82), (156, 206, 239)
# the field: navy, every pixel of it an LCD's three stripes, red, green
# and blue, and a dark line under each row (lcd3x's own pattern, faint)
NAVY = (8, 30, 66)
STRIPES = ((22, 30, 70), (8, 44, 70), (8, 32, 92))
ROW_LINE = (5, 20, 46)

S = 3                                      # the look drawn at 427 x 240, then 3x
LW, LH = 427, 240
PILLS = (36, 12, 386, 26)
GAME = (7, 30, 256, 199)
PIC = (12, 35)                             # the picture at 1x, 720 x 480 at video scale
SIDE = (262, 30, 419, 199)
CAPTION = (7, 204, 419, 228)
BIG = (160, 40)                            # the big window's picture at 4x, at video scale
CUT = 6                                    # the redraw, in frames


def F(bar, beat=0.0):
    return round(bar * BAR + beat * BEAT)


_fonts = {}


def font(size, bold=True):
    k = (size, bold)
    if k not in _fonts:
        _fonts[k] = ImageFont.truetype(CAPS if bold else CAPS_THIN, size)
    return _fonts[k]


def text(d, xy, words, size, fill, bold=True, anchor='la', shade=None, width=None):
    """Silkscreen's capitals, shrunk to `width` where given"""
    while width and size > 12 and font(size, bold).getlength(words) > width:
        size -= 1
    if shade:
        d.text((xy[0] + 3, xy[1] + 3), words, font=font(size, bold), fill=shade, anchor=anchor)
    d.text(xy, words, font=font(size, bold), fill=fill, anchor=anchor)


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


# ---- the PET's parts, at 427 x 240 ----

_field = None


def field():
    """the LCD's field: each pixel three faint stripes and a dark row line"""
    global _field
    if _field is None:
        im = Image.new('RGB', (LW, LH), NAVY)
        px = im.load()
        for y in range(LH):
            for x in range(LW):
                px[x, y] = ROW_LINE if y % 3 == 2 else STRIPES[x % 3]
        _field = im
    return _field.copy()


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
    """a line of Options' list: navy in a light edge"""
    x0, y0, x1, y1 = box
    d.rectangle([x0, y0, x1, y1], fill=PLATE_EDGE)
    d.rectangle([x0 + 1, y0 + 1, x1 - 1, y1 - 1], fill=PLATE)
    d.line([(x0 + 2, y1 - 2), (x1 - 2, y1 - 2)], fill=INK)


def pill(d, box, on, k=1):
    """Options' tab: a pill pointed at both ends, the shown one RUN!'s
    green in a white edge, the others cyan"""
    x0, y0, x1, y1 = box
    h = (y1 - y0) // 2
    fill, edge = (PILL_ON, PILL_EDGE) if on else (PILL_OFF, PILL_OFF_LO)
    pts = [(x0, y0 + h), (x0 + h, y0), (x1 - h, y0), (x1, y0 + h), (x1 - h, y1), (x0 + h, y1)]
    d.polygon(pts, fill=edge)
    inner = [(x0 + 2 * k, y0 + h), (x0 + h + k, y0 + k), (x1 - h - k, y0 + k), (x1 - 2 * k, y0 + h),
             (x1 - h - k, y1 - k), (x0 + h + k, y1 - k)]
    d.polygon(inner, fill=fill)


def arrow(d, x, y, size, fill, left=False):
    """the orange cursor arrow, pointing right (or left)"""
    if left:
        d.polygon([(x + size, y - size), (x + size, y + size), (x, y)], fill=fill)
    else:
        d.polygon([(x, y - size), (x, y + size), (x + size, y)], fill=fill)


# ---- the timeline ----

class Shot:
    """One bar's shot of a segment: its caption; `feed` [(frame, scene,
    sample[, rate])] (from each frame on, that scene's footage from that
    sample at 30 fps, or held with rate 0; a screen's whole picture is
    taken as it is); the side window's setting, its values and [(frame,
    value)] as it turns"""

    def __init__(self, start, caption, feed, setting='', values=(), turns=(), count=''):
        self.start, self.caption, self.feed = start, caption, feed
        self.setting, self.values, self.turns, self.count = setting, values, turns, count

    def sample(self, fr):
        e = max((e for e in self.feed if e[0] <= fr), default=self.feed[0])
        rate = e[3] if len(e) > 3 else 1
        return e[1], e[2] + max(0, fr - e[0]) * rate

    def value(self, fr):
        v = None
        for f, x in self.turns:
            if f <= fr:
                v = x
        return v


class Segment:
    def __init__(self, card, number, label, title, lines, shots):
        self.card, self.number, self.label, self.title, self.lines, self.shots = card, number, label, title, lines, shots


TITLE, SEG1, SEG2, SEG3, RECAP, STINGER, END = F(2), F(4), F(8), F(13), F(17), F(19), F(20)
# the same battle's footage in every look: the canvas scenes from game
# frame 1180, the screens' from 1240 (30 samples on)
LCD_AT = 30
COLOURS = ('OFF', 'GBA', 'VBA', 'DS', 'PSP')
BLENDS = ('OFF', 'SIMPLE', 'SMART', 'LCD', 'LCD FAST')
LEVELS = ('OFF',) + tuple(str(i) for i in range(1, 11))
OUTPUTS = ('SPEAKERS', 'HEADPHONES', 'MONO')
SOUNDS = ('GBA', 'SOFT', 'MGBA')


def at_beats(start, scenes, base):
    """a battle's feed: its scenes a beat each from `start`, the footage
    running on from `base` through them (the looks the same battle)"""
    return [(start + i * BEAT, sc, base + i * BEAT - (LCD_AT if sc.startswith('lcd') else 0))
            for i, sc in enumerate(scenes)]


def segments():
    s5, s6, s7 = F(5), F(6), F(7)
    s9, s10, s11, s12 = F(9), F(10), F(11), F(12)
    s14, s15, s16 = F(14), F(15), F(16)
    return [
        Segment(SEG1, '01', 'OPTIONS', 'OPTIONS IN PAGES', ('DISPLAY', 'AUDIO', 'GAME'), [
            # the pages turned by R: Audio, Game, Display, a beat each (each
            # page as it stands once BN6's fade has brought it in, the
            # turn drawn by the refresh)
            Shot(s5, 'THREE PAGES: L AND R TURN THEM', [(s5, 'opt-tabs', 14), (s5 + 16, 'opt-tabs', 40, 0),
                                                       (s5 + 32, 'opt-tabs', 56, 0), (s5 + 48, 'opt-tabs', 72)],
                 'PAGE', ('DISPLAY', 'AUDIO', 'GAME'),
                 [(s5, 'DISPLAY'), (s5 + 16, 'AUDIO'), (s5 + 32, 'GAME'), (s5 + 48, 'DISPLAY')]),
            # down to Colour correction: the battle beside the lines, GBA
            Shot(s6, 'A BATTLE BESIDE THE LINES, LIVE', [(s6, 'opt-display', 66)], 'COLOUR CORRECTION', COLOURS,
                 [(s6, 'OFF'), (s6 + 48, 'GBA')]),
            # VBA, DS, PSP and off: the page itself in each one's colours
            Shot(s7, "CYBERWORLD'S OWN SCREENS TAKE IT", [(s7, 'opt-display', 130)], 'COLOUR CORRECTION', COLOURS,
                 [(s7, 'VBA'), (s7 + 16, 'DS'), (s7 + 32, 'PSP'), (s7 + 48, 'OFF')]),
        ]),
        Segment(SEG2, '02', 'SCREEN', "A GBA'S SCREEN", ('COLOUR CORRECTION', 'FRAME BLENDING', 'LCD 3X'), [
            Shot(s9, "MGBA'S COLOUR CORRECTION", at_beats(s9, ('bt-off', 'bt-gba'), 2), 'COLOUR CORRECTION', COLOURS,
                 [(s9, 'OFF'), (s9 + BEAT, 'GBA')]),
            Shot(s10, 'MORE SCREENS: VBA, DS AND PSP', at_beats(s10, ('bt-vba', 'bt-nds', 'bt-psp'), 50),
                 'COLOUR CORRECTION', COLOURS, [(s10, 'VBA'), (s10 + BEAT, 'DS'), (s10 + 2 * BEAT, 'PSP')], count='3'),
            Shot(s11, 'FRAME BLENDING: LCD GHOSTING', at_beats(s11, ('bt-off', 'bt-lcd'), 40), 'FRAME BLENDING', BLENDS,
                 [(s11, 'OFF'), (s11 + BEAT, 'LCD')]),
            Shot(s12, 'LCD 3X: EACH PIXEL STRIPED', at_beats(s12, ('bt-off', 'lcd3x'), 30), 'LCD 3X', ('OFF', 'ON'),
                 [(s12, 'OFF'), (s12 + BEAT, 'ON')]),
        ]),
        Segment(SEG3, '03', 'SOUND', 'THE SOUND YOUR WAY', ('MUSIC', 'EFFECTS', 'OUTPUT', 'SOUND'), [
            Shot(s14, 'MUSIC AND EFFECTS: 10 TO 1, OFF', [(s14, 'opt-audio', 22)], 'MUSIC', LEVELS,
                 [(s14, '10'), (s14 + 16, '9'), (s14 + 32, '8')]),
            Shot(s15, 'OUTPUT: SPEAKERS, HEADPHONES, MONO', [(s15, 'opt-audio', 118)], 'OUTPUT', OUTPUTS,
                 [(s15, 'SPEAKERS'), (s15 + 16, 'HEADPHONES'), (s15 + 32, 'MONO')]),
            Shot(s16, "MGBA'S AUDIO FILTER, SAMPLE FOR SAMPLE", [(s16, 'opt-audio', 166)], 'SOUND', SOUNDS,
                 [(s16, 'GBA'), (s16 + 16, 'SOFT'), (s16 + 32, 'MGBA')]),
        ]),
        Segment(RECAP, '04', 'MORE', f'ALSO IN {VERSION}', (), []),
    ]


# the recap: (scene, sample), its line, at 2x
TILES = [(('opt-reset', 120), 'START: BACK AS IT CAME?'),
         (('opt-game', 60), 'GAME: CONTROLS, FAST FORWARD')]
RECAP_CAPTION = 'OPTIONS OPENS WHERE YOU LEFT IT'
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


# ---- the page: the segments and the recap ----

def picture(scene, i):
    """a scene's frame at video scale: the canvas's at 3x, a screen's as
    it was taken"""
    im = kit.shot(scene, i)
    return im if im.width > 240 else im.resize((720, 480), Image.NEAREST)


def redraw(im, new, old, k, box):
    """the cut: the new picture drawn over the old row by row from the top,
    a bright line where it is drawing (an LCD's refresh)"""
    x, y = box
    if old is not None:
        im.paste(old, (x, y))
    edge = round(new.height * min(1.0, (k + 1) / CUT))
    im.paste(new.crop((0, 0, new.width, edge)), (x, y))
    if k + 1 < CUT:
        d = ImageDraw.Draw(im)
        d.rectangle([x, y + edge - 3, x + new.width - 1, y + edge + 2], fill=WHITE)
        d.rectangle([x, y + edge + 3, x + new.width - 1, y + edge + 5], fill=FRAME_HI)


def pill_row(d, segs, seg, fr):
    """the tabs: one a segment, the shown one green; L and R beside them,
    the R lit as the page turns"""
    x0, y0, x1, y1 = PILLS
    n = len(segs)
    gap = 4
    w = (x1 - x0 - gap * (n - 1)) // n
    for i, sg in enumerate(segs):
        pill(d, (x0 + i * (w + gap), y0, x0 + i * (w + gap) + w, y1), sg is seg)
    lit = segs.index(seg) > 0 and fr - seg.card < 6
    for x, left in ((x0 - 13, True), (x1 + 4, False)):
        d.rectangle([x, y0 + 2, x + 9, y1 - 2], fill=DARK_GREEN)
        arrow(d, x + 2, (y0 + y1) // 2, 3, (WHITE if lit and not left else (16, 231, 41)), left=left)


def pill_words(d, segs, seg):
    x0, y0, x1, y1 = PILLS
    n = len(segs)
    gap = 4
    w = (x1 - x0 - gap * (n - 1)) // n
    for i, sg in enumerate(segs):
        cx = (x0 + i * (w + gap) + w / 2) * S
        on = sg is seg
        text(d, (cx, (y0 + y1) / 2 * S + 1), f'{sg.number} {sg.label}', 20, WHITE if on else INK, True, 'mm',
             shade=(8, 140, 24) if on else None, width=(w - 16) * S)


def side_words(d, shot, fr):
    """the side window: the setting's name, its value between the orange
    arrows (lit as it turns), a pip for each of its values"""
    x0, y0, x1, y1 = (c * S for c in SIDE)
    cx = (x0 + x1) / 2
    text(d, (cx, y0 + 80), shot.setting, 24, CYAN_TEXT, True, 'mm', width=x1 - x0 - 90)
    v = shot.value(fr)
    since = fr - max(f for f, x in shot.turns if f <= fr)
    lit = since < 6 and fr != shot.start
    vy = y0 + 230
    text(d, (cx, vy), v, 64, WHITE, True, 'mm', shade=INK, width=x1 - x0 - 190)
    for left in (True, False):
        ax = x0 + 44 if left else x1 - 64
        arrow(d, ax, vy, 14 + (3 if lit else 0), ORANGE_LIT if lit else ORANGE, left=left)
    n = len(shot.values)
    pw = min(30, (x1 - x0 - 120) // n)
    px0 = cx - n * pw / 2
    for i, val in enumerate(shot.values):
        on = val == v
        bx = px0 + i * pw
        d.rounded_rectangle([bx + 3, vy + 90, bx + pw - 3, vy + 110], radius=8,
                            fill=PILL_ON if on else PLATE, outline=PILL_EDGE if on else PLATE_EDGE, width=3)
    if shot.count:
        text(d, (cx, vy + 190), shot.count, 60, GOLD, True, 'mm', shade=INK)
        text(d, (cx, vy + 240), 'MORE SCREENS', 20, SOFT, True, 'mm')


def card_side_words(d, seg, fr):
    """the section card's side window: the lines its shots set, one a beat
    after the card's name (never two words at once)"""
    x0, y0, x1, y1 = (c * S for c in SIDE)
    for i, line in enumerate(seg.lines):
        if fr < seg.card + (i + 1) * BEAT * 3 // len(seg.lines):
            continue
        y = y0 + 110 + 62 * i
        d.rectangle([x0 + 30, y - 24, x1 - 30, y + 24], fill=PLATE, outline=PLATE_EDGE, width=3)
        text(d, (x0 + 52, y), line, 22, WHITE, True, 'lm', width=x1 - x0 - 110)


def draw_page(segs, fr):
    small = field()
    d = ImageDraw.Draw(small)
    seg, shot = where(segs, fr)
    recap = seg.card == RECAP
    pill_row(d, segs, seg, fr)
    if recap:
        window(d, (GAME[0], GAME[1], SIDE[2], SIDE[3]))
    else:
        window(d, GAME)
        window(d, SIDE)
    plate(d, CAPTION)
    arrow(d, CAPTION[0] + 6, (CAPTION[1] + CAPTION[3]) // 2, 5, ORANGE)
    im = small.resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
    pic_at = (PIC[0] * S, PIC[1] * S)
    if shot is not None:
        scene, i = shot.sample(fr)
        new = picture(scene, i)
        k = fr - shot.start
        changes = [e[0] for e in shot.feed]
        last_change = max(c for c in changes if c <= fr)
        if k < CUT:
            idx = seg.shots.index(shot)
            old = picture(*seg.shots[idx - 1].sample(shot.start - 1)) if idx > 0 else None
            redraw(im, new, old, k, pic_at)
        elif fr - last_change < CUT and last_change > shot.start:
            # (a look turned inside a shot: redrawn the same way)
            prev = shot.sample(last_change - 1)
            redraw(im, new, picture(*prev), fr - last_change, pic_at)
        else:
            im.paste(new, pic_at)
    d = ImageDraw.Draw(im)
    pill_words(d, segs, seg)
    cy = (CAPTION[1] + CAPTION[3]) * S / 2
    cw = (CAPTION[2] - CAPTION[0]) * S - 120
    if recap:
        draw_recap(im, d, fr)
        text(d, (CAPTION[0] * S + 60, cy), RECAP_CAPTION, 40, WHITE, True, 'lm', width=cw)
    elif shot is None:
        draw_card(im, d, seg, fr)
        card_side_words(d, seg, fr)
    else:
        side_words(d, shot, fr)
        text(d, (CAPTION[0] * S + 60, cy), shot.caption, 40, WHITE, True, 'lm', width=cw)
    return im


def draw_card(im, d, seg, fr):
    """the section card in the game window: its number and name, drawn in
    row by row as a shot is"""
    card = Image.new('RGB', (720, 480), FIELD)
    cd = ImageDraw.Draw(card)
    for y in range(0, 480, 9):
        cd.line([(0, y + 8), (720, y + 8)], fill=(12, 70, 96))
    text(cd, (360, 170), seg.number, 96, GOLD, True, 'mm', shade=INK)
    text(cd, (360, 290), seg.title, 52, WHITE, True, 'mm', shade=INK, width=640)
    redraw(im, card, None, fr - seg.card, (PIC[0] * S, PIC[1] * S))


def draw_recap(im, d, fr):
    """the tiles at 2x in one window, one a beat, a line under each"""
    w, h, gap = 480, 320, 60
    x0 = (GAME[0] + SIDE[2]) * S // 2 - (len(TILES) * w + (len(TILES) - 1) * gap) // 2
    y0 = 132
    for i, ((scene, sample), line) in enumerate(TILES):
        if fr < RECAP + i * BEAT:
            continue
        x = x0 + i * (w + gap)
        d.rectangle([x - 9, y0 - 9, x + w + 8, y0 + h + 8], fill=FRAME_LO)
        d.rectangle([x - 6, y0 - 6, x + w + 5, y0 + h + 5], fill=FRAME_HI)
        im.paste(kit.shot(scene, sample).resize((w, h), Image.NEAREST), (x, y0))
        text(d, (x + w / 2, y0 + h + 52), line, 28, WHITE, True, 'mm', shade=INK, width=w + 40)


# ---- the big window: the cold open, the title, the stinger, the end ----

_logo = {}


def logo(width):
    if width not in _logo:
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        _logo[width] = lg.resize((width, round(lg.height * width / lg.width)), Image.LANCZOS)
    return _logo[width]


def big_frame():
    im = field().resize((LW * S, LH * S), Image.NEAREST).crop((0, 0, W, H))
    d = ImageDraw.Draw(im)
    window(d, (BIG[0] - 15, BIG[1] - 15, BIG[0] + 960 + 14, BIG[1] + 640 + 14), 3)
    return im, d


def big_picture(scene, i):
    im = kit.shot(scene, i)
    return im if im.width == 960 else im.resize((960, 640), Image.NEAREST)


# the cold open: the same moment twice, plain at 4x, then as a GBA's
# screen (its colours, LCD ghosting, LCD 3x) drawn in over it
COLD = [(0, 'bt-off', LCD_AT), (F(1), 'cold-lcd', 0)]


def draw_cold(fr):
    im, d = big_frame()
    start, scene, first = max(e for e in COLD if e[0] <= fr)
    new = big_picture(scene, first + fr - start)
    if start > 0 and fr - start < CUT:
        redraw(im, new, big_picture('bt-off', LCD_AT + start - 1), fr - start, BIG)
    else:
        im.paste(new, BIG)
    return im


def draw_title(fr):
    """the logo, SEE IT AS YOU SET IT and the version on the green pill"""
    im, d = big_frame()
    k = fr - TITLE
    lg = logo(640)
    im.paste(lg, (W // 2 - 320, 110), lg)
    if k < CUT:
        # (the cold open's last frame wiped away by the redraw)
        cover = big_picture('cold-lcd', COLD[1][2] + TITLE - 1 - F(1))
        edge = round(640 * (k + 1) / CUT)
        im.paste(cover.crop((0, edge, 960, 640)), (BIG[0], BIG[1] + edge))
        d.rectangle([BIG[0], BIG[1] + edge - 3, BIG[0] + 959, BIG[1] + edge + 2], fill=WHITE)
    if k >= BEAT:
        a = min(1.0, (k - BEAT + 1) / 4)
        text(d, (W // 2, 110 + lg.height + 70), 'SEE IT AS YOU SET IT', 56, lerp(FIELD, WHITE, a), True, 'mm',
             shade=INK)
    if k >= 2 * BEAT:
        box = (W // 2 - 300, 456, W // 2 + 300, 530)
        pill(d, box, True, 3)
        text(d, (W // 2, 493), f'{VERSION} · OUT NOW', 40, WHITE, True, 'mm', shade=(8, 140, 24))
    return im


def draw_stinger(fr):
    """the cadence's downbeat: the logo and the version drawn in row by
    row, the way every shot came"""
    im, d = big_frame()
    lg = logo(700)
    im.paste(lg, (W // 2 - 350, 200), lg)
    text(d, (W // 2, 200 + lg.height + 76), VERSION, 64, GOLD, True, 'mm', shade=INK)
    k = fr - STINGER
    if k < 2 * CUT:
        edge = BIG[1] + round(640 * (k + 1) / (2 * CUT))
        blank, _ = big_frame()
        im.paste(blank.crop((0, edge, W, H)), (0, edge))
        d = ImageDraw.Draw(im)
        d.rectangle([BIG[0], edge - 3, BIG[0] + 959, edge + 2], fill=WHITE)
        d.rectangle([BIG[0], edge + 3, BIG[0] + 959, edge + 5], fill=FRAME_HI)
    return im


def draw_end(fr):
    """Options' page as the end card, held still: the logo, the version on
    the green pill, where to play it, the site and the ROM lines"""
    im, d = big_frame()
    k = fr - END
    lg = logo(560)
    y = 80
    im.paste(lg, (W // 2 - 280, y), lg)
    y += lg.height + 48
    box = (W // 2 - 260, y, W // 2 + 260, y + 64)
    pill(d, box, True, 3)
    text(d, (W // 2, y + 32), f'{VERSION} · OUT NOW', 34, WHITE, True, 'mm', shade=(8, 140, 24))
    if k < BEAT // 2:
        return im
    y += 64 + 64
    for i, line in enumerate(SYSTEMS):
        text(d, (W // 2, y + 38 * i), line, 22, WHITE, False, 'mm', width=900)
    address(d, W // 2, y + 112, SITE, CYAN_TEXT)
    foot = ("Needs your own Mega Man Battle Network 6: Cybeast Gregar (USA) ROM.",
            "BN5's areas need Mega Man Battle Network 5: Team Colonel (USA) too.",
            'An unofficial fan project, not affiliated with Capcom.')
    for i, line in enumerate(foot):
        text(d, (W // 2, y + 176 + 30 * i), line, 18, SOFT, False, 'mm', width=900)
    return im


# ---- the frame ----

def draw(segs, fr):
    if fr < TITLE:
        return draw_cold(fr)
    if fr < SEG1:
        return draw_title(fr)
    if fr < STINGER:
        return draw_page(segs, fr)
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


POSTER = F(3, 3)        # (the title card held: the logo, SEE IT AS YOU SET IT, 0.19.0 · OUT NOW)
