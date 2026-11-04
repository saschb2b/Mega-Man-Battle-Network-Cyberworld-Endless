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
import re
import sys
import zlib
from struct import error as struct_error

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_twins  # noqa: E402
import xrecipe_thumb as T  # noqa: E402
from xrecipe_family import BN6, GENERIC6  # noqa: E402
from xrecipe_pairs import Pairs  # noqa: E402
from xrecipe import sprite  # noqa: E402

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
           sprite_load=0x0800287C, shadow=0x08002E96, no_shadow=0x08002FE2, palette=0x08002DEC,
           byname=0x080B6568, sprites=0x0802793C, effects=0x080E1698, neffects=111, spawn_effect=0x080E18EE)
NULLSUB6 = 0x0810A953   # BN6_FAMILY_NULLSUB
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
CODE_STEP, ART_STEP, ROOM_BATTLE, ROOM_EFFECT = 0x200, 0x800, 0x1000, 0x400
# BN4's effects BN6 has no twin of, at ids and list 5 numbers of their own: the families' ranges' top (0x98-0x9F of the
# effects, list 5's 0x98-0x9F), where BN5's families take theirs from the bottom (tools/xrecipe.py: effects from 0x6C, list
# 5 from 0x80) and the Navis', Souls' and chips' are above (0xA0 on); an effect's sprite of another list is a decision
EFFECT0, EFFECT_END, LIST5_0, LIST5_END = 0x98, 0xA0, 0x98, 0xA0
BN5_RECIPES = os.path.join(ROOT, 'src', 'emu', 'xvirus_bn5.c')


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

    def decide(self, kind, detail):  # noqa: D401
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


def walk_code(b4, f, roots, skip):
    """its states and every routine they reach in its region (their jump tables followed), and the routines of BN4's
    they call that BN6 has no twin of, copied (bn4_twins.COPIED): each walked, the rest of the calls out noted"""
    todo = list(roots)
    while todo:
        e = todo.pop() & ~1
        if e in f.walks:
            continue
        own = f.lo <= e < f.hi
        w = T.walk(b4, e, region=(f.lo, f.hi) if own else None)
        f.walks[e] = w
        for site, t in w.bls.items():
            if f.lo <= t < f.hi or t in bn4_twins.COPIED:
                todo.append(t)
            else:
                f.calls.setdefault(t, []).append(site)
        for lit, v in w.lits.items():
            if v is None or v in skip or v & 3 or not own or not f.lo <= v < f.hi:
                continue
            ents = [x for x in table_at(b4, v, f.lo, f.hi) if f.lo <= (x & ~1) < f.hi]
            if ents:
                f.tables[v] = ents
                todo += [x for x in ents if (x & ~1) not in f.walks]
    f.copied = sorted(e for e in f.walks if e in bn4_twins.COPIED)


FORMS = {'ldr': 'THUMB_LDR', 'str': 'THUMB_STR', 'ldrb': 'THUMB_LDRB', 'strb': 'THUMB_STRB', 'ldrh': 'THUMB_LDRH', 'strh': 'THUMB_STRH'}


def curaction(f, w, addrs, k, i):
    """a CurAction (+9) written or compared: BN4's own states from 6, BN6's from 8"""
    if i.form == 'strb' and i.f[1:] == (5, 9):
        v, chain = T.back_value(w, addrs[k], i.f[0])
        if v is not None and 6 <= v <= 0xF and len(chain) == 1 and chain[0][1] == 'movs':
            f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=i.f[0], rn=0, was=v, now=v + 2, why=f'CurAction {v} written'))
        elif v is None or v >= 6:
            f.decide('curaction', f'{addrs[k]:08X}: CurAction written from {chain}')
    prev = w.insns.get(addrs[k - 1]) if k else None
    listed = bool(f.ranges) and prev is not None and prev.form == 'ldr' and prev.f[0] == i.f[1] and prev.f[2] == 0
    if i.form == 'ldrb' and i.f[2] == 9 and (i.f[1] == 5 or listed):
        # (its own, or another object's of its family: one its NameID range found, its pointer just read from the list; any
        # other base is its own variables', +0x80 on, not a CurAction)
        who = 'its' if i.f[1] == 5 else 'another of its family\'s (its NameID range\'s)'
        for b in addrs[k + 1:k + 9]:
            j = w.insns[b]
            if j.form == 'cmp' and j.f[0] == i.f[0] and '#' in j.text and 6 <= j.f[1] <= 0xF:
                f.imms.append(dict(at=b, form='THUMB_CMP', rd=j.f[0], rn=0, was=j.f[1], now=j.f[1] + 2, why=f'{who} CurAction {j.f[1]} compared'))
            elif j.form in ('b', 'bx', 'bl') or T.is_return(j) or T.writes(j, i.f[0]):
                break


def panel_type(f, w, addrs, k, i):
    """a copied routine's read of a panel's type after BN4's getPanel (+0, BN6's +2) and its compares"""
    if i.form != 'bl' or i.target != bn4_twins.GET_PANEL:
        return
    for b in addrs[k + 1:k + 6]:
        j = w.insns[b]
        if j.form == 'ldrb' and j.f[1] == 0 and j.f[2] == 0:
            f.imms.append(dict(at=b, form='THUMB_LDRB', rd=j.f[0], rn=0, was=0, now=2, why='a panel\'s type: BN4 +0, BN6 +2'))
            for c in addrs[addrs.index(b) + 1:addrs.index(b) + 4]:
                x = w.insns[c]
                if x.form == 'cmp' and x.f[0] == j.f[0] and '#' in x.text:
                    why = f'BN4\'s panel type {x.f[1]:#x}'
                    if x.f[1] not in bn4_twins.PANEL4TO6:
                        f.decide('panel', f'{c:08X}: its panel type {x.f[1]:#x} compared, which bn4_twins.PANEL4TO6 gives no BN6 meaning')
                        to = 0xFF
                    elif bn4_twins.PANEL4TO6[x.f[1]] is None:
                        # (a type BN6 has none of: compared with 0xFF, no type of BN6's, the case never holding there)
                        to, why = 0xFF, why + ', none in BN6: compared with no type of BN6\'s'
                    else:
                        to = bn4_twins.PANEL4TO6[x.f[1]]
                        why += f', BN6\'s {to:#x}'
                    f.imms.append(dict(at=c, form='THUMB_CMP', rd=x.f[0], rn=0, was=x.f[1], now=to, why=why))
            return


def scan(f):
    """CurAction, a copied routine's panel type, the element's place, its collision data's offsets"""
    for e, w in sorted(f.walks.items()):
        addrs = sorted(w.insns)
        coll = set()
        for k, a in enumerate(addrs):
            i = w.insns[a]
            curaction(f, w, addrs, k, i)
            panel_type(f, w, addrs, k, i)
            if i.form in FORMS and i.f[1] == 5:
                if i.f[2] == ELEMENT4:
                    f.imms.append(dict(at=a, form=FORMS[i.form], rd=i.f[0], rn=5, was=ELEMENT4, now=ELEMENT6, why='its element: BN4 +0x17, BN6 +0x0E'))
                if i.form == 'ldr' and i.f[2] == 0x54:
                    coll.add(i.f[0])
                    continue
            if i.form in FORMS and i.f[1] in coll:
                to = COLL4TO6.get(i.f[2])
                if to is None:
                    f.decide('collision', f'{a:08X}: its collision data\'s +{i.f[2]:#x}, which bn4.h gives no BN6 place')
                elif to != i.f[2]:
                    f.imms.append(dict(at=a, form=FORMS[i.form], rd=i.f[0], rn=i.f[1], was=i.f[2], now=to,
                                       why=f'collision data +{i.f[2]:#x} in BN4, +{to:#x} in BN6'))
            if i.form == 'bl':
                coll -= {0, 1, 2, 3}
    seen, out = set(), []
    for i in sorted(f.imms, key=lambda x: x['at']):
        if i['at'] not in seen:
            seen.add(i['at'])
            out.append(i)
    f.imms = out


def name_ranges(f):
    """its look-ups of enemies by a NameID range (BN4's 0x080B6568) over its own ids: BN6's ids through a shim"""
    for e, w in f.walks.items():
        for site, t in w.bls.items():
            if t != BN4['byname']:
                continue
            lo, _ = T.back_value(w, site, 1)
            hi, _ = T.back_value(w, site, 2)
            if (lo, hi) != (f.first, f.first + 5):
                f.decide('range', f'{site:08X}: a NameID range {lo}-{hi}, not its own ids')
                continue
            f.ranges.append(dict(at=site, lo=f.first_id, hi=f.first_id + 5, why=f'BN4\'s ids {lo:#x}-{hi:#x}, its own'))


def pair_calls(f, pairs):
    f.pairs = []
    for t, sites in sorted(f.calls.items()):
        tw, how, cls, diff = pairs.twin(t)
        audit = pairs.audit(t, tw) if tw else None
        if tw is None:
            f.decide('call', f'{t:08X} ({len(sites)} BLs, at {", ".join(f"{s:08X}" for s in sites)}): no twin in BN6 ({how})')
        elif cls == 'DIFF' and not audit:
            f.decide('call', f'{t:08X} -> {tw:08X} ({how}): their bodies differ ({diff}), and no audit says the pair serves (bn4_twins.AUDITS)')
        f.pairs.append((t, tw, how, cls, sites, bool(audit)))


def words_of(f):
    for e, w in f.walks.items():
        for lit, v in w.lits.items():
            if v is None or any(lo <= (v & ~1) < hi for lo, hi in f.spans) or not 0x02000000 <= v < 0x0E000000:
                continue
            kind = 'RAM' if v < 0x04000000 else 'I/O' if v < 0x05000000 else 'video' if v < 0x08000000 else 'ROM'
            f.decide('literal', f'{v:08X} ({kind}) at {lit:08X}: no twin known')


def records_of(b4, f):
    st = BN4['stats']
    f.recs = [b4.bytes(st + BN4['stat_size'] * (f.first + v), BN4['stat_size']) for v in range(6)]
    for v, r in enumerate(f.recs):
        if r[2] != f.ty or r[5] != v or r[3] != 4 * f.s1['list'] or r[4] != f.s1['index']:
            f.decide('records', f'enemy id {f.first + v}\'s record names type {r[2]:#x}, sprite {r[3]}:{r[4]}, version {r[5]}')
    elems = {int.from_bytes(r[0:2], 'little') >> 12 for r in f.recs}
    if len(elems) != 1:
        f.decide('element', f'its versions\' elements differ ({sorted(elems)})')
    f.element = min(elems)
    f.notes.append('struct2 from its six records: BN4\'s u16 element << 12 | HP, the version, flags 0, and element << 12 | its attack '
                   '(+0xA) as BN6\'s damage word (the installer makes it)')


def derive(b4, b6, pairs, ty, name, first, slot, alloc):
    f = Family(ty, name, first)
    f.interim, f.ranges = [], []
    f.slot = SLOT0 + slot
    f.ai, f.first_id, f.home = XV_FIRST_AI + f.slot, XV_FIRST_ID + 6 * f.slot, XV_HOMES + XV_HOME_SIZE * f.slot
    f.lo, f.hi = region_of(b4, ty)
    tt, tw, ents = type_table(b4, f)
    if not ents:
        f.decide('type', f'its routine ({f.lo:08X}) names no jump table of three')
        return f
    f.type_table, f.init, f.update = tt, ents[0] & ~1, ents[1] & ~1
    if ents[2] & ~1 != BN4['dying']:
        f.decide('type', f'its dying entry is {ents[2]:08X}, not BN4\'s shared {BN4["dying"]:08X}')
    uw = T.walk(b4, f.update, region=(f.lo, f.hi))
    called = [t for _, t in sorted(uw.bls.items())]
    if called != [BN4['prestep'], BN4['runner'], BN4['present']]:
        f.decide('pre-step', f'its update calls {", ".join(f"{t:08X}" for t in called)}, not BN4\'s pre-step, state runner and present alone')
    site = [s for s, t in uw.bls.items() if t == BN4['runner']]
    f.st = T.back_value(uw, site[0], 0)[0] if site else None
    states = table_at(b4, f.st, f.lo, f.hi, 24) if f.st else []
    for k in range(6):
        if k >= len(states) or states[k] & ~1 != bn4_twins.GENERIC4[k]:
            f.decide('states', f'its state {k} is not BN4\'s generic one')
    f.own = states[6:]
    f.states = [('BN6', GENERIC6[k] | 1) for k in range(8)] + [('OWN', x | 1) for x in f.own]
    f.s1 = struct1_of(b4, f, f.init)
    del f.walks[f.init]
    f.dead = [(f.lo, tw), (f.init, T.walk(b4, f.init, region=(f.lo, f.hi))), (f.update, uw)]
    walk_code(b4, f, [x & ~1 for x in f.own], {f.type_table, f.st})
    name_ranges(f)
    scan(f)
    pair_calls(f, pairs)
    records_of(b4, f)
    blocks_of(b4, f)
    words_of(f)
    art_of(b4, b6, f, slot, alloc)
    effects_of(b4, b6, f, alloc)
    return f


def blocks_of(b4, f):
    """its region, then each copied routine, at its code's place (each the distance from a word its own); each block's bytes
    classed: the code its states reach, their pools and jump tables, the dead (its type's routine, jump table, init and
    update, its state table: BN6's virus code does their work; a copied routine's neighbours) zeroed, the rest raw"""
    spans = [(f.lo, f.hi)]
    for e in f.copied:
        w = f.walks[e]
        hi = max(max(a + i.size for a, i in w.insns.items()), max((l + 4 for l in w.lits), default=0))
        spans.append((e, hi))
    f.spans = spans
    code, pools, tables, dead = set(), set(), set(), set()
    for e, w in f.walks.items():
        for a, i in w.insns.items():
            code.update(range(a, a + i.size))
        for lit in w.lits:
            pools.update(range(lit, lit + 4))
    for v, ents in f.tables.items():
        tables.update(range(v, v + 4 * len(ents)))
    for e, w in f.dead:
        for a, i in w.insns.items():
            dead.update(range(a, a + i.size))
        for lit in w.lits:
            dead.update(range(lit, lit + 4))
    dead.update(range(f.type_table, f.type_table + 12))
    dead.update(range(f.st, f.st + 4 * (6 + len(f.own))))
    f.blocks, f.data, at = [], [], CODE0
    for k, (lo, hi) in enumerate(spans):
        at = rounded(at, CODE_STEP) + (lo & 3)
        f.blocks.append((lo, hi, at, zlib.crc32(b4.bytes(lo, hi - lo)) & 0xFFFFFFFF))
        at += hi - lo
        rest = 'RAW' if k == 0 else 'ZERO'
        kind_at = ['CODE' if a in code else 'POOL' if a in pools else 'TABLE' if a in tables else 'ZERO' if a in dead else rest
                   for a in range(lo, hi)]
        a = lo
        while a < hi:
            kd, b = kind_at[a - lo], a
            while b < hi and kind_at[b - lo] == kd:
                b += 1
            if kd != 'CODE':
                f.data.append((a, b, kd))
            a = b


def art_of(b4, b6, f, slot, alloc):
    """its battle sprite: BN4's list and number, copied whole (decompressed) to its art's place at a list 1 number of its own"""
    p, raw = sprite(b4, BN4['sprites'], f.s1['list'], f.s1['index'])
    if raw is None:
        f.decide('sprite', f'its sprite {f.s1["list"]}:{f.s1["index"]} is not one the generator reads')
        raw = b''
    at = rounded(alloc['art'], ART_STEP)
    f.sprites = [dict(list=f.s1['list'], index=f.s1['index'], to_list=1, to_index=LIST1_0 + slot, at=at,
                      room=rounded(len(raw), ROOM_BATTLE), lz=bool(p & 0x80000000))]
    alloc['art'] = at + f.sprites[0]['room']


_BN6_SPRITES = {}


def bn6_sprite_twin(b4, b6, lst, idx):
    """BN6's sprite of BN4's (list, number)'s bytes in the same list, else None"""
    _, raw = sprite(b4, BN4['sprites'], lst, idx)
    if raw is None:
        return None
    if lst not in _BN6_SPRITES:
        _BN6_SPRITES[lst] = {}
        for k in range(256):
            try:
                _, b = sprite(b6, BN6['sprites'], lst, k)
            except (IndexError, struct_error):
                break
            if b:
                _BN6_SPRITES[lst].setdefault(hashlib.sha1(b).hexdigest(), k)
    return _BN6_SPRITES[lst].get(hashlib.sha1(raw).hexdigest())


def effect_twin(b4, b6, e4):
    """BN6's effect of BN4's effect e4's look: the same sprite (by its bytes), animation and palette, else None"""
    r4 = b4.bytes(BN4['effects'] + 4 * e4, 4)
    tw = bn6_sprite_twin(b4, b6, r4[0] // 4, r4[1])
    if tw is None:
        return None
    for e6 in range(BN6['neffects']):
        q = b6.bytes(BN6['effects'] + 4 * e6, 4)
        if q[0] == r4[0] and q[1] == tw and q[2] == r4[2] and q[3] == r4[3]:
            return e6
    return None


def effects_of(b4, b6, f, alloc):
    """the effects its code spawns (BN4's spawner, BN6's twin, r4's low byte the id): each BN6's effect of the same look, or
    moved: an id of its own past BN6's (EFFECT0 on) whose entry names BN4's sprite, copied to a list 5 number of its own"""
    f.effects, f.effect_ids = [], {}
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != BN4['spawn_effect']:
                continue
            v, chain = T.back_value(w, site, 4)
            if v is None or len(chain) != 1 or chain[0][1] != 'movs' or v >= BN4['neffects']:
                f.decide('effect', f'{site:08X}: an effect id made otherwise ({chain})')
                continue
            e6 = f.effect_ids.get(v)
            if e6 is None:
                e6 = effect_twin(b4, b6, v)
                if e6 is None:
                    e6 = move_effect(b4, f, v, alloc)
                f.effect_ids[v] = e6
            if e6 is not None and e6 != v:
                f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=4, rn=0, was=v, now=e6,
                                   why=f'its effect: {v:#x} in BN4, {e6:#x} in BN6' + ('' if e6 >= BN6['neffects'] else ' (the same look)')))
    f.imms.sort(key=lambda x: x['at'])


def move_effect(b4, f, e4, alloc):
    """BN4's effect e4 at an id of its own, its sprite copied (once a family) at a list 5 number of its own"""
    row = b4.bytes(BN4['effects'] + 4 * e4, 4)
    lst, idx = row[0] // 4, row[1]
    if lst != 5:
        f.decide('effect', f'its effect {e4:#x}\'s sprite {lst}:{idx:#x}: the generator moves list 5\'s alone')
        return None
    if alloc['effect'] >= EFFECT_END:
        f.decide('effect', f'its effect {e4:#x}: BN4\'s effect ids ({EFFECT0:#x}-{EFFECT_END - 1:#x}) all taken')
        return None
    e6 = alloc['effect']
    alloc['effect'] += 1
    f.effects.append((e4, e6))
    if not any((s['list'], s['index']) == (lst, idx) for s in f.sprites):
        p, raw = sprite(b4, BN4['sprites'], lst, idx)
        if raw is None or alloc['list5'] >= LIST5_END:
            f.decide('effect', f'its effect {e4:#x}\'s sprite {lst}:{idx:#x} could not be read or placed')
            return e6
        at = rounded(alloc['art'], ART_STEP)
        f.sprites.append(dict(list=lst, index=idx, to_list=5, to_index=alloc['list5'], at=at,
                              room=rounded(len(raw), ROOM_EFFECT), lz=bool(p & 0x80000000)))
        alloc['list5'] += 1
        alloc['art'] = at + f.sprites[-1]['room']
    return e6


def bn5_ranges_clear():
    """BN5's families' recipes take none of BN4's effect ids nor list 5 numbers (their ranges' top)"""
    with open(BN5_RECIPES, encoding='utf-8') as fh:
        text = fh.read()
    effects = {int(x, 16) for m in re.finditer(r'_effects\[\] = \{([^;]*)\};', text) for x in re.findall(r'0x([0-9A-F]+) \}', m.group(1))}
    list5 = {int(x, 16) for x in re.findall(r'\{ \d, 0x[0-9A-F]+, 5, 0x([0-9A-F]+),', text)}
    clash = sorted(e for e in effects if EFFECT0 <= e < EFFECT_END) + sorted(n for n in list5 if LIST5_0 <= n < LIST5_END)
    if clash:
        sys.exit(f'xrecipe_bn4: BN5\'s families take BN4\'s numbers ({", ".join(f"{x:#x}" for x in clash)}): move EFFECT0 or LIST5_0')


def rounded(n, step):
    return (n + step - 1) // step * step


# ---- the C file and the report ----

HEADER = '''/* xvirus_bn4.c: BN4 Red Sun's virus families' recipes, written by tools/xrecipe_bn4.py from BN4's and BN6's ROMs
 * (python3 tools/xrecipe_bn4.py; --check says whether this is what it writes). Do not edit: change the generator (or
 * tools/bn4_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/multirom/families.md): addresses
 * in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA), counts and CRC32s. The families here are the generator's whole ones
 * (no decision open; its interims named over each), installed only where BN4's ROM is read for research (xrom_research:
 * --dev bn4families); what stays open is in its report (.build/xrecipe/bn4_report.md). */
#include "xvirus_bn4.h"

#include "bn4.h"
#include "bn6.h"
#include "rom.h"

#define OWN(a) { (a), true, 0 }
#define BN6(a) { (a), false, 0 }
'''


def ref_text(ref):
    return 'BN6(BN6_FAMILY_NULLSUB)' if ref == ('BN6', NULLSUB6) else f'{ref[0]}(0x{ref[1]:08X})'


def c_recipe(f):
    n = re.sub(r'[^a-z0-9]', '', f.name.lower())
    L = [f'/* ---- BN4\'s {f.name} (its type {f.ty:#x} there, its enemy ids {f.first}-{f.first + 5}), as AI {f.ai} with ids '
         f'{f.first_id:#x}-{f.first_id + 5:#x} ---- */']
    L += [f'/* interim: {x} */' for x in f.interim]
    L += ['', f'static const XvBlock {n}_blocks[] = {{']
    L += [f'\t{{ 0x{lo:08X}, 0x{hi:08X}, XV_CODE + 0x{at - XV_CODE:X}, 0x{crc:08X}u }},' for lo, hi, at, crc in f.blocks]
    L += ['};', f'static const XvData {n}_data[] = {{']
    L += [f'\t{{ 0x{lo:08X}, 0x{hi:08X}, XV_{k} }},' for lo, hi, k in f.data]
    L += ['};', f'static const XvPair {n}_calls[] = {{']
    for t, tw, how, cls, sites, audited in f.pairs:
        L.append(f'\t{{ 0x{t:08X}, 0x{tw:08X} }},   /* {how}, {cls}{" (audited)" if audited else ""} */')
    L += ['};', f'static const XvImm {n}_imms[] = {{']
    for i in f.imms:
        L.append(f'\t{{ 0x{i["at"]:08X}, {i["form"]}, {i["rd"]}, {i["rn"]}, 0x{i["was"]:X}, 0x{i["now"]:X} }},   /* {i["why"]} */')
    L.append('};')
    if f.ranges:
        L.append(f'static const XvRange {n}_ranges[] = {{')
        L += [f'\t{{ 0x{x["at"]:08X}, 0x{x["lo"]:03X}, 0x{x["hi"]:03X} }},   /* {x["why"]} */' for x in f.ranges]
        L.append('};')
    L += [f'static const XvRef {n}_states[] = {{', '\t' + ', '.join(ref_text(x) for x in f.states) + ',', '};']
    L += [f'static const XvSprite {n}_sprites[] = {{']
    L += [f'\t{{ {s["list"]}, 0x{s["index"]:02X}, {s["to_list"]}, 0x{s["to_index"]:02X}, XV_ART + 0x{s["at"] - XV_ART:X}, 0x{s["room"]:X} }},'
          + ('   /* (LZ77 in BN4) */' if s['lz'] else '') for s in f.sprites]
    L += ['};']
    if f.effects:
        L.append(f'static const XvId {n}_effects[] = {{ ' + ', '.join(f'{{ 0x{a:02X}, 0x{b:02X} }}' for a, b in f.effects) + ' };')
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    L += ['', f'static const XVirusFamily bn4_{n} = {{', f'\t.name = "{f.name} (BN4\'s)",', '\t.game = XROM_BN4_RED_SUN_US,',
          f'\t.src_ai = 0x{f.ty:02X},   /* (its first-kind type: BN4 has no AI index) */', f'\t.ai = {f.ai},',
          f'\t.first_id = XV_FIRST_ID + 0x{f.first_id - XV_FIRST_ID:X},', f'\t.home = XV_HOMES + 0x{f.home - XV_HOMES:X},']
    for field in ('blocks', 'data', 'calls', 'imms') + (('ranges',) if f.ranges else ()) + ('states',):
        L.append(f'\t.{field} = {n}_{field}, .n{field} = {count(field)},')
    L.append('\t.hooks = { ' + ', '.join(['BN6(BN6_FAMILY_NULLSUB)'] * 8) + ' },')
    rec = BN4['stats'] + BN4['stat_size'] * f.first
    L.append(f'\t.struct1 = 0x{rec:08X}, .struct2 = 0x{rec:08X},   /* (its first version\'s enemy record: BN4 has no struct1 or struct2) */')
    L.append('\t.names = { ' + ', '.join(f'0x{f.first + v:02X}' for v in range(6)) + ' },')
    L.append('\t.ice = 0,')
    L.append(f'\t.traits = {{ .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = {"true" if f.s1["shadow"] else "false"}, '
             f'.palette = {f.s1["palette"]} }},')
    L.append(f'\t.sprites = {n}_sprites, .nsprites = {count("sprites")},')
    if f.effects:
        L.append(f'\t.effects = {n}_effects, .neffects = {count("effects")},')
    L.append('};')
    return '\n'.join(L), f'&bn4_{n}'


def c_file(whole):
    parts, refs = [HEADER], []
    for f in whole:
        t, ref = c_recipe(f)
        parts.append(t)
        refs.append(ref)
    parts.append('const XVirusFamily *const xvirus_bn4[] = { ' + ', '.join(refs) + ' };\n'
                 'const int xvirus_bn4_count = (int)(sizeof xvirus_bn4 / sizeof *xvirus_bn4);\n')
    return '\n'.join(parts)


def report(fams, path):
    L = ['# BN4\'s virus families as tools/xrecipe_bn4.py derives them', '']
    for f in fams:
        L += [f'## {f.name} (type {f.ty:#x}, enemy ids {f.first}-{f.first + 5})', '']
        if hasattr(f, 'blocks'):
            L += [f'- region {f.lo:08X}-{f.hi:08X}, its routine\'s jump table {f.type_table:08X} (init {f.init:08X}, update {f.update:08X})',
                  f'- state table {f.st:08X}: its own states {", ".join(f"{x:08X}" for x in f.own)} as BN6\'s 8-{7 + len(f.own)}',
                  f'- struct1: sprite list {f.s1["list"]} number {f.s1["index"]}, palette step {f.s1["palette"]}, shadow {f.s1["shadow"]}, '
                  f'element {f.element}',
                  f'- as AI {f.ai}, ids {f.first_id:#x}-{f.first_id + 5:#x}, home {f.home:08X}',
                  f'- blocks: ' + ', '.join(f'{lo:08X}-{hi:08X} at {at:08X}' for lo, hi, at, _ in f.blocks),
                  f'- {len(f.walks)} routines walked, {len(f.tables)} jump tables, {len(f.imms)} immediates rewritten, '
                  f'{len(f.ranges)} NameID ranges', '', '### Calls out', '']
            for t, tw, how, cls, sites, audited in f.pairs:
                L.append(f'- {t:08X} x{len(sites)}: ' + (f'BN6 {tw:08X} ({how}, {cls}{", audited" if audited else ""})' if tw else f'none ({how})'))
            L += ['', '### Immediates', ''] + [f'- {i["at"]:08X} {i["form"]} {i["was"]:#x} -> {i["now"]:#x}: {i["why"]}' for i in f.imms]
            L += ['', '### Data', ''] + [f'- {a:08X}-{b:08X} {k}' for a, b, k in f.data]
        L += ['', f'### Open decisions ({len(f.decisions)})', ''] + [f'- **{k}**: {d}' for k, d in f.decisions]
        L += ['', f'### Interims ({len(getattr(f, "interim", []))})', ''] + [f'- {x}' for x in getattr(f, 'interim', [])]
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
    bn5_ranges_clear()
    alloc = dict(art=ART0, effect=EFFECT0, list5=LIST5_0)
    fams = [derive(b4, b6, pairs, ty, name, first, k, alloc) for k, (ty, name, first) in enumerate(FAMILIES)]
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
        print(f'{f.name}: {len(f.decisions)} decisions open, {len(getattr(f, "interim", []))} interims, {len(f.imms)} immediates, '
              f'{len(getattr(f, "pairs", []))} calls out')
    print(f'wrote {OUT_C} ({len(whole)} whole) and {os.path.join(WORK, "bn4_report.md")}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
