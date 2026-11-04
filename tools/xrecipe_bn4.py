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
number, palette step, shadow) and its records (element), its struct2 from its six enemy records (BN4_ENEMY_STATS), its
flags from its init's collision setup. What its init does past BN6's virus init's work runs as hook 5 (its spawn done),
what its update does past the pre-step, the state runner and the present as hook 3 (each frame): a routine of its own,
or the tail entered past the routine's push (XvRef.entered, bn4_twins.ENTRIES). A third-kind object its code spawns
that BN6 has no type of (its spawner in that type's region, no twin of it) is moved with it: the type's region copied,
its spawn through the second-level dispatch at a number of its own (TYPE_0 on), its sprites copied into BN6's grown
lists.

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
from xrecipe import song_sig, sprite  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_C = os.path.join(ROOT, 'src', 'emu', 'xvirus_bn4.c')
WORK = os.path.join(ROOT, '.build', 'xrecipe')
MATCH = os.path.join(ROOT, '.build', 'symbols', 'match')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
SHA1 = {'bn6': '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6', 'bn4': 'a97e96a7da03abd70f7953328e511b9fa29179f1'}

# BN4's families, in install order: (its first-kind type, the project's name, its first enemy id); each its six
# versions' ids from there (BN4_ENEMY_STATS' records name the type, checked)
FAMILIES = ((0x13, 'Billy', 13), (0x17, 'Spikey', 25), (0x16, 'Shrimpy', 31))

# ---- BN4's tables (src/emu/bn4.h; docs/rom/bn4.md) ----
BN4 = dict(t1=0x08003A24, nt1=95, t3=0x08003C40, nt3=165, stats=0x08011888, stat_size=12, names=0x0874A780,
           prestep=0x080137BC, runner=0x08013C48, present=0x08012D62, dying=0x0801052C,
           sprite_load=0x0800287C, shadow=0x08002E96, no_shadow=0x08002FE2, palette=0x08002DEC,
           byname=0x080B6568, sprites=0x0802793C, songs=0x08155FF8, effect=0x080E18EE, sound=0x08000560,
           create_coll=0x08012496, setup_coll=0x080129AC, set_flag=0x08012F82)
# the calls BN6's virus init (0x08016F56) makes its own of: an init's that are none of these, or of INIT_ENGINE's, are its own
INIT_STD = {BN4['sprite_load'], BN4['shadow'], BN4['no_shadow'], BN4['palette'], BN4['create_coll'], BN4['setup_coll'],
            0x08002E10, 0x0800283C, 0x0800ACA6, 0x08002FAE, 0x080033D4}
# its spawners (bn4.h BN4_SPAWN_*): the dispatch's (XV_SPAWN_*) in their place, each a kind's
SPAWNERS4 = {0x080032B8: (1, 'XV_SPAWN_1'), 0x080032D4: (1, 'XV_SPAWN_1_LAST'), 0x08003360: (1, 'XV_SPAWN_1_LAST'),
             0x080032F0: (3, 'XV_SPAWN_3'), 0x0800330C: (3, 'XV_SPAWN_3_LAST'), 0x08003328: (3, 'XV_SPAWN_3_FIRST'),
             0x08003344: (4, 'XV_SPAWN_4')}
NULLSUB6 = 0x0810A953   # BN6_FAMILY_NULLSUB
# The collision data's offsets BN4's code may name, as BN6's (bn4.h BN4_COLL_*; docs/rom/bn4.md, The battle's objects)
COLL4TO6 = {0x01: 0x01, 0x03: 0x04, 0x07: 0x09, 0x08: 0x0A, 0x09: 0x0B, 0x10: 0x1C, 0x18: 0x24, 0x1C: 0x28, 0x28: 0x80,
            0x54: 0x70, 0x64: 0x3C, 0x68: 0x40}
ELEMENT4, ELEMENT6 = 0x17, 0x0E   # an object's element (BN4_T1_ELEMENT), BN6's (BN6_T1_ELEMENT)

# The numbers BN4's families take, past BN5's (tools/xrecipe.py's 25 in slots 0-24, their code to XV_CODE + 0x11000 and
# their art to XV_ART + 0x54000): slot 32 on (AI 64, ids 0x2C0), their code and art apart from BN5's, one family's after
# another's; list 1's numbers from 0xC0 (past those BN5's families and the moved objects take), list 4's from 0xC0 (past
# BN5's families' 0x5C on and the Navis' 0xA0-0xB5, below the chips' from 0xFF down); the dispatch's numbers of the
# types they move from 0xE0 of a kind's 256 (past every BN5 type's, 204 a kind at most: BN5's own numbers in the tables)
XV_AT = 0x08800000 + 0x330000
XV_HOMES, XV_CODE, XV_ART = XV_AT + 0x18000, XV_AT + 0x38000, XV_AT + 0x50000
XV_HOME_SIZE, XV_FIRST_ID, XV_FIRST_AI = 0x800, 0x200, 32
SLOT0, CODE0, ART0, LIST1_0, LIST4_0, TYPE_0 = 32, XV_CODE + 0x14000, XV_ART + 0x60000, 0xC0, 0xC0, 0xE0
CODE_STEP, ROOM_BATTLE, ROOM_ATTACK = 0x200, 0x1000, 0x800
SONGS6 = 0x08159F48   # BN6's song table (RomLayout.song_table)


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
        self.decisions, self.notes, self.interim, self.ranges = [], [], [], []
        self.walks, self.tables, self.calls, self.imms, self.words = {}, {}, {}, [], {}
        self.regions, self.moved3, self.types, self.numbers, self.attack_sprites = [], {}, [], [], []
        self.hooks = [('BN6', NULLSUB6)] * 8
        self.flags, self.spawn_calls, self.entries = 0, {}, []

    def decide(self, kind, detail):  # noqa: D401
        self.decisions.append((kind, detail))


class Alloc:
    """the families' numbers and room, one family's after another's in install order"""

    def __init__(self):
        self.code, self.art, self.list1, self.list4, self.types = CODE0, ART0, LIST1_0, LIST4_0, {1: TYPE_0, 3: TYPE_0, 4: TYPE_0}


def region_of(b4, ty, table=None, n=None):
    table, n = table or BN4['t1'], n or BN4['nt1']
    types = [b4.u32(table + 4 * i) & ~1 for i in range(n)]
    starts = sorted(set(types))
    a = types[ty]
    i = starts.index(a)
    return a, starts[i + 1]


def t3_of(b4, t):
    """the third-kind type whose code holds `t` (its region: its routine to the next type's): (type, lo, hi), else None"""
    types = [b4.u32(BN4['t3'] + 4 * i) & ~1 for i in range(BN4['nt3'])]
    starts = sorted(set(a for a in types if T.is_code_ptr(a | 1, b4)))
    lo = max((a for a in starts if a <= t), default=None)
    if lo is None or lo == starts[-1] or t - lo > 0x1000:
        return None
    tys = [k for k, a in enumerate(types) if a == lo]
    return (tys[0] if len(tys) == 1 else None), lo, starts[starts.index(lo) + 1]


def own_region(f, a):
    return next((r for r in f.regions if r[0] <= a < r[1]), None)


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
            # (its version, +4, the palette's number: shifted by the step's log, or as it is, a step of 1)
            prev = [w.insns[a] for a in sorted(w.insns) if a < site][-2:]
            if [i.form for i in prev] == ['ldrb', 'lsls'] and prev[0].f[1:] == (5, 4):
                s['palette'] = 1 << prev[1].f[2]
            elif prev and prev[-1].form == 'ldrb' and prev[-1].f == (0, 5, 4):
                s['palette'] = 1
    for k, v in s.items():
        if v is None:
            f.decide('struct1', f'its init ({init:08X}) gives no {k} the generator reads')
    return s


def const_of(w, site, reg):
    """the number `reg` holds at `site`: a literal, or a movs and the lsls after it; (value or None, the instructions that
    make it)"""
    chain = []
    for a in sorted((x for x in w.insns if x < site), reverse=True)[:6]:
        i = w.insns[a]
        if not T.writes(i, reg):
            continue
        chain.append(a)
        if i.form == 'ldr_lit':
            return w.lits.get(i.lit), chain
        if i.form == 'movs':
            v = i.f[1]
            for b in reversed(chain[:-1]):
                j = w.insns[b]
                if j.form != 'lsls' or j.f[:2] != (reg, reg):
                    return None, chain
                v <<= j.f[2]
            return v, chain
        if i.form != 'lsls' or i.f[:2] != (reg, reg):
            return None, chain
    return None, chain


def init_of(f):
    """what its init does that BN6's virus init does by its records (its collision's float and the flags it sets: struct2's
    flags), and what is its own past that work: hook 5, a routine of its own it calls or the tail entered (ENTRIES)"""
    w = f.walks[f.init]
    addrs = sorted(w.insns)
    setup = [s for s, t in w.bls.items() if t == BN4['setup_coll']]
    if len(setup) != 1:
        f.decide('init', f'its init calls the collision setup {len(setup)} times')
        return
    args = [T.back_value(w, setup[0], r)[0] for r in (1, 2, 3)]
    if args not in ([1, 2, 3], [0x10, 2, 3]):
        f.decide('init', f'its collision set up with {args}, which BN6\'s virus init makes of no flags')
    used, after = set(), [a for a in addrs if a > setup[0]]
    store = next((a for a in after if w.insns[a].form == 'str' and w.insns[a].f[1:] == (5, 8)), None)
    for site, t in sorted(w.bls.items()):
        if t == BN4['set_flag']:
            v, chain = const_of(w, site, 0)
            if v not in bn4_twins.INIT_FLAGS or site < setup[0]:
                f.decide('init', f'{site:08X}: its init sets flags {v if v is None else hex(v)}, which BN6\'s virus init sets by none of its flags')
                continue
            f.flags |= bn4_twins.INIT_FLAGS[v]
            used.update(chain + [site])
        elif t in bn4_twins.INIT_ENGINE:
            used.add(site)
            f.notes.append(f'its init\'s call {t:08X} left to BN6\'s virus init: {bn4_twins.INIT_ENGINE[t]}')
    if (args[0] == 0x10) != bool(f.flags & 4):
        f.decide('init', 'its collision floats or it sets flag 0x20 (FLOATSHOE), not both, as BN6\'s virus init does by flag 4')
    tail = [a for a in after if a not in used and store is not None and a < store - 2]
    for site, t in sorted(w.bls.items()):
        # (a call of its own step's is the family's: in its tail; any other BN6's virus init must make something of)
        if t not in INIT_STD and t != f.update and t != BN4['set_flag'] and t not in bn4_twins.INIT_ENGINE and site not in tail:
            f.decide('init', f'{site:08X}: its init calls {t:08X}, which BN6\'s virus init makes nothing of')
    if not tail:
        return
    if any(a > tail[0] for a in used):
        f.decide('init', f'{tail[0]:08X}: its init\'s own step comes before flags it sets (BN6\'s init sets them first)')
    first = w.insns[tail[0]]
    if len(tail) == 1 and first.form == 'bl' and f.lo <= first.target < f.hi:
        f.hooks[2] = ('OWN', first.target | 1)
        f.roots.append(first.target)
    else:
        enter(f, 2, tail[0], 'its init')


def update_of(f, uw):
    """its update past the pre-step, the state runner and the present (BN6's dispatcher runs each): hook 3, the tail
    entered (ENTRIES), where there is one"""
    calls = sorted(uw.bls.items())
    if [t for _, t in calls[:3]] != [BN4['prestep'], BN4['runner'], BN4['present']]:
        f.decide('pre-step', f'its update calls {", ".join(f"{t:08X}" for _, t in calls)}, not BN4\'s pre-step, state runner and present first')
        return
    at = calls[2][0] + 4
    if not T.is_return(uw.insns[at]):
        enter(f, 0, at, 'its update')


def enter(f, k, at, what):
    """its hook 3 + k entered at `at`, past its routine's push {lr} (bn4_twins.ENTRIES)"""
    if at not in bn4_twins.ENTRIES:
        f.decide('hook', f'{at:08X}: {what}\'s own step past the engine\'s work, which no entry of bn4_twins.ENTRIES reads (hook {3 + k})')
        return
    f.hooks[k] = ('ENTERED', at | 1)
    f.roots.append(at)
    f.entries.append((3 + k, at, bn4_twins.ENTRIES[at]))


def walk_code(b4, f, roots, skip, pairs):
    """its states and hooks and every routine they reach in its regions (their jump tables followed), the routines of
    BN4's they call that BN6 has no twin of, copied (bn4_twins.COPIED), and the third-kind types its code spawns that BN6
    has no twin of, moved (their regions walked from their routines): each walked, the rest of the calls out noted"""
    todo = list(roots)
    while todo:
        e = todo.pop() & ~1
        if e in f.walks:
            continue
        reg = own_region(f, e)
        w = T.walk(b4, e, region=reg[:2] if reg else None)
        f.walks[e] = w
        for site, t in w.bls.items():
            if t == f.update or (own_region(f, t) is None and t not in bn4_twins.COPIED and moved3_of(b4, f, t, pairs) is None):
                f.calls.setdefault(t, []).append(site)
            else:
                todo.append(t)
        for lit, v in w.lits.items():
            if v is None or v in skip or v & 3 or not reg or not reg[0] <= v < reg[1]:
                continue
            ents = [x for x in table_at(b4, v, reg[0], reg[1]) if reg[0] <= (x & ~1) < reg[1]]
            if ents:
                f.tables[v] = ents
                todo += [x for x in ents if (x & ~1) not in f.walks]
        todo += [r for r in f.regions_roots if (r & ~1) not in f.walks]
        f.regions_roots = []
    f.copied = sorted(e for e in f.walks if e in bn4_twins.COPIED)


def moved3_of(b4, f, t, pairs):
    """a call of its code into a third-kind type's code BN6 has no twin of: the type moved (its region one of the family's,
    walked from its routine), else None"""
    if t in pairs.twins:
        return None
    hit = t3_of(b4, t)
    if hit is None:
        return None
    ty, lo, hi = hit
    if ty is None:
        f.decide('type', f'{t:08X}: in code more than one third-kind type runs')
        return None
    # (its spawner: the routine called spawns that type through BN4's own spawner, r0 the type)
    w = T.walk(b4, t, region=(lo, hi))
    if not any(s in SPAWNERS4 and T.back_value(w, a, 0)[0] == ty for a, s in w.bls.items()):
        return None
    if ty not in f.moved3:
        f.moved3[ty] = dict(lo=lo, hi=hi, routine=lo)
        f.regions.append((lo, hi, ty))
        f.regions_roots.append(lo)
    return ty


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
    listed = bool(f.ranges) and i.form == 'ldrb' and prev is not None and prev.form == 'ldr' and prev.f[0] == i.f[1] and prev.f[2] == 0
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


def bases(i, coll, objs, spawned):
    """the registers that hold its collision data or an object it spawned, as the instruction leaves them"""
    if i.form == 'bl':
        coll -= {0, 1, 2, 3}
        objs -= {0, 1, 2, 3}
        if i.target == BN4['create_coll']:
            coll.add(0)
        if i.target in SPAWNERS4:
            spawned[0] = True
        return
    if i.form == 'adds3' and i.f[2] == 0:
        for src, dst in ((coll, coll), (objs, objs)):
            (dst.add if i.f[1] in src else dst.discard)(i.f[0])
        if spawned[0] and i.f[1] == 5:
            objs.add(i.f[0])
            spawned[0] = False
        return
    for r in range(8):
        if r != 5 and T.writes(i, r) and not (i.form == 'ldr' and i.f[1:] == (5, 0x54) and i.f[0] == r):
            coll.discard(r)
            objs.discard(r)


def field_imms(f, a, i, coll, objs):
    """an object's element (+0x17, BN6's +0xE: its own, or one it spawned) and its collision data's offsets"""
    if i.form not in FORMS:
        return
    if (i.f[1] == 5 or i.f[1] in objs) and i.f[2] == ELEMENT4:
        who = 'its' if i.f[1] == 5 else 'the object it spawned:'
        f.imms.append(dict(at=a, form=FORMS[i.form], rd=i.f[0], rn=i.f[1], was=ELEMENT4, now=ELEMENT6, why=f'{who} element: BN4 +0x17, BN6 +0x0E'))
    if i.f[1] in coll:
        to = COLL4TO6.get(i.f[2])
        if to is None:
            f.decide('collision', f'{a:08X}: its collision data\'s +{i.f[2]:#x}, which bn4.h gives no BN6 place')
        elif to != i.f[2]:
            f.imms.append(dict(at=a, form=FORMS[i.form], rd=i.f[0], rn=i.f[1], was=i.f[2], now=to,
                               why=f'collision data +{i.f[2]:#x} in BN4, +{to:#x} in BN6'))


def scan(f):
    """CurAction (in its own code: an attack's states are its own), a copied routine's panel type, the element's place, its
    collision data's offsets"""
    for e, w in sorted(f.walks.items()):
        addrs = sorted(w.insns)
        coll, objs, spawned = set(), set(), [False]
        reg = own_region(f, e)
        for k, a in enumerate(addrs):
            i = w.insns[a]
            if not reg or reg[2] is None:
                curaction(f, w, addrs, k, i)
            panel_type(f, w, addrs, k, i)
            field_imms(f, a, i, coll, objs)
            if i.form == 'ldr' and i.f[1:] == (5, 0x54):
                coll.add(i.f[0])
                continue
            bases(i, coll, objs, spawned)
    seen, out = set(), []
    for i in sorted(f.imms, key=lambda x: x['at']):
        if i['at'] not in seen:
            seen.add(i['at'])
            out.append(i)
    f.imms = out


def spawns_of(f, alloc):
    """each spawn of its code (a BN4 spawner, r0 the type): a type it moves, at a number of the dispatch's of its own (the
    immediate that names it changed), its call to the dispatch's spawner"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t not in SPAWNERS4:
                continue
            kind, name = SPAWNERS4[t]
            ty, chain = T.back_value(w, site, 0)
            if kind != 3 or ty not in f.moved3 or len(chain) != 1 or chain[0][1] != 'movs':
                f.decide('type', f'{site:08X}: it spawns a {kind} type {ty if ty is None else hex(ty)} which it does not move')
                continue
            m = f.moved3[ty]
            if 'as' not in m:
                m['as'] = alloc.types[kind]
                alloc.types[kind] += 1
                f.types.append((kind, m['as'], ty))
            f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=ty, now=m['as'],
                               why=f'its object\'s type {ty:#x}: moved, the dispatch\'s {m["as"]:#x}'))
            f.spawn_calls[t] = name


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
        if t in f.spawn_calls:
            f.pairs.append((t, f.spawn_calls[t], 'the dispatch\'s spawner', None, sites, False))
            continue
        if t == f.update:
            f.pairs.append((t, NULLSUB6 & ~1, 'its update: BN6\'s dispatcher runs its state and present, its own step is hook 3',
                            None, sites, False))
            continue
        if t in SPAWNERS4:
            continue   # (a spawn it does not move: decided in spawns_of)
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
            if v in bn4_twins.NUMBERS:
                if lit not in [x[0] for x in f.numbers]:
                    f.numbers.append((lit, v, bn4_twins.NUMBERS[v]))
                continue
            kind = 'RAM' if v < 0x04000000 else 'I/O' if v < 0x05000000 else 'video' if v < 0x08000000 else 'ROM'
            f.decide('literal', f'{v:08X} ({kind}) at {lit:08X}: no twin known')
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
    f.notes.append('struct2 from its six records: BN4\'s u16 element << 12 | HP, the version, its init\'s flags, and element << 12 | its '
                   'attack (+0xA) as BN6\'s damage word (the installer makes it)')


def derive(b4, b6, pairs, ty, name, first, slot, alloc):
    f = Family(ty, name, first)
    f.slot = SLOT0 + slot
    f.ai, f.first_id, f.home = XV_FIRST_AI + f.slot, XV_FIRST_ID + 6 * f.slot, XV_HOMES + XV_HOME_SIZE * f.slot
    f.lo, f.hi = region_of(b4, ty)
    f.regions, f.roots, f.regions_roots = [(f.lo, f.hi, None)], [], []
    tt, tw, ents = type_table(b4, f)
    if not ents:
        f.decide('type', f'its routine ({f.lo:08X}) names no jump table of three')
        return f
    f.type_table, f.init, f.update = tt, ents[0] & ~1, ents[1] & ~1
    if ents[2] & ~1 != BN4['dying']:
        f.decide('type', f'its dying entry is {ents[2]:08X}, not BN4\'s shared {BN4["dying"]:08X}')
    uw = T.walk(b4, f.update, region=(f.lo, f.hi))
    site = [s for s, t in uw.bls.items() if t == BN4['runner']]
    f.st = T.back_value(uw, site[0], 0)[0] if site else None
    states = table_at(b4, f.st, f.lo, f.hi, 24) if f.st else []
    for k in range(6):
        if k >= len(states) or states[k] & ~1 != bn4_twins.GENERIC4[k]:
            f.decide('states', f'its state {k} is not BN4\'s generic one')
    f.own = states[6:]
    f.states = [('BN6', GENERIC6[k] | 1) for k in range(8)] + [('OWN', x | 1) for x in f.own]
    f.s1 = struct1_of(b4, f, f.init)
    init_of(f)
    update_of(f, uw)
    del f.walks[f.init]
    f.dead = [(f.lo, tw), (f.init, T.walk(b4, f.init, region=(f.lo, f.hi))), (f.update, uw)]
    walk_code(b4, f, [x & ~1 for x in f.own] + f.roots, {f.type_table, f.st}, pairs)
    name_ranges(f)
    scan(f)
    spawns_of(f, alloc)
    pair_calls(f, pairs)
    records_of(b4, f)
    blocks_of(b4, f, alloc)
    words_of(f)
    art_of(b4, b6, f, alloc)
    effects_of(f)
    sounds_of(b4, b6, f)
    f.imms.sort(key=lambda x: x['at'])
    return f


def effects_of(f):
    """the effects its code spawns (BN4_SPAWN_EFFECT, r4 the id): kept as BN6's same ids, an interim while BN4's effect table
    is not mapped"""
    ids = {}
    for e, w in f.walks.items():
        for site, t in w.bls.items():
            if t == BN4['effect']:
                v, _ = T.back_value(w, site, 4)
                ids.setdefault(v, []).append(site)
    if None in ids:
        f.decide('effect', f'an effect spawned at {", ".join(f"{s:08X}" for s in ids[None])} by an id made otherwise')
    named = sorted((v, sorted(s, reverse=True)) for v, s in ids.items() if v is not None)
    if named:
        f.interim.append(f'its effects {" and ".join(f"{v:#x}" for v, _ in named)} ({", ".join(f"{s:08X}" for _, ss in named for s in ss)}) kept '
                         f'as BN6\'s same ids: BN4\'s effect table is not mapped yet, so their twins are unchecked (effect_twin)')


def sounds_of(b4, b6, f):
    """the sounds its code plays (BN4_PLAY_SOUND, r0 the sound): each BN6's of the same number where it is the same song
    (how it sounds, xrecipe.song_sig); else kept as BN6's same number, an interim while BN4's sounds have no slots of
    BN6's empty songs (xsong.c's allocations)"""
    ids = {}
    for e, w in f.walks.items():
        for site, t in w.bls.items():
            if t == BN4['sound']:
                v, _ = T.back_value(w, site, 0)
                ids.setdefault(v, []).append(site)
    if None in ids:
        f.decide('sound', f'a sound played at {", ".join(f"{s:08X}" for s in ids[None])} made otherwise')
    odd = sorted(v for v in ids if v is not None and song_sig(b4, BN4['songs'], v) != song_sig(b6, SONGS6, v))
    same = sorted(v for v in ids if v is not None and v not in odd)
    if same:
        f.notes.append(f'its sounds {", ".join(f"{v:#x}" for v in same)}: BN6\'s of the same numbers, the same songs')
    if odd:
        f.interim.append(f'its sounds {" and ".join(f"{v:#x}" for v in odd)} ({", ".join(f"{s:08X}" for v in odd for s in sorted(ids[v]))}) '
                         f'kept as BN6\'s same numbers, other songs: BN4\'s sounds have no slots among BN6\'s empty songs yet '
                         f'(src/audio/xsong.c\'s allocations), where a copy of each would go')


def blocks_of(b4, f, alloc):
    """its region, each type it moves and each copied routine, at its code's place (each the distance from a word its own);
    each block's bytes classed: the code its states reach, their pools and jump tables, the dead (its type's routine, jump
    table, init and update but for a tail entered, its state table: BN6's virus code does their work; a copied routine's
    neighbours) zeroed, the rest raw, a word of it that points into a block a pool's"""
    spans = [(r[0], r[1]) for r in f.regions]
    for e in f.copied:
        w = f.walks[e]
        hi = max(max(a + i.size for a, i in w.insns.items()), max((l + 4 for l in w.lits), default=0))
        if e in bn4_twins.COPIED_DATA:
            hi = max(hi, bn4_twins.COPIED_DATA[e][1])
        spans.append((e, hi))
    f.spans = spans
    code, pools, tables, dead, raw = set(), set(), set(), set(), set()
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
    for e, (lo, hi, _) in bn4_twins.COPIED_DATA.items():
        if e in f.copied:
            raw.update(range(lo, hi))
    f.blocks, f.data, at = [], [], alloc.code
    for k, (lo, hi) in enumerate(spans):
        at = rounded(at, CODE_STEP) + (lo & 3)
        f.blocks.append((lo, hi, at, zlib.crc32(b4.bytes(lo, hi - lo)) & 0xFFFFFFFF))
        at += hi - lo
        rest = 'RAW' if k < len(f.regions) else 'ZERO'
        kind_at = ['CODE' if a in code else 'POOL' if a in pools else 'TABLE' if a in tables else 'ZERO' if a in dead else
                   'RAW' if a in raw else rest for a in range(lo, hi)]
        pointers(b4, f, lo, hi, kind_at)
        # (a range of data a word points into begins where it points: the installer moves a pointer to code, to a range's
        # start or into raw bytes)
        starts = {b4.u32(x) & ~1 for x in range(lo & ~3, hi - 3, 4) if kind_at[x - lo] == 'POOL'} | {lo}
        a = lo
        while a < hi:
            kd, b = kind_at[a - lo], a + 1
            while b < hi and kind_at[b - lo] == kd and (b not in starts or kd == 'RAW'):
                b += 1
            if kd != 'CODE':
                f.data.append((a, b, kd))
            a = b
    alloc.code = at


def pointers(b4, f, lo, hi, kind_at):
    """a word of its raw data that points into its blocks (a table of pointers to byte lists): a pool's, moved with them"""
    for a in range((lo + 3) & ~3, hi - 3, 4):
        if all(kind_at[a - lo + k] == 'RAW' for k in range(4)) and any(s <= (b4.u32(a) & ~1) < e for s, e in f.spans):
            for k in range(4):
                kind_at[a - lo + k] = 'POOL'


def art_of(b4, b6, f, alloc):
    """its battle sprite (BN4's list and number, copied whole, decompressed, to its art's place at a list 1 number of its
    own), then each sprite the types it moves load (list 4: copied likewise at a number of its own, the immediate that
    names it changed)"""
    p, raw = sprite(b4, BN4['sprites'], f.s1['list'], f.s1['index'])
    if raw is None:
        f.decide('sprite', f'its sprite {f.s1["list"]}:{f.s1["index"]} is not one the generator reads')
        raw = b''
    room = rounded(len(raw), ROOM_BATTLE)
    f.sprite = dict(list=f.s1['list'], index=f.s1['index'], to_list=1, to_index=alloc.list1, at=alloc.art, room=room, lz=bool(p & 0x80000000))
    alloc.list1 += 1
    alloc.art += room
    for e, w in sorted(f.walks.items()):
        reg = own_region(f, e)
        if not reg or reg[2] is None:
            continue
        for site, t in sorted(w.bls.items()):
            if t == BN4['sprite_load']:
                attack_sprite(b4, f, alloc, w, site)


def attack_sprite(b4, f, alloc, w, site):
    lst, _ = T.back_value(w, site, 1)
    num, chain = T.back_value(w, site, 2)
    if lst != 0x10 or num is None or len(chain) != 1 or chain[0][1] != 'movs':
        f.decide('sprite', f'{site:08X}: a moved object\'s sprite {lst}:{num}, which the generator moves none of (list 4 alone, by a movs)')
        return
    p, raw = sprite(b4, BN4['sprites'], 4, num)
    if raw is None:
        f.decide('sprite', f'{site:08X}: its sprite 4:{num:#x} is not one the generator reads')
        return
    s = next((x for x in f.attack_sprites if x['index'] == num), None)
    if s is None:
        room = rounded(len(raw), ROOM_ATTACK)
        s = dict(list=4, index=num, to_list=4, to_index=alloc.list4, at=alloc.art, room=room, lz=bool(p & 0x80000000))
        f.attack_sprites.append(s)
        alloc.list4 += 1
        alloc.art += room
    f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=2, rn=0, was=num, now=s['to_index'],
                       why=f'its object\'s sprite: list 4 {num:#x} in BN4, list 4 {s["to_index"]:#x} in BN6 (copied)'))


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
#define ENTERED(a) { (a), true, 0, true }
'''


def ref_text(ref):
    return 'BN6(BN6_FAMILY_NULLSUB)' if ref == ('BN6', NULLSUB6) else f'{ref[0]}(0x{ref[1]:08X})'


def pair_text(t, tw, how, cls, audited):
    if isinstance(tw, str):
        return f'\t{{ 0x{t:08X}, {tw} }},   /* {how} */'
    if cls is None:
        return f'\t{{ 0x{t:08X}, 0x{tw:08X} }},   /* {how} */'
    return f'\t{{ 0x{t:08X}, 0x{tw:08X} }},   /* {how}, {cls}{" (audited)" if audited else ""} */'


def sprite_text(s):
    return (f'\t{{ {s["list"]}, 0x{s["index"]:02X}, {s["to_list"]}, 0x{s["to_index"]:02X}, XV_ART + 0x{s["at"] - XV_ART:X}, 0x{s["room"]:X} }},'
            + ('   /* (LZ77 in BN4) */' if s['lz'] else ''))


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
    L += [pair_text(t, tw, how, cls, audited) for t, tw, how, cls, sites, audited in f.pairs]
    L += ['};', f'static const XvImm {n}_imms[] = {{']
    for i in f.imms:
        L.append(f'\t{{ 0x{i["at"]:08X}, {i["form"]}, {i["rd"]}, {i["rn"]}, 0x{i["was"]:X}, 0x{i["now"]:X} }},   /* {i["why"]} */')
    L.append('};')
    if f.ranges:
        L.append(f'static const XvRange {n}_ranges[] = {{')
        L += [f'\t{{ 0x{x["at"]:08X}, 0x{x["lo"]:03X}, 0x{x["hi"]:03X} }},   /* {x["why"]} */' for x in f.ranges]
        L.append('};')
    if f.numbers:
        L.append(f'static const XvNumber {n}_numbers[] = {{')
        L += [f'\t{{ 0x{lit:08X}, 0x{v:08X}, 0x{v:08X} }},   /* {why} */' for lit, v, why in f.numbers]
        L.append('};')
    if f.types:
        L.append(f'static const XvType {n}_types[] = {{')
        L += [f'\t{{ {k}, 0x{a:02X}, XV_TYPE_MOVED, 0x{t:02X} }},   /* BN4\'s third-kind {t:#x}, moved, the dispatch\'s {a:#x} */'
              for k, a, t in f.types]
        L.append('};')
    L += [f'static const XvRef {n}_states[] = {{', '\t' + ', '.join(ref_text(x) for x in f.states) + ',', '};']
    L += [f'static const XvSprite {n}_sprites[] = {{'] + [sprite_text(s) for s in [f.sprite] + f.attack_sprites] + ['};']
    L += c_family(f, n)
    return '\n'.join(L), f'&bn4_{n}'


def c_family(f, n):
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    L = ['', f'static const XVirusFamily bn4_{n} = {{', f'\t.name = "{f.name} (BN4\'s)",', '\t.game = XROM_BN4_RED_SUN_US,',
         f'\t.src_ai = 0x{f.ty:02X},   /* (its first-kind type: BN4 has no AI index) */', f'\t.ai = {f.ai},',
         f'\t.first_id = XV_FIRST_ID + 0x{f.first_id - XV_FIRST_ID:X},', f'\t.home = XV_HOMES + 0x{f.home - XV_HOMES:X},']
    for field, there in (('blocks', True), ('data', True), ('calls', True), ('imms', True), ('ranges', f.ranges),
                         ('numbers', f.numbers), ('types', f.types), ('states', True)):
        if there:
            L.append(f'\t.{field} = {n}_{field}, .n{field} = {count(field)},')
    L.append('\t.hooks = { ' + ', '.join(ref_text(h) for h in f.hooks) + ' },')
    L += [f'\t/* hook {k}: {why} */' for k, at, why in sorted(f.entries)]
    rec = BN4['stats'] + BN4['stat_size'] * f.first
    L.append(f'\t.struct1 = 0x{rec:08X}, .struct2 = 0x{rec:08X},   /* (its first version\'s enemy record: BN4 has no struct1 or struct2) */')
    L.append('\t.names = { ' + ', '.join(f'0x{f.first + v:02X}' for v in range(6)) + ' },')
    L.append('\t.ice = 0,')
    L.append(f'\t.traits = {{ .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = {"true" if f.s1["shadow"] else "false"}, '
             f'.palette = {f.s1["palette"]}, .flags = 0x{f.flags:02X} }},')
    L.append(f'\t.sprites = {n}_sprites, .nsprites = {count("sprites")},')
    L.append('};')
    return L


def c_file(whole):
    parts, refs = [HEADER], []
    for f in whole:
        t, ref = c_recipe(f)
        parts.append(t)
        refs.append(ref)
    parts.append('const XVirusFamily *const xvirus_bn4[] = { ' + ', '.join(refs) + ' };\n'
                 'const int xvirus_bn4_count = (int)(sizeof xvirus_bn4 / sizeof *xvirus_bn4);\n')
    return '\n'.join(parts)


def report_family(f):
    L = [f'- region {f.lo:08X}-{f.hi:08X}, its routine\'s jump table {f.type_table:08X} (init {f.init:08X}, update {f.update:08X})',
         f'- state table {f.st:08X}: its own states {", ".join(f"{x:08X}" for x in f.own)} as BN6\'s 8-{7 + len(f.own)}',
         f'- struct1: sprite list {f.s1["list"]} number {f.s1["index"]}, palette step {f.s1["palette"]}, shadow {f.s1["shadow"]}, '
         f'element {f.element}; struct2\'s flags {f.flags:#x}',
         f'- hooks: ' + ', '.join(f'{3 + k} {ref_text(h)}' for k, h in enumerate(f.hooks) if h != ('BN6', NULLSUB6)),
         f'- as AI {f.ai}, ids {f.first_id:#x}-{f.first_id + 5:#x}, home {f.home:08X}',
         f'- moved types: ' + ', '.join(f'3:{t:#x} as {a:#x} ({f.moved3[t]["lo"]:08X}-{f.moved3[t]["hi"]:08X})' for k, a, t in f.types),
         f'- blocks: ' + ', '.join(f'{lo:08X}-{hi:08X} at {at:08X}' for lo, hi, at, _ in f.blocks),
         f'- sprites: ' + ', '.join(f'{s["list"]}:{s["index"]:#x} as {s["to_list"]}:{s["to_index"]:#x} at {s["at"]:08X}'
                                    for s in [f.sprite] + f.attack_sprites),
         f'- {len(f.walks)} routines walked, {len(f.tables)} jump tables, {len(f.imms)} immediates rewritten, '
         f'{len(f.ranges)} NameID ranges', '', '### Calls out', '']
    for t, tw, how, cls, sites, audited in f.pairs:
        L.append(f'- {t:08X} x{len(sites)}: ' + (f'{tw} ({how})' if isinstance(tw, str) else f'BN6 {tw:08X} ({how})' if cls is None and tw else
                                               f'BN6 {tw:08X} ({how}, {cls}{", audited" if audited else ""})' if tw else f'none ({how})'))
    L += ['', '### Immediates', ''] + [f'- {i["at"]:08X} {i["form"]} {i["was"]:#x} -> {i["now"]:#x}: {i["why"]}' for i in f.imms]
    L += ['', '### Data', ''] + [f'- {a:08X}-{b:08X} {k}' for a, b, k in f.data]
    return L


def report(fams, path):
    L = ['# BN4\'s virus families as tools/xrecipe_bn4.py derives them', '']
    for f in fams:
        L += [f'## {f.name} (type {f.ty:#x}, enemy ids {f.first}-{f.first + 5})', '']
        if hasattr(f, 'blocks'):
            L += report_family(f)
        L += ['', f'### Open decisions ({len(f.decisions)})', ''] + [f'- **{k}**: {d}' for k, d in f.decisions]
        L += ['', f'### Interims ({len(f.interim)})', ''] + [f'- {x}' for x in f.interim]
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
    alloc = Alloc()
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
        print(f'{f.name}: {len(f.decisions)} decisions open, {len(f.interim)} interims, {len(f.imms)} immediates, '
              f'{len(getattr(f, "pairs", []))} calls out')
    print(f'wrote {OUT_C} ({len(whole)} whole) and {os.path.join(WORK, "bn4_report.md")}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
