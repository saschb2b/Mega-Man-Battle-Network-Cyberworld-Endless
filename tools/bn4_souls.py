#!/usr/bin/env python3
"""bn4_souls.py [--walk N]: BN4 Red Sun's twelve Souls read from the player's ROM, each against what moving it onto
BN6's proxy form takes (docs/multirom/bn4-souls.md; docs/rom/bn4.md, Souls in battle; the names in src/emu/bn4.h).

For each Soul: its pseudo-chip's name, the chip kind it unites by and its event flag (Red Sun's or refused), its row of
the tables by form (the loadout's three power attacks, the turns, the element, the enter routine, the companion), each
power attack's routine and the attack family it returns (a chip's, where the routine names one: BN4_CHIP_ATTACK), its
charge times, and the places BN4's engine compares the form with its number. Then the code it would bring: from its
roots (its power attacks, their families, its enter routine) every routine walked (tools/xrecipe_thumb.py), each call
out paired with BN6's twin by tools/xrecipe_bn4.py's pairer (BN4's project pairs, tools/bn4_twins.py, then a BN6
routine of the same body) and classed SAME, LAYOUT or DIFF; a call with no twin is walked as code the Soul brings, up to
--walk routines a Soul (the cost's measure, as tools/xsoul.py's blocks are BN5's). Fails where the ROM is not as
docs/rom/bn4.md says.

  python3 tools/bn4_souls.py
  python3 tools/bn4_souls.py --walk 400 --calls

Local only: BN4 Red Sun's ROM and BN6 Gregar's (by their SHA-1s in CYBERWORLD_ROM_DIR or ~/.cache/mmbn-ref/roms); the
symbol map of tools/bn6f_match.py in .build/symbols/match where it was made. Prints addresses and counts alone."""
import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_lab  # noqa: E402
import xrecipe_thumb as T  # noqa: E402
from xrecipe_bn4 import Pairs4, find_roms  # noqa: E402

# src/emu/bn4.h
SOUL_KINDS, SOUL_FLAGS, SOUL_CHIPS = 0x08020008, 0x08020018, 0x160
FORM_LOADOUTS, FORM_LOADOUT_SIZE = 0x080184F0, 4
FORM_ENTERS, FORM_TURNS, FORM_ELEMENTS, FORM_COMPANIONS = 0x0800E0A0, 0x080180A8, 0x0800C964, 0x08018068
POWER_ATTACKS, POWER_ATTACK_COUNT, CHARGE_TIMES, CHARGE_ROW = 0x0800CA7C, 108, 0x080180B8, 10
ATTACK_FAMILIES, ATTACK_FAMILY_COUNT = 0x080EAEFC, 116   # BN4_PLAYER_STATES, BN4_PLAYER_STATE_COUNT: by CurAction
CHIP_ATTACK, CHIP_RECORD_SIZE, CHIP_FAMILY = 0x0800D406, 0x2C, 0x0B   # BN4_CHIP_ATTACK, the record's size, BN4_CHIP_FAMILY
FORM_GETTERS = (0x0800D7CA, 0x0800D772)   # BN4_NAVI_BYTE, BN4_NAVI_BYTE_SIDE: read with r1 BN4_NAVI_FORM
NAVI_FORM = 0x24
FORM_NULLSUB = 0x0800E0E0                  # (mov pc, lr) the enter routine of a form with none
NAMES = (None, 'Roll', 'Guts', 'Wind', 'Search', 'Fire', 'Thunder', 'Proto', 'Number', 'Metal', 'Junk', 'Aqua', 'Wood')
ROLES = ('plain shot', 'chip charge', 'charged shot')   # the loadout's +1, +2, +3 (AI data +0xD, +0xE, +0xF)

# what docs/rom/bn4.md says of the tables, checked against the ROM: each Soul's charged shot's power attack and the
# family it returns (a chip's by its record where the routine names one; JunkSoul's chip is the battle's)
EXPECT = {1: (0x22, 0x1B), 2: (0x24, 0x12), 3: (0x09, 0x32), 4: (0x0A, 0x3E), 5: (0x0C, 0x19), 6: (0x58, 0x47),
          7: (0x3E, 0x0A), 8: (0x11, 0x15), 9: (0x13, 0x14), 10: (0x61, None), 11: (0x23, 0x1E), 12: (0x57, 0x28)}


def last_r0(w, at):
    """the constant `movs r0, #N` gives r0 last before `at` in a walk, or None where r0 is set otherwise"""
    j = at - 2
    while j in w.insns:
        t = w.insns[j].text
        if t.startswith('movs r0, #'):
            return int(t.split('#')[1], 16)
        if t.split(',')[0].split()[-1:] == ['r0'] or t.startswith('bl'):
            return None
        j -= 2
    return None


def power_attack(b4, k):
    """(its routine, the family it returns or None, the chip it names or None) of power attack k"""
    entry = b4.u32(POWER_ATTACKS + 4 * k) & ~1
    w = T.walk(b4, entry)
    chip = next((last_r0(w, at) for at, t in w.bls.items() if t == CHIP_ATTACK), None)
    if chip is not None:
        return entry, b4.u8(bn4_lab.CHIP_RECORDS + CHIP_RECORD_SIZE * chip + CHIP_FAMILY), chip
    fams = {last_r0(w, at) for at, i in w.insns.items() if T.is_return(i)} - {None}
    return entry, (min(fams) if fams else None), None


def form_compares(b4):
    """{form: [sites]}: where BN4's engine reads the form (a getter with r1 BN4_NAVI_FORM) and compares it, or a copy of
    it, with a number before the copy is set again, read straight on to the first jump (each a place a form's ability or
    look lives in the engine's shared code)"""
    out = {}
    for a in range(0x08000000, 0x08100000, 2):
        i = T.decode(b4, a)
        if i.form != 'bl' or i.target not in FORM_GETTERS or T.decode(b4, a - 2).text != f'movs r1, #{NAVI_FORM:#x}':
            continue
        held, at = {0}, a + 4
        for _ in range(40):
            x = T.decode(b4, at)
            f = x.text.replace(',', '').split()
            if x.form == 'cmp' and f[1][1:].isdigit() and int(f[1][1:]) in held and f[2].startswith('#'):
                n = int(f[2][1:], 16)
                if a not in out.get(n, []):
                    out.setdefault(n, []).append(a)
            if x.form in ('b', 'bx') or T.is_return(x):
                break
            if x.form == 'bl':   # (a call keeps r4-r7)
                held -= {0, 1, 2, 3}
            copy = len(f) == 4 and f[0] == 'adds' and f[3] in ('#0', '#0x0') and f[2][1:].isdigit() and int(f[2][1:]) in held
            held = {r for r in held if not T.writes(x, r)} | ({int(f[1][1:])} if copy else set())
            if not held:
                break
            at += x.size
    return out


def tables_of(b4, w):
    """the routines of the jump tables a walk's literals name (a table: two code pointers or more in a row)"""
    out = []
    for v in w.lits.values():
        if not (0x08000000 <= v < 0x08800000) or v & 3 or not b4.has(v, 8):
            continue
        k, got = 0, []
        while k < 64 and b4.has(v + 4 * k, 4) and T.is_code_ptr(b4.u32(v + 4 * k), b4):
            got.append(b4.u32(v + 4 * k) & ~1)
            k += 1
        if len(got) >= 2:
            out += got
    return out


class Unit:
    """a Soul's code: its routines walked from its roots, its calls out paired"""

    def __init__(self):
        self.walked, self.calls, self.stopped = {}, {}, False

    def bytes(self):
        return sum(sum(i.size for i in w.insns.values()) + 4 * len(w.lits) for w in self.walked.values())

    def classes(self, pairs):
        out = {}
        for t, tw in self.calls.items():
            c = tw[2] + (', audited' if tw[2] == 'DIFF' and pairs.audit(t, tw[0]) else '')
            out[c] = out.get(c, 0) + 1
        return out


def bring(b4, pairs, roots, limit, seen):
    """the routines a Soul brings from its roots and its calls out: a routine BN6 has the twin of is a call out, any other
    walked (seen: the routines another Soul brought first, counted once)"""
    u = Unit()
    todo = sorted({r & ~1 for r in roots}, reverse=True)
    while todo:
        e = todo.pop()
        if e in u.walked or e in u.calls:
            continue
        tw = pairs.twin(e)
        if tw[0]:
            u.calls[e] = tw
            continue
        if len(u.walked) >= limit:
            u.stopped = True
            break
        w = T.walk(b4, e)
        if e not in seen:
            u.walked[e] = w
        seen.add(e)
        nxt = {t & ~1 for t in w.bls.values()} | set(tables_of(b4, w))
        todo.extend(sorted(nxt - set(u.walked) - set(u.calls), reverse=True))
    return u


def soul_rows(b4, L, compares):
    rows, bad = [], []
    for n in range(1, 13):
        lo = [b4.u8(FORM_LOADOUTS + FORM_LOADOUT_SIZE * n + k) for k in range(FORM_LOADOUT_SIZE)]
        powers = []
        for role, k in zip(ROLES, lo[1:]):
            if k in (0x00, 0xFF):
                continue
            entry, fam, chip = power_attack(b4, k)
            times = [b4.u16(CHARGE_TIMES + CHARGE_ROW * k + 2 * c) for c in range(5)]
            powers.append((role, k, entry, fam, chip, times))
        charged = next((p for p in powers if p[0] == 'charged shot'), None)
        if not charged or (charged[1], charged[3]) != EXPECT[n]:
            bad.append(f'{NAMES[n]}Soul\'s charged shot: {charged and (hex(charged[1]), charged[3])}, not {EXPECT[n]}')
        enter = b4.u32(FORM_ENTERS + 4 * n) & ~1
        rows.append(dict(n=n, name=L.text(bn4_lab.CHIP_NAMES[1], SOUL_CHIPS + n - 256), kind=b4.u8(SOUL_KINDS + n - 1),
                         flag=b4.u8(SOUL_FLAGS + n - 1), loadout=lo, powers=powers,
                         enter=None if enter in (FORM_NULLSUB, FORM_NULLSUB + 2) else enter,
                         turns=b4.u8(FORM_TURNS + n), element=b4.u8(FORM_ELEMENTS + n), companion=b4.u16(FORM_COMPANIONS + 2 * n),
                         compares=compares.get(n, [])))
    return rows, bad


def report(rows, units, pairs, calls=False):
    total = {}
    for row in rows:
        u = units[row['n']]
        print(f"{row['n']:2} {row['name']:8} kind {row['kind']:2}  flag {row['flag']:#04x}"
              f" ({'Red Sun' if row['flag'] != 0xFF else 'refused: Blue Moon'})  loadout {bytes(row['loadout']).hex(' ')}"
              f"  turns {row['turns']}  element {row['element']}  companion {row['companion']:#06x}")
        for role, k, entry, fam, chip, times in row['powers']:
            print(f"     {role}: power {k:#04x} at {entry:08X}, family "
                  + (f"{fam:#04x}" if fam is not None else 'the battle\'s chip')
                  + (f" (chip {chip:#x}'s)" if chip is not None else '') + f", charge {times[0]} frames")
        enter = f"{row['enter']:08X}" if row['enter'] else 'none'
        print(f"     enter routine {enter}; the engine compares the form "
              f"with {row['n']} at {len(row['compares'])} places: " + ' '.join(f'{a:08X}' for a in row['compares']))
        cls = u.classes(pairs)
        for c, v in cls.items():
            total[c] = total.get(c, 0) + v
        print(f"     brings {len(u.walked)} routines ({u.bytes()} bytes){' and more (--walk)' if u.stopped else ''}; calls out: "
              + ', '.join(f'{v} {c}' for c, v in sorted(cls.items())))
        if calls:
            for t, tw in sorted(u.calls.items()):
                if tw[2] != 'SAME':
                    why = pairs.audit(t, tw[0])
                    print(f"       {t:08X} -> {tw[0]:08X} {tw[2]} ({tw[1]})" + (f': {why[1]}' if why else f': {tw[3]}' if tw[3] else ''))
    print('calls out, all twelve: ' + ', '.join(f'{v} {c}' for c, v in sorted(total.items())))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--walk', type=int, default=200, help='routines walked a Soul at most (default 200)')
    ap.add_argument('--calls', action='store_true', help='also each call out BN6 has a twin of by LAYOUT or DIFF')
    a = ap.parse_args()
    b4, b6 = find_roms()
    L = bn4_lab.Rom()
    rows, bad = soul_rows(b4, L, form_compares(b4))
    pairs, seen, units = Pairs4(b4, b6), set(), {}
    for row in rows:
        roots = [p[2] for p in row['powers']] + [b4.u32(ATTACK_FAMILIES + 4 * p[3]) for p in row['powers']
                                                  if p[3] is not None and p[3] < ATTACK_FAMILY_COUNT]
        units[row['n']] = bring(b4, pairs, roots + ([row['enter']] if row['enter'] else []), a.walk, seen)
    report(rows, units, pairs, a.calls)
    for b in bad:
        print(f'  not as docs/rom/bn4.md says: {b}')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
