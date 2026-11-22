#!/usr/bin/env python3
"""library_pages.py [--check] [--report]: the Library's pages of the other entries, src/core/library_pages_data.c, made
from the three ROMs and the chip registry (docs/game/meta/library.md).

A page is its game's own Library: its standard chips, Megas and Gigas in the game's order, each with the run's chip
that fills its slot. The order is the record's sort key (+0x1C, u16), one run of numbers over the classes in BN5 and
BN4 (1 Cannon to 180 ColorPt, then the Megas, then the Gigas). A chip's number is its place in its class among the
chips of the version read (Team Colonel, Red Sun): the other version's Megas (bit 0 of the record's +0x16: its Navis'
chips) and Gigas (its version mark there, bit 2 the pair's first version and bit 3 its second: docs/rom/chips.md, The
Gigas' version marks) are left out of the count, and those of them a run has a chip for follow the class's own,
numbered on. The run's chip is the registry's for BN5's (BN6's own where both games name it), and for BN4's the
chip the engine pays for it (src/core/xchips.c's pair_bn4, mirrored here on the names in BN6's codes: BN6's own of its
name, else the registry's own id of BN5's of its name, else by its Navi's V1 and version), else its own row of the
registry (src/core/xchips_registry_bn4.c, where the tree has it). A chip no run has an id for has no slot.

Numbers only: the game's chip id, the run's, its number and class; the names are comments.

--check: exit 1 where the file is not what the ROMs give. --report: each page's slots by class and by where the run's
chip comes from. Local only (CYBERWORLD_ROM_DIR, ~/.cache/mmbn-ref/roms)."""
import argparse
import collections
import hashlib
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
OUT = os.path.join(ROOT, 'src', 'core', 'library_pages_data.c')
SHA1 = {'bn6': '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6', 'bn5': '5f472f78d8de2df01d5039e045c043cb40969a39',
        'bn4': 'a97e96a7da03abd70f7953328e511b9fa29179f1'}
# (ROM offsets, as src/core/rom.c's layout and src/emu/bn5.h, bn4.h name them)
NAMES = {'bn6': (0x6E88D0, 0x6E92D8), 'bn5': (0x736084, 0x736AB8), 'bn4': (0x7481D8, 0x748C14)}
RECORDS = {'bn6': 0x21DA8, 'bn5': 0x1E210, 'bn4': 0x197EC}
COUNT = {'bn6': 314, 'bn5': 424, 'bn4': 389}
REC = 0x2C
CLASS = {'bn5': 7, 'bn4': 8}   # a record's class: BN6's and BN5's +7, BN4's +8
FAMILY, SUB, PARAMS, SORT = 0x0B, 0x0C, 0x10, 0x1C
SP, DS = 3, 4          # a Navi chip's version, its record's first parameter
SP_MARK = 0x43         # BN6's SP mark in its names
# each class's keys in the game's order: (first, last), its numbers from 1
KEYS = {'bn5': ((1, 180), (181, 261), (262, 274)), 'bn4': ((1, 186), (201, 280), (281, 290))}
OTHERS_FLAG, FLAGS = 0x01, 0x16                     # a Mega of the other version (its Navis' chips), the record's +0x16
OTHERS_GIGAS = {'bn5': 0x04, 'bn4': 0x08}           # ... a Giga's: the first version's mark in Team Colonel, the second's in Red Sun
CLASSES = ('StdChip', 'MegaChip', 'GigaChip')


def roms():
    found = {}
    for name in sorted(os.listdir(ROM_DIR)) if os.path.isdir(ROM_DIR) else []:
        path = os.path.join(ROM_DIR, name)
        if name.lower().endswith('.gba') and os.path.getsize(path) == 0x800000:
            with open(path, 'rb') as f:
                data = f.read()
            for k, v in SHA1.items():
                if hashlib.sha1(data).hexdigest() == v:
                    found[k] = data
    missing = [k for k in SHA1 if k not in found]
    if missing:
        sys.exit(f'library_pages: no {", ".join(missing).upper()} ROM in {ROM_DIR}')
    return found


def bn5_code(c):
    """BN5's text code as BN6's (src/core/rom.c's bn5_code): -1 none, -2 the DS mark"""
    if c < 0x40:
        return c
    if c <= 0x92 or 0x94 <= c <= 0x9A:
        return c + 5
    if 0xA0 <= c <= 0xAD and c != 0xA5:
        return c + 3
    return {0x93: 0x98, 0x9C: 0x98, 0x9D: 0xA2, 0x9E: 0x43, 0x9F: -2, 0x9B: 0xA0, 0xA5: 0xA0, 0xB1: 0xA0, 0xB4: 0xA0,
            0xAE: -1, 0xAF: -1, 0xB0: -1, 0xB5: -1, 0xB6: -1, 0xB7: -1}.get(c, c)


def bn4_code(c):
    """BN4's (its punctuation, 0x40-0x57, is BN6's 0x58 on: rom.c's bn4_code)"""
    return c + 0x58 if 0x40 <= c <= 0x57 else c


CONV = {'bn6': lambda c: c, 'bn5': bn5_code, 'bn4': bn4_code}


def name(rom, game, i):
    """chip i's name in BN6's codes, 19 at most (XCHIP_NAME less its end)"""
    arch = NAMES[game][i >> 8]
    p, out = arch + struct.unpack_from('<H', rom, arch + 2 * (i & 0xFF))[0], []
    while rom[p] < 0xE5 and len(out) < 19:
        c = CONV[game](rom[p])
        out += [0x0E, 0x1D] if c == -2 else [c] if c >= 0 else []
        p += 1
    return bytes(out[:19])


def spell(b):
    return ''.join(chr(65 + c - 0x0B) if 0x0B <= c <= 0x24 else chr(97 + c - 0x26) if 0x26 <= c <= 0x3F else
                   chr(48 + c - 1) if 1 <= c <= 10 else {0: ' ', 0x98: '-', 0x9E: '+', 0xA6: '.', 0x43: 'SP'}.get(c, '?') for c in b)


def registry(path, pattern):
    """a registry file's rows: the game's id -> (its id in BN6's tables, how); {} where the tree has no such file"""
    if not os.path.exists(path):
        return {}
    with open(path, encoding='utf-8') as f:
        return {int(m.group(1)): (int(m.group(2)), m.group(3)) for m in re.finditer(pattern, f.read())}


def placed(game, rec):
    """a record's (class, key, the other version's) where it is one of its game's Library's three classes, else None"""
    cls, key = rec[CLASS[game]], struct.unpack_from('<H', rec, SORT)[0]
    if cls > 2 or all(c == 0xFF for c in rec[0:4]):
        return None
    first, last = KEYS[game][cls]
    if not first <= key <= last:
        return None
    return cls, key, (cls == 1 and rec[FLAGS] & OTHERS_FLAG != 0) or (cls == 2 and rec[FLAGS] & OTHERS_GIGAS[game] != 0)


def numbers(game, recs):
    """each chip's (class, number): its place in its class among the version's own, the other version's after them"""
    out = {}
    for cls in range(3):
        mine = sorted((p[1], x) for x, p in recs.items() if p[0] == cls and not p[2])
        theirs = sorted((p[1], x) for x, p in recs.items() if p[0] == cls and p[2])
        for n, (_, x) in enumerate(mine + theirs):
            out[x] = (cls, n + 1)
    return out


def pages(r):
    rec = lambda g, i: r[g][RECORDS[g] + REC * i:RECORDS[g] + REC * (i + 1)]   # noqa: E731
    names = {g: {i: name(r[g], g, i) for i in range(1, COUNT[g])} for g in SHA1}
    reg5 = registry(os.path.join(ROOT, 'src', 'core', 'xchips_registry.c'), r'\{\s*(\d+),\s*(\d+),\s*XREG_(\w+)')
    reg4 = registry(os.path.join(ROOT, 'src', 'core', 'xchips_registry_bn4.c'), r'\{\s*(\d+),\s*(\d+),\s*XREG_(\w+)')
    own5 = {v[0] for v in reg5.values() if v[1] not in ('SAME', 'ALIAS')}
    out = {'bn5': [], 'bn4': []}
    left = {'bn5': [], 'bn4': []}
    slot = {}
    for g in out:
        recs = {x: placed(g, rec(g, x)) for x in range(1, COUNT[g]) if names[g][x]}
        slot[g] = numbers(g, {x: p for x, p in recs.items() if p})
    # BN5's: the registry's chip, no DarkChip and no Program Advance
    for x in range(1, COUNT['bn5']):
        s = slot['bn5'].get(x)
        if not s:
            continue
        if x in reg5 and reg5[x][1] not in ('DARK', 'PA'):
            out['bn5'].append((s[0], s[1], x, reg5[x][0], 'bn6' if reg5[x][1] in ('SAME', 'ALIAS') else 'own', spell(names['bn5'][x])))
        else:
            left['bn5'].append((s[0], s[1], spell(names['bn5'][x])))
    # BN4's: the chip a run pays for it (pair_bn4's three rules), else its own row
    paid = {}
    for x in range(1, COUNT['bn4']):
        rx = rec('bn4', x)
        if not names['bn4'][x] or rx[CLASS['bn4']] > 2:
            continue
        by6 = next((i for i in range(1, COUNT['bn6']) if i not in own5 and names['bn6'][i] == names['bn4'][x]), 0)
        by5 = next((j for j in range(1, COUNT['bn5']) if names['bn5'][j] == names['bn4'][x] and j in reg5 and
                    reg5[j][1] not in ('SAME', 'ALIAS', 'PA')), 0)
        if by6:
            paid[x] = (by6, 'bn6')
        elif by5:
            paid[x] = (reg5[by5][0], 'bn5')
    # (one chip under two spellings, the registry's xreg4_alias: BN6's)
    with open(os.path.join(ROOT, 'src', 'core', 'xchips_registry_bn4.c')) as f:
        alias = re.search(r'xreg4_alias\[\]\[2\] = \{(.*?)\};', f.read(), re.S)
    for a, b in re.findall(r'\{\s*(\d+),\s*(\d+)\s*\}', alias.group(1) if alias else ''):
        paid[int(a)] = (int(b), 'bn6')
    for x in range(1, COUNT['bn4']):
        rx = rec('bn4', x)
        if x in paid or not names['bn4'][x] or rx[CLASS['bn4']] != 1 or rx[PARAMS] not in (SP, DS):
            continue
        v = x - (rx[PARAMS] - SP + 1)
        rv = rec('bn4', v)
        if v not in paid or rv[CLASS['bn4']] != 1 or rv[PARAMS] != 0 or rv[FAMILY:SUB + 1] != rx[FAMILY:SUB + 1]:
            continue
        v5 = next((j for j in reg5 if reg5[j][0] == paid[v][0] and reg5[j][1] not in ('SAME', 'ALIAS')), 0)
        if v5:
            j = v5 + rx[PARAMS] - SP + 1
            if j in reg5 and reg5[j][1] not in ('SAME', 'ALIAS') and rec('bn5', j)[PARAMS] == rx[PARAMS]:
                paid[x] = (reg5[j][0], 'bn5')
        elif rx[PARAMS] == SP:
            ry = rec('bn6', paid[v][0])
            i = next((i for i in range(1, COUNT['bn6']) if rec('bn6', i)[FAMILY:SUB + 1] == ry[FAMILY:SUB + 1] and
                      names['bn6'][i][-1:] == bytes([SP_MARK])), 0)
            if i:
                paid[x] = (i, 'bn6')
    for x in range(1, COUNT['bn4']):
        s = slot['bn4'].get(x)
        if not s:
            continue
        hit = paid.get(x) or ((reg4[x][0], 'own') if x in reg4 and reg4[x][1] != 'XREG_PA' and reg4[x][1] != 'PA' else None)
        if hit:
            out['bn4'].append((s[0], s[1], x, hit[0], hit[1], spell(names['bn4'][x])))
        else:
            left['bn4'].append((s[0], s[1], spell(names['bn4'][x])))
    for g in out:
        out[g].sort()
    return out, left


HEAD = '''/* The Library's pages of the other entries (library_pages.h; docs/game/meta/library.md): each game's standard chips,
 * Megas and Gigas a run has a chip for, in the game's own Library order, with the run's chip that fills the slot.
 * A table written by tools/library_pages.py from the three ROMs and the chip registry: numbers only (the game's chip id, the
 * run's, its number in its class, its class); the names are comments. */
#include "library_pages.h"

'''


def text(out):
    L = [HEAD]
    for g in ('bn5', 'bn4'):
        L.append(f'const LibSlot library_{g}_rows[] = {{\n')
        for cls, number, x, chip, _, nm in out[g]:
            L.append(f'\t{{ {x:3}, {chip:4}, {number:3}, {cls} }},   /* {nm} */\n')
        L.append(f'}};\nconst int library_{g}_count = (int)(sizeof library_{g}_rows / sizeof *library_{g}_rows);\n\n')
    return ''.join(L).rstrip('\n') + '\n'


def report(out, left):
    for g in ('bn5', 'bn4'):
        print(f'{g.upper()}: {len(out[g])} slots')
        for cls in range(3):
            by = collections.Counter(r[4] for r in out[g] if r[0] == cls)
            gone = [f'{n} {nm}' for c, n, nm in left[g] if c == cls]
            print(f'  {CLASSES[cls]}: {sum(by.values())} of {KEYS[g][cls][1] - KEYS[g][cls][0] + 1} numbers'
                  f' (BN6\'s by name {by["bn6"]}, BN5\'s own {by["bn5"] if g == "bn4" else by["own"]}'
                  + (f', its own {by["own"]}' if g == 'bn4' else '') + f'); no chip of the run: {len(gone)}')
            if gone:
                print('    ' + ', '.join(gone))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--check', action='store_true')
    ap.add_argument('--report', action='store_true')
    a = ap.parse_args()
    out, left = pages(roms())
    t = text(out)
    if a.report:
        report(out, left)
        return
    if a.check:
        with open(OUT, encoding='utf-8') as f:
            if f.read() != t:
                sys.exit('library_pages: src/core/library_pages_data.c is not what the ROMs give (python3 tools/library_pages.py)')
        print('library_pages: the file is what the ROMs give')
        return
    with open(OUT, 'w', encoding='utf-8') as f:
        f.write(t)
    print('library_pages: ' + ', '.join(f'{g.upper()} {len(out[g])} slots' for g in ('bn5', 'bn4')))


if __name__ == '__main__':
    main()
