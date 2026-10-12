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
GAMES = {   # (state: the battle's state, BN5_BATTLE_STATE and BN6_BATTLE_STATE)
    5: dict(t1=0x0203B200, t3=0x0203CA40, t4=0x02036F00, panels=0x0203A100, list=0x0200A410, list_end=0x0200ABA0, n1=16, state=0x02034A90),
    6: dict(t1=0x0203A9B0, t3=0x0203CFE0, t4=0x02036870, panels=0x02039AE0, list=0x02009380, list_end=0x02009AB0, n1=32, state=0x02034880),
}
RNG_XOR = 0x873CA9E5                              # both games' RNG step (BN5_RNG_XOR)
XV_CODE = (0x08B68000, 0x08B80000)                # where the install puts the families' code in BN6 (src/emu/xvirus_recipe.h, XV_CODE to XV_ART)


@functools.lru_cache(maxsize=None)
def souls_code():
    """where the Souls' install puts BN5's code and its data in BN6 (src/emu/soul_room.h's SOULS_CODE to SOULS_CODE_END)"""
    d = c_defines('src/emu/free_space.h', 'src/emu/soul_room.h')
    return d['SOULS_CODE'], d['SOULS_CODE_END']
MEGAMAN_BUSTER = 0x11                             # MegaMan's CurAction as his buster fires, in both
TWIN = 32                                         # the bytes two ROM pointers' data must share to be the same data
NODE = 0x10                                       # the list node before an object (BN6_OBJECT_NODE)
SIZE = {'T1': 0xD8, 'T3': 0xD8, 'T4': 0xC8}       # an object's record, from its node
SPRITE = {'T1': 0x90, 'T3': 0x90, 'T4': 0x80}     # its sprite's state in it (object offsets)
SPRITE_DATA = (0x18, 0x1C, 0x20, 0x24)            # ... the words that point into the sprite's own data
AI_SIZE, COLL_ROOM, PANEL_ROOM, PANELS = 0x100, 0xA8, 0x24, 40
TRACE_VERSION = 5   # (src/dev/xdiff.c's: the player block and the dispatch's types; version 2 traces, with neither, still read)
# MegaMan's side's player block (src/dev/xdiff.c, trace version 5): his battle NaviStats, the word of what was used this
# battle (BN5's Souls, BN6's Crosses), his pending form's 16 bytes, the Souls' state in BN6's engine (BN6_SOULS' 0x80
# bytes, BN5's zero), the Custom screen's element 0xB (UNITE: BN5_CUSTOM_UNITE, BN6_CUSTOM_UNITE)
PLAYER_NAVI, PLAYER_USED, PLAYER_PENDING, NAVI_FORM = 0x00, 0x64, 0x68, 0x2C
PLAYER_SOULS, PLAYER_UNITE = 0x78, 0xF8
NAVI_MOOD = 0x0E                  # his battle NaviStats' mood (BN5's battle mood; BN6's own)
SOULS_SIDE = 0x20                 # BN6_SOULS_SIDE: a side's Souls' state
PENDING_OBJECT = PLAYER_PENDING + 8   # (the pending record's +8: his battle object)
OBJ_NAME_ID, OBJ_ACTION, OBJ_TYPE, OBJ_EFFECT = 0x28, 0x09, 0x01, 0x04
OBJ_CHIP = 0x2A   # the chip an actor uses next, u16 (BN6_T1_CHIP, BN5's the same place): MegaMan's by the spec's folders
ATTACK_CHIP = 0xB4   # ... and the chip his attack uses, u16: his AI data's AttackVars (+0xA0) +0x14 (BN6_ATTACK_CHIP)
# The numbers a chip's use writes into his AttackVars from its record (BN6 0x080126E4, BN5's twin 0x0800FE6A): AI data
# offset -> the record's (its power +0x1A into the damage +8's low 15 bits, its lockout +0x14 into +5, its parameters
# +0x10-+0x13 into +0xC-+0xF); a chip the spec's folders pair under two ids (another chip of the same name in each game,
# its numbers each its own: tools/xdiff.py's folder_chips) is compared by what its use added to its own record's
CHIP_RECORDS = {5: 0x0801E210, 6: 0x08021DA8}   # (BN5_CHIP_RECORDS, BN6's records: 0x2C each)
CHIP_NUMBERS = {0xA3: 0x0C, 0xA5: 0x14, 0xA8: 0x1A, 0xA9: 0x1B, 0xAC: 0x10, 0xAD: 0x11, 0xAE: 0x12, 0xAF: 0x13}
CHIP_FAMILY = 0x0B   # a chip record's attack family: his CurAction as he uses it (+3 its sub-family, AttackVars +3)
FLASH_TYPE = 5   # the first kind's type 5: an arm's flash, its kind at +4 (OBJ_EFFECT)
OBJ_SAVED_ACTION = 0x5D   # a battle object's state word saved as a status (paralysis, confusion) stops it (+0x5C): its CurAction
DISPATCH = {'T1': 0x5C, 'T3': 0xD2, 'T4': 0x91}  # each kind's second-level dispatch type (src/emu/xvirus_recipe.h, XV_T*_DISPATCH)
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


def dispatched(rec, kind, side, game, version, seen):
    """an object's record with its type as the second-level dispatch runs it: where BN6's is its kind's dispatch type, the
    number the trace read from the kind's side table by its slot (TRACE_VERSION's), counted into seen"""
    if version < TRACE_VERSION or game != 6 or rec[NODE + OBJ_TYPE] != DISPATCH[kind]:
        return rec
    seen[0] += 1
    return rec[:NODE + OBJ_TYPE] + bytes([side]) + rec[NODE + OBJ_TYPE + 1:]


def read_trace(path):
    with open(path, 'rb') as f:
        b = f.read()
    if b[:4] != b'XDF1':
        sys.exit(f'{path}: no trace')
    version, game, coll, psize, nm = struct.unpack_from('<5I', b, 4)
    p = 24
    moved = [struct.unpack_from('<3I', b, p + 12 * i) for i in range(nm)]
    p += 12 * nm
    if version not in (2, TRACE_VERSION):
        sys.exit(f'{path}: a trace of version {version}, which this tool does not read (its {TRACE_VERSION}): run the differential again')
    player_size = 0
    if version >= TRACE_VERSION:
        player_size = struct.unpack_from('<I', b, p)[0]
        p += 4
    g, frames, seen = GAMES[game], [], [0]
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
            rec = dispatched(b[p + 4:p + 4 + 0xD8], 'T1', b[p + 3], game, version, seen)
            ai = b[p + 4 + 0xD8:p + 4 + 0x1D8]
            co = b[p + 4 + 0x1D8:p + 4 + 0x1D8 + COLL_ROOM]
            objs.append(Obj('T1', slot, rec, ai if has_ai else None, co[:coll] if has_coll else None, g['t1'] + slot * 0xD8))
            p += 4 + 0x1D8 + COLL_ROOM
        for i in range(n3):
            slot, has_coll = b[p], b[p + 2]
            rec = dispatched(b[p + 4:p + 4 + 0xD8], 'T3', b[p + 3], game, version, seen)
            co = b[p + 4 + 0xD8:p + 4 + 0xD8 + COLL_ROOM]
            objs.append(Obj('T3', slot, rec, None, co[:coll] if has_coll else None, g['t3'] + slot * 0xD8))
            p += 4 + 0xD8 + COLL_ROOM
        for i in range(n4):
            slot = b[p]
            objs.append(Obj('T4', slot, dispatched(b[p + 4:p + 4 + 0xC8], 'T4', b[p + 3], game, version, seen), None, None, g['t4'] + slot * 0xC8))
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
    return dict(game=game, coll=coll, psize=psize, moved=moved, frames=frames, dispatched=seen[0], version=version)


# ---- each game's numbers as BN6's (legit.json's maps) ----

class Canon:
    def __init__(self, legit):
        m = legit['maps']
        num = lambda d: {int(k, 0): int(v, 0) for k, v in d.items() if not k.startswith('_')}
        self.name_id = num(m['name_id']['bn5_to_bn6'])
        self.t3_type = num(m['t3_type']['bn5_to_bn6'])
        self.effect = num(m['effect']['bn5_to_bn6'])
        self.effect_copies = num(m['effect'].get('copies', {}).get('bn6_to_bn5', {}))   # (BN6's copies of BN5's effects its engine has too)
        self.panel_type = num(m['panel_type']['bn5_to_bn6'])
        self.ai_index = num(m['ai_index']['bn5_to_bn6'])
        self.megaman = num(m['name_id']['megaman'])
        self.mm_action = num(m['action']['megaman'])
        # (MegaMan's family by his attack: BN5's number BN6's another where his AttackVars' byte names whose it is)
        self.mm_action_by = [(int(r['bn5'], 0), int(r['ai'], 0), int(r['is'], 0), int(r['bn6'], 0))
                             for r in m['action'].get('megaman_by_attack', [])]
        self.layouts = legit['layouts']
        self.legit = legit
        self.t3_actions = {int(x, 0) for x in m['action'].get('t3_types', [])}
        self.tables = [dict(t, bn5=int(t['bn5'], 0), bn6=int(t['bn6'], 0)) for t in m.get('rom_tables', {}).get('tables', [])]
        self.natives = set()   # (a spec's: enemies both games have, by BN6's id; their attacks' types, their effects)
        self.chips = {}        # (a spec's: BN5's chip id -> BN6's, by its folders, tools/xdiff.py's folder_chips)
        self.souls_on = False  # (a spec's: the Souls installed in BN6's engine, souls6)
        self.native_attacks, self.native_effects = set(), set()
        self.family_effects = {int(x, 0) for x in m['family_effects']['bn6']}
        self.form = num(m['form']['bn5_to_bn6']) if 'form' in m else {}
        # (MegaMan's NameID in a form of BN6's: its base plus the form, legit.json's name_id; his role is MegaMan's whatever his form)
        forms = m['name_id'].get('megaman_forms', {})
        self.form_base = int(forms.get('base', '0'), 0)
        self.form_names = {self.form_base + f for f in range(1, int(forms.get('forms', '0'), 0) + 1)}
        self.souls = m.get('souls', {})
        self.power_attack = num(m['power_attack']['bn5_to_bn6']) if 'power_attack' in m else {}
        self.flash = num(m['flash']['bn5_to_bn6']) if 'flash' in m else {}
        self.song = num(m['sound'].get('bn5_to_bn6', {}))
        self.sprite_pairs = m['sprites'] if 'sprites' in m else None
        self._sprites = None

    def sprite_name(self, game, base):
        """the registry's name of a sprite (its data's start) in this game, or None (maps, sprites: each pair a list and entry of
        each game's sprite lists, resolved from its ROM; a copy the install made of a BN5 sprite by its bytes, BN6's, named as
        that BN5 sprite: sprites' copies)"""
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
            names5 = {(p[0], p[1]): p[4] for p in (self.sprite_pairs or {}).get('pairs', [])}
            for (lst, idx), at in sprite_copies((self.sprite_pairs or {}).get('copies', {}).get('recipes', [])).items():
                if (lst, idx) in names5:
                    self._sprites[6][at] = names5[(lst, idx)]
        return self._sprites[game].get(base)

    def nid(self, game, v):
        return self.name_id.get(v, v) if game == 5 else v

    def is_megaman(self, game, nid):
        return nid in (set(self.megaman.values()) | self.form_names if game == 6 else self.megaman.keys())

    def megaman_role(self, game, nid):
        """MegaMan's role whatever his form: BN6's NameID in a form (0x1AB + the form) read as his own"""
        return next(iter(self.megaman.values())) if game == 6 and nid in self.form_names else nid

    def megaman_nid(self, form5):
        """BN5's MegaMan's NameID (0x180 in any form) as BN6 writes his: 0x1A0, or the form's (0x1AB + the form, the form
        BN6's for BN5's: a Soul behind the proxy)"""
        form = self.form_of(5, form5)
        return self.form_base + form if form and self.form_base else next(iter(self.megaman.values()))

    def action(self, game, megaman, v, ai=None):
        """CurAction as BN6 numbers it: a virus's 0-5 the same, BN5's 6 BN6's 8, 7 on two up; MegaMan's by his map, and by
        his attack where his AI data `ai` names it (maps, action, megaman_by_attack)"""
        if game == 6:
            return v
        if megaman:
            for a5, at, want, a6 in self.mm_action_by:
                if v == a5 and ai is not None and len(ai) > at and ai[at] == want:
                    return a6
            return self.mm_action.get(v, v)
        return v if v < 6 else 8 if v == 6 else v + 2

    def form_of(self, game, v):
        """MegaMan's form as BN6 numbers it (a BN5 Soul by legit.json's map: none until the Souls are installed)"""
        return self.form.get(v, v) if game == 5 else v

    def t3(self, game, v):
        return self.t3_type.get(v, v) if game == 5 else v

    def fx(self, game, v):
        return self.effect.get(v, v) if game == 5 else self.effect_copies.get(v, v)


# ---- roles ----

def identity(game, o, canon):
    """what an object is, in BN6's numbers: an enemy by its id, an attack by its type, an effect by its type, and an
    effect object (the fourth kind's type 0) by its effect id too, so one spawned with the wrong id is another role"""
    if o.kind == 'T1':
        return canon.megaman_role(game, canon.nid(game, o.o16(OBJ_NAME_ID)))
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


def c_defines(*paths):
    """the plain defines of C headers: NAME 0xN, or (OTHER + 0xN), each resolved to a number (one whose OTHER is none of
    them left out)"""
    import re
    raw = {}
    for p in paths:
        with open(os.path.join(ROOT, p), encoding='utf-8') as f:
            for m in re.finditer(r'^#define\s+(\w+)\s+\(?\s*(?:(\w+)\s*\+\s*)?(0x[0-9A-Fa-f]+)u?\s*\)?', f.read(), re.M):
                raw[m.group(1)] = (m.group(2), int(m.group(3), 16))
    out = {}

    def value(name):
        if name not in out:
            base, n = raw[name]
            out[name] = (value(base) if base else 0) + n
        return out[name]
    for name in raw:
        try:
            value(name)
        except KeyError:
            out.pop(name, None)
    return out


@functools.lru_cache(maxsize=None)
def sprite_copies_of(recipe):
    """(BN5's list, number) -> BN6's address of its copy, from a recipe's XvSprite table (tools/xsoul.py's, the Souls' room)"""
    import re
    names = c_defines('src/emu/free_space.h', 'src/emu/soul_room.h')
    with open(os.path.join(ROOT, recipe), encoding='utf-8') as f:
        text = f.read()
    out = {}
    for m in re.finditer(r'\{ (\d+), 0x([0-9A-F]+), \d+, 0x[0-9A-F]+, (\w+) \+ 0x([0-9A-F]+), 0x[0-9A-F]+ \}', text):
        out[(int(m.group(1)), int(m.group(2), 16))] = names[m.group(3)] + int(m.group(4), 16)
    return out


def sprite_copies(recipes):
    out = {}
    for r in recipes:
        out.update(sprite_copies_of(r))
    return out


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
        self.moved = trace['moved'] if self.game == 6 else [m for m in trace.get('code', ()) if XV_CODE[0] <= m[1] < XV_CODE[1] or
                                                             souls_code()[0] <= m[1] < souls_code()[1]]

    def ptr(self, v):
        """a pointer as what it names"""
        if v == self.list:
            return ('list head',)
        if v == self.list_end:
            return ('list end',)
        # (each game's battle state: an enemy's deletion leaves its address in the effect it spawns, its +0x34 and +0x3C, the
        # registers the twins BN5 0x0801343A / BN6 0x0801664E hand the spawner, their 0x080095D4 / 0x0800AE44 leaving it in r1,
        # where the effect's code never reads them)
        if v == GAMES[self.game]['state']:
            return ('battle state',)
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
            if off == OBJ_ACTION and mm and paired_chip_family(canon, oa, ob, a, b):
                a = b   # (a chip the spec's folders pair: each game's own record's family, BN5's 0x25 WideSht BN6's 0x30)
            elif off == OBJ_ACTION and (kind == 'T1' or kind == 'T3' and canon.t3(5, oa.o8(OBJ_TYPE)) in canon.t3_actions):
                a = canon.action(5, mm, a, oa.ai)
            elif off in (OBJ_CHIP, OBJ_CHIP + 1) and kind == 'T1' and mm:
                ca, cb = canon.chips.get(oa.o16(OBJ_CHIP), oa.o16(OBJ_CHIP)), ob.o16(OBJ_CHIP)
                a, b = (ca, cb) if off == OBJ_CHIP else (0, 0)   # (the chip he holds, BN5's by the spec's folders: a halfword)
            elif off == OBJ_SAVED_ACTION and kind == 'T1' and not mm and a:
                a = canon.action(5, mm, a)   # (the CurAction a status saved, to take up again as it ends: numbered alike)
            elif off == OBJ_TYPE and kind == 'T3':
                a = canon.t3(5, a)
            elif off == OBJ_EFFECT and kind == 'T4' and oa.o8(OBJ_TYPE) == 0:
                a, b = canon.fx(5, a), canon.fx(6, b)
            elif off == OBJ_EFFECT and kind == 'T1' and oa.o8(OBJ_TYPE) == FLASH_TYPE and b in canon.flash.values():
                a = canon.flash.get(a, a)   # (a flash of the Souls' kinds: BN5's kind it copies, maps, flash)
            elif off in (OBJ_NAME_ID, OBJ_NAME_ID + 1) and kind == 'T1':
                na, nb = canon.nid(5, oa.o16(OBJ_NAME_ID)), ob.o16(OBJ_NAME_ID)
                if mm and fa.fr.get('player'):   # (his NameID by his form, BN5's read as BN6 writes it in that form)
                    na = canon.megaman_nid(fa.fr['player'][PLAYER_NAVI + NAVI_FORM])
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
    ai5 = soul_ai_read(fa.canon, oa.ai) if megaman else oa.ai
    for w in range(0, AI_SIZE, 4):
        wa, wb = u32(ai5, w), u32(ob.ai, w)
        view = word_view(fa, fb, wa, wb, None)
        if view is not None:
            if view[0] != view[1]:
                out.append(('ai', w, view[0], view[1]))
            continue
        for k in range(4):
            a, b = ai5[w + k], ob.ai[w + k]
            if w + k == AI_INDEX:
                a = fa.canon.ai_index.get(a, a)
            elif megaman and w + k in LOADOUT and a != 0xFF:
                a = fa.canon.power_attack.get(a, a)
            elif megaman and w + k in (ATTACK_CHIP, ATTACK_CHIP + 1):
                ca = fa.canon.chips.get(u16(ai5, ATTACK_CHIP), u16(ai5, ATTACK_CHIP))   # (the chip his attack uses, by the spec's folders)
                a, b = (ca, u16(ob.ai, ATTACK_CHIP)) if w + k == ATTACK_CHIP else (0, 0)
            elif megaman and w + k in CHIP_NUMBERS and fa.canon.chips.get(u16(ai5, ATTACK_CHIP)) == u16(ob.ai, ATTACK_CHIP):
                a, b = chip_number(5, ai5, w + k), chip_number(6, ob.ai, w + k)
            if a != b:
                out.append(('ai', w + k, a, b))
    return out


def paired_chip_family(canon, oa, ob, a, b):
    """whether MegaMan's CurActions are each his chip's record's family, the chip one the spec's folders pair"""
    if oa.ai is None or ob.ai is None:
        return False
    c5, c6 = u16(oa.ai, ATTACK_CHIP), u16(ob.ai, ATTACK_CHIP)
    if canon.chips.get(c5) != c6:
        return False
    return all(rom(g)[CHIP_RECORDS[g] - ROM[0] + 0x2C * c + CHIP_FAMILY] == v for g, c, v in ((5, c5, a), (6, c6, b)))


def chip_number(game, ai, off):
    """a byte of his AttackVars a chip's use took from its record, read as what it adds to the record's: its damage as
    hundredths of the record's power (a charged chip's double, 200), the double bit 0x8000 as it is (a Soul's or a field's
    modifier, BN5's 0x0801045C), the rest as each byte less the record's"""
    rec = CHIP_RECORDS[game] - ROM[0] + 0x2C * u16(ai, ATTACK_CHIP)
    if off in (0xA8, 0xA9):
        dmg, power = u16(ai, 0xA8), u16(rom(game), rec + 0x1A)
        share = (dmg & 0x7FFF) * 100 // power if power else dmg & 0x7FFF
        return ('the damage, % of the record\'s power', share, 'doubled' if dmg & 0x8000 else '') if off == 0xA8 else 0
    return (ai[off] - rom(game)[rec + CHIP_NUMBERS[off]]) & 0xFF


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


def panel_typed(canon, pa):
    """BN5's panel with its types in BN6's numbers (legit.json, maps, panel_type): its type, the type it is drawn as
    and the one-frame override's, and its flags' low nibble, where the type is ORed in"""
    lay, b = canon.layouts['panel'], bytearray(pa)
    for off in lay['types']:
        b[off] = canon.panel_type.get(b[off], b[off])
    f = lay['flags'][0]
    b[f] = b[f] & 0xF0 | canon.panel_type.get(b[f] & 0xF, b[f] & 0xF)
    return bytes(b)


def panel_timer(canon, pa, pb, t, where):
    """BN5's lava or sea timer, where one counts, against BN6's one (legit.json, layouts, panel, timers; the frame
    one runs out, each game's own end, the same): the places it read, each game's, and a difference or None;
    nothing where BN5's both are idle"""
    rule = canon.layouts['panel']['timers']
    idle, b6, out = int(rule['idle'], 0), int(rule['bn6'], 0), tuple(int(x, 0) for x in rule['out'])
    for off in (int(x, 0) for x in rule['bn5']):
        if u16(pa, off) != idle:
            same = u16(pa, off) == u16(pb, b6) or (u16(pa, off), u16(pb, b6)) == out
            diff = None if same else Diff(t, where, 'panel', b6, u16(pa, off), u16(pb, b6))
            return {o + k for o in (int(x, 0) for x in rule['bn5']) for k in (0, 1)}, {b6, b6 + 1}, diff
    return set(), set(), None


def panel_cells(fa, fb, t):
    """the panels through each game's layout (legit.json): a byte outside it zero in both, or a difference"""
    out = []
    segs = fa.canon.layouts['panel']['segments']
    for i in range(PANELS):
        pa, pb, where = panel_typed(fa.canon, fa.fr['panels'][i]), fb.fr['panels'][i], f'panel {i % 8},{i // 8}'
        timed5, timed6, timed = panel_timer(fa.canon, pa, pb, t, where)
        out += [timed] if timed else []
        seen5 = {a0 + k for a0, _, n in segs for k in range(n)} | timed5
        seen6 = {b0 + k for _, b0, n in segs for k in range(n)} | timed6
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


# ---- a Soul's state: BN5's in MegaMan's AI data and its form byte, BN6's engine's in BN6_SOULS (legit.json, maps, souls) ----

def soul_ai(canon):
    """BN5's AI data's places of a Soul's state: {field: (offset, size)}"""
    return {k: (int(v[0], 0), v[1]) for k, v in canon.souls.get('bn5_ai', {}).items()}


def soul_ai_read(canon, ai):
    """BN5's MegaMan's AI data read as BN6's: its Soul's state is BN5's alone there (BN6 keeps it in BN6_SOULS, its own
    bytes at those places being its own), so it reads 0 there and is compared through soul_cells"""
    out = bytearray(ai)
    for off, size in soul_ai(canon).values():
        out[off:off + size] = bytes(size)
    return bytes(out)


def soul_pending(canon):
    """BN5's pending record's places of a Soul's state (ColonelSoul's chip committed at OK): {field: (offset, size)}"""
    return {k: (int(v[0], 0), v[1]) for k, v in canon.souls.get('bn5_pending', {}).items()}


def soul_bn5(canon, field, v, size):
    """a field of BN5's read in BN6's terms: a u16's 0xFFFF none BN6's 0, a chip BN6's id (the spec's folders' pairs)"""
    if size > 1 and v == (1 << 8 * size) - 1:
        return 0
    return canon.chips.get(v, v) if 'chip' in field else v


def soul_view(canon, game, fr, ai):
    """the Soul MegaMan is united with, as BN6_SOULS' side lays it out: {field: value} (BN5's from its form byte and his AI
    data while it is a Soul, all 0 in none; BN6's from its block, side 0)"""
    p = fr.get('player', b'')
    if len(p) < PLAYER_UNITE:
        return None
    fields = canon.souls.get('bn6_side', {})
    if game == 6:
        side = p[PLAYER_SOULS:PLAYER_SOULS + SOULS_SIDE]
        return {k: int.from_bytes(side[int(v[0], 0):int(v[0], 0) + v[1]], 'little') for k, v in fields.items()}
    form = p[PLAYER_NAVI + NAVI_FORM]
    view = {k: 0 for k in fields}
    if form and ai is not None:
        view['soul'] = form
        for k, (off, size) in soul_ai(canon).items():
            view[k] = soul_bn5(canon, k, int.from_bytes(ai[off:off + size], 'little'), size)
    for k, (off, size) in soul_pending(canon).items():   # (his pending record's: ColonelSoul's chip committed, whatever his form)
        view[k] = soul_bn5(canon, k, int.from_bytes(p[PLAYER_PENDING + off:PLAYER_PENDING + off + size], 'little'), size)
    view['used'] = int.from_bytes(p[PLAYER_USED:PLAYER_USED + 4], 'little')
    if 'mood' in view:
        view['mood'] = p[PLAYER_NAVI + NAVI_MOOD] if canon.souls_on else 0   # (BN5's battle mood, which BN6's engine keeps beside its own: soulmood.c)
    return view


def soul_cells(fa, fb, ai5, ai6):
    """A Soul's state in both games (BN5's own places, BN6's engine's block): each field the same, the used word BN5's
    Souls' against the block's, BN6's Crosses' word none (BN5 has no Cross), and the Custom screen's element 0xB (UNITE)
    byte for byte; each a difference of MegaMan's (role 'player')"""
    canon, t = fa.canon, fa.fr['t']
    va, vb = soul_view(canon, 5, fa.fr, ai5), soul_view(canon, 6, fb.fr, ai6)
    if va is None or vb is None:
        return []
    out = [Diff(t, 'player', f'soul {k}', 0, va[k], vb[k]) for k in va if va[k] != vb.get(k)]
    crosses = int.from_bytes(fb.fr['player'][PLAYER_USED:PLAYER_USED + 4], 'little')
    if crosses:
        out.append(Diff(t, 'player', 'crosses used', PLAYER_USED, 0, crosses))
    ua, ub = fa.fr['player'][PLAYER_UNITE:PLAYER_UNITE + 12], fb.fr['player'][PLAYER_UNITE:PLAYER_UNITE + 12]
    out += [Diff(t, 'player', 'unite', k, ua[k], ub[k]) for k in range(len(ua)) if ua[k] != ub[k]]
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
    for off in range(0, min(len(pa), len(pb), PLAYER_SOULS)):
        if PLAYER_USED <= off < PLAYER_USED + 4:
            continue   # (BN5's Souls used, BN6's Crosses: soul_cells)
        if any(PLAYER_PENDING + o <= off < PLAYER_PENDING + o + n for o, n in soul_pending(fa.canon).values()):
            continue   # (BN5's pending record's Soul fields: soul_cells)
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
    # (a song by its number: the music player it plays on is its song table entry's, each game's own: legit.json, maps; BN5's
    # song read as the slot BN6 copied it into where BN6 played that slot, maps, sound, bn5_to_bn6)
    sb = sorted(s for _, s in fb.fr['sounds'])
    song = fa.canon.song
    sa = sorted(song[s] if song.get(s) in sb else s for _, s in fa.fr['sounds'])
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
    mm = [next((o.ai for o in f.fr['objs'] if o.kind == 'T1' and f.canon.is_megaman(f.game, o.o16(OBJ_NAME_ID))), None) for f in (fa, fb)]
    if fb.fr.get('player') and len(fb.fr['player']) >= PLAYER_UNITE:
        out += soul_cells(fa, fb, mm[0], mm[1])
    return out
