#!/usr/bin/env python3
"""Build, test and package Cyberworld Endless.

Compilation runs inside the `cyberworld-build` Docker image (Debian trixie),
which matches the glibc and SDL2 that current ROCKNIX ships.

  python3 build.py              host and device binaries
  python3 build.py host         host binary only
  python3 build.py device       aarch64 binary only
  python3 build.py shot ...     run the host binary headlessly (options below)
  python3 build.py atlas [BIOMES] [SEEDS]
                                every area's layers drawn, one sheet per area
                                in .build/atlas (docs/DEVTOOLS.md)
  python3 build.py test         ROM-free unit tests
  python3 build.py package      assemble build/port/ for PortMaster
"""
import argparse
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
IMAGE = 'cyberworld-build'
CONTEXT = os.environ.get('DOCKER_CONTEXT_NAME', 'desktop-linux')


def docker(*cmd, mounts=()):
    args = ['docker']
    if CONTEXT:
        args += ['--context', CONTEXT]
    args += ['run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src']
    for host, guest in mounts:
        args += ['-v', f'{host}:{guest}']
    for var in ('CYBERWORLD_AUDIO_DUMP', 'CYBERWORLD_SFX_LOG', 'CYBERWORLD_AUDIO_OFFLINE', 'CYBERWORLD_EMU_DEBUG', 'CYBERWORLD_AUTOPILOT'):
        if os.environ.get(var):
            args += ['-e', f'{var}={os.environ[var]}']
    args += [IMAGE, *cmd]
    return subprocess.call(args)


def ensure_image():
    probe = ['docker'] + (['--context', CONTEXT] if CONTEXT else []) + ['image', 'inspect', IMAGE]
    if subprocess.call(probe, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) != 0:
        build = ['docker'] + (['--context', CONTEXT] if CONTEXT else []) + ['build', '-t', IMAGE, os.path.join(ROOT, 'docker')]
        if subprocess.call(build) != 0:
            sys.exit('docker build failed')


def build(target):
    ensure_image()
    if docker('make', f'TARGET={target}', f'-j{os.cpu_count() or 4}') != 0:
        sys.exit(f'{target} build failed')


def package():
    out = os.path.join(ROOT, 'build', 'port')
    game = os.path.join(out, 'cyberworld')
    if os.path.isdir(out):
        shutil.rmtree(out)
    os.makedirs(os.path.join(game, 'rom'))
    shutil.copy2(os.path.join(ROOT, 'build', 'aarch64', 'cyberworld.aarch64'), game)
    shutil.copy2(os.path.join(ROOT, 'port', 'README.md'), game)
    shutil.copy2(os.path.join(ROOT, 'port', 'gameinfo.xml'), game)
    shutil.copy2(os.path.join(ROOT, 'port', 'port.json'), game)
    shutil.copy2(os.path.join(ROOT, 'LICENSE'), game)
    shutil.copytree(os.path.join(ROOT, 'build', 'aarch64', 'licenses'), os.path.join(game, 'licenses'))
    with open(os.path.join(game, 'rom', 'PUT_YOUR_ROM_HERE.txt'), 'w') as f:
        f.write('Copy your own Mega Man Battle Network 6: Cybeast Gregar (USA) .gba file into this folder.\n')
    shutil.copy2(os.path.join(ROOT, 'port', 'Cyberworld Endless.sh'), out)
    print('packaged', out)


def atlas(biomes='all', seeds='1'):
    """Every area's generated layers, drawn headless, one PNG sheet per area."""
    out = os.path.join(ROOT, '.build', 'atlas')
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    rom_dir = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
    code = docker('build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', '/src/.build/data',
                  '--atlas', f'/src/.build/atlas:{biomes}:{seeds}', mounts=[(rom_dir, '/rom:ro')])
    if code:
        return code
    from PIL import Image
    import collections
    import glob
    import re
    layers = collections.defaultdict(list)
    for path in sorted(glob.glob(os.path.join(out, 'b*.bmp'))):
        layers[int(re.match(r'b(\d+)_', os.path.basename(path)).group(1))].append(path)
    for biome, paths in layers.items():
        whole, crops = [], []
        for path in paths:
            im = Image.open(path).convert('RGB')
            box = im.getbbox() or (0, 0, im.width, im.height)
            im = im.crop(box)
            thumb = im.copy()
            thumb.thumbnail((400, 400))
            whole.append(thumb)
            crops.append(densest(im, 200, 130).resize((400, 260), Image.NEAREST))
        sheet = Image.new('RGB', (400 * len(paths), 400 + 260), (20, 20, 24))
        for i, (t, c) in enumerate(zip(whole, crops)):
            sheet.paste(t, (i * 400, 0))
            sheet.paste(c, (i * 400, 400))
        sheet.save(os.path.join(out, f'sheet_b{biome:02d}.png'))
        for path in paths:
            os.remove(path)
    report = open(os.path.join(out, 'report.txt')).read()
    print(report, end='')
    flagged = [l for l in report.splitlines() if 'NOT BUILT' in l or 'arena NO' in l or
               float(re.search(r'fallback ([\d.]+)%', l).group(1)) > 1.0]
    print(f'{len(layers)} sheets in .build/atlas; {len(flagged)} layers flagged')
    for l in flagged:
        print('  !', l)
    return 0


def densest(im, w, h):
    """The w x h window with the most floor in it."""
    px = im.load()
    step = 10
    best = (0, 0, -1)
    for y0 in range(0, max(1, im.height - h), step):
        for x0 in range(0, max(1, im.width - w), step):
            n = sum(px[x, y] != (40, 40, 48) for y in range(y0, min(im.height, y0 + h), step) for x in range(x0, min(im.width, x0 + w), step))
            if n > best[2]:
                best = (x0, y0, n)
    return im.crop((best[0], best[1], best[0] + w, best[1] + h))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('action', nargs='?', default='all', choices=['all', 'host', 'device', 'package', 'shot', 'asan', 'test', 'clean', 'atlas'])
    ap.add_argument('rest', nargs=argparse.REMAINDER)
    a = ap.parse_args()
    if a.action == 'clean':
        shutil.rmtree(os.path.join(ROOT, 'build'), ignore_errors=True)
        return
    if a.action == 'test':
        ensure_image()
        sys.exit(docker('make', 'test'))
    if a.action == 'atlas':
        build('host')
        sys.exit(atlas(*a.rest[:2]))
    if a.action in ('all', 'host', 'shot'):
        build('host')
    if a.action == 'asan':
        build('asan')
    if a.action in ('all', 'device', 'package'):
        build('aarch64')
    if a.action == 'package':
        package()
    if a.action in ('shot', 'asan'):
        # Headless run inside the build image; the ROM directory is mounted read-only.
        rom_dir = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
        extra = [] if '--data-dir' in a.rest else ['--data-dir', '/src/.build/data']
        os.makedirs(os.path.join(ROOT, '.build', 'data'), exist_ok=True)
        binary = 'build/asan/cyberworld' if a.action == 'asan' else 'build/host/cyberworld'
        code = docker(binary, '--headless', '--rom-dir', '/rom', *extra, *a.rest,
                      mounts=[(rom_dir, '/rom:ro')])
        sys.exit(code)


if __name__ == '__main__':
    main()
