#!/usr/bin/env python3
"""xrecords.py [--all] [--no-build]: BN5's battle records in BN6's own engine
(issue #82; docs/MULTIROM.md, One net; docs/DEVTOOLS.md).

Every random record of the maps BN5's areas take their battles from
(NetAreaDef.xbattles, src/core/rom.c), as BN5 has it and as BN6's engine
fights it (src/core/xbattles.c): its foes by BN6's ids (a family both games
have by name is BN6's, its version by tier; a family moved into the engine
its installed ids), its field by BN6's number (src/emu/xfields.c), its
green Mystery Data and rocks; or why it cannot be fought yet (a family BN6
lacks, a field with panels BN6 has none of, a Navi). Per area how many are
fightable, whether all are (native), and which families still hold how
many back. The host build's --xrecords (src/dev/xrecords_report.c) writes
the listing as a core starts with the families installed; this prints the
areas' lines (--all: every record's). Needs both ROMs (local only), copied
into .build/xrecords/rom. Exits 0 once listed."""
import argparse
import hashlib
import os
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(ROOT, '.build', 'xrecords')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
SHA1 = {'bn6.gba': '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6', 'bn5.gba': '5f472f78d8de2df01d5039e045c043cb40969a39'}


def find_roms():
    """Both ROMs, told by their SHA-1, copied into WORK/rom (nothing else there)."""
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


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--all', action='store_true', help="every record's line, not only the areas'")
    ap.add_argument('--no-build', action='store_true', help='the host binary as it is')
    a = ap.parse_args()
    rom = find_roms()
    if not a.no_build:
        subprocess.run([sys.executable, os.path.join(ROOT, 'build.py'), 'host'], cwd=ROOT, check=True, capture_output=True)
    os.makedirs(os.path.join(WORK, 'data'), exist_ok=True)
    listing = os.path.join(WORK, 'list.txt')
    if os.path.exists(listing):
        os.remove(listing)
    cmd = [sys.executable, os.path.join(ROOT, 'build.py'), 'shot', '--data-dir', '/src/.build/xrecords/data', '--xrecords',
           '/src/.build/xrecords/list.txt']
    r = subprocess.run(cmd, cwd=ROOT, env=dict(os.environ, CYBERWORLD_ROM_DIR=rom), capture_output=True, text=True)
    for line in (r.stdout + r.stderr).splitlines():
        if line.startswith(('xrecords', 'xvirus', 'xfields')):
            print(line)
    if r.returncode or not os.path.exists(listing):
        sys.exit('xrecords: the run failed\n' + r.stderr[-2000:])
    with open(listing) as f:
        lines = f.read().splitlines()
    for line in lines:
        if a.all or not line.startswith('    '):
            print(line)
    print(f'(the whole listing: {os.path.relpath(listing, ROOT)})')
    return 0


if __name__ == '__main__':
    sys.exit(main())
