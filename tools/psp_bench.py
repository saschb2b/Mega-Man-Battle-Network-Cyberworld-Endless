#!/usr/bin/env python3
"""A GBA core's cost on BN6 on a PSP, measured in PPSSPP (docs/engine/emulation/psp-and-ds.md).

Builds tools/psp_bench/bench.c twice for the PSP (pspdev's toolchain in
Docker): against mGBA 0.10.5, the core the game drives, and against libretro
gpSP, whose MIPS dynarec was written for the PSP. Runs each in PPSSPP's
headless runner (built from a pinned commit) at 333 MHz on BN6's scenes and
prints each frame's time in emulated milliseconds, against a 60th of a
second's 16.74.

    python3 tools/psp_bench.py [--frames 900] [--scenes battle,intro,boot]
                               [--cores mgba,gpsp] [--dump 90]

The scenes: battle and intro are romlab's states (tools/romlab:
~/.cache/mmbn-ref/lab/running.ss, BN6's plain ROM in a battle against
BlastMan, and cont.ss, NEW GAME's opening text), loaded into mGBA as they
are and converted into gpSP's state by the harness; boot is BN6 from
power-on through the title to NEW GAME's text by scripted input. The ROM is
CYBERWORLD_ROM_DIR's bn6g.gba (~/.cache/mmbn-ref/roms by default). Work and
pictures (--dump: a PPM each N frames) go to .build/psp-bench; nothing of
the game leaves it.

PPSSPP counts about a cycle for each MIPS instruction and no cache misses,
so a time it gives is the least the PSP itself would take.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PSPDEV = 'pspdev/pspdev@sha256:ee385df3aa783189d5a033af5eb1d4a9cdd93718fb8bf652ec7bb48e54038a8b'
PPSSPP_COMMIT = '1aebad2c651ff9d0684ee335e43aceff8fc7e3ec'   # 13 November 2026
GPSP_COMMIT = '5819380c2ffb0900219d700a382ee68c464ebb99'     # libretro/gpsp, 19 September 2026
MGBA_VER = '0.10.5'
MGBA_SHA = '91d6fbd32abcbdf030d58d3f562de25ebbc9d56040d513ff8e5c19bee9dacf14'
PPSSPP_IMAGE = 'cw-psp-bench-ppsspp'
SCENES = {   # name: (romlab's state or None, the input script)
    'battle': ('running.ss', None),
    'intro': ('cont.ss', None),
    'boot': (None, '480:,6:START,90:,6:A,120:,6:A,120:,6:A,120:,6:A,9000:'),
}

PPSSPP_DOCKERFILE = """FROM debian:trixie
RUN apt-get update -qq && apt-get install -y -qq build-essential cmake git python3 libsdl3-dev \\
 libcurl4-openssl-dev libgl1-mesa-dev libglu1-mesa-dev libegl-dev libfontconfig-dev libzstd-dev \\
 zlib1g-dev libpng-dev libopengl0 >/dev/null && rm -rf /var/lib/apt/lists/* \\
 && git config --system --add safe.directory '*'
"""

MGBA_PSP = """set -e
cd /w
OPTS="-DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON -DBUILD_SHARED=OFF -DBUILD_QT=OFF -DBUILD_SDL=OFF \\
 -DBUILD_GL=OFF -DBUILD_GLES2=OFF -DBUILD_GLES3=OFF -DUSE_EPOXY=OFF -DDISABLE_DEPS=ON -DUSE_DEBUGGERS=OFF \\
 -DUSE_GDB_STUB=OFF -DUSE_EDITLINE=OFF -DENABLE_SCRIPTING=OFF -DUSE_LUA=OFF -DM_CORE_GB=OFF -DM_CORE_GBA=ON \\
 -DUSE_DISCORD_RPC=OFF -DBUILD_LTO=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DLIBMGBA_ONLY=ON"
psp-cmake -S mgba-{ver} -B build-mgba-psp $OPTS -DCMAKE_C_FLAGS="-O2 -DDISABLE_THREADING -D_GNU_SOURCE -G0" \\
 -DHAVE_PTHREAD_H=OFF >/dev/null
cmake --build build-mgba-psp --target mgba -j8 >/dev/null
"""

EBOOT_MAKEFILE = """TARGET = cwbench
OBJS = {objs}
INCDIR = {incdir}
CFLAGS = -O2 -G0 -Wall {defs}
LIBS = {libs}
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = cwbench {core}
PSP_LARGE_MEMORY = 1
BUILD_PRX = 1
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
"""


def sh(cmd, **kw):
    print('+', ' '.join(cmd) if isinstance(cmd, list) else cmd, file=sys.stderr)
    subprocess.run(cmd, check=True, **kw)


def docker(work, image, script, name):
    """`script` in `image` as this user, the work folder at /w"""
    sh(['docker', 'run', '--rm', '--name', name, '--user', f'{os.getuid()}:{os.getgid()}',
        '-e', 'HOME=/tmp', '-v', f'{work}:/w', '-w', '/w', image, 'sh', '-c', script])


def setup_ppsspp(work):
    exe = os.path.join(work, 'ppsspp', 'build-headless', 'PPSSPPHeadless')
    if os.path.exists(exe):
        return
    img = os.path.join(work, 'ppsspp-image')
    os.makedirs(img, exist_ok=True)
    with open(os.path.join(img, 'Dockerfile'), 'w') as f:
        f.write(PPSSPP_DOCKERFILE)
    sh(['docker', 'build', '-q', '-t', PPSSPP_IMAGE, img])
    src = os.path.join(work, 'ppsspp')
    if not os.path.isdir(src):
        sh(['git', 'init', '-q', src])
        sh(['git', '-C', src, 'fetch', '-q', '--depth', '1', 'https://github.com/hrydgard/ppsspp.git', PPSSPP_COMMIT])
        sh(['git', '-C', src, 'checkout', '-q', 'FETCH_HEAD'])
        sh(['git', '-C', src, 'submodule', 'update', '-q', '--init', '--depth', '1', '--recursive'])
    # (headless without SDL: HEADLESS_CROSS; the build itself nice'd, other
    # sessions share the machine)
    docker(work, PPSSPP_IMAGE, 'cd ppsspp && cmake -B build-headless -DHEADLESS=ON -DHEADLESS_CROSS=ON '
           '-DCMAKE_BUILD_TYPE=Release -DUSE_DISCORD=OFF -DUSE_MINIUPNPC=OFF -DUNITTEST=OFF >/dev/null && '
           'nice -n 10 cmake --build build-headless --target PPSSPPHeadless -j 12 >/dev/null', 'cw-psp-bench-ppsspp')


def setup_cores(work):
    if not os.path.exists(os.path.join(work, 'build-mgba-psp', 'libmgba.a')):
        tar = os.path.join(work, 'mgba.tar.gz')
        sh(['curl', '-fsSL', '-o', tar, f'https://github.com/mgba-emu/mgba/archive/refs/tags/{MGBA_VER}.tar.gz'])
        sh(f'echo "{MGBA_SHA}  {tar}" | sha256sum -c -', shell=True)
        sh(['tar', 'xzf', tar, '-C', work])
        docker(work, PSPDEV, MGBA_PSP.format(ver=MGBA_VER), 'cw-psp-bench-mgba')
    gpsp = os.path.join(work, 'gpsp')
    if not os.path.exists(os.path.join(gpsp, 'gpsp_libretro_psp1.a')):
        if not os.path.isdir(gpsp):
            sh(['git', 'init', '-q', gpsp])
            sh(['git', '-C', gpsp, 'fetch', '-q', '--depth', '1', 'https://github.com/libretro/gpsp.git', GPSP_COMMIT])
            sh(['git', '-C', gpsp, 'checkout', '-q', 'FETCH_HEAD'])
        docker(work, PSPDEV, 'cd gpsp && make platform=psp1 -j8 >/dev/null', 'cw-psp-bench-gpsp')


def build_eboot(work, core):
    d = os.path.join(work, 'eboot-' + core)
    os.makedirs(d, exist_ok=True)
    shutil.copy(os.path.join(ROOT, 'tools', 'psp_bench', 'bench.c'), d)
    if core == 'mgba':
        for f in ('vfs-fd.c', 'vfs-dirent.c'):   # (mGBA's POSIX files, which a "Generic" system leaves out)
            shutil.copy(os.path.join(work, f'mgba-{MGBA_VER}', 'src', 'util', 'vfs', f), d)
        mk = EBOOT_MAKEFILE.format(objs='bench.o vfs-fd.o vfs-dirent.o', core=core,
                                   incdir=f'/w/mgba-{MGBA_VER}/include /w/build-mgba-psp/include',
                                   defs='-DBACKEND_MGBA -DDISABLE_THREADING -D_GNU_SOURCE',
                                   libs='/w/build-mgba-psp/libmgba.a -lm -lpsppower')
    else:
        mk = EBOOT_MAKEFILE.format(objs='bench.o', core=core, incdir='/w/gpsp/libretro/libretro-common/include',
                                   defs='-DBACKEND_GPSP', libs='/w/gpsp/gpsp_libretro_psp1.a -lstdc++ -lm -lpsppower')
    with open(os.path.join(d, 'Makefile'), 'w') as f:
        f.write(mk)
    docker(work, PSPDEV, f'cd eboot-{core} && make clean >/dev/null && make >/dev/null 2>&1 && test -f EBOOT.PBP',
           'cw-psp-bench-' + core)
    return os.path.join(d, 'EBOOT.PBP')


def lay_out(work, core, eboot, rom, lab):
    game = os.path.join(work, 'ms', 'PSP', 'GAME', 'cw' + core)
    os.makedirs(game, exist_ok=True)
    shutil.copy(eboot, game)
    shutil.copy(rom, os.path.join(game, 'bn6.gba'))
    for st, _ in SCENES.values():
        if st and os.path.exists(os.path.join(lab, st)):
            shutil.copy(os.path.join(lab, st), game)
    return game


def run(work, core, game, scene, frames, dump):
    st, script = SCENES[scene]
    for f in os.listdir(game):
        if f.endswith('.ppm'):
            os.remove(os.path.join(game, f))
    with open(os.path.join(game, 'bench.txt'), 'w') as f:
        f.write(f'frames {frames}\ndump {dump}\n')
        f.write(f'state {st}\nscript {frames}:\n' if st else f'script {script}\n')
    rel = os.path.relpath(game, work)
    out = subprocess.run(['docker', 'run', '--rm', '--name', f'cw-psp-bench-run-{core}', '--user',
                          f'{os.getuid()}:{os.getgid()}', '-e', 'HOME=/tmp', '-v', f'{work}:/w', PPSSPP_IMAGE,
                          '/w/ppsspp/build-headless/PPSSPPHeadless', '--memstick=/w/ms', '--timeout-wall=900',
                          f'/w/{rel}/EBOOT.PBP'], capture_output=True, text=True).stdout
    lines = [l for l in out.splitlines() if l.startswith('bench:')]
    if dump:
        shots = os.path.join(work, 'shots', f'{core}-{scene}')
        os.makedirs(shots, exist_ok=True)
        for f in os.listdir(game):
            if f.endswith('.ppm'):
                shutil.move(os.path.join(game, f), os.path.join(shots, f))
    return lines


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--frames', type=int, default=900)
    ap.add_argument('--scenes', default='battle,intro,boot')
    ap.add_argument('--cores', default='mgba,gpsp')
    ap.add_argument('--dump', type=int, default=0, help='a picture each N frames, into .build/psp-bench/shots')
    ap.add_argument('--work', default=os.path.join(ROOT, '.build', 'psp-bench'))
    a = ap.parse_args()
    work = os.path.abspath(a.work)
    os.makedirs(work, exist_ok=True)
    rom = os.path.join(os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms')), 'bn6g.gba')
    lab = os.path.expanduser('~/.cache/mmbn-ref/lab')
    if not os.path.exists(rom):
        sys.exit(f'psp_bench: no ROM at {rom} (CYBERWORLD_ROM_DIR)')
    setup_ppsspp(work)
    setup_cores(work)
    rows = []
    for core in a.cores.split(','):
        game = lay_out(work, core, build_eboot(work, core), rom, lab)
        for scene in a.scenes.split(','):
            st = SCENES[scene][0]
            if st and not os.path.exists(os.path.join(game, st)):
                print(f'psp_bench: {scene}: no {st} in {lab} (romlab\'s states), left out', file=sys.stderr)
                continue
            lines = run(work, core, game, scene, a.frames, a.dump)
            print('\n'.join(f'{core} {scene}: {l[7:]}' for l in lines if 'frames ' in l and ' in ' in l or 'heap' in l))
            m = re.search(r'mean ([\d.]+) ms, median ([\d.]+), p90 ([\d.]+), p99 ([\d.]+), max ([\d.]+)', '\n'.join(lines))
            heap = re.search(r'heap in use (\d+) KB', '\n'.join(lines))
            rows.append((core, scene, m.groups() if m else None, heap.group(1) if heap else '?'))
    print('\n| Core | Scene | Mean | Median | p90 | p99 | Max | Of 16.74 ms | Heap |')
    print('| --- | --- | --- | --- | --- | --- | --- | --- | --- |')
    for core, scene, g, heap in rows:
        if not g:
            print(f'| {core} | {scene} | failed | | | | | | |')
            continue
        print(f'| {core} | {scene} | ' + ' | '.join(g) + f' | {float(g[0]) / 16.74:.0%} | {int(heap) // 1024} MB |')


if __name__ == '__main__':
    main()
