#!/usr/bin/env python3
"""heap3ds.py [--seeds 1-12] [--frames N] [--cases today,bn6,bn5] [--cap MB] [--no-build]: the
New 3DS's heap modelled on this computer (3ds/README.md, Testing on a 3DS;
docs/EMULATION.md, The 3DS's memory).

The host build plays long runs headless (the test autopilot, MegaMan kept
alive and his foes deleted at a hit: --dev god,onehit, so a run goes
through act after act and area after area) with tools/heap3ds.c preloaded,
which counts the game's allocations as the 3DS's heap holds them (96 MB;
the core's ROM copy kept as the 3DS keeps it) and refuses one past it, as
the 3DS's malloc does. Each case is a 3DS:

  today  BN6's ROM alone, mGBA's 16 MB ROM buffer held and every area's
         learned maps kept (the 3DS before issue #82's 3DS round): the
         host build as it is
  bn6    BN6's ROM alone, the buffer given back, the layer's own area's
         learned maps kept alone (src/map/netmap.c): the host built with
         CW_HEAP_3DS
  bn5    the same with BN5's ROM beside BN6's, read whole

Per case and seed: the heap's peak, the allocations refused (the first's
place named), a crash's place, how deep the run went and how often an area
was learned again. Needs the ROMs (local only), copied into
.build/heap3ds/rom. Exits 0 once every run is reported."""
import argparse
import hashlib
import importlib.util
import os
import re
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(ROOT, '.build', 'heap3ds')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
SHA1 = {'bn6.gba': '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6', 'bn5.gba': '5f472f78d8de2df01d5039e045c043cb40969a39'}
MB = 1 << 20
CAP = 96 * MB              # the heap (src/core/start_3ds.c: OS_HEAP_AREA's 96 MB)
BUFFER = 16 * MB           # mGBA's ROM buffer (docker/mgba.sh's 16 MB), held before main where not given back
CORE_COPY = 16 * MB        # the core's ROM copy (EMU_ROM_SIZE), which the 3DS keeps (src/emu/emu.c)
HOST = 'build/host/cyberworld'
AS_3DS = 'build/heap3ds/cyberworld'   # the host build with CW_HEAP_3DS: the 3DS's rules where they are its own
CASES = {'today': (False, BUFFER, HOST), 'bn6': (False, 0, AS_3DS), 'bn5': (True, 0, AS_3DS)}


def build_py():
    spec = importlib.util.spec_from_file_location('build', os.path.join(ROOT, 'build.py'))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def find_roms():
    """Both ROMs, told by their SHA-1, copied into WORK/rom."""
    rom = os.path.join(WORK, 'rom')
    os.makedirs(rom, exist_ok=True)
    want = {k: v for k, v in SHA1.items() if not os.path.exists(os.path.join(rom, k))}
    for name in sorted(os.listdir(ROM_DIR)) if want else []:
        path = os.path.join(ROM_DIR, name)
        if name.lower().endswith('.gba') and os.path.isfile(path):
            with open(path, 'rb') as f:
                sha = hashlib.sha1(f.read()).hexdigest()
            for k, v in list(want.items()):
                if sha == v:
                    shutil.copy(path, os.path.join(rom, k))
                    del want[k]
    if want:
        sys.exit(f'no {" or ".join(want)} in {ROM_DIR} (BN6 Gregar and BN5 Team Colonel, USA)')
    return rom


def seeds_of(spec):
    out = []
    for part in spec.split(','):
        a, _, b = part.partition('-')
        out += range(int(a), int(b or a) + 1)
    return out


def docker_run(bp, name, args, mounts=(), env=()):
    cmd = ['docker'] + (['--context', bp.CONTEXT] if bp.CONTEXT else []) + [
        'run', '--rm', '--name', name, '-u', f'{os.getuid()}:{os.getgid()}', '-v', f'{ROOT}:/src', '-w', '/src']
    for host, guest in mounts:
        cmd += ['-v', f'{host}:{guest}']
    for k, v in env:
        cmd += ['-e', f'{k}={v}']
    return subprocess.run(cmd + [bp.IMAGE, *args], capture_output=True, text=True)


def frames_after(text, marker, binary):
    """The binary's offsets in the frames said after the first `marker`"""
    if marker not in text:
        return []
    return re.findall(r'heap3ds:   at \S*' + re.escape(os.path.basename(binary)) + r'\+(0x[0-9a-f]+)', text.split(marker, 1)[1].split('heap3ds: refused', 1)[0])


def one(bp, rom, case, seed, frames, cap):
    """A long run of `seed` on the 3DS of `case`, its heap `cap` bytes"""
    bn5, fixed, binary = CASES[case]
    data = os.path.join(WORK, case, f's{seed}')
    shutil.rmtree(data, ignore_errors=True)
    os.makedirs(data)
    mounts = [(os.path.join(rom, 'bn6.gba'), '/rom/bn6.gba:ro')]
    if bn5:
        mounts.append((os.path.join(rom, 'bn5.gba'), '/rom/bn5.gba:ro'))
    env = [('LD_PRELOAD', '/src/.build/heap3ds/heap3ds.so'), ('HEAP3DS_CAP', cap), ('HEAP3DS_FIXED', fixed),
           ('HEAP3DS_KEEP', CORE_COPY), ('CYBERWORLD_AUTOPILOT', '1')]
    rel = os.path.relpath(data, ROOT)
    r = docker_run(bp, f'3ds-heap3ds-{case}-s{seed}-{os.getpid()}',
                   [binary, '--headless', '--size', '400x240', '--rom-dir', '/rom', '--data-dir', f'/src/{rel}',
                    '--scene', 'emu', '--seed', str(seed), '--dev', 'god,onehit', '--frames', str(frames)], mounts, env)
    out = r.stdout + r.stderr
    with open(data + '.out', 'w') as f:
        f.write(out)
    m = re.search(r'heap3ds: peak ([\d.]+) MB .* (\d+) refused', out)
    depth = 0
    log = os.path.join(data, 'runlog.txt')
    if os.path.exists(log):
        with open(log) as f:
            depth = max([int(d) for d in re.findall(r'depth (\d+)', f.read())] or [0])
    return {'peak': float(m.group(1)) if m else None, 'refused': int(m.group(2)) if m else None, 'depth': depth,
            'end': 'whole' if m else f'ended early ({r.returncode})', 'binary': binary,
            'first': frames_after(out, 'heap3ds: refused', binary)[:2], 'crash': frames_after(out, 'heap3ds: crashed', binary)[:3],
            'again': len(re.findall(r'netmap: area \d+ learned again', out))}


def names(bp, binary, addrs):
    """addr2line's function names for `binary`'s offsets"""
    if not addrs:
        return {}
    r = docker_run(bp, f'3ds-heap3ds-names-{os.getpid()}', ['addr2line', '-f', '-s', '-e', binary, *addrs])
    lines = r.stdout.splitlines()
    return {a: f'{lines[2 * i]} ({lines[2 * i + 1]})' for i, a in enumerate(addrs) if 2 * i + 1 < len(lines)}


def build(bp):
    """The host build, and the host built with the 3DS's rules (CW_HEAP_3DS)"""
    subprocess.run([sys.executable, os.path.join(ROOT, 'build.py'), 'host'], cwd=ROOT, check=True, capture_output=True)
    r = docker_run(bp, f'3ds-heap3ds-make-{os.getpid()}', ['make', 'TARGET=host', 'OUT=build/heap3ds', 'VERSION=heap3ds',
                                                           f'-j{os.cpu_count() or 4}', 'EXTRA_CFLAGS=-DCW_HEAP_3DS', AS_3DS])
    if r.returncode:
        sys.exit('the host build with CW_HEAP_3DS failed\n' + r.stdout[-2000:] + r.stderr[-2000:])


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--seeds', default='1-12', help='seeds, as 1-12 or 1,4,7')
    ap.add_argument('--frames', type=int, default=120000, help="each run's frames (120000: some 25 layers)")
    ap.add_argument('--cases', default='today,bn6,bn5', help='which 3DS: ' + ', '.join(CASES))
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    ap.add_argument('--no-build', action='store_true', help='the host binaries as they are')
    ap.add_argument('--as-3ds', default=AS_3DS, help="the bn6 and bn5 cases' binary (a variant built with --no-build)")
    ap.add_argument('--cap', type=int, default=CAP // MB, help="the heap's size in MB (less than 96: fragmentation's share, or a test of what a refusal does)")
    a = ap.parse_args()
    for case in ('bn6', 'bn5'):
        CASES[case] = CASES[case][:2] + (a.as_3ds,)
    rom = find_roms()
    bp = build_py()
    if not a.no_build:
        build(bp)
    os.makedirs(WORK, exist_ok=True)
    r = docker_run(bp, f'3ds-heap3ds-cc-{os.getpid()}', ['gcc', '-O2', '-shared', '-fPIC', '-o', '.build/heap3ds/heap3ds.so',
                                                         'tools/heap3ds.c', '-ldl'])
    if r.returncode:
        sys.exit('heap3ds.c did not build\n' + r.stderr)
    cases = [c for c in a.cases.split(',') if c in CASES]
    jobs = [(c, s) for c in cases for s in seeds_of(a.seeds)]
    with ThreadPoolExecutor(a.jobs) as pool:
        results = dict(zip(jobs, pool.map(lambda j: one(bp, rom, j[0], j[1], a.frames, a.cap * MB), jobs)))
    places = {}
    for binary in {res['binary'] for res in results.values()}:
        places.update({(binary, k): v for k, v in names(bp, binary, sorted({f for res in results.values() if res['binary'] == binary
                                                                           for f in res['first'] + res['crash']})).items()})
    print(f'{"case":6} {"seed":>4} {"peak MB":>8} {"of":>4} {"refused":>8} {"depth":>6} {"again":>6}  end')
    for (case, seed), res in results.items():
        name = lambda f, res=res: places.get((res['binary'], f), f)
        peak = res['peak'] if res['peak'] is not None else '-'
        refused = res['refused'] if res['refused'] is not None else '-'
        print(f'{case:6} {seed:4} {peak:>8} {a.cap:4} {refused:>8} {res["depth"]:6} {res["again"]:6}  {res["end"]}'
              + (f'; first refused in {" < ".join(name(f) for f in res["first"])}' if res['first'] else '')
              + (f'; crashed in {" < ".join(name(f) for f in res["crash"])}' if res['crash'] else ''))
    for case in cases:
        runs = [results[(case, s)] for s in seeds_of(a.seeds)]
        peaks = [x['peak'] for x in runs if x['peak'] is not None]
        print(f'{case}: peak {max(peaks) if peaks else "-"} MB at most; {sum(x["refused"] or 0 for x in runs)} allocations refused, '
              f'in {sum(1 for x in runs if x["refused"])} of {len(runs)} runs; {sum(1 for x in runs if x["crash"])} crashed; '
              f'{sum(x["again"] for x in runs)} areas learned again')
    print(f'(each run\'s output: {os.path.relpath(WORK, ROOT)}/CASE/sSEED.out)')
    return 0


if __name__ == '__main__':
    sys.exit(main())
