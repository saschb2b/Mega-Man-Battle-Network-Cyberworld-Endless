#!/usr/bin/env python3
"""ram_survey.py [--canary ADDR:LEN[,...] | --watch ADDR:LEN[,...]] [--min N] [--frames N] [--keep]: which of the GBA's
work RAM BN6 leaves alone, measured over a battery of headless autopilot runs (docs/devtools/ram-survey.md).

Each run marks every byte of EWRAM and IWRAM that a frame ended with not zero (CYBERWORLD_RAM_SURVEY, src/emu/
ram_survey.c); the maps are merged and the stretches that stayed zero in every run printed, longest first, each with
the symbols of bn6f's ewram.s inside it and the last one before it where a checkout is at hand (~/.cache/mmbn-ref/bn6f:
a stretch with a symbol inside is a buffer the runs did not use, not free RAM). A stretch that stays zero is a lead:
a write of zeros does not show. --canary fills the stretches named with a pattern instead and tells every byte of them a
frame changed (CYBERWORLD_RAM_CANARY), which a write of zeros does change; the power-on clear's is expected, at the
boot's frames 14 and 15. --watch changes nothing and tells every read and write the game's code makes of the stretches
named (CYBERWORLD_RAM_WATCH: the work RAM's, or the cart's save RAM's from 0x0E000000), each place once, with the
instruction's address and the access's size, and the runs' sums.

The battery: the autopilot's seeds 1-16, every area of BN6's and BN5's at one seed, deep runs, the town and home,
BN5's guardians, and the dark layer, the tournament, the Glitch layer, visitors and a route (--dev). It reaches no
NetBattle, no shop bought from, no PET screen: what it finds is checked there by hand before it is claimed.

Local only: the host build (build/host/cyberworld; build.py or tools/cw.py build host first) and the ROMs
(CYBERWORLD_ROM_DIR, ~/.cache/mmbn-ref/roms). Nothing of them is written: addresses and counts.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN = os.path.join(ROOT, 'build', 'host', 'cyberworld')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
BN6F = os.path.expanduser('~/.cache/mmbn-ref/bn6f/ewram.s')
WORK = os.path.join(ROOT, '.build', 'ram-survey')
EWRAM, EWRAM_SIZE, IWRAM, IWRAM_SIZE = 0x02000000, 0x40000, 0x03000000, 0x8000


def battery(frames):
    runs = [('s%d' % s, ['--scene', 'emu', '--seed', str(s)], frames) for s in range(1, 17)]
    runs += [('b%s' % b, ['--scene', 'emu', '--seed', '3', '--net-biome', b], frames * 3 // 5)
             for b in ['0', '1', '2', '3', '4', '5', '6', '7', 'x0', 'x1', 'x2', 'x3', 'x4', 'x5']]
    runs += [('d%d' % d, ['--scene', 'emu', '--seed', str(d), '--run-depth', str(d)], frames * 3 // 4) for d in (2, 4, 6, 8, 10, 12, 15)]
    runs += [('t%d' % s, ['--scene', 'town', '--seed', str(s)], frames * 3 // 5) for s in (1, 2, 3, 4)]
    runs += [('h%d' % s, ['--scene', 'home', '--run-depth', '7', '--seed', str(s)], frames * 3 // 5) for s in (3, 5)]
    runs += [('g%d' % g, ['--scene', 'emu', '--seed', '2', '--guardian', str(g)], frames * 3 // 5) for g in range(24, 30)]
    runs += [('v%s' % v, ['--scene', 'emu', '--seed', '4', '--dev', v], frames * 3 // 4) for v in ('dark', 'tourney', 'glitch', 'visitors', 'route')]
    return runs


def run(job, canary, watch=None):
    name, args, frames = job
    env = dict(os.environ, CYBERWORLD_AUTOPILOT='1')
    if watch:
        env['CYBERWORLD_RAM_WATCH'] = watch
    elif canary:
        env['CYBERWORLD_RAM_CANARY'] = canary
    else:
        env['CYBERWORLD_RAM_SURVEY'] = os.path.join(WORK, 'm-%s.bin' % name)
    out = subprocess.run([BIN, '--headless', '--rom-dir', ROM_DIR, '--data-dir', os.path.join(WORK, 'd-' + name)] + args + ['--frames', str(frames)],
                         env=env, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
    return name, [line for line in out.stderr.splitlines() if line.startswith('ram canary') or line.startswith('ram watch')]


def watched(done, watch, runs):
    """the runs' watch lines as one: each place (a stretch, read or written, the access's size, the instruction) with
    the runs that met it and the bytes it touched, then each stretch's sums"""
    places, sums = {}, {}
    for name, lines in done:
        for line in lines:
            m = re.match(r'ram watch: (\w+\+\w+) (read|written) (\w+), (\d+) bytes?, at (\w+) lr (\w+) frame (\d+)', line)
            if m:
                key = (m.group(1), m.group(2), int(m.group(4)), m.group(5))
                p = places.setdefault(key, {'runs': set(), 'lo': m.group(3), 'frame': int(m.group(7)), 'lr': m.group(6)})
                p['runs'].add(name)
                continue
            m = re.match(r'ram watch: (\w+\+\w+) in all: (\d+) reads(?: \((\w+)-(\w+)\))?, (\d+) writes(?: \((\w+)-(\w+)\))?, (\d+) wider', line)
            if m:
                t = sums.setdefault(m.group(1), {'reads': 0, 'writes': 0, 'wide': 0, 'r': [], 'w': [], 'runs': 0})
                t['reads'] += int(m.group(2)); t['writes'] += int(m.group(5)); t['wide'] += int(m.group(8)); t['runs'] += 1
                if m.group(3):
                    t['r'] += [int(m.group(3), 16), int(m.group(4), 16)]
                if m.group(6):
                    t['w'] += [int(m.group(6), 16), int(m.group(7), 16)]
    print('%d runs under a watch of %s' % (runs, watch))
    for stretch, t in sorted(sums.items()):
        span = lambda v: ' (%08x-%08x)' % (min(v), max(v)) if v else ''
        print('  %s: %d reads%s, %d writes%s, %d wider than a byte, over %d runs' % (
            stretch, t['reads'], span(t['r']), t['writes'], span(t['w']), t['wide'], t['runs']))
    for (stretch, how, size, pc), p in sorted(places.items()):
        print('  %s %s by %d byte%s at %s (lr %s; first %s, frame %d): %d runs' % (
            stretch, how, size, '' if size == 1 else 's', pc, p['lr'], p['lo'], p['frame'], len(p['runs'])))
    if not places:
        print('  no read and no write of them')


def symbols():
    if not os.path.isfile(BN6F):
        return []
    with open(BN6F) as f:
        return sorted((int(m.group(2), 16), m.group(1)) for m in re.finditer(r'^(\w+)::\s*//\s*0x([0-9a-fA-F]+)', f.read(), re.M))


def stretches(seen, base, lo, hi, least):
    out, i = [], lo
    while i < hi:
        if seen[i]:
            i += 1
            continue
        j = i
        while j < hi and not seen[j]:
            j += 1
        if j - i >= least:
            out.append((j - i, base + i - lo, base + j - lo))
        i = j
    return sorted(out, reverse=True)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--canary', help='ADDR:LEN[,ADDR:LEN...]: these stretches filled with a pattern, every changed byte told')
    ap.add_argument('--watch', help='ADDR:LEN[,ADDR:LEN...]: every read and write of these stretches told, nothing changed')
    ap.add_argument('--min', type=lambda v: int(v, 0), default=0x300, help='the shortest stretch printed (0x300)')
    ap.add_argument('--frames', type=int, default=20000, help="a seed's frames (20000); the other runs a share of them")
    ap.add_argument('--keep', action='store_true', help="keep the runs' data folders and maps in .build/ram-survey")
    a = ap.parse_args()
    if not os.path.isfile(BIN):
        sys.exit('no host build: python3 tools/cw.py build host')
    shutil.rmtree(WORK, ignore_errors=True)
    os.makedirs(WORK)
    jobs = battery(a.frames)
    with ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        done = list(pool.map(lambda j: run(j, a.canary, a.watch), jobs))
    if a.watch:
        watched(done, a.watch, len(jobs))
    elif a.canary:
        told = {}
        for name, lines in done:
            for line in lines:
                told.setdefault(re.sub(r' over \d+ frames', ' in the run', line), []).append(name)
        print('%d runs with a canary over %s' % (len(jobs), a.canary))
        for line in sorted(told):
            print('  %s (%d runs)' % (line, len(told[line])))
    else:
        seen = bytearray(EWRAM_SIZE + IWRAM_SIZE)
        for name, _ in done:
            path = os.path.join(WORK, 'm-%s.bin' % name)
            if not os.path.isfile(path):
                sys.exit('%s wrote no map' % name)
            with open(path, 'rb') as f:
                for i, b in enumerate(f.read()):
                    if b:
                        seen[i] = 1
        syms = symbols()
        print('%d runs: %d bytes of EWRAM and %d of IWRAM never held anything at a frame\'s end' % (
            len(jobs), EWRAM_SIZE - sum(seen[:EWRAM_SIZE]), IWRAM_SIZE - sum(seen[EWRAM_SIZE:])))
        for n, lo, hi in stretches(seen, EWRAM, 0, EWRAM_SIZE, a.min) + stretches(seen, IWRAM, EWRAM_SIZE, EWRAM_SIZE + IWRAM_SIZE, a.min):
            inside = [s for s in syms if lo <= s[0] < hi]
            before = [s for s in syms if s[0] < lo]
            print('%08x-%08x %6d  %3d of bn6f\'s symbols inside%s' % (
                lo, hi, n, len(inside), ', after %s (%08x)' % (before[-1][1], before[-1][0]) if before and lo < IWRAM else ''))
    if not a.keep:
        shutil.rmtree(WORK, ignore_errors=True)


if __name__ == '__main__':
    main()
