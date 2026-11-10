#!/usr/bin/env python3
"""xchiprecipe_bn4.py [--check] [--report] [--unit KIND:SUB:NAME ...]: BN4 Red Sun's chips' code as units for BN6's battle
engine, made from BN4's ROM and BN6 Gregar's, as tools/xchiprecipe.py makes BN5's (docs/devtools/chip-recipe-generator.md,
The BN4 path; docs/multirom/bn4-chips.md, The build plan's units 3 and 5).

BN4 runs a chip by a state of MegaMan's (BN4_PLAYER_STATES by the record's family): its support state (family 0x0C,
BN4_SUPPORT_RUN) runs its support chips and its Navi chips from one table of routines by subfamily (BN4_CHIP_ROUTINES),
where BN6 has two. A unit here is one of those routines: a spawner of a fourth-kind object, in that object's type's region,
of the shape BN6's support routines have (the element's place apart). The unit is that region (the type's routine, its
states, the spawner), walked with tools/xrecipe_bn4.py's parts as a moved family's objects are: every routine it reaches
paired with BN6's twin (tools/bn4_twins.py, the Navis' pairs, and TWINS below; else a BN6 routine of the same body), a
third- or fourth-kind type it spawns that BN6 has no twin of moved with it through the second-level dispatch, its
immediates BN6's (the element's place, the collision data's offsets, a panel's fields, its sounds and sprites). Its entry
is the spawner's copy, at BN6's grown support routines' NUMBER0 + its subfamily (src/emu/xchips_code.h XC_SUPPORT; BN5's
are at 0x2A + theirs), which its chips' records take under BN6's support family (the registry's XREG_MOVED rows, their
`sub`: tools/xreg_bn4.py MOVED, checked here).

  python3 tools/xchiprecipe_bn4.py           write src/emu/xchips_bn4.c (UNITS, each whole) and .build/xchiprecipe/bn4_report.md
  python3 tools/xchiprecipe_bn4.py --check   write nothing: fail where src/emu/xchips_bn4.c is not what it would write
  python3 tools/xchiprecipe_bn4.py --report --unit support:0x19:ShotStar   derive more units for the report alone

Local only: both ROMs. Nothing of either is written: addresses, counts and CRC32s."""
import argparse
import os
import re
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_twins  # noqa: E402
import xnavirecipe_bn4_objects as C  # noqa: E402
import xrecipe_bn4 as X4  # noqa: E402
import xrecipe_thumb as T  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_C = os.path.join(ROOT, 'src', 'emu', 'xchips_bn4.c')
WORK = os.path.join(ROOT, '.build', 'xchiprecipe')
REGISTRY = os.path.join(ROOT, 'src', 'core', 'xchips_registry_bn4.c')
EMU = os.path.join(ROOT, 'src', 'emu')

# ---- both games' support chips (src/emu/bn4_chips.h, bn6_chips.h; docs/rom/bn4.md, Its chips' code) ----
ROUTINES4, NROUTINES4 = 0x080220E0, 0x4E   # BN4_CHIP_ROUTINES: its support state's routines by subfamily, Navi chips among them
ROUTINES6, NROUTINES6 = 0x0802CCB4, 0x2A   # BN6_CHIP_ROUTINES, BN6_CHIP_ROUTINE_COUNT
RECORDS4, NRECORDS4, RECORD = 0x080197EC, 389, 0x2C   # BN4_CHIP_RECORDS: +0xB its family, +0xC its subfamily
FAMILY4_SUPPORT, FAMILY6_SUPPORT = 0x0C, 0x15
# ... and its misc chips: MegaMan's state 0x20 (BN4_MISC_RUN) runs a routine by subfamily and ends the attack at once, BN6's
# family 0x1C (BN6_MISC_RUN) the same instruction for instruction; a 'misc' unit is one of BN4's routines there, whatever
# its shape, at BN6's grown misc routines' NUMBER0 + its subfamily (src/emu/xchips_bn4.h XC4_MISC)
MISC4, NMISC4, MISC6, NMISC6 = 0x080EC9CC, 0x12, 0x080ED730, 7   # BN4_MISC_ROUTINES, BN6_MISC_ROUTINES
FAMILY4_MISC, FAMILY6_MISC = 0x20, 0x1C
FAMILY4 = {'support': FAMILY4_SUPPORT, 'misc': FAMILY4_MISC}
SPAWN4_4, SPAWN6_4 = 0x08003344, 0x080033AC   # BN4_SPAWN_4, BN6's fourth-kind spawner (r0 the type)
# BN6's support routines' shape, Invisibl's (0x080E8886): the object spawned and its panel, element, parent, side, r6 and r7
# written; BN4's the same instruction for instruction but for the element's place (ELEMENT4, ELEMENT6: the scan's immediate)
# (the walker's form of a conditional branch is 'bcc', whatever its condition)
SHAPE = ('push', 'push', 'movs', 'bl', 'adds3', 'pop', 'bcc', 'strb', 'strb', 'strb', 'str', 'ldrb', 'strb', 'str', 'str', 'pop')
SHAPE6 = 0x080E8886
# The unit's number in BN6's grown support routines (XC_SUPPORT, 256): BN6's own 0-0x29, BN5's at 0x2A + its subfamily (to
# 0x7B), BN4's at NUMBER0 + its subfamily
NUMBER0 = 0x80
# The generic states of a support chip's object (the chip's name shown and the time freeze's turn taken, then its own, then
# its end), which every such type's tables name: BN6's of the same place in its own types' tables (Invisibl's 0x5D:
# 0x080E8828, 0x080E8848), each pair read in both ROMs (AUDITS)
TWINS = {
    0x08008BD8: (0x0800B916, 'chips'),   # its state 0 (+8): its turn asked for
    0x08008EEE: (0x0800BD34, 'chips'),   # its state 8 (+8): its turn given back, the object freed
    0x08008BF8: (0x0800B94C, 'chips'),   # its step 0 (+9): the other side's turn waited out (the same body)
    0x08008C2A: (0x0800B9B0, 'chips'),   # its step 4 (+9): the chip's name shown
    0x08008EB8: (0x0800BC88, 'chips'),   # its step 12 (+9): its own work done, its state 8
    0x08009BAC: (0x0800C9F8, 'chips'),   # a panel broken where it stands free (PanlSht's shot's own panel)
}
AUDITS = {
    (0x08008BD8, 0x0800B916): ('ok', "a support chip's object's state 0: its side's turn of the time freeze asked for and the state "
                                     "set to 4; BN6's the same place in its types' tables (Invisibl's 0x5D, 0x080E8828), with BN6's "
                                     "own bookkeeping of its freeze (its flag 0x40, its side's word at 0x0200F3B8), which BN6's "
                                     "engine reads and BN4's has none of: BN6's object's protocol, as bn4_chip_invis runs it"),
    (0x08008EEE, 0x0800BD34): ('ok', "a support chip's object's state 8: its turn given back (the other side's waiting object set "
                                     "going, the freeze ended) and the object freed, instruction for instruction BN6's with BN6's "
                                     "own clearing of its side's word (+0xC of its freeze's record) added"),
    (0x08008C2A, 0x0800B9B0): ('ok', "a support chip's object's step 4: the chip's name shown for its side (the chip's id at +0x30, "
                                     "its power at +0x2C and +0x32 where its record shows one) and the step moved on by 4 or 8 on "
                                     "the same tests; BN6's shows it by its own text routine and plays its sound 0x173 "
                                     "(tests/xdiff/legit.json's bn4 section, BN6's sound as a support chip's name shows)"),
    (0x08009BAC, 0x0800C9F8): ('ok', "a panel (r0, r1) of a floor (0x10) with none of its occupants' flags broken (type 1, its flags' "
                                     "low bits 1, the crack's sound), 1 back, else 0; BN6's the same at its offsets (flags +0x14, "
                                     "type +2 and its drawn type +6) and its own sound, but BN4's test of flag 0x20000 first, "
                                     "which only BN4's type 5 gives (BN4_PANEL_FLAGS_OF 0x30010; BN6's occupants' mask lacks that "
                                     "bit likewise): no BN6 panel type's word has it, so the test never holds in BN6"),
}
# The dispatch's numbers the chips' moved types take, by kind: the fourth kind's from 0x91 up, past BN5's 145 types (its own
# numbers in the tables) and below BN4's families' and Navis' (0xE0 on); the third kind's numbers no BN5 track names (BN5's
# own are 0-0xCB, the families' and Navis' 0xCC on, 11 left between them: tools/xrecipe_bn4.py TYPE_MORE, LATE_TYPES),
# each checked against every other recipe (numbers_clear)
TYPES4 = (0x91, 0xE0)
TYPES3 = (0xCA, 0xC9, 0xC8, 0xC6, 0xC5, 0xC4, 0xC3, 0xC2, 0xBE, 0xBD, 0xBB, 0xB9, 0xB6, 0xB5, 0xB4, 0xB0)
# The units written, in install order: (kind, BN4's subfamily, a name); CANDIDATES for the report alone
UNITS = (('support', 0x26, 'PanlSht'), ('support', 0x19, 'ShotStar'), ('misc', 0x0B, 'Repair'))
CANDIDATES = ()
# BN4's sounds a unit plays that BN6 has no song of (xrecipe.song_sig), each copied into a slot of BN6's grown song table
# fixed for it (the owner's call 6, docs/multirom/bn4-chips.md: src/emu/song_table.h, slots from SONG_GROWN_FIRST up in
# the order the units took them; src/audio/xsong.c XSOUNDS, the same numbers; xsong_install_at as the unit installs), one
# slot a sound whichever unit plays it. A sound not here is a decision: the next slot
SOUND_SLOT0, SOUND_SLOT_END = 0x1DA, 0x400   # (src/emu/song_table.h SONG_GROWN_FIRST, SONG_GROWN_END)
SOUND_SLOTS = {0x1C6: 0x1DA, 0xBF: 0x1DB,   # (PanlSht's shot's)
               0x146: 0x1DC}                # (ShotStar's star's fall)
# ... and a sound a BN4 family plays too, at the family's fixed slot (tools/xrecipe_bn4.py SOUND_SLOTS, xsong.c's XSOUNDS:
# one slot a sound whichever track copies it), a byte a movs names
FAMILY_SLOTS = {0x70: 0x3A,                 # (ShotStar's star's burst: Walla's)
                0x9A: 0x39}                 # (Repair's: Walla's)
# The kinds' tables a moved object reads (as tools/bn4_twins.py TABLES: its size, a row's size, the kinds the unit spawns,
# where a row keeps its sprite's number, the sprite's list and the load that reads it): copied with the unit, each spawned
# kind's sprite BN6's of the same bytes or copied at a number of the chips' (its byte in the copy set)
TABLES = {
    0x080D8580: dict(size=42, entry=6, kinds=(1,), sprite=(None, 0), list=3, loads={0x080D85E4},
                     why="the star's kinds (the third kind's 0x56: 6 bytes each, its sprite's number in list 3, its animation, its "
                         "collision's two arguments), read by its init (0x080D85D2, by +4); ShotStar's object spawns kind 1 "
                         "(0x080E5D3E: movs r4,#1)"),
}
# The numbers the chips' copies take, each from the top of its range down (the late families' way, tools/xrecipe_bn4.py):
# effect ids 0xCF-0xB0 (below BN5's chips' 0xD0 on, above BN4's Navis' from 0xA0 up), list 5's 0xEF-0xB0, list 4's 0x9F-0x70,
# and list 3's from 0x70 up to 0xDF (past BN6's own 105, below BN5's chips' 0xE4 on)
CHIP_EFFECTS, CHIP_LIST5, CHIP_LIST4, CHIP_LIST3 = (0xCF, 0xB0), (0xEF, 0xB0), (0x9F, 0x70), (0x70, 0xE0)
ROOM_SPRITE = 0x100
# An immediate of a unit's code that names a place of BN4's engine BN6 keeps elsewhere, read by hand in both games' code:
# BN4's address -> (the instruction's form, its register, BN4's number, BN6's, why)
IMMS = {
    0x0800C728: ('THUMB_ADDS', 0, 0x68, 0xA0,
                 "the battle state's obstacles by side: BN4 +0x68, BN6 +0xA0 (BN4_OBSTACLE_ADD 0x0800B230 and BN6_OBSTACLE_ADD "
                 "0x0800F614, the same routine but for that adds; three words a side, 0xC apart in both)"),
}
# A call into another object type's code whose BN6 twin the search found by its body alone: the two types compared to their
# depth by the differential or by hand before the pair is trusted (the census's lesson: docs/multirom/bn4-chips.md, The six
# "on BN6's code" are not): BN4's routine -> why BN6's twin serves. One not here is a decision
TYPE_TWINS = {}
# A panel's fields a moved object reads after BN4's getPanel (bn4.h BN4_PANEL_*): BN4's offset -> BN6's
PANEL4TO6_FIELDS = {0x01: 0x03}

# ---- where they go (src/emu/xchips_bn4_code.h; src/emu/free_space.plan FREE_BN4_CHIPS) ----
EMU_FREE = 0x08800000
X4C_AT = EMU_FREE + 0x0EF000
X4C_HOMES, X4C_HOME_SIZE, X4C_UNITS = X4C_AT, 0x400, 96
X4C_CODE = X4C_HOMES + X4C_HOME_SIZE * X4C_UNITS
X4C_ART = X4C_AT + 0x30000
X4C_END = X4C_AT + 0x50000


class Alloc(X4.Alloc):
    """the chips' numbers and room: their code and art in their own region, their types' numbers the chips' own"""

    def __init__(self):
        super().__init__()
        self.code, self.code_end, self.art, self.art_end, self.room = X4C_CODE, X4C_ART, X4C_ART, X4C_END, 'BN4\'s chips'
        self.types = {1: 0x100, 3: 0x100, 4: TYPES4[0]}
        self.types3 = list(TYPES3)
        self.late_effect, self.late_list5, self.late_list4, self.list3 = CHIP_EFFECTS[0], CHIP_LIST5[0], CHIP_LIST4[0], CHIP_LIST3[0]
        self.list3_sprites = {}   # (BN4's list 3 number copied: the copy)


def install(pairs):
    pairs.twins.update({a: b for a, (b, _) in TWINS.items()})
    pairs.twin_src.update({a: 'project:' + s for a, (_, s) in TWINS.items()})
    bn4_twins.AUDITS.update(AUDITS)


def chips_of(b4, kind, sub):
    """BN4's chip records that run the unit: (id, its record)"""
    out = []
    for i in range(1, NRECORDS4):
        r = b4.bytes(RECORDS4 + RECORD * i, RECORD)
        if r[0xB] == FAMILY4[kind] and r[0xC] == sub and any(c != 0xFF for c in r[0:4]):
            out.append((i, r))
    return out


def shape_of(b4, b6, f, entry):
    """the unit's routine: BN6's support routines' shape (SHAPE6 the control), the fourth-kind type it spawns"""
    w, w6 = T.walk(b4, entry), T.walk(b6, SHAPE6)
    forms = lambda x: tuple(x.insns[a].form for a in sorted(x.insns))
    if forms(w6) != SHAPE:
        sys.exit(f'xchiprecipe_bn4: BN6\'s support routine at {SHAPE6:08X} is not the shape this tool reads')
    if forms(w) != SHAPE or [t for _, t in sorted(w.bls.items())] != [SPAWN4_4]:
        f.decide('entry', f'its routine ({entry:08X}) is not the common spawner (BN6\'s {SHAPE6:08X}): another kind of unit')
        return None
    site = next(iter(w.bls))
    ty, chain = T.back_value(w, site, 0)
    if ty is None or len(chain) != 1 or chain[0][1] != 'movs':
        f.decide('entry', f'its routine ({entry:08X}) spawns a type made otherwise')
        return None
    return ty


def type_number(f, alloc, kind):
    """the dispatch's next number of a kind for a chip's moved type"""
    if kind == 3:
        if not alloc.types3:
            f.decide('type', 'the chips\' third-kind dispatch numbers (TYPES3) are spent')
            return 0
        return alloc.types3.pop(0)
    n = alloc.types[kind]
    alloc.types[kind] += 1
    if kind != 4 or n >= TYPES4[1]:
        f.decide('type', f'the chips\' dispatch numbers of the {kind} kind are spent ({n:#x} wanted)')
        return 0
    return n


def derive(b4, b6, pairs, kind, sub, name, alloc, slot):
    f = X4.Family(0, name, 0)
    f.late = True   # (its copies' numbers from the top of the chips' ranges down: CHIP_EFFECTS, CHIP_LIST5, CHIP_LIST4)
    f.kind, f.sub, f.number, f.slot = kind, sub, NUMBER0 + sub, slot
    f.b4, f.b6, f.home = b4, b6, X4C_HOMES + X4C_HOME_SIZE * slot
    f.update, f.type_table, f.st, f.own, f.dead, f.copied = None, 0, 0, [], [], []
    f.chips = chips_of(b4, kind, sub) if kind in FAMILY4 else []
    if kind not in FAMILY4 or not 0 <= sub < (NROUTINES4 if kind == 'support' else NMISC4):
        f.decide('unit', f'a {kind} unit {sub:#x}: the support and misc states\' routines alone are read')
        return f
    f.regions, f.regions_roots, f.moved4 = [], [], {}
    saved_copied = dict(bn4_twins.COPIED)
    if kind == 'support':
        f.entry = b4.u32(ROUTINES4 + 4 * sub) & ~1
        hit = C.t4_of(b4, f.entry)
        ty = shape_of(b4, b6, f, f.entry)
        if ty is None or hit is None or hit[0] != ty:
            f.decide('entry', f'its routine ({f.entry:08X}) does not lie in the region of the type it spawns')
            return f
        f.ty, (f.lo, f.hi) = ty, hit[1:]
        f.regions, f.roots = [(f.lo, f.hi, ('4', ty))], [f.entry, f.lo]
        f.moved4 = {ty: dict(lo=f.lo, hi=f.hi, routine=f.lo)}
        f.notes.append(f'its fourth-kind {ty:#x} ({f.lo:08X}-{f.hi:08X}), which its routine ({f.entry:08X}) spawns: moved')
    else:
        f.entry = b4.u32(MISC4 + 4 * sub) & ~1
        hit = X4.t3_of(b4, f.entry) or C.t4_of(b4, f.entry)
        if hit is not None:
            f.decide('entry', f'its routine ({f.entry:08X}) lies in an object type\'s code ({hit[1]:08X}-{hit[2]:08X}): that type\'s move with a '
                              f'misc unit is not read yet')
            return f
        # (a routine of BN4's engine by itself: copied, a block of its own)
        bn4_twins.COPIED[f.entry] = 'a misc chip\'s routine, the unit\'s entry'
        f.roots = [f.entry]
        f.notes.append(f'its routine ({f.entry:08X}), a routine of BN4\'s by itself: copied')
    try:
        roots = list(f.roots)
        while roots:
            X4.walk_code(b4, f, roots, set(), pairs)
            roots = sorted((C.moved4(b4, f, pairs) | moved3(b4, f, pairs)) - set(f.walks))
    finally:
        bn4_twins.COPIED.clear()
        bn4_twins.COPIED.update(saved_copied)
    site_imms(f)
    X4.side_sites(f)
    X4.scan(f)
    X4.battle_players(f)
    saved = X4.type_number
    X4.type_number = type_number
    try:
        X4.spawns_of(f, alloc)
        C.spawns4(f, alloc)
    finally:
        X4.type_number = saved
    X4.pair_calls(f, pairs)
    shallow_twins(b4, f)
    panel_fields(f)
    X4.queued_sites(f)
    saved_tables = dict(bn4_twins.TABLES)
    saved_late = X4.LATE_EFFECTS, X4.LATE_LIST5, X4.LATE_LIST4
    bn4_twins.TABLES.update(TABLES)
    X4.LATE_EFFECTS, X4.LATE_LIST5, X4.LATE_LIST4 = CHIP_EFFECTS, CHIP_LIST5, CHIP_LIST4
    try:
        X4.blocks_of(b4, f, alloc)
        X4.words_of(f, pairs)
        sprites_of(b4, b6, f, alloc)
        table_sprites(b4, b6, f, alloc)
        X4.effects_of(b4, b6, f, alloc)
    finally:
        bn4_twins.TABLES.clear()
        bn4_twins.TABLES.update(saved_tables)
        X4.LATE_EFFECTS, X4.LATE_LIST5, X4.LATE_LIST4 = saved_late
    sounds_of(b4, b6, f)
    if f.sides or f.ranges:
        f.decide('unit', 'it needs a shim the chips\' emitter writes none of yet (a side, a NameID range)')
    rows_check(b4, f)
    f.imms.sort(key=lambda x: x['at'])
    seen, out = set(), []
    for i in f.imms:
        if i['at'] not in seen:
            seen.add(i['at'])
            out.append(i)
    f.imms = out
    return f


def moved3(b4, f, pairs):
    """its calls into a third-kind type's code that reach that type's spawn through a wrapper (tools/xrecipe_bn4.py's moved3_of
    reads the spawner alone), where no pair of the project's nor TYPE_TWINS answers for BN6's: the type moved with the unit,
    its region one of its blocks; the new roots"""
    roots = set()
    for t in sorted(f.calls):
        hit = None if t in X4.SPAWNERS4 or t in pairs.twins or t in TYPE_TWINS else X4.t3_of(b4, t)
        if hit is None or hit[0] is None or hit[0] in f.moved3:
            continue
        ty, lo, hi = hit
        seen, todo, spawns = set(), [t], False
        while todo and not spawns:   # (the routine called and what it calls in the type's region: a spawn of the type)
            e = todo.pop()
            if e in seen:
                continue
            seen.add(e)
            w = T.walk(b4, e, region=(lo, hi))
            spawns = any(s in X4.SPAWNERS4 and T.back_value(w, a, 0)[0] == ty for a, s in w.bls.items())
            todo += [s for s in w.bls.values() if lo <= s < hi]
        if not spawns:
            continue
        f.moved3[ty] = dict(lo=lo, hi=hi, routine=lo)
        f.regions.append((lo, hi, ty))
        f.notes.append(f'its third-kind {ty:#x} ({lo:08X}-{hi:08X}), spawned through {t:08X}: no BN6 type proved its twin, moved')
        del f.calls[t]
        roots |= {t, lo}
    return roots


def site_imms(f):
    """the immediates of its code IMMS names, each checked against the instruction there"""
    for e, w in sorted(f.walks.items()):
        for a, i in sorted(w.insns.items()):
            if a not in IMMS:
                continue
            form, rd, was, now, why = IMMS[a]
            if i.form != {'THUMB_ADDS': 'adds8', 'THUMB_MOVS': 'movs'}.get(form) or i.f[0] != rd or i.f[1] != was:
                f.decide('immediate', f'{a:08X}: IMMS names {form} r{rd}, #{was:#x} there, the code holds {i.text}')
                continue
            f.imms.append(dict(at=a, form=form, rd=rd, rn=0, was=was, now=now, why=why))


def shallow_twins(b4, f):
    """a call into another object type's code paired by the search alone (TYPE_TWINS)"""
    for t, tw, how, cls, sites, audited in f.pairs:
        if how != 'search' or isinstance(tw, str) or tw is None:
            continue
        hit = X4.t3_of(b4, t) or C.t4_of(b4, t)
        if hit is not None and t not in TYPE_TWINS:
            f.decide('type', f'{t:08X} -> {tw:08X}: a routine of another object type\'s code ({hit[1]:08X}-{hit[2]:08X}) paired by its body '
                             f'alone: its type compared to its depth first (TYPE_TWINS), or moved')


def panel_fields(f):
    """a panel's side read after BN4's getPanel: BN6's offset (PANEL4TO6_FIELDS; its type's, +0, is the scan's)"""
    for e, w in sorted(f.walks.items()):
        addrs = sorted(w.insns)
        for k, a in enumerate(addrs):
            i = w.insns[a]
            if i.form != 'bl' or i.target != bn4_twins.GET_PANEL:
                continue
            for b in addrs[k + 1:k + 6]:
                j = w.insns[b]
                if j.form in ('ldrb', 'strb') and j.f[1] == 0 and j.f[2] in PANEL4TO6_FIELDS:
                    to = PANEL4TO6_FIELDS[j.f[2]]
                    f.imms.append(dict(at=b, form=X4.FORMS[j.form], rd=j.f[0], rn=0, was=j.f[2], now=to,
                                       why=f'a panel\'s side: BN4 +{j.f[2]:#x}, BN6 +{to:#x}'))
                if j.form == 'bl' or T.writes(j, 0):
                    break


def sounds_of(b4, b6, f):
    """the sounds its code plays (BN4_PLAY_SOUND, r0 the sound): each BN6's of the same number where it is the same song (how
    it sounds, xrecipe.song_sig); else BN4's song copied into its slot of BN6's grown song table (SOUND_SLOTS) and the call
    made through a shim of the unit's home that makes r0 the slot, a number past what a movs names (XvSoundCall), the
    instructions that made BN4's number left as they are"""
    ids = {}
    for e, w in f.walks.items():
        for site, t in w.bls.items():
            if t == X4.BN4['sound']:
                ids.setdefault(T.back_value(w, site, 0)[0], []).append(site)
    if None in ids:
        f.decide('sound', f'a sound played at {", ".join(f"{s:08X}" for s in ids[None])} made otherwise')
    odd = sorted(v for v in ids if v is not None and X4.song_sig(b4, X4.BN4['songs'], v) != X4.song_sig(b6, X4.SONGS6, v))
    same = sorted(v for v in ids if v is not None and v not in odd)
    if same:
        f.notes.append(f'its sounds {", ".join(f"{v:#x}" for v in same)}: BN6\'s of the same numbers, the same songs')
    f.sounds = []
    for v in odd:
        if v in FAMILY_SLOTS:
            slot = FAMILY_SLOTS[v]
            f.sounds.append((v, slot))
            for e, w in sorted(f.walks.items()):
                for site, t in sorted(w.bls.items()):
                    chain = T.back_value(w, site, 0)[1] if t == X4.BN4['sound'] and site in ids[v] else None
                    if chain is None:
                        continue
                    if len(chain) != 1 or chain[0][1] != 'movs':
                        f.decide('sound', f'its sound {v:#x} at {site:08X} named otherwise than by a movs ({chain})')
                        continue
                    f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=v, now=slot,
                                       why=f'its sound {v:#x}: BN4\'s copied into BN6\'s slot {slot:#x}, a BN4 family\'s too'))
            f.notes.append(f'its sound {v:#x}: BN4\'s song at slot {slot:#x}, the families\' slot for it (FAMILY_SLOTS)')
            continue
        slot = SOUND_SLOTS.get(v)
        if slot is None:
            f.decide('sound', f'its sound {v:#x} is no song of BN6\'s and has no slot (SOUND_SLOTS: the next of the grown table\'s)')
            continue
        if not SOUND_SLOT0 <= slot < SOUND_SLOT_END:
            f.decide('sound', f'its sound {v:#x}\'s slot {slot:#x} is none of the grown table\'s')
            continue
        f.sounds.append((v, slot))
        player = b4.u16(X4.BN4['songs'] + 8 * v + 4)
        for site in sorted(ids[v]):
            f.queue_calls.append(dict(at=site, slot=slot, why=f'its sound {v:#x}: BN4\'s song at the grown table\'s slot {slot:#x}'))
        f.notes.append(f'its sound {v:#x} ({", ".join(f"{s:08X}" for s in sorted(ids[v]))}): BN4\'s song copied into the grown song table\'s '
                       f'slot {slot:#x}, on BN4\'s player {player}')


def registry_rows():
    """BN4's rows of the chip registry: BN4's id -> (BN6's id, how, sub)"""
    rows = {}
    with open(REGISTRY, encoding='utf-8') as fh:
        for m in re.finditer(r'\{\s*(\d+),\s*(\d+),\s*XREG_(\w+),\s*(\d+),\s*(0x[0-9A-Fa-f]+|\d+),', fh.read()):
            rows[int(m.group(1))] = (int(m.group(2)), m.group(3), int(m.group(5), 0))
    return rows


def rows_check(b4, f):
    """its chips' rows: each XREG_MOVED with the unit's number its subfamily (tools/xreg_bn4.py MOVED), none a dark chip
    (docs/game/meta/darkchips.md: BN4's dark chips sit out until their price is wired)"""
    rows = registry_rows()
    f.rows = []
    for i, r in f.chips:
        if r[0x15] == 2:
            f.decide('row', f'its chip {i} is a dark chip (its record\'s +0x15 is 2): not to play before its 20 max HP is wired')
        if i not in rows:
            f.notes.append(f'its chip {i} has no row of the registry (the run\'s by name, or none)')
            continue
        bn6, how, sub = rows[i]
        f.rows.append((i, bn6))
        if how != 'MOVED' or sub != f.number:
            f.decide('row', f'its chip {i}\'s row (id {bn6}) is XREG_{how} with subfamily {sub:#x}, not XREG_MOVED with {f.number:#x}: '
                            f'tools/xreg_bn4.py MOVED')


def sprites_of(b4, b6, f, alloc):
    """the sprites its moved objects load (BN4's sprite load, r1 the list's offset, r2 the number, each a movs): BN6's of the
    same bytes in the same list where it has one (the movs given its number), else a decision"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != X4.BN4['sprite_load'] or any(site in x['loads'] for x in TABLES.values()):
                continue
            lst, _ = T.back_value(w, site, 1)
            num, chain = T.back_value(w, site, 2)
            if lst is None or lst & 3 or num is None or len(chain) != 1 or chain[0][1] != 'movs':
                f.decide('sprite', f'{site:08X}: a sprite loaded by numbers made otherwise ({lst}, {num})')
                continue
            tw = X4.bn6_sprite_twin(b4, b6, lst // 4, num)
            if tw is None:
                f.decide('sprite', f'{site:08X}: its sprite {lst // 4}:{num:#x} is no sprite of BN6\'s list {lst // 4}: to be copied')
                continue
            f.notes.append(f'its sprite {lst // 4}:{num:#x} ({site:08X}): BN6\'s {lst // 4}:{tw:#x}, the same bytes')
            if tw != num:
                f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=2, rn=0, was=num, now=tw,
                                   why=f'its object\'s sprite: list {lst // 4} {num:#x} in BN4, {tw:#x} in BN6 (the same bytes)'))


def list3_copy(b4, f, alloc, num):
    """BN4's list 3 sprite `num` copied at a list 3 number of the chips', once for every unit: the copy, None unread or spent"""
    if num in alloc.list3_sprites:
        s, who = alloc.list3_sprites[num]
        if who != f.name:
            f.notes.append(f'its sprite 3:{num:#x}: list 3 {s["to_index"]:#x}, as {who} copied it')
        return s
    p, raw = X4.sprite(b4, X4.BN4['sprites'], 3, num)
    if raw is None or alloc.list3 >= CHIP_LIST3[1]:
        return None
    s = dict(list=3, index=num, to_list=3, to_index=alloc.list3, at=alloc.art, room=X4.rounded(len(raw), ROOM_SPRITE), lz=bool(p & 0x80000000))
    alloc.list3 += 1
    alloc.art += s['room']
    alloc.list3_sprites[num] = (s, f.name)
    f.attack_sprites.append(s)
    if alloc.art > alloc.art_end:
        f.decide('room', f'its sprite 3:{num:#x} ends at {alloc.art:08X}, past the chips\' art')
    return s


def table_sprites(b4, b6, f, alloc):
    """the sprites of the kinds' tables its objects read (TABLES): each spawned kind's BN6's of the same bytes, else copied
    (list 3 alone yet), the row's byte in the table's copy set to BN6's number"""
    named = {v for w in f.walks.values() for v in w.lits.values() if v in TABLES}
    for v in sorted(named):
        t = TABLES[v]
        for k in t['kinds']:
            at = v + t['entry'] * k + t['sprite'][1]
            num = b4.u8(at)
            tw = X4.bn6_sprite_twin(b4, b6, t['list'], num)
            if tw is None and t['list'] == 3:
                s = list3_copy(b4, f, alloc, num)
                tw, how = (s['to_index'], 'copied') if s else (None, '')
            else:
                how = 'the same bytes'
            if tw is None:
                f.decide('sprite', f'{at:08X}: its table\'s kind {k} draws {t["list"]}:{num:#x}, which BN6 has none of and the generator copies none of')
                continue
            f.notes.append(f'its table {v:08X}\'s kind {k}: sprite {t["list"]}:{num:#x} in BN4, {tw:#x} in BN6 ({how})')
            if tw != num:
                f.bytes.append((at, num, tw, f'its table\'s kind {k}: list {t["list"]} {num:#x} in BN4, {tw:#x} in BN6 ({how})'))


def report_unit(f):
    L = [f'## {f.name} ({f.kind} {f.sub:#x}, BN6\'s {f.kind} routine {f.number:#x})', '']
    L.append('chips: ' + ', '.join(str(i) for i, _ in f.chips))
    if getattr(f, 'blocks', None):
        L.append(f'blocks: ' + ', '.join(f'{lo:08X}-{hi:08X}' for lo, hi, _, _ in f.blocks) + f' ({sum(hi - lo for lo, hi, _, _ in f.blocks)} bytes)')
        L.append(f'walked: {len(f.walks)} routines; types moved: ' + ', '.join(f'{k}:{t:#x} as {a:#x}' for k, a, t in f.types))
        L += ['', 'calls:'] + [f'- {t:08X} -> {tw if isinstance(tw, str) else f"{tw:08X}" if tw else None} ({how}, {cls}{", audited" if aud else ""}; {len(sites)} sites)'
                               for t, tw, how, cls, sites, aud in f.pairs]
        L += ['', 'words:'] + [f'- {a:08X} -> {b:08X} ({why})' for a, b, why in f.words]
        L += ['', 'numbers:'] + [f'- {lit:08X}: {was:08X} -> {now:08X} ({why})' for lit, was, now, why in f.numbers]
        L += ['', 'immediates:'] + [f'- {i["at"]:08X} {i["form"]} {i["was"]:#x} -> {i["now"]:#x}: {i["why"]}' for i in f.imms]
    L += ['', 'notes:'] + [f'- {x}' for x in f.notes]
    L += ['', f'decisions open: {len(f.decisions)}'] + [f'- [{k}] {d}' for k, d in f.decisions]
    return '\n'.join(L) + '\n'


HEADER = '''/* xchips_bn4.c: BN4's chips' code as units for BN6's battle engine, written by tools/xchiprecipe_bn4.py from the two
 * ROMs (python3 tools/xchiprecipe_bn4.py; --check says whether this is what it writes). Do not edit: change the generator
 * (or tools/bn4_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/multirom/bn4-chips.md):
 * addresses in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA), counts and the CRC32s of the ranges the install copies from
 * the player's ROM; nothing of either game's. The units are the generator's, whole (no decision open), in install order;
 * src/emu/xchips_code.c installs them, and the registry's XREG_MOVED rows of BN4's (src/core/xchips_registry_bn4.c) name
 * the chips that run them. */
#include "xchips_bn4.h"

#include "bn4.h"
#include "bn6.h"
#include "rom.h"
'''
KIND_C = {'support': 'XCHIP_SUPPORT', 'misc': 'XCHIP_MISC'}


def place(at, base, name):
    return f'{name} + 0x{at - base:X}'


def c_unit(f):
    n = re.sub(r'[^a-z0-9]', '', f.name.lower())
    L = [f'/* ---- {f.name}: BN4\'s {f.kind} {f.sub:#x} ({f.entry | 1:08X}) as BN6\'s {f.number:#x}; its chips '
         f'{", ".join(f"{i} ({b})" for i, b in f.rows)} ---- */']
    L += [f'/* {x} */' for x in f.notes]
    L += ['', f'static const XvBlock {n}_blocks[] = {{']
    L += [f'\t{{ 0x{lo:08X}, 0x{hi:08X}, {place(at, X4C_CODE, "XC4_CODE")}, 0x{crc:08X}u }},' for lo, hi, at, crc in f.blocks]
    L += ['};', f'static const XvData {n}_data[] = {{']
    L += [f'\t{{ 0x{lo:08X}, 0x{hi:08X}, XV_{k} }},' for lo, hi, k in f.data]
    L += ['};', f'static const XvPair {n}_calls[] = {{']
    L += [X4.pair_text(t, tw, how, cls, audited) for t, tw, how, cls, sites, audited in f.pairs]
    L += ['};', f'static const XvPair {n}_words[] = {{']
    L += [f'\t{{ 0x{a:08X}, 0x{b:08X} }},   /* {why} */' for a, b, why in f.words]
    L += ['};', f'static const XvImm {n}_imms[] = {{']
    L += [f'\t{{ 0x{i["at"]:08X}, {i["form"]}, {i["rd"]}, {i["rn"]}, 0x{i["was"]:X}, 0x{i["now"]:X} }},   /* {i["why"]} */' for i in f.imms]
    L.append('};')
    if f.numbers:
        L.append(f'static const XvNumber {n}_numbers[] = {{')
        L += [f'\t{{ 0x{lit:08X}, 0x{was:08X}, 0x{now:08X} }},   /* {why} */' for lit, was, now, why in f.numbers]
        L.append('};')
    L.append(f'static const XvType {n}_types[] = {{')
    L += [f'\t{{ {k}, 0x{a:02X}, XV_TYPE_MOVED, 0x{t:02X} }},   /* BN4\'s {X4.KIND_NAMES[k]}-kind {t:#x}, moved, the dispatch\'s {a:#x} */'
          for k, a, t in f.types]
    L.append('};')
    if f.attack_sprites:
        L.append(f'static const XvSprite {n}_sprites[] = {{')
        L += [f'\t{{ {x["list"]}, 0x{x["index"]:02X}, {x["to_list"]}, 0x{x["to_index"]:02X}, {place(x["at"], X4C_ART, "XC4_ART")}, 0x{x["room"]:X} }},'
              f'   /* BN4\'s {x["list"]}:{x["index"]:#x} copied{" (LZ77 in BN4)" if x["lz"] else ""} */' for x in f.attack_sprites]
        L.append('};')
    if f.effects:
        L.append(f'static const XvId {n}_effects[] = {{ ' + ', '.join(f'{{ 0x{a:02X}, 0x{b:02X} }}' for a, b in f.effects) + ' };')
    if f.bytes:
        L.append(f'static const XvByte {n}_bytes[] = {{')
        L += [f'\t{{ 0x{at:08X}, 0x{was:02X}, 0x{now:02X} }},   /* {why} */' for at, was, now, why in f.bytes]
        L.append('};')
    if f.queue_calls:
        L.append(f'static const XvSoundCall {n}_sound_calls[] = {{')
        L += [f'\t{{ 0x{x["at"]:08X}, 0x{x["slot"]:X} }},   /* {x["why"]} */' for x in f.queue_calls]
        L.append('};')
    if f.sounds:
        L.append(f'static const XcSong {n}_songs[] = {{ ' + ', '.join(f'{{ 0x{v:X}, 0x{slot:02X} }}' for v, slot in f.sounds) + ' };')
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    fields = [f'.{a} = {n}_{a}, .n{a} = {count(a)}' for a, have in (('blocks', True), ('data', True), ('calls', True), ('words', True),
                                                                   ('imms', True), ('numbers', f.numbers), ('types', True),
                                                                   ('sound_calls', f.queue_calls), ('sprites', f.attack_sprites),
                                                                   ('effects', f.effects), ('bytes', f.bytes)) if have]
    L += ['', f'#define {n.upper()}_CODE {{ .name = "{f.name} (BN4\'s)", .game = XROM_BN4_RED_SUN_US, \\',
          f'\t.home = XC4_SHIMS_HOME({place(f.home, X4C_HOMES, "XC4_HOMES")}), \\']
    L += [f'\t{x}, \\' for x in fields[:-1]] + [f'\t{fields[-1]} }}']
    songs = f'{n}_songs, {count("songs")}' if f.sounds else 'NULL, 0'
    return '\n'.join(L), (f'\t{{ {KIND_C[f.kind]}, 0x{f.sub:02X}, 0x{f.number:02X}, 0x{f.entry | 1:08X}, {place(f.home, X4C_HOMES, "XC4_HOMES")}, '
                           f'{n.upper()}_CODE, {songs}, 0 }},')


def c_file(units):
    parts, rows = [HEADER], []
    for f in units:
        t, row = c_unit(f)
        parts.append(t)
        rows.append(row)
    parts.append('const XChipUnit xchips_bn4[] = {\n' + '\n'.join(rows) + '\n};\n'
                 'const int xchips_bn4_count = (int)(sizeof xchips_bn4 / sizeof *xchips_bn4);\n')
    return '\n'.join(parts)


def numbers_clear(units):
    """no other recipe names a dispatch number the chips' moved types take (an XvType row of its kind)"""
    for n in sorted(os.listdir(EMU)):
        if not n.endswith('.c') or n == os.path.basename(OUT_C):
            continue
        with open(os.path.join(EMU, n), encoding='utf-8') as fh:
            text = fh.read()
        for f in units:
            for k, a, t in f.types:
                if re.search(rf'\{{ {k}, 0x{a:02X}, ', text, re.I):
                    sys.exit(f'xchiprecipe_bn4: {n} names the dispatch\'s {k}:{a:#x}, which {f.name}\'s moved type takes: another number')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--check', action='store_true')
    ap.add_argument('--report', action='store_true')
    ap.add_argument('--unit', nargs='*', default=[])
    a = ap.parse_args()
    b4, b6 = X4.find_roms()
    pairs = X4.Pairs4(b4, b6)
    C.install(pairs)
    install(pairs)
    alloc = Alloc()
    more = tuple((k, int(s, 0), n) for k, s, n in (u.split(':') for u in a.unit))
    units = [derive(b4, b6, pairs, k, s, n, alloc, slot) for slot, (k, s, n) in enumerate(UNITS + CANDIDATES + more)]
    os.makedirs(WORK, exist_ok=True)
    with open(os.path.join(WORK, 'bn4_report.md'), 'w', encoding='utf-8') as fh:
        fh.write('# BN4\'s chips\' units (tools/xchiprecipe_bn4.py)\n\n' + '\n'.join(report_unit(f) for f in units))
    for f in units:
        print(f'{f.name}: {len(f.decisions)} decisions open')
    whole = units[:len(UNITS)]
    if any(f.decisions for f in whole):
        sys.exit('xchiprecipe_bn4: a unit of UNITS has decisions open (.build/xchiprecipe/bn4_report.md): nothing written')
    if len(whole) > X4C_UNITS:
        sys.exit('xchiprecipe_bn4: more units than homes')
    numbers_clear(whole)
    text = c_file(whole)
    if a.check:
        with open(OUT_C, encoding='utf-8') as fh:
            if fh.read() != text:
                sys.exit('xchiprecipe_bn4: src/emu/xchips_bn4.c is not what the generator writes')
        print('xchiprecipe_bn4: src/emu/xchips_bn4.c is what the generator writes')
    elif not a.report:
        with open(OUT_C, 'w', encoding='utf-8') as fh:
            fh.write(text)
        print(f'xchiprecipe_bn4: {len(whole)} units written')


if __name__ == '__main__':
    main()
