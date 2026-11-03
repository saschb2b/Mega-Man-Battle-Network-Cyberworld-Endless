#!/usr/bin/env python3
"""xrecipe_bn4.py [--check]: BN4 Red Sun's virus families' recipes, made from BN4's ROM and BN6 Gregar's, as
tools/xrecipe.py makes BN5's (docs/multirom/families.md; docs/devtools/recipe-generator.md; docs/rom/bn4.md, Virus
families).

BN4 has none of BN6's per-family tables: a family is a first-kind object type of its own (BN4_T1_TYPES), its routine a
jump table on +8 (0 its init, 4 its update, 8 the shared dying state). This finds the family by its type, its region (to
the next type's routine), its state table from the literal its update hands the state runner, its own states (past
BN4's generic 0-5, bn4_twins.GENERIC4) as BN6's from 8; walks every routine those states reach (their jump tables
followed) and pairs each call out with BN6's twin (tools/bn4_twins.py, each pair compared by its two bodies; else a BN6
routine of the same body, tools/xrecipe_pairs.py's search); rewrites the immediates BN6 numbers otherwise (CurAction,
the element's place, the collision data's offsets, bn4.h's BN4_COLL_*); its struct1 made from its init (sprite list and
number, palette step, shadow) and its records (element), its struct2 from its six enemy records (BN4_ENEMY_STATS).

  python3 tools/xrecipe_bn4.py           write src/emu/xvirus_bn4.c (the families whole, no decision open) and the report,
                                         .build/xrecipe/bn4_report.md: what each family takes and what is still open
  python3 tools/xrecipe_bn4.py --check   write nothing: fail where src/emu/xvirus_bn4.c is not what it would write

Local only: both ROMs. Nothing of either is written: addresses, counts and CRC32s."""
import argparse
import hashlib
import os
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_twins  # noqa: E402
import xrecipe_thumb as T  # noqa: E402
from xrecipe_family import BN6, GENERIC6  # noqa: E402
from xrecipe_pairs import Pairs  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_C = os.path.join(ROOT, 'src', 'emu', 'xvirus_bn4.c')
WORK = os.path.join(ROOT, '.build', 'xrecipe')
MATCH = os.path.join(ROOT, '.build', 'symbols', 'match')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
SHA1 = {'bn6': '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6', 'bn4': 'a97e96a7da03abd70f7953328e511b9fa29179f1'}

# BN4's families, in install order: (its first-kind type, the project's name, its first enemy id); each its six
# versions' ids from there (BN4_ENEMY_STATS' records name the type, checked)
FAMILIES = ((0x13, 'Billy', 13),)

# ---- BN4's tables (src/emu/bn4.h; docs/rom/bn4.md) ----
BN4 = dict(t1=0x08003A24, nt1=95, stats=0x08011888, stat_size=12, names=0x0874A780,
           prestep=0x080137BC, runner=0x08013C48, present=0x08012D62, dying=0x0801052C,
           sprite_load=0x0800287C, shadow=0x08002E96, no_shadow=0x08002FE2, palette=0x08002DEC)
# The collision data's offsets BN4's code may name, as BN6's (bn4.h BN4_COLL_*; docs/rom/bn4.md, The battle's objects)
COLL4TO6 = {0x03: 0x04, 0x08: 0x0A, 0x09: 0x0B, 0x10: 0x1C, 0x18: 0x24, 0x1C: 0x28, 0x28: 0x80, 0x64: 0x3C, 0x68: 0x40}
ELEMENT4, ELEMENT6 = 0x17, 0x0E   # an object's element (BN4_T1_ELEMENT), BN6's (BN6_T1_ELEMENT)

# The numbers BN4's families take, past BN5's (tools/xrecipe.py's 25 in slots 0-24, their code to XV_CODE + 0x11000 and
# their art to XV_ART + 0x54000): slot 32 on (AI 64, ids 0x2C0), their code and art apart from BN5's, list 1's numbers
# from 0xC0 (past those BN5's families and the moved objects take)
XV_AT = 0x08800000 + 0x330000
XV_HOMES, XV_CODE, XV_ART = XV_AT + 0x18000, XV_AT + 0x38000, XV_AT + 0x50000
XV_HOME_SIZE, XV_FIRST_ID, XV_FIRST_AI = 0x800, 0x200, 32
SLOT0, CODE0, ART0, LIST1_0 = 32, XV_CODE + 0x14000, XV_ART + 0x60000, 0xC0
CODE_STEP, ART_STEP, ROOM_BATTLE = 0x200, 0x800, 0x1000


def find_roms():
    out = {}
    for n in sorted(os.listdir(ROM_DIR)) if os.path.isdir(ROM_DIR) else []:
        p = os.path.join(ROM_DIR, n)
        if os.path.isfile(p) and os.path.getsize(p) <= 0x2000000:
            with open(p, 'rb') as f:
                data = f.read()
            for k, s in SHA1.items():
                if hashlib.sha1(data).hexdigest() == s:
                    out[k] = T.Rom(data, k)
    missing = [k for k in SHA1 if k not in out]
    if missing:
        sys.exit(f'xrecipe_bn4: no ROM of {", ".join(missing)} in {ROM_DIR}')
    return out['bn4'], out['bn6']


class Pairs4(Pairs):
    """tools/xrecipe_pairs.py's pairs with BN4 in BN5's place: the project's pairs (bn4_twins) first, then a BN6 routine of
    the same body; no symbol map of BN4's"""

    def __init__(self, b4, b6):
        self.b5, self.b6, self.search, self.cache = b4, b6, True, WORK
        self.twins = {a: b for a, b, _ in bn4_twins.TWINS}
        self.twin_src = {a: s for a, _, s in bn4_twins.TWINS}
        _, self.by6name, _ = self.read_map(MATCH)
        self.by5, self.map_ok, self._index, self._anchors, self.memo = {}, False, None, None, {}
        self.known_cache = {}

    def map_twin(self, a5):
        return None, None

    def expected(self, a5):
        return None

    def audit(self, a5, a6):
        return bn4_twins.AUDITS.get((a5 & ~1, a6 & ~1))


class Family:
    def __init__(self, ty, name, first):
        self.ty, self.name, self.first = ty, name, first
        self.decisions, self.notes = [], []
        self.walks, self.tables, self.calls, self.imms, self.words = {}, {}, {}, [], {}

    def decide(self, kind, detail):
        self.decisions.append((kind, detail))


def region_of(b4, ty):
    types = [b4.u32(BN4['t1'] + 4 * i) & ~1 for i in range(BN4['nt1'])]
    starts = sorted(set(types))
    a = types[ty]
    i = starts.index(a)
    return a, starts[i + 1]


def table_at(b4, v, lo, hi, n=16):
    ents = []
    for k in range(n):
        x = b4.u32(v + 4 * k)
        if not T.is_code_ptr(x, b4):
            break
        ents.append(x)
    return ents


def type_table(b4, f):
    """its routine's jump table on +8: init, update, dying"""
    w = T.walk(b4, f.lo, region=(f.lo, f.hi))
    for lit, v in w.lits.items():
        if v and f.lo <= v < f.hi and not v & 3:
            ents = table_at(b4, v, f.lo, f.hi, 3)
            if len(ents) == 3:
                return v, w, ents
    return None, w, None


def struct1_of(b4, f, init):
    """its sprite (list, number), palette step and shadow from its init's calls"""
    w = T.walk(b4, init, region=(f.lo, f.hi))
    f.walks[init & ~1] = w
    s = dict(list=None, index=None, palette=None, shadow=None)
    for site, t in sorted(w.bls.items()):
        if t == BN4['sprite_load']:
            lst, _ = T.back_value(w, site, 1)
            num, _ = T.back_value(w, site, 2)
            s['list'], s['index'] = (lst // 4 if lst is not None and not lst & 3 else None), num
        elif t == BN4['shadow']:
            s['shadow'] = True
        elif t == BN4['no_shadow']:
            s['shadow'] = False
        elif t == BN4['palette']:
            prev = [w.insns[a] for a in sorted(w.insns) if a < site][-2:]
            if [i.form for i in prev] == ['ldrb', 'lsls'] and prev[0].f[1:] == (5, 4):
                s['palette'] = 1 << prev[1].f[2]
    for k, v in s.items():
        if v is None:
            f.decide('struct1', f'its init ({init:08X}) gives no {k} the generator reads')
    return s


def walk_states(b4, f, roots, skip):
    todo = list(roots)
    while todo:
        e = todo.pop() & ~1
        if e in f.walks and e not in roots:
            continue
        w = T.walk(b4, e, region=(f.lo, f.hi))
        f.walks[e] = w
        for site, t in w.bls.items():
            if f.lo <= t < f.hi:
                todo.append(t)
            else:
                f.calls.setdefault(t, []).append(site)
        for lit, v in w.lits.items():
            if v is None or v in skip or v & 3 or not f.lo <= v < f.hi:
                continue
            ents = [x for x in table_at(b4, v, f.lo, f.hi) if f.lo <= (x & ~1) < f.hi]
            if ents:
                f.tables[v] = ents
                todo += [x for x in ents if (x & ~1) not in f.walks]
        roots = ()


def scan(f):
    """CurAction (+9: BN4's own states from 6, BN6's from 8), the element's place, its collision data's offsets, the
    object's own fields"""
    forms = {'ldr': 'THUMB_LDR', 'str': 'THUMB_STR', 'ldrb': 'THUMB_LDRB', 'strb': 'THUMB_STRB', 'ldrh': 'THUMB_LDRH', 'strh': 'THUMB_STRH'}
    fields = set()
    for e, w in sorted(f.walks.items()):
        if e == f.init:
            continue
        addrs = sorted(w.insns)
        coll = set()
        for k, a in enumerate(addrs):
            i = w.insns[a]
            if i.form == 'strb' and i.f[1:] == (5, 9):
                v, chain = T.back_value(w, a, i.f[0])
                if v is not None and 6 <= v <= 0xF and len(chain) == 1 and chain[0][1] == 'movs':
                    f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=i.f[0], rn=0, was=v, now=v + 2, why=f'CurAction {v} written'))
                elif v is None or v >= 6:
                    f.decide('curaction', f'{a:08X}: CurAction written from {chain}')
            if i.form == 'ldrb' and i.f[1:] == (5, 9):
                for b in addrs[k + 1:k + 9]:
                    j = w.insns[b]
                    if j.form == 'cmp' and j.f[0] == i.f[0] and '#' in j.text and 6 <= j.f[1] <= 0xF:
                        f.imms.append(dict(at=b, form='THUMB_CMP', rd=j.f[0], rn=0, was=j.f[1], now=j.f[1] + 2, why=f'CurAction {j.f[1]} compared'))
                    elif j.form in ('b', 'bx', 'bl') or T.is_return(j) or T.writes(j, i.f[0]):
                        break
            if i.form in forms and i.f[1] == 5:
                off = i.f[2]
                if off == ELEMENT4:
                    f.imms.append(dict(at=a, form=forms[i.form], rd=i.f[0], rn=5, was=off, now=ELEMENT6, why='its element: BN4 +0x17, BN6 +0x0E'))
                elif off >= 0x5C:
                    fields.add(off)
                if i.form == 'ldr' and off == 0x54:
                    coll.add(i.f[0])
                    continue
            if i.form == 'movs' and i.f[1] >= 0x5C and k + 1 < len(addrs):
                j = w.insns[addrs[k + 1]]
                if j.form == 'adds_r' or 'r5' in j.text and j.text.startswith('adds'):
                    fields.add(i.f[1])
            if i.form in forms and i.f[1] in coll:
                to = COLL4TO6.get(i.f[2])
                if to is None:
                    f.decide('collision', f'{a:08X}: its collision data\'s +{i.f[2]:#x}, which bn4.h gives no BN6 place')
                elif to != i.f[2]:
                    f.imms.append(dict(at=a, form=forms[i.form], rd=i.f[0], rn=i.f[1], was=i.f[2], now=to,
                                       why=f'collision data +{i.f[2]:#x} in BN4, +{to:#x} in BN6'))
            if i.form == 'bl':
                coll -= {0, 1, 2, 3}
    for off in sorted(fields):
        f.decide('object field', f'its object\'s +{off:#x} (BN4 keeps a family\'s variables in the object; what BN6\'s engine '
                                 f'keeps there for a virus is the field audit\'s to say)')


def pair_calls(f, pairs):
    f.pairs = []
    for t, sites in sorted(f.calls.items()):
        tw, how, cls, diff = pairs.twin(t)
        if tw is None:
            f.decide('call', f'{t:08X} ({len(sites)} BLs, at {", ".join(f"{s:08X}" for s in sites)}): no twin in BN6 ({how})')
        elif cls == 'DIFF' and not pairs.audit(t, tw):
            f.decide('call', f'{t:08X} -> {tw:08X} ({how}): their bodies differ ({diff}), and no audit says the pair serves (bn4_twins.AUDITS)')
        f.pairs.append((t, tw, how, cls, sites))


def words_of(b4, f):
    for e, w in f.walks.items():
        if e == f.init:
            continue
        for lit, v in w.lits.items():
            if v is None or f.lo <= (v & ~1) < f.hi or not 0x02000000 <= v < 0x0E000000:
                continue
            kind = 'RAM' if v < 0x04000000 else 'I/O' if v < 0x05000000 else 'video' if v < 0x08000000 else 'ROM'
            if kind == 'I/O':
                f.words[v] = v
            else:
                f.decide('literal', f'{v:08X} ({kind}) at {lit:08X}: no twin known')


def records_of(b4, f):
    st = BN4['stats']
    recs = [b4.bytes(st + BN4['stat_size'] * (f.first + v), BN4['stat_size']) for v in range(6)]
    for v, r in enumerate(recs):
        if r[2] != f.ty or r[5] != v:
            f.decide('records', f'enemy id {f.first + v}\'s record names type {r[2]:#x}, version {r[5]} (not {f.ty:#x}, {v})')
    elems = {int.from_bytes(r[0:2], 'little') >> 12 for r in recs}
    if len(elems) != 1:
        f.decide('element', f'its versions\' elements differ ({sorted(elems)})')
    f.element = min(elems)
    f.notes.append('struct2 from its six records: BN4\'s u16 element << 12 | HP and u16 attack (+0xA) as BN6\'s first and last words, '
                   'the version its place; BN6\'s flags byte 0')
    f.decide('struct2', 'the attack word\'s element: BN4\'s +0xA has none in its high bits; BN6\'s damage word takes element << 12 '
                        '(which BN6\'s engine reads of it is the field audit\'s to say)')


def derive(b4, b6, pairs, ty, name, first, slot):
    f = Family(ty, name, first)
    f.lo, f.hi = region_of(b4, ty)
    tt, tw, ents = type_table(b4, f)
    if not ents:
        f.decide('type', f'its routine ({f.lo:08X}) names no jump table of three')
        return f
    f.type_table, f.init, f.update = tt, ents[0] & ~1, ents[1] & ~1
    if ents[2] & ~1 != BN4['dying']:
        f.decide('type', f'its dying entry is {ents[2]:08X}, not BN4\'s shared {BN4["dying"]:08X}')
    uw = T.walk(b4, f.update, region=(f.lo, f.hi))
    f.walks[f.update] = uw
    f.walks[f.lo] = tw
    called = [t for _, t in sorted(uw.bls.items())]
    if called != [BN4['prestep'], BN4['runner'], BN4['present']]:
        f.decide('pre-step', f'its update calls {", ".join(f"{t:08X}" for t in called)}, not BN4\'s pre-step, state runner and present alone')
    site = [s for s, t in uw.bls.items() if t == BN4['runner']]
    f.st, _ = T.back_value(uw, site[0], 0) if site else (None, None)
    states = table_at(b4, f.st, f.lo, f.hi, 24) if f.st else []
    for k in range(6):
        if k >= len(states) or states[k] & ~1 != bn4_twins.GENERIC4[k]:
            f.decide('states', f'its state {k} is not BN4\'s generic one')
    f.own = [x for x in states[6:]]
    f.states = [('BN6', GENERIC6[k] | 1) for k in range(8)] + [('OWN', x | 1) for x in f.own]
    f.s1 = struct1_of(b4, f, f.init)
    walk_states(b4, f, [x & ~1 for x in f.own], {f.type_table, f.st})
    scan(f)
    pair_calls(f, pairs)
    words_of(b4, f)
    records_of(b4, f)
    f.slot = SLOT0 + slot
    f.ai, f.first_id, f.home = XV_FIRST_AI + f.slot, XV_FIRST_ID + 6 * f.slot, XV_HOMES + XV_HOME_SIZE * f.slot
    f.at = CODE0 + (f.lo & 3)
    f.crc = zlib.crc32(b4.bytes(f.lo, f.hi - f.lo)) & 0xFFFFFFFF
    f.data = data_of(b4, f)
    f.sprite = dict(list=f.s1['list'], index=f.s1['index'], to_list=1, to_index=LIST1_0 + slot, at=ART0 + ART_STEP * slot, room=ROOM_BATTLE)
    f.decisions.append(('sprite', 'its battle sprite\'s bytes (BN4\'s list 1 entry, LZ77 or not) and its room, read by the art step '
                                  'tools/xrecipe.py\'s art_of does for BN5: not ported yet'))
    f.decisions.append(('init', f'its init\'s collision setup (object_setupCollisionData r1-r3 1, 2, 3), its call {0x080167C0:08X} '
                                f'and its flag set, which BN6\'s virus init does from its records: each to be held to BN6\'s'))
    return f


def data_of(b4, f):
    """its region's bytes classed: code its states reach, their pools and jump tables, the dead (its type's routine, jump
    table, init and update, its state table: BN6's virus code does their work) zeroed, the rest raw"""
    code, pools = set(), set()
    for e, w in f.walks.items():
        if e in (f.lo, f.init, f.update):
            continue
        for a, i in w.insns.items():
            code.update(range(a, a + i.size))
        for lit in w.lits:
            pools.update(range(lit, lit + 4))
    tables = set()
    for v, ents in f.tables.items():
        tables.update(range(v, v + 4 * len(ents)))
    dead = set()
    for e in (f.lo, f.init, f.update):
        w = f.walks.get(e) or T.walk(b4, e, region=(f.lo, f.hi))
        for a, i in w.insns.items():
            dead.update(range(a, a + i.size))
        for lit in w.lits:
            dead.update(range(lit, lit + 4))
    dead.update(range(f.type_table, f.type_table + 12))
    dead.update(range(f.st, f.st + 4 * (6 + len(f.own))))
    out, kind_at = [], []
    for a in range(f.lo, f.hi):
        k = 'CODE' if a in code else 'POOL' if a in pools else 'TABLE' if a in tables else 'ZERO' if a in dead else 'RAW'
        kind_at.append(k)
    a = f.lo
    while a < f.hi:
        k = kind_at[a - f.lo]
        b = a
        while b < f.hi and kind_at[b - f.lo] == k:
            b += 1
        if k != 'CODE':
            out.append((a, b, k))
        a = b
    return out


# ---- the C file and the report ----

HEADER = '''/* xvirus_bn4.c: BN4 Red Sun's virus families' recipes, written by tools/xrecipe_bn4.py from BN4's and BN6's ROMs
 * (python3 tools/xrecipe_bn4.py; --check says whether this is what it writes). Do not edit: change the generator (or
 * tools/bn4_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/multirom/families.md): addresses
 * in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA), counts and CRC32s. The families here are the generator's whole ones
 * (no decision open), installed only where BN4's ROM is read for research (xrom_research: --dev bn4families); the open
 * ones stay in its report (.build/xrecipe/bn4_report.md). */
#include "xvirus_bn4.h"

#include <stddef.h>
'''


def c_file(whole):
    L = [HEADER]
    if whole:
        sys.exit('xrecipe_bn4: a whole BN4 family: its C writer is the next step (tools/xrecipe.py\'s c_recipe for game BN4)')
    L.append('/* (none whole yet: Billy\'s open decisions are in the report) */')
    L.append('const XVirusFamily *const xvirus_bn4[1] = { NULL };')
    L.append('const int xvirus_bn4_count = 0;')
    return '\n'.join(L) + '\n'


def report(fams, path):
    L = ['# BN4\'s virus families as tools/xrecipe_bn4.py derives them', '']
    for f in fams:
        L += [f'## {f.name} (type {f.ty:#x}, enemy ids {f.first}-{f.first + 5})', '']
        if hasattr(f, 'st'):
            L += [f'- region {f.lo:08X}-{f.hi:08X}, its routine\'s jump table {f.type_table:08X} (init {f.init:08X}, update {f.update:08X})',
                  f'- state table {f.st:08X}: its own states {", ".join(f"{x:08X}" for x in f.own)} as BN6\'s 8-{7 + len(f.own)}',
                  f'- struct1: sprite list {f.s1["list"]} number {f.s1["index"]}, palette step {f.s1["palette"]}, shadow {f.s1["shadow"]}, '
                  f'element {f.element}',
                  f'- as AI {f.ai}, ids {f.first_id:#x}-{f.first_id + 5:#x}, home {f.home:08X}, code at {f.at:08X} (CRC32 {f.crc:08X})',
                  f'- {len(f.walks)} routines walked, {len(f.tables)} jump tables, {len(f.imms)} immediates rewritten', '',
                  '### Calls out', '']
            for t, tw, how, cls, sites in f.pairs:
                L.append(f'- {t:08X} x{len(sites)}: ' + (f'BN6 {tw:08X} ({how}, {cls})' if tw else f'none ({how})'))
            L += ['', '### Immediates', '']
            L += [f'- {i["at"]:08X} {i["form"]} {i["was"]:#x} -> {i["now"]:#x}: {i["why"]}' for i in f.imms]
            L += ['', '### Data', ''] + [f'- {a:08X}-{b:08X} {k}' for a, b, k in f.data]
        L += ['', f'### Open decisions ({len(f.decisions)})', ''] + [f'- **{k}**: {d}' for k, d in f.decisions]
        L += ['', '### Notes', ''] + [f'- {n}' for n in f.notes] + ['']
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8') as fh:
        fh.write('\n'.join(L))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--check', action='store_true')
    a = ap.parse_args()
    if a.check and not os.path.isdir(ROM_DIR):
        print('xrecipe_bn4: no ROMs here: not checked')
        return 0
    b4, b6 = find_roms()
    pairs = Pairs4(b4, b6)
    fams = [derive(b4, b6, pairs, ty, name, first, k) for k, (ty, name, first) in enumerate(FAMILIES)]
    whole = [f for f in fams if not f.decisions]
    text = c_file(whole)
    if a.check:
        with open(OUT_C, encoding='utf-8') as fh:
            if fh.read() != text:
                print(f'xrecipe_bn4: {OUT_C} is not what the generator writes (python3 tools/xrecipe_bn4.py)')
                return 1
        return 0
    with open(OUT_C, 'w', encoding='utf-8') as fh:
        fh.write(text)
    report(fams, os.path.join(WORK, 'bn4_report.md'))
    for f in fams:
        print(f'{f.name}: {len(f.decisions)} decisions open, {len(f.imms)} immediates, {len(getattr(f, "pairs", []))} calls out')
    print(f'wrote {OUT_C} ({len(whole)} whole) and {os.path.join(WORK, "bn4_report.md")}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
