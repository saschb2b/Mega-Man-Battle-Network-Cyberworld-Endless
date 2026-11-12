"""0.21.0's trailer, THREE GAMES. ONE RUN.: what CHANGELOG.md's 0.21.0
brings from BN5 and BN4 into BN6's run and the two places that hold it
all. Every act has its virus (the act card names a family of any game,
met alone, beside a partner, then in a battle built around it); eight
more Navis to fight (BurnerMan, SparkMan, TopMan and JunkMan in BN4's
tournament, CloudMan and CosmoMan on the dark boards, GridMan and LarkMan
in the deep) and Red Sun's chips in the finds; the Library, a page per
game; the Battle Lab (the keeper in Lan's HP, foes of any game set side
by side, the eighteen panels painted, the stage and song chosen, the
fight, the Cybeast among its Navis). In the skill's structure (SKILL.md,
The structure): a cold open of two bars with no words, the title card,
four segments (a section card of a bar, then a shot a bar), the recap,
the stinger on the music's cadence and the end card held.

The idea: the mix. Each game is a channel, in the colours the ROMs
screen's cartridges already have (src/launcher/launcher_art.c: BN6's
green, BN5's blue, BN4's red), and those three are a screen's own green,
blue and red: added together they are the whole picture. All drawn here:
- The game's picture at 4x, nearly the whole frame; a column of three
  tags beside it, one a game (the Library's band has such tags), each lit
  in its colour while the shot takes something from that game, with one
  word saying what (ENGINE, AREA, VIRUS, NAVI, CHIP, PAGE, FOE, PANEL,
  STAGE, SONG); a tag's colour runs along the picture's edge.
- Every cut is the three channels meeting: the new picture's red, green
  and blue arrive apart, from three sides, and close into one over a few
  frames, on the beat. Words arrive the same way: three coloured ghosts
  that add up to white.
- The section label stands over the tags, a number and two short lines;
  the caption is one line on a navy plate along the picture's bottom
  edge, the PET's orange cursor arrow before it.
- Type: Ubuntu Bold, upright.
None of 0.20.0's devices (the bracket, its plates and its line running
in), 0.19.0's (the LCD stripes, pill tabs, the refresh), 0.17.0's (the
battle's screens, the chip window), 0.16.0's (the PET list and its
cursor), 0.15.0's map or 0.14.0's slanted seam.

Its shots are whole frames of the game running from the developer's ROMs:
seed 7's third act (Shrimpy's, BN4's, in BN6's CopyBot's Comp: its card
and its three layers' own first battles by the battle step), the Navis
and chips by the foes step and the folder switch in their own games'
areas (docs/devtools/battle-on-demand.md), the Library on a profile with
every chip (--dev library=1), the Battle Lab's keeper in Lan's HP
(--dev lab) and its builder with every row open (the lab all step: the
list a profile with every family complete has).

    python3 build.py host && python3 tools/trailer.py 0.21.0 [--keep] [--frames A,B]
"""
import os
import shutil
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer as kit  # noqa: E402
from trailer import ROOT  # noqa: E402
from trailers import v0_21_0_music as music  # noqa: E402

VERSION = '0.21.0'
PLAY_LABEL = 'WATCH THE 0.21.0 TRAILER'
BN6_ONLY = ()


# ---- the footage ----

AUTO = {'CYBERWORLD_AUTOPILOT': '1'}
PAGE = '300:,' + '4:A,56:,' * 10 + '300:,'          # a layer's arrival paged: 1210 frames
PET_LIB = '6:START,40:,6:DOWN,8:,6:DOWN,8:,6:A,90:,'  # the PET, its Library row


def fight(biome, foes, last=1500, seed=4, field=''):
    return (['--scene', 'emu', '--seed', str(seed), '--dev', 'quiet,god', '--net-biome', biome], AUTO,
            f'100:,0:foes {foes}{field}', 150, last)


def chip(biome, cid, foes='01@42/01@53/01@63'):
    """one chip in every hand (the folder switch), used on three Mettaurs"""
    return (['--scene', 'emu', '--seed', '4', '--dev', f'quiet,god,folder={cid}', '--net-biome', biome], AUTO,
            f'100:,0:foes {foes}', 150, 1300)


def arc(depth, seed=7, last=1900):
    """an act's layer's own first battle, the arc's beat, by the battle step"""
    return (['--scene', 'emu', '--run-depth', str(depth), '--seed', str(seed), '--dev', 'quiet,god'], AUTO, '300:battle', 150, last)


LAB = ['--scene', 'emu', '--net-biome', '0', '--seed', '15', '--dev', 'quiet,god']
LAB_FIRST = 1300
SET = '4:A,30:,{}4:A,60:,4:B,30:,'
# the builder's run, part by part (lab_at gives a part's frame): the list
# opened; a Mettaur (BN6's) set top, Draggin (BN5's, 45 rows on) middle
# right, VolGear (BN4's, 19 more) bottom; FIELD, three panels turned to
# BN5 Lava (12 presses), BN5 Magnet (14) and BN4 Hole (15); STAGE, the
# background turned back through BN4's and BN5's areas, the song through
# BN4's; BATTLE!, the Custom screen's OK and the fight
LAB_PARTS = (
    ('open', PAGE + '0:lab all,120:,'),
    ('mettaur', SET.format('4:UP,12:,')),
    ('to-draggin', '3:DOWN,3:,' * 45 + '30:,'),
    ('draggin', SET.format('4:DOWN,12:,4:RIGHT,12:,')),
    ('to-volgear', '3:DOWN,3:,' * 19 + '30:,'),
    ('volgear', SET.format('4:DOWN,12:,4:LEFT,12:,')),
    ('to-field', '60:,4:R,60:,4:DOWN,20:,'),
    ('lava', '4:A,10:,' * 12 + '40:,'),
    ('magnet', '4:RIGHT,14:,' + '4:A,10:,' * 14 + '40:,'),
    ('hole', '4:UP,14:,' + '4:A,10:,' * 15 + '40:,'),
    ('to-stage', '20:,4:R,60:,'),
    ('backgrounds', '4:LEFT,44:,' * 4),
    ('songs', '4:DOWN,20:,' + '4:LEFT,60:,' * 3),
    ('battle', '60:,20:START,300:,4:START,30:,4:A,260:,' + '4:B,12:,' * 60),
)
LAB_BUILD = ''.join(part for _, part in LAB_PARTS)


def script_frames(script):
    return sum(int(step.split(':')[0]) for step in script.split(',') if step)


def lab_at(name, frames=0, end=False):
    """the sample of lab-build's footage `frames` game frames into part
    `name` of its run (from its end where `end`)"""
    t = 0
    for part, script in LAB_PARTS:
        n = script_frames(script)
        if part == name:
            return (t + (n if end else 0) + frames - LAB_FIRST) // 2
        t += n
    raise KeyError(name)


HP_IN = ('40:,' + '4:A,6:,' * 46 + '30:,0:place -40 -8 7,10:,6:R,500:,' + '4:A,16:,' * 12
         + '60:,0:place -130 -15 1,60:,0:place -76 -30 7,50:,4:A,110:,4:A,90:,4:A,110:,4:A,160:,120:')
LIB = (PAGE + PET_LIB + '96:,' + '6:R,42:,' * 2 + '6:RIGHT,42:,' * 3 + '6:R,42:,' * 2 + '6:RIGHT,42:,' * 3
       + '6:R,42:,' * 3 + '6:RIGHT,42:,' * 2 + '96:')


def scenes():
    return {
        'card': (['--scene', 'emu', '--run-depth', '7', '--seed', '7'], {}, None, 40, 320),
        'arc1': arc(7), 'arc2': arc(8), 'arc3': arc(9),
        'navi-burner': fight('x15', '3B8@52'), 'navi-spark': fight('x16', '3C0@52'),
        'navi-top': fight('x17', '3B0@52'), 'navi-junk': fight('x18', '3A4@52'),
        'navi-cloud': fight('x3', '1E8@52'), 'navi-cosmo': fight('x5', '1EC@52'),
        'navi-grid': fight('x4', '3F0@52'), 'navi-lark': fight('x5', '3EC@52'),
        'vir-bn4': fight('x1', '2F0@42/2F6@53/2FC@63'),          # VolGear, Kilby, MagTect on a layer of BN5's
        'chip-meteor': chip('x15', 674), 'chip-flame': chip('x16', 595),   # Meteors1, FlmLine1 (BN4's)
        'chip-shade': chip('x5', 505),                                    # ShadeMan's (BN5's)
        'lab-navi': (LAB, {}, PAGE + '0:lab all,120:,' + '3:DOWN,3:,' * 85 + '60:,4:A,30:,4:A,60:,4:B,30:,60:,'
                     + '20:START,300:,4:START,30:,4:A,260:,' + '4:B,12:,' * 80, 1800, 3500),
        'library': (['--scene', 'emu', '--net-biome', '0', '--seed', '15', '--dev', 'quiet,library=1'], {}, LIB, 1200, 2300),
        'lab-hp': (['--scene', 'town', '--seed', '2', '--dev', 'lab'], {}, HP_IN, 1380, 2060),
        'lab-build': (LAB, {}, LAB_BUILD, LAB_FIRST, LAB_FIRST + script_frames(LAB_BUILD) - LAB_FIRST - 400),
    }


# ---- the look ----

FPS, W, H = 30, 1280, 720
BAR = round(FPS * 60 * 4 / music.BPM)
BEAT = BAR / 4
FRAMES = music.BARS * BAR + round(music.TAIL * FPS)

FACE = '/usr/share/fonts/truetype/ubuntu/Ubuntu[wdth,wght].ttf'     # (its variable axes: full width, weight 760)

# the three games, each in its cartridge's colours on the ROMs screen
# (src/launcher/launcher_art.c's label_top, label_foot, label_stripe)
GAMES = ('6', '5', '4')
NAME = {'6': 'BN6', '5': 'BN5', '4': 'BN4'}
TOP = {'6': (8, 189, 115), '5': (49, 156, 222), '4': (231, 74, 49)}
FOOT = {'6': (0, 140, 82), '5': (16, 99, 165), '4': (165, 24, 16)}
STRIPE = {'6': (74, 231, 115), '5': (107, 222, 255), '4': (255, 140, 107)}
NAVY, GRID, INK = (8, 18, 46), (18, 36, 78), (5, 10, 26)
WHITE, SOFT = (247, 255, 247), (176, 196, 228)
PLATE, PLATE_EDGE, OFF = (12, 26, 64), (44, 64, 112), (66, 88, 138)
ORANGE = (255, 148, 16)

PIC = (40, 40)                   # the game's picture at 4x: 960 x 640
PW, PH = 960, 640
COL = (1024, 1216)               # the column: the section label, the three tags
TAG_Y = {'6': 132, '5': 302, '4': 472}
TAG_H = 152
CAP = (PIC[0], 632, PIC[0] + PW, PIC[1] + PH)     # the caption's plate, on the picture's last eleven lines
CUT, QUICK = 5, 3                # the channels' closing, in frames: on a bar, inside one


def F(bar, beat=0.0):
    return round(bar * BAR + beat * BEAT)


_fonts = {}


def font(size):
    if size not in _fonts:
        f = ImageFont.truetype(FACE, size)
        f.set_variation_by_axes([100, 760])
        _fonts[size] = f
    return _fonts[size]


def fit(words, size, width):
    while width and size > 14 and font(size).getlength(words) > width:
        size -= 1
    return size


def text(d, xy, words, size, fill, anchor='la', width=None):
    d.text(xy, words, font=font(fit(words, size, width)), fill=fill, anchor=anchor)


def lerp(a, b, t):
    t = max(0.0, min(1.0, t))
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def ease(k, n):
    """how far apart the channels still are, 1 to 0 over n frames"""
    t = min(1.0, (k + 1) / n)
    return (1 - t) ** 2


_field = None


def field():
    """the net's navy, a mark at each crossing of its grid"""
    global _field
    if _field is None:
        im = Image.new('RGB', (W, H), NAVY)
        d = ImageDraw.Draw(im)
        for x in range(20, W, 40):
            for y in range(20, H, 40):
                d.line([(x - 3, y), (x + 3, y)], fill=GRID)
                d.line([(x, y - 3), (x, y + 3)], fill=GRID)
        _field = im
    return _field.copy()


# ---- the three channels ----

SIDES = {'6': (0, -1), '5': (-1, 0), '4': (1, 0)}     # where each game's ghost comes from: above, left, right


def converge(new, k, n, reach=84):
    """the cut: the picture's red, green and blue apart (BN4's, BN6's and
    BN5's), closing into one over n frames"""
    a = ease(k, n)
    if a <= 0:
        return new
    w, h = new.size
    out = []
    for band, g in zip(new.split(), ('4', '6', '5')):
        dx, dy = SIDES[g]
        ch = Image.new('L', (w, h), 0)
        ch.paste(band, (round(dx * reach * a), round(dy * reach * a * 0.66)))
        out.append(ch)
    return Image.merge('RGB', out)


def ghosts(im, draw_one, spread):
    """something drawn three times, a game's colour each, `spread` pixels
    apart, added to im: together the three are white"""
    for g in GAMES:
        dx, dy = SIDES[g]
        one = Image.new('RGB', im.size, (0, 0, 0))
        draw_one(one, round(dx * spread), round(dy * spread * 0.66), TOP[g])
        im = ImageChops.add(im, one)
    return im


def ghost_text(im, xy, words, size, spread, anchor='mm', width=None):
    size = fit(words, size, width)

    def one(layer, dx, dy, colour):
        ImageDraw.Draw(layer).text((xy[0] + dx, xy[1] + dy), words, font=font(size), fill=colour, anchor=anchor)
    return ghosts(im, one, spread)


_logo = {}


def logo(width):
    if width not in _logo:
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        _logo[width] = lg.resize((width, round(lg.height * width / lg.width)), Image.LANCZOS)
    return _logo[width]


def ghost_logo(im, xy, width, spread):
    """the logo's shape as the three ghosts, then the logo itself"""
    lg = logo(width)
    if spread <= 0:
        im.paste(lg, xy, lg)
        return im
    mask = lg.getchannel('A')

    def one(layer, dx, dy, colour):
        layer.paste(Image.new('RGB', lg.size, colour), (xy[0] + dx, xy[1] + dy), mask)
    return ghosts(im, one, spread)


def tri_bar(d, box):
    """a bar in the three colours, a third each"""
    x0, y0, x1, y1 = box
    w = (x1 - x0) / 3
    for i, g in enumerate(GAMES):
        d.rectangle([round(x0 + i * w), y0, round(x0 + (i + 1) * w) - 1, y1], fill=TOP[g])


# ---- the timeline ----

class Shot:
    """a shot: its caption and its feed [(frame, scene, sample, roles,
    cut)]: the scene played from that sample at 30 fps from that frame on,
    the three tags' words from there (roles: {game: word}), the channels
    closing over `cut` frames; a scene of None goes on with the footage
    and changes the words alone"""

    def __init__(self, start, caption, feed):
        self.start, self.caption, self.feed = start, caption, feed

    def entry(self, fr):
        return max((e for e in self.feed if e[0] <= fr), key=lambda e: e[0], default=self.feed[0])

    def sample(self, fr):
        hard = max((e for e in self.feed if e[0] <= fr and e[1]), key=lambda e: e[0], default=self.feed[0])
        return hard[1], hard[2] + max(0, fr - hard[0])

    def cut_at(self, fr):
        """(the frame of the last cut, its length)"""
        hard = max((e for e in self.feed if e[0] <= fr and e[1]), key=lambda e: e[0], default=self.feed[0])
        return hard[0], hard[4]


class Segment:
    def __init__(self, card, number, label, title, shots):
        self.card, self.number, self.label, self.title, self.shots = card, number, label, title, shots


def beats(start, parts):
    """a feed cutting on the beat: (scene, sample, beats held, roles)"""
    feed, b = [], 0
    for scene, sample, n, roles in parts:
        cut = (CUT if b == 0 else QUICK) if scene else 0
        feed.append((start + F(0, b), scene, sample, roles, cut))
        b += n
    return feed


TITLE, SEG1, SEG2, SEG3, SEG4, RECAP, STINGER, END = F(2), F(4), F(8), F(13), F(17), F(23), F(25), F(26)

ENGINE = {'6': 'ENGINE'}
N4, N5 = {'6': 'ENGINE', '4': 'NAVI'}, {'6': 'ENGINE', '5': 'NAVI'}
C4 = {'6': 'ENGINE', '4': 'CHIP'}

# the cold open: BurnerMan's flames, CloudMan's storm, the act's battle of
# both older nets' viruses, the Cybeast in the Battle Lab (no words: the
# tags light without theirs)
COLD = Shot(0, None, beats(0, (('navi-burner', 250, 2, N4), ('navi-cloud', 330, 2, N5),
                               ('arc3', 332, 2, {'6': 'AREA', '5': 'VIRUS', '4': 'VIRUS'}),
                               ('lab-navi', 432, 2, {'6': 'CYBEAST'}))))


def segments():
    lib = 'library'
    return [
        Segment(SEG1, '1', ('EVERY ACT,', 'ITS VIRUS'), 'EVERY ACT HAS ITS VIRUS', [
            Shot(F(5), "THE ACT'S CARD NAMES IT", beats(F(5), ((('card', 14, 4, {'6': 'AREA', '4': 'VIRUS'})),))),
            Shot(F(6), 'ALONE, THEN BESIDE A PARTNER', beats(F(6), (
                ('arc1', 262, 2, {'6': 'AREA', '4': 'VIRUS'}), ('arc2', 300, 2, {'6': 'AREA', '5': 'PARTNER', '4': 'VIRUS'})))),
            Shot(F(7), 'THEN A BATTLE BUILT AROUND IT', beats(F(7), (
                ('arc3', 278, 4, {'6': 'AREA', '5': 'VIRUS', '4': 'VIRUS'}),))),
        ]),
        Segment(SEG2, '2', ('NAVIS', 'AND CHIPS'), 'MORE NAVIS, MORE CHIPS', [
            Shot(F(9), "BN4'S TOURNAMENT: FOUR MORE NAVIS", beats(F(9), (
                ('navi-burner', 304, 1, N4), ('navi-spark', 346, 1, N4), ('navi-top', 416, 1, N4), ('navi-junk', 222, 1, N4)))),
            Shot(F(10), 'CLOUDMAN, COSMOMAN, GRIDMAN, LARKMAN', beats(F(10), (
                ('navi-cloud', 400, 1, N5), ('navi-cosmo', 384, 1, N5), ('navi-grid', 300, 1, N5), ('navi-lark', 400, 1, N5)))),
            Shot(F(11), "RED SUN'S CHIPS IN THE FINDS", beats(F(11), (
                ('chip-flame', 134, 1, C4), ('chip-flame', 230, 3, C4),
                ('chip-meteor', 142, 1, C4), ('chip-meteor', 248, 2, C4), ('chip-meteor', 480, 1, C4)))),
        ]),
        Segment(SEG3, '3', ('THE', 'LIBRARY'), 'THE LIBRARY', [
            Shot(F(14), 'PET > LIBRARY: ALL YOU HAVE FOUND', [
                (F(14), lib, 16, {'6': 'PET'}, CUT), (F(14) + 36, None, 0, {'6': 'PAGE'}, 0)]),
            Shot(F(15), 'A PAGE FOR EACH GAME', beats(F(15), ((lib, 244, 4, {'5': 'PAGE'}),))),
            Shot(F(16), 'COUNTED ACROSS EVERY RUN', beats(F(16), ((lib, 364, 4, {'4': 'PAGE'}),))),
        ]),
        Segment(SEG4, '4', ('THE', 'BATTLE LAB'), 'THE BATTLE LAB', [
            Shot(F(18), "A NAVI OF DAD'S LAB, IN LAN'S HP", beats(F(18), (('lab-hp', 156, 4, {'6': "LAN'S HP"}),))),
            Shot(F(19), 'SET FOES OF ANY GAME, SIDE BY SIDE', beats(F(19), (
                ('lab-build', lab_at('mettaur', 44), 1, {'6': 'FOE'}),
                ('lab-build', lab_at('draggin', 68), 1, {'6': 'FOE', '5': 'FOE'}),
                ('lab-build', lab_at('volgear', 60), 2, {'6': 'FOE', '5': 'FOE', '4': 'FOE'})))),
            Shot(F(20), 'TURN ALL EIGHTEEN PANELS', beats(F(20), (
                ('lab-build', lab_at('lava', -70, end=True), 1.5, {'5': 'PANEL'}),
                ('lab-build', lab_at('magnet', -70, end=True), 1.5, {'5': 'PANEL'}),
                ('lab-build', lab_at('hole', -60, end=True), 1, {'5': 'PANEL', '4': 'PANEL'})))),
            Shot(F(21), 'PICK THE STAGE AND THE SONG', beats(F(21), (
                ('lab-build', lab_at('backgrounds', 4), 2, {'4': 'STAGE'}),
                ('lab-build', lab_at('songs', 28), 2, {'4': 'SONG'})))),
            Shot(F(22), 'NO RISK, NO REWARD: A TEST BATTLE', beats(F(22), (
                ('lab-build', lab_at('battle', 500), 2, {'6': 'FOE', '5': 'FOE', '4': 'FOE'}),
                ('lab-navi', 400, 2, {'6': 'CYBEAST'})))),
        ]),
    ]


# the recap: a tile each at 2x, its line, the games it takes from
TILES = [('vir-bn4', 250, 'VOLGEAR, KILBY AND MAGTECT VISIT', {'6': 'ENGINE', '5': 'AREA', '4': 'VIRUS'}),
         ('chip-shade', 296, '21 NAVI CHIPS FROM BN5', {'6': 'ENGINE', '5': 'CHIP'})]
RECAP_CAPTION = 'AND MORE IN THE CHANGELOG'
HEADLINE = 'THREE GAMES. ONE RUN.'
SYSTEMS = ('PortMaster handhelds · Linux · Windows · macOS · Android',
           'iPhone and iPad · New 3DS · Switch · PS Vita · PS4 · PS5 · the browser')
SITE = 'saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless'
FOOT_LINES = ('Needs your own Mega Man Battle Network 6: Cybeast Gregar (USA) ROM.',
              "BN5's and BN4's own need Team Colonel (USA) and Red Sun (USA) beside it.",
              'An unofficial fan project, not affiliated with Capcom.')


def where(segs, fr):
    seg = max((s for s in segs if s.card <= fr), key=lambda s: s.card)
    shot = max((sh for sh in seg.shots if sh.start <= fr), key=lambda sh: sh.start, default=None)
    return seg, shot


# ---- the page: the picture, the column, the caption ----

def picture(scene, i):
    return kit.shot(scene, i).resize((PW, PH), Image.NEAREST)


def card_picture(seg, fr):
    """the section card in the picture's place: its number, then on the
    second beat its name, three ghosts closing into white"""
    k = fr - seg.card
    card = Image.new('RGB', (PW, PH), INK)
    d = ImageDraw.Draw(card)
    for y in range(0, PH, 32):
        d.line([(0, y), (PW, y)], fill=(10, 20, 48))
    tri_bar(d, (PW // 2 - 150, 196, PW // 2 + 150, 207))
    card = ghost_text(card, (PW // 2, 130), seg.number, 120, 4 + 60 * ease(k, 8))
    if k >= BEAT:
        card = ghost_text(card, (PW // 2, 300), seg.title, 76, 70 * ease(k - BEAT, 7), width=PW - 120)
    return card


def settled(segs, fr):
    """the picture as it stands at frame fr, no cut drawn (what a cut
    closes over)"""
    if fr < TITLE:
        return picture(*COLD.sample(fr))
    if fr < SEG1:
        return Image.new('RGB', (PW, PH), INK)
    seg, shot = where(segs, fr)
    if shot is None:
        return card_picture(seg, fr)
    return picture(*shot.sample(fr))


def tag(im, d, g, role, flash=0.0, words=True):
    """a game's tag: lit in its colours with the shot's word for it, or off"""
    x0, x1 = COL
    y0 = TAG_Y[g]
    y1 = y0 + TAG_H
    if role is None:
        d.rectangle([x0, y0, x1, y1], fill=PLATE_EDGE)
        d.rectangle([x0 + 3, y0 + 3, x1 - 3, y1 - 3], fill=PLATE)
        if words:
            text(d, (x0 + 18, y0 + 50), NAME[g], 64, OFF, 'lm')
        return
    top, foot, stripe = (lerp(c, WHITE, flash) for c in (TOP[g], FOOT[g], STRIPE[g]))
    d.rectangle([PIC[0] + PW, y0 + 52, x0, y0 + 100], fill=top)          # (its colour running into the picture's edge)
    d.rectangle([PIC[0] + PW, y0, PIC[0] + PW + 5, y1], fill=stripe)
    d.rectangle([x0, y0, x1, y1], fill=stripe)
    d.rectangle([x0 + 3, y0 + 3, x1 - 3, y1 - 3], fill=top)
    d.rectangle([x0 + 3, y0 + 102, x1 - 3, y1 - 3], fill=foot)
    d.rectangle([x0 + 3, y0 + 98, x1 - 3, y0 + 102], fill=stripe)
    if words:
        text(d, (x0 + 18, y0 + 50), NAME[g], 64, WHITE, 'lm')
        text(d, (x0 + 18, y0 + 126), role, 26, WHITE, 'lm', width=x1 - x0 - 34)


def column(im, d, seg, roles, flashes, words=True):
    x0, x1 = COL
    if seg is not None:
        number, lines = seg
        d.rectangle([x0, 44, x0 + 44, 104], fill=WHITE)
        text(d, (x0 + 22, 74), number, 44, INK, 'mm')
        for i, line in enumerate(lines):
            text(d, (x0 + 56, 60 + 30 * i), line, 22, WHITE, 'lm', width=x1 - x0 - 56)
    for g in GAMES:
        tag(im, d, g, roles.get(g), flashes.get(g, 0.0), words)
    if words:
        text(d, (x1, 662), VERSION, 24, SOFT, 'rm')


def caption(im, words):
    """one line on the picture's last lines, the PET's orange arrow before it"""
    x0, y0, x1, y1 = CAP
    plate = Image.blend(im.crop((x0, y0, x1, y1)), Image.new('RGB', (x1 - x0, y1 - y0), INK), 0.88)
    im.paste(plate, (x0, y0))
    d = ImageDraw.Draw(im)
    d.line([(x0, y0), (x1 - 1, y0)], fill=WHITE, width=2)
    cy = (y0 + y1) // 2 + 1
    d.polygon([(x0 + 20, cy - 12), (x0 + 20, cy + 12), (x0 + 38, cy)], fill=ORANGE)
    text(d, (x0 + 54, cy), words, 30, WHITE, 'lm', width=x1 - x0 - 80)


def frame(d):
    x, y = PIC
    d.rectangle([x - 4, y - 4, x + PW + 3, y + PH + 3], fill=WHITE)


def lit_since(shot, fr, start):
    """each game's flash: 1 at the frame its word last changed, gone five on"""
    out = {}
    prev = {}
    for e in sorted(shot.feed):
        if e[0] > fr:
            break
        for g in GAMES:
            if e[3].get(g) != prev.get(g) and e[3].get(g):
                out[g] = e[0]
        prev = e[3]
    return {g: max(0.0, 1 - (fr - t) / 5) * 0.7 for g, t in out.items()}


def draw_shot(segs, shot, fr, label, words=True):
    im = field()
    d = ImageDraw.Draw(im)
    frame(d)
    last, n = shot.cut_at(fr)
    new = picture(*shot.sample(fr))
    k = fr - last
    if k < n and last > 0:
        old = settled(segs, last - 1)
        new = Image.blend(old, converge(new, k, n), min(1.0, (k + 1) / n * 1.7))
    elif k < n:
        new = converge(new, k, n)
    im.paste(new, PIC)
    roles = shot.entry(fr)[3]
    column(im, d, label, roles, lit_since(shot, fr, shot.start), words)
    if shot.caption:
        caption(im, shot.caption)
    return im


def draw_card(segs, seg, fr):
    im = field()
    d = ImageDraw.Draw(im)
    frame(d)
    k = fr - seg.card
    new = card_picture(seg, fr)
    if k < CUT:
        new = Image.blend(settled(segs, seg.card - 1), new, min(1.0, (k + 1) / CUT * 1.7))
    im.paste(new, PIC)
    column(im, d, (seg.number, seg.label), {}, {})
    return im


def draw_recap(fr):
    """two tiles at 2x where the picture stands, one a bar, a line under each"""
    im = field()
    d = ImageDraw.Draw(im)
    frame(d)
    d.rectangle([PIC[0], PIC[1], PIC[0] + PW - 1, PIC[1] + PH - 1], fill=INK)
    for y in range(PIC[1], PIC[1] + PH, 32):
        d.line([(PIC[0], y), (PIC[0] + PW - 1, y)], fill=(10, 20, 48))
    roles = {}
    for i, (scene, sample, line, r) in enumerate(TILES):
        start = RECAP + i * BAR
        if fr < start:
            continue
        roles = r
        k = fr - start
        x, y = PIC[0] + 480 * i, PIC[1] + 110
        tile = kit.shot(scene, sample + k).resize((480, 320), Image.NEAREST)
        if k < CUT:
            tile = converge(tile, k, CUT, 48)
        im.paste(tile, (x, y))
        d = ImageDraw.Draw(im)
        text(d, (x + 240, y + 320 + 48), line, 28, WHITE, 'mm', width=450)
    last = max(RECAP + i * BAR for i in range(len(TILES)) if RECAP + i * BAR <= fr)
    column(im, d, ('+', ('ALSO IN', VERSION)), roles, {g: max(0.0, 1 - (fr - last) / 5) * 0.7 for g in roles})
    caption(im, RECAP_CAPTION)
    return im


# ---- the whole frame: the title, the stinger, the end ----

def draw_title(fr):
    """the logo closing out of its three ghosts, the headline on the
    second beat, the version on the third"""
    k = fr - TITLE
    im = field()
    lg = logo(680)
    im = ghost_logo(im, (W // 2 - 340, 120), 680, round(150 * ease(k, 9)))
    y = 120 + lg.height
    if k >= BEAT:
        im = ghost_text(im, (W // 2, y + 96), HEADLINE, 86, 90 * ease(k - BEAT, 7), width=W - 200)
    if k >= 2 * BEAT:
        d = ImageDraw.Draw(im)
        tri_bar(d, (W // 2 - 250, y + 170, W // 2 + 250, y + 181))
        text(d, (W // 2, y + 222), f'{VERSION} · OUT NOW', 46, WHITE, 'mm')
    return im


def draw_stinger(fr):
    """the three colours running in from their sides and meeting: the
    logo, the version under it"""
    k = fr - STINGER
    im = field()
    d = ImageDraw.Draw(im)
    run = 6
    cy = 300
    if k < run:
        t = (k + 1) / run
        d.rectangle([0, cy - 10, round((W // 2) * t), cy + 10], fill=TOP['5'])
        d.rectangle([W - round((W // 2) * t), cy - 10, W, cy + 10], fill=TOP['4'])
        d.rectangle([W // 2 - 10, 0, W // 2 + 10, round(cy * t)], fill=TOP['6'])
        return im
    lg = logo(760)
    im = ghost_logo(im, (W // 2 - 380, cy - lg.height // 2 - 30), 760, round(120 * ease(k - run, 7)))
    if k >= run + 7:
        d = ImageDraw.Draw(im)
        tri_bar(d, (W // 2 - 150, cy + lg.height // 2 + 6, W // 2 + 150, cy + lg.height // 2 + 17))
        text(d, (W // 2, cy + lg.height // 2 + 66), VERSION, 76, WHITE, 'mm')
    return im


def draw_end(fr):
    """the end card, held still: the logo, the version, where to play it,
    the site and the ROM lines"""
    im = field()
    d = ImageDraw.Draw(im)
    lg = logo(620)
    y = 84
    im.paste(lg, (W // 2 - 310, y), lg)
    y += lg.height + 34
    tri_bar(d, (W // 2 - 270, y, W // 2 + 270, y + 11))
    text(d, (W // 2, y + 54), f'{VERSION} · OUT NOW', 50, WHITE, 'mm')
    if fr - END < BEAT // 2:
        return im
    y += 138
    for i, line in enumerate(SYSTEMS):
        text(d, (W // 2, y + 42 * i), line, 30, WHITE, 'mm', width=1100)
    text(d, (W // 2, y + 118), SITE, 32, STRIPE['5'], 'mm', width=1100)
    for i, line in enumerate(FOOT_LINES):
        text(d, (W // 2, y + 186 + 32 * i), line, 24, SOFT, 'mm', width=1100)
    return im


# ---- the frame ----

def draw(segs, fr):
    if fr < TITLE:
        return draw_shot(segs, COLD, fr, None, words=False)
    if fr < SEG1:
        return draw_title(fr)
    if fr < RECAP:
        seg, shot = where(segs, fr)
        if shot is None:
            return draw_card(segs, seg, fr)
        return draw_shot(segs, shot, fr, (seg.number, seg.label))
    if fr < STINGER:
        return draw_recap(fr)
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


POSTER = F(3, 3)        # (the title card held: the logo, THREE GAMES. ONE RUN., 0.21.0 · OUT NOW)
