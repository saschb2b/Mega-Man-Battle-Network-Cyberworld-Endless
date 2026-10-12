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

import bn5_twins
import xrecipe_thumb as T

BUS = T.BUS
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# ---- each game's tables (src/emu/bn5.h, bn6.h and bn6_families.h name them; docs/ROM_DATA.md says how each was found) ----

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
    byname=0x0800CE54,                               # an enemy by its NameID range (r1, r2)
    rng=(0x08001490, 0x080014A6, 0x080014C0, 0x080014D6),   # BN5_RNG_DRAW*
    chip_writer=(0x0800FE6A, 0x0800FE70, 0x0800FE76),       # a chip's numbers written for an attack (BN6 0x080126E4)
    set_panel=0x0800B2A6,                            # object_setPanelType: r2 the type
    chip_names=(0x08736084, 0x08736AB8),             # BN5_CHIP_NAMES_LOW, _HIGH
    shared=(0x08104AC0, 0x08104E88), objects=(0x080B0000, 0x08104AC0), families=(0x08104E88, 0x08114000),
)
BN6 = dict(
    sprites=0x08031CC4, songs=0x08159F48, effects=0x080E16D4, neffects=108,
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
        self.last = False

    def decide(self, kind, detail):
        if (kind, detail) not in self.decisions:
            self.decisions.append((kind, detail))


class Deriver:
    def __init__(self, b5, b6, pairs):
        self.b5, self.b6, self.pairs = b5, b6, pairs
        self.s1s = sorted(set(self.table(ai, 1) for ai in range(BN5['nfam'])))

    def table(self, ai, k):
        return self.b5.u32(BN5['fam'] + BN5['step'] * k + 4 * ai)

    # ---- what a target is ----

    def kind_of(self, f, t):
        """a call's or pointer's target: ('own',), ('engine', twin), ('foreign', twin) (copied: no twin, in BN5's shared
        virus code, its attacks' or another family's), ('missing', twin)"""
        t &= ~1
        if f.region[0] <= t < f.region[1]:
            return 'own', None
        tw = self.pairs.twin(t)
        if tw[0]:
            return 'engine', tw
        if any(lo <= t < hi for lo, hi in (BN5['shared'], BN5['objects'], BN5['families'])):
            return 'foreign', tw
        return 'missing', tw

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
                if not T.is_code_ptr(x, self.b5) or abs((x & ~1) - v) > 0x4000:
                    break
                ents.append(x)
            if ents:
                out[v] = ents
        return out

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
                    kind, tw = self.kind_of(f, x)
                    if kind == 'engine':
                        f.words[x] = (tw[0] | 1, 'a jump table entry', tw)
                    else:
                        todo.append((x & ~1, ctx))
            for lit, v in w.lits.items():
                if v is None or v in w.tables or not T.is_code_ptr(v, self.b5):
                    continue
                kind, tw = self.kind_of(f, v)
                if kind == 'engine':
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
        if key not in f.types:
            f.types[key] = self.type_twin(kind, v)
        if f.types[key]['bn6'] is None and f.types[key]['routine']:
            todo.append((f.types[key]['routine'], 'object'))

    def type_twin(self, kind, t5):
        tab5, n5 = BN5['types'][kind]
        if t5 >= n5:
            return dict(bn6=None, routine=None, why=f'type {t5:#x} past BN5\'s {n5}')
        r5 = self.b5.u32(tab5 + 4 * t5)
        tab6, n6 = BN6['types'][kind]
        for t6 in [t5] + [t for t in range(n6) if t != t5]:
            if t6 >= n6:
                continue
            r6 = self.b6.u32(tab6 + 4 * t6)
            if r6 and T.is_code_ptr(r5, self.b5) and T.is_code_ptr(r6, self.b6) and self.pairs.deep_same(r5 & ~1, r6 & ~1):
                return dict(bn6=t6, routine=r5 & ~1, why=f'BN6\'s type {t6:#x} runs the same code')
        return dict(bn6=None, routine=r5 & ~1 if T.is_code_ptr(r5, self.b5) else None, why='moved: no BN6 type runs the same code')

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
        for v in sorted(targets):
            if v in cov:
                continue
            for k in range(0x80):
                if k and v + k in cov:
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
        return blocks

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
            a = lo
            while a < hi:
                k = kinds.get(a, 'RAW')
                b = a + 2
                while b < hi and kinds.get(b, 'RAW') == k and not (k == 'RAW' and b in ptrs) and not (k == 'TABLE' and b in tables):
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
                if virus and i.form == 'strb' and i.f[1:] == (5, 9):
                    v, chain = T.back_value(w, a, i.f[0])
                    if v is not None and 6 <= v <= 0xF and chain and chain[-1][1] == 'movs' and len(chain) == 1:
                        f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=i.f[0], rn=0, was=v, now=v + 2, why=f'CurAction {v} written'))
                    elif v is None or v >= 6:
                        f.decide('curaction', f'{a:08X}: CurAction written from {chain}')
                if virus and i.form == 'ldrb' and i.f[1:] == (5, 9):
                    for b in addrs[k + 1:k + 4]:
                        j = w.insns[b]
                        if j.form == 'cmp' and j.f[0] == i.f[0]:
                            if 6 <= j.f[1] <= 0xF:
                                f.imms.append(dict(at=b, form='THUMB_CMP', rd=j.f[0], rn=0, was=j.f[1], now=j.f[1] + 2,
                                                   why=f'CurAction {j.f[1]} compared'))
                            break
                if i.form in ('strb', 'strh', 'str') and i.f[1] == 5 and i.f[2] == 0x17:
                    f.flip.append(a)
                if i.form == 'ldrb' and i.f[1] == 5 and i.f[2] == 1:
                    f.decide('type read', f'{a:08X}: its object\'s type read (ldrb r{i.f[0]},[r5,#1]), which the dispatch keeps apart')
            self.scan_collision(f, w)

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
        elif t == BN5['byname']:
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
                    f.numbers[lit] = (v, v, 'a number (tested, masked, ORed or a spawn\'s parameters): kept')
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
