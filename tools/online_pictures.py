#!/usr/bin/env python3
"""The release pictures of online play (issue #132): the Net Square with
eight players and their phrases round its landmark, the gate and the
NetPass, Trade Central's chip trade, a NetBattle on the Arena's ring, the
stands and the week's DeepNet on the Board.

A visit needs a server and other players, so these are not single
scripted games as build.py's SCREENSHOTS and CLIPS are. They are games in
a Square of their own on a local server, several at once, each scripted
by the wall clock (tools/trailer_online.py: the Linux build and the
ROM-free server, never the official one), as the 0.13.0 trailer's online
scenes were (tools/trailers/v0_13_0.py, online_scenes(), whose Visit
scripts these use); their places are the Square's of today
(src/online/square_plan.c, SQUARE_MAKE 3: the welcome plaza and its
parts across the void), given here by its panels. A picture is one whole
frame of one player's game, a clip a run of them, every second frame at
30 a second as build.py's clips are; a retake's frames may fall a frame
or two apart, the games keeping the clock's pace.

One more is the browser's (online-browser): the site's player in headless
Chrome beside a game on this machine, tools/online_browser_picture.py.

    python3 build.py screenshots online-square ...   (build.py makes the Linux build and the server first)
    python3 build.py clips online-battle ...
    python3 tools/online_pictures.py [--keep] [NAME ...]   (both kinds; --keep: the footage already captured)
"""
import os
import shutil
import sys

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import trailer_online as online  # noqa: E402
from trailers import v0_13_0 as cut  # noqa: E402  (Visit: a game's script by the wall clock)

WORK = os.path.join(ROOT, '.build', 'online-pictures')

# docs/screenshots/NAME.png: (scene, game, sample), a sample being the
# game's every second frame from its first kept (trailer_online.run_game)
SHOTS = {
    'online-gate': ('gate', 'kei', 90),         # MegaMan's question at the gate, the NetPass's strip over it
    'online-netpass': ('gate', 'kei', 360),     # the full pass, after the greeter stamped it
    'online-deepnet': ('gate', 'kei', 930),     # the Board's DeepNet: the week's deepest dives, the Board by MegaMan
    'online-square': ('meet', 'kei', 600),      # eight players round the landmark, their phrases over them
    'online-trade': ('trade', 'deal', 30),      # the chip trade, MegaMan's question, the cursor on Yes
    'online-battle': ('battle', 'kei', 396),    # a NetBattle, Kei's side: Mira's charged shot, Kei's flash
    'online-stands': ('battle', 'dex', 474),    # Dex in the stands watching it (Kei's shot landing), the referee's keys under it
}

# docs/screenshots/NAME.png of the browser, by tools/online_browser_picture.py
# (agent-browser's Chrome, the browser build; build.py makes the browser build too)
BROWSER = ('online-browser',)

# docs/clips/NAME.*: (scene, game, first sample, last sample, poster sample)
CLIPS = {
    'online-square': ('meet', 'kei', 480, 710, 600),      # round the landmark, the channel's phrases popping
    'online-netpass': ('gate', 'kei', 150, 430, 360),     # the greeter's STAMP,STAMP,STAMP!, the stamps landing
    'online-trade': ('trade', 'deal', 0, 185, 30),        # Yes, waiting on Beta, the cards crossing
    'online-battle': ('battle', 'kei', 296, 616, 396),    # charged shots both ways, from Kei's side
}


# ---- the scenes: the Square of seed 120 (Central Area, the Undernet's
# statue ringed by braziers), its panels from square_plan.c ----

SEED = 120
# (the landmark's world x, y in that Square: a visit's log says
# "online: square area 0, arrival at ..., the landmark at 20,44")
LANDMARK = (20, 44)


def floor(dx, dy, facing):
    """`place`'s world x, y and facing for the Square's panel (dx, dy) from
    the plaza's middle, where the landmark stands (square_plan.c's CX, CY;
    its places, props and parts are given from there): netmap_world, world
    x falling 32 a panel of grid y and world y rising 32 a panel of grid x.
    The screen: grid x down to the right, grid y down to the left, about
    four panels each way from MegaMan. Facing is BN6's, 0 up the screen:
    1 up-right (grid -y), 3 down-right (+x), 5 down-left (+y), 7 up-left
    (-x)."""
    return round(LANDMARK[0] - 32 * dy), round(LANDMARK[1] + 32 * dx), facing


def at(dx, dy, facing):
    return '0:place %d %d %d,' % floor(dx, dy, facing)


# (the plaza: 9 panels a side, -4 to 4; the landmark's base walled off from
# -1 to 1, the braziers at the corners of (2, 2); the arrival at (3, 0),
# the keeper (4, -2), the greeter (4, 2), the Board's front (3, -4) before
# it at (3, -5). The eight round the landmark's front, the picture Kei's:
# on its +x face and its +y face, and a row behind, each a panel or so
# apart, facing it)
RING = [(2, 1, 7), (2, -0.25, 7), (1, 2, 1), (0, 2, 1), (3, 0, 7), (3, 1.5, 0), (2.5, 2.75, 0), (2, -1.25, 6)]
OPERATORS = ['Kei', 'Mira', 'Dex', 'Rio', 'Nova', 'Juno', 'Tomo', 'Sora']
DELAYS = [0, 2.5, 4, 5, 6, 7, 7.5, 8]


def meet_game(i):
    """The eight arriving in the plaza, the first two saying hello, then
    round the landmark: their phrases, the Square's cheer, an emote each."""
    v = cut.Visit(DELAYS[i])
    if i < 2:
        v.at(4 + 0.3 * i, cut.say(v, 0))
    v.at(13, at(*RING[i]))
    v.at(15 + 0.6 * (i % 4), cut.say(v, i % 2))
    v.at(26 + 0.4 * i, cut.emote(v, i % 2))
    v.at(40)
    first = 1150 if i < 2 else v.frame(12)
    return dict(name=OPERATORS[i].lower(), operator=OPERATORS[i], delay=DELAYS[i], script=v.s, files=cut.STAMPED,
                first=first, last=v.t)


def gate_scene():
    """Kei through the gate as a player goes, his first visit with the full
    pass (the greeter's stamp moment by the arrival), then the Board's posts
    and the week's DeepNet, from its front (its entries seeded on the
    server)."""
    v = cut.Visit(0, ask=True)
    v.at(2.6, '6:A,')          # (the greeter's first page)
    v.at(4.6, '6:A,')          # (... and "STAMP,STAMP,STAMP!": the pass opens, the stamps land)
    v.at(11, '6:A,')           # (the full pass closed)
    v.at(12.5, at(3, -4, 1) + '20:,6:A,')      # (the Board, past the plaza's back rim, from its front)
    for k in range(10):
        v.at(14 + 2.2 * k, '6:A,')
    v.at(37)
    return dict(name='kei', operator='Kei', ask=True, script=v.s, first=1150, last=v.t)


def battle_scene():
    """A NetBattle on the Arena's ring (the part up the plaza's -y warp:
    its stands' floor -3 to 3 by -12 to -10, the ring -2 to 2 by -18 to
    -15), Kei and Mira face to face on it, both games playing it by
    script (charged shots); Dex on the stands before the referee (2, -12),
    in the stands from his WANNA WATCH?, cheering, and the referee's word
    after it."""
    mira = cut.Visit(0).at(3, at(0, -16.75, 1)).at(55)
    kei = cut.Visit(1).at(4, at(0, -17.25, 5) + '20:,')
    for w in (5, 6.2, 7.4, 8.6, 9.8, 11):       # (his card's pages typed out, then Battle)
        kei.at(w, '6:A,')
    kei.at(40, '6:A,').at(55)                  # (back in the Square: "Alright! We won,Lan!!")
    dex = cut.Visit(2).at(9, at(1, -12, 3) + '20:,')
    for k in range(9):
        dex.at(11 + 1.4 * k, '6:A,')            # (the referee's pages, until his WANNA WATCH?'s Yes)
    for w in (25, 27, 29):
        dex.at(w, '6:L,')                       # (cheers)
    dex.at(31, '6:R,')
    for k in range(8):
        dex.at(39 + 1.4 * k, '6:A,')            # (back in the Square, the referee calls the bout)
    dex.at(55)
    hp = ['arenahp=40']
    return {'seed': SEED, 'games': [
        dict(name='mira', operator='Mira', delay=0, script=mira.s, dev=['arenaboth'] + hp, files=cut.STAMPED,
             first=mira.frame(4), last=mira.t),
        dict(name='kei', operator='Kei', delay=1, script=kei.s, dev=['arenaboth'] + hp, files=cut.STAMPED,
             first=kei.frame(4), last=kei.t),
        dict(name='dex', operator='Dex', delay=2, script=dex.s, files=cut.STAMPED,
             first=dex.frame(9), last=dex.t),
    ]}


def online_scenes():
    """The scenes the pictures come from (trailer_online.capture's); the
    chip trade is Trade Central's two screens as the trade dev step opens
    them in a run, no visit, the 0.13.0 cut's own."""
    return {
        'gate': {'seed': SEED, 'weeks': cut.WEEK, 'games': [gate_scene()]},
        'meet': {'seed': SEED, 'games': [meet_game(i) for i in range(8)]},
        'battle': battle_scene(),
        'trade': cut.online_scenes()['trade'],
    }


def scenes_for(names):
    return {spec[0] for n, spec in list(SHOTS.items()) + list(CLIPS.items()) if n in names}


def footage(names, keep=False):
    """The scenes the pictures named need, captured into WORK (with
    `keep`, those already there kept)."""
    online.capture(WORK, online_scenes(), keep, only=scenes_for(names))


def sample(scene, game, i):
    return Image.open(os.path.join(WORK, f'{scene}-{game}', f'{i:04d}.png')).convert('RGB')


def screenshots(names):
    out = os.path.join(ROOT, 'docs', 'screenshots')
    for name in names:
        if name in SHOTS:
            sample(*SHOTS[name]).save(os.path.join(out, f'{name}.png'), optimize=True)
            print('screenshot', name)
    return 0


def clips(names, encode=None):
    if encode is None:
        import build
        encode = build.encode_clip
    out = os.path.join(ROOT, 'docs', 'clips')
    tmp = os.path.join(ROOT, '.build', 'clips')
    for name in names:
        if name not in CLIPS:
            continue
        scene, game, first, last, poster = CLIPS[name]
        shutil.rmtree(tmp, ignore_errors=True)
        os.makedirs(os.path.join(tmp, 'png'))
        for n, i in enumerate(range(first, last + 1)):
            shutil.copy(os.path.join(WORK, f'{scene}-{game}', f'{i:04d}.png'), os.path.join(tmp, 'png', f'{n:05d}.png'))
        sample(scene, game, poster).save(os.path.join(out, f'{name}.png'), optimize=True)
        if encode(name, tmp, out, last + 1 - first, (240, 160)):
            return 1
    return 0


def main(argv, kinds=('screenshots', 'clips'), keep=None, encode=None):
    """build.py's screenshots or clips for the online pictures in argv (all
    of them where none is named; `encode` build.py's encode_clip)."""
    keep = '--keep' in argv if keep is None else keep
    names = [a for a in argv if not a.startswith('--')]
    pool = (SHOTS.keys() | set(BROWSER) if 'screenshots' in kinds else set()) | (CLIPS if 'clips' in kinds else {}).keys()
    names = sorted(n for n in pool if not names or n in names)
    scenes = [n for n in names if n not in BROWSER]
    if scenes:
        footage(scenes, keep)
    code = screenshots(scenes) if 'screenshots' in kinds else 0
    if not code and any(n in BROWSER for n in names):
        if shutil.which('agent-browser'):
            import online_browser_picture
            code = online_browser_picture.main()
        else:
            print('online-browser skipped: it needs agent-browser (npm i -g agent-browser)')
    return code or (clips(scenes, encode) if 'clips' in kinds and scenes else 0)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
