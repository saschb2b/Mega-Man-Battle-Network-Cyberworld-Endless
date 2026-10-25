"""xrecipe_family.py: one BN5 virus family derived from the two ROMs for tools/xrecipe.py (issue #82): its code (every
routine its states and hooks reach, the objects it spawns that BN6 has no twin of, the routines it calls that BN6 has no
twin of) in blocks, each block's bytes classed (code, literal pool, jump table, its dead state table, raw data), its
calls out paired with BN6's twins, its literals and jump tables' entries that name BN5's routines or memory, the
immediates BN6 numbers otherwise (CurAction, effects, sprites, sounds, collision data offsets, held chips), its states and
hooks in BN6's layout, its records, what it draws and spawns, and apart what still needs a decision. Numbers only."""
import json
import os
import re
import struct

import bn5_objects as OB
import bn5_twins
import xrecipe_thumb as T

BUS = T.BUS
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# ---- each game's tables (src/emu/bn5.h, bn6.h and bn6_families.h name them; docs/rom/ says how each was found) ----

BN5 = dict(
    fam=0x081045D4, step=0x8C, nfam=35,              # BN5_FAMILY_TABLES, BN5_FAMILY_STEP, BN5_FAMILIES
    nullsub=0x08104B1A,                              # the per-family tables' empty hook (mov pc,lr)
    ids=0x08014C8C, nids=0x191,                      # BN5_ENEMY_IDS: (version, kind, AI) an id
    names=0x0873C0B4,                                # BN5_ENEMY_NAMES
    held=0x08113BE0,                                 # the held chips' triple (BN6_HELD_CHIPS_REF's twin): its virus word
    sprites=0x0803272C,                              # XRomLayout.sprite_lists
    songs=0x08155BF4,                                # XRomLayout.song_table
    effects=0x080DFD7C, neffects=108,                # BN5_EFFECTS, BN5_EFFECT_COUNT
    types={1: (0x08003958, 95), 3: (0x08003B80, 204), 4: (0x08003F60, 145)},   # BN5_T*_TYPES
    spawners={0x08003140: 1, 0x0800315C: 1, 0x080031E8: 1, 0x08003178: 3, 0x08003194: 3, 0x080031B0: 3, 0x080031CC: 4},  # BN5_SPAWN_*
    effect=0x080DFFD2,                               # BN5_SPAWN_EFFECT: r4's low byte the effect
    sound=0x08000598,                                # BN5_PLAY_SOUND: r0 the song
    sprite_load=0x08002650,                          # BN5_SPRITE_LOAD: r1 the list's offset, r2 the number
    set_attack=0x0800F200,                           # setAttack0-6 (BN5_SET_ATTACK - 0x1C): 4 bytes apart, r0 the CurAction
    set_flag=0x08016C42,                             # object_setFlag (BN6 0x0801A152): r0 the flags
    byname=(0x0800CE54, 0x08104B1C),                 # an enemy by its NameID range (r1, r2), and the shared virus code's count of
                                                     # those with a word nonzero (r0 its offset), which hands r1 and r2 on to it
    rng=(0x08001490, 0x080014A6, 0x080014C0, 0x080014D6),   # BN5_RNG_DRAW*
    chip_writer=(0x0800FE6A, 0x0800FE70, 0x0800FE76),       # a chip's numbers written for an attack (BN6 0x080126E4)
    chip_query=0x0800D010,                           # the chip an object uses: r0 a virus's held chip (its +0x2A), r3 its +0x1A
    obj_chip=0x2A,                                   # an object's chip it uses next (u16; BN6_T1_CHIP the same place)
    set_panel=0x0800B2A6,                            # object_setPanelType: r2 the type
    obstacle_states=0x08017FCC,                      # an obstacle's state run from its state table (r0; BN6 0x0801B394)
    chip_names=(0x08736084, 0x08736AB8),             # BN5_CHIP_NAMES_LOW, _HIGH
    shared=(0x08104AC0, 0x08104E88), objects=(0x080B0000, 0x08104AC0), families=(0x08104E88, 0x08114000),
)
BN6 = dict(
    sprites=0x08031CC4, songs=0x08159F48, effects=0x080E16D4, neffects=108, sprite_load=0x080026E4,
    types={1: (0x08003C9C, 95), 3: (0x08003EC4, 213), 4: (0x080042C8, 146)},
    nullsub=0x0810A953, chip_names=(0x086E88D0, 0x086E92D8),
)
# The generic states: BN5's 0-5, BN6's 0-7 (6 frozen, 7 bubbled: BN6's alone); each BN5 family's state table names them
GENERIC5 = {0: 0x080131A0, 1: 0x080133C4, 2: 0x080133CE, 3: 0x080134A0, 4: 0x08013824, 5: 0x08013944}
GENERIC6 = {0: 0x08016380, 1: 0x080165B8, 2: 0x080165C2, 3: 0x080166AE, 4: 0x08016B02, 5: 0x08016CE8, 6: 0x08016B36, 7: 0x08016B72}
# BN5's tables 3-8 (each frame, spawning, spawned, paralysed, flinching, dragged) as BN6's 3-10: BN6's 7 and 8 (frozen,
# bubbled) take its paralysis hook, as BN6's AI 17 gives one hook all four
HOOKS6 = {3: 3, 4: 4, 5: 5, 6: 6, 7: 6, 8: 6, 9: 7, 10: 8}
HELD_LISTS = 0xC0                  # six lists of 16 u16 a family's records end with (struct1 + 0x2C)
CODE_FROM = 0x8 + 0x24 + 0xC0      # its code after struct1, struct2 and those lists


def coll_map():
    """BN5's collision data offsets as BN6's (tests/xdiff/legit.json, layouts, collision: BN5's, BN6's, bytes)"""
    with open(os.path.join(ROOT, 'tests', 'xdiff', 'legit.json'), encoding='utf-8') as f:
        return json.load(f)['layouts']['collision']['segments']


SEGMENTS = coll_map()


def coll_to6(off):
    for a, b, n in SEGMENTS:
        if a <= off < a + n:
            return b + off - a
    return None


class Family:
    """A BN5 family as derived; `decisions` what the generator cannot decide, each (kind, detail)"""

    def __init__(self, ai):
        self.ai = ai
        self.walks, self.ctx, self.calls, self.words, self.types = {}, {}, {}, {}, {}
        self.decisions, self.notes = [], []
        self.imms, self.numbers, self.bytes_ = [], {}, []
        self.sounds, self.effects, self.loads = {}, {}, {}
        self.rng, self.chips, self.nameranges, self.flags_set, self.flip, self.attack6, self.panel_types = [], [], [], [], [], [], []
        self.chip_cmps = []   # (the compare's address, its register, BN5's chip id, the routine's entry) of a held chip's id
        self.chip_stores = []   # (the movs's address, its register, BN5's chip id) of a chip id its code gives its object (+0x2A)
        self.effect_bytes = set()   # its AI data's bytes that hold an effect id (the shared teleport's)
        self.helper_types = {}      # (kind, type) -> the engine routine whose twin spawns it as BN6's own (its call aimed there)
        self.needs = set()          # (kind, type) of the moved objects its code calls into (tools/bn5_objects.py)
        self.last = False

    def decide(self, kind, detail):
        if (kind, detail) not in self.decisions:
            self.decisions.append((kind, detail))


class Deriver:
    def __init__(self, b5, b6, pairs, moved_before=None, art_same=None):
        self.b5, self.b6, self.pairs = b5, b6, pairs
        self.s1s = sorted(set(self.table(ai, 1) for ai in range(BN5['nfam'])))
        # (a type of its game a family before it in install order moved: (kind, type) -> that family's name; its routine is
        # that family's copy, which the dispatch's MOVED entry names, so this one spawns it and copies none of it)
        self.moved_before = moved_before if moved_before is not None else {}
        self._spawns = {}
        # (whether BN5's sprite (list offset, number) and BN6's draw the same: xrecipe.py's, by their bytes or what they draw)
        self.art_same = art_same
        # (a number its code keeps must be one bn5_twins.KEPT reads in BN6: the families' rule; the Navis' generator keeps its
        # numbers unaudited, its Navis proven frame for frame by tools/xdiff.py instead)
        self.audit_numbers = True

    join_pool_gaps = True   # (blocks_of: a routine's run and its pool's joined across other code, the bytes between zeroed)

    def table(self, ai, k):
        return self.b5.u32(BN5['fam'] + BN5['step'] * k + 4 * ai)

    # ---- what a target is ----

    def kind_of(self, f, t, pointer=False):
        """a call's or pointer's target: ('own',), ('engine', twin), ('foreign', twin) (copied: no twin, in BN5's shared
        virus code, its attacks' or another family's), ('missing', twin). A pointer (a jump table's entry, a callback) to a
        routine whose twin's body differs and no audit takes is copied too, where it can be: what it names is run by the
        engine as it stands, never checked against what the family's code asks of it as a call's twin is. One that would
        be copied but is BN5's engine objects' moved once for every track (tools/bn5_objects.py) is reached at their copy,
        installed before any family, which runs their type for this family's spawns too"""
        t &= ~1
        if f.region[0] <= t < f.region[1]:
            return 'own', None
        kind, tw, note = self.target_of(f, t, pointer)
        m = OB.module_of(t) if kind == 'foreign' else None
        if m is not None:
            f.needs.add((m['kind'], m['type']))
            return 'engine', (OB.objects_at(t), 'the moved objects', 'SAME', None)
        if note:
            f.notes.append(note)
        return kind, tw

    def target_of(self, f, t, pointer):
        """kind_of's classes but the moved objects': (class, twin, the note a copy leaves or None)"""
        tw = self.pairs.twin(t)
        copyable = any(lo <= t < hi for lo, hi in (BN5['shared'], BN5['objects'], BN5['families']))
        audit = self.pairs.audit(t, tw[0]) if tw[0] else None
        if audit and audit[0] == 'copy':
            # (a twin whose fix or change the family's code shows: BN5's own body served, copied with its code)
            return 'foreign', tw, f'copies {t:08X}: its audit serves BN5\'s own body ({audit[1]})'
        if tw[0] and pointer and tw[2] == 'DIFF' and self.pairs.audit(t, tw[0]) is None and copyable:
            return 'foreign', tw, f'copies {t:08X}, which a pointer of its code names: BN6\'s twin {tw[0]:08X} differs ({tw[3]}) and no audit takes it'
        bad = self.spawns_other(t) if tw[0] else None
        if bad and copyable:
            return 'foreign', tw, (f'copies {t:08X}: BN6\'s twin {tw[0]:08X} spawns the {kind_name(bad[0])} kind\'s {bad[1]:#x}, which BN6 runs '
                                   f'otherwise (its code or what it draws): BN5\'s spawns it through the dispatch')
        if tw[0]:
            # (the types its twin spawns as BN6's own, each a decision of bn5_twins.TYPES: twin)
            for key in self.spawned_types(t):
                f.helper_types.setdefault(key, t)
            return 'engine', tw, None
        if copyable:
            return 'foreign', tw, None
        return 'missing', tw, None

    # ---- the walk ----

    def find_tables(self, f, w):
        """the jump tables a routine's literals name: Thumb pointers near it, each near the table"""
        out = {}
        for lit, v in w.lits.items():
            if v is None or v & 3 or not self.b5.has(v, 4):
                continue
            if not (w.insns and min(w.insns) - 0x40 <= v <= max(w.insns) + 0x400) and not f.region[0] <= v < f.region[1]:
                continue
            ents = []
            for k in range(48):
                x = self.b5.u32(v + 4 * k)
                # (an entry near the table, or one of the family's own, BN5's shared virus code's or an engine routine BN6 has a
                # twin of: a jump table of virus code names no other)
                if not T.is_code_ptr(x, self.b5) or (abs((x & ~1) - v) > 0x4000 and not self.known_code(f, x)):
                    break
                ents.append(x)
            if ents:
                out[v] = ents
        return out

    def type_same(self, kind, v5, v6, code=True):
        """BN6's type v6 of the kind runs what BN5's v5 does: the same code (Pairs.deep_same; `code` false: an engine
        routine's spawn, whose twin's code was compared as the routine's), not a type whose table by its kind BN6 laid out
        otherwise (bn5_twins.MOVE_TYPES), and the same art (type_sprites)"""
        tab5, n5 = BN5['types'][kind]
        tab6, n6 = BN6['types'][kind]
        r5 = self.b5.u32(tab5 + 4 * v5) if v5 < n5 else 0
        r6 = self.b6.u32(tab6 + 4 * v6) if v6 < n6 else 0
        if not (r6 and T.is_code_ptr(r5, self.b5) and T.is_code_ptr(r6, self.b6)):
            return False
        if code and not self.pairs.deep_same(r5 & ~1, r6 & ~1):
            return False
        if (kind, v5) in bn5_twins.MOVE_TYPES:
            return False
        rec = bn5_twins.TYPES.get((kind, v5))
        if rec is not None and rec[0] == 'moved':
            return False   # (one decision a type for every track: moved where any track's code needs BN5's, a merge's moved winning)
        if not self.art_same:
            return True
        s5 = self.type_sprites(self.b5, r5 & ~1, BN5['sprite_load'])
        s6 = self.type_sprites(self.b6, r6 & ~1, BN6['sprite_load'])
        return len(s5) == len(s6) and all(self.art_same(a, b) for a, b in zip(s5, s6))

    def type_sprites(self, rom, routine, load, depth=2):
        """the sprites an object type's code loads by constant numbers (list, number), in its walk's order, its jump
        tables' routines after it (to `depth`)"""
        w = T.walk(rom, routine)
        out = []
        for site, s in sorted(w.bls.items()):
            if s == load:
                l, _ = T.back_value(w, site, 1)
                v, _ = T.back_value(w, site, 2)
                out.append((l, v))
        if depth:
            for lit, v in sorted(w.lits.items()):
                if v is None or v & 3 or not rom.has(v, 4):
                    continue
                for k in range(16):
                    x = rom.u32(v + 4 * k)
                    if not T.is_code_ptr(x, rom) or abs((x & ~1) - v) > 0x4000:
                        break
                    out += self.type_sprites(rom, x & ~1, load, depth - 1)
        return out

    def spawned_by(self, t):
        """an engine routine's own spawn of a constant type: (kind, type), else (None, None)"""
        if not self.b5.has(t, 2):
            return None, None
        w = T.walk(self.b5, t)
        for site, s in sorted(w.bls.items()):
            if s in BN5['spawners']:
                v, _ = T.back_value(w, site, 0)
                if v is not None and v <= 0xFF:
                    return BN5['spawners'][s], v
        return None, None

    def runs_states(self, kind, t5):
        """an object type of BN5's whose code runs a state table by the engine's obstacle runner (BN5 0x08017FCC, BN6's
        0x0801B394): its +9 a state numbered as a virus's (BN6's two status states before its own)"""
        tab5, n5 = BN5['types'][kind]
        r5 = self.b5.u32(tab5 + 4 * t5) if t5 < n5 else 0
        if not T.is_code_ptr(r5, self.b5):
            return False
        todo, seen = [(r5 & ~1, 3)], set()
        while todo:
            a, depth = todo.pop()
            if a in seen:
                continue
            seen.add(a)
            w = T.walk(self.b5, a)
            if BN5['obstacle_states'] in w.bls.values():
                return True
            if depth:
                todo += [(t, depth - 1) for t in w.bls.values() if self.b5.has(t, 2) and t >= BN5['objects'][0]]
                for v in w.lits.values():
                    if v is not None and not v & 3 and self.b5.has(v, 4):
                        for k in range(16):
                            x = self.b5.u32(v + 4 * k)
                            if not T.is_code_ptr(x, self.b5) or abs((x & ~1) - v) > 0x4000:
                                break
                            todo.append((x & ~1, depth - 1))
        return False

    def spawns_other(self, t):
        """an engine routine of BN5's that spawns an object of a type (a constant, by one of its spawners) whose BN6 type of
        that number draws other sprites (its code's constant sprite numbers, each pair's art: Deriver.art_same): (kind,
        type) of the first, else None. Its twin in BN6 spawns BN6's own of that number, as a family's call to it would"""
        if t in self._spawns:
            return self._spawns[t]
        self._spawns[t] = None
        w = T.walk(self.b5, t)
        for site, s in sorted(w.bls.items()):
            if s not in BN5['spawners']:
                continue
            kind = BN5['spawners'][s]
            v, _ = T.back_value(w, site, 0)
            if v is None or v > 0xFF:
                continue
            rec = bn5_twins.TYPES.get((kind, v))
            if rec is not None and rec[0] == 'unreachable':
                continue   # (a spawn no battle of BN6's reaches: BN6's twin left to it)
            if not self.type_same(kind, v, v, code=False) or (rec is not None and rec[0] == 'moved'):
                self._spawns[t] = (kind, v)
                break
        return self._spawns[t]

    def spawned_types(self, t):
        """the types (kind, type) an engine routine of BN5's spawns by a constant through one of its spawners"""
        w = T.walk(self.b5, t)
        out = []
        for site, s in sorted(w.bls.items()):
            if s in BN5['spawners']:
                v, _ = T.back_value(w, site, 0)
                if v is not None and v <= 0xFF:
                    out.append((BN5['spawners'][s], v))
        return out

    def type_decisions(self, f):
        """each type its objects come to be against its decision in bn5_twins.TYPES (moved, else BN6's twin of the same number:
        the same code and the same sprites by their bytes): one not recorded, or recorded otherwise, is a decision"""
        got = {}
        for key, v in f.types.items():
            got[key] = ('moved', None) if v['bn6'] is None else ('twin', v['bn6'])
        for key, t in f.helper_types.items():
            rec = bn5_twins.TYPES.get(key)
            got.setdefault(key, ('unreachable', None) if rec is not None and rec[0] == 'unreachable' else ('twin', key[1]))
        for (kind, t5), (how, t6) in sorted(got.items()):
            rec = bn5_twins.TYPES.get((kind, t5))
            if rec is None or rec[0] != how or (how == 'twin' and rec[1] != t6):
                f.decide('type', f'the {kind_name(kind)} kind\'s {t5:#x}: derived {how}{f" as {t6:#x}" if t6 is not None else ""}, '
                                 f'recorded {rec[:2] if rec else "nowhere"} (bn5_twins.TYPES)')

    def kind_decisions(self, f):
        """each kind of a table's type its code spawns (bn5_twins.ID_TABLES' `kinds_of`, and the moved image object's,
        tools/bn5_objects.py), named by the register of a call to the type's spawner, is one the table fixes (its `live`):
        another keeps BN5's sprite bytes, and is refused"""
        image = dict(at=OB.IMAGE['kinds'], kinds_of=(OB.IMAGE['kind'], OB.IMAGE['type']), spawner=OB.IMAGE['spawner'], kind_reg=4,
                     live=tuple(sorted(set(OB.IMAGE_TWIN) | set(OB.IMAGE_COPIED))), where='tools/bn5_objects.py')
        for t in list(bn5_twins.ID_TABLES) + [image]:
            if 'kinds_of' not in t:
                continue
            for w in f.walks.values():
                for site, target in w.bls.items():
                    if target != t['spawner']:
                        continue
                    k, chain = T.back_value(w, site, t['kind_reg'])
                    if k is None and chain and chain[-1][1] == 'lsls':
                        # (lsls rK, rX, #8 or more, then adds: the low byte the adds' alone, the high byte another's)
                        a = chain[-1][0]
                        if w.insns[a].f[2] >= 8:
                            k = sum(x[2] for x in chain if x[1] == 'adds8')
                    if k is None or (k & 0xFF) not in t['live']:
                        f.decide('kind', f'{site:08X}: the {kind_name(t["kinds_of"][0])} kind\'s {t["kinds_of"][1]:#x} of '
                                         f'kind {"made otherwise" if k is None else hex(k & 0xFF)} ({chain}), not one its '
                                         f'table {t["at"]:08X} fixes ({t.get("where", "bn5_twins.ID_TABLES")})')

    def pointer_twin_ok(self, f, x, tw):
        """a pointer's twin of a body that differs serves only where an audit takes it (else it would have been copied)"""
        if tw[2] == 'DIFF' and self.pairs.audit(x & ~1, tw[0]) is None:
            f.decide('pair', f'{x & ~1:08X} -> {tw[0]:08X} ({tw[1]}): a pointer\'s twin whose body differs ({tw[3]}), which no audit '
                             f'takes and which cannot be copied')

    def known_code(self, f, x):
        """a Thumb pointer to the family's own code, BN5's shared virus code or an engine routine BN6 has a twin of"""
        a = x & ~1
        return f.region[0] <= a < f.region[1] or BN5['shared'][0] <= a < BN5['shared'][1] or bool(self.pairs.twin(a)[0])

    def walk_family(self, f, extra=()):
        todo = [(e, 'virus') for e in extra]
        for k, s in enumerate(f.states5):
            if k < 6 and (s & ~1) == GENERIC5.get(k):
                continue
            kind, tw = self.kind_of(f, s)
            if kind == 'engine':
                continue
            todo.append((s & ~1, 'virus'))
        for h in f.hooks5:
            if (h & ~1) == BN5['nullsub']:
                continue
            kind, tw = self.kind_of(f, h)
            if kind != 'engine':
                todo.append((h & ~1, 'virus'))
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
            for tab, ents in w.tables.items():
                for x in ents:
                    kind, tw = self.kind_of(f, x, pointer=True)
                    if kind == 'engine':
                        self.pointer_twin_ok(f, x, tw)
                        f.words[x] = (tw[0] | 1, 'a jump table entry', tw)
                    else:
                        todo.append((x & ~1, ctx))
            for lit, v in w.lits.items():
                if v is None or v in w.tables or not T.is_code_ptr(v, self.b5):
                    continue
                kind, tw = self.kind_of(f, v, pointer=True)
                if kind == 'engine':
                    self.pointer_twin_ok(f, v, tw)
                    f.words[v] = (tw[0] | 1, 'a callback', tw)
                else:
                    todo.append((v & ~1, ctx))
            for site, t in w.bls.items():
                if t in BN5['spawners']:
                    self.spawn(f, w, site, t, todo)
                    continue
                kind, tw = self.kind_of(f, t)
                if kind in ('own', 'foreign'):
                    todo.append((t, ctx))
                    if kind == 'foreign':
                        f.notes.append(f'copies {t:08X} (called at {site:08X}): no BN6 twin, outside the family')
                elif kind == 'engine':
                    f.calls[t] = tw
                else:
                    f.calls[t] = tw
                    f.decide('call', f'{t:08X} (called at {site:08X}) has no twin in BN6 ({tw[1] if tw else "none"})')
        # contexts on to the routines each calls
        changed = True
        while changed:
            changed = False
            for e, w in f.walks.items():
                kids = [t for t in w.bls.values() if t in f.walks] + [x & ~1 for ents in w.tables.values() for x in ents if (x & ~1) in f.walks]
                for ctx in list(f.ctx.get(e, ())):
                    for k in kids:
                        if ctx not in f.ctx.setdefault(k, set()):
                            f.ctx[k].add(ctx)
                            changed = True

    def spawn(self, f, w, site, spawner, todo):
        kind = BN5['spawners'][spawner]
        v, chain = T.back_value(w, site, 0)
        f.calls[spawner] = (None, 'the dispatch', None, None)
        if v is None or v > 0xFF:
            f.decide('spawn', f'{site:08X}: an object of the {kind_name(kind)} kind whose type is made otherwise ({chain})')
            return
        key = (kind, v)
        rec = bn5_twins.TYPES.get(key)
        if rec is not None and rec[0] == 'unreachable':
            # (a spawn no battle of BN6's reaches: through the dispatch, whose MOVED entry for a type no family names is the
            # stray's, which lets the object go as it first runs)
            f.notes.append(f'{site:08X}: the {kind_name(kind)} kind\'s {v:#x}, unreachable in BN6\'s engine (bn5_twins.TYPES): the '
                           f'dispatch\'s stray')
            return
        if key not in f.types:
            f.types[key] = self.type_twin(kind, v)
            if f.types[key]['bn6'] is None and key in self.moved_before:
                f.types[key] = dict(bn6=None, routine=None, shared=self.moved_before[key],
                                    why=f'moved by {self.moved_before[key]} before it: its copy run for this one too')
        if f.types[key]['bn6'] is None and f.types[key]['routine']:
            todo.append((f.types[key]['routine'], 'object'))

    def type_twin(self, kind, t5):
        tab5, n5 = BN5['types'][kind]
        if t5 >= n5:
            return dict(bn6=None, routine=None, why=f'type {t5:#x} past BN5\'s {n5}')
        r5 = self.b5.u32(tab5 + 4 * t5)
        tab6, n6 = BN6['types'][kind]
        for t6 in [t5] + [t for t in range(n6) if t != t5]:
            if t6 < n6 and self.type_same(kind, t5, t6):
                return dict(bn6=t6, routine=r5 & ~1, why=f'BN6\'s type {t6:#x} runs the same code on the same data and art')
        return dict(bn6=None, routine=r5 & ~1 if T.is_code_ptr(r5, self.b5) else None,
                    why='moved: no BN6 type runs the same code on the same data and art')

    # ---- blocks ----

    def covered(self, f):
        cov, targets = {}, set()
        for e, w in f.walks.items():
            for a, i in w.insns.items():
                for k in range(i.size):
                    cov[a + k] = 'code'
            for lit, v in w.lits.items():
                for k in range(4):
                    cov.setdefault(lit + k, 'POOL')
                if v is not None and not v & 1 and self.b5.has(v) and v not in w.tables and (abs(v - e) < 0x800 or f.region[0] <= v < f.region[1]):
                    targets.add(v)
            for tab, ents in w.tables.items():
                for k in range(4 * len(ents)):
                    cov[tab + k] = 'TABLE'
        tables = {t["at"]: t["size"] for t in bn5_twins.ID_TABLES}
        for v in sorted(targets):
            if v in cov:
                continue
            for k in range(tables.get(v, 0x80)):
                if k and v + k in cov and v not in tables:
                    break
                cov[v + k] = 'RAW'
        return cov

    def blocks_of(self, f):
        cov = self.covered(f)
        blocks = []
        lo, hi = f.s1 + CODE_FROM, f.region[1]
        inside = [a for a in cov if f.region[0] <= a < f.region[1]]
        if f.last and inside:
            # (the last family's code ends where its walks do: no family's records follow it)
            hi = (max(max(inside) + 1, f.st + 4 * len(f.states5)) + 3) & ~3
        if inside:
            if min(inside) < lo:
                f.decide('block', f'code at {min(inside):08X} before its records end ({lo:08X})')
                lo = min(inside) & ~1
            blocks.append((lo, hi))
        run, runs = None, []
        for a in sorted(a for a in cov if not f.region[0] <= a < f.region[1]):
            if run and a <= run[1] + 4:
                run[1] = a + 1
            else:
                if run:
                    runs.append([run[0], (run[1] + 1) & ~1])
                run = [a, a + 1]
        if run:
            runs.append([run[0], (run[1] + 1) & ~1])
        # (a routine whose literal pool lies past other code of its game, which no walk reached: one block from its code to
        # its pool, the bytes between dead in the copy, zeroed; a deriver that joins such runs its own way leaves them apart:
        # the chips', tools/xchiprecipe.py, its gaps' routines walked as its own)
        f.gaps = getattr(f, 'gaps', [])
        merged = self.join_pool_gaps
        while merged:
            merged = False
            for w in f.walks.values():
                for lit in w.lits:
                    at = next((k for k, r in enumerate(runs) if r[0] <= lit < r[1]), None)
                    for a in w.insns:
                        k = next((k for k, r in enumerate(runs) if r[0] <= a < r[1]), None)
                        if at is not None and k is not None and k != at:
                            lo, hi = sorted((k, at))
                            if runs[hi][0] - runs[lo][1] <= 0x400:
                                f.gaps.append((runs[lo][1], runs[hi][0]))
                                runs[lo:hi + 1] = [[runs[lo][0], runs[hi][1]]]
                                merged = True
                                break
                    if merged:
                        break
                if merged:
                    break
        return blocks + [tuple(r) for r in runs]

    def dead_roots(self, f, blocks):
        """routines in its main block nothing names (push {..., lr}), walked as if live so their calls and pools hold"""
        cov, roots = self.covered(f), []
        if not blocks or not f.region[0] <= blocks[0][0] < f.region[1]:
            return roots
        lo, hi = blocks[0]
        st = (f.st, f.st + 4 * len(f.states5))
        a = lo
        while a < hi:
            if a not in cov and not st[0] <= a < st[1] and self.b5.u16(a) & 0xFF00 == 0xB500:
                roots.append(a)
                while a < hi and a not in cov:
                    a += 2
                continue
            a += 2
        return roots

    def classify(self, f, blocks):
        """each block's bytes that are no code: literal pools, jump tables, its state table (zeroed), raw data (split
        where a literal names a place in it)"""
        code, pools, tables, ptrs = set(), set(), {}, set()
        for w in f.walks.values():
            for a, i in w.insns.items():
                code.add(a)
                if i.size == 4:
                    code.add(a + 2)
            for lit, v in w.lits.items():
                pools.add(lit)
                if v is not None and self.b5.has(v) and not v & 1:
                    ptrs.add(v)
            ptrs |= w.adrs
            tables.update(w.tables)
        st = (f.st, f.st + 4 * len(f.states5))
        # (a word of raw data that points into a block is a pointer kept in a table of its own, never read by a literal load:
        # a data pointer moved with its block as a pool's is (its target the start of a range), a Thumb one to its code as a
        # jump table's entry is)
        in_blocks = lambda v: any(lo <= v < hi for lo, hi in blocks)
        for lo, hi in blocks:
            for a in range((lo + 3) & ~3, hi - 3, 4):
                if a in code or a in pools or any(t <= a < t + 4 * len(e) for t, e in tables.items()) or st[0] <= a < st[1]:
                    continue
                v = self.b5.u32(a)
                if v & 1 and in_blocks(v & ~1) and (v & ~1) in code:
                    tables[a] = [v]
                elif in_blocks(v & ~1) and (v & ~1) not in code and v != a:
                    # (a byte table's place may be odd: a pointer to raw data, not to code)
                    pools.add(a)
                    if not v & 1:
                        ptrs.add(v)
                    f.notes.append(f'{a:08X}: a pointer to its data ({v:08X}) in a table of its own: moved with it')
        data = []
        for lo, hi in blocks:
            kinds = {}
            for a in range(lo, hi, 2):
                if a in code:
                    kinds[a] = 'code'
            for p in pools:
                if lo <= p < hi:
                    kinds[p] = kinds[p + 2] = 'POOL'
            for t, ents in tables.items():
                if lo <= t < hi:
                    for k in range(len(ents)):
                        kinds[t + 4 * k] = kinds[t + 4 * k + 2] = 'TABLE'
            for a in range(max(lo, st[0]), min(hi, st[1]), 2):
                kinds[a] = 'ZERO'
            for g0, g1 in getattr(f, 'gaps', []):
                for a in range(max(lo, g0), min(hi, g1), 2):
                    if a not in kinds:
                        kinds[a] = 'ZERO'
            a = lo
            while a < hi:
                k = kinds.get(a, 'RAW')
                b = a + 2
                while b < hi and kinds.get(b, 'RAW') == k and not (k in ('RAW', 'POOL') and b in ptrs) and not (k == 'TABLE' and b in tables):
                    b += 2
                if k != 'code':
                    data.append((a, b, k))
                a = b
        for a, b, k in data:
            if k == 'RAW' and b - a >= 8:
                bls = [x for x in range(a, b - 2, 2) if self.b5.u16(x) >> 11 == 0x1E and self.b5.u16(x + 2) >> 11 == 0x1F]
                if bls and not any(a <= p < b for p in ptrs):
                    f.decide('unreached', f'{a:08X}-{b:08X}: {b - a} bytes no walk reached, with {len(bls)} BL-like pairs')
        return data

    # ---- what its code asks of BN6 ----

    def scan(self, f):
        for e, w in f.walks.items():
            virus = 'virus' in f.ctx.get(e, ())
            addrs = sorted(w.insns)
            for k, a in enumerate(addrs):
                i = w.insns[a]
                if i.form == 'bl':
                    self.scan_call(f, w, a, i.target, virus)
                    if i.target == BN5['chip_query']:
                        self.chip_compares(f, e, w, addrs[k + 1:])
                if virus and i.form == 'strb' and i.f[1:] == (5, 9):
                    v, chain = T.back_value(w, a, i.f[0])
                    if v is not None and 6 <= v <= 0xF and chain and chain[-1][1] == 'movs' and len(chain) == 1:
                        f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=i.f[0], rn=0, was=v, now=v + 2, why=f'CurAction {v} written'))
                    elif v is None or v >= 6:
                        f.decide('curaction', f'{a:08X}: CurAction written from {chain}')
                if virus and i.form == 'ldrb' and i.f[1:] == (5, 9):
                    # (each compare of the CurAction read while its register holds it, past the conditional branches
                    # between them: cmp #2; beq; cmp #9; beq compares two)
                    for b in addrs[k + 1:k + 9]:
                        j = w.insns[b]
                        if j.form == 'cmp' and j.f[0] == i.f[0]:
                            if '#' in j.text and 6 <= j.f[1] <= 0xF:   # (an immediate's, not cmp rd, rs)
                                f.imms.append(dict(at=b, form='THUMB_CMP', rd=j.f[0], rn=0, was=j.f[1], now=j.f[1] + 2,
                                                   why=f'CurAction {j.f[1]} compared'))
                        elif j.form in ('b', 'bx', 'bl', 'hi_mov') or T.is_return(j) or T.writes(j, i.f[0]):
                            break
                if i.form in ('strb', 'strh', 'str') and i.f[1] == 5 and i.f[2] == 0x17:
                    f.flip.append(a)
                if i.form == 'strh' and i.f[1] == 5 and i.f[2] == BN5['obj_chip']:
                    self.chip_store(f, w, a, i.f[0])
                if virus and i.form == 'strb' and i.f[1] == 7 and i.f[2] == 3:
                    self.teleport_effects(f, w, addrs, k)
                if i.form == 'ldrb' and i.f[1] == 5 and i.f[2] == 1:
                    f.decide('type read', f'{a:08X}: its object\'s type read (ldrb r{i.f[0]},[r5,#1]), which the dispatch keeps apart')
            self.scan_collision(f, w)

    def chip_store(self, f, w, a, reg):
        """a chip id its code gives its object (+0x2A, the chip it uses next: BN5's engine sets it from the held chips as
        the object spawns, and Zomon's set-up writes its AreaGrab there by hand, BN5 0x08108932-0x08108934): a constant's
        movs, for BN6's id in its place (chip_ids); none (0xFFFF, movs #0 then subs #1: Appley's; or a literal's) kept"""
        v, chain = T.back_value(w, a, reg)
        if v is not None and len(chain) == 1 and chain[0][1] == 'movs':
            f.chip_stores.append((chain[0][0], reg, v))
            return
        writes = [w.insns[x] for x in sorted((x for x in w.insns if x < a), reverse=True)[:6] if T.writes(w.insns[x], reg)][:2]
        if [(x.form, x.f) for x in writes] == [('subs8', (reg, 1)), ('movs', (reg, 0))]:
            return
        if v == 0xFFFF and len(chain) == 1 and chain[0][1] == 'ldr_lit':
            return   # (none from a literal: ShadowMan's, BN5 0x080F9406)
        f.decide('chip', f'{a:08X}: a chip id given its object from {chain}')

    def teleport_effects(self, f, w, addrs, k):
        """the effects BN5's shared teleport spawns (0x08104DEA: its attack's variables +0x0C where it leaves, read at
        0x08104E2C, +0x0D where it lands, at 0x08104E0A, where their +3 holds bit 2): a constant store to +3 with that bit,
        then constant stores to +0x0C and +0x0D close after, each an effect id its code names"""
        v3, _ = T.back_value(w, addrs[k], w.insns[addrs[k]].f[0])
        if v3 is None or not v3 & 2:
            return
        for b in addrs[k + 1:k + 13]:
            j = w.insns[b]
            if j.form == 'strb' and j.f[1] == 7 and j.f[2] in (0x0C, 0x0D):
                v, chain = T.back_value(w, b, j.f[0])
                if v is not None and v < BN5['neffects'] and len(chain) == 1 and chain[0][1] == 'movs':
                    f.effects.setdefault(v, []).append((b, chain, j.f[0]))
                    f.effect_bytes.add(0xA0 + j.f[2])   # (its AI data's byte: the attack's variables are its +0xA0)

    def chip_compares(self, f, entry, w, after):
        """a held chip's id (the query's r0, its low 9 bits by lsls #0x17 then lsrs #0x17) compared with a number before
        the chip writer takes it: each compare (its address, register, BN5's id), for BN6's id in its place"""
        held, shifted, ids = {0}, set(), set()
        for b in after[:32]:
            i = w.insns[b]
            if i.form == 'bl':
                if i.target in BN5['chip_writer']:
                    break
                held -= {0, 1, 2, 3}
                ids -= {0, 1, 2, 3}
                shifted -= {0, 1, 2, 3}
                continue
            if i.form in ('b', 'bx') or T.is_return(i):
                break
            if i.form == 'lsls' and i.f[2] == 0x17 and i.f[1] in held | ids:
                shifted.add(i.f[0])
                ids.discard(i.f[0])
                held.discard(i.f[0])
                continue
            if i.form == 'lsrs' and i.f[2] == 0x17 and i.f[1] in shifted and i.f[0] == i.f[1]:
                shifted.discard(i.f[0])
                ids.add(i.f[0])
                continue
            if i.form == 'adds3' and i.f[2] == 0 and i.f[1] in ids:   # (mov rd,rs as adds rd,rs,#0)
                ids.add(i.f[0])
                continue
            if i.form == 'cmp' and i.f[0] in ids:
                f.chip_cmps.append((b, i.f[0], i.f[1], entry))
                continue
            for r in range(8):
                if T.writes(i, r):
                    held.discard(r)
                    ids.discard(r)
                    shifted.discard(r)

    def scan_call(self, f, w, a, t, virus):
        sa = BN5['set_attack']
        if sa <= t < sa + 7 * 4 and not (t - sa) & 3:
            n = (t - sa) // 4
            v, chain = T.back_value(w, a, 0)
            if n == 6:
                f.attack6.append(a)
                f.decide('setattack6', f'{a:08X}: setAttack6 (an attack of kind 6, which BN6 lacks)')
            if not virus:
                return
            before = w.insns.get(a - 4)
            if v is None and before is not None and before.form == 'bl' and before.target in BN5['chip_writer']:
                f.notes.append(f'{a:08X}: setAttack{n} with the chip\'s action the chip writer gives (BN6\'s, for BN6\'s chip ids)')
            elif v is None:
                f.decide('curaction', f'{a:08X}: setAttack{n} with a CurAction made otherwise')
            elif 6 <= v <= 0xF:
                if len(chain) == 1 and chain[0][1] == 'movs':
                    f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=v, now=v + 2,
                                       why=f'CurAction {v} (setAttack{n}): BN6\'s two status states before the family\'s own'))
                else:
                    f.decide('curaction', f'{a:08X}: setAttack{n} with {v} made by {chain}')
            elif v >= 0x10:
                f.decide('curaction', f'{a:08X}: setAttack{n} with the chip action {v:#x}')
        elif t in BN5['chip_writer']:
            f.chips.append(a)
        elif t == BN5['sound']:
            v, chain = T.back_value(w, a, 0)
            f.sounds.setdefault(v, []).append((a, chain))
        elif t == BN5['effect']:
            v, chain = T.back_value(w, a, 4)
            f.effects.setdefault(None if v is None else v & 0xFF, []).append((a, chain))
        elif t == BN5['sprite_load']:
            l, _ = T.back_value(w, a, 1)
            v, chain = T.back_value(w, a, 2)
            f.loads.setdefault((l, v), []).append((a, chain))
        elif t in BN5['byname']:
            lo, _ = T.back_value(w, a, 1)
            hi, _ = T.back_value(w, a, 2)
            f.nameranges.append((a, lo, hi))
        elif t in BN5['rng']:
            f.rng.append(a)
        elif t == BN5['set_flag']:
            v, _ = T.back_value(w, a, 0)
            f.flags_set.append((a, v))
        elif t == BN5['set_panel']:
            v, _ = T.back_value(w, a, 2)
            f.panel_types.append((a, v))

    def scan_collision(self, f, w):
        """loads and stores through a register loaded from an object's +0x54 (its collision data): BN5's offsets as BN6's"""
        forms = {'ldr': 'THUMB_LDR', 'str': 'THUMB_STR', 'ldrb': 'THUMB_LDRB', 'strb': 'THUMB_STRB', 'ldrh': 'THUMB_LDRH', 'strh': 'THUMB_STRH'}
        coll = set()
        for a in sorted(w.insns):
            i = w.insns[a]
            if i.form == 'ldr' and i.f[2] == 0x54:
                coll.add(i.f[0])
                continue
            if i.form in forms and i.f[1] in coll:
                off = i.f[2]
                to = coll_to6(off)
                if to is None:
                    f.decide('collision', f'{a:08X}: its collision data\'s +{off:#x}, which BN6 has no place for')
                elif to != off:
                    f.imms.append(dict(at=a, form=forms[i.form], rd=i.f[0], rn=i.f[1], was=off, now=to,
                                       why=f'collision data +{off:#x} in BN5, +{to:#x} in BN6'))
            if i.form == 'bl':
                coll -= {0, 1, 2, 3}
            elif T.writes(i, i.f[0] if i.f else -1) and i.f and i.f[0] in coll and i.form not in forms:
                coll.discard(i.f[0])
            elif i.form in ('ldr', 'ldrb', 'ldrh') and i.f[0] in coll:
                coll.discard(i.f[0])

    # ---- literals: RAM, I/O and ROM outside its blocks ----

    def scan_words(self, f, blocks):
        inside = lambda v: any(lo <= (v & ~1) < hi for lo, hi in blocks)
        for e, w in f.walks.items():
            for lit, v in w.lits.items():
                if v is None or inside(v) or not 0x02000000 <= v < 0x0E000000 or T.is_code_ptr(v, self.b5) or v in f.words:
                    continue
                use = literal_use(w, lit)
                if use == 'number':
                    if v not in bn5_twins.KEPT and self.audit_numbers:
                        f.decide('number', f'{v:08X} at {lit:08X}: a number its code keeps (tested, masked, ORed or a spawn\'s '
                                           f'parameters) that no audit reads in BN6 (bn5_twins.KEPT)')
                    f.numbers[lit] = (v, v, 'a number (tested, masked, ORed or a spawn\'s parameters): kept, ' +
                                      bn5_twins.KEPT.get(v, 'not audited'))
                    continue
                kind = 'RAM' if v < 0x04000000 else 'I/O' if v < 0x05000000 else 'video' if v < BUS else 'ROM'
                to = None
                if kind == 'ROM':
                    to, why = self.rom_twin(v)
                elif kind == 'I/O':
                    to, why = v, 'the same register'
                else:
                    why = 'no twin known'
                if to is None:
                    f.decide('literal', f'{v:08X} ({kind}) at {lit:08X}: {why}')
                else:
                    f.words[v] = (to, f'{kind}: {why}', None)

    def rom_twin(self, v):
        """BN6's twin of a table of BN5's: the project's (bn5_twins.DATA_TWINS); the same bytes once in BN6 (64 at most);
        or a table of pointers whose targets' bytes BN6 keeps, moved alike"""
        for lo, n, to, why in bn5_twins.DATA_TWINS:
            if lo <= v < lo + n:
                return to + (v - lo), why
        blob = self.b5.bytes(v, 64)
        for n in (64, 32, 16):
            hits, i = [], self.b6.data.find(blob[:n])
            while i >= 0 and len(hits) < 8:
                hits.append(BUS + i)
                i = self.b6.data.find(blob[:n], i + 1)
            if len(hits) == 1:
                return hits[0], f'the same {n} bytes, once in BN6'
            if hits:
                exp = self.pairs.expected(v)
                best = sorted(hits, key=lambda h: abs(h - exp)) if exp else []
                if best and abs(best[0] - exp) < 0x1000 and abs(best[1] - exp) > abs(best[0] - exp):
                    return best[0], f'the same {n} bytes, {len(hits)} times in BN6: where its neighbours put it'
                break
        ptrs = []
        for k in range(32):
            x = self.b5.u32(v + 4 * k)
            if not BUS <= x < BUS + 0x800000:
                break
            ptrs.append(x)
        if len(ptrs) >= 2:
            lo = min(ptrs)
            i = self.b6.data.find(self.b5.bytes(lo, 24))
            if i >= 0:
                d = BUS + i - lo
                j = self.b6.data.find(b''.join(((x + d) & 0xFFFFFFFF).to_bytes(4, 'little') for x in ptrs))
                if j >= 0:
                    return BUS + j, f'a table of {len(ptrs)} pointers whose targets BN6 keeps {d:#x} on'
        return None, 'no twin found by its bytes'


def literal_use(w, lit):
    """how the register a literal is loaded into is used next: 'address' (loaded or stored through), 'number' (tested,
    masked, ORed, compared, multiplied, or handed on as a spawn's words), None"""
    for a in sorted(w.insns):
        i = w.insns[a]
        if i.lit != lit:
            continue
        r = i.f[0]
        for b in sorted(x for x in w.insns if x > a)[:8]:
            j = w.insns[b]
            if j.form in ('ldr', 'str', 'ldrb', 'strb', 'ldrh', 'strh') and j.f[1] == r or j.form.endswith('_r') and r in j.f[1:]:
                return 'address'
            if j.form in ('tst', 'ands', 'orrs', 'bics', 'eors', 'cmp', 'cmn', 'muls') and r in j.f:
                return 'number'
            if j.form == 'bl':
                return 'number' if r in (1, 2, 3, 4) else None
            if j.f and j.f[0] == r and T.writes(j, r):
                break
    return None


def kind_name(kind):
    return {1: 'first', 3: 'third', 4: 'fourth'}[kind]
