"""0.16.0's trailer, FIGHT ON?: BN6's Virus Battler in the run, and the
Crosses a run holds dealt into a dark layer's team. The release's news is
a hand: the viruses on a BtlrCard that lasts across runs, staked against a
Battler's pair, the Navis whose Crosses the run holds picked for a board.
So the cut is a card table: a green felt under a lamp, and every shot a
card dealt onto the pile, landing on its beat, the game's picture its art
at 3x, its headline the card's name, its caption the card's text, its
section the type line, its frame the colour of its suit (the Battler's
gold, the team's blue, the dark layer's violet, the feel's red). A card
lands square and whole, so the picture stays sharp; the cards before it
stay under it, askew. The drop turns the lamp down; the climax deals a
card a half bar, a foil's light sweeping each; the end gathers the pile
up and leaves the Battler's own question. The devices the earlier
trailers used (chrome type, typed briefing words, panels freed, linked
windows, a slanted seam, a war room's map and its pans) are not here.

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
import os
import shutil
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

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


# ---- the look ----

FPS, W, H = 30, 1280, 720
BAR = round(FPS * 60 * 4 / music.BPM)     # (48 frames at 150 beats a minute)
BEAT = BAR // 4
FRAMES = music.BARS * BAR + round(music.TAIL * FPS)

NAME_FACE = '/usr/share/fonts/opentype/urw-base35/URWBookman-Demi.otf'
TEXT_FACE = '/usr/share/fonts/opentype/urw-base35/C059-Italic.otf'
SMALL_FACE = '/usr/share/fonts/opentype/urw-base35/NimbusSansNarrow-Bold.otf'

FELT = (17, 78, 57)
FELT_DEEP = (6, 34, 25)
CREAM = (247, 243, 231)
INK = (30, 26, 20)
# the suits: (frame, plate, the type line's ink)
GOLD = ((214, 160, 40), (251, 238, 200), (128, 86, 10))         # the Virus Battler
BLUE = ((48, 112, 206), (222, 234, 252), (26, 70, 150))         # the team
VIOLET = ((120, 62, 186), (234, 222, 250), (84, 36, 140))       # the dark layer
RED = ((206, 52, 66), (252, 224, 224), (150, 26, 40))           # the feel

CW, CH = 768, 664                 # a card
ART = (24, 92)                    # its picture's place: 720 x 480, the game at 3x


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


def back(t, s=1.4):
    """an ease that overshoots a little and settles: a card slapped down"""
    t = max(0.0, min(1.0, t)) - 1
    return 1 + t * t * ((s + 1) * t + s)


def lerp(a, b, t):
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def fit(path, text, size, width, least):
    f = font(path, size)
    while size > least and f.getlength(text) > width:
        size -= 1
        f = font(path, size)
    return f


# ---- the table ----

_table = []


def table(dim):
    """the felt under a lamp: its weave a fine twill, the light pooled in
    the middle; `dim` (0-1) the dark layer's lamp turned down"""
    if not _table:
        light = Image.radial_gradient('L').resize((1700, 1150), Image.BICUBIC).crop((210, 215, 1490, 935))
        base = Image.composite(Image.new('RGB', (W, H), FELT_DEEP), Image.new('RGB', (W, H), FELT), light)
        tile = Image.new('L', (6, 6), 0)
        for i in range(6):
            tile.putpixel((i, i), 255)
            tile.putpixel(((i + 1) % 6, i), 110)
        weave = Image.new('L', (W, H))
        for x in range(0, W, 6):
            for y in range(0, H, 6):
                weave.paste(tile, (x, y))
        grain = Image.merge('RGB', [weave] * 3)
        base = ImageChops.add(base, ImageChops.multiply(grain, Image.new('RGB', (W, H), (7, 9, 8))))
        _table.append(base)
        _table.append(Image.blend(base, Image.new('RGB', (W, H), (8, 4, 18)), 0.62))
    if dim <= 0:
        return _table[0].copy()
    if dim >= 1:
        return _table[1].copy()
    return Image.blend(_table[0], _table[1], dim)


# ---- the cards ----

class Card:
    """A shot as a card dealt onto the pile: its suit, its name (the
    headline), its type line, its text (the caption) and its picture,
    `feed` [(frame, scene, sample, rate)] as 0.15.0's (from each frame on,
    that scene's footage from that sample; rate 0 holds it). It lands at
    `start`, on the beat, dealt from above over the `deal` frames before;
    `glint` frames a light sweeping its face, `buzz` frames the marks of a
    rumble at its sides."""

    def __init__(self, start, suit, name, kind, text, feed, deal=7, glint=(), buzz=()):
        self.start, self.suit, self.name, self.kind, self.text, self.feed = start, suit, name, kind, text, feed
        self.deal, self.glint, self.buzz = deal, glint, buzz
        self.number, self.index, self.tilt, self.nudge, self.end = '', 0, 0.0, (0, 0), 0
        self._face = None
        self._pile = None

    def sample(self, fr):
        i = max([j for j, e in enumerate(self.feed) if e[0] <= fr] or [0])
        start, scene, first, rate = self.feed[i]
        return scene, first + max(0, int((fr - start) * rate))

    def face(self):
        """the card without its picture: drawn once"""
        if self._face:
            return self._face
        frame, plate, kind_ink = self.suit
        im = Image.new('RGBA', (CW, CH), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.rounded_rectangle([0, 0, CW - 1, CH - 1], radius=26, fill=CREAM + (255,))
        d.rounded_rectangle([10, 10, CW - 11, CH - 11], radius=19, fill=frame + (255,))
        d.rounded_rectangle([20, 20, CW - 21, 84], radius=10, fill=plate + (255,))
        d.rectangle([ART[0] - 4, ART[1] - 4, ART[0] + 723, ART[1] + 483], fill=(14, 12, 10, 255))
        d.rounded_rectangle([20, 582, CW - 21, CH - 21], radius=10, fill=plate + (255,))
        kf = font(SMALL_FACE, 19)
        kw = kf.getlength(self.kind)
        d.text((CW - 36, 53), self.kind, font=kf, fill=kind_ink, anchor='rm')
        nf = fit(NAME_FACE, self.name, 38, CW - 96 - kw, 24)
        d.text((36, 54), self.name, font=nf, fill=INK, anchor='lm')
        nb = font(SMALL_FACE, 17)
        d.text((CW - 36, 612), self.number, font=nb, fill=kind_ink, anchor='rm')
        tf = fit(TEXT_FACE, self.text, 25, CW - 120 - nb.getlength(self.number), 17)
        d.text((36, 612), self.text, font=tf, fill=INK, anchor='lm')
        self._face = im
        return im

    def image(self, fr):
        """the card at frame fr: its face with the picture in, sharp"""
        im = self.face().copy()
        scene, i = self.sample(fr)
        im.paste(kit.shot(scene, i).resize((720, 480), Image.NEAREST), ART)
        for g in self.glint:
            if 0 <= fr - g < 12:
                sweep(im, (fr - g) / 11)
        return im

    def pile_image(self):
        """the card as it lies under the next ones: its last picture, askew"""
        if not self._pile:
            self._pile = self.image(self.end - 1).rotate(self.tilt, resample=Image.BICUBIC, expand=True)
        return self._pile


_band = []


def sweep(im, t):
    """a light sweeping across a card's face, as over a foil"""
    if not _band:
        b = Image.new('L', (CW * 3, CH), 0)
        ImageDraw.Draw(b).polygon([(CW, CH), (CW + 90, CH), (CW + 350, 0), (CW + 260, 0)], fill=120)
        _band.append(b.filter(ImageFilter.GaussianBlur(16)))
    x = round((1 - t) * 2 * CW)
    mask = ImageChops.multiply(_band[0].crop((x, 0, x + CW, CH)), im.getchannel('A'))
    im.paste(Image.new('RGBA', (CW, CH), (255, 250, 236, 255)), (0, 0), mask)


_shadow = []


def shadow(angle):
    if not _shadow:
        s = Image.new('RGBA', (CW + 80, CH + 80), (0, 0, 0, 0))
        ImageDraw.Draw(s).rounded_rectangle([40, 40, CW + 39, CH + 39], radius=26, fill=(0, 0, 0, 150))
        _shadow.append(s.filter(ImageFilter.GaussianBlur(14)))
    return _shadow[0].rotate(angle, resample=Image.BICUBIC, expand=True) if angle else _shadow[0]


def put(canvas, im, cx, cy):
    """an image centred on (cx, cy), whole pixels"""
    canvas.paste(im, (round(cx - im.width / 2), round(cy - im.height / 2)), im)


TILTS = [-2.6, 1.9, -1.3, 2.9, -3.4, 1.4, -2.0, 2.4, -1.7, 3.1, -2.8, 1.1, -3.0, 2.2, -1.5, 2.7, -2.3]
NUDGE = [(-16, 7), (12, -9), (-9, 12), (17, 4), (-13, -6), (8, 11), (-18, -3), (14, 8), (-7, -12), (10, 6),
         (-15, 9), (16, -5), (-11, -9), (9, 12), (-6, 7), (13, -8), (-12, 5)]


def deal_pose(card, fr):
    """(centre x, centre y, angle) of a card being dealt: from above, from
    the side opposite the last one's, slapped down square"""
    side = 1 if card.index % 2 == 0 else -1
    t = (fr - card.start + card.deal + 1) / card.deal
    p, a = back(t), ease(t)
    cx0, cy0 = W / 2 + side * 280, -CH / 2 - 60
    return cx0 + (W / 2 - cx0) * p, cy0 + (H / 2 - cy0) * p, side * -15 * (1 - a)


def draw_cards(canvas, cards, fr, gone=0.0):
    """the pile, the card before (nudged askew as the next lands) and the
    card being dealt or lying square on top; `gone` (0-1) the whole pile
    gathered off the table at the end"""
    live = [c for c in cards if c.start - c.deal <= fr]
    if not live:
        return
    top, lying = live[-1], live[:-1]
    landing = fr < top.start
    t = ease((fr - top.start + top.deal + 1) / top.deal)
    ox, oy = 0, -gone * (H + CH)
    for c in (lying[-4:-1] if landing else lying[-3:]):
        put(canvas, shadow(c.tilt), W / 2 + c.nudge[0] + 10 + ox, H / 2 + c.nudge[1] + 14 + oy)
        put(canvas, c.pile_image(), W / 2 + c.nudge[0] + ox, H / 2 + c.nudge[1] + oy)
    if landing and lying:
        c = lying[-1]
        a = c.tilt * t
        im = c.image(c.end - 1)
        im = im.rotate(a, resample=Image.BICUBIC, expand=True) if a else im
        cx, cy = W / 2 + c.nudge[0] * t, H / 2 + c.nudge[1] * t
        put(canvas, shadow(a), cx + 10, cy + 14)
        put(canvas, im, cx, cy)
    if landing:
        cx, cy, a = deal_pose(top, fr)
        lift = 1 - t
        put(canvas, shadow(a), cx + 10 + 30 * lift, cy + 14 + 40 * lift)
        put(canvas, top.image(fr).rotate(a, resample=Image.BICUBIC, expand=True), cx, cy)
        return
    cx, cy = W / 2 + ox, H / 2 + oy
    put(canvas, shadow(0), cx + 10, cy + 14)
    put(canvas, top.image(fr), cx, cy)
    for b in top.buzz:
        buzz(canvas, fr - b, cx, cy)


def buzz(canvas, k, cx, cy):
    """a rumble's marks: arcs at a card's sides, spreading and fading"""
    if not 0 <= k < 9:
        return
    over = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(over)
    for i, r in enumerate((18, 36, 54)):
        al = round(235 * (1 - k / 9) * (1 - 0.25 * i))
        for side in (-1, 1):
            x = cx + side * (CW / 2 + r + 3 * k)
            start, end = (305, 55) if side > 0 else (125, 235)
            d.arc([x - 36, cy - 130, x + 36, cy + 130], start, end, fill=(255, 238, 206, al), width=6)
    canvas.paste(over, (0, 0), over)


# ---- the run of cards ----

def cards():
    B, T, D, R = GOLD, BLUE, VIOLET, RED
    VB, LT = 'VIRUS BATTLER', 'LIBERATION · TEAM'
    cs = [
        # the intro: MegaMan walks up to a Battler at a side path's end; his offer
        Card(F(0, 2), B, 'A VIRUS BATTLER', VB, "A Mr.Prog in his own BN6 colour, at a side path's end",
             [(F(0, 2), 'bv-series', 12, 1)], deal=8),
        Card(F(2), B, "BN6'S VIRUS BATTLER", 'CYBERWORLD ENDLESS 0.16.0',
             'On the first layer of about half the acts, from the second', [(F(2), 'bv-series', 86, 1)]),
        # the main section: the stake, the list, the fight, the pot
        Card(F(4), B, 'STAKE YOUR ZENNY', VB, '500 to 3000 by act; each win adds it again to the pot',
             [(F(4), 'bv-series', 186, 1)]),
        Card(F(5), B, "YOUR BTLRCARD'S VIRUSES", VB, 'Every Rare you delete joins it, this run and every run after',
             [(F(5), 'bv-series', 440, 0), (F(5, 1.5), 'bv-series', 624, 1)]),
        Card(F(6), B, 'PLACE THEM, THEN WATCH', VB, "They fight his pair on their own; MegaMan's HP is untouched",
             [(F(6), 'bv-series', 656, 1)]),
        Card(F(7), B, 'THE FIGHT RUNS FAST', VB, 'It takes no input, so fast forward can run through it',
             [(F(7), 'bv-series', 768, 1)]),
        Card(F(9), B, 'THE POT GROWS', VB, 'A win adds the stake; a loss or a draw loses it all',
             [(F(9), 'bv-series', 896, 1)]),
        Card(F(10), B, 'TAKE THE POT, OR FIGHT ON?', VB, 'He shows his next pair before you choose',
             [(F(10), 'bv-series', 1006, 1), (F(10, 2), 'bv-series', 1062, 1)]),
        Card(F(11), B, 'CASH OUT', VB, 'Two wins on a 500 stake: 1500 zenny', [(F(11), 'bv-series', 2296, 1)],
             glint=(F(11, 1),)),
        # the drop: the dark layer's team
        Card(F(12), D, 'YOUR CROSSES JOIN THE TEAM', LT, "On a dark layer's board, each in his own BN6 body",
             [(F(12), 'heat', 16, 1)], deal=10),
        Card(F(13), D, 'PICK WHO COMES', LT, 'Where you hold more Navis than the board has places',
             [(F(13), 'team', 150, 1)]),
        # the climax: the commands, the Cross, the feel
        Card(F(14), T, 'HEATMAN · NAPALM', LT, 'Four ahead and both sides of the third, one battle',
             [(F(14), 'heat', 122, 1), (F(14) + 12, 'heat', 134, 0)], deal=5, glint=(F(14, 0.5),)),
        Card(F(14, 2), T, 'ELECMAN · MAGNETBARRIER', LT, 'No harm on the board to the team till the next phase',
             [(F(14, 2), 'elec', 45, 0), (F(14, 2) + 9, 'elec', 52, 1), (F(14, 2) + 15, 'elec', 58, 0)], deal=5, glint=(F(14, 2.5),)),
        Card(F(15), T, 'CHARGEMAN · BOMBLIBERATION', LT, 'He flies over the dark and frees the panel, no battle',
             [(F(15), 'charge', 84, 0), (F(15) + 8, 'charge', 90, 1), (F(15) + 14, 'charge', 96, 0)], deal=5, glint=(F(15, 0.5),)),
        Card(F(15, 2), T, 'SLASHMAN · WIDESWRD', LT, 'Three across, one battle',
             [(F(15, 2), 'slash', 148, 1), (F(15, 2) + 10, 'slash', 158, 0)], deal=5, glint=(F(15, 2.5),)),
        Card(F(16), T, 'HIS BATTLES, IN HIS CROSS', LT, "MegaMan in HeatCross from the first turn, on HeatMan's HP",
             [(F(16), 'heat', 327, 1)]),
        Card(F(17), R, 'FEEL EVERY HIT', 'RUMBLE · TRIGGERS', "Rumble on a fight's beats; a DualSense's triggers click",
             [(F(17), 'heat', 476, 1)], buzz=(F(17, 0.75), F(17, 1.25), F(17, 1.75), F(17, 2.5))),
    ]
    for i, c in enumerate(cs):
        c.index, c.number = i, f'{i + 1:02d} / {len(cs):02d}'
        c.tilt, c.nudge = TILTS[i % len(TILTS)], NUDGE[i % len(NUDGE)]
        c.end = cs[i + 1].start - cs[i + 1].deal if i + 1 < len(cs) else F(18)
    return cs


def dim_at(fr):
    """the lamp turned down for the dark layer's two bars"""
    if fr < F(12):
        return 0.0
    if fr < F(14):
        return ease((fr - F(12)) / 6)
    return max(0.0, 1 - (fr - F(14)) / 5)


# ---- the end: the cards gathered up, the logo on the felt ----

_logo = []
NEWS = ("BN6's Virus Battler  ·  your Crosses on the dark boards  ·  a Soul's chips in the finds",
        'rumble and DualSense triggers  ·  your own Net Square, with nothing to set')


def ending(im, fr):
    k = fr - F(18) - 8
    if k < 0:
        return
    if not _logo:
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        _logo.append(lg.resize((lg.width * 3 // 4, lg.height * 3 // 4), Image.LANCZOS))
    lg = _logo[0]
    a = ease(k / 10)
    part = lg.copy()
    part.putalpha(part.getchannel('A').point(lambda v: int(v * a)))
    im.paste(part, ((W - lg.width) // 2, 104 + round(12 * (1 - a))), part)
    d = ImageDraw.Draw(im)

    def line(at, text, face, size, color, y):
        t = ease((fr - at) / 8)
        if t > 0:
            d.text((W / 2, y), text, font=font(face, size), fill=lerp(FELT, color, t), anchor='mm')
    line(F(18, 2), 'FIGHT ON?', NAME_FACE, 60, CREAM, 376)
    line(F(18, 3), NEWS[0], TEXT_FACE, 25, (236, 214, 150), 438)
    line(F(18, 3.5), NEWS[1], TEXT_FACE, 25, (236, 214, 150), 470)
    line(F(19), 'saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless', SMALL_FACE, 24, (170, 230, 255), 530)
    line(F(19, 1), 'Unofficial fan project, not affiliated with Capcom. Free; runs from your own Mega Man Battle Network 6: '
         'Cybeast Gregar (USA) ROM.', SMALL_FACE, 17, (176, 200, 186), 640)
    line(F(19, 1), 'The dark boards need Mega Man Battle Network 5: Team Colonel (USA) beside it.', SMALL_FACE, 17,
         (176, 200, 186), 664)


def compose(only=None):
    frames_dir = os.path.join(kit.WORK, 'frames')
    if only is None:
        shutil.rmtree(frames_dir, ignore_errors=True)
    os.makedirs(frames_dir, exist_ok=True)
    cs = cards()
    for fr in range(FRAMES):
        if only is not None and fr not in only:
            continue
        im = table(dim_at(fr))
        gone = ease((fr - F(18) + 1) / 10) if fr >= F(18) else 0.0
        if gone < 1:
            draw_cards(im, cs, min(fr, F(18) - 1), gone)
        ending(im, fr)
        if fr < 8:
            im = Image.blend(Image.new('RGB', (W, H)), im, ease(fr / 7))
        if fr > FRAMES - 18:
            im = Image.blend(im, Image.new('RGB', (W, H)), min(1.0, (fr - (FRAMES - 18)) / 16))
        im.save(os.path.join(frames_dir, f'{fr:05d}.png'), compress_level=1)
        if fr % 100 == 0:
            print('frame', fr, '/', FRAMES, flush=True)


POSTER = F(3, 2)        # (the title card lying square, his first pair named)
