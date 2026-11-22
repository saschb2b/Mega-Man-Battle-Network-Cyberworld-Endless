#!/usr/bin/env python3
"""The PSP build's pictures for a release's notes: build/psp's EBOOT started
in PPSSPP's headless runner (tools/psp_bench.py builds it, psp/README.md,
Testing on a PSP and in PPSSPP), a whole frame of the PSP's 480x272 screen
at an emulated second.

    python3 build.py psp                     # the EBOOT (TAG=vX.Y.Z for a release's)
    python3 tools/psp_picture.py [NAME ...]  # docs/screenshots/NAME.png
    python3 tools/psp_picture.py --bench DIR # another checkout's .build/psp-bench, its runner built already

A picture: its second, and args.txt's words (the command line a PSP's
start has not; none is the game's own start). The memory stick is made
afresh under .build/psp-picture, BN6's ROM copied into its rom/ for the
run and removed after it. PPSSPP writes its 512-wide buffer; the screen
is its left 480.
"""
import argparse
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import build  # noqa: E402
import psp_bench  # noqa: E402

# docs/screenshots/NAME.png: (emulated second, args.txt)
SHOTS = {
    # 0.22.0's notes: saves, the dive, systems
    'psp-title': (30, ''),                                              # the title, after the start's screens
    'psp-layer': (27, '--scene emu --seed 7 --run-depth 1 --dev quiet'),   # a run's first layer, MegaMan's first words
}


def picture(name, bench):
    from PIL import Image
    second, args = SHOTS[name]
    work = os.path.join(ROOT, '.build', 'psp-picture', name)
    shutil.rmtree(work, ignore_errors=True)
    game = os.path.join(work, 'ms', 'PSP', 'GAME', 'cyberworld-endless')
    os.makedirs(os.path.join(game, 'rom'))
    shutil.copy(os.path.join(ROOT, 'build', 'psp', 'EBOOT.PBP'), game)
    rom = os.path.join(game, 'rom', 'bn6.gba')
    shutil.copy(build.docs_rom_mounts()[0][0], rom)
    if args:
        with open(os.path.join(game, 'args.txt'), 'w', encoding='utf-8') as f:
            f.write(args + '\n')
    try:
        code = subprocess.call(['docker', 'run', '--rm', '--name', f'cw-psp-picture-{os.getpid()}-{name}', '--user',
                                f'{os.getuid()}:{os.getgid()}', '-e', 'HOME=/tmp', '-v', f'{bench}:/w:ro', '-v', f'{work}:/s',
                                psp_bench.PPSSPP_IMAGE, '/w/ppsspp/build-headless/PPSSPPHeadless', '--memstick=/s/ms',
                                '--timeout-wall=600', f'--timeout-emulated={second + 1}', f'--screenshot-at={second}:/s/shot.png',
                                '/s/ms/PSP/GAME/cyberworld-endless/EBOOT.PBP'], stdout=subprocess.DEVNULL)
    finally:
        os.remove(rom)
    shot = os.path.join(work, 'shot.png')
    if not os.path.exists(shot):
        print(f'{name}: no picture (PPSSPP ended with {code})')
        return 1
    Image.open(shot).convert('RGB').crop((0, 0, 480, 272)).save(os.path.join(ROOT, 'docs', 'screenshots', f'{name}.png'), optimize=True)
    shutil.rmtree(work, ignore_errors=True)
    print('screenshot', name)
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('names', nargs='*')
    ap.add_argument('--bench', default=os.path.join(ROOT, '.build', 'psp-bench'))
    a = ap.parse_args()
    if not os.path.exists(os.path.join(ROOT, 'build', 'psp', 'EBOOT.PBP')):
        sys.exit('no build/psp/EBOOT.PBP: python3 build.py psp')
    bench = os.path.abspath(a.bench)
    psp_bench.setup_ppsspp(bench)
    for name in a.names or SHOTS:
        if name not in SHOTS:
            sys.exit(f'{name}: not one of {", ".join(SHOTS)}')
        if picture(name, bench):
            return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
