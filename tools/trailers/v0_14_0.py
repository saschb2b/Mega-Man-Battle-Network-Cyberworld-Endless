"""0.14.0's trailer, PICK A SIDE: the Net Square's people answering each
other and fighting each other, a red side and a blue side. The release's
news is a pair at every step (an emote and its answer, a wave and a wave
back, a challenge and its Yes, a rematch's two Yeses, a team of two, two
MegaMen a side), so the cut is a versus screen: the frame cut by a slanted
seam, the red side's game on the left, the blue side's on the right, each
side's stripes running toward the seam; a call slides in on the red side
and its answer on the blue side on the next beat; the seam itself carries
the cuts, sweeping across the frame. Where both sides meet (the "!!" of a
wave answered, the Square's cheer, a team's two) the seam flashes gold.

Its shots are whole frames of the game running from the developer's ROMs,
BN6's and BN5 Team Colonel's beside it, at a whole scale (4x, 3x, 2x):
games in a Square of their own on a local server, several at once
(tools/trailer_online.py): the emotes by the clock, the NetBattles, the
relay and the two a side by remote play, each course waiting for what the
games' logs say. Everything else is drawn here; its music is
v0_14_0_music.py's, an original piece in the manner of BN6's battle
themes.

    python3 build.py linux && python3 build.py server && python3 tools/trailer.py 0.14.0 [--keep]
"""
import json
import math
import os
import shutil
import sys
import time

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import trailer as kit  # noqa: E402
import trailer_online as online  # noqa: E402
from trailer import ROOT  # noqa: E402
from trailers import v0_14_0_music as music  # noqa: E402
from trailers.v0_13_0 import Visit  # noqa: E402  (a game's script by the wall clock)

VERSION = '0.14.0'
PLAY_LABEL = 'WATCH THE 0.14.0 TRAILER'
BN6_ONLY = ()


# ---- the footage: games in a Square of their own, at once ----

STAMPED = {'netpass.ini': 'stamps=4\n'}


def floor(gx, gy, facing):
    """`place`'s world x, y and facing for the Square plan's panel (gx, gy)
    (src/online/square_plan.c: a 36-panel grid, the plaza 11 to 23 each
    way, the landmark's base 16 to 18, its props in holes at 15 and 19 on
    the diagonals; netmap_world: x falls 32 a panel of gy, y rises 32 a
    panel of gx). Facing 1 is +x, 3 +y, 5 -x, 7 -y. Only from 15 to 19
    each way does the plaza's floor fill the whole picture round a
    MegaMan (the screen shows about four panels of the grid either side)."""
    return round(596 - 32 * gy), round(32 * gx - 500), facing


def at(gx, gy, facing):
    return 'place %d %d %d' % floor(gx, gy, facing)


# (in front of the landmark, round its base: its +x face, then its +y face;
# Kei and Mira first, face to face a quarter of a panel apart)
KEI_SPOT, MIRA_SPOT = (19, 17.25, 5), (19, 17.75, 1)
RING = [(19, 17.25, 7), (19, 17.75, 7), (19, 16.25, 7), (17.25, 19, 1), (16.25, 19, 1), (18.25, 19, 1)]
CROWD = ['kei', 'mira', 'dex', 'rio', 'nova', 'juno']
CHORD_UP = '6:L,8:L+UP,10:L,'      # (L held and up: the first favourite, Wave)


def square_game(i):
    """The Square by the clock: Kei and Mira face to face by the pad; Kei's
    strip (SELECT, the cursor over Cheer, then Buster, sent), his Wave at
    Mira and her Wave back; Mira's emotes as Kei sees them; then all six
    round the landmark, cheering within a second, and bowing together."""
    name = CROWD[i]
    v = Visit(0.5 * i)
    if name == 'kei':
        v.at(3, '0:' + at(*KEI_SPOT) + ',')
        v.at(5, '6:SELECT,')
        v.at(6.2, '6:RIGHT,')                 # (Cheer: the window plays it, AT MIRA)
        v.at(7.4, '6:DOWN,')                  # (Buster, TO EVERYONE)
        v.at(8.6, '6:A,')
        v.at(11.5, CHORD_UP)                  # (Wave, aimed at Mira, faced)
    elif name == 'mira':
        v.at(3, '0:' + at(*MIRA_SPOT) + ',')
        v.at(13.7, CHORD_UP)                  # (her Wave back: the "!!" between them)
        for k, (w, emote) in enumerate(((17.5, 5), (20, 2), (22.5, 10), (25, 6))):
            v.at(w, f'0:emote {emote},')      # (Bow, Laugh, Spin, Got it!)
    v.at(28, '0:' + at(*RING[i]) + ',')
    v.at(30 + 0.12 * i, '0:emote 1,')         # (all six cheer within a second: the Square answers)
    v.at(34 + 0.12 * i, '0:emote 5,')         # (... and bow in step)
    v.at(39)
    first = v.frame(2.5) if name in ('kei', 'mira') else v.frame(27) if name in ('dex', 'nova') else None
    return dict(name=name, operator=name.capitalize(), delay=0.5 * i, script=v.s, files=STAMPED,
                first=first, last=v.t, frames=v.t + 1)


# (remote play's steps: A on the faced MegaMan reads his card, three pages,
# then the box of Battle, Team up and More...)
CARD = '4 A;130;4 A;130;4 A;130;150'
FIGHT = ['arenaboth', 'arenahp=40']
MARKS = {}


def mark(scene, game, what):
    """the sample (30 fps) a game's footage is at as `what` happens"""
    MARKS.setdefault(scene, {}).setdefault(game.name, {})[what] = game.sample()
    print(f'  {scene}: {game.name} {what} at {game.sample()}', flush=True)


def save_marks(work, scene):
    with open(os.path.join(work, f'{scene}.marks.json'), 'w') as f:
        json.dump(MARKS.get(scene, {}), f, indent=1, sort_keys=True)


def into_square(games, t0):
    for g in games:
        g.do(online.REMOTE_GATE)
        online.wait(lambda: g.has('online: square seed='), f'{g.name} in the Square', 120, t0)


def ask_scene(work, scene, port, roms):
    """Kei's Battle on Mira, whose game asks her: the PET ringing, the "!"
    over Kei, his MegaMan's card read, "Take it,Lan?"; then B (no battle:
    this scene is the question)."""
    t0 = time.time()
    kei = online.Remote(work, scene, 'kei', port, roms, FIGHT)
    mira = online.Remote(work, scene, 'mira', port, roms, ['arenahp=40'])
    into_square((kei, mira), t0)
    time.sleep(1)
    mira.do(at(*MIRA_SPOT) + ';10')
    kei.do(at(*KEI_SPOT) + ';10')
    time.sleep(2)
    mark(scene, kei, 'card')
    kei.do(CARD + ';4 A;120;4 A;60')
    online.wait(lambda: mira.has('challenged us'), 'Mira challenged', 60, t0)
    mark(scene, mira, 'challenged')
    mira.do('40;4 A;70;4 A;70;4 A;70;4 A;70;4 A;70;200')
    online.wait(lambda: mira.idle(), 'Mira at the question', 60, t0)
    mark(scene, mira, 'question')
    time.sleep(2)
    mira.do('4 B;120')
    time.sleep(4)
    for g in (kei, mira):
        g.quit()
    save_marks(work, scene)


def duel_scene(work, scene, port, roms):
    """Kei and Mira's NetBattle from the ask to the result, both games
    playing it by themselves (arenaboth): the way in, the battle, the
    result card; both Yes on it, a rematch from the kept pair; its card."""
    t0 = time.time()
    kei = online.Remote(work, scene, 'kei', port, roms, FIGHT)
    mira = online.Remote(work, scene, 'mira', port, roms, FIGHT)
    into_square((kei, mira), t0)
    time.sleep(1)
    mira.do(at(*MIRA_SPOT) + ';10')
    kei.do(at(*KEI_SPOT) + ';10')
    time.sleep(2)
    mark(scene, kei, 'card')
    kei.do(CARD + ';4 A;120;4 A;60')
    for bout in (1, 2):
        for g in (kei, mira):
            online.wait(lambda: g.count('arena: ready, the pair') >= bout, f'{g.name} ready, bout {bout}', 240, t0)
            mark(scene, g, f'ready{bout}')
        for g in (kei, mira):
            online.wait(lambda: g.count('arena: over at frame') >= bout, f'{g.name}: bout {bout} over', 400, t0)
            mark(scene, g, f'over{bout}')
        time.sleep(7)
        if bout == 1:
            # (Yes on the card: LEFT to it, A; Kei first, Mira a second and a half after)
            for g in (kei, mira):
                g.do('4 LEFT;20;4 A;30')
                online.wait(lambda: g.has('yes on the card'), f'{g.name} said yes', 30, t0)
                mark(scene, g, 'yes')
                time.sleep(1.5)
    time.sleep(6)
    for g in (kei, mira):
        g.do('4 B;150')
    time.sleep(6)
    for g in (kei, mira):
        mark(scene, g, 'back')
    time.sleep(3)
    for g in (kei, mira):
        g.quit()
    save_marks(work, scene)


def teams(work, scene, port, roms, extra=None, watcher=False):
    """Four games in the Square, two teams: Kei asks Rio, Mira asks Dex,
    each answered Yes; the captains then stand apart (as drive.py's
    four-game proofs, docs/ONLINE_ARENA.md)."""
    t0 = time.time()
    extra = extra or {}
    g = {}
    for name in ('rio', 'kei', 'dex', 'mira'):
        g[name] = online.Remote(work, scene, name, port, roms, FIGHT + extra.get(name, []))
        into_square((g[name],), t0)
    nova = None
    if watcher:
        nova = online.Remote(work, scene, 'nova', port, roms, FIGHT)
        into_square((nova,), t0)
    kei, rio, mira, dex = g['kei'], g['rio'], g['mira'], g['dex']
    time.sleep(2)
    # (Kei faces Rio on the landmark's +x face, Mira faces Dex on its +y face)
    rio.do(at(19, 16.25, 5) + ';10')
    kei.do(at(19, 16.75, 1) + ';10')
    dex.do(at(16.25, 19, 3) + ';10')
    mira.do(at(16.75, 19, 7) + ';10')
    time.sleep(1)
    for asker, asked in ((mira, dex), (kei, rio)):
        mark(scene, asker, 'teamup')
        asker.do(CARD + ';4 DOWN;30;4 A;120;4 A;60')
        online.wait(lambda: asked.has(f'team: {asker.name.capitalize()} asks us to team up'),
                    f'{asked.name} asked by {asker.name}', 60, t0)
    time.sleep(0.5)
    for x in (rio, dex):
        mark(scene, x, 'asked')
        x.do('30;4 A;40;4 LEFT;30;4 A;60')
    for x in g.values():
        online.wait(lambda: x.has('team: a team with'), f'{x.name} on a team', 60, t0)
    time.sleep(0.5)
    # (each faced away from the others while the chat boxes are paged)
    spots = {'kei': at(19, 16.75, 3), 'rio': at(19, 16.25, 7), 'mira': at(16.75, 19, 1), 'dex': at(16.25, 19, 5)}
    for x in g.values():
        x.do('20;' + spots[x.name] + ';4 A;90;4 A;60')
    if nova:
        nova.do(at(15, 17.5, 5) + ';10')
    time.sleep(4)
    return t0, g, nova


def relay_scene(work, scene, port, roms):
    """A Team NetBattle one by one: Kei's team (Kei, Rio) against Mira's
    (Mira, Dex), the order picked blind (Kei sends Rio first, Mira goes
    herself), the bouts playing themselves, whoever waits watching."""
    t0, g, _ = teams(work, scene, port, roms, {'kei': ['teamform=1']})
    kei, rio, mira, dex = g['kei'], g['rio'], g['mira'], g['dex']
    mark(scene, kei, 'challenge')
    kei.do(at(17.25, 19, 7) + ';20;' + CARD + ';4 A;120;4 A;60')
    for x in (kei, mira):
        online.wait(lambda: x.has('team: match'), f'{x.name} in the team match', 60, t0)
    time.sleep(0.5)
    mark(scene, kei, 'order')
    kei.do('30;4 A;60;4 DOWN;40;4 A;60')
    mira.do('30;4 A;60;4 A;60')
    for x in g.values():
        online.wait(lambda: x.has('team: the order'), f'{x.name} told the order', 60, t0)
        mark(scene, x, 'ordered')
    for x in g.values():
        x.do('30;4 A;90;4 A;90;4 A;60')
        x.page = True
    bout = 0
    over = lambda: all('over, result' in x.text() for x in g.values())
    start = time.time()
    while not over() and time.time() - start < 600:
        called = max(x.count('team: bout ') for x in g.values())
        if called > bout:
            bout = called
            for x in g.values():
                mark(scene, x, f'bout{bout}')
        for x in g.values():
            n = x.count('arena: ready, the pair')
            if n and f'ready{n}' not in MARKS.get(scene, {}).get(x.name, {}):
                mark(scene, x, f'ready{n}')
            n = x.count('watch: the pair agrees')
            if n and f'watch{n}' not in MARKS.get(scene, {}).get(x.name, {}):
                mark(scene, x, f'watch{n}')
        time.sleep(0.5)
    for x in g.values():
        mark(scene, x, 'over')
    time.sleep(12)
    for x in g.values():
        x.page = False
        x.do('4 A;90;4 A;90')
    time.sleep(4)
    for x in g.values():
        x.quit()
    save_marks(work, scene)


def sides_scene(work, scene, port, roms):
    """Two MegaMen a side over the server: Kei's Battle on Mira asks how
    to fight (Side by side); all four in one battle, each partner on his
    own Custom screen (Rio's Cross, Dex's Beast Out: test partners whose
    choices cross as a player's do); Nova watching from the stands."""
    t0, g, nova = teams(work, scene, port, roms, {'rio': ['teamcross=1'], 'dex': ['teambeast=1']}, watcher=True)
    kei, rio, mira, dex = g['kei'], g['rio'], g['mira'], g['dex']
    mark(scene, kei, 'challenge')
    kei.do(at(17.25, 19, 7) + ';20;' + CARD + ';4 A;120;4 A;90;4 DOWN;40;4 A;120;4 A;60;4 A;60')
    for x in g.values():
        online.wait(lambda: x.has('team: match'), f'{x.name} in the side-by-side match', 90, t0)
        mark(scene, x, 'match')
        x.page = True
    # (Nova to the referee as the four go in: his WANNA WATCH? once the bout is on)
    time.sleep(6)
    mark(scene, nova, 'referee')
    nova.do(';'.join(['place -428 28 3;20;4 A;60'] + ['4 A;70'] * 14))
    for x in g.values():
        online.wait(lambda: x.has('both ready') or x.has('arena: ready, the pair'), f'{x.name} ready', 240, t0)
        mark(scene, x, 'ready')
    online.wait(lambda: nova.has('watch: the pair agrees'), 'Nova watching', 120, t0)
    mark(scene, nova, 'watching')
    over = lambda: all('over, result' in x.text() for x in g.values())
    online.wait(over, 'the match over', 600, t0)
    for x in g.values():
        mark(scene, x, 'over')
    time.sleep(12)
    for x in list(g.values()) + [nova]:
        x.page = False
        x.do('4 A;90;4 A;90')
    time.sleep(4)
    for x in list(g.values()) + [nova]:
        x.quit()
    save_marks(work, scene)


def online_scenes():
    return {
        'square': {'seed': 120, 'games': [square_game(i) for i in range(6)]},
        'ask': ask_scene,
        'duel': duel_scene,
        'relay': relay_scene,
        'sides': sides_scene,
    }


def scenes():
    """(none from the release pictures' runs: every scene is online_scenes's,
    captured by footage() below)"""
    return {}


def footage(keep):
    online.capture(kit.WORK, online_scenes(), keep)



# ---- the look: two sides ----

FPS, W, H = 30, 1280, 720
BAR = round(FPS * 60 * 4 / music.BPM)     # (48 frames at 150 beats a minute)
BEAT = BAR // 4
BARS = music.BARS
FRAMES = BARS * BAR + round(music.TAIL * FPS)

SANS_I = '/usr/share/fonts/truetype/ubuntu/UbuntuSans-Italic[wdth,wght].ttf'
SANS = '/usr/share/fonts/truetype/ubuntu/UbuntuSans[wdth,wght].ttf'

INK = (12, 10, 20)
WHITE = (250, 248, 244)
GOLD = (255, 204, 48)
# a side's colour, its dark and middle (the field's stripes) and its soft one (small words)
SIDES = {'r': ((238, 50, 72), (54, 9, 22), (84, 14, 32), (255, 194, 200)),
         'b': ((40, 126, 255), (8, 20, 62), (12, 34, 98), (192, 216, 255)),
         'g': (GOLD, (58, 42, 6), (92, 66, 10), (255, 236, 168))}
K = 0.25        # the seam's slant: it leans K pixels left for each one down
WIPE = 7        # a cut's sweep, frames


def F(bar, beat=0.0):
    return round(bar * BAR + beat * BEAT)


_fonts = {}


def font(size, wght=800, wdth=75, italic=True):
    key = (size, wght, wdth, italic)
    if key not in _fonts:
        f = ImageFont.truetype(SANS_I if italic else SANS, size)
        f.set_variation_by_axes([wdth, wght])
        _fonts[key] = f
    return _fonts[key]


def lerp(a, b, t):
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def ease(t):
    t = max(0.0, min(1.0, t))
    return 1 - (1 - t) ** 3


def seam_x(sx, y):
    return sx + K * (H / 2 - y)


# ---- the field: each side's stripes running toward the seam on the beat ----

P = 72          # the stripes' period across


def stripes(side):
    _, dk, mid, _ = SIDES[side]
    tw = W + 2 * P
    im = Image.new('RGB', (tw, H), dk)
    d = ImageDraw.Draw(im)
    for x in range(-P - round(K * H), tw + P, P):
        d.polygon([(x, 0), (x + P // 2, 0), (x + P // 2 - K * H, H), (x - K * H, H)], fill=mid)
    return im


_tex = {}
_vignette = []


def vignette():
    if not _vignette:
        v = Image.new('L', (W // 8, H // 8))
        px = v.load()
        for y in range(H // 8):
            for x in range(W // 8):
                r = math.hypot((x - W / 16) / (W / 16), (y - H / 16) / (H / 16))
                px[x, y] = int(max(0, min(210, (r - 0.5) * 300)))
        _vignette.append(v.resize((W, H), Image.BICUBIC))
    return _vignette[0]


def scroll(fr):
    """the stripes' run: half a period on each beat, in its first frames"""
    beat, phase = divmod(fr, BEAT)
    return (beat + ease(phase / 4)) * P / 2


def field(fr, sx=W / 2, still=False, dim=0.0, sides='rb'):
    """the frame's ground: the left side's colour left of the seam, the
    right side's right of it, their stripes running toward it"""
    for s in set(sides):
        if s not in _tex:
            _tex[s] = stripes(s)
    run = 0 if still else scroll(fr)
    left = _tex[sides[0]].crop((round(-run) % P, 0, round(-run) % P + W, H))
    right = _tex[sides[1]].crop((round(run) % P, 0, round(run) % P + W, H))
    mask = Image.new('L', (W, H), 0)
    ImageDraw.Draw(mask).polygon([(0, 0), (seam_x(sx, 0), 0), (seam_x(sx, H), H), (0, H)], fill=255)
    im = right.copy()
    im.paste(left, (0, 0), mask)
    im.paste(INK, (0, 0), vignette())
    if dim:
        im = Image.blend(im, Image.new('RGB', (W, H), INK), dim)
    return im


def seam(im, sx, glow=0.0, y0=0, y1=H):
    """the seam: a white line between the sides' colours; gold where the
    two sides meet (`glow`)"""
    d = ImageDraw.Draw(im)
    a, b = (seam_x(sx, y0), y0), (seam_x(sx, y1), y1)
    if glow > 0:
        d.line([a, b], fill=lerp(INK, GOLD, min(1.0, glow)), width=round(8 + 26 * glow))
    for dx, c in ((-5, SIDES['r'][0]), (5, SIDES['b'][0])):
        d.line([(a[0] + dx, a[1]), (b[0] + dx, b[1])], fill=c, width=4)
    d.line([a, b], fill=WHITE, width=4)


# ---- a player's screen: the game whole at a whole scale, his side's frame and plate ----

class Pic:
    """A player's game, `side` 'r' or 'b', at `scale` with its top left at
    (x, y). `feed` is [(frame, scene, sample, rate)]: from each frame on,
    that scene's footage from that sample (rate 0 holds it). A feed's cut
    inside a shot sweeps across the picture as the seam sweeps the frame."""

    def __init__(self, side, scale, x, y, feed, name=None, plate=None):
        self.side, self.s, self.x, self.y, self.feed, self.name = side, scale, x, y, feed, name
        self.plate = plate or ('l' if side == 'r' else 'r')
        self.pw, self.ph = 240 * scale, 160 * scale

    def source(self, fr):
        i = max([j for j, e in enumerate(self.feed) if e[0] <= fr] or [0])
        start, scene, sample, rate = self.feed[i]
        return i, scene, sample + max(0, int((fr - start) * rate))

    def goes_on(self, i):
        """whether feed entry i only holds or slows the footage where the last
        one had it (no cut to sweep)"""
        (s0, scene0, a, rate), (s1, scene1, b, _) = self.feed[i - 1], self.feed[i]
        return scene0 == scene1 and abs(a + int((s1 - s0) * rate) - b) <= 2

    def picture(self, fr):
        i, scene, sample = self.source(fr)
        pic = kit.shot(scene, sample)
        if self.s > 1:
            pic = pic.resize((self.pw, self.ph), Image.NEAREST)
        since = fr - self.feed[i][0]
        if i > 0 and since < 5 and not self.goes_on(i):
            # (the cut inside the shot: the new picture swept in behind a slanted line)
            old = self.picture(self.feed[i][0] - 1)
            edge = -K * self.ph + (self.pw + K * self.ph) * ease((since + 1) / 5)
            mask = Image.new('L', (self.pw, self.ph), 0)
            ImageDraw.Draw(mask).polygon([(0, 0), (edge + K * self.ph, 0), (edge, self.ph), (0, self.ph)], fill=255)
            old.paste(pic, (0, 0), mask)
            ImageDraw.Draw(old).line([(edge + K * self.ph, 0), (edge, self.ph)], fill=WHITE, width=2 * self.s)
            pic = old
        return pic

    def draw(self, im, fr, dim=0.0):
        color, dk, _, _ = SIDES[self.side]
        d = ImageDraw.Draw(im)
        x0, y0, x1, y1 = self.x, self.y, self.x + self.pw, self.y + self.ph
        sh = 10
        d.rectangle([x0 - 5 + sh, y0 - 5 + sh, x1 + 4 + sh, y1 + 4 + sh], fill=INK)
        d.rectangle([x0 - 5, y0 - 5, x1 + 4, y1 + 4], fill=color)
        d.rectangle([x0 - 2, y0 - 2, x1 + 1, y1 + 1], fill=INK)
        pic = self.picture(fr)
        if dim:
            pic = Image.blend(pic, Image.new('RGB', pic.size, INK), dim)
        im.paste(pic, (x0, y0))
        if self.name:
            f = font(30 if self.s >= 3 else 26)
            tw = f.getlength(self.name)
            h = 40 if self.s >= 3 else 34
            w = tw + 44
            yy = y1 - h // 2
            xx = x0 + 16 if self.plate == 'l' else x1 - 16 - w
            k = K * h
            d.polygon([(xx + k + 6, yy + 6), (xx + w + k + 6, yy + 6), (xx + w + 6, yy + h + 6), (xx + 6, yy + h + 6)], fill=dk)
            d.polygon([(xx + k, yy), (xx + w + k, yy), (xx + w, yy + h), (xx, yy + h)], fill=color)
            d.text((xx + 22 + k / 2, yy + h / 2), self.name, font=f, fill=WHITE, anchor='lm')


# where the games stand: one at 4x (the frame's middle), two at 2x apart
# on their sides, four at 2x staggered along the seam's slant
HERO = (160, 40)
DUO = [(56, 196), (744, 196)]
QUAD = [(118, 22), (742, 22), (58, 386), (682, 386)]


# ---- words: a call on one side, its answer on the other ----

def fit(text, width, size, least=18, wght=600, wdth=85, italic=False, lines=2):
    """`text` wrapped to `width` in at most `lines` lines, its font shrunk
    until it does"""
    for s in range(size, least - 1, -1):
        f = font(s, wght, wdth, italic)
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


class Ribbon:
    """A headline on a slanted band in its side's colour, run in from the
    frame's edge (`edge` 'l' or 'r') or out to the seam ('seam-l': the
    band's inner end on the seam, the left side's; 'seam-r' the right's),
    its ends leaning as the seam leans; a kicker over it and a caption
    under it, on a plate."""

    def __init__(self, start, end, text, side, y, edge='l', size=96, kicker=None, caption=None, sx=W / 2,
                 cap_width=520, gap=14):
        self.start, self.end, self.text, self.side, self.y = start, end, text, side, y
        self.edge, self.size, self.kicker, self.caption, self.sx, self.cap_width = edge, size, kicker, caption, sx, cap_width
        self.gap = gap

    def draw(self, im, fr):
        k = fr - self.start
        if k < 0 or fr >= self.end + 5:
            return
        color, dk, _, soft = SIDES[self.side]
        f = font(self.size)
        tw = f.getlength(self.text)
        h = round(self.size * 1.08)
        sk = K * h
        pad = 34
        y = self.y
        if self.edge == 'l':
            x0, x1 = -30, 48 + tw + pad
        elif self.edge == 'r':
            x0, x1 = W - 48 - tw - pad - sk, W + 30
        elif self.edge == 'seam-l':
            x0, x1 = -30, seam_x(self.sx, y + h) - self.gap
        else:
            x0, x1 = seam_x(self.sx, y + h) + self.gap, W + 30
        span = (x1 - x0) + sk + 60
        go = 1 - ease(k / 6) if fr < self.end else ease((fr - self.end) / 5)
        dx = -span * go if self.edge in ('l', 'seam-l') else span * go
        band = [(x0 + sk + dx, y), (x1 + sk + dx, y), (x1 + dx, y + h), (x0 + dx, y + h)]
        d = ImageDraw.Draw(im)
        d.polygon([(px + 8, py + 8) for px, py in band], fill=dk)
        d.polygon(band, fill=color)
        ink = INK if self.side == 'g' else WHITE
        if self.edge == 'l':
            tx, anchor = 48 + dx, 'lm'
        elif self.edge == 'r':
            tx, anchor = W - 48 - sk / 2 + dx, 'rm'
        elif self.edge == 'seam-l':
            tx, anchor = x1 - pad + sk / 2 + dx, 'rm'
        else:
            tx, anchor = x0 + pad + sk / 2 + dx, 'lm'
        d.text((tx, y + h / 2 + self.size * 0.02), self.text, font=f, fill=ink, anchor=anchor)
        right = anchor == 'rm'
        if self.kicker and fr < self.end:
            fk = font(round(self.size * 0.3) + 6, 700)
            a = ease((k - 2) / 5)
            d.text((tx + (sk if not right else sk / 2), y - 8), self.kicker, font=fk, fill=lerp(INK, soft, a),
                   anchor='rd' if right else 'ld')
        if self.caption and fr < self.end and k >= 5:
            a = ease((k - 5) / 5)
            fc, rows = fit(self.caption, self.cap_width, 30)
            lh = round(sum(fc.getmetrics()) * 1.05)
            cw = max(fc.getlength(r) for r in rows)
            cy = y + h + 22
            cx = tx - sk / 2 if not right else tx - sk / 2 - cw
            plate = Image.new('L', (round(cw) + 28, lh * len(rows) + 16), round(215 * a))
            im.paste(INK, (round(cx - 14), round(cy - 8)), plate)
            for i, r in enumerate(rows):
                d.text((cx if not right else cx + cw, cy + i * lh), r, font=fc, fill=lerp(INK, WHITE, a),
                       anchor='la' if not right else 'ra')


class Caption:
    """A line under the games on a plate, the middle of the frame"""

    def __init__(self, start, end, text, y=636, width=1000):
        self.start, self.end, self.text, self.y, self.width = start, end, text, y, width

    def draw(self, im, fr):
        k = fr - self.start
        if k < 0 or fr >= self.end:
            return
        a = ease(k / 5) * ((1 - ease((fr - (self.end - 4)) / 4)) if fr >= self.end - 4 else 1)
        fc, rows = fit(self.text, self.width, 32, lines=1)
        tw = fc.getlength(rows[0])
        lh = sum(fc.getmetrics())
        plate = Image.new('L', (round(tw) + 40, lh + 18), round(220 * a))
        im.paste(INK, (round(W / 2 - tw / 2 - 20), self.y - 9), plate)
        ImageDraw.Draw(im).text((W / 2, self.y + lh / 2), rows[0], font=fc, fill=lerp(INK, WHITE, a), anchor='mm')


class Badge:
    """the diamond on the seam where the two sides face each other"""

    def __init__(self, start, end, cx, cy, word='VS', side='g', size=64):
        self.start, self.end, self.cx, self.cy, self.word, self.side, self.size = start, end, cx, cy, word, side, size

    def draw(self, im, fr):
        k = fr - self.start
        if k < 0 or fr >= self.end:
            return
        s = [0.4, 0.8, 1.15, 1.1, 1.0][k] if k < 5 else 1.0
        if fr >= self.end - 4:
            s *= 1 - ease((fr - (self.end - 4)) / 4)
        if s <= 0.05:
            return
        cx, cy = self.cx, self.cy
        f = font(max(8, round(self.size * s)))
        tw = f.getlength(self.word)
        r = (max(self.size * 0.95, tw * 0.75) + 10) * s
        d = ImageDraw.Draw(im)
        d.polygon([(cx + 6, cy - r), (cx + r + 12, cy + 6), (cx + 6, cy + r + 12), (cx - r, cy + 6)], fill=INK)
        d.polygon([(cx, cy - r), (cx + r, cy), (cx, cy + r), (cx - r, cy)], fill=SIDES[self.side][0])
        d.polygon([(cx, cy - r + 9), (cx + r - 9, cy), (cx, cy + r - 9), (cx - r + 9, cy)], fill=INK)
        d.text((cx, cy + 2), self.word, font=f, fill=WHITE, anchor='mm')


class Burst:
    """where the two sides meet: gold diamonds thrown out from a point"""

    def __init__(self, at, cx, cy, size=1.0):
        self.at, self.cx, self.cy, self.size = at, cx, cy, size

    def draw(self, im, fr):
        k = fr - self.at
        if not 0 <= k < 14:
            return
        d = ImageDraw.Draw(im)
        cx, cy = self.cx, self.cy
        for j, (delay, grow) in enumerate(((0, 1.0), (3, 0.7))):
            t = (k - delay) / 11
            if t < 0 or t > 1:
                continue
            r = (30 + 190 * ease(t) * grow) * self.size
            w = max(1, round(12 * (1 - t)))
            d.line([(cx, cy - r), (cx + r, cy), (cx, cy + r), (cx - r, cy), (cx, cy - r)],
                   fill=lerp(GOLD, WHITE, 0.3 * (1 - t)) if j == 0 else GOLD, width=w)


class Text:
    """a line of words in the frame, faded in at `start`"""

    def __init__(self, start, end, text, x, y, size, color=WHITE, wght=800, wdth=75, italic=True, anchor='mm'):
        self.start, self.end, self.text, self.x, self.y = start, end, text, x, y
        self.size, self.color, self.wght, self.wdth, self.italic, self.anchor = size, color, wght, wdth, italic, anchor

    def draw(self, im, fr):
        k = fr - self.start
        if k < 0 or fr >= self.end:
            return
        a = ease(k / 6)
        ImageDraw.Draw(im).text((self.x, self.y + 10 * (1 - a)), self.text, fill=lerp(INK, self.color, a),
                                font=font(self.size, self.wght, self.wdth, self.italic), anchor=self.anchor)


# ---- the shots, in bars and beats ----

class Shot:
    """From `start` to the next shot: the field (its seam at `sx`, its
    stripes `still` in the drop), the players' games, the words and the
    rest drawn over them. It comes in behind the seam sweeping across the
    frame from the side that `push` names ('r' from the left, 'b' from the
    right; None: a cut in place)."""

    def __init__(self, start, pics=(), words=(), push='r', still=False, dim=0.0, sx=W / 2, glow=(), pic_dim=None,
                 sides='rb', span=(0, H)):
        self.start, self.pics, self.words, self.push, self.still, self.dim = start, pics, words, push, still, dim
        self.sx, self.glow, self.pic_dim, self.sides, self.span = sx, glow, pic_dim, sides, span

    def frame(self, fr):
        im = field(fr, self.sx, self.still, self.dim, self.sides)
        g = 0.0
        for at, n in self.glow:
            if at <= fr < at + n:
                g = max(g, 1 - (fr - at) / n)
        seam(im, self.sx, g, *self.span)
        dim = self.pic_dim(fr) if self.pic_dim else 0.0
        for p in self.pics:
            p.draw(im, fr, dim)
        for w in self.words:
            w.draw(im, fr)
        return im


def title_dim(start):
    return lambda fr: 0.62 * ease((fr - start) / 5)


def shots():
    hook = Pic('b', 4, *HERO, [(0, 'square-mira', 244, 1), (F(2, 1), 'square-mira', 352, 0), (F(2, 1) + 8, 'square-mira', 353, 1)],
               'MIRA')
    duo = lambda scene_l, feed_l, scene_r, feed_r, names=('KEI', 'MIRA'): [
        Pic('r', 2, *DUO[0], [(f, scene_l, s, r) for f, s, r in feed_l], names[0]),
        Pic('b', 2, *DUO[1], [(f, scene_r, s, r) for f, s, r in feed_r], names[1])]
    quad = lambda feeds, names: [Pic(side, 2, *QUAD[i], [(f, sc, s, r) for f, sc, s, r in feeds[i]], names[i])
                                 for i, side in enumerate('rbrb')]
    mid = (W / 2, DUO[0][1] + 160)
    MIRA = 434      # (the duel's footage: Mira's game a sample this far behind Kei's)
    S = {'kei': 0, 'mira': -115, 'rio': 54, 'dex': -56}     # (the two a side's four games against Kei's)
    out = [
        # 0: the field and its seam out of black, then Mira's PET swept in
        Shot(0, [], [], push=None),
        # 0-2: Kei waves at Mira (her PET), she waves back: the "!!" between them, held a moment
        Shot(6, [hook], [Ribbon(F(1), F(3), 'WAVE', 'r', 404, 'l', 112, kicker='KEI'),
                         Ribbon(F(2, 1), F(3), 'WAVE BACK', 'b', 528, 'r', 112, kicker='MIRA'),
                         Burst(F(2, 1), 648, 100, 1.2)],
             push='b', glow=[(F(2, 1), 16)]),
        # 3: the title over the two of them
        Shot(F(3), [hook], [Text(F(3), F(4), 'CYBERWORLD ENDLESS', W / 2, 212, 40, WHITE, 800),
                            Ribbon(F(3), F(4), 'PICK A', 'r', 252, 'seam-l', 168),
                            Ribbon(F(3, 0.5), F(4), 'SIDE', 'b', 252, 'seam-r', 168),
                            Text(F(3, 1.5), F(4), '0.14.0  ·  THE NET SQUARE', W / 2, 470, 30, SIDES['g'][3], 700)],
             push=None, pic_dim=title_dim(F(3)), glow=[(F(3), 12)]),
        # 4: SELECT's strip on Kei's PET, the cursor over Cheer at Mira, then Buster
        Shot(F(4), [Pic('r', 4, *HERO, [(F(4), 'square-kei', 98, 1), (F(4, 2.5), 'square-kei', 160, 1)], 'KEI')],
             [Ribbon(F(4), F(5), 'THE EMOTE STRIP', 'r', 64, 'r', 84, kicker='PRESS SELECT',
                     caption="TWELVE EMOTES, MEGAMAN'S OWN POSES; EIGHT PHRASES", cap_width=430)], push='r'),
        # 5: sent: Kei's Buster on his PET and on Mira's
        Shot(F(5), duo('square-kei', [(F(5), 181, 1)], 'square-mira', [(F(5), 181, 1)]),
             [Ribbon(F(5), F(6), 'STRIKE A POSE', 'r', 40, 'l', 72),
              Ribbon(F(5, 1), F(6), 'ON EVERY SCREEN', 'b', 104, 'r', 72),
              Caption(F(5, 1), F(6), "MEGAMAN'S OWN BN6 POSES AND SOUNDS, IN STEP ON EVERY PET")], push='b'),
        # 6: Mira's emotes as Kei sees them, one a beat
        Shot(F(6), [Pic('r', 4, *HERO, [(F(6), 'square-kei', 466, 1), (F(6, 1), 'square-kei', 540, 1),
                                         (F(6, 2), 'square-kei', 612, 1), (F(6, 3), 'square-kei', 684, 1)], 'KEI')],
             [Ribbon(F(6), F(7), 'TWELVE EMOTES', 'b', 64, 'r', 84, kicker="MIRA'S, ON KEI'S PET",
                     caption='SIX OF THEM NEW', cap_width=430)], push='r'),
        # 7: six MegaMen cheer at once, the Square cheers back: Kei's PET and Dex's
        Shot(F(7), duo('square-kei', [(F(7), 830, 1)], 'square-dex', [(F(7), 830 - 735, 1)], ('KEI', 'DEX')),
             [Ribbon(F(7), F(8), 'CHEER', 'r', 40, 'l', 72),
              Ribbon(F(7, 1), F(8), 'TOGETHER', 'b', 104, 'r', 72),
              Burst(F(7, 1), *mid, 0.8),
              Caption(F(7, 1), F(8), "THE BIGGER THE CROWD, THE BIGGER THE SQUARE'S CHEER")],
             push='b', glow=[(F(7, 1), 14)]),
        # 8: the ask: Kei's challenge, Mira's PET ringing, his card, "Take it,Lan?"
        Shot(F(8), duo('ask-kei', [(F(8), 376, 1)], 'ask-mira', [(F(8), 812, 1), (F(8, 2), 924, 1)]),
             [Ribbon(F(8), F(9), 'BATTLE?', 'r', 40, 'l', 72),
              Ribbon(F(8, 2), F(9), 'TAKE IT, LAN?', 'b', 104, 'r', 72),
              Caption(F(8), F(9), "HIS PET RINGS, AND HE READS YOUR MEGAMAN'S CARD")], push='r'),
        # 9: the way in on the PET's panel, then BN6's Custom screen, the battle
        Shot(F(9), duo('duel-kei', [(F(9), 368, 0.5), (F(9, 2), 380, 0.4), (F(9, 3), 600, 1)], 'duel-mira',
                       [(F(9), 368 + MIRA, 0.5), (F(9, 2), 380 + MIRA, 0.4), (F(9, 3), 600 + MIRA, 1)]),
             [Ribbon(F(9), F(10), 'NETBATTLE', 'r', 40, 'l', 72),
              Badge(F(9), F(10), *mid),
              Caption(F(9), F(10), "THE WAY IN ON THE PET: BOTH MEGAMEN AS CARRIED")], push='b'),
        # 10: the end, the result card
        Shot(F(10), duo('duel-kei', [(F(10), 964, 1), (F(10, 2), 1036, 1)], 'duel-mira',
                        [(F(10), 964 + MIRA, 1), (F(10, 2), 1036 + MIRA, 1)]),
             [Ribbon(F(10), F(11), 'THE RESULT CARD', 'b', 40, 'r', 72),
              Caption(F(10, 1), F(11), "HP LEFT, CHIPS USED, THE TIME AND THE SET'S SCORE")], push='r'),
        # 11: Yes and Yes on the card, back into the battle
        Shot(F(11), duo('duel-kei', [(F(11), 1244, 1), (F(11, 2), 1300, 1.2)], 'duel-mira',
                        [(F(11), 1244 + MIRA, 1), (F(11, 2), 1300 + MIRA, 1.2)]),
             [Ribbon(F(11), F(12), 'AGAIN?', 'r', 40, 'l', 72),
              Ribbon(F(11, 1), F(12), 'AGAIN!', 'b', 104, 'r', 72),
              Burst(F(11, 2) + 12, *mid, 0.8),
              Caption(F(11, 1), F(12), 'TWO YESES ON THE CARD, BACK IN ABOUT A SECOND')],
             push='b', glow=[(F(11, 2) + 12, 14)]),
        # 12 (the drop): a team of two, the order picked blind
        Shot(F(12), [Pic('r', 4, *HERO, [(F(12), 'relay-rio', 772, 1), (F(12, 2), 'relay-kei', 1234, 1)], 'RIO')],
             [Ribbon(F(12), F(12, 2), 'TEAM UP', 'r', 64, 'r', 84, kicker='TEAM NETBATTLES'),
              Ribbon(F(12, 2), F(13), 'WHO GOES FIRST?', 'r', 64, 'r', 84, caption='EACH CAPTAIN PICKS, BLIND',
                     cap_width=430)], push='r', still=True, dim=0.35),
        # 13: one by one: the winner stays on with what HP he has, the others watch
        Shot(F(13), duo('relay-rio', [(F(13), 2398, 1), (F(13, 2), 2476, 1)], 'relay-mira', [(F(13), 2398, 1)],
                        ('RIO', 'MIRA')),
             [Ribbon(F(13), F(14), 'ONE BY ONE', 'r', 40, 'l', 72),
              Ribbon(F(13, 2), F(14), 'THE REST WATCH', 'b', 104, 'r', 72),
              Caption(F(13), F(14), 'THE WINNER STAYS ON WITH THE HP HE HAS LEFT')], push='b', still=True, dim=0.2),
        # 14 (the climax): side by side, asked and gone into
        Shot(F(14), [Pic('r', 3, 280, 224, [(F(14), 'sides-kei', 1446, 1), (F(14, 2), 'sides-kei', 1802, 1)], 'KEI')],
             [Ribbon(F(14), F(15), 'SIDE', 'r', 52, 'seam-l', 128, gap=58),
              Ribbon(F(14, 1), F(15), 'SIDE', 'b', 52, 'seam-r', 128, gap=58),
              Badge(F(14, 0.5), F(15), seam_x(W / 2, 121), 121, 'BY', 'g', 52)], push='r', glow=[(F(14), 12)]),
        # 15: two MegaMen a side, the four PETs at once
        Shot(F(15), quad([[(F(15), f'sides-{n}', 2092 + S[n], 1)] for n in ('kei', 'mira', 'rio', 'dex')],
                         ['KEI', 'MIRA', 'RIO', 'DEX']),
             [Ribbon(F(15), F(16), 'TWO MEGAMEN A SIDE', 'g', 318, 'l', 72)], push='b'),
        # 16: each partner on his own Custom screen; Dex's Beast Out
        Shot(F(16), duo('sides-rio', [(F(16), 1890, 1), (F(16, 2), 2140, 1)], 'sides-dex',
                        [(F(16), 1806, 1), (F(16, 2), 2030, 1)], ('RIO', 'DEX')),
             [Ribbon(F(16), F(17), 'HIS OWN SCREEN', 'r', 40, 'l', 72),
              Caption(F(16), F(17), "EACH PARTNER: HIS OWN FOLDER, CROSS, BEAST OUT AND SOULS")], push='r'),
        # 17: the stands: the referee calls it, Nova watches
        Shot(F(17), [Pic('b', 4, *HERO, [(F(17), 'sides-nova', 1996, 1), (F(17, 2), 'sides-nova', 2190, 1),
                                          (F(17, 3), 'sides-nova', 2222, 1)], 'NOVA')],
             [Ribbon(F(17), F(18), 'FROM THE STANDS', 'g', 64, 'r', 84, kicker='ANYONE IN THE CHANNEL')],
             push='b'),
    ]
    return out


# ---- the logo and the end card ----

_logo = []


def ending(im, fr):
    k = fr - F(18)
    if not _logo:
        lg = Image.open(os.path.join(ROOT, 'linux', 'steam', 'logo.png')).convert('RGBA')
        _logo.append(lg.resize((lg.width * 3 // 4, lg.height * 3 // 4), Image.LANCZOS))
    lg = _logo[0]
    a = ease((k - 2) / 10)
    if a > 0:
        part = lg.copy()
        part.putalpha(part.getchannel('A').point(lambda v: int(v * a)))
        im.paste(part, ((W - lg.width) // 2, 64 + round(16 * (1 - a))), part)


def end_shot():
    notes = ('Unofficial fan project, not affiliated with Capcom. Free; runs from your own Mega Man Battle '
             'Network 6: Cybeast Gregar (USA) ROM.',
             'NetBattles need Mega Man Battle Network 5: Team Colonel (USA) beside it. On every system but the New 3DS.')
    words = [Ribbon(F(18, 1), FRAMES, 'PICK A', 'r', 300, 'seam-l', 96),
             Ribbon(F(18, 1.5), FRAMES, 'SIDE.', 'b', 300, 'seam-r', 96),
             Text(F(19), FRAMES, 'WAVE  ·  CHEER  ·  NETBATTLE  ·  TEAM UP  ·  SIDE BY SIDE', W / 2, 470, 34, WHITE, 700, 85),
             Text(F(19, 0.5), FRAMES, 'saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless', W / 2, 526, 28,
                  SIDES['g'][3], 600, 85, False)]
    words += [Text(F(19, 1), FRAMES, n, W / 2, 628 + 30 * i, 19, (176, 176, 196), 500, 85, False) for i, n in enumerate(notes)]
    s = Shot(F(18), [], words, push='b', dim=0.45, glow=[(F(18, 1), 24)], span=(276, 424))
    s.after = ending
    return s


# ---- the frames ----

def compose(only=None):
    frames_dir = os.path.join(kit.WORK, 'frames')
    if only is None:
        shutil.rmtree(frames_dir, ignore_errors=True)
    os.makedirs(frames_dir, exist_ok=True)
    timeline = shots() + [end_shot()]

    def draw(shot, fr):
        im = shot.frame(fr)
        if hasattr(shot, 'after'):
            shot.after(im, fr)
        return im

    for fr in range(FRAMES):
        if only is not None and fr not in only:
            continue
        i = max(j for j, s in enumerate(timeline) if s.start <= fr)
        shot = timeline[i]
        im = draw(shot, fr)
        k = fr - shot.start
        if i > 0 and shot.push and k < WIPE:
            # (the cut: the seam sweeps across the frame, the new shot behind it)
            old = draw(timeline[i - 1], fr)
            t = ease((k + 1) / (WIPE + 1))
            reach = W + K * H + 80
            mid = -K * H / 2 - 40 + reach * t if shot.push == 'r' else W + K * H / 2 + 40 - reach * t
            mask = Image.new('L', (W, H), 0)
            pts = [(0, 0), (seam_x(mid, 0), 0), (seam_x(mid, H), H), (0, H)] if shot.push == 'r' else \
                [(W, 0), (seam_x(mid, 0), 0), (seam_x(mid, H), H), (W, H)]
            ImageDraw.Draw(mask).polygon(pts, fill=255)
            old.paste(im, (0, 0), mask)
            im = old
            d = ImageDraw.Draw(im)
            color = SIDES[shot.push][0]
            a, b = (seam_x(mid, 0), 0), (seam_x(mid, H), H)
            d.line([a, b], fill=color, width=22)
            d.line([a, b], fill=WHITE, width=6)
        if fr < 10:
            # (the first frames: the field and its seam out of black)
            im = Image.blend(Image.new('RGB', (W, H), (0, 0, 0)), im, ease(fr / 9))
        end = FRAMES - 18
        if fr > end:
            im = Image.blend(im, Image.new('RGB', (W, H), (0, 0, 0)), min(1.0, (fr - end) / 16))
        im.save(os.path.join(frames_dir, f'{fr:05d}.png'), compress_level=1)
        if fr % 100 == 0:
            print('frame', fr, '/', FRAMES, flush=True)


POSTER = F(3, 2)        # (the title over the wave and its answer)
