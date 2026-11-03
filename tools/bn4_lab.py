#!/usr/bin/env python3
"""bn4_lab.py [report] | boot | battle (--record GROUP:INDEX:K | --enemies ID@COL,ROW ...):
Battle Network 4: Red Sun (USA) read from the player's own ROM and played in
romlab: what docs/rom/bn4.md maps of it, checked (the names in src/emu/bn4.h).

report (the default) prints what its tables hold (the chip records, the enemy
records, each net group's battle lists and the enemies they field) and fails
where the ROM is not as docs/rom/bn4.md says.
boot plays the plain ROM from power-on to Lan's room, where MegaMan's PET
answers, by a fixed script of presses (the same frames every time), and keeps
the state in .build/bn4lab/room.state.
battle starts a battle from that state: the encounter check (BN4_ENCOUNTER_CHECK)
patched to return a record, one of the ROM's (--record: the net group, the
map index the roll reads and the record's place in its list) or one written
in the ROM's free space with the enemies given (--enemies: an enemy id at a
panel each, kind 1), and keeps its first frames in .build/bn4lab/ (the
Custom screen at 260, the field after it). Needs the cyberworld-romlab image
and tools/romlab/romlab.bin.

  python3 tools/bn4_lab.py
  python3 tools/bn4_lab.py boot
  python3 tools/bn4_lab.py battle --record 84:0:0
  python3 tools/bn4_lab.py battle --enemies 7@4,1 13@5,2
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
OUT = os.path.join(ROOT, '.build', 'bn4lab')
SHA1 = 'a97e96a7da03abd70f7953328e511b9fa29179f1'
# src/emu/bn4.h
TOOLKIT, TOOLKIT_GAMESTATE = 0x02009E70, 0x40
ENCOUNTER_CHECK, BATTLE_TABLES, RECORD_SIZE, ENTITY_SIZE = 0x080F5370, 0x08017FB8, 12, 4
ENEMY_STATS, ENEMY_STAT_SIZE, ENEMY_COUNT = 0x08011888, 12, 253
CHIP_RECORDS, CHIP_COUNT, CHIP_NAMES, ENEMY_NAMES = 0x080197EC, 389, (0x087481D8, 0x08748C14), 0x0874A780
NET_MAPS, NET_GROUPS = 0x0802831C, 21   # XRomLayout.map_table: the net's groups 0x80-0x94 (the map loader's, 0x0802615A)
FREE = 0x087F8000   # past the ROM's data (it ends at 0x087F739C): a record written for --enemies


class Rom:
    def __init__(self):
        for path in sorted(glob.glob(os.path.join(ROMS, '*.gba'))):
            with open(path, 'rb') as f:
                data = f.read()
            if hashlib.sha1(data).hexdigest() == SHA1:
                self.data, self.name = data, os.path.basename(path)
                return
        sys.exit(f'bn4_lab: no BN4 Red Sun (USA) in {ROMS}')

    def u8(self, a):
        return self.data[a & 0x1FFFFFF]

    def u16(self, a):
        return struct.unpack_from('<H', self.data, a & 0x1FFFFFF)[0]

    def u32(self, a):
        return struct.unpack_from('<I', self.data, a & 0x1FFFFFF)[0]

    def text(self, archive, i):
        """an entry's letters and digits (BN6's codes; others by their code)"""
        a, s = archive + self.u16(archive + 2 * i), ''
        while self.u8(a) < 0xE5 and len(s) < 24:
            c = self.u8(a)
            s += chr(ord('A') + c - 0x0B) if 0x0B <= c <= 0x24 else chr(ord('a') + c - 0x26) if 0x26 <= c <= 0x3F else \
                chr(ord('0') + c - 0x01) if 0x01 <= c <= 0x0A else ' ' if c == 0 else f'{{{c:02x}}}'
            a += 1
        return s

    def records(self, group, index):
        """the records of a net group's map index, as the roll reads them"""
        lst = self.u32(self.u32(BATTLE_TABLES + 4) + 4 * (group - 0x80))
        a, out = self.u32(lst + 4 * index), []
        while self.u8(a) != 0xFF and len(out) < 64:
            out.append(a)
            a += RECORD_SIZE
        return out

    def entities(self, record):
        a, out = self.u32(record + 8), []
        while self.u8(a) != 0xFF and len(out) < 8:
            out.append(tuple(self.data[(a & 0x1FFFFFF) + k] for k in range(ENTITY_SIZE)))
            a += ENTITY_SIZE
        return out


def literal(rom, at):
    """the word a Thumb `ldr rN, [pc, #imm]` at `at` loads"""
    return rom.u32(((at + 4) & ~3) + rom.u8(at) * 4)


def report(rom):
    bad = []

    def expect(what, got, want):
        if got != want:
            bad.append(f'{what}: {got!r}, not {want!r}')
    cannon = CHIP_RECORDS + 0x2C
    expect('Cannon (chip 1) codes', rom.data[cannon & 0x1FFFFFF:(cannon & 0x1FFFFFF) + 4].hex(), '0001021a')
    expect('Cannon power (+0x1A)', rom.u16(cannon + 0x1A), 40)
    expect('chip 1 name', rom.text(CHIP_NAMES[0], 1), 'Cannon')
    expect('enemy 1', (rom.text(ENEMY_NAMES, 1), rom.u16(ENEMY_STATS + ENEMY_STAT_SIZE) & 0xFFF), ('Mettaur', 40))
    expect('the roll\'s literal', literal(rom, 0x080F5460), BATTLE_TABLES)   # ldr r3,=BN4_BATTLE_TABLES
    expect('eToolkit\'s setup', literal(rom, 0x080060D8), TOOLKIT)          # ldr r0,=eToolkit; mov r10,r0
    viruses = sum(1 for i in range(1, 127) if rom.u8(ENEMY_STATS + i * ENEMY_STAT_SIZE + 3) == 4)
    navis = sum(1 for i in range(128, ENEMY_COUNT) if rom.u8(ENEMY_STATS + i * ENEMY_STAT_SIZE + 3) == 8)
    named = sum(1 for i in range(CHIP_COUNT) if rom.text(CHIP_NAMES[i >= 256], i % 256 if i < 256 else i - 256).strip())
    print(f'{CHIP_COUNT} chip records, {named} named; {viruses} virus and {navis} Navi enemy records')
    for g in range(0x80, 0x80 + NET_GROUPS):
        if not rom.u32(NET_MAPS + 4 * (g - 0x80)):
            continue
        lst = rom.u32(rom.u32(BATTLE_TABLES + 4) + 4 * (g - 0x80))
        indices = 0
        while indices < 64 and 0x08000000 <= rom.u32(lst + 4 * indices) < 0x08800000:
            indices += 1
        fielded = set()
        for k in range(indices):
            for r in rom.records(g, k):
                for e in rom.entities(r):
                    if e[3] == 1:
                        fielded.add(e[0])
        families = sorted({rom.u8(ENEMY_STATS + i * ENEMY_STAT_SIZE + 4) for i in fielded if i < 128})
        navis = sorted({rom.text(ENEMY_NAMES, i & ~3) for i in fielded if i >= 128})
        print(f'  group {g:#x}: {indices} map indices, {len(fielded)} enemies: {len(families)} families'
              + (f', Navis {", ".join(navis)}' if navis else ''))
    for b in bad:
        print(f'  not as docs/rom/bn4.md says: {b}')
    return 1 if bad else 0


def romlab(rom, script, name):
    os.makedirs(OUT, exist_ok=True)
    p = subprocess.run(['docker', 'run', '--rm', '-i', '--name', f'cw-bn4lab-{name}-{os.getpid()}', '-v', f'{ROOT}:/src',
                        '-v', f'{ROMS}:/roms:ro', '-w', '/src', 'cyberworld-romlab', 'tools/romlab/romlab.bin',
                        f'/roms/{rom.name}'], input='\n'.join(script + ['']).encode(), capture_output=True)
    if p.returncode:
        sys.exit(f'bn4_lab: romlab failed: {p.stderr.decode()[-400:]}')
    return p.stdout.decode()


def boot(rom):
    """power-on to Lan's room: the title, NEW GAME, the opening's text paged"""
    s = ['run 700', 'tap START', 'run 120', 'tap A', 'run 200']
    s += ['tap A', 'run 20'] * 60 + ['tap A', 'run 10'] * 480
    s += ['save /src/.build/bn4lab/room.state', 'run 2', 'shot /src/.build/bn4lab/room.ppm', f'dump {TOOLKIT:08x} 80 /src/.build/bn4lab/toolkit.bin']
    romlab(rom, s, 'boot')
    gs = struct.unpack_from('<I', open(os.path.join(OUT, 'toolkit.bin'), 'rb').read(), TOOLKIT_GAMESTATE)[0]
    out = romlab(rom, ['load /src/.build/bn4lab/room.state', 'run 2', f'peek32 {gs + 4:08x}'], 'map')
    where = int(out.split('=')[-1].split()[0], 16)
    print(f'game state {gs:#010x}, map {where & 0xFF:#x}:{where >> 8 & 0xFF:#x}; .build/bn4lab/room.state and room.ppm')
    return 0


def battle(rom, record, enemies):
    state = os.path.join(OUT, 'room.state')
    if not os.path.exists(state):
        boot(rom)
    s = ['load /src/.build/bn4lab/room.state', f'pokes {ENCOUNTER_CHECK:08x} 00487047']
    if record:
        g, i, k = (int(x, 16 if n == 0 else 10) for n, x in enumerate(record.split(':')))
        recs = rom.records(g, i)
        if k >= len(recs):
            sys.exit(f'bn4_lab: group {g:#x} index {i} has {len(recs)} records')
        at = recs[k]
    else:
        placed = [tuple(int(x) for x in e.replace('@', ',').split(',')) for e in enemies]
        ents = '00020200' + ''.join(f'{i:02x}{c:02x}{r:02x}01' for i, c, r in placed) + 'ff'
        # (a record as ACDC Area's: weight 0x20, every progress, its entities 0x10 on)
        s += [f'pokes {FREE:08x} 2000141400ff1f00' + struct.pack('<I', FREE + 0x10).hex(), f'pokes {FREE + 0x10:08x} {ents}']
        at = FREE
    s += [f'poke32 {ENCOUNTER_CHECK + 4:08x} {at:08x}', 'run 260', 'shot /src/.build/bn4lab/battle_260.ppm',
          'tap A', 'run 120', 'shot /src/.build/bn4lab/battle_390.ppm']
    romlab(rom, s, 'battle')
    for e in (rom.entities(at) if record else [(i, c, r, 1) for i, c, r in placed]):
        stat = ENEMY_STATS + e[0] * ENEMY_STAT_SIZE
        print(f'  {rom.text(ENEMY_NAMES, e[0]) if e[3] == 1 else "MegaMan" if e[3] == 0 else f"kind {e[3]}"} '
              f'at {e[1]},{e[2]}' + (f': HP {rom.u16(stat) & 0xFFF}, element {rom.u16(stat) >> 12}' if e[3] == 1 else ''))
    print('.build/bn4lab/battle_260.ppm (the Custom screen) and battle_390.ppm')
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('what', nargs='?', default='report', choices=('report', 'boot', 'battle'))
    ap.add_argument('--record', help='GROUP:INDEX:K, the net group in hex')
    ap.add_argument('--enemies', nargs='+', help='ID@COL,ROW each (decimal)')
    a = ap.parse_args()
    rom = Rom()
    if a.what == 'battle' and not (a.record or a.enemies):
        sys.exit('bn4_lab: battle wants --record or --enemies')
    return {'report': lambda: report(rom), 'boot': lambda: boot(rom),
            'battle': lambda: battle(rom, a.record, a.enemies)}[a.what]()


if __name__ == '__main__':
    sys.exit(main())
