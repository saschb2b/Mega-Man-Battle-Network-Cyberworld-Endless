#!/usr/bin/env python3
"""virus_battler.py [report] | lab [--record N] [--team A,B] [--time-up] [--mash] [--state FILE]:
BN6 Cybeast Gregar's Virus Battler read from the player's own ROM and played
in romlab (docs/world/level-design/virus-battler.md; docs/rom/virus-battler.md, The
Virus Battler; the names in src/emu/bn6_battler.h).

report (the default) prints what the card holds (each family's HP, attack and
MB, the numbers the VIRUS LIST shows) and the five Battlers' series (each
round's record, battlefield and the pair it fields), and fails where the ROM
is not as docs/rom/ says: 33 records of mode 6 from list 0's 0x1A,
their rounds counting up, a series' last with music 0x16.

lab plays one of those records in romlab on the plain ROM, from an engine
boot state (any data folder's boot-3.state, MegaMan on an internet map): that
map's random battles all made the record, the card and every family's data
set, MegaMan walked into a battle, the team picked by the list's entries
(0 Mettaur, 13 ErthDrgn...), placed, confirmed, and the battle run; it
prints the result (BN6_BATTLE_RESULT: 1 won, 2 lost, 7 a draw at time up)
and keeps a picture every 30 frames in .build/vb/. --time-up fills the gauge
a few seconds in; --mash presses each button in turn through the fight
(the pictures the same as without: no button acts). Needs the cyberworld-romlab image and tools/romlab/romlab.bin.
"""
import argparse
import glob
import hashlib
import os
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROMS = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
OUT = os.path.join(ROOT, '.build', 'vb')
SHA1 = '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6'

# src/emu/bn6_battler.h, src/emu/bn6.h, src/core/rom.c
LIST0 = 0x080B06E0          # BN6_BATTLE_LIST0
FIRST, RECORDS = 0x1A, 33   # BN6_VB_FIRST_RECORD, BN6_VB_RECORDS
CARD_FLAGS, CARD_FAMILIES = 0x0802B2E0, 27
FAMILY_DATA = 0x0802B5FC    # BN6_VB_FAMILY_DATA
FLAG_VIRUS_DATA, FLAG_BTLRCARD = 0x196, 0x1D8
ENEMY_IDS, ENEMY_NAMES = 0x080182C4, (0x086EE400, 0x086EED00)
ENCOUNTERS = 0x08020170     # RomLayout.encounters
EVENT_FLAGS, GAMESTATE = 0x02001C88, 0x02001B80
VB_LIST, GAUGE, RESULT = 0x020364C0, 0x020352A0, 0x0200A008
BATTLERS = ('RoboDog comp, Central Town', 'the water comp, Seaside', "the punishment chair's comp, Green Town",
            "the oxygen tank's comp, Sky Town", 'Central Area 1')
SERIES = (3, 5, 5, 10, 10)
MASH = ('A', 'B', 'L', 'R', 'UP', 'DOWN', 'LEFT', 'RIGHT', 'SELECT')   # (START pauses, as in any battle)


class Rom:
    def __init__(self):
        for path in sorted(glob.glob(os.path.join(ROMS, '*.gba'))):
            with open(path, 'rb') as f:
                data = f.read()
            if hashlib.sha1(data).hexdigest() == SHA1:
                self.data, self.name = data, os.path.basename(path)
                return
        sys.exit(f'virus_battler: no BN6 Cybeast Gregar (USA) in {ROMS}')

    def u8(self, a):
        return self.data[a & 0x1FFFFFF]

    def u16(self, a):
        return struct.unpack_from('<H', self.data, a & 0x1FFFFFF)[0]

    def u32(self, a):
        return struct.unpack_from('<I', self.data, a & 0x1FFFFFF)[0]

    def text(self, archive, i):
        """an entry's letters and digits (others by their code), for the report"""
        a, s = archive + self.u16(archive + 2 * i), ''
        while self.u8(a) < 0xE5 and len(s) < 24:
            c = self.u8(a)
            s += chr(ord('A') + c - 0x0B) if 0x0B <= c <= 0x24 else chr(ord('a') + c - 0x26) if 0x26 <= c <= 0x3F else \
                chr(ord('0') + c - 0x01) if 0x01 <= c <= 0x0A else ' ' if c == 0 else f'{{{c:02x}}}'
            a += 1
        return s

    def enemy(self, i):
        return self.text(ENEMY_NAMES[i >> 8], i & 0xFF)

    def triple(self, i):
        a = ENEMY_IDS + 3 * i
        return self.u8(a), self.u8(a + 1), self.u8(a + 2)

    def record(self, n):
        return LIST0 + 16 * n


def card(rom):
    """the card's families in the VIRUS LIST's order: (AI, name, HP, attack, MB)"""
    out = []
    for i in range(CARD_FAMILIES):
        ai = rom.u32(CARD_FLAGS + 4 * i) - FLAG_VIRUS_DATA
        hp, at, mb = (rom.u16(FAMILY_DATA + 8 * ai + k) for k in (0, 2, 4))
        out.append((ai, rom.enemy(1 + 6 * (ai - 1)), hp, at, mb))
    return out


def pair(rom, n):
    """a record's enemies: (panel, id, name, version)"""
    a, out = rom.u32(rom.record(n) + 12), []
    while rom.u8(a) != 0xF0:
        if rom.u8(a) == 0x11:
            i = rom.u16(a + 2)
            out.append((rom.u8(a + 1), i, rom.enemy(i), rom.triple(i)[0]))
        a += 4
    return out


def report(rom):
    bad = []
    print('The card: the VIRUS LIST\'s families (the V1\'s numbers; a team of 50 MB at most)')
    for ai, name, hp, at, mb in card(rom):
        print(f'  AI {ai:2}  {name:<9} HP {hp:3}  AT {at:3}  {mb} MB')
    n = FIRST
    for b, (where, rounds) in enumerate(zip(BATTLERS, SERIES)):
        print(f'Battler {b + 1}, {where}: {rounds} rounds, records {n:#x}-{n + rounds - 1:#x}')
        for r in range(rounds):
            rec = rom.record(n + r)
            mode, rnd, music = rom.u8(rec + 3), rom.u8(rec + 5), rom.u8(rec + 2)
            foes = ', '.join(f'{name} V{v + 1} (column {p & 15}, row {p >> 4})' for p, _, name, v in pair(rom, n + r))
            print(f'  round {rnd:2}  field {rom.u8(rec):02x}  {foes}')
            if mode != 6 or rnd != r + 1 or (music == 0x16) != (r == rounds - 1):
                bad.append(f'record {n + r:#x}: mode {mode}, round {rnd}, music {music:#x}')
        n += rounds
    for b in bad:
        print('  not as docs/rom/ says:', b)
    return not bad


def flag_pokes(flags, have):
    """romlab poke8 lines setting these event flags over the bytes read"""
    by = bytearray(have)
    for f in flags:
        by[f >> 3] |= 0x80 >> (f & 7)
    return [f'poke8 {EVENT_FLAGS + i:08X} {by[i]:02X}' for i in sorted({f >> 3 for f in flags})]


def romlab(script, name):
    p = subprocess.run(['docker', 'run', '--rm', '-i', '--name', f'cw-vb-{name}-{os.getpid()}', '-v', f'{ROOT}:/src',
                        '-v', f'{ROMS}:/roms:ro', '-w', '/src', 'cyberworld-romlab', 'tools/romlab/romlab.bin',
                        f'/roms/{ROM.name}'], input='\n'.join(script + ['']).encode(), capture_output=True)
    if p.returncode:
        sys.exit(f'virus_battler: romlab failed: {p.stderr.decode()[-400:]}')


def lab(rom, record, team, time_up, mash, state):
    os.makedirs(OUT, exist_ok=True)
    src = os.path.relpath(os.path.abspath(state), ROOT)
    if src.startswith('..'):
        sys.exit('virus_battler: the state must lie inside the repository (Docker mounts it alone)')
    romlab([f'load /src/{src}', 'run 2', 'dump 02001B80 16 /src/.build/vb/gs.bin',
            f"dump {EVENT_FLAGS:08X} 5A8 /src/.build/vb/flags.bin"], 'read')
    gs = open(os.path.join(OUT, 'gs.bin'), 'rb').read()
    group, number = gs[4], gs[5]
    if group < 0x80:
        sys.exit(f'virus_battler: the state is on map {group:#x}/{number:#x}, not an internet map')
    rec = rom.data[rom.record(record) & 0x1FFFFFF:(rom.record(record) & 0x1FFFFFF) + 16].hex()
    s = [f'load /src/{src}']
    for table in range(4):   # every table's list for this map: each of its records made the Battler's
        internet = rom.u32(ENCOUNTERS + 8 * table + 4)
        recs = rom.u32(rom.u32(internet + 4 * (group - 0x80)) + 4 * number)
        a = recs
        while rom.u8(a) != 0xFF:
            s.append(f'pokes {a:08X} {rec}')
            a += 16
    flags = [FLAG_BTLRCARD] + [rom.u32(CARD_FLAGS + 4 * i) for i in range(CARD_FAMILIES)]
    s += flag_pokes(flags, open(os.path.join(OUT, 'flags.bin'), 'rb').read())
    for _ in range(12):
        for key in ('RIGHT', 'DOWN', 'LEFT', 'UP'):
            s += [f'keys {key}', 'run 40']
    s += ['keys', 'run 30', 'shot /src/.build/vb/lab-list.ppm']

    def press(key, wait=20):
        s.extend([f'keys {key}', 'run 3', 'keys', f'run {wait}'])
    at = 0
    for i, entry in enumerate(team):
        for _ in range(abs(entry - at)):
            press('DOWN' if entry > at else 'UP', 8)
        at = entry
        press('A', 30)
    if len(team) == 1:
        press('RIGHT')
    press('A', 60)   # OK
    press('A', 40)   # the first placed at column 2 row 2
    if len(team) == 2:
        press('UP')
        press('A', 40)   # the second, a row up
    s += ['shot /src/.build/vb/lab-placed.ppm', 'run 60']
    press('A', 3)   # Fight with this set? Yes
    for n in range(60):
        if time_up and n == 6:
            s += [f'poke8 {GAUGE:08X} F0', f'poke8 {GAUGE + 1:08X} 3F']
        if mash and 2 <= n < 18:   # a button a step through the fight: the same fight if none acts
            s += [f'keys {MASH[n % len(MASH)]}', 'run 4', 'keys', 'run 26']
        else:
            s += ['run 30']
        s += [f'shot /src/.build/vb/lab-{n:02d}.ppm']
    s += [f'dump {RESULT:08X} 16 /src/.build/vb/result.bin', f'dump {VB_LIST:08X} 32 /src/.build/vb/list.bin']
    romlab(s, 'fight')
    result = open(os.path.join(OUT, 'result.bin'), 'rb').read()[1]
    print(f'record {record:#x} on map {group:#x}/{number:#x}, team {team}: result {result} '
          f'({ {1: "won", 2: "lost", 7: "draw at time up"}.get(result, "?") }); pictures in .build/vb/lab-*.ppm')
    return result in (1, 2, 7)


def main():
    global ROM
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('what', nargs='?', default='report', choices=('report', 'lab'))
    ap.add_argument('--record', type=lambda v: int(v, 0), default=FIRST)
    ap.add_argument('--team', default='0,1', help="the list's entries picked, one or two")
    ap.add_argument('--time-up', action='store_true')
    ap.add_argument('--mash', action='store_true', help='press each button in turn through the fight')
    ap.add_argument('--state', help='an engine boot state on an internet map (default: the newest .build/**/boot-3.state)')
    a = ap.parse_args()
    ROM = Rom()
    if a.what == 'report':
        sys.exit(0 if report(ROM) else 1)
    state = a.state or max(glob.glob(os.path.join(ROOT, '.build', '**', 'boot-3.state'), recursive=True),
                           key=os.path.getmtime, default=None)
    if not state:
        sys.exit('virus_battler: no boot-3.state in .build (any build.py shot --scene emu makes one)')
    sys.exit(0 if lab(ROM, a.record, [int(t) for t in a.team.split(',')], a.time_up, a.mash, state) else 1)


ROM = None
if __name__ == '__main__':
    main()
