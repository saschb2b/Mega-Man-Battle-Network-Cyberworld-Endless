#!/usr/bin/env python3
"""xchiprecipe.py [--check] [--report] [--unit NAME ...]: the code of BN5's chips moved into BN6's battle engine, as recipes
made from the two ROMs (issue #82's C3; docs/MULTIROM.md, The chips' code; docs/DEVTOOLS.md, The chip recipe generator).

A chip of BN5's runs by its record's family (its attack family's routine, the CurAction its user takes) or, for a support or
a Navi chip, by a routine its family calls by its subfamily. Where BN6 has no code of a chip's, this moves BN5's: a unit is
one such family or routine, its code every routine it reaches that BN6 has no twin of (walked instruction by instruction,
tools/xrecipe_thumb.py), its calls out paired with BN6's twins (tools/xrecipe_pairs.py, the project's pairs and audits in
tools/bn5_twins.py), the objects it spawns BN6 has no twin of moved with it through the second-level dispatch, its
literals, immediates and data each in BN6's terms, as tools/xrecipe.py does for a virus family (its Deriver: the same
walk, the same checks). Each unit takes a number of BN6's: a family of the grown attack families' table (0x60 on,
src/emu/xattack_grow.h), or an entry of the grown support or Navi routines' tables (src/emu/xchips_code.h).

  python3 tools/xchiprecipe.py           write src/emu/xchips_bn5.c (the units' recipes) and src/core/xchips_moved.c (the chips
                                         that run them: BN6's id, family and subfamily), the units whole (no decision open)
  python3 tools/xchiprecipe.py --check   write nothing: fail where the two files are not what it would write
  python3 tools/xchiprecipe.py --report  every unit of UNITS, CANDIDATES and --unit, what it takes and what needs a decision,
                                         into .build/xchiprecipe/report.md

Local only: both ROMs. Nothing of either is written: the C files hold addresses, counts and CRC32s."""
import argparse
import os
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import xrecipe as X  # noqa: E402
import xrecipe_thumb as T  # noqa: E402
from xrecipe_family import BN5, BN6, Deriver, Family, kind_name  # noqa: E402
from xrecipe_pairs import Pairs  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_C = os.path.join(ROOT, 'src', 'emu', 'xchips_bn5.c')
OUT_MOVED = os.path.join(ROOT, 'src', 'core', 'xchips_moved.c')
WORK = os.path.join(ROOT, '.build', 'xchiprecipe')
REGISTRY = os.path.join(ROOT, 'src', 'core', 'xchips_registry.c')

# ---- each game's chip tables (src/emu/bn5.h, bn6_chips.h; docs/ROM_DATA.md, The chips' code) ----

BN5_FAMILY_TABLE, BN5_FAMILY_COUNT = 0x080EB514, 73           # its attack families 0x10-0x58 (RunAIAttack's, 0x0801857A)
BN5_SUPPORT_TABLE, BN5_NAVI_TABLE = 0x080297BC, 0x080298C8     # its support and Navi chips' routines by subfamily (0x080EC450, 0x080E1AC0's)
BN6_FAMILY_TABLE, BN6_FAMILY_COUNT = 0x080EBFA0, 79
BN5_RECORDS, BN6_RECORDS = 0x0801E210, 0x08021DA8
BN5_SUPPORT_FAMILY, BN5_NAVI_FAMILY = 0x15, 0x41
BN6_SUPPORT_FAMILY, BN6_NAVI_FAMILY = 0x15, 0x1B
BN5_SET_ATTACK = 0x0800F200                                     # setAttack0-6 (r0 the CurAction), 4 bytes apart

# Where moved code may come from (copied where BN6 has no twin): BN5's objects' code, its chips' families and routines,
# its virus code shared by families, and the engine's helpers the units call that BN6 has no twin of (each its why)
COPY_RANGES = ((0x080B0000, 0x08104AC0), (0x080E0000, 0x080F2000))
COPY_HELPERS = {}
# Routines copied though BN6 has a twin of the same body, and object types moved though BN6's runs the same code: where the
# data they read is BN6's own and another (each found by the chips' differential, tests/xdiff)
FORCE_COPY = {
    0x080C7614: "the shockwave's spawner (Guard's, the reflected wave): BN6's twin 0x080C8554 spawns BN6's type 0x16",
}
FORCE_MOVED = {
    (3, 0x16): "the shockwave: BN6's type 0x16 runs the same code but reads its kinds from its own table (BN6 0x080C8370), BN5's "
               "(0x080C7448) another: speed and reach by kind (chip_guard1_reflect: BN6's wave a panel ahead from its first)",
}
# Object types BN6's own serves though its code is not BN5's to its depth: the engine's objects every attack of both games
# spawns, their differences the engine's (each audited; docs/ROM_DATA.md, The chips' code)
TYPE_AUDITS = {
    (3, 0x03): "the engine's hit area (an attack's damage on a panel for its frames; spawned by BN5 0x080C4EAE, BN6 0x080C6C16): "
               "the same states, its collision data's hit word read at each game's own place (BN5 +0x68 at 0x080C4E42, BN6 +0x70 "
               "at 0x080C6BAA) and BN6's end state freeing its panel's reservation first (0x08016C8C calls 0x0801BB78), as BN6 "
               "does for every object",
    (4, 0x00): "the engine's effect (an effect id's sprite and animation, spawned by BN5 0x080DFFD2, BN6 0x080E1932): the same "
               "states, BN6's but for a call each frame (0x080E1952) that copies its owner's visibility where its second spawner "
               "(0x080E194A) set +0x60, which its first leaves clear; its id read from each game's own table (each effect a unit "
               "spawns compared, a decision where the entries differ)",
}

# ---- the places (src/emu/free_space.h, src/emu/xchips_code.h) ----

EMU_FREE = 0x08800000
FREE_CHIP_CODE = EMU_FREE + 0x600000
XC_AT = FREE_CHIP_CODE + 0x400            # FREE_CHIP_MOVED
XC_HOMES = XC_AT + 0x1000                 # a unit's veneers, XC_HOME_SIZE each
XC_HOME_SIZE, XC_UNITS = 0x400, 64
XC_CODE = XC_HOMES + XC_HOME_SIZE * XC_UNITS
XC_ART = FREE_CHIP_CODE + 0xA0000
XC_END = FREE_CHIP_CODE + 0x100000
CODE_STEP = 0x10
# Each unit's number in BN6, fixed by BN5's (the static model: the same at every boot and in every build, a state saved
# mid-attack names it): BN5's attack family F is BN6's 0x50 + F (0x60-0xA8, within src/emu/xattack_grow.h's XATTACK_CHIPS),
# BN5's support or Navi subfamily S the grown table's entry past BN6's own routines + S
FAMILY_BASE = 0x50
FIRST_SUPPORT, FIRST_NAVI = 0x2A, 0x1D     # (BN6_CHIP_ROUTINE_COUNT, BN6_NAVI_ROUTINE_COUNT)


def number_of(kind, n):
    return FAMILY_BASE + n if kind == 'attack' else (FIRST_SUPPORT if kind == 'support' else FIRST_NAVI) + n

# The units moved, in install order: (kind, BN5's family or subfamily, the project's name), each
# its number by BN5's (number_of); each whole (no decision open) and its chips' specs passing (tests/xdiff/chip_*.json).
UNITS = (
    ('attack', 0x1B, 'CrakOut'),
    ('attack', 0x58, 'HotBody'),
    ('support', 0x13, 'Jealousy'),
    ('support', 0x15, 'NrthWind'),
)
# The next units, derived for the report alone (--report) until what each needs is in: its decisions there, and the
# chips' differential's finding where a spec ran it (docs/MULTIROM.md, The chips' code: what remains)
CANDIDATES = (
    ('attack', 0x47, 'AqWhirl'),
    ('support', 0x06, 'MetaGel'),
    ('support', 0x08, 'Blinder'),
    ('support', 0x20, 'BlakWing'),
    ('support', 0x23, 'HolyDrem'),
    ('support', 0x3A, 'BoyBomb'),
    ('support', 0x3D, 'RainyDay'),
)


# ---- the registry's rows: which chips a unit plays ----

def registry_rows():
    import re
    with open(REGISTRY, encoding='utf-8') as f:
        text = f.read()
    rows = []
    for a, b, how in re.findall(r'^\s*\{\s*(\d+),\s*(\d+),\s*XREG_(\w+)', text, re.M):
        rows.append((int(a), int(b), how))
    return rows


def record(rom, base, i):
    return rom.bytes(base + 0x2C * i, 0x2C)


# ---- a unit ----

class Unit:
    pass


def entry_of(b5, kind, n):
    if kind == 'attack':
        return b5.u32(BN5_FAMILY_TABLE + 4 * (n - 0x10)) & ~1
    table = BN5_SUPPORT_TABLE if kind == 'support' else BN5_NAVI_TABLE
    return b5.u32(table + 4 * n) & ~1


class ChipDeriver(Deriver):
    """the family deriver's walk and checks, for a chip's code: no family region, every routine without a twin in BN5's
    objects', chips' and virus code copied (COPY_RANGES), or one of COPY_HELPERS"""

    def kind_of(self, f, t):
        t &= ~1
        if t in f.own:
            return 'own', None
        if t in FORCE_COPY:
            f.notes.append(f'copies {t:08X}: {FORCE_COPY[t]}')
            return 'foreign', (None, 'forced', None, None)
        tw = self.pairs.twin(t)
        if tw[0] and any(lo <= t < hi for lo, hi in COPY_RANGES) and not self.same(t, tw[0]):
            # (an object's routine whose twin is not the same code to its depth: a dispatcher alike, its states not; BN5's
            # copied, as a virus family's are)
            f.notes.append(f'copies {t:08X}: BN6\'s {tw[0]:08X} ({tw[1]}) is not the same code to its depth')
            return 'foreign', (None, 'not the same to its depth', None, None)
        if tw[0]:
            if any(lo <= t < hi for lo, hi in COPY_RANGES) and self.spawns_moved(t, forced=False):
                # (BN6's twin would spawn BN6's own of a type whose code is not BN5's: BN5's copied, spawning BN5's moved)
                f.notes.append(f'copies {t:08X}: BN6\'s {tw[0]:08X} would spawn BN6\'s own of a type whose code differs')
                return 'foreign', (None, 'spawns a moved type', None, None)
            if self.spawns_moved(t):
                f.notes.append(f'{t:08X} -> {tw[0]:08X}: BN6\'s twin spawns BN6\'s own of a type FORCE_MOVED (the differential says '
                               f'whether it matters: FORCE_COPY where it does)')
            return 'engine', tw
        if t in COPY_HELPERS or any(lo <= t < hi for lo, hi in COPY_RANGES):
            return 'foreign', tw
        return 'missing', tw

    def classify(self, f, blocks):
        """the family deriver's, and the aligned words of raw data that point into the unit's blocks (a table of pointers to
        byte lists: CrakOut's crack patterns, 0x080C7900) a pool's, moved with the code"""
        data = super().classify(f, blocks)
        inside = lambda v: any(lo <= (v & ~1) < hi for lo, hi in blocks)
        out = []
        for a, b, k in data:
            if k != 'RAW':
                out.append((a, b, k))
                continue
            words = [x for x in range((a + 3) & ~3, b - 3, 4) if inside(self.b5.u32(x))]
            at = a
            for x in words:
                if at < x:
                    out.append((at, x, 'RAW'))
                out.append((x, x + 4, 'POOL'))
                at = x + 4
            if at < b:
                out.append((at, b, 'RAW'))
        return out

    def covered(self, f):
        """the family deriver's, but raw data (a table a literal names) ends where a routine of another's begins: a chip's code
        stands among other families' routines, not after its records, so a table runs 0x80 bytes at most to the next push
        {..., lr} or the next word its code reads"""
        cov = super().covered(f)
        raw = sorted(a for a, k in cov.items() if k == 'RAW')
        for a in raw:
            if cov.get(a) != 'RAW' or a & 1:
                continue
            if self.b5.u16(a) & 0xFF00 == 0xB500 and cov.get(a - 2) == 'RAW' and cov.get(a - 1) == 'RAW':
                k = a
                while cov.get(k) == 'RAW':
                    del cov[k]
                    k += 1
        return cov

    def spawns_moved(self, a, forced=True, depth=6, seen=None):
        """whether the BN5 routine at `a`, a twin's, spawns (itself, or through the routines it calls and the objects it
        spawns, `depth` deep) an object BN6 has no twin of (or one FORCE_MOVED, where `forced`): BN6's twin would spawn
        BN6's own there"""
        seen = set() if seen is None else seen
        if a in seen or depth < 0 or not any(lo <= a < hi for lo, hi in COPY_RANGES):
            return False
        seen.add(a)
        w = T.walk(self.b5, a)
        for site, t in w.bls.items():
            if t in BN5['spawners']:
                v, _ = T.back_value(w, site, 0)
                if v is None or v > 0xFF:
                    return True
                tt = self.plain_twin(BN5['spawners'][t], v, forced)
                if tt['bn6'] is None or (tt['routine'] and self.spawns_moved(tt['routine'], forced, depth - 1, seen)):
                    return True
            elif self.spawns_moved(t, forced, depth - 1, seen):
                return True
        # (and its states', where it runs them from a table: an object's routine is its states' dispatcher, each state
        # its substates', a hop that takes no depth)
        return any(self.spawns_moved(x, forced, depth, seen) for ents in self.pairs.tables_of(self.b5, w) for x in ents)

    def find_tables(self, f, w):
        """the family deriver's, and tables whose entries lie anywhere in the ROM: a chip family's table of routines by its
        subfamily (BN5's misc family 0x1A's at 0x080EC818) names routines of objects' code far from it; a literal is such a
        table where it holds two code pointers or more one after another and the routine loads a word from it by a register
        it then calls or jumps to"""
        out = super().find_tables(f, w)
        for lit, v in w.lits.items():
            if v is None or v & 3 or not self.b5.has(v, 8) or not used_as_table(w, lit):
                continue
            ents = []
            for k in range(64):
                x = self.b5.u32(v + 4 * k)
                if not T.is_code_ptr(x, self.b5):
                    break
                ents.append(x)
            # (an object's states end in the engine's own routines, far from it: HotBody's aura's third state, BN5's
            # 0x0800325C, the family deriver's table stopped short of)
            if len(ents) >= 2 and len(ents) > len(out.get(v, ())):
                out[v] = ents
        return out

    def plain_twin(self, kind, t5, forced=True):
        """the family deriver's twin of an object type (the same code to its depth), FORCE_MOVED's moved (where `forced`),
        TYPE_AUDITS' BN6's own"""
        if (kind, t5) in TYPE_AUDITS:
            return dict(bn6=t5, routine=None, why='BN6\'s own, audited: ' + TYPE_AUDITS[(kind, t5)])
        out = self.same_type(kind, t5)
        if forced and (kind, t5) in FORCE_MOVED and out['routine']:
            return dict(bn6=None, routine=out['routine'], why='moved: ' + FORCE_MOVED[(kind, t5)])
        return out

    def type_twin(self, kind, t5):
        """plain_twin's, and moved where BN6's twin, the same code, would spawn BN6's own of a type whose code is not
        BN5's (MetaGel's thrower and its gel: the gel's sprite another in BN6)"""
        out = self.plain_twin(kind, t5)
        if out['bn6'] is not None and out['routine'] and self.spawns_moved(out['routine'], forced=False):
            return dict(bn6=None, routine=out['routine'], why='moved: BN6\'s twin would spawn BN6\'s own of a type whose code differs')
        return out

    def same(self, a5, a6, depth=3):
        """the pairs' deep_same, where a pair the project audited as serving (bn5_twins.AUDITS, 'ok') counts as the same: an
        object's end, its states' generic routines; and the sprites the two load by number each game's of the same bytes
        (the same code drawing another picture is not the same: CrakOut's crack, list 5's 0xE, another in BN6)"""
        audit = self.pairs.audit(a5, a6)
        if audit and audit[0] == 'ok':
            return True
        n5, w5 = T.norm(self.b5, a5)
        n6, w6 = T.norm(self.b6, a6)
        if n5 != n6 or not self.pairs.bls_agree(w5, w6):
            return False
        l5, l6 = sprite_loads(w5, 5), sprite_loads(w6, 6)
        if len(l5) != len(l6) or not all(same_sprite(self.b5, self.b6, x, y) for x, y in zip(l5, l6)):
            return False
        if depth <= 0:
            return True
        t5, t6 = self.pairs.tables_of(self.b5, w5), self.pairs.tables_of(self.b6, w6)
        return len(t5) == len(t6) and all(len(x) == len(y) and all(self.same(p, q, depth - 1) for p, q in zip(x, y))
                                          for x, y in zip(t5, t6))

    def same_type(self, kind, t5):
        """the family deriver's type_twin by `same`: BN6's type of the same number first, then any"""
        tab5, n5 = BN5['types'][kind]
        if t5 >= n5:
            return dict(bn6=None, routine=None, why=f'type {t5:#x} past BN5\'s {n5}')
        r5 = self.b5.u32(tab5 + 4 * t5)
        if not T.is_code_ptr(r5, self.b5):
            return dict(bn6=None, routine=None, why='moved: no BN6 type runs the same code')
        tab6, n6 = BN6['types'][kind]
        for t6 in [t5] + [t for t in range(n6) if t != t5]:
            if t6 >= n6:
                continue
            r6 = self.b6.u32(tab6 + 4 * t6)
            if r6 and T.is_code_ptr(r6, self.b6) and self.same(r5 & ~1, r6 & ~1):
                return dict(bn6=t6, routine=r5 & ~1, why=f'BN6\'s type {t6:#x} runs the same code')
        return dict(bn6=None, routine=r5 & ~1, why='moved: no BN6 type runs the same code')

    def walk_unit(self, f, roots):
        f.states5, f.hooks5 = [], []
        f.own = set(roots)
        self.walk_family(f, extra=roots)


def used_as_table(w, lit):
    """the literal's register indexed by a word load (ldr rd,[rX,rY] or [rY,rX]) within the next few instructions"""
    addrs = sorted(w.insns)
    for k, a in enumerate(addrs):
        i = w.insns[a]
        if i.lit != lit:
            continue
        r = i.f[0]
        for b in addrs[k + 1:k + 6]:
            j = w.insns[b]
            if j.form == 'ldr_r' and r in j.f[1:]:
                return True
    return False


def chip_family(f):
    """a Family-shaped record of a unit's walk (no region, no states, no records)"""
    f.region = (0, 0)
    f.st, f.s1, f.s2 = 0, 0, 0
    f.ids5 = [None] * 6
    return f


def derive(b5, b6, pairs, kind, n, label, alloc, number):
    d = ChipDeriver(b5, b6, pairs)
    f = chip_family(Family(-1))
    f.name = label
    entry = entry_of(b5, kind, n)
    roots = [entry]
    d.walk_unit(f, roots)
    blocks = d.blocks_of(f)
    for _ in range(4):
        more = gap_roots(b5, f, blocks)
        if not more:
            break
        roots += more
        f = chip_family(Family(-1))
        f.name = label
        d.walk_unit(f, roots)
        blocks = d.blocks_of(f)
    f.data = d.classify(f, blocks)
    # (a table's word aimed at a twin whose body differs serves only where audited: the symbol map's pair of BN5's
    # 0x0800A2F0 was another routine, NrthWind's wind never ending)
    for v, (to, why, tw) in sorted(f.words.items()):
        if tw and tw[2] == 'DIFF':
            audit = pairs.audit(v & ~1, tw[0])
            if not (audit and audit[0] == 'ok'):
                f.decide('pair', f'a table\'s word {v:08X} -> {tw[0]:08X} ({tw[1]}): the bodies differ ({tw[3]}) and no audit says it serves')
    d.scan(f)
    d.scan_words(f, blocks)
    u = Unit()
    u.f, u.kind, u.n5, u.label, u.entry, u.number = f, kind, n, label, entry, number
    u.blocks = []
    at = alloc['code']
    for lo, hi in blocks:
        at = X.rounded(at, CODE_STEP) + (lo & 3)
        u.blocks.append((lo, hi, at, zlib.crc32(b5.bytes(lo, hi - lo)) & 0xFFFFFFFF))
        at += hi - lo
    alloc['code'] = at
    u.data = f.data
    X.calls_of(u, d, blocks)
    u.clones = []
    u.held_out = []
    u.more_imms = []
    chip_actions(u, d)
    u.sprites, u.effects = [], []
    art_of(u, b5, b6, alloc)
    numbered_calls(u, b5, b6)
    X.types_of(u)
    X.sounds_of(u, b5, b6)
    X.conditions_of(u, pairs)
    u.imms = X.unique_imms(f.imms + u.more_imms)
    u.words = sorted((v, to) for v, (to, why, tw) in f.words.items())
    u.numbers = sorted((lit, was, now) for lit, (was, now, why) in f.numbers.items())
    return u


def gap_roots(b5, f, blocks):
    """the routines between a load and the pool it reads where they fall in two blocks (BN5 put HotBody's pool past a
    routine of its own that no walk reaches, 0x080DA57E): walked as the unit's too, so one block holds them (the
    installer moves a block whole, its loads reading their own block's pools)"""
    def block(a):
        return next((k for k, (lo, hi) in enumerate(blocks) if lo <= a < hi), None)
    out = set()
    for w in f.walks.values():
        for a, i in w.insns.items():
            if i.form != 'ldr_lit' or i.lit is None:
                continue
            ka, kl = block(a), block(i.lit)
            if ka is None or kl is None or ka == kl:
                continue
            lo, hi = sorted((blocks[ka][1], blocks[kl][0]))
            out |= {g for g in range(lo & ~1, hi, 2) if b5.u16(g) & 0xFF00 == 0xB500 and g not in f.walks}
    return sorted(out)


def chip_actions(u, d):
    """the CurActions a chip's code writes or compares (scan's virus rules are not a chip's): MegaMan's ready state, BN5's 6,
    is BN6's 8 (legit.json, maps, action); a family's number is its BN6 number where the unit moves it; anything else a
    decision"""
    f = u.f
    for e, w in f.walks.items():
        if 'virus' not in f.ctx.get(e, ()):
            continue   # (an object's own routine: its CurAction its own phase, numbered alike in both)
        for a in sorted(w.insns):
            i = w.insns[a]
            if i.form == 'bl' and BN5_SET_ATTACK <= i.target < BN5_SET_ATTACK + 7 * 4 and not (i.target - BN5_SET_ATTACK) & 3:
                v, chain = T.back_value(w, a, 0)
                f.decide('curaction', f'{a:08X}: setAttack{(i.target - BN5_SET_ATTACK) // 4} with {v if v is None else hex(v)}: a chip '
                                      f'that starts another attack')
            if i.form == 'strb' and i.f[1:] == (5, 9):
                v, chain = T.back_value(w, a, i.f[0])
                if v == 6 and len(chain) == 1 and chain[0][1] == 'movs':
                    u.more_imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=i.f[0], rn=0, was=6, now=8,
                                            why='CurAction 6 (BN5\'s ready state) written: BN6\'s 8'))
                elif v is None or v >= 6:
                    f.decide('curaction', f'{a:08X}: CurAction written from {chain}')


# Routines whose argument is a number of a table each game keeps its own (BN5's, BN6's, entry size, the register): the muzzle
# flash spawner's flash (the first kind's type 5's table, read at BN5 0x080B9BA8 / BN6 0x080BA570)
NUMBERED = {0x080B9CAE: ('a muzzle flash', 0x080B9A5C, 0x080BA444, 5, 4)}


def numbered_calls(u, b5, b6):
    """each call of a NUMBERED routine: its number the same entry in both games' tables (kept), else a decision"""
    f = u.f
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t not in NUMBERED:
                continue
            what, t5, t6, size, reg = NUMBERED[t]
            v, chain = T.back_value(w, site, reg)
            if v is None or len(chain) != 1:
                f.decide('number', f'{site:08X}: {what} of a number made otherwise ({chain})')
            elif b5.bytes(t5 + size * (v & 0xFF), size) != b6.bytes(t6 + size * (v & 0xFF), size):
                f.decide('number', f'{site:08X}: {what} {v & 0xFF:#x}, another entry in BN6\'s table')


# BN5's routine that sets up an object's sprite and collision from two packed words (r0: its sprite's list x 4 << 8 | its
# number, its animation << 16; r1 its collision's), BN6's twin 0x08011504 the same: a sprite loaded by number all the same
SPRITE_SETUP = 0x0800F198
SPRITE_SETUP6 = 0x08011504
SPRITE_LOAD6 = 0x080026E4   # (BN6's sprite load, BN5's 0x08002650: r1 the list's offset, r2 the number)
BN5_SPRITE_LISTS = 0x0803272C
# The grown sprite lists (src/emu/xvirus_grow.c: 1, 2, 4 and 5, 256 numbers each) the chips take numbers of, from the top
# down, where the families take theirs from the bottom up (tools/xrecipe.py, LIST_NUMBERS): one numbering, each slot the
# install checks to hold the fill first; a sprite BN6 keeps another of at the number BN5's code loads is copied there
CHIP_LISTS = (1, 2, 4, 5)
CHIP_LIST_TOP = 0xFF
SPRITE_ROOM = 0x800


def sprite_loads(w, game):
    """the sprites a walk's code loads by number, in address order: (its list's offset, its number), None where made
    otherwise (the loader's r1 and r2, or the packed word SPRITE_SETUP takes in r0)"""
    load, setup = (BN5['sprite_load'], SPRITE_SETUP) if game == 5 else (SPRITE_LOAD6, SPRITE_SETUP6)
    out = []
    for site in sorted(w.bls):
        t = w.bls[site]
        if t == load:
            l, _ = T.back_value(w, site, 1)
            v, _ = T.back_value(w, site, 2)
            out.append((l, v))
        elif t == setup:
            v, _ = T.back_value(w, site, 0)
            out.append(None if v is None else ((v >> 8) & 0xFF, v & 0xFF))
    return out


def same_sprite(b5, b6, x, y):
    """two loads (sprite_loads') the same picture: each game's sprite of its list and number the same bytes"""
    if x is None or y is None or None in x or None in y:
        return x == y
    _, s5 = X.sprite(b5, BN5_SPRITE_LISTS, x[0] // 4, x[1])
    _, s6 = X.sprite(b6, BN6['sprites'], y[0] // 4, y[1])
    return bool(s5) and s5 == s6


def packed_loads(f):
    """the sprites a unit's code sets up through SPRITE_SETUP, as the family deriver's loads (its list's offset, its number)"""
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t != SPRITE_SETUP:
                continue
            v, chain = T.back_value(w, site, 0)
            key = (None, None) if v is None else ((v >> 8) & 0xFF, v & 0xFF)
            f.loads.setdefault(key, []).append((site, chain))


def load_rewrite(u, site, chain, was, now, why):
    """a load's number set to BN6's: the immediate that makes it, or the packed word's low byte in its literal"""
    w = next((w for w in u.f.walks.values() if site in w.bls), None)
    if w is not None and w.bls[site] == SPRITE_SETUP:
        lit = w.insns[chain[0][0]].lit if chain and chain[0][1] == 'ldr_lit' else None
        if lit is None:
            u.f.decide('sprite', f'{why}: its packed word made otherwise ({chain})')
            return
        word = w.lits[lit]
        u.f.numbers[lit] = (word, (word & ~0xFF) | now, why)
        return
    X.imm_for(u, chain, 2, was, now, why)


def art_of(u, b5, b6, alloc):
    """what a unit draws: the sprites its code loads by number, each BN6's own where its list holds the same bytes at that
    number, else BN5's copied at a number of the chips' in its grown list (CHIP_LISTS, from the top down), its loads set
    to it; and its effects"""
    f = u.f
    packed_loads(f)
    for (l, v), sites in sorted(f.loads.items(), key=lambda x: str(x[0])):
        if l is None or v is None:
            f.decide('sprite', f'a sprite {l if l is None else l // 4}:{v if v is None else hex(v)} loaded at '
                               f'{", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        lst = l // 4
        _, s5 = X.sprite(b5, BN5_SPRITE_LISTS, lst, v)
        _, s6 = X.sprite(b6, BN6['sprites'], lst, v)
        if s5 and s5 == s6:
            continue
        if lst not in CHIP_LISTS or not s5:
            f.decide('sprite', f'a sprite {lst}:{v:#x} loaded at {", ".join(f"{s:08X}" for s, _ in sites)}: BN6\'s of that number another, '
                               + ('its list not grown' if s5 else 'BN5\'s not read'))
            continue
        to = alloc['memo'].get((lst, v))
        if to is None:
            to = alloc['lists'][lst]
            alloc['lists'][lst] -= 1
            alloc['memo'][(lst, v)] = to
            room = X.rounded(len(s5), SPRITE_ROOM)
            u.sprites.append(dict(list=lst, index=v, to_list=lst, to_index=to, at=alloc['art'], room=room))
            alloc['art'] += room
        for site, chain in sites:
            load_rewrite(u, site, chain, v, to, f'its sprite: list {lst} {v:#x} in BN5, {to:#x} in BN6 (BN5\'s copied there)')
    for e, sites in sorted(f.effects.items(), key=lambda x: str(x[0])):
        if e is None:
            f.decide('effect', f'an effect id made otherwise at {", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        r5 = b5.bytes(BN5['effects'] + 4 * e, 4)
        r6 = b6.bytes(BN6['effects'] + 4 * e, 4) if e < BN6['neffects'] else b''
        if r5 != r6:
            f.decide('effect', f'its effect {e:#x} ({r5.hex()} in BN5, {r6.hex() or "none"} in BN6) at '
                               f'{", ".join(f"{s:08X}" for s, _ in sites)}')


# ---- the C files ----

KIND_C = {'attack': 'XCHIP_ATTACK', 'support': 'XCHIP_SUPPORT', 'navi': 'XCHIP_NAVI'}


def c_name(label):
    import re
    return re.sub(r'[^a-z0-9]', '', label.lower())


def c_unit(u, slot):
    n = c_name(u.label)
    L = [f'/* ---- {u.label}: BN5\'s {u.kind} {u.n5:#x} ({u.entry | 1:08X}) as BN6\'s {u.number:#x} ---- */', '']
    L.append(f'static const XvBlock {n}_blocks[] = {{')
    for lo, hi, at, crc in u.blocks:
        L.append(f'\t{{ 0x{lo:08X}, 0x{hi:08X}, XC_CODE + 0x{at - XC_CODE:X}, 0x{crc:08X}u }},')
    L.append('};')
    if u.data:
        L.append(f'static const XvData {n}_data[] = {{')
        for lo, hi, k in u.data:
            L.append(f'\t{{ 0x{lo:08X}, 0x{hi:08X}, XV_{k} }},')
        L.append('};')
    L.append(f'static const XvPair {n}_calls[] = {{')
    for c in u.calls:
        if c['spawner']:
            L.append(f'\t{{ {X.BN5_SPAWN_NAME[c["bn5"]]}, {c["spawner"]} }},   /* the dispatch\'s spawner */')
        else:
            how = c['cls'] + (' (audited)' if c.get('audit') else '')
            L.append(f'\t{{ 0x{c["bn5"]:08X}, 0x{c["bn6"] or 0:08X} }},   /* {c["source"]}, {how} */')
    L.append('};')
    if u.words:
        L.append(f'static const XvPair {n}_words[] = {{')
        L += [f'\t{{ 0x{v:08X}, 0x{to:08X} }},' for v, to in u.words]
        L.append('};')
    if u.imms:
        L.append(f'static const XvImm {n}_imms[] = {{')
        for i in u.imms:
            L.append(f'\t{{ 0x{i["at"]:08X}, {i["form"]}, {i["rd"]}, {i["rn"]}, 0x{i["was"]:X}, 0x{i["now"]:X} }},   /* {i["why"]} */')
        L.append('};')
    if u.numbers:
        L.append(f'static const XvNumber {n}_numbers[] = {{')
        L += [f'\t{{ 0x{lit:08X}, 0x{was:08X}, 0x{now:08X} }},' for lit, was, now in u.numbers]
        L.append('};')
    if u.sprites:
        L.append(f'static const XvSprite {n}_sprites[] = {{')
        for x in u.sprites:
            L.append(f'\t{{ {x["list"]}, 0x{x["index"]:02X}, {x["to_list"]}, 0x{x["to_index"]:02X}, XC_ART + 0x{x["at"] - XC_ART:X}, 0x{x["room"]:X} }},')
        L.append('};')
    if u.effects:
        L.append(f'static const XvId {n}_effects[] = {{ ' + ', '.join(f'{{ 0x{a:02X}, 0x{b:02X} }}' for a, b in u.effects) + ' };')
    if u.types:
        L.append(f'static const XvType {n}_types[] = {{ ' + ', '.join(
            f'{{ {k}, 0x{t5:02X}, {"XV_TYPE_MOVED" if t6 is None else f"0x{t6:02X}"} }}' for k, t5, t6, _ in u.types) + ' };')
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    fields = [f'.{a} = {n}_{a}, .n{a} = {count(a)}' for a, have in (('blocks', True), ('data', u.data), ('calls', True), ('words', u.words),
                                                                   ('imms', u.imms), ('numbers', u.numbers), ('sprites', u.sprites),
                                                                   ('effects', u.effects), ('types', u.types)) if have]
    L += ['', f'#define {n.upper()}_CODE {{ .name = "{u.label} (BN5\'s)", .game = XROM_BN5_COLONEL_US, \\']
    L += [f'\t{f}, \\' for f in fields[:-1]] + [f'\t{fields[-1]} }}']
    return '\n'.join(L), (f'\t{{ {KIND_C[u.kind]}, 0x{u.n5:02X}, 0x{u.number:02X}, 0x{u.entry | 1:08X}, XC_HOMES + 0x{XC_HOME_SIZE * slot:X}, '
                          f'{n.upper()}_CODE }},')


HEADER_C = '''/* xchips_bn5.c: BN5's chips' code as units for BN6's battle engine, written by tools/xchiprecipe.py from the two ROMs
 * (python3 tools/xchiprecipe.py; --check says whether this is what it writes). Do not edit: change the generator (or
 * tools/bn5_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/MULTIROM.md, The chips' code):
 * addresses in BN5 Team Colonel (USA) and BN6 Cybeast Gregar (USA), counts and the CRC32s of the ranges the install copies
 * from the player's ROM; nothing of either game's. The units are the generator's, whole (no decision open), in install
 * order; src/emu/xchips_code.c installs them and src/core/xchips_moved.c names the chips that run them. */
#include "xchips_bn5.h"

#include "bn5.h"
#include "bn6.h"
#include "rom.h"
#include "xchips_code.h"
'''


def c_file(units):
    parts, rows = [HEADER_C], []
    for slot, u in enumerate(units):
        text_, row = c_unit(u, slot)
        parts.append(text_)
        rows.append(row)
    parts.append('const XChipUnit xchips_bn5[] = {\n' + '\n'.join(rows) + '\n};\n'
                 'const int xchips_bn5_count = (int)(sizeof xchips_bn5 / sizeof *xchips_bn5);\n')
    return '\n'.join(parts)


HEADER_MOVED = '''/* xchips_moved.c: the chips whose code moved from BN5's (src/emu/xchips_bn5.c, issue #82's C3), written by
 * tools/xchiprecipe.py from the two ROMs and the registry (src/core/xchips_registry.c): each its id in BN6's tables and the
 * family and subfamily its record takes in BN6's engine (a moved attack family's number, BN5's subfamily under it; or the
 * support or Navi family with the moved routine's entry). Do not edit: write it again. Numbers alone. */
#include "xchips_moved.h"

#include <stddef.h>

const XChipMoved xchips_moved[] = {
'''


def moved_file(rows):
    body = '\n'.join(f'\t{{ {bn6}, 0x{fam:02X}, 0x{sub:02X} }},   /* {name} */' for bn6, fam, sub, name in rows)
    return (HEADER_MOVED + body + '\n};\nconst int xchips_moved_count = (int)(sizeof xchips_moved / sizeof *xchips_moved);\n\n'
            'const XChipMoved *xchips_moved_of(int bn6) {\n\tfor (int i = 0; i < xchips_moved_count; ++i)\n'
            '\t\tif (xchips_moved[i].bn6 == bn6) return &xchips_moved[i];\n\treturn NULL;\n}\n')


def moved_rows(units, b5):
    """the registry's chips each unit plays: BN5's records of that family (an attack unit's: every subfamily, BN5's kept), or
    of the support or Navi family with that subfamily; each a chip the registry gives an id of its own (XREG_MOVED)"""
    names = X.chip_names(b5, BN5['chip_names'], 424)
    rows = []
    reg = registry_rows()
    for u in units:
        for bn5, bn6, how in reg:
            rec = record(b5, BN5_RECORDS, bn5)
            fam, sub = rec[0x0B], rec[0x0C]
            if u.kind == 'attack' and fam == u.n5:
                rows.append((bn6, u.number, sub, names.get(bn5, '?'), how))
            elif u.kind != 'attack' and fam == (BN5_SUPPORT_FAMILY if u.kind == 'support' else BN5_NAVI_FAMILY) and sub == u.n5:
                rows.append((bn6, BN6_SUPPORT_FAMILY if u.kind == 'support' else BN6_NAVI_FAMILY, u.number, names.get(bn5, '?'), how))
    # (BN6's own chips by name run BN6's code; a DarkChip or a Program Advance waits for its own milestone)
    bad = [f'{name} ({bn6}, {how})' for bn6, _, _, name, how in rows if how in ('PENDING', 'RECORD')]
    return sorted((bn6, fam, sub, name) for bn6, fam, sub, name, how in rows if how == 'MOVED'), bad


# ---- the report ----

def report(units, path):
    lines = ['# BN5\'s chips\' code as tools/xchiprecipe.py derives it', '']
    for u in units:
        f = u.f
        cls = {}
        for c in u.calls:
            k = 'dispatch' if c['spawner'] else 'none' if c['bn6'] is None else c['cls'] + ('+audit' if c.get('audit') else '')
            cls[k] = cls.get(k, 0) + 1
        moved = [f'{kind_name(k)} {t5:#x}' + ('' if t6 is None else f' as BN6\'s {t6:#x}') for k, t5, t6, _ in u.types]
        lines += [f'## {u.label}: BN5\'s {u.kind} {u.n5:#x} ({u.entry:08X}) as BN6\'s {u.number:#x}', '',
                  f'- code: {len(u.blocks)} blocks, {sum(b - a for a, b, _, _ in u.blocks)} bytes: '
                  + ', '.join(f'{a:08X}-{b:08X}' for a, b, _, _ in u.blocks),
                  f'- calls {len(u.calls)} ({", ".join(f"{v} {k}" for k, v in sorted(cls.items()))}): '
                  + ', '.join(f'{c["bn5"]:08X}->{c["bn6"] or 0:08X}' for c in u.calls if not c['spawner']),
                  f'- immediates {len(u.imms)}; words {len(u.words)}; numbers kept {len(u.numbers)}; objects: {", ".join(moved) or "-"}; '
                  f'sounds ' + (', '.join(f'{v:#x}{"" if same else " (differs)"}' for v, same in u.sounds) or '-') +
                  f'; random draws {len(f.rng)}']
        if f.notes:
            lines.append('- notes: ' + '; '.join(dict.fromkeys(f.notes)))
        lines.append('- decisions: ' + ('none' if not f.decisions else ''))
        lines += [f'  - {k}: {d}' for k, d in f.decisions]
        lines.append('')
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8') as o:
        o.write('\n'.join(lines) + '\n')


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--check', action='store_true')
    ap.add_argument('--report', action='store_true')
    ap.add_argument('--unit', nargs='*', default=[], help='more units for the report: KIND:N:NAME (attack:0x4f:DrilArm)')
    a = ap.parse_args()
    b5, b6 = X.find_roms()
    pairs = Pairs(b5, b6, X.MATCH, X.WORK)
    if not pairs.map_ok:
        sys.exit(f'xchiprecipe: no symbol map in {X.MATCH}: build.py symbols --bn6f DIR makes it')
    alloc = dict(code=XC_CODE, art=XC_ART, lists={k: CHIP_LIST_TOP for k in CHIP_LISTS}, memo={})
    order = list(UNITS) + list(CANDIDATES) + [(k, int(n, 0), name) for k, n, name in (x.split(':') for x in a.unit)]
    units = []
    for kind, n, label in order:
        units.append(derive(b5, b6, pairs, kind, n, label, alloc, number_of(kind, n)))
    report(units, os.path.join(WORK, 'report.md'))
    print(f'xchiprecipe: {len(units)} units derived, {sum(1 for u in units if not u.f.decisions)} with no decision open; '
          f'{os.path.join(WORK, "report.md")}')
    if a.report:
        return 0
    whole = units[:len(UNITS)]
    for u in whole:
        if u.f.decisions:
            sys.exit(f'xchiprecipe: {u.label} has decisions open: ' + '; '.join(f'{k}: {d}' for k, d in u.f.decisions))
    rows, bad = moved_rows(whole, b5)
    if bad:
        sys.exit('xchiprecipe: the registry does not give these chips XREG_MOVED: ' + ', '.join(bad))
    out = {OUT_C: c_file(whole), OUT_MOVED: moved_file(rows)}
    if a.check:
        same = all(open(p, encoding='utf-8').read() == t if os.path.exists(p) else False for p, t in out.items())
        print(f'xchiprecipe: the two files {"are" if same else "are NOT"} what the generator writes')
        return 0 if same else 1
    for p, t in out.items():
        with open(p, 'w', encoding='utf-8') as f:
            f.write(t)
    print(f'xchiprecipe: {", ".join(u.label for u in whole)} written into {os.path.relpath(OUT_C, ROOT)}; '
          f'{len(rows)} chips into {os.path.relpath(OUT_MOVED, ROOT)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
