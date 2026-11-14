#!/usr/bin/env python3
"""layer_bench.py: the layer lab. A change to how layers are made, against a
base, over every area's layers (docs/devtools/profiling.md, The lab).

Both builds draw every area's layers (the atlas, without the game: no boot,
no ROM hash, no installs) and must draw them the same: every picture and
text file byte for byte, every report line but its times, and the
coordinate data's hash where both report it. Then each part of making a
layer, summed over the layers (learning an area's maps, the floor made
drawable, the tiles picked, the coordinate data), base against change: in
milliseconds here, or in instructions with --callgrind (Valgrind in the
build image; slower, exact, the measure for a slow CPU).

    python3 tools/layer_bench.py                       # origin/dev against this tree, seeds 1-4
    python3 tools/layer_bench.py --base HEAD~1 --seeds 1-2 --biomes 0,3,8
    python3 tools/layer_bench.py --callgrind           # instructions too (seed 1 alone by default)
    python3 tools/layer_bench.py --base-binary PATH    # a base built already

The base is built once a commit in a worktree (.build/layer-bench/src,
build.py host there) and kept as .build/layer-bench/bin/base-SHA; this
tree's is build.py host's. The ROMs are CYBERWORLD_ROM_DIR's
(~/.cache/mmbn-ref/roms), BN5's and BN4's areas included where their ROMs
are there. Exit 1 where the layers differ.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(ROOT, '.build', 'layer-bench')
IMAGE = 'cw-layer-prof'   # the build image with Valgrind (docs/devtools/profiling.md)
PARTS = ['learn', 'fix', 'pick', 'coords']
# each part's function, for its instructions (callgrind_annotate --inclusive)
FUNCTIONS = {'total': 'netmap_build_layer', 'learn': 'nm_learn', 'fix': 'legal_fix', 'pick': 'tilemap_pick',
             'coords': 'coords_write'}
TIMES = re.compile(r'built in \d+ ms|, parts learn [\d.]+ fix [\d.]+ pick [\d.]+ coords [\d.]+ ms|, coordinates [0-9a-f]{8}')


def sh(cmd, **kw):
    return subprocess.run(cmd, check=True, **kw)


def rom_dir():
    return os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))


def base_binary(ref):
    """The base's host binary, built once a commit in a worktree."""
    sha = subprocess.run(['git', '-C', ROOT, 'rev-parse', ref], check=True, capture_output=True, text=True).stdout.strip()
    binary = os.path.join(WORK, 'bin', f'base-{sha[:12]}')
    if os.path.exists(binary):
        return binary, sha
    tree = os.path.join(WORK, 'src')
    if not os.path.exists(tree):
        sh(['git', '-C', ROOT, 'worktree', 'add', '--detach', tree, sha])
    else:
        sh(['git', '-C', tree, 'checkout', '-q', '--detach', sha])
    sh([sys.executable, os.path.join(tree, 'build.py'), 'host'], stdout=subprocess.DEVNULL)
    os.makedirs(os.path.dirname(binary), exist_ok=True)
    shutil.copy2(os.path.join(tree, 'build', 'host', 'cyberworld'), binary)
    return binary, sha


def new_binary():
    sh([sys.executable, os.path.join(ROOT, 'build.py'), 'host'], stdout=subprocess.DEVNULL)
    binary = os.path.join(WORK, 'bin', 'new')
    os.makedirs(os.path.dirname(binary), exist_ok=True)
    shutil.copy2(os.path.join(ROOT, 'build', 'host', 'cyberworld'), binary)
    return binary


def atlas(binary, name, biomes, seeds):
    """Every layer of the areas drawn by `binary` into WORK/NAME; its wall-clock seconds."""
    out, data = os.path.join(WORK, name), os.path.join(WORK, name + '-data')
    for d in (out, data):
        shutil.rmtree(d, ignore_errors=True)
    os.makedirs(out)
    t0 = time.monotonic()
    sh(['nice', '-n', '10', binary, '--headless', '--rom-dir', rom_dir(), '--data-dir', data, '--atlas', f'{out}:{biomes}:{seeds}'],
       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return time.monotonic() - t0


def same(base, new):
    """The differences between two atlases (an empty list: none)."""
    diffs = []
    names = sorted(set(os.listdir(base)) | set(os.listdir(new)))
    for n in names:
        a, b = os.path.join(base, n), os.path.join(new, n)
        if n == 'learn.txt':
            continue
        if not (os.path.exists(a) and os.path.exists(b)):
            diffs.append(f'{n}: only in {"base" if os.path.exists(a) else "the change"}')
        elif n == 'report.txt':
            diffs += report_diffs(a, b)
        elif open(a, 'rb').read() != open(b, 'rb').read():
            diffs.append(f'{n}: differs')
    return diffs


def report_diffs(a, b):
    la, lb = open(a).read().splitlines(), open(b).read().splitlines()
    if len(la) != len(lb):
        return [f'report.txt: {len(la)} lines against {len(lb)}']
    out = []
    for x, y in zip(la, lb):
        if TIMES.sub('', x) != TIMES.sub('', y):
            out.append(f'report.txt: {y[:90]}...')
        hx, hy = re.search(r'coordinates ([0-9a-f]{8})', x), re.search(r'coordinates ([0-9a-f]{8})', y)
        if hx and hy and hx[1] != hy[1]:
            out.append(f'report.txt: coordinate data {hx[1]} against {hy[1]} in {y[:60]}...')
    return out


def times(out):
    """Milliseconds by part, summed over the layers (None where the build reports no parts)."""
    t = {'total': 0.0}
    for line in open(os.path.join(out, 'report.txt')):
        m = re.search(r'built in (\d+) ms', line)
        if m:
            t['total'] += int(m[1])
        p = re.search(r'parts learn ([\d.]+) fix ([\d.]+) pick ([\d.]+) coords ([\d.]+) ms', line)
        if p:
            for k, v in zip(PARTS, p.groups()):
                t[k] = t.get(k, 0.0) + float(v)
    learn = os.path.join(out, 'learn.txt')
    if os.path.exists(learn):
        t['learn'] = sum(float(m[1]) for m in re.finditer(r'learned in ([\d.]+) ms', open(learn).read()))
    return t


def valgrind_image():
    if subprocess.run(['docker', 'image', 'inspect', IMAGE], capture_output=True).returncode == 0:
        return
    d = os.path.join(WORK, 'img')
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, 'Dockerfile'), 'w') as f:
        f.write('FROM cyberworld-build\nRUN apt-get update -qq && apt-get install -y -qq valgrind >/dev/null\n')
    sh(['docker', 'build', '-q', '-t', IMAGE, d], stdout=subprocess.DEVNULL)


def instructions(binary, name, biomes, seeds):
    """Instructions by part (inclusive of what each calls) of the atlas under callgrind."""
    rel = os.path.relpath(binary, ROOT)
    out = f'/src/.build/layer-bench/{name}-cg'
    shutil.rmtree(os.path.join(WORK, name + '-cg'), ignore_errors=True)
    os.makedirs(os.path.join(WORK, name + '-cg', 'atlas'))
    script = (f'valgrind --tool=callgrind --callgrind-out-file={out}/cg.out {rel} --headless --rom-dir /rom '
              f'--data-dir {out}/data --atlas {out}/atlas:{biomes}:{seeds} > /dev/null 2>&1; '
              f'callgrind_annotate --inclusive=yes --threshold=100 {out}/cg.out')
    r = subprocess.run(['docker', 'run', '--rm', '--name', f'cw-layer-bench-{name}', '--user', f'{os.getuid()}:{os.getgid()}',
                        '-v', f'{ROOT}:/src', '-v', f'{rom_dir()}:/rom:ro', '-w', '/src', IMAGE, 'sh', '-c', script],
                       check=True, capture_output=True, text=True)
    counts = {}
    for part, fn in FUNCTIONS.items():
        m = re.search(r'^\s*([\d,]+) \([^)]*\)\s+\S*:' + fn + r' \[', r.stdout, re.M)
        counts[part] = int(m[1].replace(',', '')) if m else None
    return counts


def table(title, base, new, unit, scale=1.0):
    print(f'\n{title}')
    print(f'  {"part":8} {"base":>14} {"change":>14} {"change/base":>12}')
    for k in ['total'] + PARTS:
        b, n = base.get(k), new.get(k)
        fb = f'{b / scale:,.1f}' if b is not None else '-'
        fn = f'{n / scale:,.1f}' if n is not None else '-'
        ratio = f'{n / b:.2f}' if b and n is not None else '-'
        print(f'  {k:8} {fb:>14} {fn:>14} {ratio:>12}  {unit}')


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--base', default='origin/dev', help='the commit to compare against (default origin/dev)')
    ap.add_argument('--base-binary', help='a base host binary built already')
    ap.add_argument('--seeds', default='1-4', help="the atlas's seeds (default 1-4)")
    ap.add_argument('--biomes', default='all', help="the atlas's areas (default all)")
    ap.add_argument('--callgrind', action='store_true', help='instructions by part too (Valgrind in the build image)')
    ap.add_argument('--cg-seeds', default='1', help='the seeds under callgrind (default 1)')
    a = ap.parse_args()
    if a.base_binary:
        base, sha = os.path.abspath(a.base_binary), 'given'
    else:
        base, sha = base_binary(a.base)
    new = new_binary()
    print(f'base {sha[:12]}, change this tree; areas {a.biomes}, seeds {a.seeds}; ROMs in {rom_dir()}')
    sb = atlas(base, 'base', a.biomes, a.seeds)
    sn = atlas(new, 'new', a.biomes, a.seeds)
    diffs = same(os.path.join(WORK, 'base'), os.path.join(WORK, 'new'))
    layers = sum(1 for line in open(os.path.join(WORK, 'new', 'report.txt')) if line.startswith('biome'))
    files = len(os.listdir(os.path.join(WORK, 'new')))
    if diffs:
        print(f'DIFFERENT: {len(diffs)} of {files} files or report lines ({layers} layers)')
        for d in diffs[:20]:
            print('  ' + d)
    else:
        print(f'the same: {layers} layers, {files} files')
    print(f'atlas wall clock: base {sb:.1f} s, change {sn:.1f} s')
    table('milliseconds, summed over the layers (this machine; noisy)', times(os.path.join(WORK, 'base')),
          times(os.path.join(WORK, 'new')), 'ms')
    if a.callgrind:
        valgrind_image()
        ib = instructions(base, 'base', a.biomes, a.cg_seeds)
        inew = instructions(new, 'new', a.biomes, a.cg_seeds)
        table(f'instructions, millions (callgrind, seeds {a.cg_seeds}; exact)', ib, inew, 'M', 1e6)
    return 1 if diffs else 0


if __name__ == '__main__':
    sys.exit(main())
