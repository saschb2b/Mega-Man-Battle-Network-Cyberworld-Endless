#!/usr/bin/env python3
"""docs/screenshots/building-the-net.png: the "Building the net..." screen,
which a headless run never holds (scene_emu.c presents it once, through
platform_present_now, before the layer is made). The host build runs in a
window on a virtual display (Xvfb), ffmpeg grabs that display at 20 frames
a second (without the pointer), and the first frame that is white letters
on black and nothing else is the picture, halved to the game's 240x160.

    python3 tools/cw.py build host
    python3 tools/loading_picture.py [--seed N]

Needs Xvfb and ffmpeg on this machine, and the ROM in ~/.cache/mmbn-ref/roms
(CYBERWORLD_ROM_DIR another folder).
"""
import argparse
import glob
import os
import shutil
import subprocess
import sys

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'docs', 'screenshots', 'building-the-net.png')


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--seed', default='7')
    seed = ap.parse_args().seed
    tmp = os.path.join(ROOT, '.build', 'loading-picture')
    shutil.rmtree(tmp, ignore_errors=True)
    os.makedirs(os.path.join(tmp, 'data'))
    roms = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
    game = [os.path.join(ROOT, 'build', 'host', 'cyberworld'), '--scene', 'emu', '--seed', seed, '--size', '480x320',
            '--data-dir', os.path.join(tmp, 'data'), '--rom-dir', roms, '--dev', 'quiet']
    script = ('{game} >/dev/null 2>&1 & G=$!; ffmpeg -loglevel error -f x11grab -draw_mouse 0 -framerate 20 -video_size 480x320 '
              '-i $DISPLAY -t 3 {tmp}/f%03d.png; kill $G').format(game=' '.join(game), tmp=tmp)
    env = dict(os.environ, SDL_AUDIODRIVER='dummy', SDL_VIDEODRIVER='x11')
    subprocess.run(['xvfb-run', '-a', '-s', '-screen 0 480x320x24 -nocursor', 'sh', '-c', script], env=env, check=False)
    for path in sorted(glob.glob(os.path.join(tmp, 'f*.png'))):
        im = Image.open(path).convert('RGB')
        colours = im.getcolors(16)
        white = dict((c, n) for n, c in colours or []).get((255, 255, 255), 0)
        if colours and max(colours)[1] == (0, 0, 0) and white > 800:   # (the letters, at the window's 2x)
            im.resize((240, 160), Image.NEAREST).save(OUT, optimize=True)
            print('screenshot building-the-net')
            shutil.rmtree(tmp, ignore_errors=True)
            return 0
    print('loading_picture: no frame held the screen', file=sys.stderr)
    return 1


if __name__ == '__main__':
    sys.exit(main())
