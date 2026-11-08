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
from struct import error as struct_error

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_twins  # noqa: E402
import xrecipe_bn4_copy  # noqa: E402
import xrecipe_thumb as T  # noqa: E402
from xrecipe_family import BN6, GENERIC6  # noqa: E402
from xrecipe_pairs import Pairs  # noqa: E402
from xrecipe import song_bytes, song_sig, sprite  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_C = os.path.join(ROOT, 'src', 'emu', 'xvirus_bn4.c')
WORK = os.path.join(ROOT, '.build', 'xrecipe')
MATCH = os.path.join(ROOT, '.build', 'symbols', 'match')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
SHA1 = {'bn6': '89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6', 'bn4': 'a97e96a7da03abd70f7953328e511b9fa29179f1'}

# BN4's families, in install order: (its first-kind type, the project's name, its first enemy id); each its six
# versions' ids from there (BN4_ENEMY_STATS' records name the type, checked)
FAMILIES = ((0x13, 'Billy', 13), (0x17, 'Spikey', 25), (0x16, 'Shrimpy', 31), (0x25, 'Walla', 103), (0x2E, 'Elmperor', 109),
            (0x20, 'CirKill', 91), (0x02, 'Mettaur', 1))
# the families moved for a Navi's summon alone, which BN6 has by name (the owner's call of 8 November 2026: Roll's summon
# brings BN4's own Mettaur; docs/multirom/bn4.md, Only what is new visits): never a net's kin nor a visitor (XVirusFamily.summoned)
SUMMONED = {'Mettaur'}

# ---- BN4's tables (src/emu/bn4.h; docs/rom/bn4.md) ----
BN4 = dict(t1=0x08003A24, nt1=95, t3=0x08003C40, nt3=165, stats=0x08011888, stat_size=12, names=0x0874A780,
           prestep=0x080137BC, runner=0x08013C48, present=0x08012D62, dying=0x0801052C, after=0x08010736,
           sprite_load=0x0800287C, shadow=0x08002E96, no_shadow=0x08002FE2, palette=0x08002DEC,
           byname=0x080B6568, sprites=0x0802793C, songs=0x08155FF8, effect=0x080E18EE, sound=0x08000560,
           effects=0x080E1698, neffects=111,
           create_coll=0x08012496, setup_coll=0x080129AC, set_flag=0x08012F82)
# the calls BN6's virus init (0x08016F56) makes its own of: an init's that are none of these, or of INIT_ENGINE's, are its own
INIT_STD = {BN4['sprite_load'], BN4['shadow'], BN4['no_shadow'], BN4['palette'], BN4['create_coll'], BN4['setup_coll'],
            0x08002E10, 0x0800283C, 0x0800ACA6, 0x08002FAE, 0x080033D4}
# its spawners (bn4.h BN4_SPAWN_*): the dispatch's (XV_SPAWN_*) in their place, each a kind's
SPAWNERS4 = {0x080032B8: (1, 'XV_SPAWN_1'), 0x080032D4: (1, 'XV_SPAWN_1_LAST'), 0x08003360: (1, 'XV_SPAWN_1_LAST'),
             0x080032F0: (3, 'XV_SPAWN_3'), 0x0800330C: (3, 'XV_SPAWN_3_LAST'), 0x08003328: (3, 'XV_SPAWN_3_FIRST'),
             0x08003344: (4, 'XV_SPAWN_4')}
NULLSUB6 = 0x0810A953   # BN6_FAMILY_NULLSUB
BN6_PRESENT = 0x08016E64   # BN6_VIRUS_PRESENT, BN4's after-state's twin but for its pause test
BX_R0, STATE_STEP6 = 0x08148864, 0x0810A349   # BN6_BX_R0 (its routine's address), BN6_VIRUS_STATE_STEP (Thumb)
# The collision data's offsets BN4's code may name, as BN6's (bn4.h BN4_COLL_*; docs/rom/bn4.md, The battle's objects)
COLL4TO6 = {0x01: 0x01, 0x03: 0x04, 0x06: 0x08, 0x07: 0x09, 0x08: 0x0A, 0x09: 0x0B, 0x0A: 0x0C, 0x10: 0x1C, 0x18: 0x24,
            0x1C: 0x28, 0x28: 0x80, 0x54: 0x70, 0x64: 0x3C, 0x68: 0x40}
ELEMENT4, ELEMENT6 = 0x17, 0x0E   # an object's element (BN4_T1_ELEMENT), BN6's (BN6_T1_ELEMENT)
T3_PALETTE4, T3_PALETTE6 = 0x0E, 0x0F   # a third-kind object's palette (BN4_T3_PALETTE), BN6's place for it (BN6_T3_PALETTE)
KIND_NAMES = {1: 'first', 3: 'third', 4: 'fourth'}   # (a moved type's kind, as the C file's comments name it)

# The numbers BN4's families take, past BN5's (tools/xrecipe.py's 25 in slots 0-24, their code to XV_CODE + 0x11000 and
# their art to XV_ART + 0x54000): slot 32 on (AI 64, ids 0x2C0), their code and art apart from BN5's, one family's after
# another's; list 1's numbers from 0xC0 (past those BN5's families and the moved objects take), list 4's from 0xC0 (past
# BN5's families' 0x5C on and the Navis' 0xA0-0xB5, below the chips' from 0xFF down); the dispatch's numbers of the
# types they move from 0xE0 of a kind's 256 (past every BN5 type's, 204 a kind at most: BN5's own numbers in the tables)
XV_AT = 0x08800000 + 0x330000
XV_HOMES, XV_CODE, XV_ART = XV_AT + 0x18000, XV_AT + 0x38000, XV_AT + 0x50000
XV_HOME_SIZE, XV_FIRST_ID, XV_FIRST_AI = 0x800, 0x200, 32
SLOT0, CODE0, ART0, LIST1_0, LIST4_0, TYPE_0 = 32, XV_CODE + 0x14000, XV_ART + 0x60000, 0xC0, 0xC0, 0xE0
# the families past the first six (Mettaur on: ROOM2_FROM), as the first six fill their rooms: their code from BN5's
# families' code's end (to XV_CODE + 0x11000, bn5_ranges_clear checks) to BN4's first room (CODE0), their art from BN5's
# families' art's end (XV_ART + 0x54000) to the sounds' room (SONG_ART0)
ROOM2_FROM, CODE2, ART2 = 6, XV_CODE + 0x11000, XV_ART + 0x54000
CODE_STEP, ROOM_BATTLE, ROOM_ATTACK = 0x10, 0x1000, 0x800   # (its blocks packed: BN4's code has 0x4000 bytes to XV_ART)
CODE_END, ART_END = XV_ART, 0x08800000 + 0x400000 - 256 * 0x44   # (xvirus_recipe.h: XV_ART, XV_ARMS)
SONGS6 = 0x08159F48   # BN6's song table (RomLayout.song_table)
MPLAYS4, MPLAYS6 = 0x08155E78, 0x08159DC8   # the music players' tables (BN4_MPLAYS, BN6_MPLAYS): 12 bytes a player, +8 its tracks
# BN4's sounds BN6 has no song of (xrecipe.song_sig), each copied into one of BN6's empty song slots, one slot a sound
# whichever family plays it (src/audio/xsong.c's XSOUNDS, the same numbers): the per-run themes' top (0x34-0x3F, the themes
# below, 12 copied today; the sounds' own 0x40-0x62 are BN5's, its chips' and Darkloids' to come in 0x41-0x4B), by install
# order from 0x3F down; each played on the player BN4's entry names (BN6's 32 players by BN4's numbers, BN4_MPLAYS); a
# sound not here is a decision
SOUND_SLOTS = {0xBC: 0x3F, 0xC4: 0x3E, 0x9F: 0x3D, 0xC9: 0x3C, 0xCA: 0x3B, 0x70: 0x3A, 0x9A: 0x39, 0xA6: 0x38, 0xB7: 0x37,
               0x9D: 0x36, 0xFC: 0x35, 0xF9: 0x34}
# A summoned family's sounds BN6 has no song of, in its summoner's per-battle pool instead (the BN4 Navis' pool,
# src/emu/xnavi_bn4_sounds.h; the owner's call 8 keeps 0x41-0x4B BN5's): sound -> (its pool slot, its summoner), the slot
# the summoner's next past its own (tools/xnavirecipe_bn4_sounds.py pool_of checks it), the song listed with the summoner's
# songs and pointed in her battles alone, which are the only ones the family is in (SUMMONED)
SUMMONER_POOL = {0x8B: (0xE0, 'Roll')}
# their copies' rooms: below their families' sprites (ART0), above BN5's families' art (to XV_ART + 0x54000 today,
# bn5_ranges_clear checks), as the families' sprites leave no room past theirs (ART_END); each its bytes rounded up to
# SONG_STEP (song_bytes, an upper bound)
SONG_ART0, SONG_ART_END, SONG_STEP = XV_ART + 0x5A000, ART0, 0x40
# BN4's effects BN6 has no effect of the same look of, at ids of their own and their sprites at numbers of their own: the
# families' ranges' top (0x90-0x9F of the effects, list 5's 0x98-0x9F), below the Navis' (0xA0 on), where BN5's families
# take theirs from the bottom (tools/xrecipe.py: effects from 0x6C, list 5 from 0x80; bn5_ranges_clear checks); a sprite of
# a list BN6 has not grown for them (3) listed in list 4, as BN5's are (tools/xrecipe.py, BN6_EFFECT_LIST)
EFFECT0, EFFECT_END, LIST5_0, LIST5_END, ROOM_EFFECT, EFFECT_LIST = 0x90, 0xA0, 0x98, 0xA0, 0x100, 4
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
        self.copies = {}   # BN6's spawner of a type copied for BN4's families (bn4_twins.COPIED6): the copy (xrecipe_bn4_copy.Copy)

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
        self.effects, self.copies = [], set()
        self.hooks = [('BN6', NULLSUB6)] * 8
        self.flags, self.spawn_calls, self.entries = 0, {}, []
        self.aim, self.prestep, self.outs, self.bytes = {}, None, {}, []
        self.sides = []   # (its calls to BN4's direction rule, each through a shim: side_sites)
        self.queue_calls = []   # (its calls to BN4's appear queue, each through a shim that gives r0 the queue: queued_sites)
        self.after = None

    def decide(self, kind, detail):  # noqa: D401
        self.decisions.append((kind, detail))


class Alloc:
    """the families' numbers and room, one family's after another's in install order"""

    def __init__(self):
        self.code, self.art, self.list1, self.list4, self.types = CODE0, ART0, LIST1_0, LIST4_0, {1: TYPE_0, 3: TYPE_0, 4: TYPE_0}
        # (the room's ends and its name in a decision: the families'; tools/xnavirecipe_bn4.py gives the Navis' their own)
        self.code_end, self.art_end, self.room = CODE_END, ART_END, 'the families'
        # (BN4's effects moved, by its id: BN6's; their sprites copied, by BN4's list and number: the copy, its family's name)
        self.effect, self.list5, self.effects, self.effect_sprites = EFFECT0, LIST5_0, {}, {}
        self.effect_owner = {}   # (BN4's effect moved: the family that moved it and lists it)


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


def after_of(f, tw):
    """its after-state: its type routine's one call, after its state (init, update or dying), BN4's (BN4_VIRUS_PRESENT),
    whose BN6 twin skips it while the battle is paused (bn4_twins.COPIED): copied with it and run after its state 0 (its
    appear) and its hook 5 (its spawn done) while BN6's skips it for the pause alone (XVirusFamily.present)"""
    ts = sorted(tw.bls.items())
    if [t for _, t in ts] != [BN4['after']]:
        f.decide('after-state', f'its type routine calls {", ".join(f"{t:08X}" for _, t in ts)}, not BN4\'s after-state alone')
        return
    f.after = BN4['after']
    f.roots.append(f.after)
    f.notes.append(f'its after-state: BN4\'s {f.after:08X}, copied, run after its state 0 (its appear) and its hook 5 while BN6\'s battle is paused '
                   f'(BN6\'s twin {BN6_PRESENT:08X} skips it there)')


def update_of(f, uw):
    """its update past the pre-step, the state runner and the present (BN6's dispatcher runs each): hook 3, the tail
    entered (ENTRIES), where there is one; or its own work between the pre-step and the runner, its pre-step"""
    calls = sorted(uw.bls.items())
    ts = [t for _, t in calls]
    if ts[:3] != [BN4['prestep'], BN4['runner'], BN4['present']] and f.update in bn4_twins.PRESTEP:
        prestep_of(f, uw, calls)
        return
    if [t for _, t in calls[:3]] != [BN4['prestep'], BN4['runner'], BN4['present']]:
        f.decide('pre-step', f'its update calls {", ".join(f"{t:08X}" for _, t in calls)}, not BN4\'s pre-step, state runner and present first')
        return
    at = calls[2][0] + 4
    if not T.is_return(uw.insns[at]):
        enter(f, 0, at, 'its update')


def prestep_of(f, uw, calls):
    """its update's own work between BN4's pre-step and its state runner, run as its pre-step: BN6's dispatcher jumps there
    from its NameID cases (after its own pre-step, before the state: where BN4 runs it), its state table's load made BN6's
    state step's address and its call to the runner a call to a bx r0 of BN6's: on to BN6's state step (bn4_twins.PRESTEP)"""
    ts = [t for _, t in calls]
    run = [s for s, t in calls if t == BN4['runner']]
    lit = T.back_value(uw, run[0], 0)[1] if run else []
    if (ts[0] != BN4['prestep'] or ts[-2:] != [BN4['runner'], BN4['present']] or ts.count(BN4['runner']) != 1
            or len(lit) != 1 or lit[0][1] != 'ldr_lit'):
        f.decide('pre-step', f'its update calls {", ".join(f"{t:08X}" for t in ts)}, not BN4\'s pre-step, its own work, the runner and '
                             f'the present')
        return
    f.prestep = calls[0][0] + 4
    f.roots.append(f.prestep)
    f.aim[BN4['runner']] = (BX_R0, 'its pre-step\'s way on: a bx r0 of BN6\'s, r0 BN6\'s state step')
    f.numbers.append((uw.insns[lit[0][0]].lit, f.st, STATE_STEP6, 'its pre-step\'s way on: BN6\'s state step, not its state table'))
    f.notes.append(f'its pre-step: its update from {f.prestep:08X}, {bn4_twins.PRESTEP[f.update]}')


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
            ents = table_at(b4, v, reg[0], reg[1])
            if reg[2] is None or not any(own_region(f, x & ~1) is None for x in ents[:3]):
                ents = [x for x in ents if reg[0] <= (x & ~1) < reg[1]]
            if ents:
                # (a moved object's jump table whole: an entry out of its regions a routine of BN4's whose twin the words name)
                f.tables[v] = ents
                todo += [x for x in ents if (x & ~1) not in f.walks and own_region(f, x & ~1)]
                for x in ents:
                    if own_region(f, x & ~1) is None:
                        f.outs.setdefault(x, []).append(v)
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
                    why = f'BN4\'s panel type {x.f[1]:#x}'
                    if x.f[1] not in bn4_twins.PANEL4TO6:
                        f.decide('panel', f'{c:08X}: its panel type {x.f[1]:#x} compared, which bn4_twins.PANEL4TO6 gives no BN6 meaning')
                        to = 0xFF
                    elif bn4_twins.PANEL4TO6[x.f[1]] is None:
                        # (a type BN6 has none of, decided: compared with 0xFF, no type of BN6's, the case never holding there
                        # as in BN6's own routine)
                        to, why = 0xFF, why + ', none in BN6: compared with no type of BN6\'s'
                    else:
                        to = bn4_twins.PANEL4TO6[x.f[1]]
                        why += f', BN6\'s {to:#x}'
                    f.imms.append(dict(at=c, form='THUMB_CMP', rd=x.f[0], rn=0, was=x.f[1], now=to, why=why))
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


def palette_imms(f, a, i, objs):
    """a moved third-kind type's palette (BN4's +0xE, its sprite's +4 by 0x08002DEC, BN6's 0x08002D80): BN6's +0xE is the
    element (BN4's +0x17, field_imms), so in a moved third-kind type's code its own +0xE and that of the object it spawned
    are BN6's +0xF, which BN6's engine reads only on its actors (FireMan's 0x09: its init 0x080CE358, its spawner's store
    0x080CE4BC and its chain's load 0x080CE486)"""
    reg = own_region(f, a)
    if i.form not in FORMS or not reg or not isinstance(reg[2], int) or i.f[2] != T3_PALETTE4:
        return
    if i.f[1] == 5 or i.f[1] in objs:
        who = 'its' if i.f[1] == 5 else 'the object it spawned:'
        f.imms.append(dict(at=a, form=FORMS[i.form], rd=i.f[0], rn=i.f[1], was=T3_PALETTE4, now=T3_PALETTE6,
                           why=f'{who} palette: BN4 +0x0E, BN6 +0x0F (its +0x0E the element)'))


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
            palette_imms(f, a, i, objs)
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
            # (a fourth-kind type a Navi's recipe moves: f.moved4, tools/xnavirecipe_bn4_summon.py's)
            moved = f.moved3 if kind == 3 else getattr(f, 'moved4', {}) if kind == 4 else {}
            if ty not in moved or len(chain) != 1 or chain[0][1] != 'movs':
                f.decide('type', f'{site:08X}: it spawns a {kind} type {ty if ty is None else hex(ty)} which it does not move')
                continue
            m = moved[ty]
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


def side_sites(f):
    """its calls (its own code's or a copied routine's) to a rule of BN4's whose answer BN6 numbers by the object's side
    (bn4_twins.SIDED): each through a shim that numbers it so (src/emu/xvirus_shim.h)"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t in bn4_twins.SIDED:
                f.sides.append((site, f'its call to {t:08X} (bn4_twins.SIDED): its left and right by the object\'s side, BN6\'s way'))


def queued_sites(f):
    """its calls to BN4's appear queue's routines (bn4_twins.QUEUED; one queue, BN6's by r0): each through a shim that makes r0
    the queue's number and goes on to BN6's twin (XvSoundCall, xvirus.c's r0 calls)"""
    f.queue_calls = [dict(at=s, slot=q, why=f'its call to {t:08X}: BN6\'s twin of queue {q} (bn4_twins.QUEUED)')
                     for t, q in sorted(bn4_twins.QUEUED.items()) for s in sorted(f.calls.get(t, []))]


def pair_calls(f, pairs):
    f.pairs = []
    for t, sites in sorted(f.calls.items()):
        if t in f.aim:
            f.pairs.append((t, f.aim[t][0], f.aim[t][1], 'AIMED', sites, True))
            continue
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
        if tw is not None and (tw & ~1) in pairs.copies:
            # (BN6's spawner of a type copied for BN4's families: the copy's spawner, its type run through the dispatch)
            c = pairs.copies[tw & ~1]
            to = xrecipe_bn4_copy.moved(c, tw & ~1)
            f.pairs.append((t, f'XV_CODE + 0x{to - XV_CODE:X}', f'BN6\'s {tw:08X} as copied for BN4\'s families: its third-kind {c.ty:#x} '
                            f'run as the dispatch\'s {c.number:#x} (bn4_twins.COPIED6; {how}, {cls}{", audited" if audit else ""})',
                            None, sites, bool(audit)))
            f.copies.add(c.ty)
            continue
        if tw is None:
            f.decide('call', f'{t:08X} ({len(sites)} BLs, at {", ".join(f"{s:08X}" for s in sites)}): no twin in BN6 ({how})')
        elif cls == 'DIFF' and not audit:
            f.decide('call', f'{t:08X} -> {tw:08X} ({how}): their bodies differ ({diff}), and no audit says the pair serves (bn4_twins.AUDITS)')
        f.pairs.append((t, tw, how, cls, sites, bool(audit)))


def words_of(f, pairs):
    """its literal words that name RAM, I/O or ROM out of its blocks (a number read by hand, bn4_twins.NUMBERS, kept), and
    its moved objects' jump tables' entries out of them (a routine of BN4's: its twin)"""
    f.words = []
    for x, tabs in sorted(f.outs.items()):
        tw, how, cls, diff = pairs.twin(x & ~1)
        if tw is None or (cls == 'DIFF' and not pairs.audit(x & ~1, tw)):
            f.decide('table', f'{x:08X} (in the jump table at {", ".join(f"{t:08X}" for t in tabs)}): no twin that serves ({how}, {cls})')
            continue
        f.words.append((x, tw | 1, f'{how}, {cls}'))
    for e, w in f.walks.items():
        for lit, v in w.lits.items():
            if v is None or any(lo <= (v & ~1) < hi for lo, hi in f.spans) or not 0x02000000 <= v < 0x0E000000:
                continue
            if v in bn4_twins.NUMBERS:
                if lit not in [x[0] for x in f.numbers]:
                    f.numbers.append((lit, v, v, bn4_twins.NUMBERS[v]))
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
    if slot == ROOM2_FROM and alloc.room == 'the families':
        # (the families past the first six in their second room; tools/xnavirecipe_bn4.py's families_alloc derives them too)
        alloc.code, alloc.code_end, alloc.art, alloc.art_end = CODE2, CODE0, ART2, SONG_ART0
        alloc.room = 'the families\' second room'
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
    after_of(f, tw)
    if ents[2] & ~1 != BN4['dying']:
        f.decide('type', f'its dying entry is {ents[2]:08X}, not BN4\'s shared {BN4["dying"]:08X}')
    uw = T.walk(b4, f.update, region=(f.lo, f.hi))
    site = [s for s, t in uw.bls.items() if t == BN4['runner']]
    f.st = T.back_value(uw, site[0], 0)[0] if site else None
    states = table_at(b4, f.st, f.lo, f.hi, 24) if f.st else []
    f.states = [('BN6', GENERIC6[k] | 1) for k in range(8)]
    for k in range(6):
        if k < len(states) and states[k] & ~1 != bn4_twins.GENERIC4[k] and states[k] & ~1 in bn4_twins.OWN_STATES:
            # (a generic state of its own: a wrapper of BN4's, its call to the generic one paired with BN6's)
            f.states[k] = ('OWN', states[k] | 1)
            f.roots.append(states[k] & ~1)
        elif k >= len(states) or states[k] & ~1 != bn4_twins.GENERIC4[k]:
            f.decide('states', f'its state {k} is not BN4\'s generic one')
    if name in SUMMONED:
        # (its generic state 0 its own, BN4's copied with BN4's summoned appear: bn4_twins.SUMMON_APPEAR)
        f.states[0] = ('OWN', bn4_twins.GENERIC4[0] | 1)
        f.roots.append(bn4_twins.GENERIC4[0])
        f.regions += [(lo, hi, 'appear') for lo, (hi, _) in sorted(bn4_twins.SUMMON_APPEAR.items())]
    f.own = states[6:]
    f.states += [('OWN', x | 1) for x in f.own]
    f.s1 = struct1_of(b4, f, f.init)
    init_of(f)
    update_of(f, uw)
    del f.walks[f.init]
    f.dead = [(f.lo, tw), (f.init, T.walk(b4, f.init, region=(f.lo, f.hi))), (f.update, uw)]
    walk_code(b4, f, [x & ~1 for x in f.own] + f.roots, {f.type_table, f.st}, pairs)
    name_ranges(f)
    side_sites(f)
    scan(f)
    spawns_of(f, alloc)
    pair_calls(f, pairs)
    queued_sites(f)
    records_of(b4, f)
    blocks_of(b4, f, alloc)
    words_of(f, pairs)
    art_of(b4, b6, f, alloc)
    effects_of(b4, b6, f, alloc)
    sounds_of(b4, b6, f)
    f.interim += list(bn4_twins.INTERIMS.get(name, ()))
    if f.prestep and any(not f.lo <= s < f.hi for s in f.calls.get(BN4['runner'], [])):
        f.decide('pre-step', f'BN4\'s state runner called out of its update too: not to be aimed at a bx r0')
    f.imms.sort(key=lambda x: x['at'])
    return f


def effects_of(b4, b6, f, alloc):
    """the effects its code spawns (BN4_SPAWN_EFFECT, r4 the id, a movs): each BN6's of the same look (effect_twin), else
    moved (move_effect), the movs given BN6's id"""
    ids = {}
    for e, w in f.walks.items():
        for site, t in w.bls.items():
            if t == BN4['effect']:
                v, chain = T.back_value(w, site, 4)
                # (an id with a parameter in its high byte, lsls #8 then adds #id: Roll's flash's, the fourth kind's 0x17)
                if v is not None and len(chain) == 2 and chain[0][1] == 'adds8' and chain[1][1] == 'lsls' and chain[1][2] == 8:
                    ids.setdefault(chain[0][2], []).append((chain[0][0], 'THUMB_ADDS'))
                    continue
                if v is None or len(chain) != 1 or chain[0][1] != 'movs' or v >= BN4['neffects']:
                    f.decide('effect', f'an effect spawned at {site:08X} by an id made otherwise ({chain})')
                    continue
                ids.setdefault(v, []).append((chain[0][0], 'THUMB_MOVS'))
    for v, at in sorted(ids.items()):
        e6, how = effect_twin(b4, b6, v), 'the same look'
        if e6 is None:
            e6, how = move_effect(b4, f, alloc, v), 'moved'
        if e6 is None:
            continue
        if e6 != v:
            for a, form in sorted(set(at)):
                f.imms.append(dict(at=a, form=form, rd=4, rn=0, was=v, now=e6, why=f'its effect: {v:#x} in BN4, {e6:#x} in BN6 ({how})'))
        f.notes.append(f'its effect {v:#x}: BN6\'s {e6:#x}, ' + ('BN6\'s own of the same sprite, animation and palette' if how != 'moved' else
                       'moved (no effect of BN6\'s draws its sprite\'s bytes)'))


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


def move_effect(b4, f, alloc, e4):
    """BN4's effect e4 at an id of its own (one for every family that spawns it), its row BN4's with its sprite as BN6
    lists the copy: list 5's at a number of its own, list 4's and list 3's (BN6's list 3 not grown for them) in list 4,
    copied once (a family after the one that copied it finds it installed)"""
    row = b4.bytes(BN4['effects'] + 4 * e4, 4)
    lst, idx = row[0] // 4, row[1]
    if row[0] & 3 or lst not in (3, 4, 5):
        f.decide('effect', f'its effect {e4:#x}\'s sprite {row[0]:#x}:{idx:#x}: the generator moves lists 3, 4 and 5\'s alone')
        return None
    e6 = alloc.effects.get(e4)
    if e6 is not None and alloc.effect_owner.get(e4) not in (None, f.name):
        # (moved by one installed before it, its id and row the same: listed by each that spawns it, so each recipe names
        # every effect it draws, test_core's numbers apart but for such a pair)
        f.notes.append(f'its effect {e4:#x}: {e6:#x}, as {alloc.effect_owner[e4]} moved it')
    if e6 is None:
        if alloc.effect >= EFFECT_END:
            f.decide('effect', f'its effect {e4:#x}: BN4\'s effect ids ({EFFECT0:#x}-{EFFECT_END - 1:#x}) all taken')
            return None
        e6 = alloc.effects[e4] = alloc.effect
        alloc.effect_owner[e4] = f.name
        alloc.effect += 1
    f.effects.append((e4, e6))
    have = alloc.effect_sprites.get((lst, idx))
    if have is not None:
        if have[1] != f.name:
            f.notes.append(f'its effect {e4:#x}\'s sprite {lst}:{idx:#x} is listed by {have[1]} ({have[0]["to_list"]}:{have[0]["to_index"]:#x}): '
                           f'installed with it')
        return e6
    p, raw = sprite(b4, BN4['sprites'], lst, idx)
    to_list = 5 if lst == 5 else EFFECT_LIST
    num = alloc.list5 if to_list == 5 else alloc.list4
    if raw is None or (to_list == 5 and num >= LIST5_END):
        f.decide('effect', f'its effect {e4:#x}\'s sprite {lst}:{idx:#x} could not be read or numbered')
        return e6
    s = dict(list=lst, index=idx, to_list=to_list, to_index=num, at=alloc.art, room=rounded(len(raw), ROOM_EFFECT), lz=bool(p & 0x80000000))
    f.attack_sprites.append(s)
    alloc.effect_sprites[(lst, idx)] = (s, f.name)
    if to_list == 5:
        alloc.list5 += 1
    else:
        alloc.list4 += 1
    alloc.art += s['room']
    if alloc.art > alloc.art_end:
        f.decide('room', f'its effect {e4:#x}\'s sprite ends at {alloc.art:08X}, past {alloc.room}\'s art ({alloc.art_end:08X})')
    return e6


def bn5_ranges_clear():
    """BN5's families' recipes take none of BN4's effect ids nor list 5 numbers (their ranges' top), nor its sounds' room"""
    with open(BN5_RECIPES, encoding='utf-8') as fh:
        text = fh.read()
    effects = {int(x, 16) for m in re.finditer(r'_effects\[\] = \{([^;]*)\};', text) for x in re.findall(r'0x([0-9A-F]+) \}', m.group(1))}
    list5 = {int(x, 16) for x in re.findall(r'\{ \d, 0x[0-9A-F]+, 5, 0x([0-9A-F]+),', text)}
    clash = sorted(e for e in effects if EFFECT0 <= e < EFFECT_END) + sorted(n for n in list5 if LIST5_0 <= n < LIST5_END)
    if clash:
        sys.exit(f'xrecipe_bn4: BN5\'s families take BN4\'s numbers ({", ".join(f"{x:#x}" for x in clash)}): move EFFECT0 or LIST5_0')
    # (nor BN4's sounds' room, below BN4's sprites: BN5's art spans, a sprite's or a song's, ending at or before it)
    art_end = max((int(a, 16) + int(r, 16) for a, r in re.findall(r'XV_ART \+ 0x([0-9A-F]+), 0x([0-9A-F]+)', text)), default=0)
    if XV_ART + art_end > ART2:
        sys.exit(f'xrecipe_bn4: BN5\'s families\' art ends at {XV_ART + art_end:08X}, past BN4\'s second room ({ART2:08X}): move ART2')
    code = [(int(lo, 16), int(hi, 16), int(at, 16)) for lo, hi, at in
            re.findall(r'\{ 0x([0-9A-F]+), 0x([0-9A-F]+), XV_CODE \+ 0x([0-9A-F]+)', text)]
    code_end = max((at + hi - lo for lo, hi, at in code), default=0)
    if XV_CODE + code_end > CODE2:
        sys.exit(f'xrecipe_bn4: BN5\'s families\' code ends at {XV_CODE + code_end:08X}, past BN4\'s second room ({CODE2:08X}): move CODE2')


def sounds_of(b4, b6, f):
    """the sounds its code plays (BN4_PLAY_SOUND, r0 the sound, a movs): each BN6's of the same number where it is the same
    song (how it sounds, xrecipe.song_sig); else BN4's copied into its slot of BN6's empty songs (SOUND_SLOTS, xsong.c's
    XSOUNDS) and the movs given the slot, the copy's room given once every family is derived (songs_room)"""
    ids = {}
    for e, w in f.walks.items():
        for site, t in w.bls.items():
            if t == BN4['sound']:
                v, chain = T.back_value(w, site, 0)
                ids.setdefault(v, []).append((site, chain))
    if None in ids:
        f.decide('sound', f'a sound played at {", ".join(f"{s:08X}" for s, _ in ids[None])} made otherwise')
    odd = sorted(v for v in ids if v is not None and song_sig(b4, BN4['songs'], v) != song_sig(b6, SONGS6, v))
    same = sorted(v for v in ids if v is not None and v not in odd)
    if same:
        f.notes.append(f'its sounds {", ".join(f"{v:#x}" for v in same)}: BN6\'s of the same numbers, the same songs')
    f.sounds = []
    for v in odd:
        slot = SOUND_SLOTS.get(v)
        if slot is None and f.name in SUMMONED and v in SUMMONER_POOL:
            slot, who = SUMMONER_POOL[v]
            for site, chain in sorted(ids[v]):
                if len(chain) != 1 or chain[0][1] != 'movs':
                    f.decide('sound', f'its sound {v:#x} at {site:08X} named otherwise than by a movs ({chain})')
                    continue
                f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=v, now=slot,
                                   why=f'its sound {v:#x}: at {who}\'s pool slot {slot:#x}, her songs\' (her battles alone)'))
            f.notes.append(f'its sound {v:#x}: {who}\'s pool slot {slot:#x} (SUMMONER_POOL), her recipe\'s song')
            continue
        if slot is None:
            f.decide('sound', f'its sound {v:#x} is no song of BN6\'s and has no slot (SOUND_SLOTS): ask')
            continue
        hp = b4.u32(BN4['songs'] + 8 * v)
        player = b4.u16(BN4['songs'] + 8 * v + 4)
        tracks, room6 = b4.u8(hp), b6.u8(MPLAYS6 + 12 * player + 8)
        f.sounds.append((v, slot))
        for site, chain in sorted(ids[v]):
            if len(chain) != 1 or chain[0][1] != 'movs':
                f.decide('sound', f'its sound {v:#x} at {site:08X} named otherwise than by a movs ({chain})')
                continue
            f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=v, now=slot,
                               why=f'its sound {v:#x}: BN4\'s copied into BN6\'s empty slot {slot:#x}, on its player {player}'))
        f.notes.append(f'its sound {v:#x} ({", ".join(f"{s:08X}" for s, _ in sorted(ids[v]))}): BN4\'s song copied into BN6\'s empty slot '
                       f'{slot:#x}, on BN4\'s player {player} ({tracks} track{"s" if tracks != 1 else ""}; BN6\'s player {player} has {room6})')
        if tracks > room6:
            f.interim.append(f'its sound {v:#x} plays {room6} of its {tracks} tracks: BN6\'s player {player} has {room6} where BN4\'s has '
                             f'{b4.u8(MPLAYS4 + 12 * player + 8)} (BN6_MPLAYS against BN4_MPLAYS), the rest let go as BN6\'s MPlayStart '
                             f'starts no more tracks than its player has')


def songs_room(b4, whole):
    """each whole family's copied sounds a room in the families' art below their sprites (SONG_ART0 to ART0), one after
    another in install order, a sound once whichever family plays it (the next finds it installed: xvirus_songs.c xvirus_songs_set)"""
    done, at = set(), SONG_ART0
    for f in whole:
        f.songs = []
        for v, slot in getattr(f, 'sounds', []):
            if v in done:
                continue
            done.add(v)
            room = rounded(song_bytes(b4, v, BN4['songs']), SONG_STEP)
            f.songs.append(dict(song=v, slot=slot, at=at, room=room))
            at += room
            if at > SONG_ART_END:
                sys.exit(f'xrecipe_bn4: {f.name}\'s sound {v:#x} ends at {at:08X}, past the sounds\' room ({SONG_ART_END:08X})')


def blocks_of(b4, f, alloc):
    """its region, each type it moves and each copied routine, at its code's place (each the distance from a word its own);
    each block's bytes classed: the code its states reach, their pools and jump tables, the dead (its type's routine, jump
    table, init and update but for a tail entered, its state table: BN6's virus code does their work; a copied routine's
    neighbours) zeroed, the rest raw, a word of it that points into a block a pool's"""
    spans = [(r[0], r[1]) for r in f.regions]
    named = sorted({v for w in f.walks.values() for v in w.lits.values() if v in bn4_twins.TABLES})
    spans += [(v, v + bn4_twins.TABLES[v]['size']) for v in named]
    copied = []
    for e in f.copied:
        w = f.walks[e]
        hi = max(max(a + i.size for a, i in w.insns.items()), max((l + 4 for l in w.lits), default=0))
        if e in bn4_twins.COPIED_DATA:
            hi = max(hi, bn4_twins.COPIED_DATA[e][1])
        # (a copied routine within another's span, its pool past it: one block, the two routines its code)
        if copied and e < copied[-1][1]:
            copied[-1] = (copied[-1][0], max(hi, copied[-1][1]))
        else:
            copied.append((e, hi))
    spans += copied
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
        rest = 'RAW' if k < len(f.regions) + len(named) else 'ZERO'
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
    if at > alloc.code_end:
        f.decide('room', f'its code ends at {at:08X}, past {alloc.room}\'s code ({alloc.code_end:08X})')


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
    # (a Navi's battle sprite where the Navis' generator places it, a room the battles share: tools/xnavirecipe_bn4.py
    # battle_sprite_place; a family's resident in its art)
    place = getattr(alloc, 'battle_place', None)
    if place:
        f.sprite.update(place(f, raw))
    else:
        alloc.art += room
    if alloc.art > alloc.art_end:
        f.decide('room', f'its sprite ends at {alloc.art:08X}, past {alloc.room}\'s art ({alloc.art_end:08X})')
    for e, w in sorted(f.walks.items()):
        reg = own_region(f, e)
        if not reg or reg[2] is None:
            continue
        for site, t in sorted(w.bls.items()):
            tab = next((v for v, x in bn4_twins.TABLES.items() if site in x['loads']), None)
            if t == BN4['sprite_load'] and tab is not None:
                table_art(b4, f, alloc, tab)
            elif t == BN4['sprite_load']:
                attack_sprite(b4, f, alloc, w, site)


def list4_sprite(b4, f, alloc, num):
    """list 4's sprite num of BN4's, copied once a family at a list 4 number of its own"""
    p, raw = sprite(b4, BN4['sprites'], 4, num)
    if raw is None:
        return None
    s = next((x for x in f.attack_sprites if x['index'] == num), None)
    if s is None:
        room = rounded(len(raw), ROOM_ATTACK)
        s = dict(list=4, index=num, to_list=4, to_index=alloc.list4, at=alloc.art, room=room, lz=bool(p & 0x80000000))
        f.attack_sprites.append(s)
        alloc.list4 += 1
        alloc.art += room
    return s


def table_art(b4, f, alloc, v):
    """a table its moved object reads its sprite from (bn4_twins.TABLES): each kind it spawns, its sprite copied and its
    number's byte in the table's copy given the copy's"""
    t = bn4_twins.TABLES[v]
    for k in t['kinds']:
        at = v + t['entry'] * k
        # (a table that holds no list: its object's sprite list 4 by a movs, its load's r1)
        lst = 0x10 if t['sprite'][0] is None else b4.u8(at + t['sprite'][0])
        num = b4.u8(at + t['sprite'][1])
        s = list4_sprite(b4, f, alloc, num) if lst == 0x10 else None
        if s is None:
            f.decide('sprite', f'{at:08X}: its table\'s kind {k} draws {lst // 4}:{num:#x}, which the generator moves none of')
            continue
        if all(b[0] != at + t['sprite'][1] for b in f.bytes):
            f.bytes.append((at + t['sprite'][1], num, s['to_index'], f'its table\'s kind {k}: list 4 {num:#x} in BN4, {s["to_index"]:#x} copied'))


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
 * (no decision open; its interims named over each), installed wherever BN4's ROM is read, as the nets' visitors
 * (docs/multirom/bn4.md); what stays open is in its report (.build/xrecipe/bn4_report.md). First, BN6's own types copied for
 * them with a number BN4's (tools/xrecipe_bn4_copy.py; xvirus_bn4_copy.h installs them before the families). */
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
    if f.sides:
        L.append(f'static const uint32_t {n}_sides[] = {{')
        L += [f'\t0x{at:08X},   /* {why} */' for at, why in f.sides]
        L.append('};')
    if getattr(f, "queue_calls", None):
        L.append(f'static const XvSoundCall {n}_sound_calls[] = {{')
        L += [f'\t{{ 0x{x["at"]:08X}, 0x{x["slot"]:X} }},   /* {x["why"]} */' for x in f.queue_calls]
        L.append('};')
    if f.numbers:
        L.append(f'static const XvNumber {n}_numbers[] = {{')
        L += [f'\t{{ 0x{lit:08X}, 0x{was:08X}, 0x{now:08X} }},   /* {why} */' for lit, was, now, why in f.numbers]
        L.append('};')
    if f.words:
        L.append(f'static const XvPair {n}_words[] = {{')
        L += [f'\t{{ 0x{a:08X}, 0x{b:08X} }},   /* {why} */' for a, b, why in f.words]
        L.append('};')
    if f.bytes:
        L.append(f'static const XvByte {n}_bytes[] = {{')
        L += [f'\t{{ 0x{at:08X}, 0x{was:02X}, 0x{now:02X} }},   /* {why} */' for at, was, now, why in f.bytes]
        L.append('};')
    if f.types:
        L.append(f'static const XvType {n}_types[] = {{')
        L += [f'\t{{ {k}, 0x{a:02X}, XV_TYPE_MOVED, 0x{t:02X} }},   /* BN4\'s {KIND_NAMES[k]}-kind {t:#x}, moved, the dispatch\'s {a:#x} */'
              for k, a, t in f.types]
        L.append('};')
    L += [f'static const XvRef {n}_states[] = {{', '\t' + ', '.join(ref_text(x) for x in f.states) + ',', '};']
    L += [f'static const XvSprite {n}_sprites[] = {{'] + [sprite_text(s) for s in [f.sprite] + f.attack_sprites] + ['};']
    if f.effects:
        L.append(f'static const XvId {n}_effects[] = {{ ' + ', '.join(f'{{ 0x{a:02X}, 0x{b:02X} }}' for a, b in f.effects) + ' };')
    if getattr(f, 'songs', None):
        L.append(f'static const XvSong {n}_songs[] = {{')
        L += [f'\t{{ 0x{x["song"]:X}, 0x{x["slot"]:02X}, XV_ART + 0x{x["at"] - XV_ART:X}, 0x{x["room"]:X}'
              f'{f", {x["player"]}" if x.get("player") else ""} }},   /* its sound {x["song"]:#x}, BN4\'s song copied'
              f'{f", on BN6\'s player {x["player"]}" if x.get("player") else ""} */' for x in f.songs]
        L.append('};')
    L += c_family(f, n)
    return '\n'.join(L), f'&bn4_{n}'


def c_family(f, n):
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    L = ['', f'static const XVirusFamily bn4_{n} = {{', f'\t.name = "{f.name} (BN4\'s)",', '\t.game = XROM_BN4_RED_SUN_US,',
         f'\t.src_ai = 0x{f.ty:02X},   /* (its first-kind type: BN4 has no AI index) */', f'\t.ai = {f.ai},',
         f'\t.first_id = XV_FIRST_ID + 0x{f.first_id - XV_FIRST_ID:X},', f'\t.home = XV_HOMES + 0x{f.home - XV_HOMES:X},']
    for field, there in (('blocks', True), ('data', True), ('calls', True), ('words', f.words), ('imms', True), ('ranges', f.ranges),
                         ('sides', f.sides), ('sound_calls', getattr(f, 'queue_calls', None)), ('numbers', f.numbers), ('bytes', f.bytes),
                         ('types', f.types), ('states', True)):
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
    if f.effects:
        L.append(f'\t.effects = {n}_effects, .neffects = {count("effects")},')
    if getattr(f, 'songs', None):
        L.append(f'\t.songs = {n}_songs, .nsongs = {count("songs")},')
    if f.prestep:
        L.append(f'\t.prestep = 0x{f.prestep:08X},   /* its update\'s own work on, run as its pre-step (bn4_twins.PRESTEP) */')
    if f.name in SUMMONED:
        L.append('\t.summoned = true,   /* (moved for Roll\'s summon alone, BN6\'s by name: no kin of its net\'s, no visitor) */')
    if f.after:
        L.append(f'\t.present = 0x{f.after:08X},   /* its after-state, copied: run after its state 0 and its hook 5 while BN6\'s battle is paused */')
    L.append('};')
    return L


def c_file(whole, copies):
    parts, refs = [HEADER], []
    # (BN6's types copied for them, installed before them: xvirus_bn6copy.c; one, the thunder, while COPIED6 holds one)
    parts += [xrecipe_bn4_copy.c_text(c, XV_CODE) for c in copies]
    if len(copies) != 1:
        sys.exit('xrecipe_bn4: the C file names one copy of BN6\'s (xvirus_bn4_bn6copy): more want a list')
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
         f'{len(f.ranges)} NameID ranges, {len(f.sides)} directions by the side', '', '### Calls out', '']
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
    bn5_ranges_clear()
    alloc = Alloc()
    copies = []
    for ty in sorted(bn4_twins.COPIED6):
        # (at the dispatch's number of its own type, which BN4's battle runs it as too: no BN5 track names it, checked)
        xrecipe_bn4_copy.number_clear(ROOT, 3, ty)
        c = xrecipe_bn4_copy.derive(b6, ty, alloc.code, ty)
        alloc.code = rounded(c.room_end, CODE_STEP)
        copies.append(c)
        if not c.decisions:
            pairs.copies[c.spawner] = c
    fams = [derive(b4, b6, pairs, ty, name, first, k, alloc) for k, (ty, name, first) in enumerate(FAMILIES)]
    whole = [f for f in fams if not f.decisions]
    songs_room(b4, whole)
    for c in copies:
        for d in c.decisions:
            print(f'xrecipe_bn4: BN6\'s {c.ty:#x}\'s copy: {d}')
    used = {t for f in whole for t in f.copies}
    text = c_file(whole, [c for c in copies if c.ty in used and not c.decisions])
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
