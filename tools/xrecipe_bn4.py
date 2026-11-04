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
FAMILIES = ((0x13, 'Billy', 13), (0x17, 'Spikey', 25), (0x16, 'Shrimpy', 31), (0x25, 'Walla', 103), (0x2E, 'Elmperor', 109),
            (0x20, 'CirKill', 91))

# ---- BN4's tables (src/emu/bn4.h; docs/rom/bn4.md) ----
BN4 = dict(t1=0x08003A24, nt1=95, stats=0x08011888, stat_size=12, names=0x0874A780,
           prestep=0x080137BC, runner=0x08013C48, present=0x08012D62, dying=0x0801052C,
           sprite_load=0x0800287C, shadow=0x08002E96, no_shadow=0x08002FE2, palette=0x08002DEC,
           byname=0x080B6568, sprites=0x0802793C, effect=0x080E18EE)
NULLSUB6 = 0x0810A953   # BN6_FAMILY_NULLSUB
BX_R0, STATE_STEP6 = 0x08148864, 0x0810A349   # BN6_BX_R0 (its routine's address), BN6_VIRUS_STATE_STEP (Thumb)
# The collision data's offsets BN4's code may name, as BN6's (bn4.h BN4_COLL_*; docs/rom/bn4.md, The battle's objects)
COLL4TO6 = {0x03: 0x04, 0x06: 0x08, 0x08: 0x0A, 0x09: 0x0B, 0x0A: 0x0C, 0x10: 0x1C, 0x18: 0x24, 0x1C: 0x28, 0x28: 0x80, 0x54: 0x70,
            0x64: 0x3C, 0x68: 0x40}
ELEMENT4, ELEMENT6 = 0x17, 0x0E   # an object's element (BN4_T1_ELEMENT), BN6's (BN6_T1_ELEMENT)
# BN4's spawners (bn4.h BN4_SPAWN_*), each BN5's of the same body, and the second-level dispatch's of its kind and place
SPAWNERS = {0x080032B8: (1, 'BN4_SPAWN_1', 'XV_SPAWN_1'), 0x080032D4: (1, 'BN4_SPAWN_1_LAST', 'XV_SPAWN_1_LAST'),
            0x080032F0: (3, 'BN4_SPAWN_3', 'XV_SPAWN_3'), 0x0800330C: (3, 'BN4_SPAWN_3_LAST', 'XV_SPAWN_3_LAST'),
            0x08003328: (3, 'BN4_SPAWN_3_FIRST', 'XV_SPAWN_3_FIRST'), 0x08003344: (4, 'BN4_SPAWN_4', 'XV_SPAWN_4')}
KINDS = {1: (0x08003A24, 95), 3: (0x08003C40, 165), 4: (0x08003F78, 130)}   # (BN4_T1_TYPES, BN4_T3_TYPES, BN4_T4_TYPES)
# The numbers BN4's moved types run as in the second-level dispatch, one after another in FAMILIES' order from these, past
# every number of BN5's of the kind (95, 204, 145 types: the dispatch's tables are by its game's number, BN5's from 0;
# xvirus_recipe.h XV_BN4_TYPES_*), and the list 4 numbers their sprites take (past BN5's families' 0x5C-0x6C and the
# Navis' 0xA0-0xB5, below the chips' 0xEF on)
TYPE_FIRST, TYPE_END = {1: 0x60, 3: 0xD0, 4: 0xA0}, 0x100
LIST4_0, LIST4_END = 0xC0, 0xEF

# The numbers BN4's families take, past BN5's (tools/xrecipe.py's 25 in slots 0-24, their code to XV_CODE + 0x11000 and
# their art to XV_ART + 0x54000): slot 32 on (AI 64, ids 0x2C0), their code and art apart from BN5's, list 1's numbers
# from 0xC0 (past those BN5's families and the moved objects take)
XV_AT = 0x08800000 + 0x330000
XV_HOMES, XV_CODE, XV_ART = XV_AT + 0x18000, XV_AT + 0x38000, XV_AT + 0x50000
XV_HOME_SIZE, XV_FIRST_ID, XV_FIRST_AI = 0x800, 0x200, 32
SLOT0, CODE0, ART0, LIST1_0 = 32, XV_CODE + 0x14000, XV_ART + 0x60000, 0xC0
CODE_STEP, ROOM_BATTLE = 0x10, 0x1000
# each family's code and art after the one before it in FAMILIES (whole or not, so a family's place does not move as
# another's decisions close), to the families' art (XV_ART) and the arm points' records (xvirus_recipe.h XV_ARMS)
CODE_END, ART_END = XV_ART, 0x08800000 + 0x400000 - 256 * 0x44


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
            elif prev[-1].form == 'ldrb' and prev[-1].f == (0, 5, 4):
                s['palette'] = 1   # (its version as it is)
    for k, v in s.items():
        if v is None:
            f.decide('struct1', f'its init ({init:08X}) gives no {k} the generator reads')
    return s


def type_regions(b4):
    """every type routine of the three kinds: (start, end, [(kind, type), ...]), each to the next routine's start"""
    by = {}
    for kind, (base, n) in KINDS.items():
        for t in range(n):
            x = b4.u32(base + 4 * t)
            if T.is_code_ptr(x, b4):
                by.setdefault(x & ~1, []).append((kind, t))
    starts = sorted(by)
    return [(a, starts[k + 1], by[a]) for k, a in enumerate(starts[:-1])]


def object_of(f, t):
    """the moved object (bn4_twins.TYPES) whose type's routine's region holds BN4's address t, made one of the family's
    regions the first time; None where t is in none"""
    for o in f.objects:
        if o['lo'] <= t < o['hi']:
            return o
    for lo, hi, types in f.type_regions:
        if lo <= t < hi:
            for kind, ty in types:
                d = bn4_twins.TYPES.get((kind, ty))
                if d and d[0] == 'moved':
                    o = dict(kind=kind, type=ty, lo=lo, hi=hi, why=d[1], routine=lo, n=None)
                    f.objects.append(o)
                    f.regions.append((lo, hi))
                    return o
            return None
    return None


def region_at(f, e):
    return next(((lo, hi) for lo, hi in f.regions if lo <= e < hi), None)


def walk_code(b4, f, roots, skip):
    """its states and every routine they reach in its region (their jump tables followed), the objects they spawn that
    are moved with it (bn4_twins.TYPES: each a region of its own, walked from its type's routine), and the routines of
    BN4's they call that BN6 has no twin of, copied (bn4_twins.COPIED): each walked, the rest of the calls out noted"""
    todo = list(roots)
    while todo:
        e = todo.pop() & ~1
        if e in f.walks:
            continue
        own = region_at(f, e)
        w = T.walk(b4, e, region=own)
        f.walks[e] = w
        for site, t in w.bls.items():
            if region_at(f, t) or t in bn4_twins.COPIED:
                todo.append(t)
            elif t not in SPAWNERS and t not in f.aim and object_of(f, t):
                todo += [t, object_of(f, t)['routine']]
            else:
                f.calls.setdefault(t, []).append(site)
        for lit, v in w.lits.items():
            if v is None or v in skip or v & 3 or not own or not own[0] <= v < own[1]:
                continue
            ents = table_at(b4, v, *own)
            if (f.lo, f.hi) == own:
                ents = [x for x in ents if f.lo <= (x & ~1) < f.hi]
            if ents:
                # (a moved object's jump table whole: an entry out of its region a routine of BN4's whose twin the words name)
                f.tables[v] = ents
                todo += [x for x in ents if (x & ~1) not in f.walks and region_at(f, x & ~1)]
                for x in ents:
                    if not region_at(f, x & ~1):
                        f.outs.setdefault(x, []).append(v)
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
    listed = (i.form == 'ldrb' and bool(f.ranges) and prev is not None and prev.form == 'ldr' and prev.f[0] == i.f[1]
              and prev.f[2] == 0)
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
                    to = bn4_twins.PANEL4TO6.get(x.f[1])
                    if to is None:
                        to = 0xFF
                        f.interim.append(f'{c:08X}: its panel type {x.f[1]:#x} compared, which no BN6 type is known to be: compared with '
                                         f'0xFF, no type of BN6\'s (the case never holds; bn4_twins.PANEL4TO6 to give it)')
                    f.imms.append(dict(at=c, form='THUMB_CMP', rd=x.f[0], rn=0, was=x.f[1], now=to, why=f'BN4\'s panel type {x.f[1]:#x}'))
            return


def scan(f):
    """CurAction, a copied routine's panel type, the element's place, its collision data's offsets"""
    for e, w in sorted(f.walks.items()):
        addrs = sorted(w.insns)
        coll = set()
        # (a moved object's +9 is its own state's step, not a CurAction)
        mine = not any(o['lo'] <= e < o['hi'] for o in f.objects)
        for k, a in enumerate(addrs):
            i = w.insns[a]
            if mine:
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


def spawns_of(f, alloc):
    """its spawns of BN4's object types (a BL to one of BN4's spawners, r0 the type from a movs): a moved one's call to the
    dispatch's spawner of its kind and place, its number the one it runs as there; its element's store into the object
    spawned (r0 just after, +0x17 as BN6's +0x0E)"""
    f.spawners = {}
    for e, w in sorted(f.walks.items()):
        addrs = sorted(w.insns)
        for k, site in enumerate(addrs):
            t = w.insns[site].target if w.insns[site].form == 'bl' else None
            if t not in SPAWNERS:
                continue
            kind = SPAWNERS[t][0]
            ty, chain = T.back_value(w, site, 0)
            o = next((x for x in f.objects if (x['kind'], x['type']) == (kind, ty)), None)
            if o is None or len(chain) != 1 or chain[0][1] != 'movs':
                f.decide('spawn', f'{site:08X}: its spawn of the {kind} kind\'s type {ty} ({chain}) is none of its moved objects\' '
                                  f'(bn4_twins.TYPES)')
                continue
            if o['n'] is None:
                o['n'] = alloc.setdefault((kind, ty), TYPE_FIRST[kind] + sum(1 for (k2, _) in alloc if k2 == kind))
                if o['n'] >= TYPE_END:
                    f.decide('room', f'its object {kind}:{ty:#x} has no number left in the dispatch')
            f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=ty, now=o['n'],
                               why=f'its object {kind}:{ty:#x}, moved, run as the dispatch\'s {o["n"]:#x}'))
            f.spawners.setdefault(t, []).append(site)
            for b in addrs[k + 1:k + 14]:
                j = w.insns[b]
                if j.form == 'strb' and j.f[1:] == (0, ELEMENT4):
                    f.imms.append(dict(at=b, form='THUMB_STRB', rd=j.f[0], rn=0, was=ELEMENT4, now=ELEMENT6,
                                       why='the element of the object it spawned: BN4 +0x17, BN6 +0x0E'))
                if T.is_return(j):
                    break
    f.imms.sort(key=lambda x: x['at'])


def object_art(b4, f, list4):
    """the sprites its moved objects load (BN4's sprite_load, r1 the list's offset, r2 the number, each a movs): copied to
    list 4's numbers of BN4's (one a sprite whichever family draws it), the number's movs rewritten"""
    f.osprites = []
    for e, w in sorted(f.walks.items()):
        if not any(o['lo'] <= e < o['hi'] for o in f.objects):
            continue
        for site, t in sorted(w.bls.items()):
            if t != BN4['sprite_load']:
                continue
            tab = next((v for v, t in bn4_twins.TABLES.items() if site in t['loads']), None)
            if tab is not None:
                table_art(b4, f, list4, tab)
                continue
            lst, c1 = T.back_value(w, site, 1)
            num, c2 = T.back_value(w, site, 2)
            if lst != 0x10 or len(c2) != 1 or c2[0][1] != 'movs':
                f.decide('sprite', f'{site:08X}: its object loads sprite {lst}:{num} ({c1}, {c2}), not list 4\'s by a movs')
                continue
            to = osprite(b4, f, list4, num)
            if to is None:
                continue
            f.imms.append(dict(at=c2[0][0], form='THUMB_MOVS', rd=2, rn=0, was=num, now=to,
                               why=f'its object\'s sprite, list 4\'s {num:#x} in BN4, {to:#x} copied'))
    f.imms.sort(key=lambda x: x['at'])


def osprite(b4, f, list4, num):
    """list 4's sprite num of BN4's as one of BN4's moved objects' (its list 4 number of BN4's, its copy listed once a family)"""
    p, raw = sprite(b4, BN4['sprites'], 4, num)
    if raw is None:
        f.decide('sprite', f'its object\'s sprite 4:{num:#x} is not one the generator reads')
        return None
    to = list4.setdefault(num, LIST4_0 + len(list4))
    if to >= LIST4_END:
        f.decide('room', f'its object\'s sprite 4:{num:#x} has no list 4 number left')
    if not any(s['index'] == num for s in f.osprites):
        f.osprites.append(dict(list=4, index=num, to_list=4, to_index=to, at=None, room=rounded(len(raw), ROOM_BATTLE),
                               lz=bool(p & 0x80000000)))
    return to


def table_art(b4, f, list4, v):
    """a table its object reads its sprite from (bn4_twins.TABLES): each kind it spawns, its sprite copied and its number's
    byte given the copy's"""
    t = bn4_twins.TABLES[v]
    for k in t['kinds']:
        at = v + t['entry'] * k
        lst, num = b4.u8(at + t['sprite'][0]), b4.u8(at + t['sprite'][1])
        if lst != 0x10:
            f.decide('sprite', f'{at:08X}: its table\'s kind {k} draws list {lst // 4}\'s, not list 4\'s')
            continue
        to = osprite(b4, f, list4, num)
        if to is not None and all(b[0] != at + t['sprite'][1] for b in f.bytes):
            f.bytes.append((at + t['sprite'][1], num, to, f'its table\'s kind {k}: list 4\'s {num:#x} in BN4, {to:#x} copied'))


def pair_calls(f, pairs):
    f.pairs = []
    for t, sites in sorted(f.calls.items()):
        if t in SPAWNERS and t in f.spawners:
            continue
        if t in f.aim:
            f.pairs.append((t, f.aim[t][0], f.aim[t][1], 'AIMED', sites, True))
            continue
        tw, how, cls, diff = pairs.twin(t)
        audit = pairs.audit(t, tw) if tw else None
        if tw is None:
            f.decide('call', f'{t:08X} ({len(sites)} BLs, at {", ".join(f"{s:08X}" for s in sites)}): no twin in BN6 ({how})')
        elif cls == 'DIFF' and not audit:
            f.decide('call', f'{t:08X} -> {tw:08X} ({how}): their bodies differ ({diff}), and no audit says the pair serves (bn4_twins.AUDITS)')
        f.pairs.append((t, tw, how, cls, sites, bool(audit)))


def words_of(f, pairs):
    """its literal words that name RAM, I/O or ROM out of its blocks (a number read by hand, bn4_twins.KEPT, kept as it is),
    and its moved objects' jump tables' entries out of them (a routine of BN4's, its twin)"""
    f.words, f.numbers = [], []
    if f.prestep_word:
        f.numbers.append((f.prestep_word, f.st, STATE_STEP6, 'its pre-step\'s way on: BN6\'s state step, not its state table'))
    for e, w in f.walks.items():
        for lit, v in w.lits.items():
            if v is None or lit == f.prestep_word or any(lo <= (v & ~1) < hi for lo, hi in f.spans) or not 0x02000000 <= v < 0x0E000000:
                continue
            if v in bn4_twins.KEPT:
                if all(x[0] != lit for x in f.numbers):
                    f.numbers.append((lit, v, v, bn4_twins.KEPT[v]))
                continue
            kind = 'RAM' if v < 0x04000000 else 'I/O' if v < 0x05000000 else 'video' if v < 0x08000000 else 'ROM'
            f.decide('literal', f'{v:08X} ({kind}) at {lit:08X}: no twin known')
    for x, tabs in sorted(f.outs.items()):
        tw, how, cls, diff = pairs.twin(x & ~1)
        if tw is None or (cls == 'DIFF' and not pairs.audit(x & ~1, tw)):
            f.decide('table', f'{x:08X} (in the jump table at {", ".join(f"{t:08X}" for t in tabs)}): no twin that serves ({how}, {cls})')
            continue
        f.words.append((x, tw | 1, f'{how}, {cls}'))
    f.numbers.sort()


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


def derive(b4, b6, pairs, ty, name, first, slot, code_at, art_at, alloc, list4):
    f = Family(ty, name, first)
    f.interim, f.ranges, f.objects, f.outs, f.aim, f.prestep = [], [], [], {}, {}, None
    f.spawners, f.osprites, f.words, f.numbers, f.prestep_word, f.bytes = {}, [], [], [], None, []
    f.code_at, f.code_end, f.art_at, f.art_end = code_at, code_at, art_at, art_at
    f.slot = SLOT0 + slot
    f.ai, f.first_id, f.home = XV_FIRST_AI + f.slot, XV_FIRST_ID + 6 * f.slot, XV_HOMES + XV_HOME_SIZE * f.slot
    f.lo, f.hi = region_of(b4, ty)
    f.regions, f.type_regions = [(f.lo, f.hi)], type_regions(b4)
    tt, tw, ents = type_table(b4, f)
    if not ents:
        f.decide('type', f'its routine ({f.lo:08X}) names no jump table of three')
        return f
    f.type_table, f.init, f.update = tt, ents[0] & ~1, ents[1] & ~1
    if ents[2] & ~1 != BN4['dying']:
        f.decide('type', f'its dying entry is {ents[2]:08X}, not BN4\'s shared {BN4["dying"]:08X}')
    uw = T.walk(b4, f.update, region=(f.lo, f.hi))
    called = [t for _, t in sorted(uw.bls.items())]
    engine = (BN4['prestep'], BN4['runner'], BN4['present'])
    site = [s for s, t in uw.bls.items() if t == BN4['runner']]
    f.st = T.back_value(uw, site[0], 0)[0] if site else None
    if called != list(engine):
        # (its update's own work between BN4's pre-step and its state runner: run as its pre-step, BN6's dispatcher jumping
        # there from its NameID cases, after its own pre-step and before the state, where BN4 runs it; its state table's load
        # made BN6's state step's address, its call to the runner a call to a bx r0 of BN6's: on to BN6's state step)
        why = bn4_twins.PRESTEP.get(f.update)
        pre = [s for s, t in uw.bls.items() if t == BN4['prestep']]
        lit = T.back_value(uw, site[0], 0)[1] if site else []
        if (why is None or [t for t in called if t in engine] != list(engine) or called[0] != engine[0] or called[-2:] != list(engine[1:])
                or len(pre) != 1 or len(lit) != 1 or lit[0][1] != 'ldr_lit'):
            f.decide('pre-step', f'its update calls {", ".join(f"{t:08X}" for t in called)}, not BN4\'s pre-step, state runner and present alone')
        else:
            f.prestep = pre[0] + 4
            f.notes.append(f'its pre-step: its update from {f.prestep:08X}, {why}')
            f.aim[BN4['runner']] = (BX_R0, 'its pre-step\'s way on: a bx r0 of BN6\'s, r0 BN6\'s state step')
            f.prestep_word = uw.insns[lit[0][0]].lit
    else:
        f.prestep_word = None
    states = table_at(b4, f.st, f.lo, f.hi, 24) if f.st else []
    f.states = [('BN6', GENERIC6[k] | 1) for k in range(8)]
    walked = []
    for k in range(6):
        if k < len(states) and states[k] & ~1 != bn4_twins.GENERIC4[k] and states[k] & ~1 in bn4_twins.OWN_STATES:
            # (a generic state of its own: a wrapper of BN4's, its call to the generic one paired with BN6's)
            f.states[k] = ('OWN', states[k] | 1)
            walked.append(states[k] & ~1)
        elif k >= len(states) or states[k] & ~1 != bn4_twins.GENERIC4[k]:
            f.decide('states', f'its state {k} is not BN4\'s generic one')
    f.own = states[6:]
    f.states += [('OWN', x | 1) for x in f.own]
    f.s1 = struct1_of(b4, f, f.init)
    del f.walks[f.init]
    f.dead = [(f.lo, tw), (f.init, T.walk(b4, f.init, region=(f.lo, f.hi))), (f.update, uw)]
    walk_code(b4, f, [x & ~1 for x in f.own] + walked + ([f.prestep] if f.prestep else []), {f.type_table, f.st})
    if f.prestep and any(s not in uw.insns for s in f.calls.get(BN4['runner'], [])):
        f.decide('pre-step', f'BN4\'s state runner called out of its update too ({f.calls[BN4["runner"]]}): not to be aimed at a bx r0')
    name_ranges(f)
    scan(f)
    spawns_of(f, alloc)
    object_art(b4, f, list4)
    pair_calls(f, pairs)
    records_of(b4, f)
    blocks_of(b4, f)
    words_of(f, pairs)
    art_of(b4, b6, f, slot)
    effects_of(f)
    f.interim += list(bn4_twins.INTERIMS.get(name, ()))
    return f


def effects_of(f):
    """its effects (BN4's effect spawner, r4 the id): kept as BN6's same ids, an interim while BN4's effect table is not mapped"""
    sites = []
    for e, w in f.walks.items():
        for site, t in w.bls.items():
            if t == BN4['effect']:
                v, _ = T.back_value(w, site, 4)
                sites.append((site, v))
    if sites:
        ids = sorted({v for _, v in sites}, key=lambda v: -1 if v is None else v)
        f.interim.append(f'its effects {" and ".join("?" if v is None else f"{v:#x}" for v in ids)} '
                         f'({", ".join(f"{s:08X}" for s, _ in sorted(sites))}) kept as BN6\'s same ids: BN4\'s effect table is not mapped '
                         f'yet, so their twins are unchecked (effect_twin)')


def blocks_of(b4, f):
    """its region, then each copied routine, at its code's place (each the distance from a word its own); each block's bytes
    classed: the code its states reach, their pools and jump tables, the dead (its type's routine, jump table, init and
    update, its state table: BN6's virus code does their work; a copied routine's neighbours) zeroed, the rest raw"""
    spans = [(f.lo, f.hi)] + [(o['lo'], o['hi']) for o in f.objects]
    named = sorted({v for w in f.walks.values() for v in w.lits.values() if v in bn4_twins.TABLES})
    spans += [(v, v + bn4_twins.TABLES[v]['size']) for v in named]
    copied = []
    for e in f.copied:
        w = f.walks[e]
        hi = max(max(a + i.size for a, i in w.insns.items()), max((l + 4 for l in w.lits), default=0))
        # (a copied routine within another's span, its pool past it: one block, the two routines its code)
        if copied and e < copied[-1][1]:
            copied[-1] = (copied[-1][0], max(hi, copied[-1][1]))
        else:
            copied.append((e, hi))
    spans += copied
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
    f.blocks, f.data, at = [], [], f.code_at
    for k, (lo, hi) in enumerate(spans):
        at = rounded(at, CODE_STEP) + (lo & 3)
        f.blocks.append((lo, hi, at, zlib.crc32(b4.bytes(lo, hi - lo)) & 0xFFFFFFFF))
        at += hi - lo
        rest = 'RAW' if k <= len(f.objects) + len(named) else 'ZERO'
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
    f.code_end = at
    if f.code_end > CODE_END:
        f.decide('room', f'its code ends at {f.code_end:08X}, past the families\' code ({CODE_END:08X})')


def art_of(b4, b6, f, slot):
    """its battle sprite: BN4's list and number, copied whole (decompressed) to its art's place at a list 1 number of its own"""
    p, raw = sprite(b4, BN4['sprites'], f.s1['list'], f.s1['index'])
    if raw is None:
        f.decide('sprite', f'its sprite {f.s1["list"]}:{f.s1["index"]} is not one the generator reads')
        raw = b''
    f.sprite = dict(list=f.s1['list'], index=f.s1['index'], to_list=1, to_index=LIST1_0 + slot, at=f.art_at,
                    room=rounded(len(raw), ROOM_BATTLE), lz=bool(p & 0x80000000))
    f.art_end = f.art_at + f.sprite['room']
    for s in f.osprites:
        s['at'], f.art_end = f.art_end, f.art_end + s['room']
    if f.art_end > ART_END:
        f.decide('room', f'its sprite ends at {f.art_end:08X}, past the families\' art ({ART_END:08X})')


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
    L += [f'\t{{ {SPAWNERS[t][1]}, {SPAWNERS[t][2]} }},   /* the dispatch\'s spawner */' for t in sorted(f.spawners)]
    L += ['};', f'static const XvImm {n}_imms[] = {{']
    for i in f.imms:
        L.append(f'\t{{ 0x{i["at"]:08X}, {i["form"]}, {i["rd"]}, {i["rn"]}, 0x{i["was"]:X}, 0x{i["now"]:X} }},   /* {i["why"]} */')
    L.append('};')
    if f.ranges:
        L.append(f'static const XvRange {n}_ranges[] = {{')
        L += [f'\t{{ 0x{x["at"]:08X}, 0x{x["lo"]:03X}, 0x{x["hi"]:03X} }},   /* {x["why"]} */' for x in f.ranges]
        L.append('};')
    L += [f'static const XvRef {n}_states[] = {{', '\t' + ', '.join(ref_text(x) for x in f.states) + ',', '};']
    if f.words:
        L.append(f'static const XvPair {n}_words[] = {{')
        L += [f'\t{{ 0x{a:08X}, 0x{b:08X} }},   /* {why} */' for a, b, why in f.words]
        L.append('};')
    if f.numbers:
        L.append(f'static const XvNumber {n}_numbers[] = {{')
        L += [f'\t{{ 0x{at:08X}, 0x{was:08X}, 0x{now:08X} }},   /* {why} */' for at, was, now, why in f.numbers]
        L.append('};')
    if f.bytes:
        L.append(f'static const XvByte {n}_bytes[] = {{')
        L += [f'\t{{ 0x{at:08X}, 0x{was:02X}, 0x{now:02X} }},   /* {why} */' for at, was, now, why in f.bytes]
        L.append('};')
    if f.objects:
        L.append(f'static const XvType {n}_types[] = {{')
        L += [f'\t{{ {o["kind"]}, 0x{o["type"]:02X}, XV_TYPE_MOVED, 0x{o["n"]:02X} }},   /* {o["why"]} */' for o in f.objects]
        L.append('};')
    L += [f'static const XvSprite {n}_sprites[] = {{']
    for s in [f.sprite] + f.osprites:
        L.append(f'\t{{ {s["list"]}, 0x{s["index"]:02X}, {s["to_list"]}, 0x{s["to_index"]:02X}, XV_ART + 0x{s["at"] - XV_ART:X}, 0x{s["room"]:X} }},'
                 + ('   /* (LZ77 in BN4) */' if s['lz'] else ''))
    L.append('};')
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    L += ['', f'static const XVirusFamily bn4_{n} = {{', f'\t.name = "{f.name} (BN4\'s)",', '\t.game = XROM_BN4_RED_SUN_US,',
          f'\t.src_ai = 0x{f.ty:02X},   /* (its first-kind type: BN4 has no AI index) */', f'\t.ai = {f.ai},',
          f'\t.first_id = XV_FIRST_ID + 0x{f.first_id - XV_FIRST_ID:X},', f'\t.home = XV_HOMES + 0x{f.home - XV_HOMES:X},']
    fields = ('blocks', 'data', 'calls') + (('words',) if f.words else ()) + ('imms',) + (('ranges',) if f.ranges else ()) + ('states',)
    for field in fields:
        L.append(f'\t.{field} = {n}_{field}, .n{field} = {count(field)},')
    L.append('\t.hooks = { ' + ', '.join(['BN6(BN6_FAMILY_NULLSUB)'] * 8) + ' },')
    rec = BN4['stats'] + BN4['stat_size'] * f.first
    L.append(f'\t.struct1 = 0x{rec:08X}, .struct2 = 0x{rec:08X},   /* (its first version\'s enemy record: BN4 has no struct1 or struct2) */')
    L.append('\t.names = { ' + ', '.join(f'0x{f.first + v:02X}' for v in range(6)) + ' },')
    L.append('\t.ice = 0,')
    L.append(f'\t.traits = {{ .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = {"true" if f.s1["shadow"] else "false"}, '
             f'.palette = {f.s1["palette"]} }},')
    L.append(f'\t.sprites = {n}_sprites, .nsprites = {count("sprites")},')
    if f.objects:
        L.append(f'\t.types = {n}_types, .ntypes = {count("types")},')
    if f.numbers:
        L.append(f'\t.numbers = {n}_numbers, .nnumbers = {count("numbers")},')
    if f.bytes:
        L.append(f'\t.bytes = {n}_bytes, .nbytes = {count("bytes")},')
    if f.prestep:
        L.append(f'\t.prestep = 0x{f.prestep:08X},   /* its update\'s own work on, run as its pre-step (bn4_twins.PRESTEP) */')
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
            L += ['', '### Objects moved', ''] + [f'- {o["kind"]}:{o["type"]:#x} {o["lo"]:08X}-{o["hi"]:08X} as the dispatch\'s '
                                                     f'{o["n"] if o["n"] is None else hex(o["n"])}: {o["why"]}' for o in f.objects]
            L += [f'- sprite 4:{x["index"]:#x} as 4:{x["to_index"]:#x}' for x in f.osprites]
            L += [f'- word {a:08X} -> {b:08X} ({why})' for a, b, why in f.words]
            L += [f'- number {was:08X} at {at:08X} as {now:08X}: {why}' for at, was, now, why in f.numbers]
            L += [f'- byte {was:#x} at {at:08X} as {now:#x}: {why}' for at, was, now, why in f.bytes]
            if f.prestep:
                L.append(f'- pre-step: its update {f.prestep:08X}')
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
    fams, code_at, art_at, alloc, list4 = [], CODE0, ART0, {}, {}
    for k, (ty, name, first) in enumerate(FAMILIES):
        fams.append(derive(b4, b6, pairs, ty, name, first, k, code_at, art_at, alloc, list4))
        code_at, art_at = rounded(fams[-1].code_end, CODE_STEP), fams[-1].art_end
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
