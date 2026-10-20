#!/usr/bin/env python3
"""The release pictures of online play (issue #132): the Net Square with
eight players and their phrases, the gate and the NetPass, Trade Central's
chip trade, a NetBattle, the stands and the week's DeepNet on the Board.

A visit needs a server and other players, so these are not single
scripted games as build.py's SCREENSHOTS and CLIPS are. They are the
0.13.0 trailer's online scenes (tools/trailers/v0_13_0.py,
online_scenes()): games in a Square of their own on a local server,
several at once, each scripted by the wall clock (tools/trailer_online.py:
the Linux build and the ROM-free server, never the official one). A
picture is one whole frame of one player's game, a clip a run of them,
every second frame at 30 a second as build.py's clips are; a retake's
frames may fall a frame or two apart, the games keeping the clock's pace.

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
from trailers import v0_13_0 as cut  # noqa: E402

WORK = os.path.join(ROOT, '.build', 'online-pictures')

# docs/screenshots/NAME.png: (scene, game, sample), a sample being the
# game's every second frame from its first kept (trailer_online.run_game)
SHOTS = {
    'online-gate': ('gate', 'kei', 90),         # MegaMan's question at the gate, the NetPass's strip over it
    'online-netpass': ('gate', 'kei', 360),     # the full pass, after the greeter stamped it
    'online-deepnet': ('gate', 'kei', 930),     # the Board's DeepNet: the week's deepest dives
    'online-square': ('meet', 'kei', 770),      # eight players round the landmark, their phrases over them
    'online-trade': ('trade', 'deal', 30),      # the chip trade, Deal! lit
    'online-battle': ('battle', 'kei', 400),    # a NetBattle, Kei's side
    'online-stands': ('battle', 'dex', 480),    # Dex in the stands watching it, the referee's keys under it
}

# docs/clips/NAME.*: (scene, game, first sample, last sample, poster sample)
CLIPS = {
    'online-square': ('meet', 'kei', 600, 830, 770),      # Say something, Hello!, the channel's phrases popping
    'online-netpass': ('gate', 'kei', 150, 430, 360),     # the greeter's STAMP,STAMP,STAMP!, the stamps landing
    'online-trade': ('trade', 'deal', 0, 185, 30),        # Deal!, waiting on Beta, the cards crossing
    'online-battle': ('battle', 'kei', 300, 620, 400),    # charged shots both ways, from Kei's side
}


def scenes_for(names):
    return {spec[0] for n, spec in list(SHOTS.items()) + list(CLIPS.items()) if n in names}


def footage(names, keep=False):
    """The scenes the pictures named need, captured into WORK (with
    `keep`, those already there kept)."""
    online.capture(WORK, cut.online_scenes(), keep, only=scenes_for(names))


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
    pool = (SHOTS if 'screenshots' in kinds else {}).keys() | (CLIPS if 'clips' in kinds else {}).keys()
    names = sorted(n for n in pool if not names or n in names)
    if not names:
        return 0
    footage(names, keep)
    code = screenshots(names) if 'screenshots' in kinds else 0
    return code or (clips(names, encode) if 'clips' in kinds else 0)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
