#!/usr/bin/env python3
"""liberation_bn5.py [--runs N] [--show] [--draw DIR]: Battle Network 5's own nine
Liberation Missions, read from the player's ROM, measured as tools/liberation.py
measures its generated ones (docs/game/liberation/).

Reads Team Colonel (USA)'s mission records (docs/rom/liberation.md, BN5's Liberation
Missions): each mission's panels and their kinds, its DarkHoles, its team, its
Darkloid and guardians, its item panels and traps, its par and its rewards;
prints each board's numbers (panels, dark share, holes, items, traps, bonus
panels, barriers, the walk from the start to the Darkloid), lets the planner
play it with its own team, and sets the planner's phases beside BN5's par: the
check that the planner's par is BN5's. Numbers only, nothing kept; needs the
ROM (local only), told by its SHA-1 among CYBERWORLD_ROM_DIR's files.
"""
import argparse
import collections
import hashlib
import os
import statistics
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import liberation as L  # noqa: E402

ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
BN5_SHA1 = '5f472f78d8de2df01d5039e045c043cb40969a39'
MISSIONS = 0x04F628       # 9 records of 0x5C: map, offsets, par, Order Points, 20 pointers
MISSION_SIZE = 0x5C
NAVI_HP = 0x04F968        # per team Navi: its id, then his HP in each mission
# A panel's kind in BN5's lists (docs/game/liberation/bn5/board.md) and ours.
KINDS = {0x00: L.FREE, 0x01: L.DARK, 0x03: L.ITEM, 0x05: L.BAR1, 0x06: L.BAR2, 0x07: L.BAR3,
         0x09: L.BONUS, 0x0A: L.HOLE, 0x0B: L.RING, 0x0C: L.START}
NAVIS = {0x00: 'MegaMan', 0x07: 'Colonel', 0x08: 'ShadowMan', 0x09: 'NumberMan', 0x0A: 'TomahawkMan',
         0x0B: 'KnightMan', 0x0C: 'ToadMan'}
GUARDS = {0x11: 'brute', 0x12: 'hawk', 0x13: 'bladia'}
ITEMS = {0: 'heart', 1: 'key', 2: 'op', 3: 'zenny', 4: 'bugfrag', 5: 'chip'}
ACT_OF = {1: 2, 2: 3, 3: 3, 4: 4, 5: 4, 6: 5, 7: 5, 8: 6, 9: 6}


def find_rom():
    for name in sorted(os.listdir(ROM_DIR)):
        path = os.path.join(ROM_DIR, name)
        if name.lower().endswith('.gba') and os.path.isfile(path):
            with open(path, 'rb') as f:
                data = f.read()
            if hashlib.sha1(data).hexdigest() == BN5_SHA1:
                return data
    sys.exit('BN5 Team Colonel (USA) not found in %s' % ROM_DIR)


def read(rom, n):
    """Mission n (1-9) as a board of tools/liberation.py's, with BN5's own numbers beside it."""
    o = MISSIONS + (n - 1) * MISSION_SIZE
    u32 = lambda a: struct.unpack_from('<I', rom, a & 0x1FFFFFF)[0]
    u16 = lambda a: struct.unpack_from('<H', rom, a & 0x1FFFFFF)[0]
    ptr = [u32(o + 0x0C + 4 * i) for i in range(20)]
    b = L.Board(ACT_OF[n], n)
    p = ptr[1] & 0x1FFFFFF
    while rom[p] != 0xFF:
        b.cells[(rom[p + 2], rom[p + 3])] = KINDS[rom[p]]
        p += 4
    b.start = [c for c, k in b.cells.items() if k == L.START]
    p = ptr[2] & 0x1FFFFFF
    b.holes = [(rom[p + 2 * i], rom[p + 2 * i + 1]) for i in range(4) if rom[p + 2 * i]]
    b.holes += [c for c, k in b.cells.items() if k == L.HOLE and c not in b.holes]
    units = []
    p = ptr[4] & 0x1FFFFFF
    while rom[p + 5] != 0xFF:
        units.append(rom[p:p + 20]); p += 20
    boss = units[0]
    b.boss = (boss[8], boss[9])
    b.cells[b.boss] = L.BOSS
    for u in units[1:]:
        hole = u[0] if u[0] != 0xFF else None
        b.guards.append(dict(kind=GUARDS.get(u[5], 'brute'), pos=(u[8], u[9]), hole=hole if hole is not None and hole < len(b.holes) else None))
    p = ptr[10] & 0x1FFFFFF
    while u32(0x08000000 + p) != 0xFFFFFFFF:
        kind, amount = rom[p], u16(0x08000000 + p + 4)
        b.items[(rom[p + 6], rom[p + 7])] = (ITEMS[kind], amount)
        p += 8
    traps = []
    p = ptr[9] & 0x1FFFFFF
    while u32(0x08000000 + p) != 0xFFFFFFFF:
        traps.append((rom[p], rom[p + 1], rom[p + 2], rom[p + 3]))
        b.items[(rom[p + 2], rom[p + 3])] = ('trap', rom[p + 1])
        p += 4
    team = []
    p = ptr[3] & 0x1FFFFFF
    while rom[p + 4] != 0 or rom[p + 5] != 0xFF:
        if rom[p + 2:p + 4] != b'\xff\xff': break
        team.append(NAVIS.get(rom[p + 5], 'MegaMan')); p += 20
    rewards = []
    p = ptr[19] & 0x1FFFFFF
    while rom[p] != 0xFF:
        rewards.append((rom[p], rom[p + 1], rom[p + 2], rom[p + 3], u16(0x08000000 + p + 4))); p += 8
    rec = rom[o:o + 12]
    b.par = rec[10]
    if not b.start:
        # No start panels: the team's own places (world units, 32 a panel: 0x08051BA8's sum backwards).
        p = ptr[3] & 0x1FFFFFF
        for _ in team:
            wx, wy = struct.unpack_from('<hh', rom, p + 8)
            c = ((wx - 0x10 + struct.unpack_from('<b', rom, o + 6)[0]) // 32 + 0x80,
                 (wy - 0x10 + struct.unpack_from('<b', rom, o + 7)[0]) // 32 + 0x80)
            if c in b.cells: b.start.append(c)
            p += 20
    info = dict(map='%02X:%02X' % (rec[0], rec[1]), par=rec[10], op=rec[11], field9=rec[9], team=team,
                boss=units[0][5], traps=traps, rewards=rewards, bonus=sum(1 for k in b.cells.values() if k == L.BONUS))
    b.platform = set()
    return b, info


def walk(b):
    d = L.graph_dist(b, b.start, lambda k: True)
    return d.get(b.boss, -1), max(d.values())


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--runs', type=int, default=24)
    ap.add_argument('--show', action='store_true')
    ap.add_argument('--draw', default='')
    a = ap.parse_args()
    rom = find_rom()
    print('mission map   par  OP panels fields fill dense dark holes items traps bonus bars walk team                                  planner p40/p60')
    ratios = []
    for n in range(1, 10):
        b, info = read(rom, n)
        kinds = collections.Counter(b.cells.values())
        dark = (kinds[L.DARK] + kinds[L.ITEM]) / len(b.cells)
        to_boss, far = walk(b)
        res = L.play(b, info['team'], a.runs, n)
        ph = sorted(p for p, _ in res if p)
        p40 = ph[int(len(ph) * 0.4)] if ph else 0
        p60 = ph[int(len(ph) * 0.6)] if ph else 0
        if ph: ratios.append((ACT_OF[n], info['par'] / ph[len(ph) // 2]))
        f, fill = L.fields(b.cells)
        print('%7d %s %4d %3d %6d %5.0f%% %3.0f%% %5.1f %3.0f%% %5d %5d %5d %5d %4d %4d %-38s %s/%s (%d of %d won)' % (
            n, info['map'], info['par'], info['op'], len(b.cells), 100 * f, 100 * fill, L.density(b), 100 * dark, len(b.holes),
            sum(1 for i in b.items.values() if i[0] != 'trap'), len(info['traps']), info['bonus'],
            kinds[L.BAR1] + kinds[L.BAR2] + kinds[L.BAR3], to_boss, '+'.join(info['team']), p40, p60, len(ph), len(res)))
        if a.show:
            print(L.text(b.cells, b.guards))
            print('   rewards (kind, code, phases from, to, chip or zenny):', info['rewards'])
        if a.draw:
            os.makedirs(a.draw, exist_ok=True)
            L.draw(b.cells, os.path.join(a.draw, 'bn5_mission%d.png' % n), b.guards,
                   title='BN5 mission %d  par %d  %s' % (n, info['par'], '+'.join(info['team'])))
    by_act = collections.defaultdict(list)
    for act, r in ratios: by_act[act].append(r)
    print('BN5\'s par over the planner\'s median, per act of the missions it stands for: %s (tools/liberation.py PAR_SCALE %s)' % (
        ', '.join('%d: %.2f' % (a, statistics.mean(v)) for a, v in sorted(by_act.items())), L.PAR_SCALE))
    return 0


if __name__ == '__main__':
    sys.exit(main())
