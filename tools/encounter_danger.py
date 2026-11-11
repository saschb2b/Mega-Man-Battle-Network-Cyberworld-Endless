#!/usr/bin/env python3
"""encounter_danger.py families|battles FILE [--jobs N] [--seeds 3,5] [--out FILE]:
what a battle costs MegaMan, measured in BN6's engine (the encounter groups'
measuring, docs/game/progression/encounter-groups.md; docs/devtools/encounter-tags.md).

Each battle is fought twice, headless, MegaMan at 2000 HP so none ends him
(`--dev quiet,hp=2000`, the `foes` step, CYBERWORLD_BATTLE_PROBE's lines):

  idle   MegaMan stands at his panel and does nothing for some 20 seconds: the HP
         he loses in 20 (the battle's pressure on a standing
         target), the panels of his side that changed type, those the enemy
         took, and whether more enemies came than the battle set (a summon).
  fight  the test autopilot fights it with the starting folder: the HP it
         loses, whether it wins inside 50 seconds, and how long that takes.

`families` fights every family of tools/encounter_data/tags.tsv alone at V1
(its tricks and its pressure); `battles FILE` the battles of a file of lines
"label<TAB>ID@CR/ID@CR..." (enemy ids in hex at column and row, as the foes
step takes them). Rows are appended to --out (tab separated, numbers and
our own labels alone). Needs the ROMs (local only) and the host build
(`build.py host`); one battle is one run of about ten seconds."""
import argparse
import concurrent.futures
import os
import re
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(ROOT, '.build', 'encounter')
DATA = os.path.join(ROOT, 'tools', 'encounter_data')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
PROBE = re.compile(r'^probe t(\d+) phase ([0-9a-f]+) hp (-?\d+) foes (\d+) foehp (\d+) panels (\S+)')
IDLE_FRAMES, FIGHT_FRAMES = 1300, 3000
COLUMNS = ['label', 'mode', 'spec', 'seed', 'frames', 'won', 'hp_lost', 'pressure20', 'own_panels', 'foe_panels', 'taken', 'foes', 'foes_most']


def run_one(label, spec, mode, seed):
    """One battle of `spec`, idle or fought: its row (a dict), None where the battle never began."""
    name = re.sub(r'[^A-Za-z0-9]+', '_', f'{label}_{mode}_{seed}')
    data = os.path.join(WORK, 'runs', name)
    shutil.rmtree(data, ignore_errors=True)
    os.makedirs(data)
    env = dict(os.environ, CYBERWORLD_BATTLE_PROBE='1', CYBERWORLD_ROM_DIR=ROM_DIR, CYBERWORLD_DOCKER_NAME='encounter')
    if mode == 'idle':
        # (the arrival's words paged, the battle, its Custom screen left with no chip, then nothing)
        script = '200:,' + '4:A,26:,' * 40 + f'0:foes {spec},260:,4:START,10:,4:A,{IDLE_FRAMES + 200}:'
        frames = 200 + 1200 + 274 + IDLE_FRAMES + 200
    else:
        env['CYBERWORLD_AUTOPILOT'] = '1'
        script = f'400:,0:foes {spec},{FIGHT_FRAMES + 400}:'
        frames = 400 + FIGHT_FRAMES + 400
    cmd = ['nice', '-n', '10', sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--scene', 'emu', '--run-depth', '2', '--seed', str(seed),
           '--dev', 'quiet,hp=2000', '--data-dir', '/src/' + os.path.relpath(data, ROOT), '--frames', str(frames), '--input', script]
    r = subprocess.run(cmd, cwd=ROOT, env=env, capture_output=True, text=True)
    shutil.rmtree(data, ignore_errors=True)
    lines = [PROBE.match(x) for x in (r.stdout + r.stderr).splitlines()]
    probes = [(int(m[1]), int(m[2], 16), int(m[3]), int(m[4]), int(m[5]), m[6]) for m in lines if m]
    fight = [p for p in probes if p[1] == 0x0C and p[2] >= 0]
    if not fight:
        return None
    first, start_hp, start_foes, start_panels = fight[0][0], fight[0][2], fight[0][3], fight[0][5]
    limit = IDLE_FRAMES if mode == 'idle' else FIGHT_FRAMES
    fight = [p for p in fight if p[0] - first <= limit]
    won = next((p for p in fight if p[3] == 0), None)
    last = won or fight[-1]
    at20 = [p for p in fight if p[0] - first <= 1200][-1]
    own, foe, taken = set(), set(), set()
    for p in fight:
        for k, (a, b) in enumerate(zip(start_panels, p[5])):
            if a.lower() != b.lower():
                (own if k % 6 < 3 else foe).add(b.lower())
            if k % 6 < 3 and b.isupper():
                taken.add(k)
    return {'label': label, 'mode': mode, 'spec': spec, 'seed': seed, 'frames': last[0] - first, 'won': int(won is not None),
            'hp_lost': start_hp - last[2], 'pressure20': start_hp - at20[2], 'own_panels': ''.join(sorted(own)) or '-',
            'foe_panels': ''.join(sorted(foe)) or '-', 'taken': len(taken), 'foes': start_foes, 'foes_most': max(p[3] for p in fight)}


def family_battles():
    """Every family of the tags' dump alone at V1, in the enemy's middle panel."""
    out = []
    with open(os.path.join(DATA, 'tags.tsv')) as f:
        for line in f:
            c = line.rstrip('\n').split('\t')
            if c[0] == 'ver' and c[3] == '0':
                out.append((f'{c[1]}:{c[2]}', f'{int(c[4]):X}@52'))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('what', choices=['families', 'battles'])
    ap.add_argument('file', nargs='?', help='battles: lines of label<TAB>spec')
    ap.add_argument('--jobs', type=int, default=2, help='runs side by side')
    ap.add_argument('--seeds', default='3', help='the fights\' seeds, comma separated (the idle run takes the first)')
    ap.add_argument('--modes', default='idle,fight')
    ap.add_argument('--out', default=os.path.join(WORK, 'danger.tsv'))
    a = ap.parse_args()
    if a.what == 'families':
        battles = family_battles()
    else:
        with open(a.file) as f:
            battles = [tuple(x.rstrip('\n').split('\t')[:2]) for x in f if x.strip() and not x.startswith('#')]
    seeds = [int(s) for s in a.seeds.split(',')]
    jobs = []
    for label, spec in battles:
        if 'idle' in a.modes:
            jobs.append((label, spec, 'idle', seeds[0]))
        if 'fight' in a.modes:
            jobs += [(label, spec, 'fight', s) for s in seeds]
    os.makedirs(os.path.dirname(a.out), exist_ok=True)
    fresh = not os.path.exists(a.out)
    missed = 0
    with open(a.out, 'a') as out, concurrent.futures.ThreadPoolExecutor(a.jobs) as pool:
        if fresh:
            out.write('\t'.join(COLUMNS) + '\n')
        # (a run whose battle never began, the machine busy: once more)
        for job, row in zip(jobs, pool.map(lambda j: run_one(*j) or run_one(*j), jobs)):
            if row is None:
                missed += 1
                print('no battle:', *job, file=sys.stderr)
                continue
            out.write('\t'.join(str(row[c]) for c in COLUMNS) + '\n')
            out.flush()
    print(f'{len(jobs) - missed} battles measured into {os.path.relpath(a.out, ROOT)}' + (f', {missed} never began' if missed else ''))
    return 1 if missed else 0


if __name__ == '__main__':
    sys.exit(main())
