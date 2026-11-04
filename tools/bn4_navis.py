#!/usr/bin/env python3
"""bn4_navis.py [NAVI ...]: what moving BN4 Red Sun's Navis into BN6 Gregar's battle engine takes, Navi by Navi
(docs/multirom/bn4-navis.md; docs/rom/bn4.md, BN4's Navis), read from the two ROMs.

A BN4 Navi is a first-kind object type of its own (BN4_T1_TYPES, by its enemy record's +2), as BN4's virus families
are: its routine's jump table (init, update, dying), its update's pre-step and the Navi state runner with its state
table, BN4's six generic Navi states first and its own from 6. Ten of the eighteen take their side's AI data at their
init and run MegaMan's own states as theirs (BN4_PLAYER_STATES: their attacks shared with MegaMan's table); the other
eight keep their variables in the object and run only their own states. For each this walks every routine its states
reach (its block, and MegaMan's where its table names his states), pairs each call out with BN6's twin (the project's
pairs, tools/bn4_twins.py; the symbol map's, .build/symbols/match/bn6f-bn4.csv beside bn6f-gregar.csv, by bn6f's
names, which stay there; else a BN6 routine of the same body) and classes the pair by the two bodies (SAME, LAYOUT,
DIFF, none), finds the objects it spawns and whether BN6 has a type of the same bodies (init and update), and the
AI data's and the object's fields its code reads and writes.

  python3 tools/bn4_navis.py              .build/bn4-navis/report.md: each Navi, then the order by cost and the engine
                                          routines to port once
  python3 tools/bn4_navis.py GutsMan      those Navis alone

Local only: both ROMs (CYBERWORLD_ROM_DIR, ~/.cache/mmbn-ref/roms). Nothing of either is written: addresses, counts
and the classes of pairs."""
import collections
import csv
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_twins  # noqa: E402
import xrecipe_thumb as T  # noqa: E402
from xrecipe_bn4 import Pairs4, find_roms  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, '.build', 'bn4-navis')
MATCH = os.path.join(ROOT, '.build', 'symbols', 'match')

# ---- BN4's tables (src/emu/bn4.h; docs/rom/bn4.md) ----
T1, T3, T4 = 0x08003A24, 0x08003C40, 0x08003F78   # BN4_T1_TYPES, BN4_T3_TYPES, BN4_T4_TYPES
NTYPES = {1: 95, 3: 165, 4: 130}
STATS, STAT_SIZE = 0x08011888, 12                 # BN4_ENEMY_STATS
PLAYER, PLAYER_STATES = 0x080EAEA8, 0x080EAEFC    # BN4_MEGAMAN, BN4_PLAYER_STATES
PLAYER_END = 0x080F6000                           # (past his last state's code, 0x080F5025)
NAVI_GENERIC = (0x08010D31, 0x08010FA7, 0x08010D4D, 0x08010E89, 0x08010F09, 0x08010FAF)   # BN4_NAVI_GENERIC_0 to _5
TAKE_AI = 0x0800C09A                              # BN4_AI_TAKE
SPAWN = {0x080032B8: 1, 0x080032D4: 1, 0x080032F0: 3, 0x0800330C: 3, 0x08003328: 3, 0x08003344: 4}   # BN4_SPAWN_1 ...
AI_NAVI, AI_ATTACK = 0x64, 0x70                   # BN4_AI_NAVI, BN4_AI_ATTACK_VARS

# BN6's: the object types' tables (BN6's first, third and fourth kinds), for the objects' twins
T6 = {1: (0x08003C9C, 95), 3: (0x08003EC4, 213), 4: (0x080042C8, 146)}

# The eighteen neither BN5 Team Colonel nor BN6 Gregar has (docs/multirom/bn4.md): (name, type, first enemy id), its
# four ids from there (V1, V2, V3, SP)
NAVIS = (('Roll', 0x1D, 128), ('GutsMan', 0x0A, 132), ('WindMan', 0x30, 136), ('SearchMan', 0x2B, 140),
         ('FireMan', 0x0C, 144), ('ThunderMan', 0x32, 148), ('MetalMan', 0x3F, 160), ('JunkMan', 0x47, 164),
         ('AquaMan', 0x0D, 168), ('WoodMan', 0x46, 172), ('TopMan', 0x09, 176), ('BurnerMan', 0x22, 184),
         ('ColdMan', 0x4A, 188), ('SparkMan', 0x0B, 192), ('LaserMan', 0x29, 208), ('KendoMan', 0x2D, 212),
         ('VideoMan', 0x2F, 216), ('Duo', 0x2A, 224))
# The routines a Navi's moved copy no longer calls: BN6's Navi tables stand in for its runner, pre-steps, generic
# states, dying, collision set-up and sprite update (docs/multirom/navis.md), its AI data comes from BN6's pool
REPLACED = {0x08010D30, 0x08010FA6, 0x08010D4C, 0x08010E88, 0x08010F08, 0x08010FAE, 0x08013DB4, 0x080135A8, 0x0801374A,
            0x08010F60, 0x0801052C, 0x08012496, 0x080129AC, 0x08012D62, 0x080143FC, TAKE_AI}


class Pairs(Pairs4):
    """Pairs4 with the symbol map's BN4 table: a routine bn6f's search located in BN4 paired with Gregar's of its name"""

    def __init__(self, b4, b6):
        super().__init__(b4, b6)
        self.by4 = {}
        path = os.path.join(MATCH, 'bn6f-bn4.csv')
        if os.path.exists(path):
            with open(path, newline='', encoding='utf-8') as f:
                for r in csv.DictReader(f):
                    if r['kind'] == 'function' and r['address']:
                        self.by4[int(r['address'], 16) & ~1] = (r['name'], r['confidence'])

    def map_twin(self, a5):
        r = self.by4.get(a5)
        if not r:
            return None, None
        return self.by6name.get(r[0].split('+')[0]), r[1]


class Map:
    def __init__(self):
        self.b4, self.b6 = find_roms()
        self.pairs = Pairs(self.b4, self.b6)
        self.types = {k: [self.b4.u32(t + 4 * i) for i in range(NTYPES[k])] for k, t in ((1, T1), (3, T3), (4, T4))}
        self.starts = sorted({x & ~1 for k in self.types.values() for x in k if T.is_code_ptr(x, self.b4)} | {PLAYER})
        self.pstates = []
        while True:
            x = self.b4.u32(PLAYER_STATES + 4 * len(self.pstates))
            if not (x & 1 and T.is_code_ptr(x, self.b4)):
                break
            self.pstates.append(x)
        self.memo = {}

    def block(self, a):
        """the object type's block an address is in (its routine to the next type's), or None for the engine's"""
        a &= ~1
        if a < self.starts[0] or a >= PLAYER_END:
            return None
        k = max(i for i, s in enumerate(self.starts) if s <= a)
        lo = self.starts[k]
        hi = self.starts[k + 1] if k + 1 < len(self.starts) else PLAYER_END
        return (lo, hi) if lo == PLAYER or a - lo < 0x1800 else None

    def type_of(self, a):
        b = self.block(a)
        if not b:
            return 'engine'
        if b[0] == PLAYER:
            return 'MegaMan'
        for k, tab in self.types.items():
            for i, x in enumerate(tab):
                if x & ~1 == b[0]:
                    return f'T{k}:{i:#04x}'
        return '?'

    def twin(self, a):
        """(BN6's routine, how found, its class: SAME, LAYOUT, DIFF or None)"""
        a &= ~1
        if a not in self.memo:
            a6, how, cls, _ = self.pairs.twin(a)
            self.memo[a] = (a6, how, cls)
        return self.memo[a]

    def table_at(self, v, stop):
        ents = []
        for k in range(64):
            x = self.b4.u32(v + 4 * k)
            if not T.is_code_ptr(x, self.b4) or x & ~1 >= 0x08200000 or v + 4 * k in stop:
                break
            ents.append(x)
        return ents

    def walk(self, ty):
        """a Navi's routines from its type's: its jump table, state table, own states and MegaMan's it names, their
        jump tables; its calls out and the pointers its literals and tables give out"""
        b4 = self.b4
        rt = self.types[1][ty] & ~1
        lo, hi = self.block(rt)
        w0 = T.walk(b4, rt)
        jt = next(v for v in w0.lits.values() if v and not v & 3 and lo <= v < hi)
        init, update, dying = (b4.u32(jt + 4 * k) for k in range(3))
        wu = T.walk(b4, update)
        stab = next(v for v in wu.lits.values() if v and not v & 3 and lo <= v < hi)
        states = []
        while len(states) < 64:
            x = b4.u32(stab + 4 * len(states))
            if not T.is_code_ptr(x, b4) or x & ~1 >= 0x08200000:
                break
            states.append(x)
            if any(lo <= s & ~1 <= stab + 4 * len(states) for s in states):
                break   # (the table ends where the code it names begins)
        mine = [s for s in states if s in self.pstates and not lo <= s & ~1 < hi]

        def inside(a):
            a &= ~1
            return lo <= a < hi or bool(mine) and PLAYER <= a < PLAYER_END
        walks, calls, ptrs, tabs = {}, collections.defaultdict(set), collections.defaultdict(set), {}
        todo = [rt, init, update] + [s for s in states if lo <= s & ~1 < hi] + mine
        while todo:
            e = todo.pop() & ~1
            if e in walks:
                continue
            w = T.walk(b4, e)
            walks[e] = w
            stop = {a for ww in walks.values() for a in ww.insns}
            for v in w.lits.values():
                if v is None:
                    continue
                if not v & 3 and inside(v) and v != stab and b4.has(v, 4) and T.is_code_ptr(b4.u32(v), b4):
                    tabs[v] = self.table_at(v, stop)
                    for x in tabs[v]:
                        (todo.append(x) if inside(x) else ptrs[x & ~1].add(e))
                elif T.is_code_ptr(v, b4) and v & ~1 < 0x08200000:
                    (todo.append(v) if inside(v) else ptrs[v & ~1].add(e))
            for t in w.bls.values():
                (todo.append(t) if inside(t) else calls[t].add(e))
        return dict(ty=ty, rt=rt, lo=lo, hi=hi, init=init, update=update, dying=dying, stab=stab, states=states,
                    mine=mine, walks=walks, calls=calls, ptrs=ptrs, tabs=tabs,
                    upd_calls=[wu.bls[s] for s in sorted(wu.bls)])

    def code(self, r, part):
        out = set()
        for e, w in r['walks'].items():
            if (part == 'own') != (r['lo'] <= e < r['hi']):
                continue
            for a, i in w.insns.items():
                out.update(range(a, a + i.size))
        return len(out)

    def calls_out(self, r):
        """the calls out a moved copy makes: none from its type's routine, init and update (BN6's Navi tables'),
        the literals' pointers but its states"""
        skip = {r['rt'], r['init'] & ~1, r['update'] & ~1}
        out = collections.defaultdict(set)
        for e, w in r['walks'].items():
            if e in skip:
                continue
            for t in w.bls.values():
                if t in r['calls']:
                    out[t].add(e)
        for p, by in r['ptrs'].items():
            if by - skip and self.b4.u16(p) & 0xFF00 in (0xB500, 0xB400):   # (a routine's push, not data)
                out[p] |= by - skip
        return out

    def objects(self, r):
        """the objects a Navi spawns (its init's spawns are companions), and the routines it calls in another type's
        block that spawn nothing (copied with it)"""
        spawned, helpers = {}, set()
        for t, by in r['calls'].items():
            kind = self.type_of(t)
            if not kind.startswith('T'):
                continue
            w = T.walk(self.b4, t)
            sp = [(SPAWN[x], T.back_value(w, s, 0)[0]) for s, x in w.bls.items() if x in SPAWN]
            if t in SPAWN:
                sp = [(SPAWN[t], T.back_value(r['walks'][e], s, 0)[0]) for e in by for s, x in r['walks'][e].bls.items()
                      if x == t]
            if not sp:
                if self.twin(t)[2] not in ('SAME', 'LAYOUT'):
                    helpers.add((kind, t))
                continue
            for k, v in sp:
                key = f'T{k}:{v:#04x}' if v is not None else f'T{k}:?'
                spawned[key] = spawned.get(key, False) or (r['init'] & ~1) in by
        return spawned, helpers

    def obj_twins(self, key):
        """BN6's types of the same kind whose init and update are its init's and update's bodies (SAME or LAYOUT)"""
        k, i = int(key[1]), int(key.split(':')[1], 16)
        if i >= len(self.types[k]):
            return None
        e4 = self.jump_table(self.b4, self.types[k][i] & ~1)
        out = []
        base, n = T6[k]
        for t in range(n):
            a = self.b6.u32(base + 4 * t)
            if not T.is_code_ptr(a, self.b6):
                continue
            e6 = self.jump_table(self.b6, a & ~1)
            if e4 and len(e6) == len(e4) and all(T.compare(self.b4, x, self.b6, y)[0] != 'DIFF' for x, y in zip(e4[:2], e6[:2])):
                out.append(t)
        return out

    @staticmethod
    def jump_table(rom, a):
        w = T.walk(rom, a)
        for v in w.lits.values():
            if v and not v & 3 and rom.has(v, 4) and T.is_code_ptr(rom.u32(v), rom):
                out = []
                while len(out) < 8 and T.is_code_ptr(rom.u32(v + 4 * len(out)), rom):
                    out.append(rom.u32(v + 4 * len(out)) & ~1)
                return out
        return []

    def fields(self, r):
        """the AI data's and the object's fields the Navi's code reads and writes: a register tracker over each
        routine in address order, a state starting as the runner leaves it (r6 the collision data, r7 the AI data),
        a jump table's entries with r4, r6 and r7 as the routine that dispatches them has them"""
        init = {s & ~1: {6: ('COLL', 0), 7: ('AI', 0)} for s in r['states']}
        out = collections.defaultdict(set)
        for _ in range(4):
            for e in sorted(r['walks']):
                w = r['walks'][e]
                before = track(w, init.get(e, {}), out)
                for v in w.lits.values():
                    if v in r['tabs']:
                        at = (w.calls_ind + w.indirect)[:1]
                        st = before.get(at[0], {}) if at else {}
                        for x in r['tabs'][v]:
                            init.setdefault(x & ~1, {k: st[k] for k in (4, 6, 7) if st.get(k)})
        return out


def track(w, regs, out):
    """registers by what they point at (the object, its collision data, its AI data, a number) through a walk"""
    regs = dict(regs)
    regs[5] = ('OBJ', 0)
    before = {}
    for a in sorted(w.insns):
        i, f = w.insns[a], w.insns[a].form
        before[a] = dict(regs)
        if f in T.MEM_FORMS and len(i.f) == 3:
            rd, rn, off = i.f
            base = regs.get(rn)
            if base and (base[0] in ('AI', 'COLL') or base[1] + off >= 0x5C):
                out[(base[0], base[1] + off)].add(f)
            if f.startswith('ldr'):
                is_ptr = base == ('OBJ', 0) and f == 'ldr' and off in (0x54, 0x58)
                (regs.__setitem__(rd, ('COLL' if off == 0x54 else 'AI', 0)) if is_ptr else regs.pop(rd, None))
            continue
        if f == 'movs':
            regs[i.f[0]] = ('IMM', i.f[1])
        elif f == 'adds8' and regs.get(i.f[0]):
            regs[i.f[0]] = (regs[i.f[0]][0], regs[i.f[0]][1] + i.f[1])
        elif f == 'adds3' and regs.get(i.f[1]):
            regs[i.f[0]] = (regs[i.f[1]][0], regs[i.f[1]][1] + i.f[2])
        elif f == 'adds' and len(i.f) == 3 and sum_of(regs.get(i.f[1]), regs.get(i.f[2])):
            regs[i.f[0]] = sum_of(regs.get(i.f[1]), regs.get(i.f[2]))
        elif f == 'hi_mov' and max(i.f) < 8 and regs.get(i.f[1]):
            regs[i.f[0]] = regs[i.f[1]]
        else:
            for k in range(8):
                if T.writes(i, k):
                    regs.pop(k, None)
    return before


def sum_of(x, y):
    """a base plus a number, or None"""
    if not x or not y or (x[0] == 'IMM') == (y[0] == 'IMM'):
        return None
    base, n = (x, y) if y[0] == 'IMM' else (y, x)
    return base[0], base[1] + n[1]


def report(m, which):
    lines, rows, users = [], [], collections.defaultdict(set)
    handled = set(bn4_twins.COPIED) | {a for a, _ in bn4_twins.AUDITS}
    for name, ty, first in NAVIS:
        if which and name not in which:
            continue
        r = m.walk(ty)
        out = m.calls_out(r)
        cls = collections.Counter()
        for t in out:
            if m.type_of(t) == 'engine':
                c = m.twin(t)[2] or 'none'
                cls[c] += 1
                if c in ('none', 'DIFF') and t not in REPLACED and t not in handled:
                    users[t].add(name)
        spawned, helpers = m.objects(r)
        objs = {k: m.obj_twins(k) for k in spawned}
        fl = m.fields(r)
        ai = sorted(o for b, o in fl if b == 'AI')
        var = sorted(o for b, o in fl if b == 'OBJ' and o >= 0x60)
        takes_ai = TAKE_AI in T.walk(m.b4, r['init']).bls.values()
        st = ['g' if k < 6 and s == NAVI_GENERIC[k] else str(k) if r['lo'] <= s & ~1 < r['hi']
              else f'MegaMan {m.pstates.index(s)}' if s in r['mine'] else f'{s:08X}' for k, s in enumerate(r['states'])]
        hp = [m.b4.u16(STATS + STAT_SIZE * (first + k)) & 0xFFF for k in range(4)]
        own, theirs = m.code(r, 'own'), m.code(r, 'player')
        extra = [f'{t:08X}' for t in r['upd_calls'] if t not in (0x080135A8, 0x08013DB4, 0x08012D62)]
        lines += [f'## {name}', '',
                  f'Ids {first}-{first + 3} (HP {", ".join(map(str, hp))}), type {ty:#04x}, routine {r["rt"]:08X}, block '
                  f'{r["lo"]:08X}-{r["hi"]:08X}; init {r["init"]:08X}, update {r["update"]:08X}, dying {r["dying"]:08X}'
                  + (f'; update also calls {", ".join(extra)}' if extra else '') + '.', '',
                  f'- AI data: {"taken at its init (its side\'s)" if takes_ai else "none: its variables in the object"}; '
                  f'its fields read or written: {" ".join(f"{o:#x}" for o in ai) or "none"}; the object\'s from 0x60: '
                  f'{" ".join(f"{o:#x}" for o in var) or "none"}',
                  f'- States ({len(st)} at {r["stab"]:08X}): {", ".join(st)}',
                  f'- Code: {own} bytes its own, {theirs} of MegaMan\'s states; {len(r["walks"])} routines',
                  f'- Calls out of the engine\'s: ' + ', '.join(f'{k} {v}' for k, v in sorted(cls.items())),
                  '- Objects: ' + (', '.join(f'{k}{" (at its spawn)" if spawned[k] else ""} '
                                             f'{"BN6 " + " ".join(f"{t:#04x}" for t in objs[k]) if objs[k] else "no BN6 twin"}'
                                             for k in sorted(objs)) or 'none'),
                  '- Routines in another type\'s block: ' + (', '.join(f'{t:08X} ({k})' for k, t in sorted(helpers)) or 'none'),
                  '']
        obj_code = 0
        for k in spawned:
            if k.endswith('?'):
                continue
            kk, i = int(k[1]), int(k.split(':')[1], 16)
            if i < len(m.types[kk]) and not objs.get(k):
                a = m.types[kk][i] & ~1
                b = m.block(a)
                obj_code += b[1] - b[0] if b else 0
        rows.append((own + theirs + obj_code, name, own, theirs, len(spawned), obj_code))
    lines += ['## By cost', '', '| Navi | Own code | MegaMan\'s states | Objects | Their blocks | All |', '| --- | --- | --- | --- | --- | --- |']
    for total, name, own, theirs, n, oc in sorted(rows):
        lines.append(f'| {name} | {own} | {theirs} | {n} | {oc} | {total} |')
    lines += ['', '## The engine\'s routines to port or audit once', '',
              'Calls out with no twin in BN6 or a twin of another body, but those BN6\'s Navi tables stand in for '
              'and those the families\' work already took (tools/bn4_twins.py):', '']
    for t, u in sorted(users.items(), key=lambda x: (-len(x[1]), x[0])):
        lines.append(f'- {t:08X} ({m.twin(t)[2] or "none"}): {len(u)}, {", ".join(sorted(u))}')
    return '\n'.join(lines) + '\n'


def main():
    which = set(sys.argv[1:])
    m = Map()
    text = report(m, which)
    os.makedirs(OUT, exist_ok=True)
    path = os.path.join(OUT, 'report.md')
    with open(path, 'w', encoding='utf-8') as f:
        f.write('# BN4 Red Sun\'s Navis against BN6\'s engine (tools/bn4_navis.py)\n\n' + text)
    print(path)
    return 0


if __name__ == '__main__':
    sys.exit(main())
