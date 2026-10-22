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
# (round the plaza's landmark, in front of it on the screen: world x, y, facing)
RING = [(-76, 76, 1), (-60, 132, 0), (-4, 148, 7), (40, 140, 6), (-100, 40, 1), (-110, 120, 0)]
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
        v.at(3, '0:place -44 300 5,')
        v.at(5, '6:SELECT,')
        v.at(6.2, '6:RIGHT,')                 # (Cheer: the window plays it, AT MIRA)
        v.at(7.4, '6:DOWN,')                  # (Buster, TO EVERYONE)
        v.at(8.6, '6:A,')
        v.at(11.5, CHORD_UP)                  # (Wave, aimed at Mira, faced)
    elif name == 'mira':
        v.at(3, '0:place -60 300 1,')
        v.at(13.7, CHORD_UP)                  # (her Wave back: the "!!" between them)
        for k, (w, emote) in enumerate(((17.5, 5), (20, 2), (22.5, 10), (25, 6))):
            v.at(w, f'0:emote {emote},')      # (Bow, Laugh, Spin, Got it!)
    x, y, f = RING[i]
    v.at(28, f'0:place {x} {y} {f},')
    v.at(30 + 0.12 * i, '0:emote 1,')         # (all six cheer within a second: the Square answers)
    v.at(34 + 0.12 * i, '0:emote 5,')         # (... and bow in step)
    v.at(39)
    first = v.frame(2.5) if name in ('kei', 'mira') else v.frame(27) if name in ('dex', 'nova') else None
    return dict(name=name, operator=name.capitalize(), delay=0.5 * i, script=v.s, files=STAMPED,
                first=first, last=v.t, frames=v.t + 1)


# (remote play's steps: A on the faced MegaMan reads his card, three pages,
# then the box of Battle, Team up and More...)
CARD = '4 A;100;4 A;100;4 A;100;60'
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
    mira.do('place -60 300 1;10')
    kei.do('place -44 300 5;10')
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
    mira.do('place -60 300 1;10')
    kei.do('place -44 300 5;10')
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
    rio.do('place 28 404 5;10')
    dex.do('place 92 404 5;10')
    kei.do('place 44 404 5;10')
    mira.do('place 108 404 5;10')
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
    spots = {'kei': 'place 200 300 1', 'rio': 'place -100 300 1', 'mira': 'place 108 404 1', 'dex': 'place -100 500 1'}
    for x in g.values():
        x.do('20;' + spots[x.name] + ';4 A;90;4 A;60')
    if nova:
        nova.do('place -300 100 1;10')
    time.sleep(4)
    return t0, g, nova


def relay_scene(work, scene, port, roms):
    """A Team NetBattle one by one: Kei's team (Kei, Rio) against Mira's
    (Mira, Dex), the order picked blind (Kei sends Rio first, Mira goes
    herself), the bouts playing themselves, whoever waits watching."""
    t0, g, _ = teams(work, scene, port, roms, {'kei': ['teamform=1']})
    kei, rio, mira, dex = g['kei'], g['rio'], g['mira'], g['dex']
    mark(scene, kei, 'challenge')
    kei.do('place 124 404 5;20;' + CARD + ';4 A;120;4 A;60')
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
    kei.do('place 124 404 5;20;' + CARD + ';4 A;120;4 A;90;4 DOWN;40;4 A;120;4 A;60;4 A;60')
    for x in g.values():
        online.wait(lambda: x.has('team: match'), f'{x.name} in the side-by-side match', 90, t0)
        mark(scene, x, 'match')
        x.page = True
    for x in g.values():
        online.wait(lambda: x.has('both ready') or x.has('arena: ready, the pair'), f'{x.name} ready', 240, t0)
        mark(scene, x, 'ready')
    time.sleep(4)
    steps = ['place -428 28 3;20;4 A;60'] + ['4 A;70'] * 10
    mark(scene, nova, 'referee')
    nova.do(';'.join(steps))
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


def marks(scene):
    with open(os.path.join(kit.WORK, f'{scene}.marks.json')) as f:
        return json.load(f)
