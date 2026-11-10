#!/usr/bin/env python3
"""xreg_bn4.py [--check]: BN4 Red Sun's rows of the chip registry, src/core/xchips_registry_bn4.c, made from the three
ROMs (docs/multirom/bn4-chips.md, The build plan's unit 2).

A BN4 chip takes a row where the run has no chip it is: none of BN6's own of its name, none of the registry's of BN5's
of its name, and for an SP or a DS none by its Navi's V1 (the engine's rules, src/core/xchips.c's pair_bn4, mirrored
here on the names in BN6's codes). BugCharg has none (it reads the NaviCust's bugs: the owner's rule for BN5's), and
ProtoMan's DS none (BN6's ProtoMan has no DS chip; it pays the stand-in). Numbers only: BN4's id, its id in BN6's
tables, its codes, class and MB, which a placeholder keeps where BN4's ROM is not read; the names are comments.

The ids, fixed here for good (a saved folder names them): 527-602, which have a pack entry today (docs/rom/chips-code.md,
The pack past 511), go to the chips that play first, in the plan's order (FIRST below: the six on BN6's code and Roll's
two, the Navi chips of the seven other moved Navis, the units under 300 bytes, then the plan's next units while they
fit whole); 605-623 to its nineteen Program Advances (BN6's bit for one formed is the engine's from 603, 32 of them);
635 on to the rest, in BN4's order.

--check: exit 1 where the file is not what the ROMs give. Local only (CYBERWORLD_ROM_DIR, ~/.cache/mmbn-ref/roms)."""
import argparse
import hashlib
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
OUT = os.path.join(ROOT, 'src', 'core', 'xchips_registry_bn4.c')
SHA1 = {'bn6': '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6', 'bn5': '5f472f78d8de2df01d5039e045c043cb40969a39',
        'bn4': 'a97e96a7da03abd70f7953328e511b9fa29179f1'}
# (ROM offsets, as src/core/rom.c's layout and src/emu/bn5.h, bn4.h name them)
NAMES = {'bn6': (0x6E88D0, 0x6E92D8), 'bn5': (0x736084, 0x736AB8), 'bn4': (0x7481D8, 0x748C14)}
RECORDS = {'bn6': 0x21DA8, 'bn5': 0x1E210, 'bn4': 0x197EC}
COUNT = {'bn6': 314, 'bn5': 424, 'bn4': 389}
REC = 0x2C
BN4_MB, BN4_CLASS, FAMILY, SUB, PARAMS = 6, 8, 0x0B, 0x0C, 0x10
SP, DS = 3, 4          # a Navi chip's version, its record's first parameter
OUT_OF = {310: 'BugCharg'}   # (no row: the owner's rule)
# One chip under two spellings (the owner's call, 10 November 2026: docs/multirom/bn4-chips.md, The owner's calls): BN4's id ->
# BN6's chip, the run's; it takes no row, and where FIRST gave it an id that id is retired (ids are fixed)
ALIAS = {201: 182}   # LifeAura: BN6's LifeAur; id 530 retired
# A dark chip of BN4's (the owner's call, 10 November 2026: docs/game/meta/darkchips.md): its record's alignment dark (BN4 plays it only
# for a dark MegaMan, src/emu/bn4_chips.h BN4_CHIP_ALIGN) or a DarkChip by its effect flags (src/emu/bn4.h BN4_CHIP_DARK). A row of
# one is listed in xreg4_dark, which the price's hook reads: a battle it is used in costs 20 max HP, as every dark chip's
ALIGN, ALIGN_DARK, FLAGS, FLAG_DARK = 0x15, 2, 9, 0x20
# BN4's chips that run on BN6's own code (docs/multirom/bn4-chips.md, Their code: BN4's family and subfamily a routine BN6 has
# by its body), each proved by its chip_* spec against BN4's own battle before it is listed: BN4's id -> (the BN6 chip whose
# family, lock-on and lock-on type its record takes, the subfamily or None for the twin's)
ON_BN6 = {
    133: (177, None),   # Invis: BN4's support 0x31, BN6's subfamily 1 (Invisibl's), the same frames in its parameters (chip_invis)
}
# BN4's chips whose code is moved into BN6's engine as a unit (tools/xchiprecipe_bn4.py UNITS, src/emu/xchips_bn4.c), each
# proved by its bn4_chip_* spec against BN4's own battle before it is listed: BN4's id -> the unit's number in BN6's grown
# support routines (XC_SUPPORT: 0x80 + BN4's subfamily), which its record takes under BN6's support family, or for a chip
# of BN4's misc family (its record's +0xB 0x20) in BN6's grown misc routines (XC4_MISC), under BN6's misc family; the generator
# refuses to write a unit whose chips are not here
MOVED = {
    178: 0xA6, 179: 0xA6, 180: 0xA6,   # PanlSht1-3: BN4's support 0x26 (bn4_chip_panlsht1-3)
    210: 0x99,                         # ShotStar: BN4's support 0x19 (bn4_chip_shotstar)
    115: 0x8B,                         # Repair: BN4's misc 0x0B, BN6's grown misc routine 0x8B (bn4_chip_repair)
    60: 0x80, 61: 0x80, 62: 0x80,      # Counter1-3: BN4's misc 0, BN6's grown misc routine 0x80 (bn4_chip_counter1-3)
    72: 0x88, 73: 0x89, 74: 0x8A,      # WhitWeb1-3: BN4's misc 8, 9 and 0xA, one unit's three routines (bn4_chip_whitweb1-3)
    75: 0x82, 76: 0x82, 77: 0x82,      # Boomer1-3: BN4's misc 2 (bn4_chip_boomer1-3)
    130: 0x81, 131: 0x81, 132: 0x81,   # MokoRus1-3: BN4's misc 1 (bn4_chip_mokorus1-3)
}
# BN4's Navi chips whose Navi is moved into BN6's engine as a unit of kind 'navi' (tools/xchiprecipe_bn4_navi.py: the chip's own
# first-kind Navi, run by BN6's Navi object), each proved by its bn4_chip_* spec: BN4's id -> the unit's number in BN6's grown
# Navi routines (XC_NAVI: 0x80 + BN4's subfamily), its record's subfamily under BN6's Navi family, which the row takes from
# NAVI_TWIN (XREG_MOVED with a twin: the twin's family alone)
NAVI = {
    224: 0x85,                         # GutsMan: BN4's support 5, his chip's Navi the first kind's 0x14 (bn4_chip_gutsman)
}
NAVI_TWIN = 221                        # (BN6's Roll, its first Navi chip: family 0x1B)
MOVED.update(NAVI)
PACK_FIRST, PACK_END, PA_FIRST, REST_FIRST = 527, 603, 605, 635

# BN4's ids in the order they take 527-602 (the plan's order of play: docs/multirom/bn4-chips.md, Their code)
FIRST = (
    82, 216, 97, 201, 308, 44, 222, 223,                                            # on BN6's code; RollSP, RollDS
    224, 225, 226, 236, 237, 238, 227, 228, 229, 233, 234, 235, 230, 231, 232,      # GutsMan, ThunMan, WindMan, FireMan, SerchMan,
    251, 252, 253, 254, 255, 256,                                                   # AquaMan, WoodMan: the moved Navis'
    281, 115, 133, 178, 179, 180, 193, 285, 286, 287, 288, 60, 61, 62, 210,         # the units under 300 bytes
    78, 79, 80, 72, 73, 74, 130, 131, 132, 38, 39, 40, 47, 75, 76, 77, 211,         # SidBmbo, WhitWeb, MokoRus, Binder, SandRing, Boomer, BugChain
    19, 20, 21, 22, 23, 24, 192, 12, 13, 14, 160, 161, 162, 190, 289,               # the spread chips, FlmLine, SeekBom, DrkVulcn, BlakWeap
)


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
        sys.exit(f'xreg_bn4: no {", ".join(missing).upper()} ROM in {ROM_DIR}')
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
                   chr(48 + c - 1) if 1 <= c <= 10 else {0: ' ', 0x98: '-', 0x9E: '+', 0xA6: '.'}.get(c, '?') for c in b)


def registry_own():
    """the registry's rows of BN5's: BN5's id -> (its id in BN6's tables, how)"""
    with open(os.path.join(ROOT, 'src', 'core', 'xchips_registry.c'), encoding='utf-8') as f:
        return {int(m.group(1)): (int(m.group(2)), m.group(3)) for m in re.finditer(r'\{\s*(\d+),\s*(\d+),\s*XREG_(\w+)', f.read())}


def rows(r):
    rec = lambda g, i: r[g][RECORDS[g] + REC * i:RECORDS[g] + REC * (i + 1)]   # noqa: E731
    names = {g: {i: name(r[g], g, i) for i in range(1, COUNT[g])} for g in SHA1}
    reg = registry_own()
    own = {v[0] for v in reg.values() if v[1] not in ('SAME', 'ALIAS')}
    paid, new, pas = {}, [], []
    for x in range(1, COUNT['bn4']):
        rx = rec('bn4', x)
        if not names['bn4'][x] or all(c == 0xFF for c in rx[0:4]):
            continue
        by6 = next((i for i in range(1, 350) if i not in own and (i < COUNT['bn6'] or i >= 320) and
                    name(r['bn6'], 'bn6', i) == names['bn4'][x]), 0)
        by5 = next((j for j in range(1, COUNT['bn5']) if names['bn5'][j] == names['bn4'][x] and j in reg and
                    reg[j][1] not in ('SAME', 'ALIAS')), 0)
        if rx[BN4_CLASS] == 4:
            # (a Program Advance BN6 or the registry has by name is the run's)
            if not by6 and not by5:
                pas.append(x)
            continue
        if rx[BN4_CLASS] > 2:
            continue
        if x in ALIAS:
            # (held to the two names being one cut short: BN6's the start of BN4's)
            if not spell(names['bn4'][x]).startswith(spell(name(r['bn6'], 'bn6', ALIAS[x]))):
                sys.exit(f'xreg_bn4: ALIAS {x} is not BN6\'s {ALIAS[x]} by name')
            paid[x] = ALIAS[x]
            continue
        # (BN6's own folder chip of its name; else the registry's own id of BN5's, no Program Advance)
        hit = by6 if by6 and by6 < COUNT['bn6'] else reg[by5][0] if by5 and reg[by5][1] != 'PA' else 0
        if hit:
            paid[x] = hit
        else:
            new.append(x)
    # (an SP or a DS whose Navi's V1 the run has: the run's chip of that version, where it has one)
    out_by_navi = set()
    for x in list(new):
        rx = rec('bn4', x)
        if rx[BN4_CLASS] != 1 or rx[PARAMS] not in (SP, DS):
            continue
        v = x - (rx[PARAMS] - SP + 1)
        rv = rec('bn4', v)
        if v in paid and rv[BN4_CLASS] == 1 and rv[PARAMS] == 0 and rv[FAMILY:SUB + 1] == rx[FAMILY:SUB + 1]:
            # (the registry's row of BN5's id as many after the V1's; BN6's own SP; else none: Roll's two and ProtoMan's DS)
            v5 = next((j for j in reg if reg[j][0] == paid[v] and reg[j][1] not in ('SAME', 'ALIAS')), 0)
            if v5 and (v5 + rx[PARAMS] - SP + 1) in reg:
                new.remove(x)
            elif not v5 and spell(names['bn4'][v]) == 'ProtoMan':
                new.remove(x)
                out_by_navi.add(x)
    new = [x for x in new if x not in OUT_OF]
    missing = [x for x in FIRST if x not in new and x not in ALIAS]
    if missing or len(set(FIRST)) != len(FIRST) or len(FIRST) != PACK_END - PACK_FIRST:
        sys.exit(f'xreg_bn4: FIRST does not hold {PACK_END - PACK_FIRST} of the new chips once each (not new: {missing})')
    order = list(FIRST) + [x for x in new if x not in FIRST]
    out = []
    for k, x in enumerate(order):
        if x in ALIAS:   # (its id retired: the ids after it stay)
            continue
        out.append((x, PACK_FIRST + k if k < len(FIRST) else REST_FIRST + k - len(FIRST), 'XREG_RECORD' if x in ON_BN6 else 'XREG_MOVED' if x in MOVED else 'XREG_PENDING'))
    for k, x in enumerate(pas):
        out.append((x, PA_FIRST + k, 'XREG_PA'))
    if len(pas) > REST_FIRST - PA_FIRST or out[len(order) - 1][1] > 1023:
        sys.exit('xreg_bn4: more Program Advances or chips than their ids')
    return [(x, to, how, rec('bn4', x), spell(names['bn4'][x])) for x, to, how in sorted(out)]


HEAD = '''/* BN4 Red Sun's rows of the chip registry (xchips_registry.h; docs/multirom/bn4-chips.md, The build plan's unit 2):
 * the chips a run has none of by name or by Navi, each with its id in BN6's tables, fixed here. Written by
 * tools/xreg_bn4.py from the three ROMs: numbers only (BN4's id, the id, how it comes in, its twin and subfamily where its
 * record runs on BN6's code, its codes, class and MB,
 * which its placeholder keeps where BN4's ROM is not read; and which rows are BN4's dark chips). 527-602 the chips that play first, 605-623 its Program
 * Advances, 635 on the rest. */
#include "xchips_registry.h"

const XChipReg4 xreg4_rows[] = {
'''


def text(rws):
    L = [HEAD]
    for x, to, how, rec, nm in rws:
        codes = ', '.join(f'0x{c:02X}' for c in rec[0:4])
        twin, sub = ON_BN6.get(x, (NAVI_TWIN if x in NAVI else 0, MOVED.get(x)))
        L.append(f'\t{{ {x:3}, {to:4}, {how}, {twin:3}, 0x{0xFF if sub is None else sub:02X}, {{ {codes} }}, {rec[BN4_CLASS]}, {rec[BN4_MB]:2} }},   /* {nm} */\n')
    L.append('};\nconst int xreg4_count = (int)(sizeof xreg4_rows / sizeof *xreg4_rows);\n')
    L.append('/* BN4\'s chips that are a chip of BN6\'s under another spelling: BN4\'s id, BN6\'s (pair_bn4) */\n')
    L.append('const uint16_t xreg4_alias[][2] = { ' + ', '.join(f'{{ {a}, {b} }}' for a, b in sorted(ALIAS.items())) + ' };\n')
    L.append('const int xreg4_alias_count = (int)(sizeof xreg4_alias / sizeof *xreg4_alias);\n')
    L.append('/* BN4\'s dark chips among the rows, by their id in BN6\'s tables: a record whose alignment is dark or a DarkChip\'s (xreg4_dark) */\n')
    dark = [to for x, to, how, rec, nm in rws if rec[ALIGN] == ALIGN_DARK or rec[FLAGS] & FLAG_DARK]
    L.append('const uint16_t xreg4_dark_ids[] = {\n' + ''.join('\t' + ', '.join(str(d) for d in dark[i:i + 16]) + ',\n' for i in range(0, len(dark), 16)) + '};\n')
    L.append('const int xreg4_dark_count = (int)(sizeof xreg4_dark_ids / sizeof *xreg4_dark_ids);\n')
    return ''.join(L)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--check', action='store_true')
    a = ap.parse_args()
    rws = rows(roms())
    t = text(rws)
    if a.check:
        with open(OUT, encoding='utf-8') as f:
            if f.read() != t:
                sys.exit('xreg_bn4: src/core/xchips_registry_bn4.c is not what the ROMs give (python3 tools/xreg_bn4.py)')
        print('xreg_bn4: the file is what the ROMs give')
        return
    with open(OUT, 'w', encoding='utf-8') as f:
        f.write(t)
    folder = sum(1 for r in rws if r[2] != 'XREG_PA')
    print(f'xreg_bn4: {len(rws)} rows ({folder} folder chips, {len(rws) - folder} Program Advances), ids to {max(r[1] for r in rws)}')


if __name__ == '__main__':
    main()
