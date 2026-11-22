#!/usr/bin/env python3
"""party_guardian.py [--seeds 1-6] [--acts 1-3] [--fighters 1-4] [--hp-per P]
[--steps S] [--jobs 4] [--frames N]: a party's guardian against going alone
(issue #754; docs/devtools/party-guardian.md). For each act's guardian
(layers 3, 6 and 9) and each count of fighters, the host build runs a seed
headless: the autopilot walks to his door, and his fight goes to the
party's core (--dev partycore=N, one the solo on the same field), every
MegaMan the same bot (partybot=1: charged shots, a chip now and then), the
guardian scaled for the party as a party's door scales him (partyscale=1),
or by another rule to measure (--hp-per: his HP P percent of his own more a
fighter past the first; --steps: his version S on). From the field's log
lines: who won, the frames to the end, each fighter's HP lost (of his
max) and deletions, the guardian's version and HP. Needs the ROM (local
only) and build/host. Changes nothing in play."""
import argparse
import os
import re
import shutil
import statistics
import subprocess
import tempfile
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN = os.environ.get('CYBERWORLD_BIN', os.path.join(ROOT, 'build', 'host', 'cyberworld'))
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
LAYERS_AN_ACT = 3

TAKEN = re.compile(r"the guardian's fight taken over, (\d) MegaMen, my seat \d, his HP (\d+) \(his own (\d+)\)")
GUARDIAN = re.compile(r'guardian \d+: enemy id \S+, version (\d+), HP (\d+) fought at (\d+)')
IN = re.compile(r"party: iteration \d+: side 0's actor (\d) in \(name \S+, HP (\d+)\)")
HP = re.compile(r"party: iteration \d+: side 0's actor (\d) HP (\d+) to (\d+)")
OVER = re.compile(r'party: frame (\d+): the battle over, (won|lost)')


def seeds_of(spec):
    out = []
    for part in spec.split(','):
        a, _, b = part.partition('-')
        out += range(int(a), int(b or a) + 1)
    return out


def fight(act, n, seed, a):
    """One guardian's fight: a dict of what the log said, or None where the field never took it"""
    dev = f'quiet,partycore={n},partybot=1,partyscale=1'
    if a.hp_per is not None:
        dev += f',partyhpper={a.hp_per}'
    if a.steps is not None:
        dev += f',partysteps={a.steps}'
    data = tempfile.mkdtemp(prefix=f'pg-a{act}-n{n}-s{seed}-', dir=os.path.join(ROOT, '.build'))
    env = dict(os.environ, CYBERWORLD_AUTOPILOT='1', CYBERWORLD_EMU_DEBUG='1', SDL_AUDIODRIVER='dummy')
    cmd = [BIN, '--headless', '--rom-dir', ROM_DIR, '--data-dir', data, '--scene', 'emu', '--run-depth', str(act * LAYERS_AN_ACT),
           '--seed', str(seed), '--dev', dev, '--frames', str(a.frames)]
    out = subprocess.run(cmd, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, errors="replace").stdout
    shutil.rmtree(data, ignore_errors=True)
    r = {'act': act, 'n': n, 'seed': seed}
    m = TAKEN.search(out)
    if not m:
        return None
    r['hp'], r['own'] = int(m[2]), int(m[3])
    g = GUARDIAN.search(out)
    r['version'] = int(g[1]) if g else -1
    start, last = {}, {}
    for k, hp in IN.findall(out):
        start.setdefault(int(k), int(hp))
        last.setdefault(int(k), int(hp))
    for k, _, to in HP.findall(out):
        last[int(k)] = int(to)
    o = OVER.search(out)
    r['frames'] = int(o[1]) if o else None
    r['lost'] = sum(start[k] - last[k] for k in start)
    r['fighters'] = len(start)
    return r


def table(runs, acts, fighters):
    """Per act and party: the guardian measured, and what a fighter pays against the solo's fight (1.0)"""
    lines = ['act  fighters  guardian       HP a head  his damage/s  a head/s   time to win  HP a fighter loses',
             '                (version HP)                (measured)   (measured)  (x solo)     (x solo)']
    for act in acts:
        solo = None
        for n in fighters:
            rs = [r for r in runs if r and r['act'] == act and r['n'] == n and r['frames'] and r['lost']]
            if not rs:
                lines.append(f'{act:>3}  {n:>8}  (no fight measured)')
                continue
            hp = statistics.median(r['hp'] for r in rs)
            dps = statistics.median(r['lost'] / (r['frames'] / 60) for r in rs)
            vers = '/'.join(map(str, sorted({r['version'] for r in rs})))
            head = hp / n
            # (a fighter's time: his share of the HP at the same damage a second; his HP lost: the hits he draws over it)
            if n == 1 or solo is None:
                solo = solo or (head, dps)
            time = head / solo[0]
            cost = (dps / n) * head / (solo[1] * solo[0])
            lines.append(f'{act:>3}  {n:>8}  v{vers:<4} {int(hp):>6}  {head:>9.0f}  {dps:>12.1f}  {dps / n:>9.1f}  {time:>11.2f}  {cost:>18.2f}')
    return lines


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--seeds', default='1-6')
    ap.add_argument('--acts', default='1-3')
    ap.add_argument('--fighters', default='1-4')
    ap.add_argument('--hp-per', type=int, default=None, help="his HP P percent of his own more a fighter past the first")
    ap.add_argument('--steps', type=int, default=None, help='his version S on for any party')
    ap.add_argument('--jobs', type=int, default=4)
    ap.add_argument('--frames', type=int, default=6000)
    a = ap.parse_args()
    if not os.path.exists(BIN):
        raise SystemExit(f'{BIN}: build it first (python3 tools/cw.py build host)')
    acts, fighters, seeds = seeds_of(a.acts), seeds_of(a.fighters), seeds_of(a.seeds)
    jobs = [(act, n, s) for act in acts for n in fighters for s in seeds]
    with ThreadPoolExecutor(a.jobs) as ex:
        runs = list(ex.map(lambda j: fight(*j, a), jobs))
    rule = 'the party rule as built' if a.hp_per is None and a.steps is None else f'HP +{a.hp_per}% a fighter, versions +{a.steps}'
    print(f'party_guardian: {len(seeds)} seeds, {rule}')
    for line in table(runs, acts, fighters):
        print(line)
    missed = sum(1 for r in runs if r is None or not r['frames'])
    if missed:
        print(f'{missed} runs not measured (the guardian not reached, or his fight not over, in {a.frames} frames)')


if __name__ == '__main__':
    main()
