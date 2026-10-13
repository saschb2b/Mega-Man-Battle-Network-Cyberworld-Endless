"""xdiff_snap.py: the canonical snapshot of a fight frame for tools/xdiff.py (issue #82): the traces
src/dev/xdiff.c writes, each game's numbers read as BN6's (the engine's own words among them: the alliance
tables' words, an object's slot), objects matched by role, pointers named by what they point at, and the
differences of the two games' frames through each game's layouts (tests/xdiff/legit.json; docs/DEVTOOLS.md,
The differential)."""
import functools
import json
import os
import re
import struct
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(ROOT, '.build', 'xdiff')

# Each game's battle as the trace was read (src/emu/bn5.h, bn6.h)
GAMES = {
    5: dict(t1=0x0203B200, t3=0x0203CA40, t4=0x02036F00, panels=0x0203A100, list=0x0200A410, list_end=0x0200ABA0, n1=16,
            battle=0x02034A90),
    6: dict(t1=0x0203A9B0, t3=0x0203CFE0, t4=0x02036870, panels=0x02039AE0, list=0x02009380, list_end=0x02009AB0, n1=32,
            battle=0x02034880),
}
RNG_XOR = 0x873CA9E5                              # both games' RNG step (BN5_RNG_XOR)
XV_CODE = (0x08B68000, 0x08B80000)                # where the install puts the families' code in BN6 (src/emu/xvirus_recipe.h, XV_CODE to XV_ART)
XN_ROOM = (0x08D90000, 0x08E00000)                # ... and the Navis' code and art (src/emu/xnavi_recipe.h, XN_STUBS to XN_END)
EMU_FREE = 0x08800000                             # below it, BN6's own ROM: a moved Navi's callback's twin (xnavi_install.c)
XC_CODE = (0x08E00400, 0x08F00000)                # ... and the chips' moved code (src/emu/xchips_code.h, FREE_CHIP_MOVED to FREE_TAIL)
XC_ART = 0x08EA0000                                # ... their copies of BN5's sprites from here (src/emu/xchips_code.h, XC_ART)


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
DISPATCH_COPY = 0x3C                               # a virus's AI data's u16 BN6's dispatcher copies its object's +0x3E into
AI_SIZE, COLL_ROOM, PANEL_ROOM, PANELS = 0x100, 0xA8, 0x24, 40
TRACE_VERSION = 7   # (src/dev/xdiff.c's: the state BN6's engine keeps apart, the player block and the dispatch's types; version 2
                    # traces, with none of them, still read)
# MegaMan's side's player block (src/dev/xdiff.c, trace version 5): his battle NaviStats, the word of what was used this
# battle (BN5's Souls, BN6's Crosses), his pending form's 16 bytes, the Souls' state in BN6's engine (BN6_SOULS' 0x80
# bytes, BN5's zero), the Custom screen's element 0xB (UNITE: BN5_CUSTOM_UNITE, BN6_CUSTOM_UNITE)
PLAYER_NAVI, PLAYER_USED, PLAYER_PENDING, NAVI_FORM = 0x00, 0x64, 0x68, 0x2C
PLAYER_SOULS, PLAYER_UNITE = 0x78, 0xF8
NAVI_MOOD = 0x0E                  # his battle NaviStats' mood (BN5's battle mood; BN6's own)
SOULS_SIDE = 0x20                 # BN6_SOULS_SIDE: a side's Souls' state
PENDING_OBJECT = PLAYER_PENDING + 8   # (the pending record's +8: his battle object)
OBJ_NAME_ID, OBJ_ACTION, OBJ_TYPE, OBJ_EFFECT = 0x28, 0x09, 0x01, 0x04
OBJ_CHIP = 0x2A   # the chip a battle object uses next (BN6_T1_CHIP, u16; 0xFFFF none)
FAMILY_FIRST = 0x10   # CurAction from here an attack family's (BN6_ATTACK_FAMILY_FIRST)
OBJ_OVERLAY_KIND = 0x04   # the companion overlay's kind (the first kind's type 0x56, BN6_OVERLAY_*): its first parameter
T1_OVERLAY = 0x56
OVERLAY_KINDS6 = 19   # BN6's own overlay kinds (BN6_OVERLAY_KINDS): the Navis' past them
# an actor's CurActions kept as a status takes it (the status's, the one before it to go back to, the status's again: both
# games' paralysis writes them alike, MegaMan's ready 6 in BN5 and 8 in BN6 at +0x5D; the first kind's +0x5C its state word
# saved as the status takes it, legit.json, maps, action), read as CurAction is
OBJ_ACTIONS_KEPT = (0x5C, 0x5D, 0x5E)
FLASH = 5         # the first kind's type of a muzzle flash, its +4 the flash's number in each game's table (legit.json, maps, flash)
# The numbers a chip's use writes into MegaMan's AttackVars from its record (BN6 0x080126E4, BN5's twin 0x0800FE6A): AI
# data offset -> the record's (its power +0x1A into the damage +8's low 15 bits, its lockout +0x14 into +5, its
# parameters +0x10-+0x13 into +0xC-+0xF); a chip the spec's folders pair under two ids (another chip of the same name in
# each game, its numbers each its own: tools/xdiff.py's folder_chips) is compared by what its use added to its own record's
CHIP_NUMBERS = {0xA3: 0x0C, 0xA5: 0x14, 0xA8: 0x1A, 0xA9: 0x1B, 0xAC: 0x10, 0xAD: 0x11, 0xAE: 0x12, 0xAF: 0x13}
CHIP_FAMILY = 0x0B   # a chip record's attack family: his CurAction as he uses it (+3 its sub-family, AttackVars +3)
DISPATCH = {'T1': 0x5C, 'T3': 0xD2, 'T4': 0x91}  # each kind's second-level dispatch type (src/emu/xvirus_recipe.h, XV_T*_DISPATCH)
AI_INDEX = 0x01


def u16(b, o):
    return struct.unpack_from('<H', b, o)[0]


def u32(b, o):
    return struct.unpack_from('<I', b, o)[0]


# ---- traces (src/dev/xdiff.c) ----

class Obj:
    __slots__ = ('kind', 'slot', 'rec', 'ai', 'coll', 'addr', 'role', 'over', 'leftover', 'moved', 'sprite')

    def __init__(self, kind, slot, rec, ai, coll, addr, moved=False):
        self.kind, self.slot, self.rec, self.ai, self.coll, self.addr, self.role = kind, slot, rec, ai, coll, addr, None
        self.over = {}   # (bytes of its record read otherwise: the renderer's places as the family's, xdiff_masks.py)
        self.leftover = False   # (its sprite state the slot's last occupant's: spawned and no sprite set up since, xdiff_events.py)
        self.moved = moved      # (BN6's: run by the second-level dispatch as its game's moved type)
        self.sprite = None      # (the CRC32 of the bytes of the sprite it last set up, xdiff_events.py; None unknown)

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


SETUP = 0x10   # an event's flag: made while the battle was set up (src/dev/xdiff_events.h, XD_SETUP)


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
    if not is_dispatched(rec, kind, game, version):
        return rec
    seen[0] += 1
    return rec[:NODE + OBJ_TYPE] + bytes([side]) + rec[NODE + OBJ_TYPE + 1:]


def is_dispatched(rec, kind, game, version):
    return version >= TRACE_VERSION and game == 6 and rec[NODE + OBJ_TYPE] == DISPATCH[kind]


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
    player_size = apart_size = 0
    if version >= TRACE_VERSION:
        player_size, apart_size = struct.unpack_from('<2I', b, p)
        p += 8
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
        apart = b[p:p + apart_size]     # (BN6's: the state its engine keeps apart, the sea's room, RAM_SEA; BN5's zeros)
        p += apart_size
        objs = []
        for i in range(n1):
            slot, has_ai, has_coll = b[p], b[p + 1], b[p + 2]
            raw = b[p + 4:p + 4 + 0xD8]
            rec = dispatched(raw, 'T1', b[p + 3], game, version, seen)
            ai = b[p + 4 + 0xD8:p + 4 + 0x1D8]
            co = b[p + 4 + 0x1D8:p + 4 + 0x1D8 + COLL_ROOM]
            objs.append(Obj('T1', slot, rec, ai if has_ai else None, co[:coll] if has_coll else None, g['t1'] + slot * 0xD8,
                            is_dispatched(raw, 'T1', game, version)))
            p += 4 + 0x1D8 + COLL_ROOM
        for i in range(n3):
            slot, has_coll = b[p], b[p + 2]
            raw = b[p + 4:p + 4 + 0xD8]
            rec = dispatched(raw, 'T3', b[p + 3], game, version, seen)
            co = b[p + 4 + 0xD8:p + 4 + 0xD8 + COLL_ROOM]
            objs.append(Obj('T3', slot, rec, None, co[:coll] if has_coll else None, g['t3'] + slot * 0xD8, is_dispatched(raw, 'T3', game, version)))
            p += 4 + 0xD8 + COLL_ROOM
        for i in range(n4):
            slot = b[p]
            raw = b[p + 4:p + 4 + 0xC8]
            objs.append(Obj('T4', slot, dispatched(raw, 'T4', b[p + 3], game, version, seen), None, None, g['t4'] + slot * 0xC8,
                            is_dispatched(raw, 'T4', game, version)))
            p += 4 + 0xC8
        panels = [b[p + PANEL_ROOM * i:p + PANEL_ROOM * i + psize] for i in range(PANELS)]
        p += PANEL_ROOM * PANELS
        order = list(struct.unpack_from(f'<{no}I', b, p))
        p += 4 * no
        sounds = [struct.unpack_from('<2I', b, p + 8 * i) for i in range(ns)]
        p += 8 * ns
        events = [Event(b, p + EVENT_SIZE * i, t) for i in range(nev)]
        p += EVENT_SIZE * nev
        # (the spawns and sprites' set-ups made while the battle was set up, before its fight: told apart, for which objects set
        # up no sprite since they spawned alone, src/dev/xdiff_events.h's XD_SETUP)
        setup = [e for e in events if e.flags & SETUP]
        events = [e for e in events if not e.flags & SETUP]
        frames.append(dict(f=f, t=t, sub=sub, ph=ph, rng=rng, objs=objs, panels=panels, order=order, sounds=sounds, events=events,
                           player=player, setup=setup, apart=apart))
    return dict(game=game, coll=coll, psize=psize, moved=moved, frames=frames, dispatched=seen[0], version=version)


# ---- each game's numbers as BN6's (legit.json's maps) ----

PLACES = {}   # (id of a raw table, kind, type) -> (the table, kept so its id stays its own; its places): read per word


def places_of(raw, kind, t5):
    """the places a raw table of legit.json (by kind, then BN5's type) gives an object of kind `kind` and BN5's type t5"""
    key = (id(raw), kind, t5)
    if key not in PLACES:
        by_type = {int(k, 0): v for k, v in raw.get(kind.lower(), {}).items()}
        PLACES[key] = (raw, frozenset(int(x, 0) for x in by_type.get(t5, [])))
    return PLACES[key][1]


class Canon:
    def __init__(self, legit):
        m = legit['maps']
        num = lambda d: {int(k, 0): int(v, 0) for k, v in d.items() if not k.startswith('_')}
        self.name_id = num(m['name_id']['bn5_to_bn6'])
        self.t3_type = num(m['t3_type']['bn5_to_bn6'])
        self.effect = num(m['effect']['bn5_to_bn6'])
        self.effect.update(chip_effects())   # (and the moved chips' own, their ids from the generated units)
        # (the effect ids a Navi's own code renumbers, each Navi its own numbers; BN5's engine's or another enemy's elsewhere:
        # read as BN6's only where a spec's foes hold one of its ids)
        self.navi_effects = [({int(x, 0) for x in n['ids']}, num(n['bn5_to_bn6'])) for n in m['effect'].get('navis', [])]
        self.foes = set()   # (a spec's: its foes' BN5 ids)
        self.effect_copies = num(m['effect'].get('copies', {}).get('bn6_to_bn5', {}))   # (BN6's copies of BN5's effects its engine has too)
        self.panel_type = num(m['panel_type']['bn5_to_bn6'])
        self.ai_index = num(m['ai_index']['bn5_to_bn6'])
        self.navi_ai_index = num(m['navi_ai_index']['bn5_to_bn6']) if 'navi_ai_index' in m else {}
        self.overlay_kind = num(m['overlay_kind']['bn5_to_bn6']) if 'overlay_kind' in m else {}
        # (bytes of an enemy's AI data, or of an object of a moved type, its code writes from a pointer's low bytes: read as
        # the pointer they came from, byte k of the word at `pointer` in the same record)
        self.pointer_bytes, self.obj_pointer_bytes = {}, {}
        for rule in legit['layouts'].get('pointer_bytes', {}).get('rules', []):
            spans = {int(b, 0): (int(rule['pointer'], 0), k) for k, b in enumerate(rule['bytes'])}
            if rule['part'] == 'ai':
                for nid in rule['ids']:
                    self.pointer_bytes.setdefault(int(nid, 0), {}).update(spans)
            else:
                self.obj_pointer_bytes.setdefault((rule['kind'], int(rule['type'], 0)), {}).update(spans)
        self.megaman = num(m['name_id']['megaman'])
        self.mm_action = num(m['action']['megaman'])
        self.attack_families = num(m['action'].get('families', {}))   # (BN5's attack families BN6 numbers otherwise)
        self.players_apart = {int(x, 0) for x in m['sound'].get('players_apart', [])}
        # (MegaMan's family by his attack: BN5's number BN6's another where his AttackVars' byte names whose it is)
        self.mm_action_by = [(int(r['bn5'], 0), int(r['ai'], 0), int(r['is'], 0), int(r['bn6'], 0))
                             for r in m['action'].get('megaman_by_attack', [])]
        self.layouts = legit['layouts']
        self.legit = legit
        self.t3_actions = {int(x, 0) for x in m['action'].get('t3_types', [])}
        self._legit_t3_actions, self.effect_bytes = set(self.t3_actions), set()
        self.tables = [dict(t, bn5=int(t['bn5'], 0), bn6=int(t['bn6'], 0)) for t in m.get('rom_tables', {}).get('tables', [])]
        self.natives = set()   # (a spec's: enemies both games have, by BN6's id; their attacks' types, their effects)
        self.megaman_types = set()   # (a spec's: its chip's objects ("T4:0x3a") a hook saw no spawn of, MegaMan's)
        self.chips = {}        # (a spec's: BN5's chip id -> BN6's, by its folders, tools/xdiff.py's folder_chips)
        self.souls_on = False  # (a spec's: the Souls installed in BN6's engine, souls6)
        self.copies_on = False  # (a spec's: effects read as BN5's by their copies, maps, effect, copies: the Souls', effect_copies)
        self.native_attacks, self.native_effects = set(), set()
        self.family_effects = {int(x, 0) for x in m['family_effects']['bn6']}
        self.form = num(m['form']['bn5_to_bn6']) if 'form' in m else {}
        # (MegaMan's NameID in a form of BN6's: its base plus the form, legit.json's name_id; his role is MegaMan's whatever his form)
        forms = m['name_id'].get('megaman_forms', {})
        self.form_base = int(forms.get('base', '0'), 0)
        self.form_names = {self.form_base + f for f in range(1, int(forms.get('forms', '0'), 0) + 1)}
        self.souls = m.get('souls', {})
        # (BN5's chip ids as BN6's: the registry's rows, legit.json's maps, chip_id; the places they are kept, an AI data's and
        # an object's of a type of the third or fourth kind)
        self.chip_rows = registry_rows(m['chip_id']['registry']) if 'chip_id' in m else {}
        self.chip_id = {a: r[0] for a, r in self.chip_rows.items()}
        places = m['chip_id'].get('places', {}) if 'chip_id' in m else {}
        self.chip_places = {'ai': tuple(int(x, 0) for x in places.get('ai', []))}
        self.chip_places_raw = {k: v for k, v in places.items() if k in ('t3', 't4')}
        self.chip_subs_raw = m['chip_id'].get('subs', {}) if 'chip_id' in m else {}
        # (routines whose addresses an object keeps, each game's twins: legit.json's pairs, maps, code_twins)
        self.code_pairs = {(int(a, 0), int(b, 0)) for a, b, *_ in m.get('code_twins', {}).get('pairs', [])}
        self.flash = num(m['flash']['bn5_to_bn6']) if 'flash' in m else {}
        self.power_attack = num(m['power_attack']['bn5_to_bn6']) if 'power_attack' in m else {}
        self.flash = num(m['flash']['bn5_to_bn6']) if 'flash' in m else {}
        self.sprite_pairs = m['sprites'] if 'sprites' in m else None
        self._sprites = None
        self.sprite_bytes = False   # (a spec's "sprites": "bytes": a sprite of no pair compared by its bytes, both games' in their ROMs)
        self._sprite_sha = {5: None, 6: None}
        self.art = {}               # (the spec's families' copied sprites: BN6's list word -> BN5's list and number, select())
        self.sound, self.code_twins = {}, {}
        # (the moved families' numbers, tools/xrecipe.py's: every family's ids and AI index, the spec's families' effects and
        # sounds, select() below)
        self.families, self._legit_effects, self._legit_family = [], dict(self.effect), set(self.family_effects)
        path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'tests', 'xdiff', 'families.json')
        if os.path.exists(path):
            with open(path) as f:
                self.families = json.load(f)['families']
        for fam in self.families:
            for a, b in zip(fam['ids5'], fam['ids6']):
                self.name_id.setdefault(int(a, 0), int(b, 0))
            self.ai_index.setdefault(fam['ai5'], fam['ai6'])

    def select(self, foes5):
        """the maps of a spec whose foes are these BN5 ids: its families' effects (BN5's ids as BN6's, and BN6's the family's
        own) and its families' sounds copied or by another number, beside legit.json's"""
        self.effect, self.family_effects, self.sound, self.code_twins = dict(self._legit_effects), set(self._legit_family), {}, {}
        self.effect_bytes, self.t3_actions = set(), set(self._legit_t3_actions)
        self.art = {}
        for fam in self.families:
            if not {int(x, 0) for x in fam['ids5']} & set(foes5):
                continue
            for a, b in fam['effects'].items():
                self.effect[int(a, 0)] = int(b, 0)
                self.family_effects.add(int(b, 0))
            for a, b in fam['sounds'].items():
                self.sound[int(a, 0)] = int(b, 0)
            for a, b in fam.get('code_twins', {}).items():
                self.code_twins[int(a, 0)] = int(b, 0)
            self.effect_bytes |= {int(x, 0) for x in fam.get('effect_bytes', [])}
            self.t3_actions |= {int(x, 0) for x in fam.get('t3_actions', [])}
        for fam in self.families:
            # (every family's copies, a spec's or not: a type one moved draws them for all)
            for a, (lst, idx) in fam.get('art', {}).items():
                self.art[int(a, 0)] = (int(lst), int(idx, 0))

    def snd(self, game, v):
        """a song as BN6 numbers it: a family's sound BN6 has under another number, or copied into a slot of its own"""
        return self.sound.get(v, v) if game == 5 else v

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

    def sprite_sha(self, game, base):
        """a sprite by its bytes (its list word `base`): one of either game's lists in its ROM, uncompressed, or one a family
        copied into BN6's (families.json's art: BN5's sprite as the installer copies it, decompressed); the first 12 hex
        digits of their SHA-1 as tools/xrecipe.py's sprite() reads them (its twins' test), else None (compressed in its
        game, read from RAM: compared as before, its frames from its own data). Two games' sprites read the same here
        where they are the same picture, whatever their numbers (the chips' track's, with the families' copies)"""
        import hashlib
        import xrecipe as X
        if self._sprite_sha[game] is None:
            data, lists = rom(game), int(self.sprite_pairs['bn5_lists' if game == 5 else 'bn6_lists'], 0) if self.sprite_pairs else 0
            words, out = {}, {}
            for lst in range(8 if lists else 0):
                at = u32(data, lists - ROM[0] + 4 * lst)
                for k in range(512):
                    p = u32(data, at - ROM[0] + 4 * k) if ROM[0] <= at + 4 * k < ROM[0] + len(data) - 4 else 0
                    if not ROM[0] <= p & 0x7FFFFFFF < ROM[0] + len(data):
                        break
                    if not p & 0x80000000:
                        words.setdefault(p, None)
            for p in words:
                o = p - ROM[0]
                n = X.sprite_len(data[o:o + 0x40000], min(0x40000, len(data) - o))
                if n:
                    out[p] = hashlib.sha1(data[o:o + n]).hexdigest()[:12]
            self._sprite_sha[game] = out
        if game == 6 and base in self.art:
            lst, idx = self.art[base]
            r5 = X.T.Rom(rom(5), 'BN5')
            _, b = X.sprite(r5, int(self.sprite_pairs['bn5_lists'], 0), lst, idx)
            return hashlib.sha1(b).hexdigest()[:12] if b else None
        return self._sprite_sha[game].get(base)

    def record(self, game, chip):
        """a chip's record as each game's ROM has it (BN6's own ids; None for one of the registry's own ids, made at runtime)"""
        if game == 6 and own_id(chip):
            return None
        if not 0 < chip < 0x200:
            return None
        at = CHIP_RECORDS[game] - ROM[0] + 0x2C * chip
        return rom(game)[at:at + 0x2C]

    def behaviour6(self, chip6):
        """(family, subfamily) BN6's engine runs BN6's chip chip6 by: its record's, or for one of the registry's own ids the
        twin's family with the row's subfamily or the twin's (src/core/xchips_record.c, behaviour), or the moved code's
        (src/core/xchips_moved.c, where its code moved), else the inert family"""
        rec = self.record(6, chip6)
        if rec is not None:
            return rec[0x0B], rec[0x0C]
        row = next((r for r in self.chip_rows.values() if r[0] == chip6), None)
        moved = self.moved_chips().get(chip6)
        if moved:
            return moved
        if row and row[2]:
            twin = self.record(6, row[2])
            return twin[0x0B], row[3] if row[3] != 0xFF else twin[0x0C]
        return 0x1C, 0x03

    def moved_chips(self):
        """the chips whose code moved (src/core/xchips_moved.c: BN6's id, its family and subfamily there), {} before C3"""
        if not hasattr(self, '_moved'):
            self._moved = moved_rows()
        return self._moved

    def chip_action(self, ai5, v):
        """an object's CurAction v (BN5's) as BN6's where it is the family of the chip it uses (its AttackVars +0x14: a
        virus's, the chip writer's, BN5 0x0800FE7C / BN6 0x080126F6, which returns the chip record's +0x0B for setAttack0;
        MegaMan's, his chip use's) by BN5's record: BN6's family of BN6's chip (behaviour6: its record's, the moved code's,
        or its twin's; the inert family 0x1C of a registry id whose code BN6 lacks); else None (its own state, by the map)"""
        if ai5 is None or v < 0x10 or not self.chip_id:
            return None
        chip5 = u16(ai5, 0xB4)
        rec5 = self.record(5, chip5)
        if rec5 is None or rec5[0x0B] != v:
            return None
        return self.behaviour6(self.chip(5, chip5))[0]

    def chip_object(self, kind, t5):
        """the places (u16) an object of BN5's type t5 of this kind keeps a chip id at (legit.json, maps, chip_id, places)"""
        return places_of(self.chip_places_raw, kind, t5)

    def chip_sub(self, kind, t5):
        """the places (a byte each) an object of BN5's type t5 of this kind keeps its chip's subfamily at (legit.json, maps,
        chip_id, subs)"""
        return places_of(self.chip_subs_raw, kind, t5)

    def paired(self, c5, c6):
        """whether the spec's folders pair BN5's chip c5 with BN6's c6, another chip of the same name in each game (each its
        own record and numbers): BN6's own chip, not one of the registry's own ids (whose record is BN5's)"""
        return self.chips.get(c5) == c6 and not own_id(c6)

    def mm_attack_action(self, v, ai):
        """MegaMan's CurAction v (BN5's) as BN6's by his attack where his AI data names it (maps, action, megaman_by_attack),
        else None"""
        for a5, at, want, a6 in self.mm_action_by:
            if v == a5 and ai is not None and len(ai) > at and ai[at] == want:
                return a6
        return None

    def chip(self, game, v):
        """a chip id as BN6 numbers it: BN5's by the spec's folders where they pair it with another id (tools/xdiff.py's
        folder_chips: a chip of the same name each game's own), else by the registry (legit.json, maps, chip_id), 0xFFFF
        (none) as it is"""
        if game != 5 or v == 0xFFFF:
            return v
        return self.chips[v] if v in self.chips else self.chip_id.get(v, v)

    def copy_of(self, base):
        """BN5's sprite (its list word) a BN6 list word names where it is a moved chip's copy of it (src/emu/xchips_bn5.c's
        XvSprite lists: BN5's list and number copied to a place of XC_ART's), else None"""
        if not hasattr(self, '_copies'):
            self._copies = {}
            path = os.path.join(ROOT, 'src', 'emu', 'xchips_bn5.c')
            text = open(path, encoding='utf-8').read() if os.path.exists(path) else ''
            lists = int(self.sprite_pairs['bn5_lists'], 0) if self.sprite_pairs else 0
            for lst, idx, off in re.findall(r'\{ (\d+), 0x([0-9A-F]+), \d+, 0x[0-9A-F]+, XC_ART \+ 0x([0-9A-F]+), 0x[0-9A-F]+ \}', text):
                if lists:
                    entry = u32(rom(5), u32(rom(5), lists - ROM[0] + 4 * int(lst)) - ROM[0] + 4 * int(idx, 16)) & 0x7FFFFFFF
                    self._copies[XC_ART + int(off, 16)] = entry
        return self._copies.get(base & 0x7FFFFFFF)

    def nid(self, game, v):
        return self.name_id.get(v, v) if game == 5 else v

    def is_navi(self, game, nid):
        """a Navi's NameID in its game (BN5's 0x100-0x17F and the Dark Soul's 0x18D), whose AI index counts apart from the
        viruses'"""
        return game == 5 and (0x100 <= nid < 0x180 or nid == 0x18D)

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
        """CurAction as BN6 numbers it: a virus's 0-5 the same, BN5's 6 BN6's 8, 7 on two up, an attack family's (from
        0x10) by the families' map (a family BN6 runs as its own number the same); MegaMan's by his map, and by his
        attack where his AI data `ai` names it (maps, action, megaman_by_attack)"""
        if game == 6:
            return v
        if megaman:
            for a5, at, want, a6 in self.mm_action_by:
                if v == a5 and ai is not None and len(ai) > at and ai[at] == want:
                    return a6
            return self.mm_action.get(v, v)
        if v >= FAMILY_FIRST:
            return self.attack_families.get(v, v)
        return v if v < 6 else v + 2

    def form_of(self, game, v):
        """MegaMan's form as BN6 numbers it (a BN5 Soul by legit.json's map: none until the Souls are installed)"""
        return self.form.get(v, v) if game == 5 else v

    def t3(self, game, v):
        return self.t3_type.get(v, v) if game == 5 else v

    def foe_effects(self):
        """the BN6 effect ids the spec's foes' own code draws (their maps' targets): art an install chose"""
        return {v for ids, effects in self.navi_effects if ids & self.foes for v in effects.values()}

    def fx(self, game, v):
        # (in a spec whose BN6 code spawns the tracks' copies of BN5's effects where BN6's engine spawns its own of the same
        # number, the Souls' or the obstacle strike's: each copy read as BN5's, BN5's as itself: legit.json, maps, effect, copies)
        if game != 5:
            return self.effect_copies.get(v, v) if self.copies_on else v
        if self.copies_on and v in self.effect_copies.values():
            return v
        for ids, effects in self.navi_effects:
            if ids & self.foes and v in effects:
                return effects[v]
        return self.effect.get(v, v)


# BN6's chip records (RomLayout.chip_data) and BN5's (XRomLayout's), 0x2C bytes a chip, a record's icon element (+6) and BN6's
# hit flag by it (0x080129E4, ORed into a chip's attack element as it is used, 0x08012722); BN5's icon elements as BN6's
# (src/core/xchips_record.c, xrec_icon_element)
CHIP_RECORDS = {5: 0x0801E210, 6: 0x08021DA8}
ICON_FLAGS = 0x080129E4
ICON_BN5_TO_BN6 = (0, 1, 2, 3, 10, 4, 5, 10, 6, 7, 8, 9, 10, 11)


def own_id(v):
    """an id the registry gives a chip of BN5's own (src/core/xchips_registry.c, xreg_free_id): BN6's blank records and 411 on"""
    return 203 <= v <= 220 or 291 <= v <= 300 or v == 314 or 350 <= v <= 367 or 411 <= v < 512


def icon_flag(canon, chip6):
    """the hit flag BN6's chip use ORs into a chip's attack element (AttackVars +2) for BN6's chip chip6: by its record's icon
    element, BN6's own record's, or for a chip of the registry's own id its BN5 record's as BN6's"""
    if own_id(chip6):
        bn5 = next((k for k, v in canon.chip_id.items() if v == chip6), None)
        if bn5 is None:
            return 0
        e5 = rom(5)[CHIP_RECORDS[5] - ROM[0] + 0x2C * bn5 + 6]
        icon = ICON_BN5_TO_BN6[e5] if e5 < len(ICON_BN5_TO_BN6) else 10
    elif chip6 < 411:
        icon = rom(6)[CHIP_RECORDS[6] - ROM[0] + 0x2C * chip6 + 6]
    else:
        return 0
    return rom(6)[ICON_FLAGS - ROM[0] + icon] if icon < 16 else 0


def chip_effects():
    """the effects BN5's moved chips' code draws under ids of the chips' own (src/emu/xchips_bn5.c, written by
    tools/xchiprecipe.py: each unit's XvId list, BN5's id and BN6's), or of another track's its immediates name (the Navis'):
    {BN5's: BN6's}"""
    path = os.path.join(ROOT, 'src', 'emu', 'xchips_bn5.c')
    if not os.path.exists(path):
        return {}
    with open(path, encoding='utf-8') as f:
        text = f.read()
    out = {}
    for body in re.findall(r'static const XvId \w+_effects\[\] = \{(.*?)\};', text):
        for a, b in re.findall(r'\{ (0x[0-9A-F]+), (0x[0-9A-F]+) \}', body):
            out[int(a, 16)] = int(b, 16)
    # (and those a unit's immediate or table byte names by BN6's own id of the same sprite, or another track's: its
    # comment's pair, or its run's)
    for a, b in re.findall(r'its effect:? (0x[0-9a-f]+) in BN5, (0x[0-9a-f]+) in BN6', text):
        out.setdefault(int(a, 16), int(b, 16))
    for a0, a1, b0 in re.findall(r'its effects (0x[0-9a-f]+)-(0x[0-9a-f]+) in BN5, (0x[0-9a-f]+)-0x[0-9a-f]+ in BN6', text):
        for k in range(int(a1, 16) - int(a0, 16) + 1):
            out.setdefault(int(a0, 16) + k, int(b0, 16) + k)
    return out


def moved_rows():
    """the moved chips' table (src/core/xchips_moved.c, written by tools/xchiprecipe.py): {BN6's id: (family, subfamily)}"""
    path = os.path.join(ROOT, 'src', 'core', 'xchips_moved.c')
    if not os.path.exists(path):
        return {}
    with open(path, encoding='utf-8') as f:
        text = f.read()
    return {int(c): (int(f_, 16), int(s_, 16)) for c, f_, s_ in re.findall(r'^\s*\{\s*(\d+),\s*(0x[0-9A-Fa-f]+),\s*(0x[0-9A-Fa-f]+)', text, re.M)}


def registry_rows(rel):
    """the registry's rows (src/core/xchips_registry.c: BN5's id, BN6's, how it comes in, its twin and subfamily where it has
    them, numbers alone), read from the source the game is built from: {BN5's id: (BN6's id, how, twin, sub)}"""
    import re
    with open(os.path.join(ROOT, rel), encoding='utf-8') as f:
        text = f.read()
    rows = {}
    for a, b, how, twin, sub in re.findall(r'^\s*\{\s*(\d+),\s*(\d+),\s*XREG_(\w+)(?:,\s*(\d+),\s*(0x[0-9A-Fa-f]+))?', text, re.M):
        rows[int(a)] = (int(b), how, int(twin) if twin else 0, int(sub, 16) if sub else 0xFF)
    return rows


def registry_ids(rel):
    """BN5's chip ids as BN6's: the registry's rows"""
    return {a: r[0] for a, r in registry_rows(rel).items()}


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
    trace['first'] = {}   # (each role's first frame, its timer: a pointer to an object newer than its holder is a stale one's)
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
                trace['first'].setdefault(o.role, fr['t'])
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
BATTLE_STATE_SPAN = 0x100   # the battle state's bytes a pointer into it is read by
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
    return 0x02000000 <= v < 0x02040000 or 0x03000000 <= v < 0x03008000 or ROM[0] <= v < ROM[1]


assert RAM == ((0x02000000, 0x02040000), (0x03000000, 0x03008000))   # (is_ptr's ranges, written out: it runs per word)


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
        # (the battle's state, BN5_BATTLE_STATE / BN6_BATTLE_STATE: a pointer into it by its offset, as its decoys' slots at
        # +0x94 and +0x98 are named in both, BN5 0x0800CEE6 / BN6 0x0800EC66, the same code)
        self.spans.append((g['battle'], g['battle'] + BATTLE_STATE_SPAN, 'battle', 'state', 0))
        # (an object let go this frame, its slot named: its pointers still point there)
        self.slots = [(kind, g[kind.lower()] - NODE, SIZE[kind], n) for kind, n in (('T1', g['n1']), ('T3', 32), ('T4', 32))]
        self.list, self.list_end = g['list'], g['list_end']
        self.battle = g['battle']   # (BattleState, BN5_BATTLE_STATE / BN6_BATTLE_STATE: a pointer to it as such)
        self.by_role = {o.role: o for o in fr['objs']}
        self.moved = trace['moved'] if self.game == 6 else [m for m in trace.get('code', ())
                                                            if XV_CODE[0] <= m[1] < XV_CODE[1] or XN_ROOM[0] <= m[1] < XN_ROOM[1]
                                                            or XC_CODE[0] <= m[1] < XC_CODE[1] or m[1] < EMU_FREE
                                                            or souls_code()[0] <= m[1] < souls_code()[1]]

    def ptr(self, v):
        """a pointer as what it names"""
        if v == self.list:
            return ('list head',)
        if v == self.list_end:
            return ('list end',)
        if v == self.battle:
            # (the battle's state: BN6's 0x0800AE44 leaves it in r1, the toolkit's +0x18, and the generic delete's spawn of
            # its object after it, the fourth kind's 0x11, BN6 0x0801665C-0x08016666, keeps r1 at +0x34 and +0x3C: BN5's
            # 0x02034A90 and BN6's 0x02034880 there, each its own BattleState)
            return ('battle state',)
        for lo, hi, role, part, base in self.spans:
            if lo <= v < hi:
                off = v - lo + base
                if part == 'coll':
                    off = coll_off(self.canon, self.game, off)
                if part == 'panel':
                    return ('panel', off // self.trace['psize'], panel_off(self.canon, self.game, off % self.trace['psize']))
                if part == 'state':
                    return ('battle state', off)
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
TWO = {}


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
        # (a BN5 word two places give otherwise, where the two differ by BN6's bit 0x80 alone: either is its reading)
        TWO.update({w5: frozenset(w6s) for w5, w6s in pairs.items() if len(w6s) == 2 and {w ^ 0x80 for w in w6s} == w6s})
        for w5 in pairs:
            TABLE_BITS[0] |= w5
    return ALLIANCE


TABLE_BITS = [0]   # every bit some BN5 table word has (with the time stop's): the others in a word ORed of them are the hit pass's own


@functools.lru_cache(maxsize=None)
def alliance_or(w5):
    """a word ORed from other objects' +0x30 (what hit or touched an object, legit.json, maps, alliance) as BN6's:
    the OR of the BN6 words of the BN5 words it is made of, where every way of making it of the table's words gives
    the same, and the bits no table word has as they are (the hit pass's own flags, 0x1, 0x40 and 0x20000, the same in
    both: alliance_masks' flag_bits); None where none does or two give otherwise (then the word is compared as it is)"""
    own, w5 = w5 & ~TABLE_BITS[0], w5 & TABLE_BITS[0]
    parts = [k for k in ALLIANCE if k and k & ~w5 == 0]
    if len(parts) > 16:
        return None
    made = {(0, 0)}
    for k in parts:
        made |= {(a | k, b | ALLIANCE[k]) for a, b in made}
    got = {b for a, b in made if a == w5}
    return got.pop() | own if len(got) == 1 else None


def alliance_words(canon, ca, cb=None):
    """BN5's collision data with its alliance words read as BN6's (legit.json, layouts, collision, alliance_masks:
    the table's words and the words ORed of them, at BN6's offsets); a table word two of its places give otherwise (BN6's
    with bit 0x80 at one and without it at the other) read as BN6's where BN6's (cb) is either"""
    t, out = canon.layouts['collision']['alliance_masks'], bytearray(ca)
    alliance_map(canon)
    for key, read in (('table_words', ALLIANCE.get), ('or_words', alliance_or)):
        for x in t[key]:
            o5 = bn5_coll_off(canon, int(x, 0))
            w6 = read(u32(ca, o5))
            if w6 is None and key == 'table_words' and cb is not None and u32(cb, int(x, 0)) in TWO.get(u32(ca, o5), ()):
                w6 = u32(cb, int(x, 0))
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
        # (a routine of BN5's engine a family's code names as a callback, and its twin in BN6, each recipe's pair: legit.json,
        # maps, code_twins)
        if fa.canon.code_twins.get(wa) == wb and wb:
            return ('engine routine', wa), ('engine routine', wa)
        # (and a routine's address an object keeps, each game's twin: legit.json's pairs)
        if (wa, wb) in fa.canon.code_pairs:
            return ('code twin', wa), ('code twin', wa)
        va, vb = fa.ptr(wa), fb.ptr(wb)
        if va[0] == 'raw' and vb[0] == 'raw' and table_view(fa.canon, wa, wb):
            return table_view(fa.canon, wa, wb)
        if va[0] == 'raw' and vb[0] == 'raw':
            if ROM[0] <= wa < ROM[1] and ROM[0] <= wb < ROM[1] and twin(wa, wb):
                return ('twin data',), ('twin data',)
            return None
        return va, vb
    return None


def stale(f, holder, view):
    """a pointer's view (Frame.ptr) as a stale one's where it is one: to a slot let go, or to an object newer than the
    object holding it (the slot's occupant as the pointer was written gone, another there since): ('stale', its slot,
    its part, its offset)"""
    if not isinstance(view, tuple) or len(view) != 3 or not isinstance(view[0], str):
        return view
    if view[0].endswith('(let go)'):
        return ('stale', view[0].split(' (')[0], view[1], view[2])
    o = next((x for x in f.fr['objs'] if role_name(x.role) == view[0]), None)
    first = f.trace.get('first', {})
    if o is not None and holder.role in first and first.get(o.role, 0) > first[holder.role]:
        return ('stale', f'{o.kind} slot {o.slot}', view[1], view[2])
    return view


def art_chosen(canon, oa, ob):
    """whether an object draws art an install chose, compared by its sprite's bytes (xdiff_events.py, Obj.sprite): a moved
    type's (BN6's run by the dispatch), a moved foe's own body, an effect of a family's or a foe's own ids, a companion overlay
    of a Navi's own kind; the engine's and MegaMan's own draw each game's art, apart by design"""
    t = ob.o8(OBJ_TYPE)
    if ob.moved:
        return True
    if ob.kind == 'T1' and t == 0:
        # (a foe both games have, a native, draws each game's own art: BN6's Mettaur is BN6's)
        return oa.o16(OBJ_NAME_ID) in canon.foes and ob.o16(OBJ_NAME_ID) not in canon.natives
    if ob.kind == 'T4' and t == 0:
        return ob.o8(OBJ_EFFECT) in canon.family_effects | canon.foe_effects()
    return ob.kind == 'T1' and t == T1_OVERLAY and ob.o8(OBJ_OVERLAY_KIND) >= OVERLAY_KINDS6


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
            # (the sprite itself, its data pointer: by the registry where either game's is one of its pairs; art an install
            # chose by its bytes below, whichever pair its game's numbers name: the Dark Soul's body BN5's MegaMan's copied)
            na, nb = canon.sprite_name(5, base_a - 4), canon.sprite_name(6, base_b - 4)
            # (a sprite BN6 draws from a moved chip's copy of BN5's: as BN5's sprite it copies, the same picture)
            copy = canon.copy_of(base_b - 4) if w - sp == 0x18 else None
            if copy is not None:
                rel = ('sprite', na or f'{base_a - 4:#x}'), ('sprite', canon.sprite_name(5, copy) or f'{copy:#x}')
            elif w - sp == 0x18 and (na or nb) and not art_chosen(canon, oa, ob):
                rel = ('sprite', na or f'{base_a - 4:#x}'), ('sprite', nb or f'{base_b - 4:#x}')
            elif w - sp == 0x18 and canon.sprite_bytes and not (kind == 'T4' and oa.o8(OBJ_TYPE) == 0):
                # (a sprite of no pair, both games' in their ROMs or a family's copy: the same picture where the same bytes;
                # not an effect object's, the fourth kind's 0, whose picture is its effect id's, compared as the id)
                ha, hb = canon.sprite_sha(5, base_a - 4), canon.sprite_sha(6, base_b - 4)
                if ha and hb:
                    rel = ('sprite', f'bytes {ha}'), ('sprite', f'bytes {hb}')
        if w - sp == 0x18 and oa.sprite and ob.sprite and oa.sprite != ob.sprite and art_chosen(canon, oa, ob):
            # (the sprite itself, by its bytes: the art of what an install moved or chose, which no number of the two games' says)
            out.append(('obj', w - NODE, ('sprite bytes', f'{oa.sprite:08x}'), ('sprite bytes', f'{ob.sprite:08x}')))
        view = word_view(fa, fb, wa, wb, rel)
        if view is not None and view[0] != view[1] and rel is None:
            view = (stale(fa, oa, view[0]), stale(fb, ob, view[1]))
        if view is not None:
            if view[0] != view[1]:
                out.append(('obj', w - NODE, view[0], view[1]))
            continue
        for k in range(4):
            off = w + k - NODE
            a, b = ra[w + k], rb[w + k]
            # (CurAction numbered as the state tables number it: the first kind's type 0 alone, MegaMan and the enemies, whose
            # states the tables hold; its other types run routines of their own, a moved one BN5's numbers as they are. A
            # virus's chip's family, the chip writer's, as BN6's of its chip: chip_action)
            if off == OBJ_ACTION and mm and paired_chip_family(canon, oa, ob, a, b):
                a = b   # (a chip the spec's folders pair: each game's own record's family, BN5's 0x25 WideSht BN6's 0x30)
            elif off == OBJ_ACTION and mm and canon.mm_attack_action(a, oa.ai) is not None:
                a = canon.mm_attack_action(a, oa.ai)   # (his attack's family as BN6 runs it: a Soul's, before a chip's)
            elif off == OBJ_ACTION and kind == 'T1' and oa.o8(OBJ_TYPE) == 0 and canon.chip_action(oa.ai, a) is not None:
                a = canon.chip_action(oa.ai, a)
            elif off == OBJ_ACTION and (kind == 'T1' and oa.o8(OBJ_TYPE) == 0 or kind == 'T3' and canon.t3(5, oa.o8(OBJ_TYPE)) in canon.t3_actions):
                a = canon.action(5, mm, a, oa.ai)
            elif off in OBJ_ACTIONS_KEPT and kind == 'T1' and oa.o8(OBJ_TYPE) == 0:
                a = canon.action(5, mm, a)
            elif off == OBJ_TYPE and kind == 'T3':
                a = canon.t3(5, a)
            elif off == OBJ_EFFECT and kind == 'T4' and oa.o8(OBJ_TYPE) == 0:
                a, b = canon.fx(5, a), canon.fx(6, b)
            elif off == OBJ_EFFECT and kind == 'T1' and oa.o8(OBJ_TYPE) == FLASH and not oa.o16(OBJ_NAME_ID):
                # (BN6's own flash by BN6's numbers; BN5's moved image object's, the moved objects' copy run by the dispatch
                # with BN5's table, BN5's)
                a = a if ob.moved else canon.flash.get(a, a)
            elif kind in ('T3', 'T4') and (off - 1 in canon.chip_object(kind, oa.o8(OBJ_TYPE)) or off in canon.chip_object(kind, oa.o8(OBJ_TYPE))):
                at = off - 1 if off - 1 in canon.chip_object(kind, oa.o8(OBJ_TYPE)) else off
                ca, cb = canon.chip(5, oa.o16(at)), ob.o16(at)
                a, b = (ca & 0xFF, cb & 0xFF) if off == at else (ca >> 8, cb >> 8)
            elif kind in ('T3', 'T4') and off in canon.chip_sub(kind, oa.o8(OBJ_TYPE)) and canon.chip_object(kind, oa.o8(OBJ_TYPE)):
                # (its chip's subfamily beside the chip: BN5's record's as BN6's chip's, behaviour6)
                chip5 = oa.o16(min(canon.chip_object(kind, oa.o8(OBJ_TYPE))))
                rec5 = canon.record(5, chip5)
                if rec5 is not None and a == rec5[0x0C]:
                    a = canon.behaviour6(canon.chip(5, chip5))[1]
            elif off in (OBJ_CHIP, OBJ_CHIP + 1) and kind == 'T1':
                ca, cb = canon.chip(5, oa.o16(OBJ_CHIP)), ob.o16(OBJ_CHIP)
                a, b = (ca & 0xFF, cb & 0xFF) if off == OBJ_CHIP else (ca >> 8, cb >> 8)
            elif off == OBJ_OVERLAY_KIND and kind == 'T1' and oa.o8(OBJ_TYPE) == T1_OVERLAY:
                a = canon.overlay_kind.get(a, a)
            elif off in canon.obj_pointer_bytes.get((kind, oa.o8(OBJ_TYPE)), {}):
                p, n = canon.obj_pointer_bytes[(kind, oa.o8(OBJ_TYPE))][off]
                if a == oa.o32(p) >> 8 * n & 0xFF and b == ob.o32(p) >> 8 * n & 0xFF:
                    a = b
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
    dark = oa.kind == 'T1' and oa.o16(OBJ_NAME_ID) == DARK_SOUL_5
    if dark:
        ai5 = dark_chips(ai5, DARK_CHIP_AT)
    if oa.kind == 'T1' and not megaman and not fa.canon.is_navi(5, oa.o16(OBJ_NAME_ID)) and u16(ai5, DISPATCH_COPY) == 0:
        # (BN6's virus dispatcher writes the object's +0x3E into its AI's data +0x3C after its state each frame, but while it
        # is bubbled, BN6 0x0810A362-0x0810A36C; BN5's, 0x08104550-0x081045CA, writes none: BN5's record read as BN6's
        # writes it, where BN5's own code wrote nothing there: legit.json, maps, dispatch_copy. A Navi's dispatcher is
        # another, which writes none)
        ai5 = bytes(ai5[:DISPATCH_COPY]) + bytes(oa.rec[NODE + 0x3E:NODE + 0x40]) + bytes(ai5[DISPATCH_COPY + 2:])
    raw5 = ai5   # (its chip ids BN5's: a paired chip's numbers read against its own record, chip_number)
    if not dark:
        # (the chip ids its AttackVars keep, legit.json's maps, chip_id, places: BN5's read as BN6's, the chip's subfamily
        # BN6's; the Dark Soul's by its chip AI's own reading, dark_chips)
        ai5 = chip_ids_read(fa.canon, ai5)
    # (an effect id its code keeps for the engine's teleport, legit.json's maps, effect_bytes: read as BN6's, and its word
    # compared as bytes, never as a pointer, which a pair of such bytes beside two others can look like)
    fx_bytes = [k for k in fa.canon.effect_bytes if ai5[k] != ob.ai[k] and fa.canon.fx(5, ai5[k]) == ob.ai[k]]
    if fx_bytes:
        ai5 = bytearray(ai5)
        for k in fx_bytes:
            ai5[k] = ob.ai[k]
    pointer_bytes = fa.canon.pointer_bytes.get(oa.o16(OBJ_NAME_ID), {}) if oa.kind == 'T1' and oa.o8(OBJ_TYPE) == 0 else {}
    for w in range(0, AI_SIZE, 4):
        wa, wb = u32(ai5, w), u32(ob.ai, w)
        view = None if any(w <= k < w + 4 for k in fa.canon.effect_bytes) else word_view(fa, fb, wa, wb, None)
        if view is not None:
            if view[0] != view[1]:
                out.append(('ai', w, view[0], view[1]))
            continue
        for k in range(4):
            a, b = ai5[w + k], ob.ai[w + k]
            if w + k == AI_INDEX:
                a = (fa.canon.navi_ai_index if fa.canon.is_navi(5, oa.o16(OBJ_NAME_ID)) else fa.canon.ai_index).get(a, a)
            elif megaman and w + k in LOADOUT and a != 0xFF:
                a = fa.canon.power_attack.get(a, a)
            elif w + k in pointer_bytes:
                p, n = pointer_bytes[w + k]
                if a == u32(ai5, p) >> 8 * n & 0xFF and b == u32(ob.ai, p) >> 8 * n & 0xFF:
                    a = b
            elif megaman and w + k in CHIP_NUMBERS and fa.canon.paired(u16(raw5, ATTACK_CHIP), u16(ob.ai, ATTACK_CHIP)):
                a, b = chip_number(5, raw5, w + k), chip_number(6, ob.ai, w + k)   # (a chip the spec's folders pair)
            if a != b:
                out.append(('ai', w + k, a, b))
    return out


ATTACK_SUB, ATTACK_CHIP = 0xA3, 0xB4   # an AI data's AttackVars +3, the chip's subfamily, and +0x14, the chip used


def chip_ids_read(canon, ai):
    """BN5's AI data with the chip ids it keeps (legit.json, maps, chip_id, places: AttackVars' chip) read as BN6's, and the
    chip's subfamily (AttackVars +3, written from its record's +0xC by both games' chip use, BN5 0x0800FEA0 / BN6 0x0801271A)
    BN6's record's where it is BN5's record's"""
    places = canon.chip_places.get('ai', ())
    if not places:
        return ai
    out = bytearray(ai)
    chip5 = u16(ai, ATTACK_CHIP)
    rec5 = canon.record(5, chip5)
    if rec5 is not None and ai[ATTACK_SUB] == rec5[0x0C]:
        out[ATTACK_SUB] = canon.behaviour6(canon.chip(5, chip5))[1]
    for off in places:
        struct.pack_into('<H', out, off, canon.chip(5, u16(ai, off)))
    return bytes(out)


def paired_chip_family(canon, oa, ob, a, b):
    """whether MegaMan's CurActions are each his chip's record's family, the chip one the spec's folders pair"""
    if oa.ai is None or ob.ai is None:
        return False
    c5, c6 = u16(oa.ai, ATTACK_CHIP), u16(ob.ai, ATTACK_CHIP)
    if not canon.paired(c5, c6):
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


def sea_apart(canon, raw5):
    """BN5's collision data with its sea's submerge read as BN6's engine keeps it (legit.json, layouts, sea): apart, its
    timer, flag and marker out of the record; (the record so, whether under, the marker)"""
    rule = canon.layouts.get('sea')
    if not rule:
        return raw5, False, 0
    b = bytearray(raw5)
    timer, flag, marker = (int(rule['bn5'][k], 0) for k in ('timer', 'flag', 'marker'))
    under = bool(u32(b, int(rule['bn5']['flags'], 0)) & flag)
    if u16(b, timer) == 0xFFFF:
        b[timer:timer + 2] = b'\0\0'
    off = int(rule['bn5']['flags'], 0)
    b[off:off + 4] = (u32(b, off) & ~flag & 0xFFFFFFFF).to_bytes(4, 'little')
    at = u32(b, marker)
    b[marker:marker + 4] = b'\0\0\0\0'
    return bytes(b), under, at


def sea_cells(fa, fb, under5, marker5, cb):
    """the submerge each game keeps of an object: BN5's in its collision data (sea_apart), BN6's apart (its frame's apart
    bytes, RAM_SEA: the mask of collision data's bits, then each one's marker by its bit's index)"""
    rule, apart = fa.canon.layouts.get('sea'), fb.fr.get('apart')
    if not rule or not apart:
        return []
    bit = u32(cb, int(rule['bn6']['index_bit'], 0))
    if not bit or bit & (bit - 1):
        return []
    under6 = bool(u32(apart, int(rule['bn6']['mask'], 0)) & bit)
    marker6 = u32(apart, int(rule['bn6']['markers'], 0) + 4 * (bit.bit_length() - 1))
    out = [('sea', 0, under5, under6)] if under5 != under6 else []
    va, vb = (fa.ptr(marker5) if marker5 else None), (fb.ptr(marker6) if marker6 else None)
    if va != vb:
        out.append(('sea', 4, va, vb))
    return out


def coll_cells(fa, fb, oa, ob):
    """the collision data through each game's layout (legit.json): a byte BN5 places where BN6 has it (its
    alliance words read as BN6's), BN5's own parts of a word by their rule, its sea's submerge as BN6's engine keeps
    it (apart: sea_cells), the rest of each zero in both or a difference"""
    out = []
    if (oa.coll is None) != (ob.coll is None):
        return [('coll', 0, oa.coll is not None, ob.coll is not None)]
    if oa.coll is None:
        return out
    canon, cb = fa.canon, ob.coll
    raw5, under5, marker5 = sea_apart(canon, oa.coll)
    out += sea_cells(fa, fb, under5, marker5, cb)
    ca = alliance_words(canon, raw5, cb)
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
    for slip in layout.get('bn6_slips', []):
        at = int(slip['offset'], 0)
        seen6.update(range(at, at + slip['size']))
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
    return canon.chip(5, v) if 'chip' in field else v


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


# What BN6's engine keeps apart beside the objects (src/dev/xdiff.c, the apart block): the sea's room (sea_cells), then the
# chip AI's lists, BN5_DARK_LIST_SIZE a side, then the Navis' own state (Colonel's sides' bits, BN5's from BattleState +0x5C),
# then the Dark Soul's AI data past BN6's 0x100 (its chip AI's state, BN6's in BN6_DARK_AI)
APART_SEA, DARK_LIST, APART_NAVIS, APART_DARK = 0x90, 0xE0, 0x10, 0x40
COLONEL_SIDES = 4   # (the Navis' own state's word both games hold: Colonel's sides' bits; the Dark Soul's AI data's holder BN6's alone)
DARK_CHIPS, DARK_SEQUENCES, DARK_SEQ = 0x2A, 0x58, 16
DARK_SOUL_5 = 0x18D   # (BN5_DARK_SOUL_ID: the chip AI's lists compared while it is in play, which alone reads them)
# the Dark Soul's AI data's chip ids (u16, BN5's chips as BN6's ids): its attack's (AttackVars +0x14, 0x0802C0A0), its chip
# AI's chip under way (+0xF0 +0x24) and its approach's (+0x130 +4)
DARK_CHIP_AT = (0xB4,)
DARK_CHIP_APART = (0x114 - AI_SIZE, 0x134 - AI_SIZE)
REGISTRY = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'src', 'core', 'xchips_registry.c')
_registry = None


def bn6_chip(v):
    """a chip AI list's entry of BN5's as BN6's (src/emu/xnavi_dark.c): 0xFFFF and a sequence's number as they are, a chip
    BN6's id for it by the chip registry (src/core/xchips_registry.c's rows), 0xFFFF where it holds none"""
    global _registry
    if _registry is None:
        rows = re.findall(r'^\t\{\s*(\d+),\s*(\d+),', open(REGISTRY, encoding='utf-8').read(), re.M)
        _registry = {int(a): int(b) for a, b in rows}
    if v == 0xFFFF or v & 0x8000:
        return v
    return _registry.get(v, 0xFFFF)


def dark_chip(v):
    """a chip id the Dark Soul's AI data holds, BN5's as BN6's (0 none, as it is)"""
    return v if v == 0 else bn6_chip(v)


def dark_chips(raw5, spots):
    """BN5's bytes with the chip ids at `spots` (u16 each) BN6's"""
    b = bytearray(raw5)
    for at in spots:
        b[at:at + 2] = dark_chip(u16(b, at)).to_bytes(2, 'little')
    return bytes(b)


def dark_lists(raw5):
    """BN5's chip AI lists with their chips as BN6's ids"""
    b = bytearray(raw5)
    for side in range(0, len(b) - DARK_LIST + 1, DARK_LIST):
        spots = [side + 2 * k for k in range(DARK_CHIPS)]
        spots += [side + DARK_SEQUENCES + DARK_SEQ * q + 2 + 2 * k for q in range(8) for k in range(7)]
        for at in spots:
            b[at:at + 2] = bn6_chip(u16(b, at)).to_bytes(2, 'little')
    return bytes(b)


def no_chips(lst):
    """a side's list that names no chip: each entry none (BN5's 0xFFFF, BN6's zeroed 0), as the chip AI reads them"""
    return all(u16(lst, 2 * k) in (0, 0xFFFF) for k in range(DARK_CHIPS))


def apart_cells(fa, fb, t):
    """the chip AI's lists while BN5's Dark Soul is in play (BN5's chips read as BN6's ids; a side naming no chip in both
    alike, BN5's 0xFFFF and BN6's 0 the same none) and the Navis' own state (Colonel's sides' bits), each byte the same in
    both, or a difference (role 'battle')"""
    pa, pb = fa.fr.get('apart', b''), fb.fr.get('apart', b'')
    end = APART_SEA + 2 * DARK_LIST + APART_NAVIS
    if len(pa) < end + APART_DARK or len(pb) < end + APART_DARK:
        return []
    out = []
    if any(o.kind == 'T1' and o.o16(OBJ_NAME_ID) == DARK_SOUL_5 for o in fa.fr['objs']):
        # (word by word: one that points into EWRAM, its target object as each game names it, as the AI data's are read)
        da, db = dark_chips(pa[end:end + APART_DARK], DARK_CHIP_APART), pb[end:end + APART_DARK]
        for k in range(0, APART_DARK, 4):
            wa, wb = u32(da, k), u32(db, k)
            va, vb = (fa.ptr(wa), fb.ptr(wb)) if is_ptr(wa) and is_ptr(wb) else (wa, wb)
            if va != vb:
                out.append(Diff(t, 'battle', 'the Dark Soul\'s AI data past 0x100', k, va, vb))
        la, lb = dark_lists(pa[APART_SEA:APART_SEA + 2 * DARK_LIST]), pb[APART_SEA:APART_SEA + 2 * DARK_LIST]
        for side in range(0, 2 * DARK_LIST, DARK_LIST):
            sa, sb = la[side:side + DARK_LIST], lb[side:side + DARK_LIST]
            if no_chips(sa) and no_chips(sb):
                continue
            out += [Diff(t, 'battle', 'chip AI lists', side + k, sa[k], sb[k]) for k in range(DARK_LIST) if sa[k] != sb[k]]
    na, nb = pa[APART_SEA + 2 * DARK_LIST:][:COLONEL_SIDES], pb[APART_SEA + 2 * DARK_LIST:][:COLONEL_SIDES]   # (the rest BN6's own)
    out += [Diff(t, 'battle', 'the Navis\' state', k, na[k], nb[k]) for k in range(len(na)) if na[k] != nb[k]]
    return out


def requested(f):
    """the songs asked for (its sound events, BN6's own or a copy's number as BN5's) in this record and the one before,
    whose songs this record's begun ones may be"""
    frames = f.trace['frames']
    k = next((i for i, fr in enumerate(frames) if fr is f.fr), 0)
    return {e.a & 0xFFFF for fr in frames[max(0, k - 1):k + 1] for e in fr['events'] if e.kind == SOUND}


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
    # (a song by its number: the music player it plays on is its song table entry's, each game's own: legit.json, maps; one
    # each game plays on another player, asked for in both, begun or not as its own player there is free: maps, sound; a
    # family's sound BN6 has under another number by that number, canon.snd)
    apart = requested(fa) & requested(fb) & fa.canon.players_apart
    sa = sorted(fa.canon.snd(fa.game, s) for _, s in fa.fr['sounds'] if s not in apart)
    sb = sorted(fb.canon.snd(fb.game, s) for _, s in fb.fr['sounds'] if s not in apart)
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
    out += apart_cells(fa, fb, t)
    mm = [next((o.ai for o in f.fr['objs'] if o.kind == 'T1' and f.canon.is_megaman(f.game, o.o16(OBJ_NAME_ID))), None) for f in (fa, fb)]
    if fb.fr.get('player') and len(fb.fr['player']) >= PLAYER_UNITE:
        out += soul_cells(fa, fb, mm[0], mm[1])
    return out
