#!/usr/bin/env python3
"""battle_floor.py [--seeds 1-12] [--frames N] [--out FILE]: the random
battles per layer and per act, against the battle floor's bands
(docs/game/progression/structure.md, The battle floor; build.py pacing runs
it after its rolls, docs/devtools/pacing-report.md).

The host build plays a run a seed headless, side by side, each in a fresh
data folder (the test autopilot, MegaMan kept alive and his foes deleted at
a hit: --dev god,onehit, so the walk is measured act after act), and each
layer MegaMan left has its line in the run log (`... act 2 layer left
battles 1`). A layer without a guardian (not a dark layer's board, not a
trip back) holds LAYER_BAND random battles, NONE marking one without; an
act whose guardian's layer was left holds ACT_MOST at most, its guardian's
layer's own battles among them (its least the floor of its layers, one a
layer: a dark layer's board has its own battles and no floor). Needs the ROM (local only) and build/host. Prints
how many were past their band, and exits 0."""
import argparse
import os
import re
import shutil
import subprocess
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(ROOT, '.build', 'battle_floor')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
LAYER_BAND = (1, 3)   # random battles on a layer without a guardian: the floor, and BN6's walk's most
ACT_MOST = 6          # ... in an act, its layers' together (BN5's End Area's eight in two, OVER)
LINE = re.compile(r'depth (\d+) area (\d+) act (\d+) (\S+)( dark)? left battles (\d+)( guardian)?')


def seeds_of(spec):
    out = []
    for part in spec.split(','):
        a, _, b = part.partition('-')
        out += range(int(a), int(b or a) + 1)
    return out


def play(seed, frames):
    """A run of `seed`, its run log's lines"""
    data = os.path.join(WORK, f's{seed}')
    shutil.rmtree(data, ignore_errors=True)
    os.makedirs(data)
    cmd = ['docker', 'run', '--rm', '--name', f'battle-floor-s{seed}-{os.getpid()}', '-u', f'{os.getuid()}:{os.getgid()}',
           '-v', f'{ROOT}:/src', '-w', '/src', '-v', f'{ROM_DIR}:/rom:ro', '-e', 'CYBERWORLD_AUTOPILOT=1', 'cyberworld-build',
           'build/host/cyberworld', '--headless', '--rom-dir', '/rom', '--data-dir', f'/src/{os.path.relpath(data, ROOT)}',
           '--scene', 'emu', '--seed', str(seed), '--dev', 'god,onehit', '--frames', str(frames)]
    with open(data + '.out', 'w') as f:
        subprocess.call(cmd, stdout=f, stderr=subprocess.STDOUT)
    log = os.path.join(data, 'runlog.txt')
    return open(log).read().splitlines() if os.path.exists(log) else []


def past(n, band):
    return 'NONE' if n == 0 else 'UNDER' if n < band[0] else 'OVER' if n > band[1] else ''


def report(runs):
    """The lines of the report and how many were past their band"""
    out, flagged, acts = [], 0, {}
    for seed, lines in runs:
        row, act = [], {}
        for line in lines:
            m = LINE.search(line)
            if not m:
                continue
            depth, a, kind, n = int(m.group(1)), int(m.group(3)), m.group(4) + (m.group(5) or ''), int(m.group(6))
            guardian = bool(m.group(7))
            mark = ''
            if kind in ('layer', 'undernet', 'secret') and not guardian:
                mark = past(n, LAYER_BAND)
                acts.setdefault(a, []).append(n)
            if kind != 'back' and 'dark' not in kind:
                act.setdefault(a, []).append(n)
            if guardian and kind == 'layer':
                total = sum(act.pop(a, []))
                acts.setdefault(('act', a), []).append(total)
                if total > ACT_MOST:
                    flagged += 1
                    row.append(f'act {a} {total} OVER')
            flagged += bool(mark)
            row.append(f'{depth}{"" if kind == "layer" else kind[0]}{"g" if guardian else ""}:{n}{" " + mark if mark else ""}')
        out.append(f'seed {seed:2}: ' + ' '.join(row))
    out.append('')
    out.append(f'per act (a layer without a guardian {LAYER_BAND[0]}-{LAYER_BAND[1]}, an act {ACT_MOST} at most):')
    for a in sorted(k for k in acts if isinstance(k, int)):
        v, whole = acts[a], acts.get(('act', a), [])
        line = f'  act {a}: {len(v)} layers, {sum(v)} battles, {sum(v) / len(v):.2f} a layer, {v.count(0)} without, most {max(v)}'
        if whole:
            line += f'; {len(whole)} whole acts, {min(whole)}-{max(whole)} battles each'
        out.append(line)
    return out, flagged


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--seeds', default='1-12')
    ap.add_argument('--frames', type=int, default=60000)
    ap.add_argument('--out', help='append the report to this file')
    a = ap.parse_args()
    seeds = seeds_of(a.seeds)
    with ThreadPoolExecutor(min(len(seeds), os.cpu_count() or 4)) as ex:
        runs = list(ex.map(lambda s: (s, play(s, a.frames)), seeds))
    lines, flagged = report(runs)
    text = '\n'.join(['', f'Battles per layer and act: seeds {a.seeds}, {a.frames} frames each (tools/battle_floor.py)', *lines,
                      f'{flagged} past their band', ''])
    print(text)
    if a.out:
        with open(a.out, 'a') as f:
            f.write(text)


if __name__ == '__main__':
    main()
