#!/usr/bin/env python3
"""xnavirecipe.py [--check] [--report] [--ai N ...]: the Navis' recipes, made from the two ROMs (issue #82;
docs/MULTIROM.md, Transplanted Navis; docs/DEVTOOLS.md, The recipe generator).

A Navi's recipe (src/emu/xnavi_recipe.h) is a virus family's (src/emu/xvirus_recipe.h, tools/xrecipe.py) with what a
Navi has more: its code walked from its own states and hooks (the Navi tables', BN5's ten or BN6's thirteen), the
objects it spawns that BN6 has no twin of moved with it through the second-level dispatch, its calls paired with BN6's
twins (tools/bn5_twins.py's pairs and audits, tools/bn5_navi_twins.py's, the symbol map's, a BN6 routine of the same
body), its state table in BN6's layout (BN6's generic 0-7, its own from 8), its pre-step and hooks as BN6's tables
take them (BN6's hook A the empty hook, its frozen and bubbled hooks the paralysis hook), its companion's start and
end, its records (struct1 laid out BN6's way, its battle sprite at the number src/emu/xnavi_sprites.c gives it), its
versions V1, V2, V3 and SP (the DS's chip AI left to a logged stub), what its attacks draw, and the calls to what
BN6's engine lacks aimed at the Navis' stubs.

And the control: BN6's own HeatMan (AI 1) derived by the same walk from BN6's ROM, every call its own routine, every
number its own, moved as AI 25 (tools/xcontrol.py --navi proves it plays frame for frame as HeatMan does).

  python3 tools/xnavirecipe.py              write src/emu/xnavi_bn5.c: the control and NAVIS' recipes, whole
  python3 tools/xnavirecipe.py --check      write nothing: fail where src/emu/xnavi_bn5.c is not what it would write
  python3 tools/xnavirecipe.py --report     every Navi of Team Colonel's and the Dark Soul derived: what each takes and
                                            what still needs a decision, into .build/xnavirecipe/report.md

Local only: both ROMs. Nothing of either is written: the C file holds addresses, counts and CRC32s.
"""
import argparse
import hashlib
import os
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn5_navi_twins as NT  # noqa: E402
import bn5_objects as OB  # noqa: E402
import bn5_twins  # noqa: E402
import xrecipe as X  # noqa: E402
import xrecipe_thumb as T  # noqa: E402
from xrecipe_family import BN5, Deriver, Family  # noqa: E402
from xrecipe_pairs import Pairs  # noqa: E402

ROOT = X.ROOT
OUT_C = os.path.join(ROOT, 'src', 'emu', 'xnavi_bn5.c')
WORK = os.path.join(ROOT, '.build', 'xnavirecipe')

# The Navis written into the C file, in install order: BN5's AI index and the project's name for it. Each one's numbers
# are fixed by its AI index (src/emu/xnavi_recipe.h: XN_AI_OF, XN_ID_OF; src/emu/xnavi_sprites.c's list 2 number); a
# Navi joins this list once its code is proven against BN5's own (tools/xdiff.py)
NAVIS = ((12, 'ToadMan'), (11, 'KnightMan'), (8, 'ShadowMan'), (9, 'NumberMan'), (10, 'TomahawkMan'), (7, 'Colonel'), (0x16, 'the Dark Soul'),
         (14, 'BlizzardMan'), (13, 'ShadeMan'))
CONTROL = (1, 'HeatMan')
SIX = (7, 8, 9, 10, 11, 12)
DARK_SOUL = 0x16
# BN5's Darkloids, the Liberation Missions' (docs/LIBERATION.md, stage 4): ShadeMan, BlizzardMan, CloudMan, CosmoMan
DARKLOIDS = (13, 14, 15, 16)
MOVED = SIX + (DARK_SOUL,) + DARKLOIDS


def seat(ai):
    """its seat among the moved Navis (src/emu/xnavi_recipe.h, XN_SEAT): the six 0-5, the Dark Soul 6, the Darkloids 7-10"""
    return 6 if ai == DARK_SOUL else ai - 6 if ai in DARKLOIDS else ai - 7


def slot_of(ai):
    """its home's, code's and art's slot: its seat, past the control's 7 for the Darkloids"""
    return seat(ai) + (1 if ai in DARKLOIDS else 0)

# The static layout's places (src/emu/free_space.h, xnavi_recipe.h)
BUS = 0x08000000
EMU_FREE = 0x08800000
FREE_NAVI_CODE, FREE_NAVI_ART, FREE_NAVI_HOMES = EMU_FREE + 0x590000, EMU_FREE + 0x5B0000, EMU_FREE + 0x588000
XN_OBJECTS, XN_CODE = FREE_NAVI_CODE + 0x100, FREE_NAVI_CODE + 0x2000
XN_SLOT_CODE, XN_SLOT_ART, XN_HOME_SIZE = 0x3C00, 0x400, 0x800
XN_FIRST_ID, XN_FIRST_AI = 0x1C4, 25
XN_SHARED_ART, XN_END = FREE_NAVI_ART + 8 * XN_SLOT_ART, FREE_NAVI_ART + 0x38000   # (to XN_MORE)
# the Darkloids' rooms (slots 8 on: xnavi_recipe.h's XN_MORE, XN_MORE_ART), past the shared art
XN_MORE = XN_END
XN_MORE_ART = XN_MORE + 4 * XN_SLOT_CODE


def code_at(slot):
    """its slot's code room and the name the C file gives its base"""
    return (XN_CODE + XN_SLOT_CODE * slot, XN_CODE, 'XN_CODE') if slot < 8 else (XN_MORE + XN_SLOT_CODE * (slot - 8), XN_MORE, 'XN_MORE')


def art_at(slot):
    """its slot's art room and the name the C file gives its base"""
    return (FREE_NAVI_ART + XN_SLOT_ART * slot, FREE_NAVI_ART, 'XN_ART') if slot < 8 else \
        (XN_MORE_ART + XN_SLOT_ART * (slot - 8), XN_MORE_ART, 'XN_MORE_ART')
XN_OBJECTS_HOME = FREE_NAVI_HOMES + 15 * XN_HOME_SIZE
CONTROL_SLOT = 7
CODE_STEP = 0x100
# the numbers the Navis' attacks' art takes in the grown lists and effects, issue #82's plan for its tracks (each its own
# range, so their branches merge: the families' effects 0x6C-0x9F, the Navis' 0xA0-0xBF, the Souls' 0xC0-0xCF, the
# chips' 0xD0-0xFF; lists 4 and 5 the families' upward from BN6's end, the Navis' 0xA0-0xB5 and 0xA0-0xA6, the chips'
# downward from 0xFF). A sprite takes one number and one place whichever Navi draws it (the shared art)
NAVI_LIST_NUMBERS = {4: list(range(0xA0, 0xB6)), 5: list(range(0xA0, 0xA7))}
NAVI_FIRST_EFFECT, NAVI_LAST_EFFECT = 0xA0, 0xBF
# BN5's attack family F in BN6's grown table (src/emu/xnavi_recipe.h, XN_FAMILY_OF): 0x50 + F, as the chips' are
XN_FAMILY_FIRST = 0x50
# the routine a Navi's attacks hide its companion's overlay by, a word an AI: BN5's words (BN5_NAVI_HIDE), BN6's
# (BN6_NAVI_HIDE_REF's), and BN5's two routines' twins: the empty companion's, the overlay's hide
HIDE5, HIDE6 = 0x0800F134, 0x08011470
HIDE_TWINS = {0x0800F194: 0x080114D4, 0x080C3836: 0x080C5D42}
# the Navis' battle sprites in BN6's list 2 (src/emu/xnavi_sprites.c: BN5's Navi n at 0x20 + n - 7, the Dark Soul's at 0x26),
# each installed with its Navi: its code's loads of its own (BN5's list 2 number n) take its number there
HELD_CHIP = 0x2A   # an object's chip it uses next (BN6_T1_CHIP), 0xFFFF none
VIDEO_PAST = ((0x05000400, 0x06000000), (0x06018000, 0x07000000), (0x07000400, 0x08000000))
NAVI_LIST = 2
NAVI_SPRITES = {n: 0x20 + seat(n) for n in SIX + DARKLOIDS}
# a sprite of a list the families' room does not grow, copied into one it does: its load's list offset made the other's
# (a sprite's list is read as it is loaded, from the lists' table by the offset, and nowhere after)
NAVI_LIST_MOVES = {3: 4}
# its ice block's size (BN6_ICE_SIZES' 0-2): BN5 freezes none, so each takes BN6's of a Navi its size (BN6's TomahawkMan
# 1, its Colonel 2, its big Navis 2, MegaMan's 1)
ICE = {7: 2, 8: 1, 9: 1, 10: 1, 11: 2, 12: 1, DARK_SOUL: 1, 13: 1, 14: 2, 15: 2, 16: 2}

# each game's Navi tables: their first, step, entries, BN6's table k at its game's index (None: BN6's alone)
NAVI5 = dict(tables=0x080F2374, step=0x5C, count=23, of=(0, 1, None, 2, 3, 4, 5, 6, 7, None, None, 8, 9), held=0x08113C78,
             comp=0x0800EDB4, comp_end=0x0800EF64, names=0x0873C978, end=0x08104550, nullsub=NT.NULLSUB5)
NAVI6 = dict(tables=0x080F36D4, step=0x64, count=25, of=tuple(range(13)), held=0x080B067C, comp=0x08010EA4,
             comp_end=0x080110F4, names=0x086EED00, end=0x08108E4C + 0x200, nullsub=NT.NULLSUB6, ice=0x080EAF8E)
STATES, PRESTEP, HOOK_A, STRUCT1, STRUCT2, POST, SPAWN, SPAWNED, PARALYSIS, FREEZE, BUBBLE, FLINCH, DRAG = range(13)
CODE_FROM = 0x8 + 0x24 + 0xC0   # (a Navi's records: struct1, struct2, its six held lists; its state table and code after)


class NaviDeriver(Deriver):
    """the Deriver's walk and classes on a Navi of `src` (BN5's ROM, or BN6's for the control)"""

    def __init__(self, src, b6, pairs, game, control=False):
        # (the families' Deriver's own state, its walk's and classes' memos, beside the Navi tables')
        self.b5, self.b6, self.pairs, self.game, self.control = src, b6, pairs, game, control
        self.moved_before, self._spawns, self.art_same, self.audit_numbers = {}, {}, None, False
        self.s1s = sorted(set(self.table(ai, STRUCT1) for ai in range(game['count'])))

    def table(self, ai, k):
        """BN6's Navi table k's entry for AI `ai` in its game (None: BN6's alone)"""
        of = self.game['of'][k]
        return None if of is None else self.b5.u32(self.game['tables'] + self.game['step'] * of + 4 * ai)

    def kind_of(self, f, t):
        t &= ~1
        if any(lo <= t < hi for lo, hi in getattr(f, 'regions', [f.region])):
            return 'own', None
        if not self.control and in_objects(t):
            return 'engine', (objects_at(t), 'the moved objects', 'SAME', None)
        if self.control:
            return 'engine', (t, 'BN6\'s own', 'SAME', None)
        if t in NT.GENERIC_TWINS:
            return 'engine', (NT.GENERIC_TWINS[t], 'navis', 'SAME', None)
        if t in NT.COMPANIONS:
            return 'foreign', (None, 'navis', None, None)
        if t in NT.SPAWNS_OTHER or t in NT.COPIED or t in family_routines(f.ai):
            return 'foreign', (None, 'navis', None, None)
        return Deriver.kind_of(self, f, t)

    def find_tables(self, f, w):
        """the jump tables its literals name, as the families' walk finds them, each run on past an entry far from it that
        names an engine routine BN6 has a twin of (an object's routine table names the engine's destroy, BN5's 0x080138EB,
        after its own init and update): such an entry left as raw data would name BN5's address in BN6. And a table its
        code indexes (ldr rX,=table; ... ldr rX,[rX,rY]) whose first entry is such a routine, which the families' walk
        passes over (NumberMan's objects' at 0x080EABC0: BN5's 0x08009FB3 first)"""
        out = Deriver.find_tables(self, f, w)
        for lit, v in w.lits.items():
            if v is None or v & 3 or v in out or not self.b5.has(v, 4) or not self.indexes(w, lit):
                continue
            x = self.b5.u32(v)
            if T.is_code_ptr(x, self.b5) and self.kind_of(f, x)[0] == 'engine':
                out[v] = [x]
        for v, ents in out.items():
            while len(ents) < 48:
                x = self.b5.u32(v + 4 * len(ents))
                near = abs((x & ~1) - v) <= 0x4000
                if not T.is_code_ptr(x, self.b5) or not (near or self.kind_of(f, x)[0] == 'engine') or (near and ents and v + 4 * len(ents) in out):
                    break
                ents.append(x)
        return out

    @staticmethod
    def indexes(w, lit):
        """whether the register a literal is loaded into is then a table's base (ldr rd,[rX,rY] with it as rX) before it
        is written otherwise: a jump table's"""
        for a in sorted(w.insns):
            i = w.insns[a]
            if i.lit != lit:
                continue
            r = i.f[0]
            for b in sorted(x for x in w.insns if x > a)[:4]:
                j = w.insns[b]
                if j.form == 'ldr_r' and j.f[1] == r:
                    return True
                if j.f and j.f[0] == r and T.writes(j, r):
                    break
        return False

    def covered(self, f):
        """the families' cover (its code, pools, tables, and up to 0x80 bytes of the data its literals name), each run of
        that data cut where a routine begins in it (push {..., lr}): another object's code past a table (TomahawkMan's
        0x080C9280, three words before 0x080C928C), which nothing of the Navi runs, is no part of its copy"""
        cov = Deriver.covered(self, f)
        for a in sorted(a for a, k in cov.items() if k == 'RAW' and cov.get(a - 1) != 'RAW'):
            k = 4
            while cov.get(a + k) == 'RAW' and not (k % 2 == 0 and self.b5.u16(a + k) & 0xFF00 == 0xB500):
                k += 1
            while cov.get(a + k) == 'RAW':
                del cov[a + k]
                k += 1
        return cov

    def scan_words(self, f, blocks):
        """the families' scan of its literals, but a word in the video range past any video memory (palettes to
        0x05000400, VRAM to 0x06018000, OAM to 0x07000400) is a number (a spawn's parameters, 0x0704FF01), kept"""
        kept = {}
        for w in f.walks.values():
            for lit, v in w.lits.items():
                if v is not None and any(lo <= v < hi for lo, hi in VIDEO_PAST):
                    f.numbers[lit] = (v, v, 'a number (no video memory there): kept')
                    kept[f'{v:08X} (video) at {lit:08X}'] = True
        for w in f.walks.values():
            for lit, v in w.lits.items():
                if v is not None and v in NT.RAM_TWINS:
                    f.words[v] = (NT.RAM_TWINS[v][0], f'RAM: {NT.RAM_TWINS[v][2]}', None)
        Deriver.scan_words(self, f, blocks)
        f.decisions = [(k, d) for k, d in f.decisions if not (k == 'literal' and d.split(':')[0] in kept)]

    def serves(self, a, tw):
        """whether BN6's twin tw of BN5's routine a runs for it: the control's own, the same body, or an audit's 'ok'"""
        audit = bn5_twins.AUDITS.get((a & ~1, tw[0] & ~1 if tw[0] else None))
        return self.control or tw[2] == 'SAME' or (audit is not None and audit[0] == 'ok')

    def walk_navi(self, f, roots):
        """its code: every routine its own states and hooks reach (BN6's own for the control), the objects it spawns BN6
        has no twin of, the routines it calls BN6 has no twin of; its calls to what BN6 lacks aimed at the stubs"""
        todo = [(r, 'virus') for r in roots]
        seen = set()
        while todo:
            e, ctx = todo.pop()
            if (e, ctx) in seen:
                continue
            seen.add((e, ctx))
            f.ctx.setdefault(e, set()).add(ctx)
            if e in f.walks:
                continue
            w = T.walk(self.b5, e)
            f.walks[e] = w
            w.tables = self.find_tables(f, w)
            # (a routine its tables or literals name: BN6's twin where it is the same body or an audit says it serves,
            # else copied with it: a moved object's parts run as BN5's, never a twin whose body differs unjudged)
            # (a moved object's own table's parts near it copied whatever their twins: its routines read its own data, which a
            # twin of the same code reads of BN6's, NumberMan's companion's sprites)
            for v, ents in w.tables.items():
                for x in ents:
                    kind, tw = self.kind_of(f, x)
                    own_part = ctx == 'object' and abs((x & ~1) - v) <= 0x4000
                    if kind == 'engine' and self.serves(x, tw) and not own_part:
                        f.words[x] = (tw[0] | 1, 'a jump table entry', tw)
                    else:
                        todo.append((x & ~1, ctx))
            for v in w.lits.values():
                if v is None or not T.is_code_ptr(v, self.b5) or v in w.tables:
                    continue
                kind, tw = self.kind_of(f, v)
                if kind == 'engine' and self.serves(v, tw):
                    f.words[v] = (tw[0] | 1, 'a callback', tw)
                else:
                    todo.append((v & ~1, ctx))
            for site, t in w.bls.items():
                self.call(f, w, site, t, ctx, todo)

    def spawn(self, f, w, site, spawner, todo):
        """the dispatch's spawn, but an object the moved objects' unit runs (tools/bn5_objects.py): needed, not walked"""
        v, _ = T.back_value(w, site, 0)
        if (BN5['spawners'][spawner], v) in OBJECT_TYPES and not in_objects(f.region[0]):
            f.calls[spawner] = (None, 'the dispatch', None, None)
            f.needs = getattr(f, 'needs', set()) | {(BN5['spawners'][spawner], v)}
            return
        Deriver.spawn(self, f, w, site, spawner, todo)

    def call(self, f, w, site, t, ctx, todo):
        if not self.control and t in NT.STUBS:
            f.calls[t] = (None, 'stub', None, NT.STUBS[t])
            if t in NT.STUB_ARGS and T.back_value(w, site, 0)[0] != NT.STUB_ARGS[t]:
                f.decide('stub', f'{site:08X}: a call to {t:08X} with r0 another than {NT.STUB_ARGS[t]:#x}, the one its stub serves')
            return
        if not self.control and t in BN5['spawners']:
            self.spawn(f, w, site, t, todo)
            return
        kind, tw = self.kind_of(f, t)
        if kind in ('own', 'foreign'):
            todo.append((t, ctx))
            if kind == 'foreign':
                f.notes.append(f'copies {t:08X} (called at {site:08X}): no BN6 twin, outside the Navi')
        else:
            f.calls[t] = tw
            if kind == 'missing':
                f.decide('call', f'{t:08X} (called at {site:08X}) has no twin in BN6 ({tw[1] if tw else "none"})')


class Recipe:
    pass


# BN5's engine objects moved once, whole, for every track (tools/bn5_objects.py: each module one block at XN_OBJECTS,
# src/emu/xnavi_recipe.h, in MODULES' order); what a Navi's code calls in one reached at its copy there
OBJECT_TYPES = {(m['kind'], m['type']) for m in OB.MODULES}
module_places, in_objects, objects_at = OB.module_places, OB.in_objects, OB.objects_at
assert OB.OBJECTS_AT == XN_OBJECTS and OB.OBJECTS_END == XN_CODE, 'tools/bn5_objects.py\'s place for the moved objects is xnavi_recipe.h\'s'


def family_routines(ai):
    """the routines of BN5's attack families a Navi takes with it (bn5_navi_twins.FAMILIES): each family's and its
    phases'"""
    out = set()
    for a, phases, _ in NT.FAMILIES.get(ai, {}).values():
        out |= {a, *phases}
    return out


def ids_of(rom, ai, kind):
    """the ids of AI `ai` of actor type `kind` in a game's id table, by version"""
    table, n = (0x080182C4, 0x1C4) if rom.name == 'bn6' else (BN5['ids'], BN5['nids'])
    out = {}
    for i in range(n):
        v, k, a = rom.bytes(table + 3 * i, 3)
        if k == kind and a == ai and v not in out:
            out[v] = i
    return [out.get(v) for v in range(6)]


def derive(src, b6, pairs, ai, slot, alloc, control=False):
    game = NAVI6 if control else NAVI5
    d = NaviDeriver(src, b6, pairs, game, control)
    f = Family(ai)
    f.st, f.s1, f.s2 = d.table(ai, STATES), d.table(ai, STRUCT1), d.table(ai, STRUCT2)
    nxt = [x for x in d.s1s if x > f.s1]
    f.region = (f.s1, nxt[0] if nxt else game['end'])
    f.last = not nxt
    f.ids5 = ids_of(src, ai, 1)
    f.states5 = []
    for k in range(24):
        v = src.u32(f.st + 4 * k)
        if not T.is_code_ptr(v, src):
            break
        f.states5.append(v)
    hooks = {k: d.table(ai, k) for k in (PRESTEP, POST, SPAWN, SPAWNED, PARALYSIS, FLINCH, DRAG, HOOK_A, FREEZE, BUBBLE)}
    # (its states and hooks of its own, and those its game keeps in another Navi's code, no BN6 twin's, copied with it:
    # ShadeMan's 9 and 12, in his last form's, AI 21's)
    roots = [s & ~1 for k, s in enumerate(f.states5) if d.kind_of(f, s)[0] in ('own', 'foreign')]
    roots += [h & ~1 for h in hooks.values() if h and d.kind_of(f, h)[0] in ('own', 'foreign')]
    comp = [src.u32(game[k] + 4 * ai) for k in ('comp', 'comp_end')]
    companion_start = comp[0] & ~1
    roots += [c & ~1 for c in comp if d.kind_of(f, c)[0] in ('own', 'foreign')]
    roots += [a for fam, (a, _, _) in NT.FAMILIES.get(ai, {}).items()] + list(NT.EXTRA_ROOTS.get(ai, {}))
    d.walk_navi(f, sorted(set(roots)))
    blocks = blocks_of(d, f)
    for _ in range(8):
        dead = [x for x in d.dead_roots(f, blocks) if x not in f.walks]
        # (a routine its words name BN6's twin of, which its blocks copy anyway: the copy, so the word and the block agree)
        inside = lambda v: any(lo <= v < hi for lo, hi in blocks)
        inner = [v & ~1 for v in f.words if v & 1 and inside(v & ~1) and (v & ~1) not in f.walks]
        inner += [t for w in list(f.walks.values()) for t in w.bls.values() if inside(t) and t not in f.walks and t not in inner]
        for v in inner:
            f.words.pop(v | 1, None)
            f.calls.pop(v, None)
        if not dead and not inner:
            break
        f.notes += [f'dead code at {x:08X} (nothing names it): walked and fixed up as if live' for x in dead]
        f.notes += [f'{x:08X}: a twin\'s routine inside its blocks, taken as its own copy' for x in inner]
        d.walk_navi(f, dead + inner)
        blocks = blocks_of(d, f)
    for _ in range(8):
        f.data = pointer_words(src, d.classify(f, blocks), blocks)
        dead = [int(det.split('-')[0], 16) for k, det in f.decisions if k == 'unreached']
        # (a routine of its game's between a block and the pool it reads, joined to it, nothing of the Navi's calls: its
        # bytes copied as they are, dead in the copy)
        gap = lambda a: any(lo <= a < hi for lo, hi in getattr(f, 'joined_gaps', ()))
        f.notes += [f'{a:08X}: a routine between its code and a pool it reads, which nothing of it calls: copied as it is, dead'
                    for a in dead if gap(a)]
        f.decisions = [x for x in f.decisions if not (x[0] == 'unreached' and gap(int(x[1].split('-')[0], 16)))]
        dead = [a for a in dead if not gap(a)]
        if not dead:
            break
        # (code no branch reaches, a compiler's leftover after one: walked as if live, so its calls and pools hold in the copy)
        f.decisions = [x for x in f.decisions if x[0] != 'unreached']
        f.notes += [f'unreached code at {x:08X}: walked and fixed up as if live' for x in dead]
        d.walk_navi(f, dead)
        blocks = blocks_of(d, f)
    if not control:
        d.scan(f)
        unflipped(f)
        raw_code_pointers(d, f, src)
    d.scan_words(f, blocks) if not control else control_words(d, f, blocks)
    r = Recipe()
    r.f, r.ai5, r.slot, r.control = f, ai, slot, control
    r.companion_start = companion_start
    r.name = X.text(src, game['names'], f.ids5[0] - 0x100) if f.ids5[0] is not None else ''
    r.ai = XN_FIRST_AI if control else 32 + seat(ai)
    r.first_id = XN_FIRST_ID + (0 if control else 4 * seat(ai))
    r.home = FREE_NAVI_HOMES + XN_HOME_SIZE * slot
    r.versions = 6 if control else (1 if ai == DARK_SOUL else 4)
    room = code_at(slot)[0]
    r.blocks, at = [], room
    for lo, hi in blocks:
        at = X.rounded(at, CODE_STEP) + (lo & 3)
        r.blocks.append((lo, hi, at, zlib.crc32(src.bytes(lo, hi - lo)) & 0xFFFFFFFF))
        at += hi - lo
    if at > room + XN_SLOT_CODE:
        f.decide('room', f'its code ends at {at:08X}, past its slot\'s {XN_SLOT_CODE:#x} bytes')
    r.data = f.data
    calls_of(r, d, blocks)
    r.clones = []
    states_of(r, d)
    hooks_of(r, d, hooks, comp)
    r.s1, r.s2 = f.s1, f.s2
    r.names = [x if x is not None else 0 for x in f.ids5]
    # (BN5's name plates show ProtoMan V3 and Colonel V3 by their V1's names: 0x08026CC2)
    r.shown = [r.names[0] if (not control and r.names[v] in (0x103, 0x127)) else 0 for v in range(6)]
    s1 = src.bytes(f.s1, 8)
    r.traits = None if control else dict(mode=s1[4], shadow=bool(s1[5]))
    r.ice = src.u8(NAVI6['ice'] + ai) if control else ICE.get(ai, 1)
    r.held, r.held_chips = held_of(r, src, game, control)
    r.sprite_of = -1 if control else ai
    r.imms, r.words, r.numbers, r.sounds = [], [], [], []
    r.sprites, r.effects, r.types, r.more_imms, r.held_out = [], [], [], [], []
    r.overlay, r.bytes, r.needs, r.kinds = None, [], [], []
    r.families = [(fam, XN_FAMILY_FIRST + fam, a | 1) for fam, (a, _, _) in sorted(NT.FAMILIES.get(ai, {}).items())]
    if not control:
        for k, d in [x for x in f.decisions if x[0] == 'curaction' and int(x[1].split(':')[0], 16) in NT.OWN_STATES]:
            f.decisions.remove((k, d))
            f.notes.append(f'{d.split(":")[0]}: {NT.OWN_STATES[int(d.split(":")[0], 16)]}: no Navi state, kept')
        for k, d in [x for x in f.decisions if x[0] == 'curaction' and int(x[1].split(':')[0], 16) in NT.CURACTIONS]:
            f.decisions.remove((k, d))
            f.notes.append(f'{d.split(":")[0]}: {NT.CURACTIONS[int(d.split(":")[0], 16)]}')
        more_paths(r)
        overlay_of(r, src)
        chips_of(r, src, b6)
        art_of(r, src, b6, alloc)
        X.types_of(r)
        types_decided(r)
        needs_of(r)
        table_sounds(r, src, b6)
        sounds_of(r, src, b6)
        panel_types(r)
        parent_curactions(r)
        navi_ai_cmps(r)
        X.conditions_of(r, pairs)
        r.imms = X.unique_imms(f.imms + r.more_imms)
        r.numbers = sorted((lit, was, now) for lit, (was, now, why) in f.numbers.items())
        if s1[0] != 8 and ai != DARK_SOUL:
            f.decide('struct1', f'its battle sprite is of list {s1[0] // 4}, not list 2')
    r.words = sorted((v, to) for v, (to, why, tw) in f.words.items())
    return r


def image_kinds_fixed(r, src, b6, alloc):
    """the image object's kinds' sprites as tools/bn5_objects.py fixes them: each twin checked by its bytes, each copy at its
    number in the shared art; every other kind BN5's bytes, noted"""
    o, f = OB.IMAGE, r.f
    for k in range(o['count']):
        at = o['kinds'] + 5 * k
        l5, i5 = src.u8(at), src.u8(at + 1)
        lst = l5 // 4
        if k in OB.IMAGE_TWIN:
            tl, ti = OB.IMAGE_TWIN[k]
            same, _ = bn6_twin(src, b6, lst, i5)
            if tl != lst or same != ti:
                f.decide('image', f'its kind {k:#x}: BN5\'s sprite {lst}:{i5:#x} is not BN6\'s {tl}:{ti:#x} ({same})')
            elif ti != i5:
                r.bytes.append(dict(at=at + 1, was=i5, now=ti, why=f'its kind {k:#x}: BN5\'s sprite {lst}:{i5:#x}, BN6\'s {tl}:{ti:#x} the same bytes'))
        elif k in OB.IMAGE_COPIED and OB.IMAGE_COPIED[k][2] == 'souls':
            # (a kind of the Souls' alone, its sprite in their own list 0: its bytes fixed here, its sprite copied and listed by
            # the Souls' install, src/emu/soulcode.c, as it grows list 0)
            tl, ti, _ = OB.IMAGE_COPIED[k]
            why = f'its kind {k:#x}: BN5\'s sprite {lst}:{i5:#x}, the Souls\' at {tl}:{ti:#x} (their install copies and lists it)'
            r.bytes += [dict(at=at + n, was=w, now=v, why=why) for n, w, v in ((0, l5, 4 * tl), (1, i5, ti)) if w != v]
        elif k in OB.IMAGE_COPIED:
            tl, ti, track = OB.IMAGE_COPIED[k]
            to, x = alloc.shared(src, lst, i5, fixed=(tl, ti))
            if to is None:
                f.decide('image', f'its kind {k:#x}: BN5\'s sprite {lst}:{i5:#x} has no place at {tl}:{ti:#x}')
                continue
            if not any(y['list'] == lst and y['index'] == i5 for y in r.sprites):
                r.sprites.append(dict(x))
            why = f'its kind {k:#x}: BN5\'s sprite {lst}:{i5:#x}, copied at {tl}:{ti:#x} ({track})'
            if 4 * tl != l5:
                r.bytes.append(dict(at=at, was=l5, now=4 * tl, why=why))
            if ti != i5:
                r.bytes.append(dict(at=at + 1, was=i5, now=ti, why=why))
        else:
            f.notes.append(f'kind {k:#x}: BN5\'s sprite {lst}:{i5:#x} kept (no installed code spawns it)')


def objects_art(r, src, b6, alloc):
    """the modules' own draws: a sprite loaded by its number BN6's of the same bytes (else a decision), an effect BN6's of
    the same sprite, animation and palette or one of the Navis' range (its sprite in the shared art), the immediate that
    names it BN6's; no sound"""
    f = r.f
    if f.sounds:
        f.decide('art', 'its code starts a sound')
    for (l, v), sites in sorted(f.loads.items(), key=lambda x: str(x[0])):
        if (l, v) == (None, None):
            continue   # (the image object's, by its kind's entry: image_kinds_fixed)
        same = bn6_twin(src, b6, l // 4, v)[0] if l is not None and v is not None else None
        if same != v:
            f.decide('sprite', f'its sprite {l}:{v}: BN6 lists its bytes at {same}')
    for e, sites in sorted(list(f.effects.items()) + [(x, []) for x in OB.ENGINE_EFFECTS], key=lambda x: str(x[0])):
        if e is None:
            f.decide('effect', f'an effect id made otherwise at {", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        e6 = X.effect_twin(src, b6, e, lambda lst, idx: bn6_twin(src, b6, lst, idx))
        if e6 is None:
            e6, _ = alloc.number('effect', e)
            row = src.bytes(BN5['effects'] + 4 * e, 4)
            if (e, e6) not in r.effects:
                r.effects.append((e, e6))
                if bn6_twin(src, b6, row[0] // 4, row[1])[0] is None:
                    to, x = alloc.shared(src, row[0] // 4, row[1])
                    if to is None:
                        f.decide('effect', f'its effect {e:#x}\'s sprite has no place')
                    elif not any(y['list'] == x['list'] and y['index'] == x['index'] for y in r.sprites):
                        r.sprites.append(dict(x))
                else:
                    f.decide('effect', f'its effect {e:#x} draws BN6\'s sprite: an entry naming BN6\'s wanted')
        if e6 != e:
            for s_, chain in sites:
                X.imm_for(r, chain, 4, e, e6, f'its effect: {e:#x} in BN5, {e6:#x} in BN6')


def derive_objects(src, b6, pairs, alloc):
    """the moved objects' unit (tools/bn5_objects.py): each module whole, one block, walked from each of its routines; the
    image object's kinds' sprites as the table fixes them; the modules' own sprites and effects as BN6 numbers them"""
    d = NaviDeriver(src, b6, pairs, NAVI5)
    f = Family(-1)
    f.regions = [m['region'] for m in OB.MODULES]
    f.region, f.st, f.s1, f.s2, f.states5, f.ids5 = f.regions[0], f.regions[0][0], f.regions[0][0], None, [], [None] * 6
    d.walk_navi(f, [x for m in OB.MODULES for x in m['roots']])
    blocks = list(f.regions)
    outside = sorted(a for a in d.covered(f) if not any(lo <= a < hi for lo, hi in blocks))
    if outside:
        f.decide('block', f'its walk covers {outside[0]:08X}, outside its modules')
    f.data = pointer_words(src, d.classify(f, blocks), blocks)
    d.scan(f)
    raw_code_pointers(d, f, src)
    d.scan_words(f, blocks)
    r = Recipe()
    r.f, r.ai5, r.slot, r.control = f, -1, None, False
    r.blocks = [(lo, hi, objects_at(lo), zlib.crc32(src.bytes(lo, hi - lo)) & 0xFFFFFFFF) for lo, hi in blocks]
    end = max(objects_at(hi - 2) + 2 for lo, hi in blocks)
    if end > XN_CODE:
        f.decide('room', f'its modules end at {end:08X}, past the moved objects\' room')
    r.data = f.data
    calls_of(r, d, blocks)
    r.sprites, r.effects, r.types, r.bytes, r.needs, r.kinds, r.more_imms = [], [], [], [], [], [], []
    X.types_of(r)
    types_decided(r)
    X.conditions_of(r, pairs)
    image_kinds_fixed(r, src, b6, alloc)
    objects_art(r, src, b6, alloc)
    r.imms = X.unique_imms(f.imms + r.more_imms)
    r.numbers = sorted((lit, was, now) for lit, (was, now, why) in f.numbers.items())
    r.words = sorted((v, to) for v, (to, why, tw) in f.words.items())
    return r


def table_sounds(r, b5, b6):
    """the sounds its code starts from a table of its kinds (tools/bn5_navi_twins.py, SOUND_TABLES): each entry's the
    same song in both, a note; else a decision"""
    f = r.f
    for site in [s for s, _ in f.sounds.get(None, []) if s in NT.SOUND_TABLES]:
        table, size, off, n = NT.SOUND_TABLES[site]
        songs = [b5.u16(table + size * k + off) for k in range(n)]
        if all(X.song_sig(b5, BN5['songs'], v, True) == X.song_sig(b6, X.BN6['songs'], v, True) for v in songs):
            f.notes.append(f'{site:08X}: its sounds from the table at {table:08X} ({", ".join(f"{v:#x}" for v in sorted(set(songs)))}): '
                           f'the same songs in both')
        else:
            f.decide('sound', f'{site:08X}: a sound of its table at {table:08X} another song in BN6')
    f.sounds[None] = [(s, c) for s, c in f.sounds.get(None, []) if s not in NT.SOUND_TABLES]
    if not f.sounds[None]:
        del f.sounds[None]


def sounds_of(r, b5, b6):
    """the sounds its code starts: each BN6 has as the same song kept; each it has as another (or none) copied in by the
    installer (src/emu/xsong.c) and named by its slot there, through the immediates that make its number (movs r0,#n; or
    movs r0,#lo then adds r0,#0xFF)"""
    r.sounds, r.xsounds = [], []
    for v, sites in sorted(r.f.sounds.items(), key=lambda x: str(x[0])):
        if v is None:
            r.f.decide('sound', f'a sound made otherwise at {", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        same = X.song_sig(b5, BN5['songs'], v, True) == X.song_sig(b6, X.BN6['songs'], v, True) or v in NT.SOUNDS_AS_BN6
        r.sounds.append((v, same))
        if same:
            continue
        for site, chain in sites:
            forms = [c[1] for c in chain]
            if forms == ['movs'] and v <= 0xFF:
                r.xsounds.append((chain[0][0], 0, v))
            elif forms == ['adds8', 'movs'] and chain[0][2] == 0xFF and v > 0xFF:
                r.xsounds.append((chain[1][0], chain[0][0], v))
            else:
                r.f.decide('sound', f'its sound {v:#x} at {site:08X} made by {chain}')
    r.xsounds = sorted(set(r.xsounds))


def back_values(w, site, reg, via=(), depth=4):
    """the value `reg` holds at `site` along each way into it (tools/xrecipe_thumb.py's back_value, which reads straight
    back): the straight way (via ()), and the way through each branch from elsewhere into the instructions between the
    value's making and the site, followed back from the branch (an if that sets the register on one way alone):
    [(value or None, chain, via: the branches taken, the last first)]"""
    v, chain = T.back_value(w, site, reg)
    out = [(v, chain, via)]
    if depth and chain:
        made = chain[-1][0]
        for a in sorted(w.insns):
            i = w.insns[a]
            if i.form in ('b', 'bcc') and i.target is not None and made < i.target <= site and not made <= a < site:
                out += back_values(w, a, reg, via + (a,), depth - 1)
    return out


def shifted_up(w, a):
    """whether the instruction at a is lsls rd,rs,#8 (a parameter shifted past an id's byte)"""
    i = w.insns.get(a)
    return i is not None and i.form == 'lsls' and i.f[2] == 8


def low_byte_effects(f):
    """the effects its code spawns with r4 made as a parameter shifted up and the effect id added (lsls r4,rX,#8; adds
    r4,#id), straight or with the adds reached by a branch past another way's: the id the adds' immediate, BN6's made by
    it"""
    f.effect_rd, f.effect_ranges = {}, []
    for site, chain in f.effects.pop(None, []):
        w = next(w for w in f.walks.values() if site in w.insns)
        if len(chain) == 2 and chain[0][1] == 'adds8' and chain[1][1] == 'lsls' and shifted_up(w, chain[1][0]):
            f.effects.setdefault(chain[0][2], []).append((site, chain[:1]))
            continue
        if chain and chain[0][1] == 'adds':
            # (adds r4,r4,rX with movs rX,#id just before it and the parameter shifted up before that: TomahawkMan's 0x46; or
            # with adds rX,#first a number of ids from a variable, one of tools/bn5_navi_twins.py's EFFECT_RANGES)
            a = chain[0][0]
            i, before = w.insns[a], sorted(x for x in w.insns if x < a)[-4:]
            first = [x for x in before if w.insns[x].form == 'adds8' and w.insns[x].f[0] == i.f[2] and x in NT.EFFECT_RANGES]
            if i.f[:2] == (4, 4) and first:
                f.effect_ranges.append((site, first[-1], w.insns[first[-1]].f[0], w.insns[first[-1]].f[1], NT.EFFECT_RANGES[first[-1]][0]))
                continue
            movs = [x for x in before if w.insns[x].form == 'movs' and w.insns[x].f[0] == i.f[2]]
            shifts = [x for x in before if shifted_up(w, x) and w.insns[x].f[0] == 4]
            if i.f[:2] == (4, 4) and movs and shifts and shifts[-1] < movs[-1]:
                k = w.insns[movs[-1]].f[1]
                f.effect_rd[site] = i.f[2]
                f.effects.setdefault(k, []).append((site, [(movs[-1], 'movs', k)]))
                continue
        if len(chain) == 1 and chain[0][1] == 'adds8':
            # (the adds the first of its way: each branch to it with the parameter shifted up before it, nothing added)
            made = chain[0][0]
            ins = [a for a, i in w.insns.items() if i.form in ('b', 'bcc') and i.target == made]
            ways = [T.back_value(w, a, 4)[1] for a in ins]
            if ins and all(len(c) == 1 and c[0][1] == 'lsls' and shifted_up(w, c[0][0]) for c in ways):
                f.effects.setdefault(chain[0][2], []).append((site, chain))
                continue
        f.effects.setdefault(None, []).append((site, chain))


def more_paths(r):
    """what its code loads, spawns as an effect or plays where the number depends on the way into the call: each way's
    number added beside the straight way's (the walk's), a sprite's list and number paired by their way"""
    f = r.f
    walk_of = lambda a: next(w for w in f.walks.values() if a in w.insns)
    for (l, v), sites in list(f.loads.items()):
        for site, _ in sites:
            w = walk_of(site)
            lists = {via: x for x, _, via in back_values(w, site, 1)}
            for x, chain, via in back_values(w, site, 2)[1:]:
                key = (lists.get(via, lists[()]), x)
                if (site, chain) not in f.loads.setdefault(key, []):
                    f.loads[key].append((site, chain))
    for table, reg in ((f.effects, 4), (f.sounds, 0)):
        for v, sites in list(table.items()):
            for site, _ in sites:
                for x, chain, via in back_values(walk_of(site), site, reg)[1:]:
                    key = None if x is None else (x & 0xFF if table is f.effects else x)
                    if (site, chain) not in table.setdefault(key, []):
                        table[key].append((site, chain))
    low_byte_effects(f)


def types_decided(r):
    """each object type its code spawns decided as tools/bn5_twins.py's TYPES decides it (one decision a type, every
    track's), else a decision"""
    for kind, t5, t6, why in r.types:
        d = bn5_twins.TYPES.get((kind, t5))
        want = 'moved' if t6 is None else 'twin'
        if d is None or d[0] != want or (want == 'twin' and d[1] != t6):
            r.f.decide('type', f'the {kind} kind\'s type {t5:#x}: {want}{"" if t6 is None else f" {t6:#x}"} here ({why}), '
                               f'{d[0] + (f" {d[1]:#x}" if d[1] is not None else "") if d else "undecided"} in tools/bn5_twins.py')


def needs_of(r):
    """the moved objects it needs (tools/bn5_objects.py): a type of theirs it spawns, a routine of theirs its code calls or
    names; and the image object's kinds it spawns, each one the table fixes (a kind left with BN5's bytes a decision)"""
    f = r.f
    needs = set(getattr(f, 'needs', set()))
    for m in OB.MODULES:
        lo, hi = m['region']
        if any(c['bn6'] is not None and c['source'] == 'the moved objects' and lo <= c['bn5'] < hi for c in r.calls) or \
                any(lo <= (v & ~1) < hi for v in f.words):
            needs.add((m['kind'], m['type']))
    r.needs = sorted(needs)
    r.kinds = sorted(image_kinds(r))
    for k in r.kinds:
        if k not in OB.IMAGE_TWIN and k not in OB.IMAGE_COPIED:
            f.decide('image', f'its image kind {k:#x}: unfixed in tools/bn5_objects.py (its sprite BN5\'s bytes in the moved copy)')


def image_kinds(r):
    """the image object's kinds its code spawns (tools/bn5_objects.py, IMAGE): {kind: [(the instruction that makes it,
    its form)]}, the kind r4's low byte, made last before the call by movs r4,#kind or adds r4,#kind"""
    f, out = r.f, {}
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t != OB.IMAGE['spawner']:
                continue
            if site in NT.IMAGE_KINDS:
                for k in NT.IMAGE_KINDS[site][0]:
                    out.setdefault(k, [])
                continue
            last = next((a for a in sorted((a for a in w.insns if a < site), reverse=True) if T.writes(w.insns[a], 4)), None)
            i = w.insns.get(last)
            if i is None or i.form not in ('movs', 'adds8') or i.f[0] != 4:
                f.decide('image', f'{site:08X}: the image object spawned with a kind made otherwise')
                continue
            out.setdefault(i.f[1], []).append((last, 'THUMB_MOVS' if i.form == 'movs' else 'THUMB_ADDS'))
    return out


def sprite_tables(r, f):
    """the sprites its code loads from a table of (list offset, number) pairs by an index (ldr rT,=table; adds rI,rI,rT;
    ldrb r1,[rI]; ldrb r2,[rI,#1]: NumberMan's companion's by its kind, 0x080C33D6): (the table, its entries, the load),
    the entries the RAW range the table begins (its copy's bytes given BN6's numbers, XvByte)"""
    out = []
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t != BN5['sprite_load']:
                continue
            before = sorted(a for a in w.insns if a < site)[-8:]
            loads = {w.insns[a].f[0]: w.insns[a] for a in before if w.insns[a].form == 'ldrb'}
            if not (1 in loads and 2 in loads and loads[1].f[1:] == (loads[2].f[1], 0) and loads[2].f[2] == 1):
                continue
            lits = [w.lits.get(w.insns[a].lit) for a in before if w.insns[a].form == 'ldr_lit']
            table = next((v for v in lits if v is not None and BUS <= v < BUS + 0x2000000), None)
            raw = next(((lo, hi) for lo, hi, k in f.data if k == 'RAW' and lo == table), None)
            if table is None or raw is None:
                f.decide('sprite', f'{site:08X}: a sprite from a table its walk cannot place ({table})')
                continue
            out.append((table, (raw[1] - raw[0]) // 2, site))
    return out


def list_chain(f, site, chain):
    """the chain that makes a sprite load's list offset (r1) on the way its number's chain (r2) came by"""
    w = next(w for w in f.walks.values() if site in w.insns)
    vias = [via for _, c, via in back_values(w, site, 2) if c == chain]
    lists = {via: c for _, c, via in back_values(w, site, 1)}
    return lists.get(vias[0] if vias else (), lists[()])


def packed_sprites(r):
    """the sprites its code sets up through a packed word (tools/bn5_navi_twins.py, PACKED_SPRITES): (its literal's address,
    the word) for each, a word made otherwise a decision"""
    f, out = r.f, set()
    for w in f.walks.values():
        for a, t in w.bls.items():
            if t not in NT.PACKED_SPRITES:
                continue
            v, chain = T.back_value(w, a, 0)
            lit = w.insns[chain[0][0]].lit if chain and chain[0][1] == 'ldr_lit' and len(chain) == 1 else None
            if lit is None or v is None:
                f.decide('sprite', f'{a:08X}: a sprite set up by a packed word made otherwise ({chain})')
                continue
            out.add((lit, v))
    return out


def navi_ai_cmps(r):
    """its copied code's compares of a Navi's AI index with BN5's number (tools/bn5_navi_twins.py, NAVI_AI_CMPS): BN6's
    number for it, the instruction checked"""
    for w in r.f.walks.values():
        for a in sorted(set(w.insns) & set(NT.NAVI_AI_CMPS)):
            x = w.insns[a]
            v = x.f[1] if x.form == 'cmp' else None
            if v not in MOVED:
                r.f.decide('navi ai', f'{a:08X}: not a compare with a moved Navi\'s AI index ({x.form})')
                continue
            to = 32 + seat(v)
            r.more_imms.append(dict(at=a, form='THUMB_CMP', rd=x.f[0], rn=0, was=v, now=to,
                                    why=f'{NT.NAVI_AI_CMPS[a]}: BN5\'s AI index {v:#x}, BN6\'s {to:#x}'))


def unflipped(f):
    """its writes of its object's DirectionFlip (+0x17) that write 0, the way it stands unflipped as it spawns: no flip
    (the collision panels' highlight pair's no_flip), noted"""
    keep = []
    for a in f.flip:
        w = next((w for w in f.walks.values() if a in w.insns), None)
        v, chain = T.back_value(w, a, w.insns[a].f[0]) if w else (None, None)
        if v == 0 and chain and len(chain) == 1 and chain[0][1] == 'movs':
            f.notes.append(f'{a:08X}: its DirectionFlip written 0, unflipped: no flip')
        else:
            keep.append(a)
    f.flip = keep


def parent_curactions(r):
    """its moved objects' compares of their parent's CurAction (ldr rP,[r5,#0x4C]: the Navi that spawned them; ldrb rX,
    [rP,#9]) with a state of the Navi's own (6 on in BN5): BN6's number, two up (its own states from 8), the compare's
    immediate or the movs that makes its other register checked"""
    for w in r.f.walks.values():
        addrs = sorted(w.insns)
        parents = set()
        for k, a in enumerate(addrs):
            i = w.insns[a]
            if i.form == 'ldr' and i.f[1:] == (5, 0x4C):
                parents.add(i.f[0])
                continue
            if not (i.form == 'ldrb' and i.f[1] in parents and i.f[2] == 9):
                if i.form not in ('cmp',) and i.f and T.writes(i, i.f[0]) and i.f[0] in parents and i.form != 'ldrb':
                    parents.discard(i.f[0])
                continue
            for b in addrs[k + 1:k + 9]:
                j = w.insns[b]
                if j.form == 'cmp' and j.f[0] == i.f[0]:
                    if '#' in j.text:
                        v, at, rd = j.f[1], b, j.f[0]
                    else:
                        v, chain = T.back_value(w, b, j.f[1])
                        if not (chain and len(chain) == 1 and chain[0][1] == 'movs'):
                            break   # (a number read otherwise: TomahawkMan's 0x080C918A and 0x080C91AE, proven as they are)
                        at, rd = chain[0][0], j.f[1]
                    if v is not None and 6 <= v <= 0xF:
                        r.more_imms.append(dict(at=at, form='THUMB_CMP' if at == b else 'THUMB_MOVS', rd=rd, rn=0, was=v, now=v + 2,
                                                why=f'its parent\'s CurAction {v} compared: BN6\'s {v + 2}'))
                    break
                if j.form in ('b', 'bx', 'bl', 'hi_mov') or T.is_return(j) or T.writes(j, i.f[0]):
                    break


def panel_types(r):
    """the panel types its code sets (setPanelType's r2) or compares with one read from a panel (its +2, after a panel's
    getter), BN6's numbers for BN5's lava, sea and holy (tools/bn5_navi_twins.py, PANEL_TYPES); those set taken off the
    list the setPanelType pair's condition reads, now BN6's own"""
    f = r.f
    for w in f.walks.values():
        addrs = sorted(w.insns)
        for k, a in enumerate(addrs):
            i = w.insns[a]
            if i.form != 'bl':
                continue
            if i.target == NT.SET_PANEL_TYPE:
                v, chain = T.back_value(w, a, 2)
                if v in NT.PANEL_TYPES:
                    X.imm_for(r, chain, 2, v, NT.PANEL_TYPES[v], f'a panel set to BN5\'s type {v:#x}: BN6\'s {NT.PANEL_TYPES[v]:#x}')
            if i.target not in NT.PANEL_GETTERS:
                continue
            for n, b in enumerate(addrs[k + 1:k + 6]):
                j = w.insns[b]
                if j.form == 'ldrb' and j.f[1:] == (0, 2):
                    for c in addrs[k + 2 + n:k + 5 + n]:
                        x = w.insns[c]
                        if x.form == 'cmp' and x.f[0] == j.f[0] and x.f[1] in NT.PANEL_TYPES:
                            r.more_imms.append(dict(at=c, form='THUMB_CMP', rd=x.f[0], rn=0, was=x.f[1], now=NT.PANEL_TYPES[x.f[1]],
                                                    why=f'a panel\'s type compared with BN5\'s {x.f[1]:#x}: BN6\'s {NT.PANEL_TYPES[x.f[1]]:#x}'))
                    break
                if T.writes(j, 0):
                    break
    for a, v in f.panel_types:
        if v is None and a in NT.PANEL_SITES:
            f.notes.append(f'{a:08X}: {NT.PANEL_SITES[a][1]}')
    f.panel_types = [(a, NT.PANEL_SITES[a][0] if v is None and a in NT.PANEL_SITES else v) for a, v in f.panel_types]
    f.panel_types = [(a, v) for a, v in f.panel_types if v not in NT.PANEL_TYPES]


def blocks_of(d, f):
    """its blocks: its region's code from its state table or its records' end (whichever first), the code it copies outside"""
    cov = d.covered(f)
    inside = [a for a in cov if f.region[0] <= a < f.region[1]]
    lo = min(f.s1 + CODE_FROM, f.st) if f.region[0] <= f.st < f.region[1] else f.s1 + CODE_FROM
    if inside and min(inside) < lo:
        f.decide('block', f'code at {min(inside):08X} before its records end ({lo:08X})')
        lo = min(inside) & ~1
    hi = (max(max(inside) + 1, f.st + 4 * len(f.states5)) + 3) & ~3 if inside else lo
    blocks = [(lo, hi)] if inside else []
    run = None
    for a in sorted(a for a in cov if not f.region[0] <= a < f.region[1]):
        if run and a <= run[1] + 4:
            run[1] = a + 1
        else:
            if run:
                blocks.append((run[0], (run[1] + 1) & ~1))
            run = [a, a + 1]
    if run:
        blocks.append((run[0], (run[1] + 1) & ~1))
    return pools_joined(f, blocks)


def pools_joined(f, blocks):
    """its blocks with each one joined to the next it reads a literal from (a pool shared past another routine: NumberMan's
    0x080C5184 reads 0x080C543C, the pool after 0x080C53DE): a load reaches its literal only in its own block's copy"""
    blocks = list(blocks)
    while True:
        at = lambda a: next((k for k, (lo, hi) in enumerate(blocks) if lo <= a < hi), None)
        cut = next(((i, j) for w in f.walks.values() for a, ins in w.insns.items() if ins.form == 'ldr_lit'
                    for i, j in [(at(a), at(ins.lit))] if i is not None and j is not None and i != j), None)
        if cut is None:
            return blocks
        lo, hi = min(blocks[cut[0]][0], blocks[cut[1]][0]), max(blocks[cut[0]][1], blocks[cut[1]][1])
        f.joined_gaps = getattr(f, 'joined_gaps', set()) | {(min(b[1] for b in (blocks[cut[0]], blocks[cut[1]])),
                                                             max(b[0] for b in (blocks[cut[0]], blocks[cut[1]])))}
        # (the two joined in the first's place, with any block between them)
        first = min(k for k, (a, b) in enumerate(blocks) if lo <= a < hi)
        blocks = [x for k, x in enumerate(blocks) if k == first or not lo <= x[0] < hi]
        blocks[blocks.index(next(x for x in blocks if lo <= x[0] < hi))] = (lo, hi)


def pointer_words(src, data, blocks):
    """a raw range's words that point into its blocks (a table of pointers to its own data, a pool's words no walk
    loads) as literal words, moved with the code they name; and each raw range split where such a word, or a pool's,
    points into it (the installer moves a pointer to a data range's start alone)"""
    inside = lambda v: any(lo <= (v & ~1) < hi for lo, hi in blocks)
    words = [w for lo, hi, k in data if k in ('RAW', 'POOL') for w in range((lo + 3) & ~3, hi - 3, 4) if inside(src.u32(w))]
    # (a word naming its raw data, at an even address or an odd one, bytes: the chip AI's regions' table, 0x08022AF8; an odd
    # one that names code is a Thumb routine's)
    in_raw = lambda v: any(lo <= v < hi for lo, hi, k in data if k == 'RAW')
    targets = {src.u32(w) for w in words if not src.u32(w) & 1 or in_raw(src.u32(w))}
    out = []
    for lo, hi, k in data:
        if k != 'RAW':
            out.append((lo, hi, k))
            continue
        cuts = sorted({lo, hi} | {t for t in targets if lo < t < hi} | {w for w in words if lo <= w < hi} |
                      {w + 4 for w in words if lo <= w < hi and w + 4 < hi})
        for a, b in zip(cuts, cuts[1:]):
            out.append((a, b, 'POOL' if a in words and b == a + 4 else 'RAW'))
    merged = []
    for lo, hi, k in out:
        if merged and merged[-1][2] == k == 'POOL' and merged[-1][1] == lo:
            merged[-1] = (merged[-1][0], hi, k)
        else:
            merged.append((lo, hi, k))
    return merged


def raw_code_pointers(d, f, src):
    """a word of its raw data that names a routine of its game's ROM outside its blocks (a table no walk read whole): a
    decision, as BN6 would read BN5's address there"""
    for lo, hi, k in f.data:
        if k != 'RAW':
            continue
        for a in range((lo + 3) & ~3, hi - 3, 4):
            v = src.u32(a)
            if T.is_code_ptr(v, src) and src.u16(v & ~1) & 0xFF00 == 0xB500 and d.kind_of(f, v)[0] != 'own':
                f.decide('raw pointer', f'{a:08X}: its raw data names the routine {v:08X} (a table no walk read whole?)')


def control_words(d, f, blocks):
    """the control's literals that name RAM, I/O or ROM outside its blocks: BN6's own, the same"""
    inside = lambda v: any(lo <= (v & ~1) < hi for lo, hi in blocks)
    for w in f.walks.values():
        for v in w.lits.values():
            if v is not None and not inside(v) and 0x02000000 <= v < 0x0E000000 and v not in f.words:
                f.words[v] = (v, 'BN6\'s own', None)


def calls_of(r, d, blocks):
    """each BL out of its blocks: its twin, the dispatch's spawner for its game's spawner, or a stub"""
    f = r.f
    inside = lambda v: any(lo <= v < hi for lo, hi in blocks)
    outs = {}
    for w in f.walks.values():
        for site, t in w.bls.items():
            if not inside(t):
                outs.setdefault(t, []).append(site)
    r.calls = []
    for t, sites in sorted(outs.items(), key=lambda x: min(x[1])):
        if not r.control and t in NT.STUBS:
            r.calls.append(dict(bn5=t, bn6=None, spawner=NT.STUBS[t], source='a stub', cls=None, sites=sites))
        elif not r.control and t in BN5['spawners']:
            r.calls.append(dict(bn5=t, bn6=None, spawner=X.XV_SPAWN[(BN5['spawners'][t], t)], source='the dispatch', cls=None, sites=sites))
        else:
            tw = f.calls.get(t) or d.kind_of(f, t)[1] or (None, 'none', None, None)
            r.calls.append(dict(bn5=t, bn6=tw[0], spawner=None, source=tw[1], cls=tw[2], detail=tw[3], sites=sites))


def states_of(r, d):
    """its state table BN6's way: BN6's generic 0-7 (its own where it overrides one of 0-5), its own from 8"""
    f, out = r.f, []
    if r.control:
        r.states = [('BN6' if d.kind_of(f, s)[0] == 'engine' else 'OWN', s | 1) for s in f.states5]
        return
    for k in range(8):
        s = f.states5[k] & ~1 if k < 6 and k < len(f.states5) else None
        if s is not None and s not in NT.NAVI_GENERIC5[k]:
            kind, tw = d.kind_of(f, s)
            out.append(('BN6', tw[0] | 1) if kind == 'engine' else ('OWN', s | 1))
        elif s is not None:
            out.append(('BN6', NT.GENERIC_TWINS[s] | 1))
        else:
            out.append(('BN6', NT.NAVI_GENERIC6[k] | 1))
    for s in f.states5[6:]:
        kind, tw = d.kind_of(f, s)
        out.append(('BN6', tw[0] | 1) if kind == 'engine' else ('OWN', s | 1))
    r.states = out


def ref_of(d, f, a, empty5=None, empty6=None):
    """a hook's entry as BN6's table takes it: its own code, the empty one's BN6 twin, or an engine routine's twin"""
    a &= ~1
    if empty5 is not None and a == empty5:
        return ('BN6', empty6 | 1)
    kind, tw = d.kind_of(f, a)
    if kind == 'own' or (kind == 'foreign' and a in f.walks):
        return ('OWN', a | 1)
    if kind == 'engine' and tw[0]:
        return ('BN6', tw[0] | 1)
    f.decide('hook', f'{a:08X} has no twin in BN6')
    return ('BN6', a | 1)


def hooks_of(r, d, hooks, comp):
    """its hooks BN6's way: post, spawn start, spawn end, paralysed, frozen and bubbled (BN6's alone: its paralysis
    hook), flinching, dragged; its pre-step (BN6's of its kind), hook A (BN6's alone: the empty hook), its companion"""
    f, g = r.f, d.game
    if r.control:
        own = lambda a: ('OWN' if d.kind_of(f, a)[0] == 'own' else 'BN6', a | 1)
        r.hooks = [own(hooks[k]) for k in (POST, SPAWN, SPAWNED, PARALYSIS, FREEZE, BUBBLE, FLINCH, DRAG)]
        r.prestep, r.hook_a = own(hooks[PRESTEP]), own(hooks[HOOK_A])
        r.companion = [own(c) for c in comp]
        r.hide = own(d.b5.u32(HIDE6 + 4 * r.ai5))
        return
    k5 = {POST: POST, SPAWN: SPAWN, SPAWNED: SPAWNED, PARALYSIS: PARALYSIS, FREEZE: PARALYSIS, BUBBLE: PARALYSIS, FLINCH: FLINCH, DRAG: DRAG}
    r.hooks = [ref_of(d, f, hooks[k5[k]], g['nullsub'], NT.NULLSUB6) for k in (POST, SPAWN, SPAWNED, PARALYSIS, FREEZE, BUBBLE, FLINCH, DRAG)]
    pre = hooks[PRESTEP] & ~1
    if pre in NT.PRESTEPS:
        r.prestep = ('BN6', NT.PRESTEPS[pre] | 1)
    else:
        r.prestep = ref_of(d, f, pre)
        f.decide('pre-step', f'its pre-step {pre:08X} is none of BN5\'s four')
    r.hook_a = ('BN6', NT.NULLSUB6 | 1)
    r.companion = [ref_of(d, f, c, NT.EMPTY_COMPANION5, NT.EMPTY_COMPANION6) for c in comp]
    # (the routine its attacks hide its companion's overlay by: BN5's words at BN5_NAVI_HIDE by its AI index, BN6's twin)
    hide = d.b5.u32(HIDE5 + 4 * r.ai5) & ~1
    r.hide = ('BN6', HIDE_TWINS[hide] | 1) if hide in HIDE_TWINS else ref_of(d, f, hide)


def overlay_of(r, src):
    """its companion's overlay where its game's kind is one BN6 lacks (tools/bn5_navi_twins.py, OVERLAY5, COMPANIONS): the
    kind its copied start spawns (movs r4,#k before the overlay's spawner) made BN6's number for it, past BN6's own
    (src/emu/xnavi_recipe.h, XN_KIND_OF: the first past BN6's, and its slot); its masks and sprite (BN5's of that kind)
    copied by art_of"""
    f, start = r.f, r.companion_start
    if start not in NT.COMPANIONS or start not in f.walks:
        return
    w = f.walks[start]
    sites = [a for a, t in w.bls.items() if t == NT.OVERLAY5['spawner']]
    if not sites:
        return
    if len(sites) != 1:
        f.decide('overlay', f'its companion {start:08X} spawns more than one overlay')
        return
    # (its parameters in r4: movs r4,#kind, then the caller's r2 shifted up and added, BN5's companions' form)
    movs = [a for a in sorted(w.insns) if a < sites[0] and w.insns[a].form == 'movs' and w.insns[a].f[0] == 4]
    k = w.insns[movs[0]].f[1] if len(movs) == 1 else None
    if k is None or not 0 <= k < NT.OVERLAY5['kinds']:
        f.decide('overlay', f'its companion {start:08X} spawns an overlay of a kind made otherwise')
        return
    chain = [(movs[0], 'movs', k)]
    kind6 = NT.OVERLAY6['kinds'] + seat(r.ai5)
    masks = sorted(src.u32(NT.OVERLAY5['masks'] + 4 * j) for j in range(NT.OVERLAY5['kinds']))
    at = src.u32(NT.OVERLAY5['masks'] + 4 * k)
    end = next((m for m in masks if m > at), NT.OVERLAY5['masks'])
    lst = src.u8(NT.OVERLAY5['sprites'] + 2 * k)
    r.overlay = dict(kind5=k, kind6=kind6, masks=at, masks_len=end - at, crc=zlib.crc32(src.bytes(at, end - at)) & 0xFFFFFFFF,
                     list=lst // 4, index=src.u8(NT.OVERLAY5['sprites'] + 2 * k + 1))
    r.more_imms.append(dict(at=chain[-1][0], form='THUMB_MOVS', rd=4, rn=0, was=k, now=kind6,
                            why=f'its overlay\'s kind: {k} in BN5, {kind6} in BN6 (its own, past BN6\'s)'))


def chips_of(r, src, b6):
    """the chips its code writes for an attack (movs r0,#id before BN5's chip writer, whose twin returns BN6's action for a
    BN6 chip id): each BN6's chip of its name"""
    f = r.f
    if not f.chips:
        return
    bn6 = X.chip_names(b6, X.BN6['chip_names'], 411)
    bn5 = X.chip_names(src, BN5['chip_names'], 424)
    by_name = {}
    for i, name in bn6.items():
        by_name.setdefault(name, i)
    for a in f.chips:
        w = next(w for w in f.walks.values() if a in w.insns)
        v, chain = T.back_value(w, a, 0)
        if v is None and not r.held and any(i.form == 'ldrh' and i.f[1:] == (5, HELD_CHIP) for i in w.insns.values()):
            f.notes.append(f'{a:08X}: the chip it writes is the one it holds next (+{HELD_CHIP:#x}), BN5\'s held chips\' (the DS '
                           f'version\'s chip AI\'s); its versions hold none, the call skipped')
            continue
        name = bn5.get(v, '') if v is not None else ''
        to = by_name.get(name)
        if v is None or len(chain) != 1 or chain[0][1] != 'movs' or to is None or to > 0xFF:
            f.decide('chips', f'{a:08X}: the chip it writes ({v}, {name or "?"}) made by {chain}, BN6\'s {to}')
            continue
        if to != v:
            r.more_imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=v, now=to,
                                    why=f'the chip it writes for its attack: BN5\'s {v:#x}, BN6\'s {to:#x} ({name})'))


def held_of(r, src, game, control):
    """its held chips: the control's lists in BN6's ROM; BN5's (Team Colonel's six hold none), as BN6's by the chip
    registry where one holds any"""
    p = src.u32(game['held'] + 4 * r.ai5)
    if control:
        return p, None
    lists = [[src.u16(p + 0x20 * v + 2 * k) for k in range(16)] for v in range(r.versions)]
    if all(c == 0xFFFF for l in lists for c in l):
        return 0, None
    r.f.decide('held chips', f'its held chips at {p:08X} are not empty: map them through the chip registry')
    return 0, None


SPRITES6 = {}   # (BN6's sprites of a list by their bytes' hash, each length once)


def bn6_twin(b5, b6, lst, idx):
    """BN6's number in list `lst` of a sprite of the same bytes as BN5's lst:idx (each decompressed, compared whole), None"""
    p, b = X.sprite(b5, 0x0803272C, lst, idx)
    if not b:
        return None, 0
    if (lst, len(b)) not in SPRITES6:
        base, out = b6.u32(0x08031CC4 + 4 * lst), {}
        for k in range(256):
            q = b6.u32(base + 4 * k)
            if not 0x08000000 <= q & 0x7FFFFFFF < 0x08800000:
                break
            _, c = X.sprite(b6, 0x08031CC4, lst, k)
            if c:
                out.setdefault(hashlib.sha1(c).hexdigest(), k)
        SPRITES6[(lst, len(b))] = out
    return SPRITES6[(lst, len(b))].get(hashlib.sha1(b).hexdigest()), len(b)


def art_of(r, b5, b6, alloc):
    """what its attacks draw: the sprites its code loads by number (BN6's own where BN6 has the same bytes, else copied
    once into the shared art at a number of the grown lists in the Navis' range, whichever Navi draws it), its effects
    (BN6's twin where one draws the same, else a new id)"""
    f = r.f
    art = art_at(r.slot)[0]

    def place(lst, idx, room_kind):
        """BN5's sprite lst:idx at its number of a grown list (its own, or NAVI_LIST_MOVES' for one not grown) and its
        place in the shared art: (BN6's list, number), None where it cannot be"""
        to, x = alloc.shared(b5, lst, idx)
        if to is None:
            f.decide('sprite', f'its sprite {lst}:{idx:#x}: unread, of a list not grown, or past its range\'s numbers or room')
            return None
        if not any(s['list'] == lst and s['index'] == idx for s in r.sprites):
            r.sprites.append(dict(x))
        return to

    def bn6_place(lst, idx, room_kind):
        if lst == NAVI_LIST and idx == r.ai5 and r.ai5 in NAVI_SPRITES:
            return lst, NAVI_SPRITES[r.ai5]
        same, _ = bn6_twin(b5, b6, lst, idx)
        return (lst, same) if same is not None else place(lst, idx, room_kind)

    for lit, v in sorted(packed_sprites(r)):
        lst, idx = (v >> 8 & 0xFF) // 4, v & 0xFF
        to = bn6_place(lst, idx, 'load')
        if to is not None and to != (lst, idx):
            f.numbers[lit] = (v, v & ~0xFFFF | 4 * to[0] << 8 | to[1],
                              f'its sprite: list {lst} {idx:#x} in BN5, list {to[0]} {to[1]:#x} in BN6 (a packed word\'s number)')
    tables = sprite_tables(r, f)
    for (l, v), sites in sorted(f.loads.items(), key=lambda x: str(x[0])):
        if (l is None or v is None) and all(s in {site for _, _, site in tables} for s, _ in sites):
            continue
        if l is None or v is None:
            f.decide('sprite', f'a sprite loaded by a number made otherwise at {", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        lst = l // 4
        to = bn6_place(lst, v, 'load')
        if to is None:
            continue
        for s, chain in sites:
            why = f'its sprite: list {lst} {v:#x} in BN5, list {to[0]} {to[1]:#x} in BN6'
            if to[1] != v:
                X.imm_for(r, chain, 2, v, to[1], why)
            if to[0] != lst:
                X.imm_for(r, list_chain(f, s, chain), 1, l, 4 * to[0], why + ' (its list\'s offset)')
    for site, at, rd, e0, n in getattr(f, 'effect_ranges', []):
        # (n effects from e0, by a variable added to it: as many of BN6's in a row, each with its sprite, the add's
        # immediate the first)
        e6s = [alloc.number('effect', e0 + k) for k in range(n)]
        if [e for e, _ in e6s] != list(range(e6s[0][0], e6s[0][0] + n)):
            f.decide('effect', f'{site:08X}: its effects {e0:#x}-{e0 + n - 1:#x} take no {n} of BN6\'s in a row')
            continue
        for k, (e6, _) in enumerate(e6s):
            r.effects.append((e0 + k, e6))
            row = b5.bytes(BN5['effects'] + 4 * (e0 + k), 4)
            if (row[0] // 4, row[1]) not in [(x['list'], x['index']) for x in r.sprites]:
                place(row[0] // 4, row[1], 'effect')
        r.more_imms.append(dict(at=at, form='THUMB_ADDS', rd=rd, rn=0, was=e0, now=e6s[0][0],
                                why=f'its effects {e0:#x}-{e0 + n - 1:#x} in BN5, {e6s[0][0]:#x} on in BN6 (a variable added)'))
    for e, sites in sorted(f.effects.items(), key=lambda x: str(x[0])):
        if e is None:
            f.decide('effect', f'an effect id made otherwise at {", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        e6 = X.effect_twin(b5, b6, e, lambda lst, idx: bn6_twin(b5, b6, lst, idx))
        if e6 is None:
            row = b5.bytes(BN5['effects'] + 4 * e, 4)
            lst, idx = row[0] // 4, row[1]
            e6, _ = alloc.number('effect', e)
            if (e, e6) not in r.effects:
                r.effects.append((e, e6))
                if (lst, idx) not in [(s['list'], s['index']) for s in r.sprites]:
                    same, _ = bn6_twin(b5, b6, lst, idx)
                    if same is None:
                        place(lst, idx, 'effect')
                    else:
                        f.decide('effect', f'its effect {e:#x} draws BN6\'s sprite {lst}:{same:#x}: an entry naming BN6\'s wanted')
        if e6 != e:
            for s, chain in sites:
                X.imm_for(r, chain, getattr(f, 'effect_rd', {}).get(s, 4), e, e6, f'its effect: {e:#x} in BN5, {e6:#x} in BN6')
    for table, n, site in tables:
        for k in range(n):
            l, i = b5.u8(table + 2 * k), b5.u8(table + 2 * k + 1)
            to = bn6_place(l // 4, i, 'load')
            if to is None:
                continue
            why = f'{site:08X}: its sprite table\'s entry {k}'
            if 4 * to[0] != l:
                r.bytes.append(dict(at=table + 2 * k, was=l, now=4 * to[0], why=why))
            if to[1] != i:
                r.bytes.append(dict(at=table + 2 * k + 1, was=i, now=to[1], why=why))
    # (its overlay's masks last: the sprites before them each at their room's step)
    if r.overlay:
        o = r.overlay
        to = bn6_place(o['list'], o['index'], 'load')
        if to is None:
            f.decide('overlay', f'its overlay\'s sprite {o["list"]}:{o["index"]:#x} has no place')
        else:
            o['list6'], o['index6'] = to
            o['masks_at'] = art
            art = X.rounded(art + o['masks_len'], 4)
    if art > art_at(r.slot)[0] + XN_SLOT_ART:
        f.decide('room', f'its art ends at {art:08X}, past its slot\'s {XN_SLOT_ART:#x} bytes')


class Alloc(X.Alloc):
    """the numbers the Navis' art takes, in install order: a sprite one number and one place in the shared art whichever
    Navi draws it, an effect one number whichever Navi (or the moved objects) spawns it (each recipe lists them, its
    install writing them where none before it did)"""

    def __init__(self):
        super().__init__()
        self.lists = {k: list(v) for k, v in NAVI_LIST_NUMBERS.items()}
        # (the numbers tools/bn5_objects.py gives the moved objects' copies from the Navis' range, kept for them)
        for lst, num, track in OB.IMAGE_COPIED.values():
            if track == 'navis' and num in self.lists.get(lst, ()):
                self.lists[lst].remove(num)
        self.shared_at = XN_SHARED_ART
        self.effect = NAVI_FIRST_EFFECT

    def shared(self, b5, lst, idx, fixed=None):
        """BN5's sprite lst:idx in the shared art (src/emu/xnavi_recipe.h, XN_SHARED_ART), at a place and number of its
        own whichever Navi draws it (`fixed` its BN6 list and number where tools/bn5_objects.py gives them):
        ((BN6's list, number), its XvSprite), (None, None) where it cannot be"""
        key = ('shared', lst, idx)
        if key not in self.memo:
            p, b = X.sprite(b5, 0x0803272C, lst, idx)
            to_list = fixed[0] if fixed else lst if lst in self.lists else NAVI_LIST_MOVES.get(lst)
            if not b or to_list is None or (not fixed and not self.lists.get(to_list)):
                return None, None
            room = X.rounded(len(b), X.ROOM['load'])
            if self.shared_at + room > XN_END:
                return None, None
            self.memo[key] = dict(list=lst, index=idx, to_list=to_list, to_index=fixed[1] if fixed else self.lists[to_list].pop(0),
                                  at=self.shared_at, room=room, lz=bool(p & 0x80000000))
            self.shared_at += room
        x = self.memo[key]
        if fixed and (x['to_list'], x['to_index']) != tuple(fixed):
            return None, None
        return (x['to_list'], x['to_index']), x

    def take_effect(self):
        if self.effect > NAVI_LAST_EFFECT:
            sys.exit(f'xnavirecipe: the Navis\' effects past {NAVI_LAST_EFFECT:#x}, the end of their range')
        return X.Alloc.take_effect(self)


# ---- the C file ----

def ref_text(ref):
    return f'{ref[0]}(0x{ref[1]:08X})'


def addr_text(a):
    """an address the C file writes: a place in the moved objects' room by its name, else the number"""
    if XN_OBJECTS <= (a & ~1) < XN_CODE:
        return f'XN_OBJECTS + 0x{a - XN_OBJECTS:X}'
    return f'0x{a:08X}'


def c_code(r, n, base, base_name):
    """its code's arrays: blocks (at `base_name` + their distance from `base`), data, calls, words, immediates, bytes,
    numbers"""
    L = [f'static const XvBlock {n}_blocks[] = {{']
    for lo, hi, at, crc in r.blocks:
        L.append(f'\t{{ 0x{lo:08X}, 0x{hi:08X}, {base_name} + 0x{at - base:X}, 0x{crc:08X}u }},')
    L += ['};', f'static const XvData {n}_data[] = {{']
    for lo, hi, k in r.data:
        L.append(f'\t{{ 0x{lo:08X}, 0x{hi:08X}, XV_{k} }},')
    L += ['};', f'static const XvPair {n}_calls[] = {{']
    for c in r.calls:
        if c['spawner'] and c['source'] == 'a stub':
            L.append(f'\t{{ 0x{c["bn5"]:08X}, {c["spawner"]} }},   /* a stub: what BN6\'s engine lacks */')
        elif c['spawner']:
            L.append(f'\t{{ {X.BN5_SPAWN_NAME[c["bn5"]]}, {c["spawner"]} }},   /* the dispatch\'s spawner */')
        else:
            how = (c['cls'] or '-') + (' (audited)' if c.get('audit') else '')
            L.append(f'\t{{ 0x{c["bn5"]:08X}, {addr_text(c["bn6"] or 0)} }},   /* {c["source"]}, {how} */')
    L.append('};')
    if r.words:
        L.append(f'static const XvPair {n}_words[] = {{')
        for v, to in r.words:
            L.append(f'\t{{ 0x{v:08X}, {addr_text(to)} }},')
        L.append('};')
    if r.imms:
        L.append(f'static const XvImm {n}_imms[] = {{')
        for i in r.imms:
            L.append(f'\t{{ 0x{i["at"]:08X}, {i["form"]}, {i["rd"]}, {i["rn"]}, 0x{i["was"]:X}, 0x{i["now"]:X} }},   /* {i["why"]} */')
        L.append('};')
    if r.bytes:
        L.append(f'static const XvByte {n}_bytes[] = {{')
        for b in sorted(r.bytes, key=lambda b: b['at']):
            L.append(f'\t{{ 0x{b["at"]:08X}, 0x{b["was"]:02X}, 0x{b["now"]:02X} }},   /* {b["why"]} */')
        L.append('};')
    if r.numbers:
        L.append(f'static const XvNumber {n}_numbers[] = {{')
        for lit, was, now in r.numbers:
            L.append(f'\t{{ 0x{lit:08X}, 0x{was:08X}, 0x{now:08X} }},')
        L.append('};')
    return L


def c_art(r, n):
    """what it draws: its sprites (each at its place in the shared art), effects and object types"""
    L = []
    if r.sprites:
        L.append(f'static const XvSprite {n}_sprites[] = {{')
        for x in r.sprites:
            L.append(f'\t{{ {x["list"]}, 0x{x["index"]:02X}, {x["to_list"]}, 0x{x["to_index"]:02X}, XN_SHARED_ART + 0x{x["at"] - XN_SHARED_ART:X}, '
                     f'0x{x["room"]:X} }},' + ('   /* (LZ77 in BN5) */' if x['lz'] else ''))
        L.append('};')
    if r.effects:
        L.append(f'static const XvId {n}_effects[] = {{ ' + ', '.join(f'{{ 0x{a:02X}, 0x{b:02X} }}' for a, b in r.effects) + ' };')
    if r.types:
        L.append(f'static const XvType {n}_types[] = {{ ' + ', '.join(
            f'{{ {k}, 0x{t5:02X}, {"XV_TYPE_MOVED" if t6 is None else f"0x{t6:02X}"} }}' for k, t5, t6, _ in r.types) + ' };')
    return L


def c_fields(r, n, fields):
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    L = []
    for field, have in fields:
        if have:
            L.append(f'\t\t.{field} = {n}_{field}, .n{field} = {count(field)},')
    return L


def c_recipe(r, label):
    n = X.c_name(label, r.ai5)
    who = 'BN6\'s own, the control' if r.control else 'BN5\'s'
    ids = ', '.join(f'{x:#x}' for x in r.names[:r.versions])
    L = [f'/* ---- {label} ({who}: AI {r.ai5} there, its ids {ids}), as Navi AI {r.ai} with ids {r.first_id:#x}-'
         f'{r.first_id + r.versions - 1:#x} ---- */', '']
    L += c_code(r, n, *code_at(r.slot)[1:])
    L.append(f'static const XvRef {n}_states[] = {{')
    L.append('\t' + ', '.join(ref_text(x) for x in r.states) + ',')
    L.append('};')
    L += c_art(r, n)
    if getattr(r, 'xsounds', None):
        L.append(f'static const XnSound {n}_sounds[] = {{')
        for at, at2, v in r.xsounds:
            L.append(f'\t{{ 0x{at:08X}, 0x{at2:08X}, 0x{v:03X} }},   /* BN6\'s song {v:#x} another */')
        L.append('};')
    if r.overlay:
        o = r.overlay
        _, base, base_name = art_at(r.slot)
        L.append(f'static const XnOverlay {n}_overlay = {{ 0x{o["masks"]:08X}, {base_name} + 0x{o["masks_at"] - base:X}, '
                 f'0x{o["crc"]:08X}u, 0x{o["masks_len"]:X}, {o["kind6"]}, {o["list6"]}, 0x{o["index6"]:02X} }};   '
                 f'/* BN5\'s kind {o["kind5"]}, its sprite {o["list"]}:{o["index"]:#x} */')
    if r.needs:
        L.append(f'static const XvType {n}_needs[] = {{ ' + ', '.join(f'{{ {k}, 0x{t:02X}, XV_TYPE_MOVED }}' for k, t in r.needs) +
                 ' };   /* the moved objects\' (xnavi_objects.h) */')
    if getattr(r, 'families', None):
        L.append(f'static const XnFamily {n}_families[] = {{ ' + ', '.join(f'{{ 0x{b5:02X}, 0x{b6:02X}, OWN(0x{a:08X}) }}' for b5, b6, a in r.families) +
                 ' };   /* BN5\'s attack families it runs, moved with it (XN_FAMILY_OF) */')
    game = 'XV_GAME_BN6' if r.control else 'XROM_BN5_COLONEL_US'
    L += ['', f'{"const" if r.control else "static const"} XNavi {"xnavi_control" if r.control else "bn5_" + n} = {{', '\t.code = {',
          f'\t\t.name = "{label} ({who})",', f'\t\t.game = {game},', f'\t\t.src_ai = {r.ai5},', f'\t\t.ai = {r.ai},',
          f'\t\t.first_id = XN_FIRST_ID + 0x{r.first_id - XN_FIRST_ID:X},', f'\t\t.home = XN_HOMES + 0x{r.home - FREE_NAVI_HOMES:X},']
    L += c_fields(r, n, (('blocks', True), ('data', True), ('calls', True), ('words', r.words), ('imms', r.imms), ('numbers', r.numbers),
                         ('bytes', r.bytes), ('states', True)))
    L.append('\t\t.hooks = { ' + ', '.join(ref_text(x) for x in r.hooks) + ' },')
    L.append(f'\t\t.struct1 = 0x{r.s1:08X}, .struct2 = 0x{r.s2:08X},')
    L.append('\t\t.names = { ' + ', '.join(f'0x{x:03X}' for x in r.names) + ' },')
    if r.held:
        L.append(f'\t\t.held = 0x{r.held:08X},')
    L.append(f'\t\t.ice = {r.ice},')
    if r.traits:
        L.append(f'\t\t.traits = {{ .laid = true, .mode = 0x{r.traits["mode"]:02X}, .element = 0, .weakness = 0, '
                 f'.shadow = {"true" if r.traits["shadow"] else "false"} }},')
    L += c_fields(r, n, (('sprites', r.sprites), ('effects', r.effects), ('types', r.types)))
    L.append('\t},')
    L.append(f'\t.versions = {r.versions},')
    L.append(f'\t.prestep = {ref_text(r.prestep)}, .hook_a = {ref_text(r.hook_a)},')
    L.append(f'\t.companion = {ref_text(r.companion[0])}, .companion_end = {ref_text(r.companion[1])}, .hide = {ref_text(r.hide)},')
    L.append(f'\t.sprite_of = {r.sprite_of},')
    if r.overlay:
        L.append(f'\t.overlay = &{n}_overlay,')
    if any(r.shown):
        L.append('\t.shown = { ' + ', '.join(f'0x{x:03X}' for x in r.shown) + ' },')
    if getattr(r, 'xsounds', None):
        L.append(f'\t.sounds = {n}_sounds, .nsounds = (int)(sizeof {n}_sounds / sizeof *{n}_sounds),')
    if r.needs:
        L.append(f'\t.needs = {n}_needs, .nneeds = (int)(sizeof {n}_needs / sizeof *{n}_needs),')
    if getattr(r, 'families', None):
        L.append(f'\t.families = {n}_families, .nfamilies = (int)(sizeof {n}_families / sizeof *{n}_families),')
    L.append('};')
    return '\n'.join(L), None if r.control else f'&bn5_{n}'


def c_objects(r):
    """the moved objects' unit (tools/bn5_objects.py): its code and art as a family's recipe has them, no AI of its own"""
    n = 'objects'
    L = ['/* ---- BN5\'s engine objects moved once for every track (tools/bn5_objects.py), each module whole: its image object (the first '
         'kind\'s type 5; its kinds\' sprites as BN6 lists them where a kind is fixed, BN5\'s bytes where none installed spawns it) and '
         'its sea\'s marker (the fourth kind\'s 0x3E) ---- */',
         '']
    L += c_code(r, n, XN_OBJECTS, 'XN_OBJECTS')
    L += c_art(r, n)
    L += ['', 'const XVirusFamily xnavi_objects_bn5 = {', '\t.name = "BN5\'s moved objects",', '\t.game = XROM_BN5_COLONEL_US,',
          '\t.src_ai = -1,', '\t.ai = -1,', f'\t.home = XN_HOMES + 0x{XN_OBJECTS_HOME - FREE_NAVI_HOMES:X},']
    L += [x[1:] for x in c_fields(r, n, (('blocks', True), ('data', True), ('calls', True), ('words', r.words), ('imms', r.imms),
                                          ('numbers', r.numbers), ('bytes', r.bytes), ('sprites', r.sprites), ('effects', r.effects),
                                          ('types', r.types)))]
    L.append('};')
    return '\n'.join(L)


HEADER = '''/* xnavi_bn5.c: the Navis' recipes, written by tools/xnavirecipe.py from the two ROMs (python3 tools/xnavirecipe.py;
 * --check says whether this is what it writes). Do not edit: change the generator (or tools/bn5_navi_twins.py, its pairs
 * and audits) and write it again. Numbers alone (docs/MULTIROM.md, Transplanted Navis): addresses in BN5 Team Colonel
 * (USA) and BN6 Cybeast Gregar (USA), counts and the CRC32s of the ranges the install copies from the player's ROM;
 * nothing of either game's. The control (BN6's own HeatMan, derived by the same walk) and BN5's Navis proven against
 * BN5's own (tools/xdiff.py), each at its fixed numbers (src/emu/xnavi_recipe.h). */
#include "xnavi_bn5.h"

#include "bn5.h"
#include "bn6.h"
#include "rom.h"
#include "xvirus_dispatch.h"

#define OWN(a) { (a), true, 0 }
#define BN6(a) { (a), false, 0 }
'''


def c_file(recipes, objects):
    parts, refs = [HEADER, c_objects(objects), ''], []
    for r, label in recipes:
        text_, ref = c_recipe(r, label)
        parts.append(text_)
        if ref:
            refs.append(ref)
    parts.append('const XNavi *const xnavi_bn5[] = { ' + ', '.join(refs) + ' };\n'
                 'const int xnavi_bn5_count = (int)(sizeof xnavi_bn5 / sizeof *xnavi_bn5);\n')
    return '\n'.join(parts)


def report(recipes, path, objects):
    lines = ['# The Navis as tools/xnavirecipe.py derives them', '', '## The moved objects (tools/bn5_objects.py)', '',
             f'- code: {len(objects.blocks)} block, {sum(b - a for a, b, _, _ in objects.blocks)} bytes; calls {len(objects.calls)}; '
             f'words {len(objects.words)}; bytes {len(objects.bytes)}; sprites {len(objects.sprites)}; types {objects.types}',
             '- notes: ' + '; '.join(dict.fromkeys(objects.f.notes)),
             '- decisions: ' + ('none' if not objects.f.decisions else '; '.join(f'{k}: {d}' for k, d in objects.f.decisions)), '']
    for r, label in recipes:
        f = r.f
        cls = {}
        for c in r.calls:
            k = c['source'] if c['spawner'] else 'none' if c['bn6'] is None else (c['cls'] or '-') + ('+audit' if c.get('audit') else '')
            cls[k] = cls.get(k, 0) + 1
        lines.append(f'## AI {r.ai5}: {label} (slot {r.slot}: Navi AI {r.ai}, ids {r.first_id:#x}-{r.first_id + r.versions - 1:#x})')
        lines.append('')
        lines.append(f'- code: {len(r.blocks)} blocks, {sum(b - a for a, b, _, _ in r.blocks)} bytes; calls {len(r.calls)} '
                     f'({", ".join(f"{v} {k}" for k, v in sorted(cls.items()))}); immediates {len(r.imms)}; words {len(r.words)}')
        lines.append(f'- states {len(r.states)}: ' + ', '.join(f'{k} {a:08X}' for k, a in r.states))
        lines.append('- hooks: ' + ', '.join(f'{k} {a:08X}' for k, a in r.hooks) + f'; pre-step {r.prestep[1]:08X}')
        lines.append(f'- draws: {len(r.sprites)} sprites; effects {r.effects or "-"}; types {r.types or "-"}; sounds {r.sounds or "-"}; '
                     f'needs {r.needs or "-"}; image kinds {[hex(k) for k in r.kinds] or "-"}')
        if f.notes:
            lines.append('- notes: ' + '; '.join(dict.fromkeys(f.notes)))
        lines.append('- decisions: ' + ('none' if not f.decisions else ''))
        for k, d in f.decisions:
            lines.append(f'  - {k}: {d}')
        lines.append('')
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8') as o:
        o.write('\n'.join(lines) + '\n')


def pairs_with_navis(b5, b6):
    """the families' pairs and audits, and the Navis' beside them"""
    pairs = Pairs(b5, b6, X.MATCH, X.WORK, search=True)
    for a5, a6, src in NT.TWINS:
        pairs.twins[a5], pairs.twin_src[a5] = a6, src
    bn5_twins.AUDITS.update(NT.AUDITS)
    return pairs


def shared_rows(recipes, b5):
    """the rows of a type's own tables a later track spawns it with (tools/bn5_twins.py's SHARED_ROWS), fixed in each copy of
    them the Navis make: the one copy every track runs"""
    for at, was, now, why in bn5_twins.shared_row_bytes(b5, 'navis'):
        for r in recipes:
            if any(lo <= at < hi for lo, hi, _, _ in r.blocks) and not any(b['at'] == at for b in r.bytes):
                r.bytes.append(dict(at=at, was=was, now=now, why=why))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--check', action='store_true', help='write nothing: fail where the C file is not what this writes')
    ap.add_argument('--report', action='store_true', help='every Navi of Team Colonel\'s and the Dark Soul: .build/xnavirecipe/report.md')
    ap.add_argument('--ai', type=int, nargs='*', help='the report\'s Navis (BN5 AI indexes)')
    a = ap.parse_args()
    b5, b6 = X.find_roms()
    pairs = pairs_with_navis(b5, b6)
    alloc = Alloc()
    recipes = [(derive(b6, b6, pairs, CONTROL[0], CONTROL_SLOT, alloc, control=True), CONTROL[1])]
    for ai, label in NAVIS:
        recipes.append((derive(b5, b6, pairs, ai, slot_of(ai), alloc), label))
    objects = derive_objects(b5, b6, pairs, alloc)
    shared_rows([r for r, _ in recipes[1:]] + [objects], b5)
    if a.report:
        done = {r.ai5 for r, _ in recipes[1:]}
        more = [(derive(b5, b6, pairs, ai, slot_of(ai), alloc), f'AI {ai}') for ai in (a.ai or MOVED) if ai not in done]
        report(recipes + more, os.path.join(WORK, 'report.md'), objects)
        print(f'xnavirecipe: {os.path.join(WORK, "report.md")}')
        return 0
    for r, label in recipes + [(objects, 'the moved objects')]:
        if r.f.decisions:
            sys.exit(f'xnavirecipe: {label} (AI {r.ai5}) has decisions open: ' + '; '.join(f'{k}: {d}' for k, d in r.f.decisions))
    text_ = c_file(recipes, objects)
    if a.check:
        with open(OUT_C, encoding='utf-8') as f:
            same = f.read() == text_
        print(f'xnavirecipe: {os.path.relpath(OUT_C, ROOT)} {"is" if same else "is NOT"} what the generator writes')
        return 0 if same else 1
    with open(OUT_C, 'w', encoding='utf-8') as f:
        f.write(text_)
    print(f'xnavirecipe: {", ".join(label for _, label in recipes)} written into {os.path.relpath(OUT_C, ROOT)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
