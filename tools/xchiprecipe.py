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
COPY_HELPERS = {
    0x08012530: "the swords' reach (Z Saver's, CustSwrd's, MoonBld's, Katana's): the first panel ahead with a foe on it, each panel "
                "tested by its flags against the side's mask (0x080125B8, through 0x0800AFB0) and 0x0800B348's object test along "
                "the rows of 0x080125C0; BN6 has no routine of its body (none found by the pairs' search), its calls BN6's twins",
}
# Routines copied though BN6 has a twin of the same body, and object types moved though BN6's runs the same code: where the
# data they read is BN6's own and another (each found by the chips' differential, tests/xdiff)
FORCE_COPY = {
    0x080093A2: "the Anti chips' trap sprung: BN5's own (its effect 0x2B where the trap was, and sound 0xA5); BN6's twin "
                "(0x0800ABC6) spawns its own effect 0x46 lower down",
    0x080C7614: "the shockwave's spawner (Guard's, the reflected wave): BN6's twin 0x080C8554 spawns BN6's type 0x16",
}
FORCE_MOVED = {
    (3, 0x16): "the shockwave: BN6's type 0x16 runs the same code but reads its kinds from its own table (BN6 0x080C8370), BN5's "
               "(0x080C7448) another: speed and reach by kind (chip_guard1_reflect: BN6's wave a panel ahead from its first)",
}
# BN5's routines a unit calls whose work no battle a run fights does, aimed at the chips' stubs (src/emu/xchips_code.h,
# XC_STUBS) instead of copied: a panel claim's two spawners (AqWhirl's, DrilArm's and Guard's wave's). Each spawns the
# fourth kind's 0x83 (0x080EA020, the one object that uses BN5's panel-claim pool 0x0802E920-0x0802EAB4 and its RAM
# 0x02035760, 0x02034B80, 0x0203C51C-0x0203C59F: no other code calls the pool, and no other code spawns 0x83) only where
# 0x080090E8 answers 1, the battle's bit 0x40 (BattleState +0x32, 0x0802D510); else it answers r0 0, Z set, r1-r3 as
# they came, as the stub does. The moved spawner would call BN6's twin of that gate, 0x0800A8F8, which reads BN6's bit
# 0x40 (0x0802E09A); BN6 sets it in one place, 0x0802E112 at a battle's start (from 0x0800795E), for a link battle alone:
# the battle's type not 1, 6, 7 or 9, its settings without 0x200000, and either the link's state 0x0200AD04 set (written
# only by the link routines with what the cable answered, 0x0803F4F8) or its settings' bit 8 with event flag 0x1722 (the
# Beast Link Gate). No run's battle is one (no link, the director sets neither); BN5 sets its own bit in one place too,
# 0x0802D594 (from 0x08006D2A), where the battle's type is not 1 and MegaMan's +0x2A is 1 or more (docs/MULTIROM.md,
# The chips' code)
STUB_ZERO = 0                             # (XC_STUBS + 0: movs r0,#0; bx lr)
STUBS = {
    0x080EA08C: (STUB_ZERO, "a panel claim's spawner: its gate (0x080090E8, the battle's bit 0x40) clear in every battle a run "
                            "fights, r0 0 as BN5 answers then"),
    0x080EA0BA: (STUB_ZERO, "a panel claim's other spawner, the same"),
}
# Helpers: BN5's code more than one unit calls, whose object only one copy may move (the dispatch moves a type once):
# installed once, before the units, from XC_HELPERS; a unit's call to its routine aimed at its copy. (BN5's routine, the
# project's name, why)
HELPERS = (
    (0x080B9CAE, 'MuzzleFlash', "the muzzle flash's spawner (r4 its number in the flash's table and the words it keeps, r7 "
                                "where it puts the flash): the first kind's 0x05, which BN6 runs other code for (its forms' "
                                "and Beast Out's own tests, its own table's numbers), moved once for every unit"),
    (0x080EBB96, 'BusterArm', "MegaMan's arm for a buster-like chip (0x080EBB96 and the routine it calls, 0x080EBBA6: SideBub's, "
                              "SpShake's, Pulsar's, WoodNos's, CactBal's, BN5's plain buster's too): its muzzle flash by his "
                              "form, through MuzzleFlash; one copy, so that BN5's own buster's calls are no unit's"),
)
# The object types a helper moves, for every unit (the units spawn them through the dispatch, the helper's copy run)
HELPER_TYPES = {(1, 0x05): 'MuzzleFlash'}
# Raw data whose extent no walk tells (a table read by a number made elsewhere): its bytes
# Code in a block no walk reaches, no word, call or branch of the ROM naming any of its halfwords (searched): copied as it is,
# never run (the walk's 'unreached' decision taken)
DEAD_CODE = {
    0x080DA27C: "AqWhirl's: between 0x080DA27A's branch past it and its target 0x080DA28E",
}
# Sprite loads by a table a helper's art sets (helper_art): BN5's load site, its table
TABLE_LOADS = {0x080B9BB2: 0x080B9A5C}
# Sprite loads by an object's kind from a table of its own, each row its sprite (its list x 4 and number at two offsets, or
# its number alone in a list the code names): BN5's load site: (the table, a row's size, the list's offset or None, the
# number's offset, its rows, the list where the code names it); every row's sprite BN6's own where the same bytes, else
# BN5's copied and the row's number set to the copy's (table_art); a row of list 0xFF draws nothing
TABLE_SPRITES = {
    0x080C5EEC: (0x080C5DF4, 16, 5, 6, 12, None),   # the bubbles' kinds (SideBub's): 12 rows to the code at 0x080C5EB4
    0x080C49B0: (0x080C4800, 12, 5, 6, 31, None),   # the third kind's 0x00's kinds (Guard's): 31 rows to the code at 0x080C4974
    0x080C74D2: (0x080C7448, 4, None, 0, 15, 4),    # the shockwave's kinds (Guard's reflected wave): 15 rows, list 4's
}
# Sprite loads of MegaMan's own sprite, whatever it is (0x0800DA6A's list and number: his body's), each game's own: BN5's
# load site: why
MEGAMAN_LOADS = {
    0x080E3648: "the swords' afterimage (the fourth kind's 0x28): MegaMan's own sprite, its list and number 0x0800DA6A's, BN6's "
                "own in BN6 (his body's look, the mask 'MegaMan's own sprite')",
}
# Effect ids a unit's code reads from a byte table of its own: BN5's spawn site: (the table, a row's size, the id's offset,
# its rows); each row's id BN6's twin of it or the chips' own (effect_of), set in its byte (u.bytes)
TABLE_EFFECTS = {
    0x080EE364: (0x080EE3D8, 1, 0, 3),   # Z Saver's three slashes' effects, 0x39-0x3B
    0x080EC016: (0x080EC110, 1, 0, 8),   # CustSwrd's slash by its kind, 0x16-0x1C and 0x28
    0x080EFC72: (0x080EFCC8, 1, 0, 2),   # Katana's two slashes, 0x17 and 0x16 (its +0x12 less one)
    0x080DAE4E: (0x080DAE74, 3, 2, 4),   # ElcReel's four sparks' rows (x, y, effect: 0x64, 0x65)
}
# Panel types an object sets from a table of its own (setPanelType's r2 a row's byte): BN5's call site: (the table, a row's
# size, the type's offset, its rows); a row of BN5's lava, sea or holy given BN6's number (u.bytes), 0xFF none
TABLE_PANELS = {
    0x080C4AAA: (0x080C4800, 12, 8, 31),   # the third kind's 0x00's kinds: rows 20 (sea) and 26 (lava)
}
DATA_SIZES = {
    0x080C5DF4: 12 * 16,  # the bubbles' kinds (TABLE_SPRITES)
    0x080C4800: 31 * 12,  # the third kind's 0x00's kinds (TABLE_SPRITES, TABLE_PANELS)
    0x080C7448: 15 * 4,   # the shockwave's kinds (TABLE_SPRITES)
    0x080B9A5C: 58 * 5,   # the muzzle flash's table: 58 entries of 5 bytes (its sprite's list x 4 and number, ...), code at 0x080B9B80
}
# The muzzle flash's numbers a call makes otherwise than by one instruction (the number its r4's low byte), each read from
# the code: BN5's call site: (the numbers, how)
FLASH_NUMBERS = {
    0x080EBC7E: ((0x23, 0x2B, 0x33), "the buster arm's (0x080EBBA6, SideBub's, SpShake's, Pulsar's, WoodNos's, CactBal's): 0x23 "
                                     "by default, 0x2B or 0x33 by his form (AI data +0/+1) and NaviStats +0x2C/+0x4C"),
    0x080EF602: ((0x2F,), "DrilArm's: r4 0x12F, 0x0100012F where its AttackVars +3 is 1: the low byte 0x2F"),
    0x080EE28C: ((0x03, 0x0A, 0x32), "Z Saver's: r4's low byte 0x080EC16A()'s, the sword's flash by his form: 3, 0xA or 0x32"),
    0x080EBF0E: ((0x03, 0x0A, 0x32), "CustSwrd's: r4's low byte 0x080EC16A()'s, as Z Saver's"),
    0x080EFB9E: ((0x03, 0x0A, 0x32), "Katana's: r4's low byte 0x080EC16A()'s, as Z Saver's"),
    0x080ED4E4: ((0x03, 0x0A, 0x32), "Slasher's: r4's low byte 0x080EC16A()'s, as Z Saver's"),
    0x080EF4C8: ((0x2C, 0x2D, 0x2E), "MrkCan's: r4 0x080EF548()'s number << 8, + 0x2C + its subfamily (AttackVars +3: 0, 1, 2 for "
                                     "MrkCan1-3)"),
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
XC_STUBS = XC_AT + 0x800                  # the stubs (STUBS), then
XC_HELPERS = XC_STUBS + 0x10              # ... the helpers' code (HELPERS), to
XC_HOMES = XC_AT + 0x2000                 # a unit's veneers, XC_HOME_SIZE each
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
    if kind == 'helper':
        return 0
    return FAMILY_BASE + n if kind == 'attack' else (FIRST_SUPPORT if kind == 'support' else FIRST_NAVI) + n

# The units moved, in install order: (kind, BN5's family or subfamily, the project's name), each
# its number by BN5's (number_of); each whole (no decision open) and its chips' specs passing (tests/xdiff/chip_*.json).
UNITS = (
    ('attack', 0x1B, 'CrakOut'),
    ('attack', 0x58, 'HotBody'),
    ('support', 0x13, 'Jealousy'),
    ('support', 0x15, 'NrthWind'),
    ('support', 0x06, 'MetaGel'),
    ('support', 0x08, 'Blinder'),
    ('attack', 0x55, 'Skully'),
    ('attack', 0x4F, 'DrilArm'),
    ('support', 0x3D, 'RainyDay'),
    ('attack', 0x28, 'WoodNos'),
    ('attack', 0x48, 'CactBal'),
    ('attack', 0x51, 'SpShake'),
    ('attack', 0x52, 'Pulsar'),
    ('attack', 0x4E, 'MrkCan'),
    ('attack', 0x1C, 'SideBub'),
    ('attack', 0x47, 'AqWhirl'),
    ('attack', 0x34, 'ZSaver'),
    ('attack', 0x54, 'Katana'),
    ('attack', 0x23, 'ElcReel'),
)
# The next units, derived for the report alone (--report) until what each needs is in: its decisions there, and the
# chips' differential's finding where a spec ran it (docs/MULTIROM.md, The chips' code: what remains)
CANDIDATES = (
    ('attack', 0x1F, 'Guard'),
    ('attack', 0x53, 'MoonBld'),
    ('support', 0x3A, 'BoyBomb'),
    ('attack', 0x27, 'CusVolt'),
    ('support', 0x20, 'BlakWing'),
    ('support', 0x23, 'HolyDrem'),
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
    if kind == 'helper':
        return n
    if kind == 'attack':
        return b5.u32(BN5_FAMILY_TABLE + 4 * (n - 0x10)) & ~1
    table = BN5_SUPPORT_TABLE if kind == 'support' else BN5_NAVI_TABLE
    return b5.u32(table + 4 * n) & ~1


class ChipDeriver(Deriver):
    """the family deriver's walk and checks, for a chip's code: no family region, every routine without a twin in BN5's
    objects', chips' and virus code copied (COPY_RANGES), or one of COPY_HELPERS"""

    helper_at = {}   # (BN5's helper routines and their copies, once the helpers are derived)

    def kind_of(self, f, t):
        t &= ~1
        if t in f.own:
            return 'own', None
        if t in self.helper_at:
            return 'engine', (self.helper_at[t], 'a helper', 'HELPER', 'its copy, installed once for every unit')
        if t in STUBS:
            return 'engine', (XC_STUBS + STUBS[t][0], 'a stub', 'STUB', STUBS[t][1])
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
        for k in [k for k in f.decisions if k[0] == 'unreached' and int(k[1][:8], 16) in DEAD_CODE]:
            f.decisions.remove(k)
            f.notes.append(f'{k[1][:17]}: dead code ({DEAD_CODE[int(k[1][:8], 16)]})')
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
        for lo, hi in getattr(self, 'gaps', ()):
            for a in range(lo, hi):
                cov.setdefault(a, 'RAW')
        # (a table DATA_SIZES knows: its bytes all, where the walk took its start for raw data)
        for v, n in DATA_SIZES.items():
            if cov.get(v) == 'RAW':
                for k in range(n):
                    if cov.get(v + k, 'RAW') != 'RAW':
                        break
                    cov[v + k] = 'RAW'
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
        BN5's (MetaGel's thrower and its gel: the gel's sprite another in BN6); a type a helper moves, none of the unit's"""
        if (kind, t5) in HELPER_TYPES and self.helper_at:
            return dict(bn6=None, routine=None, helper=HELPER_TYPES[(kind, t5)], why=f'moved by the helper {HELPER_TYPES[(kind, t5)]}')
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


def derive(b5, b6, pairs, kind, n, label, alloc, number, helper_at=None):
    d = ChipDeriver(b5, b6, pairs)
    d.helper_at = dict(helper_at or {})
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
    # (a gap no routine of a push {..., lr} starts in, a leaf's: its bytes copied as they are, raw, so one block holds the
    # load and its pool: the muzzle flash's, 0x080B9CD6-0x080B9D04)
    d.gaps = gap_spans(f, blocks)
    if d.gaps:
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
    u.f, u.kind, u.n5, u.label, u.entry, u.number, u.b5 = f, kind, n, label, entry, number, b5
    u.blocks = []
    where = 'hcode' if kind == 'helper' else 'code'
    at = alloc[where]
    for lo, hi in blocks:
        at = X.rounded(at, CODE_STEP) + (lo & 3)
        u.blocks.append((lo, hi, at, zlib.crc32(b5.bytes(lo, hi - lo)) & 0xFFFFFFFF))
        at += hi - lo
    alloc[where] = at
    if kind == 'helper' and at > XC_HOMES:
        f.decide('room', f'the helpers\' code runs to {at:08X}, past XC_HOMES')
    u.data = f.data
    X.calls_of(u, d, blocks)
    u.clones = []
    u.held_out = []
    u.more_imms = []
    chip_actions(u, d)
    u.sprites, u.effects, u.bytes = [], [], []
    art_of(u, b5, b6, alloc)
    numbered_calls(u)
    chip_panels(u)
    for key in [k for k, v in f.types.items() if v.get('helper')]:
        f.notes.append(f'spawns the {kind_name(key[0])} kind\'s {key[1]:#x}: {f.types.pop(key)["why"]}')
    X.types_of(u)
    chip_sounds(u, b5, b6)
    X.conditions_of(u, pairs)
    u.imms = X.unique_imms(f.imms + u.more_imms)
    u.words = sorted((v, to) for v, (to, why, tw) in f.words.items())
    u.numbers = sorted((lit, was, now) for lit, (was, now, why) in f.numbers.items())
    return u


def gap_spans(f, blocks):
    """the gaps between a load and the pool it reads where they still fall in two blocks"""
    def block(a):
        return next((k for k, (lo, hi) in enumerate(blocks) if lo <= a < hi), None)
    out = set()
    for w in f.walks.values():
        for a, i in w.insns.items():
            if i.form != 'ldr_lit' or i.lit is None:
                continue
            ka, kl = block(a), block(i.lit)
            if ka is not None and kl is not None and ka != kl:
                out.add(tuple(sorted((blocks[ka][1], blocks[kl][0]))))
    return sorted(out)


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
        if 'virus' not in f.ctx.get(e, ()) or 'object' in f.ctx.get(e, ()):
            continue   # (an object's own routine, an entry of its states' table too: its CurAction its own phase, numbered alike)
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


# The song slots BN5's sounds take in BN6 where BN6 has another song at their number (issue #82's shared allocations: one
# BN5 sound one slot, whichever track copies it, the slot taken only where it still holds BN6's empty header; the fixed
# ones the plan's, the chips' others from 0x52 down, each in the report): src/audio/xsong.c's xsong_install_at copies the
# song there as the units install, and the units' immediates name the slot
SOUND_SLOTS = {
    0xBC: 0x55, 0xB7: 0x61, 0x6F: 0x54, 0xE4: 0x53, 0x70: 0x5F, 0xBA: 0x60, 0xC0: 0x62,   # (the plan's)
    0x105: 0x52, 0xA6: 0x51, 0xC6: 0x50, 0xBD: 0x4F, 0x10E: 0x4E,                          # (the chips' own, from 0x52 down)
}


def chip_sounds(u, b5, b6):
    """the sounds its code starts: BN6's own where its song of the number is BN5's, else BN5's copied to its slot
    (SOUND_SLOTS), the immediates that name it given the slot (movs r,#n; or movs r,#n-0xFF and adds r,#0xFF: then the slot
    and 0)"""
    f = u.f
    u.sounds, u.songs = [], []
    for v, sites in sorted(f.sounds.items(), key=lambda x: str(x[0])):
        if v is None:
            f.decide('sound', f'a sound made otherwise at {", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        same = X.song_sig(b5, BN5['songs'], v) == X.song_sig(b6, BN6['songs'], v)
        u.sounds.append((v, same))
        if same:
            continue
        slot = SOUND_SLOTS.get(v)
        if slot is None:
            f.decide('sound', f'its sound {v:#x} is another song in BN6 and has no slot of its own (SOUND_SLOTS)')
            continue
        u.songs.append((v, slot))
        for site, chain in sites:
            sound_imms(u, b5, chain, v, slot)


def sound_imms(u, b5, chain, v, slot):
    why = f'its sound {v:#x}: BN5\'s copied to BN6\'s slot {slot:#x}'
    movs = [(a, x) for a, form, x in chain if form == 'movs']
    adds = [(a, x) for a, form, x in chain if form == 'adds8']
    lo = v if v <= 0xFF else v - 0xFF
    if len(movs) != 1 or movs[0][1] != lo or (v > 0xFF and (len(adds) != 1 or adds[0][1] != 0xFF)) or (v <= 0xFF and adds):
        u.f.decide('immediate', f'{why}: made by {chain}, not movs (and adds #0xFF)')
        return
    a = movs[0][0]
    u.more_imms.append(dict(at=a, form='THUMB_MOVS', rd=(b5.u16(a) >> 8) & 7, rn=0, was=lo, now=slot, why=why))
    if adds:
        a = adds[0][0]
        u.more_imms.append(dict(at=a, form='THUMB_ADDS', rd=(b5.u16(a) >> 8) & 7, rn=0, was=0xFF, now=0, why=why + ' (its high part)'))


# BN5's panel types BN6 numbers otherwise: its lava and sea BN6's 13 and 14 where src/emu/xpanels.c brings them in
# (XPANEL_LAVA, XPANEL_SEA), its holy BN6's 5 (BN6_PANEL_HOLY); the panel getter (object_getPanelDataOffset) and setter
PANEL_TYPES = {0x8: 0xD, 0x9: 0x5, 0xA: 0xE}
PANEL_GETTER, SET_PANEL_TYPE = 0x0800AFC2, 0x0800B2A6
PANEL_SAME = 0x10   # (a panel's first 0x10 bytes laid out alike in both: legit.json's panel layout)


def X_u8(u, a):
    return u.b5.u8(a)


def bn5_twins_same_panels():
    import bn5_twins
    return bn5_twins.SAME_PANEL_TYPES


def chip_panels(u):
    """the panel types its code sets (setPanelType's r2) or compares with a panel's (its +2, after the getter): BN6's
    numbers for BN5's lava, sea and holy; and what it reads of a panel the getter gives: within its first 0x10 bytes (the
    getter's audit), else a decision"""
    f = u.f
    for w in f.walks.values():
        addrs = sorted(w.insns)
        for k, a in enumerate(addrs):
            i = w.insns[a]
            if i.form != 'bl':
                continue
            if i.target == SET_PANEL_TYPE:
                v, chain = T.back_value(w, a, 2)
                if v in PANEL_TYPES:
                    X.imm_for(u, chain, 2, v, PANEL_TYPES[v], f'a panel set to BN5\'s type {v:#x}: BN6\'s {PANEL_TYPES[v]:#x}')
            if i.target != PANEL_GETTER:
                continue
            regs = {0}
            for n, b in enumerate(addrs[k + 1:k + 12]):
                j = w.insns[b]
                if j.form in ('ldrb', 'ldrh', 'ldr', 'strb', 'strh', 'str') and len(j.f) == 3 and j.f[1] in regs and j.f[2] >= PANEL_SAME:
                    f.decide('panel', f'{b:08X}: a panel\'s +{j.f[2]:#x} read or written (BN5\'s layout past +0x10 another)')
                if j.form == 'ldrb' and j.f[1:] == (0, 2):
                    for c in addrs[k + 2 + n:k + 5 + n]:
                        x = w.insns[c]
                        if x.form == 'cmp' and x.f[0] == j.f[0] and x.f[1] in PANEL_TYPES:
                            u.more_imms.append(dict(at=c, form='THUMB_CMP', rd=x.f[0], rn=0, was=x.f[1], now=PANEL_TYPES[x.f[1]],
                                                    why=f'a panel\'s type compared with BN5\'s {x.f[1]:#x}: BN6\'s {PANEL_TYPES[x.f[1]]:#x}'))
                if j.form == 'bl' or j.form in ('pop', 'bx'):
                    break
                if T.writes(j, 0) and j.form not in ('ldrb', 'ldrh', 'ldr'):
                    regs.discard(0)
    # (types set from a table's rows, TABLE_PANELS: each row's BN5's lava, sea or holy given BN6's number)
    for site in sorted({a for a, v in f.panel_types if v is None and a in TABLE_PANELS}):
        table, size, off, rows = TABLE_PANELS[site]
        for k in range(rows):
            at = table + size * k + off
            v = X_u8(u, at)
            if v in PANEL_TYPES:
                u.bytes.append((at, v, PANEL_TYPES[v], f'{table:08X}\'s row {k}: a panel set to BN5\'s type {v:#x}: BN6\'s {PANEL_TYPES[v]:#x}'))
            elif v != 0xFF and v not in bn5_twins_same_panels():
                u.f.decide('panel', f'{table:08X}\'s row {k}: a panel set to type {v:#x}, numbered otherwise in BN6')
    f.panel_types = [(a, v) for a, v in f.panel_types if v not in PANEL_TYPES and not (v is None and a in TABLE_PANELS)]


# Routines whose argument is a number of a table a helper moves with it (BN5's numbers, BN5's table): the muzzle flash's
# spawner, its number r4's low byte, its table's entries each its sprite's list x 4 and number first
NUMBERED = {0x080B9CAE: ('a muzzle flash', 0x080B9A5C, 5, 4)}


def numbered_calls(u):
    """each call of a NUMBERED routine: the numbers it may give (its r4's low byte by one instruction, or FLASH_NUMBERS), kept
    on the unit (u.flash) for its helper's table; else a decision"""
    f = u.f
    u.flash = set()
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t not in NUMBERED:
                continue
            what, table, size, reg = NUMBERED[t]
            v, chain = T.back_value(w, site, reg)
            if site in FLASH_NUMBERS:
                u.flash |= set(FLASH_NUMBERS[site][0])
            elif v is not None and len(chain) == 1:
                u.flash.add(v & 0xFF)
            elif len(chain) >= 2 and chain[0][1] == 'adds8' and chain[1][1] == 'lsls' and (chain[1][2] or 0) >= 8:
                u.flash.add(chain[0][2])   # (its low byte an immediate added to a value shifted past it: ElcReel's 0x35)
            else:
                f.decide('number', f'{site:08X}: {what} of a number made otherwise ({chain}): FLASH_NUMBERS')


def flash_art(helpers, units, b5, b6, alloc):
    """the muzzle flash helper's art by the numbers its callers give (the helpers' and the units')"""
    numbers = set().union(*(u.flash for u in helpers + units))
    for h in helpers:
        if h.entry in NUMBERED:
            helper_art(h, b5, b6, alloc, numbers)


def helper_art(h, b5, b6, alloc, numbers):
    """the muzzle flash's table entries the units use (`numbers`), each its sprite BN6's own where the same bytes, else BN5's
    copied at a number of the chips' and the entry's number byte set to it (h.bytes)"""
    _, table, size, _ = NUMBERED[h.entry]
    for i in sorted(numbers):
        at = table + size * i
        if not DATA_SIZES.get(table, 0) >= size * (i + 1):
            h.f.decide('number', f'the flash {i:#x}, past its table')
            continue
        lst, v = b5.u8(at) // 4, b5.u8(at + 1)
        _, s5 = X.sprite(b5, BN5_SPRITE_LISTS, lst, v)
        _, s6 = X.sprite(b6, BN6['sprites'], lst, v)
        if s5 and s5 == s6:
            continue
        if lst not in CHIP_LISTS or not s5:
            h.f.decide('sprite', f'the flash {i:#x}: its sprite {lst}:{v:#x}, BN6\'s another, ' + ('its list not grown' if s5 else 'BN5\'s not read'))
            continue
        to = place_sprite(h, b5, alloc, lst, v)
        h.bytes.append((at + 1, v, to, f'the flash {i:#x}: its sprite list {lst} {v:#x} in BN5, {to:#x} in BN6 (BN5\'s copied there)'))


# BN5's routine that sets up an object's sprite and collision from two packed words (r0: its sprite's list x 4 << 8 | its
# number, its animation << 16; r1 its collision's), BN6's twin 0x08011504 the same: a sprite loaded by number all the same
SPRITE_SETUP = 0x0800F198
SPRITE_SETUP6 = 0x08011504
SPRITE_LOAD6 = 0x080026E4   # (BN6's sprite load, BN5's 0x08002650: r1 the list's offset, r2 the number)
BN5_SPRITE_LISTS = 0x0803272C
# The grown sprite lists (src/emu/xvirus_grow.c: 1, 2, 3, 4 and 5, 256 numbers each) the chips take numbers of, from the
# top down, where the families take theirs from the bottom up (tools/xrecipe.py, LIST_NUMBERS; list 2's 0x20-0x2F the
# Navis'): one numbering, each slot the install checks to hold the fill first; a sprite BN6 keeps another of at the number
# BN5's code loads is copied there. List 3, the chips' attacks, grown for every track by the chips' (XV_LIST3)
CHIP_LISTS = (1, 2, 3, 4, 5)
CHIP_LIST_TOP = 0xFF
SPRITE_ROOM = 0x800


def sprite_loads(w, game):
    """the sprites a walk's code loads by number, in address order: (its list's offset, its number), None where made
    otherwise (the loader's r1 and r2, or the packed word SPRITE_SETUP takes in r0)"""
    load, setup = (BN5['sprite_load'], (SPRITE_SETUP, 0x08018926)) if game == 5 else (SPRITE_LOAD6, (SPRITE_SETUP6, 0x0801BE2A))
    out = []
    for site in sorted(w.bls):
        t = w.bls[site]
        if t == load:
            l, _ = T.back_value(w, site, 1)
            v, _ = T.back_value(w, site, 2)
            out.append((l, v))
        elif t in setup:
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


# BN5's other routine that sets up an object's sprite from a packed word (r0: its list x 4 << 8 | its number, a palette's
# << 24), BN6's twin 0x0801BE2A: MrkCan's mark's (0x1024)
SPRITE_SETUPS = (SPRITE_SETUP, 0x08018926)


def packed_loads(f):
    """the sprites a unit's code sets up through SPRITE_SETUPS, as the family deriver's loads (its list's offset, its number)"""
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t not in SPRITE_SETUPS:
                continue
            v, chain = T.back_value(w, site, 0)
            key = (None, None) if v is None else ((v >> 8) & 0xFF, v & 0xFF)
            f.loads.setdefault(key, []).append((site, chain))


def load_rewrite(u, site, chain, was, now, why):
    """a load's number set to BN6's: the immediate that makes it, or the packed word's low byte in its literal"""
    w = next((w for w in u.f.walks.values() if site in w.bls), None)
    if w is not None and w.bls[site] in SPRITE_SETUPS:
        lit = w.insns[chain[0][0]].lit if chain and chain[0][1] == 'ldr_lit' else None
        if lit is None:
            u.f.decide('sprite', f'{why}: its packed word made otherwise ({chain})')
            return
        word = w.lits[lit]
        u.f.numbers[lit] = (word, (word & ~0xFF) | now, why)
        return
    X.imm_for(u, chain, 2, was, now, why)


# The grown effects' ids the chips take (src/emu/xvirus_grow.c, XV_EFFECTS_MAX: BN6's 108 first, the families' from 0x6C up),
# from 0xFF down (issue #82's shared allocations: 0xD0-0xFF the chips')
CHIP_EFFECT_TOP, CHIP_EFFECT_BOTTOM = 0xFF, 0xD0


def effect_of(u, b5, b6, alloc, e):
    """BN5's effect `e` as BN6 has it: BN6's effect of the same sprite (by its bytes), animation and palette where it has one,
    else an id of the chips' (from 0xFF down) whose entry names BN5's sprite, copied (the entry and its sprite installed with
    the unit that places the sprite: xvirus.c's effects_set takes an effect's sprite from its own unit's)"""
    if e in alloc['effects']:
        return alloc['effects'][e]
    r5 = b5.bytes(BN5['effects'] + 4 * e, 4)
    lst, v = r5[0] // 4, r5[1]
    for e6 in range(BN6['neffects']):
        q = b6.bytes(BN6['effects'] + 4 * e6, 4)
        if q[2:] == r5[2:] and q[0] == r5[0] and same_sprite(b5, b6, (r5[0], r5[1]), (q[0], q[1])):
            alloc['effects'][e] = e6
            return e6
    if alloc['effect_next'] < CHIP_EFFECT_BOTTOM:
        u.f.decide('effect', f'its effect {e:#x}: the chips\' effect ids are all taken')
        return None
    if lst not in CHIP_LISTS:
        u.f.decide('effect', f'its effect {e:#x} draws sprite {lst}:{v:#x}, a list not grown')
        return None
    e6 = alloc['effect_next']
    alloc['effect_next'] -= 1
    alloc['effects'][e] = e6
    place_sprite(u, b5, alloc, lst, v)
    owner = alloc['owner'][(lst, v)]
    owner.effects.append((e, e6))
    return e6


def effect_imm(u, b5, chain, was, now):
    """the immediate the effect id's chain sets (movs, or adds8 of a byte), given BN6's id"""
    why = f'its effect: {was:#x} in BN5, {now:#x} in BN6'
    for a, form, v in chain:
        if form in ('movs', 'adds8') and v == was:
            u.more_imms.append(dict(at=a, form='THUMB_MOVS' if form == 'movs' else 'THUMB_ADDS', rd=(b5.u16(a) >> 8) & 7, rn=0, was=was,
                                    now=now, why=why))
            return
    u.f.decide('immediate', f'{why}: made by {chain}, not one instruction')


def table_art(u, b5, b6, alloc, site):
    """a kind table's sprites (TABLE_SPRITES): each row's BN6's own where the same bytes, else BN5's copied and the row's
    number byte set to the copy's (u.bytes)"""
    table, size, lo, no, rows, fixed = TABLE_SPRITES[site]
    for k in range(rows):
        at = table + size * k
        lst, v = (b5.u8(at + lo) // 4 if lo is not None else fixed), b5.u8(at + no)
        if lo is not None and b5.u8(at + lo) == 0xFF:
            continue
        _, s5 = X.sprite(b5, BN5_SPRITE_LISTS, lst, v)
        _, s6 = X.sprite(b6, BN6['sprites'], lst, v)
        if s5 and s5 == s6:
            continue
        if lst not in CHIP_LISTS or not s5:
            u.f.decide('sprite', f'{table:08X}\'s row {k}: its sprite {lst}:{v:#x}, BN6\'s another, ' + ('its list not grown' if s5 else 'BN5\'s not read'))
            continue
        to = place_sprite(u, b5, alloc, lst, v)
        u.bytes.append((at + no, v, to, f'{table:08X}\'s row {k}: its sprite list {lst} {v:#x} in BN5, {to:#x} in BN6 (BN5\'s copied there)'))


def place_sprite(u, b5, alloc, lst, v):
    """BN5's sprite `lst`:`v` at a number of the chips' in its grown list (from the top down), copied with the unit that
    places it first (the units install together)"""
    to = alloc['memo'].get((lst, v))
    if to is None:
        _, s5 = X.sprite(b5, BN5_SPRITE_LISTS, lst, v)
        to = alloc['lists'][lst]
        alloc['lists'][lst] -= 1
        alloc['memo'][(lst, v)] = to
        room = X.rounded(len(s5), SPRITE_ROOM)
        u.sprites.append(dict(list=lst, index=v, to_list=lst, to_index=to, at=alloc['art'], room=room))
        alloc['owner'][(lst, v)] = u
        alloc['art'] += room
    return to


def art_of(u, b5, b6, alloc):
    """what a unit draws: the sprites its code loads by number, each BN6's own where its list holds the same bytes at that
    number, else BN5's copied at a number of the chips' in its grown list (CHIP_LISTS, from the top down), its loads set
    to it; and its effects"""
    f = u.f
    packed_loads(f)
    for (l, v), sites in sorted(f.loads.items(), key=lambda x: str(x[0])):
        if (l is None or v is None) and all(s in MEGAMAN_LOADS for s, _ in sites):
            f.notes.append(f'MegaMan\'s own sprite at {", ".join(f"{s:08X}" for s, _ in sites)}: {MEGAMAN_LOADS[sites[0][0]]}')
            continue
        if (l is None or v is None) and all(s in TABLE_SPRITES for s, _ in sites):
            for site in sorted({s for s, _ in sites}):
                table_art(u, b5, b6, alloc, site)
            continue
        if (l is None or v is None) and all(s in TABLE_LOADS for s, _ in sites) and u.kind == 'helper':
            f.notes.append(f'its sprites by its table at {", ".join(f"{s:08X}" for s, _ in sites)}: the units\' numbers\' (helper_art)')
            continue
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
        to = place_sprite(u, b5, alloc, lst, v)
        for site, chain in sites:
            load_rewrite(u, site, chain, v, to, f'its sprite: list {lst} {v:#x} in BN5, {to:#x} in BN6 (BN5\'s copied there)')
    for e, sites in sorted(f.effects.items(), key=lambda x: str(x[0])):
        if e is None and all(s in TABLE_EFFECTS for s, _ in sites):
            for site in sorted({s for s, _ in sites}):
                table, size, off, rows = TABLE_EFFECTS[site]
                for k in range(rows):
                    at = table + size * k + off
                    e5 = b5.u8(at)
                    r5, r6 = b5.bytes(BN5['effects'] + 4 * e5, 4), b6.bytes(BN6['effects'] + 4 * e5, 4) if e5 < BN6['neffects'] else b''
                    if r5 == r6:
                        continue
                    e6 = effect_of(u, b5, b6, alloc, e5)
                    if e6 is not None and e6 != e5:
                        u.bytes.append((at, e5, e6, f'{table:08X}\'s row {k}: its effect {e5:#x} in BN5, {e6:#x} in BN6'))
            continue
        if e is None:
            f.decide('effect', f'an effect id made otherwise at {", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        r5 = b5.bytes(BN5['effects'] + 4 * e, 4)
        r6 = b6.bytes(BN6['effects'] + 4 * e, 4) if e < BN6['neffects'] else b''
        if r5 == r6:
            continue   # (BN6's entry the same sprite's number, animation and palette: BN6's own, its art the engine's, as redrawn)
        e6 = effect_of(u, b5, b6, alloc, e)
        if e6 is None:
            continue
        for site, chain in sites:
            effect_imm(u, b5, chain, e, e6)


# ---- the C files ----

KIND_C = {'attack': 'XCHIP_ATTACK', 'support': 'XCHIP_SUPPORT', 'navi': 'XCHIP_NAVI', 'helper': 'XCHIP_HELPER'}


def c_name(label):
    import re
    return re.sub(r'[^a-z0-9]', '', label.lower())


def c_unit(u, slot):
    n = c_name(u.label)
    L = [f'/* ---- {u.label}: BN5\'s {u.kind} {u.n5:#x} ({u.entry | 1:08X}) as BN6\'s {u.number:#x} ---- */' if u.kind != 'helper' else
         f'/* ---- {u.label}: BN5\'s {u.entry | 1:08X}, a helper (installed once, before the units) ---- */', '']
    L.append(f'static const XvBlock {n}_blocks[] = {{')
    for lo, hi, at, crc in u.blocks:
        base = 'XC_HELPERS' if u.kind == 'helper' else 'XC_CODE'
        L.append(f'\t{{ 0x{lo:08X}, 0x{hi:08X}, {base} + 0x{at - (XC_HELPERS if u.kind == "helper" else XC_CODE):X}, 0x{crc:08X}u }},')
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
    if u.bytes:
        L.append(f'static const XvByte {n}_bytes[] = {{')
        L += [f'\t{{ 0x{at:08X}, 0x{was:02X}, 0x{now:02X} }},   /* {why} */' for at, was, now, why in u.bytes]
        L.append('};')
    if u.types:
        L.append(f'static const XvType {n}_types[] = {{ ' + ', '.join(
            f'{{ {k}, 0x{t5:02X}, {"XV_TYPE_MOVED" if t6 is None else f"0x{t6:02X}"} }}' for k, t5, t6, _ in u.types) + ' };')
    if u.songs:
        L.append(f'static const XcSong {n}_songs[] = {{ ' + ', '.join(f'{{ 0x{v:X}, 0x{slot:02X} }}' for v, slot in u.songs) + ' };')
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    fields = [f'.{a} = {n}_{a}, .n{a} = {count(a)}' for a, have in (('blocks', True), ('data', u.data), ('calls', True), ('words', u.words),
                                                                   ('imms', u.imms), ('numbers', u.numbers), ('sprites', u.sprites),
                                                                   ('effects', u.effects), ('types', u.types), ('bytes', u.bytes)) if have]
    L += ['', f'#define {n.upper()}_CODE {{ .name = "{u.label} (BN5\'s)", .game = XROM_BN5_COLONEL_US, \\']
    L += [f'\t{f}, \\' for f in fields[:-1]] + [f'\t{fields[-1]} }}']
    songs = f'{n}_songs, {count("songs")}' if u.songs else 'NULL, 0'
    return '\n'.join(L), (f'\t{{ {KIND_C[u.kind]}, 0x{u.n5 & 0xFF if u.kind != "helper" else 0:02X}, 0x{u.number:02X}, 0x{u.entry | 1:08X}, XC_HOMES + 0x{XC_HOME_SIZE * slot:X}, '
                          f'{n.upper()}_CODE, {songs} }},')


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
        if u.kind == 'helper':
            continue
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
        head = f'BN5\'s {u.kind} {u.n5:#x} ({u.entry:08X}) as BN6\'s {u.number:#x}' if u.kind != 'helper' else f'BN5\'s {u.entry:08X}, a helper'
        lines += [f'## {u.label}: {head}', '',
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


def dispatch_disagrees(units):
    """each object type the units spawn through the dispatch, as they derive it (MOVED, or BN6's of the same number: TWIN),
    against bn5_twins.DISPATCH, one decision per type for every track: where none is written or another, what to write"""
    import bn5_twins
    out = []
    for u in units:
        for kind, t5, t6, why in u.types:
            got = 'MOVED' if t6 is None else 'TWIN' if t6 == t5 else f'TWIN as {t6:#x}'
            want = bn5_twins.DISPATCH.get((kind, t5))
            if want is None or want[0] != got:
                out.append(f'{u.label}: the {kind_name(kind)} kind\'s {t5:#x} {got} ({why})' + (f', where it says {want[0]}' if want else ''))
    return out


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
    alloc = dict(code=XC_CODE, hcode=XC_HELPERS, art=XC_ART, lists={k: CHIP_LIST_TOP for k in CHIP_LISTS}, memo={}, owner={},
                 effects={}, effect_next=CHIP_EFFECT_TOP)
    # (the helpers first, their copies' places for the units' calls; their art after the units', by what the units use)
    helpers, helper_at = [], {}
    for root, label, _ in HELPERS:
        h = derive(b5, b6, pairs, 'helper', root, label, alloc, 0, helper_at)
        helpers.append(h)
        # (every routine of its, for the units' calls: its root and those it calls)
        for e in h.f.walks:
            at = next((at + (e - lo) for lo, hi, at, _ in h.blocks if lo <= e < hi), None)
            if at is not None:
                helper_at[e] = at
    order = list(UNITS) + list(CANDIDATES) + [(k, int(n, 0), name) for k, n, name in (x.split(':') for x in a.unit)]
    units = []
    for k, (kind, n, label) in enumerate(order):
        if k == len(UNITS):
            flash_art(helpers, units, b5, b6, alloc)
        units.append(derive(b5, b6, pairs, kind, n, label, alloc, number_of(kind, n), helper_at))
    if len(order) == len(UNITS):
        flash_art(helpers, units, b5, b6, alloc)
    units = helpers + units
    report(units, os.path.join(WORK, 'report.md'))
    print(f'xchiprecipe: {len(units)} units derived, {sum(1 for u in units if not u.f.decisions)} with no decision open; '
          f'{os.path.join(WORK, "report.md")}')
    if a.report:
        return 0
    whole = units[:len(HELPERS) + len(UNITS)]
    for u in whole:
        if u.f.decisions:
            sys.exit(f'xchiprecipe: {u.label} has decisions open: ' + '; '.join(f'{k}: {d}' for k, d in u.f.decisions))
    undecided = dispatch_disagrees(whole)
    if undecided:
        sys.exit('xchiprecipe: the dispatch\'s decisions (tools/bn5_twins.py, DISPATCH) want: ' + '; '.join(undecided))
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
