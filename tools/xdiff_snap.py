"""xdiff_snap.py: the canonical snapshot of a fight frame for tools/xdiff.py (issue #82): the traces
src/dev/xdiff.c writes, each game's numbers read as BN6's (the engine's own words among them: the alliance
tables' words, an object's slot), objects matched by role, pointers named by what they point at, and the
differences of the two games' frames through each game's layouts (tests/xdiff/legit.json; docs/DEVTOOLS.md,
The differential)."""
import functools
import os
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(ROOT, '.build', 'xdiff')

# Each game's battle as the trace was read (src/emu/bn5.h, bn6.h)
GAMES = {
    5: dict(t1=0x0203B200, t3=0x0203CA40, t4=0x02036F00, panels=0x0203A100, list=0x0200A410, list_end=0x0200ABA0, n1=16),
    6: dict(t1=0x0203A9B0, t3=0x0203CFE0, t4=0x02036870, panels=0x02039AE0, list=0x02009380, list_end=0x02009AB0, n1=32),
}
RNG_XOR = 0x873CA9E5                              # both games' RNG step (BN5_RNG_XOR)
XV_CODE = (0x08B38000, 0x08B40000)                # where the install puts the families' code in BN6 (src/emu/xvirus_recipe.h, XV_CODE to XV_ART)
MEGAMAN_BUSTER = 0x11                             # MegaMan's CurAction as his buster fires, in both
TWIN = 32                                         # the bytes two ROM pointers' data must share to be the same data
NODE = 0x10                                       # the list node before an object (BN6_OBJECT_NODE)
SIZE = {'T1': 0xD8, 'T3': 0xD8, 'T4': 0xC8}       # an object's record, from its node
SPRITE = {'T1': 0x90, 'T3': 0x90, 'T4': 0x80}     # its sprite's state in it (object offsets)
SPRITE_DATA = (0x18, 0x1C, 0x20, 0x24)            # ... the words that point into the sprite's own data
AI_SIZE, COLL_ROOM, PANEL_ROOM, PANELS = 0x100, 0xA8, 0x24, 40
# MegaMan's side's player block (src/dev/xdiff.c, trace version 3): his battle NaviStats, the word of what was used this
# battle (BN5's Souls, BN6's Crosses), his pending form's 16 bytes
PLAYER_NAVI, PLAYER_USED, PLAYER_PENDING, NAVI_FORM = 0x00, 0x64, 0x68, 0x2C
PENDING_OBJECT = PLAYER_PENDING + 8   # (the pending record's +8: his battle object)
OBJ_NAME_ID, OBJ_ACTION, OBJ_TYPE, OBJ_EFFECT = 0x28, 0x09, 0x01, 0x04
AI_INDEX = 0x01


def u16(b, o):
    return struct.unpack_from('<H', b, o)[0]


def u32(b, o):
    return struct.unpack_from('<I', b, o)[0]


# ---- traces (src/dev/xdiff.c) ----

class Obj:
    __slots__ = ('kind', 'slot', 'rec', 'ai', 'coll', 'addr', 'role', 'over', 'leftover')

    def __init__(self, kind, slot, rec, ai, coll, addr):
        self.kind, self.slot, self.rec, self.ai, self.coll, self.addr, self.role = kind, slot, rec, ai, coll, addr, None
        self.over = {}   # (bytes of its record read otherwise: the renderer's places as the family's, xdiff_masks.py)
        self.leftover = False   # (its sprite state the slot's last occupant's: spawned and no sprite set up since, xdiff_events.py)

    def canonical(self):
        """its record as it is compared: its slot (+3) counted from its own (objects are matched by role, never by
        slot: legit.json, maps, slot), and what over holds"""
        b = bytearray(self.rec)
        b[NODE + 3] = (b[NODE + 3] - self.slot) & 0xFF
        for k, v in self.over.items():
            b[k] = v
        return bytes(b)

    def o8(self, off):
        return self.rec[NODE + off]

    def o16(self, off):
        return u16(self.rec, NODE + off)

    def o32(self, off):
        return u32(self.rec, NODE + off)


class Event:
    """One of a core's hooks' events (src/dev/xdiff_events.h): kind, sub and flags; the core's frame and the battle's timer
    before it; its caller, the family's code that led to it, the outermost caller; the object; a and b; ref"""
    __slots__ = ('kind', 'sub', 'flags', 'frame', 't', 'caller', 'family', 'outer', 'object', 'a', 'b', 'ref', 'at')

    def __init__(self, b, p, at):
        self.kind, self.sub, self.flags = b[p], b[p + 1], u16(b, p + 2)
        (self.frame, self.t, self.caller, self.family, self.outer, self.object, self.a, self.b,
         self.ref) = struct.unpack_from('<9I', b, p + 4)
        self.at = at   # (the timer of the record it came in)


EVENT_SIZE = 40
DRAW, SPAWN, SOUND, EFFECT, ATTACK, RESERVE, BLINK, SPRITE_SET, MOOD = range(1, 10)
KEPT, REPLAYED, UNMATCHED, UNMAPPED = 1, 2, 4, 8


def read_trace(path):
    with open(path, 'rb') as f:
        b = f.read()
    if b[:4] != b'XDF1':
        sys.exit(f'{path}: no trace')
    version, game, coll, psize, nm = struct.unpack_from('<5I', b, 4)
    p = 24
    moved = [struct.unpack_from('<3I', b, p + 12 * i) for i in range(nm)]
    p += 12 * nm
    player_size = 0
    if version >= 3:
        player_size = struct.unpack_from('<I', b, p)[0]
        p += 4
    g, frames = GAMES[game], []
    while p < len(b):
        if b[p:p + 4] != b'FRAM':
            sys.exit(f'{path}: broken at {p}')
        f, t = struct.unpack_from('<2I', b, p + 4)
        sub, ph, n1, n3, n4, ns, no = b[p + 12:p + 19]
        rng = struct.unpack_from('<2I', b, p + 20)
        p += 28
        nev = 0
        if version >= 2:
            nev = struct.unpack_from('<I', b, p)[0]
            p += 4
        player = b[p:p + player_size]   # (MegaMan's side's: his battle NaviStats, the used word, his pending form; src/dev/xdiff.c)
        p += player_size
        objs = []
        for i in range(n1):
            slot, has_ai, has_coll = b[p], b[p + 1], b[p + 2]
            rec = b[p + 4:p + 4 + 0xD8]
            ai = b[p + 4 + 0xD8:p + 4 + 0x1D8]
            co = b[p + 4 + 0x1D8:p + 4 + 0x1D8 + COLL_ROOM]
            objs.append(Obj('T1', slot, rec, ai if has_ai else None, co[:coll] if has_coll else None, g['t1'] + slot * 0xD8))
            p += 4 + 0x1D8 + COLL_ROOM
        for i in range(n3):
            slot, has_coll = b[p], b[p + 2]
            rec = b[p + 4:p + 4 + 0xD8]
            co = b[p + 4 + 0xD8:p + 4 + 0xD8 + COLL_ROOM]
            objs.append(Obj('T3', slot, rec, None, co[:coll] if has_coll else None, g['t3'] + slot * 0xD8))
            p += 4 + 0xD8 + COLL_ROOM
        for i in range(n4):
            slot = b[p]
            objs.append(Obj('T4', slot, b[p + 4:p + 4 + 0xC8], None, None, g['t4'] + slot * 0xC8))
            p += 4 + 0xC8
        panels = [b[p + PANEL_ROOM * i:p + PANEL_ROOM * i + psize] for i in range(PANELS)]
        p += PANEL_ROOM * PANELS
        order = list(struct.unpack_from(f'<{no}I', b, p))
        p += 4 * no
        sounds = [struct.unpack_from('<2I', b, p + 8 * i) for i in range(ns)]
        p += 8 * ns
        events = [Event(b, p + EVENT_SIZE * i, t) for i in range(nev)]
        p += EVENT_SIZE * nev
        frames.append(dict(f=f, t=t, sub=sub, ph=ph, rng=rng, objs=objs, panels=panels, order=order, sounds=sounds, events=events,
                           player=player))
    return dict(game=game, coll=coll, psize=psize, moved=moved, frames=frames, version=version)


# ---- each game's numbers as BN6's (legit.json's maps) ----

class Canon:
    def __init__(self, legit):
        m = legit['maps']
        num = lambda d: {int(k, 0): int(v, 0) for k, v in d.items() if not k.startswith('_')}
        self.name_id = num(m['name_id']['bn5_to_bn6'])
        self.t3_type = num(m['t3_type']['bn5_to_bn6'])
        self.effect = num(m['effect']['bn5_to_bn6'])
        self.ai_index = num(m['ai_index']['bn5_to_bn6'])
        self.megaman = num(m['name_id']['megaman'])
        self.mm_action = num(m['action']['megaman'])
        self.layouts = legit['layouts']
        self.legit = legit
        self.t3_actions = {int(x, 0) for x in m['action'].get('t3_types', [])}
        self.tables = [dict(t, bn5=int(t['bn5'], 0), bn6=int(t['bn6'], 0)) for t in m.get('rom_tables', {}).get('tables', [])]
        self.natives = set()   # (a spec's: enemies both games have, by BN6's id; their attacks' types, their effects)
        self.native_attacks, self.native_effects = set(), set()
        self.family_effects = {int(x, 0) for x in m['family_effects']['bn6']}
        self.form = num(m['form']['bn5_to_bn6']) if 'form' in m else {}
        self.power_attack = num(m['power_attack']['bn5_to_bn6']) if 'power_attack' in m else {}
        self.sprite_pairs = m['sprites'] if 'sprites' in m else None
        self._sprites = None

    def sprite_name(self, game, base):
        """the registry's name of a sprite (its data's start) in this game, or None (maps, sprites: each pair a list and entry of
        each game's sprite lists, resolved from its ROM)"""
        if self._sprites is None:
            self._sprites = {5: {}, 6: {}}
            for g, key, at in ((5, 'bn5_lists', 0), (6, 'bn6_lists', 2)):
                if not self.sprite_pairs:
                    break
                lists = int(self.sprite_pairs[key], 0)
                for pair in self.sprite_pairs['pairs']:
                    lst, idx = pair[at], pair[at + 1]
                    entry = u32(rom(g), u32(rom(g), lists - ROM[0] + 4 * lst) - ROM[0] + 4 * idx) & 0x7FFFFFFF
                    self._sprites[g][entry] = pair[4]
        return self._sprites[game].get(base)

    def nid(self, game, v):
        return self.name_id.get(v, v) if game == 5 else v

    def is_megaman(self, game, nid):
        return nid in (self.megaman.values() if game == 6 else self.megaman.keys())

    def action(self, game, megaman, v):
        """CurAction as BN6 numbers it: a virus's 0-5 the same, BN5's 6 BN6's 8, 7 on two up; MegaMan's by his map"""
        if game == 6:
            return v
        if megaman:
            return self.mm_action.get(v, v)
        return v if v < 6 else 8 if v == 6 else v + 2

    def form_of(self, game, v):
        """MegaMan's form as BN6 numbers it (a BN5 Soul by legit.json's map: none until the Souls are installed)"""
        return self.form.get(v, v) if game == 5 else v

    def t3(self, game, v):
        return self.t3_type.get(v, v) if game == 5 else v

    def fx(self, game, v):
        return self.effect.get(v, v) if game == 5 else v


# ---- roles ----

def identity(game, o, canon):
    """what an object is, in BN6's numbers: an enemy by its id, an attack by its type, an effect by its type, and an
    effect object (the fourth kind's type 0) by its effect id too, so one spawned with the wrong id is another role"""
    if o.kind == 'T1':
        return canon.nid(game, o.o16(OBJ_NAME_ID))
    if o.kind == 'T3':
        return canon.t3(game, o.o8(OBJ_TYPE))
    if o.o8(OBJ_TYPE) == 0:
        return (0, canon.fx(game, o.o8(OBJ_EFFECT)))
    return o.o8(OBJ_TYPE)


def assign_roles(trace, canon):
    """Each object its role: (kind, what it is, the how-manyth of those to appear); one
    that stays in its slot as the same thing keeps it"""
    prev, counts = {}, {}
    for fr in trace['frames']:
        now = {}
        for o in fr['objs']:
            ident = identity(trace['game'], o, canon)
            p = prev.get((o.kind, o.slot))
            if p and p[0] == ident:
                o.role = p[1]
            else:
                key = (o.kind, ident)
                o.role = (o.kind, ident, counts.get(key, 0))
                counts[key] = counts.get(key, 0) + 1
            now[(o.kind, o.slot)] = (ident, o.role)
        prev = now


def role_name(r):
    if r is None:
        return '-'
    kind, ident, n = r
    if isinstance(ident, tuple):
        return f'{kind}:0x0/fx{ident[1]:#x}#{n}'
    return f'{kind}:{ident:#x}#{n}'


# ---- pointers ----

RAM = ((0x02000000, 0x02040000), (0x03000000, 0x03008000))
ROM = (0x08000000, 0x0A000000)


def is_ptr(v):
    return any(lo <= v < hi for lo, hi in RAM) or ROM[0] <= v < ROM[1]


class Frame:
    """One frame of one game, with what its pointers name"""

    def __init__(self, trace, fr, canon):
        self.game, self.fr, self.canon, self.trace = trace['game'], fr, canon, trace
        g = GAMES[self.game]
        self.spans = []
        for o in fr['objs']:
            self.spans.append((o.addr - NODE, o.addr - NODE + SIZE[o.kind], o.role, 'obj', -NODE))
            if o.kind != 'T4':
                ai, co = o.o32(0x58) if o.kind == 'T1' else 0, o.o32(0x54)
                if o.ai is not None:
                    self.spans.append((ai, ai + AI_SIZE, o.role, 'ai', 0))
                if o.coll is not None:
                    self.spans.append((co, co + trace['coll'], o.role, 'coll', 0))
        self.spans.append((g['panels'], g['panels'] + PANELS * trace['psize'], 'panels', 'panel', 0))
        # (an object let go this frame, its slot named: its pointers still point there)
        self.slots = [(kind, g[kind.lower()] - NODE, SIZE[kind], n) for kind, n in (('T1', g['n1']), ('T3', 32), ('T4', 32))]
        self.list, self.list_end = g['list'], g['list_end']
        self.by_role = {o.role: o for o in fr['objs']}
        self.moved = trace['moved'] if self.game == 6 else [m for m in trace.get('code', ()) if XV_CODE[0] <= m[1] < XV_CODE[1]]

    def ptr(self, v):
        """a pointer as what it names"""
        if v == self.list:
            return ('list head',)
        if v == self.list_end:
            return ('list end',)
        for lo, hi, role, part, base in self.spans:
            if lo <= v < hi:
                off = v - lo + base
                if part == 'coll':
                    off = coll_off(self.canon, self.game, off)
                if part == 'panel':
                    return ('panel', off // self.trace['psize'], panel_off(self.canon, self.game, off % self.trace['psize']))
                return (role_name(role), part, off)
        for kind, base, size, n in self.slots:
            if base <= v < base + size * n:
                return (f'{kind} slot {(v - base) // size} (let go)', 'obj', (v - base) % size - NODE)
        if ROM[0] <= v < ROM[1]:
            # (BN6's: what was moved, as where it came from; BN5's: the families' code where it is, as the BN6 trace's
            # moved blocks name it, so a pointer to their code reads alike in both, a callback kept in an AI's data)
            for frm, to, ln in self.moved:
                base = to if self.game == 6 else frm
                if base <= (v & ~1) < base + ln:
                    return ('rom', v - base + frm)
        return ('raw', v)


def coll_off(canon, game, off):
    """an offset in a collision data as BN6's layout has it"""
    if game == 6:
        return off
    for a, b, n in canon.layouts['collision']['segments']:
        if a <= off < a + n:
            return off - a + b
    return ('bn5 only', off)


def panel_off(canon, game, off):
    if game == 6:
        return off
    for a, b, n in canon.layouts['panel']['segments']:
        if a <= off < a + n:
            return off - a + b
    return ('bn5 only', off)


# ---- the comparison ----

class Diff:
    __slots__ = ('t', 'role', 'part', 'off', 'a', 'b', 'mask')

    def __init__(self, t, role, part, off, a, b):
        self.t, self.role, self.part, self.off, self.a, self.b, self.mask = t, role, part, off, a, b, None

    def __str__(self):
        fmt = lambda v: f'{v:#x}' if isinstance(v, int) else str(v)
        return f't{self.t} {role_name(self.role) if isinstance(self.role, tuple) else self.role} {self.part}+{self.off:#04x}: BN5 {fmt(self.a)} | BN6 {fmt(self.b)}' \
            + (f'  [{self.mask}]' if self.mask else '')


ROMS = {}


def rom(game):
    """each game's ROM, as find_roms() copied it"""
    if not ROMS:
        for g, name in ((5, 'bn5.gba'), (6, 'bn6.gba')):
            with open(os.path.join(WORK, 'rom', name), 'rb') as f:
                ROMS[g] = f.read()
    return ROMS[game]


def twin(wa, wb):
    """two pointers into each game's own ROM at the same data (TWIN bytes the same: a table each game has)"""
    a, b = (wa & ~1) - ROM[0], (wb & ~1) - ROM[0]
    if not (0 <= a < len(rom(5)) - TWIN and 0 <= b < len(rom(6)) - TWIN):
        return False
    return rom(5)[a:a + TWIN] == rom(6)[b:b + TWIN]


ALLIANCE = {}


def alliance_map(canon):
    """BN5's alliance and region words as BN6's: each entry of BN5's table to BN6's entry at its place (its region
    and alliance), and so with the time stop's bit 0x10000 the setup ORs in (legit.json, maps, alliance); a word two
    entries would give otherwise is left as it is. The tables are checked to pair as legit.json says."""
    if not ALLIANCE:
        t = canon.layouts['collision']['alliance_masks']
        a5, a6, n, stop = int(t['bn5'], 0) - ROM[0], int(t['bn6'], 0) - ROM[0], t['words'], int(t['time_stop'], 0)
        pairs = {}
        for i in range(n):
            w5, w6 = u32(rom(5), a5 + 4 * i), u32(rom(6), a6 + 4 * i)
            if (w6 not in (w5, w5 | 0x80)) != (i in t['unpaired']):
                sys.exit(f'the alliance tables are not as legit.json has them at entry {i}: BN5 {w5:#x}, BN6 {w6:#x}')
            for s in (0, stop):
                pairs.setdefault(w5 | s, set()).add(w6 | s)
        ALLIANCE.update({w5: w6s.pop() for w5, w6s in pairs.items() if len(w6s) == 1})
    return ALLIANCE


@functools.lru_cache(maxsize=None)
def alliance_or(w5):
    """a word ORed from other objects' +0x30 (what hit or touched an object, legit.json, maps, alliance) as BN6's:
    the OR of the BN6 words of the BN5 words it is made of, where every way of making it of the table's words gives
    the same; None where none does or two give otherwise (then the word is compared as it is)"""
    parts = [k for k in ALLIANCE if k and k & ~w5 == 0]
    if len(parts) > 16:
        return None
    made = {(0, 0)}
    for k in parts:
        made |= {(a | k, b | ALLIANCE[k]) for a, b in made}
    got = {b for a, b in made if a == w5}
    return got.pop() if len(got) == 1 else None


def alliance_words(canon, ca):
    """BN5's collision data with its alliance words read as BN6's (legit.json, layouts, collision, alliance_masks:
    the table's words and the words ORed of them, at BN6's offsets)"""
    t, out = canon.layouts['collision']['alliance_masks'], bytearray(ca)
    alliance_map(canon)
    for key, read in (('table_words', ALLIANCE.get), ('or_words', alliance_or)):
        for x in t[key]:
            o5 = bn5_coll_off(canon, int(x, 0))
            w6 = read(u32(ca, o5))
            if w6 is not None:
                struct.pack_into('<I', out, o5, w6)
    return bytes(out)


def bn5_coll_off(canon, o6):
    for a, b, n in canon.layouts['collision']['segments']:
        if b <= o6 < b + n:
            return o6 - b + a
    sys.exit(f'collision +{o6:#x}: no BN5 place in legit.json\'s layout')


def table_view(canon, wa, wb):
    """two pointers into a table both games' ROMs hold (legit.json, maps, rom_tables), as (the table, the place in it), the
    entry's bytes the same in both; None where either points elsewhere"""
    for t in canon.tables:
        ia, ib, n = wa - t['bn5'], wb - t['bn6'], t['entry'] * t['count']
        if not (0 <= ia < n and 0 <= ib < n):
            continue
        k = ia - ia % t['entry']
        same = ia == ib and rom(5)[t['bn5'] - ROM[0] + k:t['bn5'] - ROM[0] + k + t['entry']] == \
            rom(6)[t['bn6'] - ROM[0] + k:t['bn6'] - ROM[0] + k + t['entry']]
        return ('table', t['name'], ia), ('table', t['name'], ib) if same or ia != ib else ('table', t['name'], ib, 'its entry otherwise')
    return None


def word_view(fa, fb, wa, wb, sprite_rel):
    """two words as each game means them: pointers by what they name, a pointer into each game's own ROM by
    the data there (None: no pointer, compare the bytes)"""
    if sprite_rel is not None:
        return sprite_rel
    if is_ptr(wa) or is_ptr(wb):
        va, vb = fa.ptr(wa), fb.ptr(wb)
        if va[0] == 'raw' and vb[0] == 'raw' and table_view(fa.canon, wa, wb):
            return table_view(fa.canon, wa, wb)
        if va[0] == 'raw' and vb[0] == 'raw':
            if ROM[0] <= wa < ROM[1] and ROM[0] <= wb < ROM[1] and twin(wa, wb):
                return ('twin data',), ('twin data',)
            return None
        return va, vb
    return None


def obj_cells(fa, fb, oa, ob):
    """(part, offset as BN6 has it, BN5's value, BN6's) for each place they differ, their numbers BN6's"""
    out = []
    canon, kind = fa.canon, oa.kind
    mm = kind == 'T1' and canon.is_megaman(5, oa.o16(OBJ_NAME_ID))
    sp = SPRITE[kind] + NODE
    ra, rb = oa.canonical(), ob.canonical()
    base_a, base_b = u32(ra, sp + 0x18), u32(rb, sp + 0x18)
    # (a sprite state is no state of the object's before it sets one up: its spawn clears its record to there, kinds 1
    # and 3 0x8C bytes from +4, kind 4 0x7C, BN5 0x080030CC / BN6 0x080032AC, and the slot's last occupant's is left)
    if oa.leftover != ob.leftover:
        out.append(('obj', sp - NODE, 'no sprite set up' if oa.leftover else 'its sprite set up', 'no sprite set up' if ob.leftover else
                    'its sprite set up'))
    end = sp if oa.leftover or ob.leftover else SIZE[kind]
    for w in range(8, end, 4):   # (its node's links: the run order, compared as such)
        wa, wb = u32(ra, w), u32(rb, w)
        rel = None
        if w - sp in SPRITE_DATA and base_a and base_b and is_ptr(wa) and is_ptr(wb):
            rel = ('sprite data', wa - base_a), ('sprite data', wb - base_b)
            # (the sprite itself, its data pointer: by the registry where either game's is one of its pairs)
            na, nb = canon.sprite_name(5, base_a - 4), canon.sprite_name(6, base_b - 4)
            if w - sp == 0x18 and (na or nb):
                rel = ('sprite', na or f'{base_a - 4:#x}'), ('sprite', nb or f'{base_b - 4:#x}')
        view = word_view(fa, fb, wa, wb, rel)
        if view is not None:
            if view[0] != view[1]:
                out.append(('obj', w - NODE, view[0], view[1]))
            continue
        for k in range(4):
            off = w + k - NODE
            a, b = ra[w + k], rb[w + k]
            if off == OBJ_ACTION and (kind == 'T1' or kind == 'T3' and canon.t3(5, oa.o8(OBJ_TYPE)) in canon.t3_actions):
                a = canon.action(5, mm, a)
            elif off == OBJ_TYPE and kind == 'T3':
                a = canon.t3(5, a)
            elif off == OBJ_EFFECT and kind == 'T4' and oa.o8(OBJ_TYPE) == 0:
                a = canon.fx(5, a)
            elif off in (OBJ_NAME_ID, OBJ_NAME_ID + 1) and kind == 'T1':
                na, nb = canon.nid(5, oa.o16(OBJ_NAME_ID)), ob.o16(OBJ_NAME_ID)
                a, b = (na & 0xFF, nb & 0xFF) if off == OBJ_NAME_ID else (na >> 8, nb >> 8)
            if a != b:
                out.append(('obj', off, a, b))
    return out


LOADOUT = {4, 5, 6, 7, 8, 0x11}   # MegaMan's AI data: his power attacks (BButton, APwrAtk, ..., the chaos/Beast Out attack)


def ai_cells(fa, fb, oa, ob):
    out = []
    if (oa.ai is None) != (ob.ai is None):
        return [('ai', 0, oa.ai is not None, ob.ai is not None)]
    if oa.ai is None:
        return out
    megaman = oa.kind == 'T1' and fa.canon.is_megaman(5, oa.o16(OBJ_NAME_ID))
    for w in range(0, AI_SIZE, 4):
        wa, wb = u32(oa.ai, w), u32(ob.ai, w)
        view = word_view(fa, fb, wa, wb, None)
        if view is not None:
            if view[0] != view[1]:
                out.append(('ai', w, view[0], view[1]))
            continue
        for k in range(4):
            a, b = oa.ai[w + k], ob.ai[w + k]
            if w + k == AI_INDEX:
                a = fa.canon.ai_index.get(a, a)
            elif megaman and w + k in LOADOUT and a != 0xFF:
                a = fa.canon.power_attack.get(a, a)
            if a != b:
                out.append(('ai', w + k, a, b))
    return out


def bn5_rules(layout, ca):
    """BN5's own bytes that are the parts of a byte both have (legit.json, layouts, collision, bn5_or: the word
    BN5 ORs them into, compared with BN6's): their offsets, and a difference where the word is not their OR"""
    parts, out = set(), []
    for r in layout.get('bn5_or', []):
        size, word = r['size'], int(r['word'], 0)
        get = (lambda o: u32(ca, o)) if size == 4 else (lambda o: ca[o])
        whole = 0
        for x in r['of']:
            whole |= get(int(x, 0))
            parts.update(range(int(x, 0), int(x, 0) + size))
        if get(word) != whole:
            out.append(('coll bn5 only', word, f'+{word:#x} {get(word):#x}, its parts\' OR {whole:#x}', None))
    return parts, out


def coll_cells(fa, fb, oa, ob):
    """the collision data through each game's layout (legit.json): a byte BN5 places where BN6 has it (its
    alliance words read as BN6's), BN5's own parts of a word by their rule, the rest of each zero in both or a
    difference"""
    out = []
    if (oa.coll is None) != (ob.coll is None):
        return [('coll', 0, oa.coll is not None, ob.coll is not None)]
    if oa.coll is None:
        return out
    canon, raw5, cb = fa.canon, oa.coll, ob.coll
    ca = alliance_words(canon, raw5)
    layout, seen6 = canon.layouts['collision'], set()
    flags = {int(x, 0) for x in layout['flag_words']}
    for a0, b0, n in layout['segments']:
        k = 0
        while k < n:
            o5, o6 = a0 + k, b0 + k
            if o5 % 4 == 0 and o6 % 4 == 0 and k + 4 <= n and o6 not in flags:
                wa, wb = u32(ca, o5), u32(cb, o6)
                view = word_view(fa, fb, wa, wb, None)
                if view is not None:
                    if view[0] != view[1]:
                        out.append(('coll', o6, view[0], view[1]))
                    seen6.update(range(o6, o6 + 4))
                    k += 4
                    continue
            if ca[o5] != cb[o6]:
                out.append(('coll', o6, ca[o5], cb[o6]))
            seen6.add(o6)
            k += 1
    seen5, broken = bn5_rules(layout, raw5)
    out += broken
    seen5 |= {a0 + k for a0, _, n in layout['segments'] for k in range(n)}
    for o5 in range(len(raw5)):
        if o5 not in seen5 and raw5[o5]:
            out.append(('coll bn5 only', o5, raw5[o5], None))
    for o6 in range(len(cb)):
        if o6 not in seen6 and cb[o6]:
            out.append(('coll', o6, None, cb[o6]))
    return out


def panel_cells(fa, fb, t):
    """the panels through each game's layout (legit.json): a byte outside it zero in both, or a difference"""
    out = []
    segs = fa.canon.layouts['panel']['segments']
    seen5 = {a0 + k for a0, _, n in segs for k in range(n)}
    seen6 = {b0 + k for _, b0, n in segs for k in range(n)}
    for i in range(PANELS):
        pa, pb, where = fa.fr['panels'][i], fb.fr['panels'][i], f'panel {i % 8},{i // 8}'
        for a0, b0, n in segs:
            for k in range(0, n, 4 if n % 4 == 0 else 1):
                size = 4 if n % 4 == 0 else 1
                wa = u32(pa, a0 + k) if size == 4 else pa[a0 + k]
                wb = u32(pb, b0 + k) if size == 4 else pb[b0 + k]
                view = word_view(fa, fb, wa, wb, None) if size == 4 else None
                if view is None:
                    view = (wa, wb)
                if view[0] != view[1]:
                    out.append(Diff(t, where, 'panel', b0 + k, view[0], view[1]))
        out += [Diff(t, where, 'panel bn5 only', o5, pa[o5], None) for o5 in range(len(pa)) if o5 not in seen5 and pa[o5]]
        out += [Diff(t, where, 'panel', o6, None, pb[o6]) for o6 in range(len(pb)) if o6 not in seen6 and pb[o6]]
    return out


def player_cells(fa, fb):
    """MegaMan's side's player block, where both traces hold one: every byte the same (his form and his pending form in
    BN6's numbers, the pending record's object word as what it names, BN5's bytes read in BN6's terms where a translation
    says so: its mood less its gains, xdiff_events.Translations), or a difference (role 'player': his)"""
    pa, pb, t = fa.fr.get('player', b''), fb.fr.get('player', b''), fa.fr['t']
    if not pa or not pb:
        return []
    if fa.fr.get('player_over'):
        pa = bytearray(pa)
        for k, v in fa.fr['player_over'].items():
            pa[k] = v
    out = []
    for off in range(0, min(len(pa), len(pb))):
        if PENDING_OBJECT <= off < PENDING_OBJECT + 4:
            if off == PENDING_OBJECT:
                va, vb = fa.ptr(u32(pa, off)), fb.ptr(u32(pb, off))
                if va != vb:
                    out.append(Diff(t, 'player', 'player', off, va, vb))
            continue
        a, b = pa[off], pb[off]
        if off in (PLAYER_NAVI + NAVI_FORM, PLAYER_PENDING):
            a = fa.canon.form_of(5, a) if a != 0xFF else a
        if a != b:
            out.append(Diff(t, 'player', 'player', off, a, b))
    return out


def frame_diffs(fa, fb):
    t, out = fa.fr['t'], []
    ra = {o.role: o for o in fa.fr['objs']}
    rb = {o.role: o for o in fb.fr['objs']}
    for role in sorted(set(ra) | set(rb), key=role_name):
        oa, ob = ra.get(role), rb.get(role)
        if oa is None or ob is None:
            out.append(Diff(t, role, 'in play', 0, oa is not None, ob is not None))
            continue
        cells = obj_cells(fa, fb, oa, ob)
        if oa.kind == 'T1':
            cells += ai_cells(fa, fb, oa, ob)
        if oa.kind != 'T4':
            cells += coll_cells(fa, fb, oa, ob)
        out += [Diff(t, role, part, off, a, b) for part, off, a, b in cells]
    if fa.fr['rng'] != fb.fr['rng']:
        out.append(Diff(t, 'battle', 'rng', 0, fa.fr['rng'], fb.fr['rng']))
    # (a song by its number: the music player it plays on is its song table entry's, each game's own: legit.json, maps)
    sa, sb = sorted(s for _, s in fa.fr['sounds']), sorted(s for _, s in fb.fr['sounds'])
    if sa != sb:
        out.append(Diff(t, 'battle', 'sounds', 0, sa, sb))
    # (the order the objects run in, of those in play in both: one in play in one alone is its own difference)
    both = {role_name(r) for r in set(ra) & set(rb)}
    oa = [p for p in (fa.ptr(a) for a in fa.fr['order']) if p[0] in both or len(p) != 3]
    ob = [p for p in (fb.ptr(a) for a in fb.fr['order']) if p[0] in both or len(p) != 3]
    if oa != ob:
        out.append(Diff(t, 'battle', 'run order', 0, oa, ob))
    out += panel_cells(fa, fb, t)
    out += player_cells(fa, fb)
    return out
