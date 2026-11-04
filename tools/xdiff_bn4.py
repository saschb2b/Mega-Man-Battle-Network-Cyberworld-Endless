"""xdiff_bn4.py: the differential's BN4 view, for tools/xdiff.py (docs/devtools/differential.md, BN4).

A spec whose "game" is "bn4" fights BN4 Red Sun's own battle on the guest core, traced as BN5's is but in BN6's units
(src/dev/xdiff_bn4_trace.h: each part read from BN4's layout and written at BN6's offsets, BN4's other bytes kept), and
compares it frame by frame by the battle's timer:

  "kind": "control"   BN4's battle fought twice, each in a process of its own (game=bn4:trace): the two traces must be
                      the same at every frame, every byte, the pipeline's own check (its keys, its timer, its seeding);
                      with "again" (items of the spec's the second run takes instead) they must part, the comparator's
                      own check, and the spec passes where they do
  "against": "bn6"    BN4's battle against BN6's with the foes by BN6's ids (game=bn4:bn6): a family BN4 and BN6 both
                      have, or a BN4 family moved into BN6's engine ("dev": ["bn4families"], its ids BN6's by its recipe),
                      "rng": "replay" giving BN6's draws BN4's (src/dev/xdiff_events.h, BN4's hooks on its own routines)

Objects are matched by role (xdiff_snap.assign_roles: an enemy by its id, an attack by its type, an effect by its type
and id), their bytes from their list node on, their AI's data and their collision data compared as BN6 lays them out,
a pointer by what it names (xdiff_snap.Frame.ptr; a moved family's sprite by its offset in it, bn4_sprites); the panels,
the RNG words, the songs begun and the run order each frame. Against BN6's, BN4's numbers are read as BN6's where a map
says so with its evidence (a moved family's enemy ids and object type, CurActions: bn4_families, bn4_as_bn6); its
sounds, effects and alliance words are its own still. The fields BN4 has none of are masked (BN4_LACKS, each with why),
and only where BN4's trace holds nothing there. Against BN6's, the family's own records (family_roles) fail the spec at
their first unexplained difference; one in the world's alone makes it INCONCLUSIVE from there, never PASS (the BN5
differential's stop rule), the family compared on. A control fails at its first difference of any kind."""
import json
import os
import re

from xdiff_masks import CACHE, NO_PALETTE, NO_PLACE, places
from xdiff_snap import (DRAW, NODE, SPRITE, OBJ_ACTION, OBJ_EFFECT, OBJ_NAME_ID, OBJ_TYPE, PLAYER_PENDING, REPLAYED, RESERVE, ROOT, SIZE,
                        SPAWN, UNMAPPED, UNMATCHED, Diff, Frame, assign_roles, is_ptr, read_trace, role_name, u16, u32)

XV_FIRST_ID = 0x200   # (src/emu/xvirus_recipe.h: the moved families' enemy ids from here)


def bn4_families():
    """BN4's families moved into BN6's engine, from their recipe (src/emu/xvirus_bn4.c, the generator's numbers): each its
    BN4 object type (.src_ai: its first-kind type, BN4 having no AI index) and its six BN4 enemy ids with BN6's for them
    (.names, .first_id: Billy's 13-18 BN6's 0x2C0-0x2C5)"""
    text = open(os.path.join(ROOT, 'src', 'emu', 'xvirus_bn4.c')).read()
    # (each family's struct, `static const XVirusFamily bn4_NAME = { ... };`, and the rows it names: its effects, `.effects =
    # NAME_effects` and `static const XvId NAME_effects[] = { { 0x15, 0x90 }, ... }`, BN4's id and BN6's; a family naming
    # none has none, so each family's are its own, never the next one's)
    rows = {k: v for k, v in re.findall(r'static const Xv\w+ (\w+)\[\] = \{(.*?)\};', text, re.S)}
    out = []
    for body in re.findall(r'static const XVirusFamily \w+ = \{(.*?)\n\};', text, re.S):
        src, first, names = (re.search(p, body, re.S) for p in (r'\.src_ai = (0x[0-9A-Fa-f]+)', r'\.first_id = XV_FIRST_ID \+ (0x[0-9A-Fa-f]+)',
                                                                 r'\.names = \{ ([^}]*) \}'))
        eff = re.search(r'\.effects = (\w+)', body)
        effects = {int(a, 0): int(b, 0) for a, b in re.findall(r'\{ (0x[0-9A-Fa-f]+), (0x[0-9A-Fa-f]+) \}', rows.get(eff.group(1), ''))} \
            if eff else {}
        if src and first and names:
            ids = [int(x, 0) for x in names.group(1).split(',')]
            out.append(dict(type=int(src.group(1), 0), ids={v: XV_FIRST_ID + int(first.group(1), 0) + k for k, v in enumerate(ids)},
                            effects=effects))
    return out


def bn4_moved_types():
    """the object types the recipe moves into BN6's engine, run by the second-level dispatch (xvirus_recipe.h XvType: kind,
    the dispatch's number, XV_TYPE_MOVED, BN4's own number): {(kind, the dispatch's number): BN4's own}, the thunder's copy
    (BN6's third-kind 0x2A) among them. A dispatched object's type reads in the BN6 trace as the dispatch's side-table
    number (xdiff_snap.dispatched: Spikey's moved 0x30 runs at 0xE0); read back as BN4's own, so each meets BN4's"""
    text = open(os.path.join(ROOT, 'src', 'emu', 'xvirus_bn4.c')).read()
    return {(int(k), int(a, 0)): int(of, 0) for k, a, of in
            re.findall(r'\{ (\d), (0x[0-9A-Fa-f]+), XV_TYPE_MOVED, (0x[0-9A-Fa-f]+) \}', text)}


XV_ART = 0x08B80000   # (src/emu/xvirus_recipe.h: the families' art in BN6's free space, xdiff_snap.XV_CODE's end)


def bn4_sprites(rom4):
    """the sprites BN4's moved families copied (src/emu/xvirus_bn4.c's XvSprite rows: BN4's list and number, BN6's copy's
    place and room): each (BN4's sprite's address, BN6's copy's, its room, its cache key, its room in BN4's RAM), BN4's from
    its sprite lists (src/core/rom.c's BN4 layout, sprite_lists) in its ROM; a pointer into either read by its offset in the
    sprite (its frames' data). A sprite BN4 keeps LZ77'd (its list word's bit 31: the recipe's `(LZ77 in BN4)`) is drawn
    from RAM, where BN4's battle preload put it (src/emu/bn4.h BN4_SPRITE_PRELOAD): its address None, found by its key
    (BN4_SPRITE_CACHED's, its list's offset << 8 | its number) in the frame's cache (bn4_cache), its room there the LZ77
    header's size less the size word the cache's pointer passes (the recipe's copy drops it too: tools/xrecipe.py sprite)"""
    text = open(os.path.join(ROOT, 'src', 'emu', 'xvirus_bn4.c')).read()
    rom_c = open(os.path.join(ROOT, 'src', 'core', 'rom.c')).read()
    m = re.search(r'\[XROM_BN4_RED_SUN_US\].*?\.sprite_lists = (0x[0-9A-Fa-f]+)', rom_c, re.S)
    if not m or not rom4:
        return []
    lists, out = 0x08000000 + int(m.group(1).rstrip('uU'), 0), []
    for lst, idx, off, room in re.findall(r'\{ (\d+), 0x([0-9A-Fa-f]+), \d+, 0x[0-9A-Fa-f]+, XV_ART \+ 0x([0-9A-Fa-f]+), 0x([0-9A-Fa-f]+) \}', text):
        word = u32(rom4, u32(rom4, lists - 0x08000000 + 4 * int(lst)) - 0x08000000 + 4 * int(idx, 16))
        base, key = word & 0x7FFFFFFF, (4 * int(lst)) << 8 | int(idx, 16)
        lz = bool(word & 0x80000000)
        out.append((None if lz else base, XV_ART + int(off, 16), int(room, 16), key,
                    (u32(rom4, base - 0x08000000) >> 8) - 4 if lz else int(room, 16)))
    return out


CACHE_SLOTS, CACHE_KEYS, CACHE_SPRITES = 8, 0x04, 0x14   # (src/emu/bn4.h BN4_SPRITE_CACHE: its count, keys and sprites)


def bn4_cache(fr):
    """BN4's battle sprite cache at this frame, {its key: its sprite's address}, from the trace's block BN6 keeps apart
    (src/dev/xdiff.c apart4_out: BN4_SPRITE_CACHE's bytes first; a trace of before, zeros, holds none)"""
    if 'sprite_cache' not in fr:
        b = fr.get('apart', b'')
        fr['sprite_cache'] = {} if len(b) < CACHE_SPRITES + 4 * CACHE_SLOTS else \
            {u16(b, CACHE_KEYS + 2 * i): u32(b, CACHE_SPRITES + 4 * i) for i in range(min(b[0], CACHE_SLOTS))}
    return fr['sprite_cache']


def sprite_ptr(sprites, game, v, cache=None):
    """a pointer into a moved family's sprite (bn4_sprites) as ('sprite', which, its offset); None where it is none. BN4's
    LZ77'd sprite by the frame's cache (bn4_cache): where its key is not there, BN4 drew none of it"""
    for i, (b4, b6, room, key, room4) in enumerate(sprites):
        if game == 4 and b4 is None:
            b4, room = (cache or {}).get(key), room4
        base = b4 if game == 4 else b6 if game == 6 else None
        if base is not None and base <= v < base + room:
            return ('sprite', i, v - base)
    return None


class Bn4Canon:
    """canon (xdiff_snap.Canon) with BN4's moved families' ids read as BN6's (bn4_families), where a BN4 trace is compared
    with BN6's; everything else canon's own"""

    def __init__(self, canon, fams, foes=()):
        self.canon, self.ids = canon, {k: v for f in fams for k, v in f['ids'].items()}
        # (the effects of the spec's own families alone, those its foes are versions of: another family's ids none of its)
        own = [f for f in fams if set(f['ids']) & set(foes)] or fams
        self.effects = {k: v for f in own for k, v in f['effects'].items()}
        self.moved_types = bn4_moved_types()

    def nid(self, game, v):
        return self.ids.get(v, self.canon.nid(game, v)) if game == 4 else self.canon.nid(game, v)

    def fx(self, game, v):
        """an effect's id as BN6's: the spec's families' BN4 effects as their recipe moved them (xvirus_bn4.c's XvId rows:
        Billy's 0x15 and 0x16 BN6's 0x90 and 0x91, its move's flashes, from BN4's effects table 0x080E1698). Any BN4 effect
        of that number is read so; one the engine spawned would then meet BN6's own of its number and differ, never hide one"""
        return self.effects.get(v, v) if game == 4 else self.canon.fx(game, v)

    def t3(self, game, v):
        """a third-kind type as BN4 numbers it: BN6's dispatched one, its side-table number, read back as BN4's own where the
        recipe moved it (bn4_moved_types); BN4's as it is"""
        return self.moved_types.get((3, v), v) if game == 6 else self.canon.t3(game, v)

    def __getattr__(self, name):
        return getattr(self.canon, name)


# BN4's numbers in an object's record read as BN6's, only against BN6's trace (a control's two BN4 traces as they are):
#   its type (+1): a moved family's object runs as BN6's virus type 0 (its recipe's .src_ai the BN4 type its code was; BN6's
#     viruses all type 0, their AI index in their AI data, bn6_families.h BN6_AI_INDEX)
#   its CurAction (+9): BN4's generic states 0-5 BN6's 0-5 (tools/bn4_twins.py GENERIC4, each BN6's of the same place), its
#     own from 6 BN6's from 8 (BN6's 6 and 7, frozen and bubbled, BN4 has none of): the recipe's state table (BN6's eight
#     generic states, then BN4's own 0x080B95A9, 0x080B979D, 0x080B97B1, 0x080B98A7) and its immediates (CurAction 6-9
#     written as 8-11, src/emu/xvirus_bn4.c billy_imms); MegaMan's ready state the same way, 6 against BN6's 8 (BN5's map
#     too, legit.json's action)
#   (not its damage word, +0x2C: its top bits are the hit's properties in both games, read by the collision setup's
#     0x08012860 / 0x08019F44, never an element: a difference there is the family's)
# - its saved state word's CurAction (+0x5D) by the same map: the status a hit gives (BN4 0x0800AE80, BN6's twin 0x0800E760:
#   ldr r0,[r5,#0x5c]; if 0, ldr r0,[r5,#8]; str r0,[r5,#0x5c]; then CurAction 4) keeps the object's +8 to +0xB at +0x5C to
#   +0x5F while it holds him, the action it gives back after (MegaMan's ready, 6 in BN4 and 8 in BN6)
OBJ_SAVED_ACTION = 0x5D


def bn4_as_bn6(rec, fams):
    """an object's record (a T1's, from its node) with BN4's numbers as BN6's: its type, its CurAction and the one its
    saved state word keeps"""
    types = {f['type'] for f in fams}
    if rec[NODE + OBJ_TYPE] in types:
        rec[NODE + OBJ_TYPE] = 0
    for at in (OBJ_ACTION, OBJ_SAVED_ACTION):
        if rec[NODE + at] >= 6:
            rec[NODE + at] += 2
    return rec


# A collision data's alliance words (+0x30 and +0x34 in BN6's layout, BN4's +0x58 and +0x5C), each read by the same
# routine from its game's table by alliance and region (BN4 0x08012EF6 from 0x08012660, its 122 words; BN6 0x0801A0BA
# from BN6_COLL_MASKS, 0x08019C7C, 178): BN4's word read as BN6's entry at its place in BN4's table, where every place of
# it gives the same (BN6's first 122 are BN4's, 98 of them with bit 0x80: read from both ROMs, bn4_alliance)
BN4_MASKS, BN4_MASK_WORDS, BN6_MASKS = 0x08012660, 122, 0x08019C7C
COLL_MASKS = (0x30, 0x34)
# (and the words a hit ORs another's alliance word into, BN6's +0x6C and +0x70, BN4's +0x50 and +0x54: the hit pass, BN4
# 0x08012D3C / 0x08012BE2, BN6 0x081D7BD2 / 0x081D783A; read as BN6's entry where the word is one of BN4's table's, which
# a hit by one alone makes; an OR of several stays as it is and differs where their bit 0x80 does)
HIT_WORDS = (0x6C, 0x70)


def bn4_alliance(rom4, rom6):
    """BN4's alliance words as BN6's: {BN4's word: BN6's}, where its every place in BN4's table gives one BN6 entry"""
    if not rom4 or not rom6:
        return {}
    seen = {}
    for i in range(BN4_MASK_WORDS):
        a, b = u32(rom4, BN4_MASKS - 0x08000000 + 4 * i), u32(rom6, BN6_MASKS - 0x08000000 + 4 * i)
        seen.setdefault(a, set()).add(b)
    return {a: next(iter(bs)) for a, bs in seen.items() if len(bs) == 1}


# A collision data's direction of its last move (BN6's +0xC, BN4's +0xA: the collision panels' update, BN4 0x08012D9A /
# BN6 0x0801A04C, writes what BN4 0x0800AF90 / BN6 0x0800E994 make of the old panel and the new): BN4's by the screen,
# 0 none, 1 up, 2 down, 3 left, 4 right; BN6's the same up and down, but left and right by its side (left 3 on side 0 and
# 4 on side 1, right the other: 0x0800E9C4 and 0x0800E9D0 test the side, r4, the object's +0x16) and 5 a move of two
# panels or more (0x0800E998-0x0800E99E), which BN4 does not tell apart: BN4's 3 and 4 read as BN6's by the side, its
# others as they are (a move of two read 5 in BN6 alone stays a difference)
COLL_ALLIANCE, COLL_DIRECTION = 0x04, 0x0C


# A panel read in BN6's terms (BN6's field init, 0x0800C51A-0x0800C566, r4 the column and r5 the row, writes what BN4's
# keeps none of; BN4's, 0x08009120-0x0800920C, its type, side and three u16): BN6's +0 and +0xC from its two tables by
# row * 8 + column (BN6_PANEL_IN_FIELD_TABLE, BN6_PANEL_SPOT_TABLE: constants of the place), its column and row at +0xA
# and +0xB, its type again at +6 (BN6_PANEL_ANIM, the type it is drawn as) and its side again at +4 and +7, as BN6's
# init writes them from BN4's own type and side; a side by its column (BN6's per-column table, 0x0800C8C2, read for every
# panel: 0xFF on columns 0 and 7, a column's side on the border rows too), where BN4's border holds 0 (its side table
# BN4_PANEL_SIDES, 0x08009218: 0 off the 18): BN4's ring read so while it holds its start's 0, the column's side that of
# BN4's own panel of the column's row 1; the timers idle in both read alike (BN4's +0xE 0x3C0 and +0x12 0x80BE at its
# start, BN6's +0x10 0 and +0x12 BN6_PANEL_TIMER_IDLE 0x708), any count compared as it is. Never a mask: BN6's own
# record must hold what its init writes, and BN4's own type and side are what it is held to
PANEL_TYPE, PANEL_SIDE, PANEL_IN_FIELD, PANEL_SIDE_COPY, PANEL_ANIM, PANEL_SIDE_DRAWN = 0x02, 0x03, 0x00, 0x04, 0x06, 0x07
PANEL_COLUMN, PANEL_ROW, PANEL_SPOT, PANEL_SIDE_TIMER, PANEL_TIMER = 0x0A, 0x0B, 0x0C, 0x10, 0x12
BN6_IN_FIELD_TABLE, BN6_SPOT_TABLE, BN4_SIDES = 0x0800C590, 0x0800C5B8, 0x08009218
BN4_TIMERS_IDLE, BN6_TIMERS_IDLE = (0x3C0, 0x80BE), (0, 0x708)


def panel_tables(rom4, rom6):
    """the tables panel_as_bn6 reads: BN6's two by place, BN4's sides at the field's start"""
    if not rom4 or not rom6:
        return None
    at = lambda rom, a: rom[a - 0x08000000:a - 0x08000000 + 40]
    return dict(in_field=at(rom6, BN6_IN_FIELD_TABLE), spot=at(rom6, BN6_SPOT_TABLE), sides4=at(rom4, BN4_SIDES))


def panel_as_bn6(i, p, tabs, panels):
    """BN4's panel i (in BN6's layout, xdiff_bn4_trace.c) as BN6's field init would hold it (the comment above)"""
    out, col, row = bytearray(p), i % 8, i // 8
    if out[PANEL_SIDE] == tabs['sides4'][i] and (col in (0, 7) or row in (0, 4)):
        out[PANEL_SIDE] = 0xFF if col in (0, 7) else panels[8 + col][PANEL_SIDE]
    out[PANEL_IN_FIELD], out[PANEL_SPOT] = tabs['in_field'][i], tabs['spot'][i]
    out[PANEL_COLUMN], out[PANEL_ROW] = col, row
    out[PANEL_ANIM], out[PANEL_SIDE_COPY], out[PANEL_SIDE_DRAWN] = out[PANEL_TYPE], out[PANEL_SIDE], out[PANEL_SIDE]
    timers = u16(out, PANEL_SIDE_TIMER), u16(out, PANEL_TIMER)
    if timers == BN4_TIMERS_IDLE:
        out[PANEL_SIDE_TIMER:PANEL_SIDE_TIMER + 2] = BN6_TIMERS_IDLE[0].to_bytes(2, 'little')
        out[PANEL_TIMER:PANEL_TIMER + 2] = BN6_TIMERS_IDLE[1].to_bytes(2, 'little')
    return out


def coll_as_bn6(coll, alliance):
    """a BN4 collision data (in BN6's layout) with its alliance words (bn4_alliance) and its last move's direction as
    BN6's"""
    out = bytearray(coll)
    for at in COLL_MASKS + HIT_WORDS:
        w = u32(out, at)
        out[at:at + 4] = alliance.get(w, w).to_bytes(4, 'little')
    if out[COLL_DIRECTION] in (3, 4) and out[COLL_ALLIANCE] & 1:
        out[COLL_DIRECTION] ^= 7
    return out

# The fields of BN6's battle BN4's has none of, in BN6's units (part, BN6's offset, bytes): each masked where BN4's trace
# holds none (zero, or no AI data at all) and the other is BN6's, never where BN4 holds something there
BN4_LACKS = [
    dict(name='bn4 no ai index', part='ai', offset=0x01, size=1,
         why="BN6's AI data +1 is the AI index its virus dispatcher runs a family's AI by (bn6.h, BN6_T1_AI_DATA; xvirus_dispatch.h); "
             "BN4's viruses have no AI data at all (bn4.h, BN4_T1_AI_DATA: 0 on a virus; its one AI data a side is MegaMan's and "
             "a Navi's, BN4_AI_DATA), its families' routines run by their object's type alone (BN4_T1_TYPES)"),
    dict(name='bn4 no ai data', part='ai', offset=0x00, size=0, present=True,
         why="a virus's AI data, BN6's own (its spawner gives each virus one of its eight, BN6_AI_DATA_SIZE), which BN4's viruses "
             "have none of (bn4.h, BN4_T1_AI_DATA: 0 on a virus): BN4 keeps a virus's state in its object"),
    dict(name='bn4 no ai data pointer', part='obj', offset=0x58, size=4,
         why="a virus's pointer to that AI data (bn6.h, BN6_T1_AI_DATA), 0 on every BN4 virus (bn4.h, BN4_T1_AI_DATA), as above"),
    dict(name='bn4 no freeze', part='coll', offset=0x2A, size=2,
         why="BN6's collision data +0x2A, the frames left frozen in an ice block (bn6.h, BN6_COLL_FREEZE): BN4 has no freeze "
             "(bn4.h, BN4_COLL_INVULNERABLE: its statuses paralysis, confusion, blindness and the hold, +0x10-+0x17)"),
    dict(name='bn4 no struck-by mask', part='coll', offset=0x7C, size=4,
         why="BN6's collision data +0x7C, the strikers' index bits a hit pass ORs in (bn6_panels.h BN6_COLL_STRUCK_BY: "
             "0x081D7832-0x081D7838, ldr r3,[r7,#0x44]; ldr r2,[r6,#0x7c]; orrs; str): BN4's hit pass, its twin, ORs the "
             "striker's alliance word alone there (0x08012BDC-0x08012BE8: bn4.h BN4_COLL_HITS_WORD), and keeps the pairs met "
             "apart as BN6 does (BN4_COLL_MET, BN6's +0x68)"),
    dict(name='bn4 no bubble', part='coll', offset=0x2C, size=2,
         why="BN6's collision data +0x2C, the frames left in a bubble (bn6.h, BN6_COLL_BUBBLE): BN4 has no bubble (bn4.h, as the "
             "freeze)"),
]


def lacks(d, ta, tb):
    """the BN4_LACKS mask that explains d (BN4's side a, BN6's b), None where none does"""
    if ta['game'] != 4 or tb['game'] != 6:
        return None
    for m in BN4_LACKS:
        if d.part != m['part']:
            continue
        if m.get('present') and d.off == 0 and d.a is False and d.b is True:
            return m['name']
        if not m.get('present') and m['offset'] <= d.off < m['offset'] + m['size'] and d.a in (0, None, ('raw', 0)):
            return m['name']
    return None


def named(canon, game, o, fams=None, reserved=()):
    """an object's bytes from its node with its NameID as BN6 numbers it (canon.nid: BN4's MegaMan, enemy id 0, BN6's; a
    moved family's by its recipe); against BN6's (fams given), a BN4 T1's type and CurAction as BN6's too (bn4_as_bn6),
    and one that has reserved a panel with BN6's 0x20 (reserved_roles)"""
    rec = bytearray(o.rec[:SIZE[o.kind]])
    for k, v in o.over.items():   # (the family's renderer places as its ranks: bn4_renderer)
        if k < len(rec):
            rec[k] = v
    if o.kind == 'T1':
        at = NODE + OBJ_NAME_ID
        rec[at:at + 2] = canon.nid(game, rec[at] | rec[at + 1] << 8).to_bytes(2, 'little')
        if game == 4 and fams is not None:
            rec = bn4_as_bn6(rec, fams)
    if game == 4 and fams is not None and o.role in reserved:
        rec[NODE] |= OBJ_RESERVED
    moved = getattr(canon, 'moved_types', None)
    if game == 6 and moved and o.moved:   # (a dispatched object's type, its side-table number, as BN4's own: bn4_moved_types)
        rec[NODE + OBJ_TYPE] = moved.get((int(o.kind[1]), rec[NODE + OBJ_TYPE]), rec[NODE + OBJ_TYPE])
    if game == 4 and fams is not None and o.kind == 'T4' and rec[NODE + OBJ_TYPE] == 0:   # (an effect object's id: Bn4Canon.fx)
        rec[NODE + OBJ_EFFECT] = canon.fx(game, rec[NODE + OBJ_EFFECT])
    if fams is not None or game == 6:
        rec[NODE + OBJ_SLOT] = (rec[NODE + OBJ_SLOT] - o.slot) & 0xFF
    return rec


# An object's +3 is its own slot in its kind's table in both games (every object record of billy_v1's traces, 2,902 in
# BN4's and 4,096 in BN6's, holds it; BN5's differential reads it so too, legit.json's maps, slot): read against BN6's as counted from
# its own, so another game's object taking a slot first (BN6's opening effect 0x08) leaves it the same
OBJ_SLOT = 0x03


# An object that has reserved a panel, read in BN6's terms from BN4's own events, as tools/xdiff_events.py reads BN5's:
# BN6's object_reservePanel (0x0801BB1C) sets the object's +0 bit 0x20 at its first reservation (0x0801BB36-0x0801BB3C:
# ldrb r0,[r5]; movs r1,#0x20; orrs; strb) and nothing clears it while the object lives; BN4's (0x080143A8, its research
# twin) reserves the panel alike (+0x14 the reserver, its flags' 0x80) and sets no bit in the object (bn4.h
# BN4_RESERVE_PANEL). Not a mask: the object's own record read as BN6 would have written it
OBJ_RESERVED = 0x20


def reserved_roles(trace):
    """each frame's roles that have reserved a panel by its end (BN4's reservation events, XD_RESERVE), into fr['reserved']"""
    roles = set()
    for fr in trace['frames']:
        by_addr = {o.addr: o for o in fr['objs']}
        roles |= {by_addr[e.object].role for e in fr['events'] if e.kind == RESERVE and e.object in by_addr}
        fr['reserved'] = frozenset(roles)


def cells(fa, fb, part, ba, bb, start=0):
    """the places two parts' bytes differ, a word that is a pointer in either by what it names (Frame.ptr: an object by
    its role and part, a panel; a pointer into a ROM as it is, BN4's ROM's data not yet read as BN6's)"""
    out, k = [], start
    while k < max(len(ba), len(bb)):
        if k % 4 == 0 and k + 4 <= min(len(ba), len(bb)):
            wa, wb = u32(ba, k), u32(bb, k)
            if is_ptr(wa) or is_ptr(wb):
                sprites = getattr(fa.canon, 'sprites', ())   # (a moved family's sprite by its offset in it: bn4_sprites)
                view = (sprite_ptr(sprites, fa.game, wa, bn4_cache(fa.fr)) or fa.ptr(wa),
                        sprite_ptr(sprites, fb.game, wb, bn4_cache(fb.fr)) or fb.ptr(wb))
                if view[0] != view[1]:
                    out.append((part, k, view[0], view[1]))
                k += 4
                continue
        a, b = ba[k] if k < len(ba) else None, bb[k] if k < len(bb) else None
        if a != b:
            out.append((part, k, a, b))
        k += 1
    return out


OBJ_OWNER, AI_COMPANION = 0x4C, 0x58


def companions(f):
    """the role names of BN6's MegaMan's companions this frame: the fourth kind's 0x08 objects MegaMan owns (+0x4C), which
    BN6's player init spawns and keeps at his AI data +0x58 (legit.json's bn4 allowed, BN6's MegaMan's companion)"""
    mm = [o for o in f.fr['objs'] if o.kind == 'T1' and o.slot == 0]
    return {role_name(o.role) for o in f.fr['objs'] if mm and o.kind == 'T4' and o.o8(OBJ_TYPE) == 8 and o.o32(OBJ_OWNER) == mm[0].addr}


def holds_nothing(a, d):
    """an allowed range's direction kept (one game's own field the other has none of): "BN4 has, BN6 not" applies only
    where BN6's side reads 0 there, "BN6 has, BN4 not" only where BN4's does, so the other game's own field, should it
    ever hold something there, is compared as ever; a range with no direction, as it is"""
    zero = lambda v: v == 0 or v == ('raw', 0)
    return {'BN4 has, BN6 not': lambda: zero(d.b), 'BN6 has, BN4 not': lambda: zero(d.a)}.get(a.get('direction'), lambda: True)()


def raw_int(v):
    """a cell's value as a number: an int as it is, a raw word's ('raw', n) its n, else None"""
    if isinstance(v, bool):
        return None
    if isinstance(v, int):
        return v
    return v[1] if isinstance(v, tuple) and len(v) == 2 and v[0] == 'raw' and isinstance(v[1], int) else None


def allowed_world(d, fb):
    """the allowed difference of legit.json's bn4 a world's difference against BN6's is (MegaMan's own sprite; BN6's
    MegaMan's companion, its being in play and his AI data's pointer to it), None where none"""
    if not isinstance(d.role, tuple):
        return None
    for a in bn4_allowed():
        if a.get('whose') != 'megaman' or d.role[0] != a['kind'] or d.role[1] != int(a['type'], 0):
            if not (a.get('whose') == 'megaman' and a['kind'] == 'T4' and d.role[0] == 'T1' and d.role[1] == 0x1A0
                    and d.part == 'ai' and d.off == AI_COMPANION and d.a == ('raw', 0) and isinstance(d.b, tuple)
                    and d.b[0] in companions(fb)):
                continue
            return a['name']
        if 'offsets' in a:
            lo, hi = (int(x, 0) for x in a['offsets'])
            if d.part == a['part'] and lo <= d.off <= hi and holds_nothing(a, d):
                return a['name']
        elif 'bit' in a:
            # (that bit alone: every other bit of the word compared as ever; a word read as a raw value, its number)
            bit0, va, vb, k = int(a['bit'], 0), raw_int(d.a), raw_int(d.b), d.off - int(a['offset'], 0)
            if d.part == a['part'] and 0 <= k < 4 and isinstance(va, int) and isinstance(vb, int):
                # (the word read whole at its offset, or one byte of it: that byte's share of the bit)
                bit = bit0 if k == 0 and max(va, vb) > 0xFF else (bit0 >> (8 * k)) & 0xFF
                if bit and va ^ vb == bit and vb & bit:
                    return a['name']
        elif role_name(d.role) in companions(fb) and d.part == 'in play' and d.a is False and d.b is True:
            return a['name']
    return None


def bn4_sounds():
    """legit.json's bn4 sounds: {BN4's sound: (BN6's, its clock)}, each pair by the routines that play them in both games;
    a BN4 sound not there is compared as its own number, by the state's clock"""
    rows = json.load(open(os.path.join(ROOT, 'tests', 'xdiff', 'legit.json'))).get('bn4', {}).get('sounds', [])
    return {int(r['bn4'], 0): (int(r['bn6'], 0), r.get('clock', 'state')) for r in rows}


def sounds_diff(fa, fb, fbt, t):
    """the frame's sounds begun: BN4's as BN6's numbers (bn4_sounds), each against BN6's by its own clock: a state's at
    the battle's offset (fb), the battle's timer's at the same fight frame (fbt, BN6's frame of BN4's t); None where alike"""
    sa = [s for _, s in fa.fr['sounds']]
    if fb.game != 6 or fa.game != 4:
        sb = [s for _, s in fb.fr['sounds']]
        return None if sorted(sa) == sorted(sb) else (sorted(sa), sorted(sb))
    m = bn4_sounds()
    timer6 = {b for b, c in m.values() if c == 'timer'}
    state_a = sorted(m.get(s, (s, 'state'))[0] for s in sa if m.get(s, (s, 'state'))[1] == 'state')
    timer_a = sorted(m[s][0] for s in sa if s in m and m[s][1] == 'timer')
    state_b = sorted(s for _, s in fb.fr['sounds'] if s not in timer6)
    timer_b = sorted(s for _, s in (fbt.fr['sounds'] if fbt else ()) if s in timer6)
    return None if (state_a, timer_a) == (state_b, timer_b) else (state_a + timer_a, state_b + timer_b)


SPRITE_COUNT = 0x91   # (a first-kind object's sprite +1: the frames its animation's count runs, legit.json's bn4 clocks)


def opening_count(d, fa, fbt):
    """legit.json's bn4 clocks, the opening's animation count: an object's sprite +1 at the frames the count from the
    opening's animation set runs (BN4's t1 and t2, the count 2 when the first fight frame's sprite update begins), which the
    sprite update counts by the fight's frames, compared at the same fight frame: True where BN6's of BN4's t holds the same"""
    if not (d.part == 'obj' and d.off == SPRITE_COUNT and isinstance(d.role, tuple) and d.role[0] == 'T1' and fbt
            and fa.fr['t'] <= 2):
        return False
    ob = next((o for o in fbt.fr['objs'] if o.role == d.role), None)
    return ob is not None and ob.o8(SPRITE_COUNT) == d.a


SHADER, SHADER_ON, PARALYSED, COLL_FLAGS6 = 0x96, 0x3FF, 0x800, 0x3C   # (legit.json's bn4 clocks, the paralysis's flash)


def paralysis_flash(d, fa, fb):
    """legit.json's bn4 clocks, the paralysis's flash: a first-kind object's colour shader (its sprite +6, u16) is 0x3FF
    while its collision flags hold 0x800 (a state's clock, the battle's offset) and its battle timer's bit 1 is set (the
    timer's), else 0, in both games (BN4 0x08010408, BN6 0x080168C8): True where BN4's at t and BN6's at t + 1 are each
    that, by BN6's flags of t + 1 and each game's own timer"""
    if not (d.part == 'obj' and d.off in (SHADER, SHADER + 1) and isinstance(d.role, tuple) and d.role[0] == 'T1'):
        return False
    oa = next((o for o in fa.fr['objs'] if o.role == d.role), None)
    ob = next((o for o in fb.fr['objs'] if o.role == d.role), None)
    if oa is None or ob is None or ob.coll is None:
        return False
    gate = int.from_bytes(bytes(ob.coll[COLL_FLAGS6:COLL_FLAGS6 + 4]), 'little') & PARALYSED
    want = lambda t: SHADER_ON if gate and t & 2 else 0
    return oa.o16(SHADER) == want(fa.fr['t']) and ob.o16(SHADER) == want(fb.fr['t'])


SPARK, SPARK_SPRITE = 0x04, (0x80, 0xBB)   # (the fourth kind's hit spark and its sprite state: legit.json's bn4 allowed)
SPARKS_SEEN = set()   # (the sparks in play in both games at a frame compared before: compare keeps it)


def is_spark(role):
    return isinstance(role, tuple) and role[0] == 'T4' and role[1] == SPARK


def spark_own(d):
    """legit.json's bn4 allowed, the hit's spark: each engine's own effect, its sprite state and its life (how long its own
    art's animation runs); its place, its spawn and everything else of it compared as ever"""
    if not is_spark(d.role):
        return False
    # (its end, the frame one game's animation ends: its update's +0 0x02 cleared and its state 8, the delete)
    end = d.part == 'obj' and ((d.off == 0x08 and 8 in (d.a, d.b)) or (d.off == 0x00 and isinstance(d.a, int) and d.a ^ d.b == 0x02))
    # (in play in one game alone only once both had it in play: its life's end, never a spawn one game lacks)
    return (d.part == 'obj' and SPARK_SPRITE[0] <= d.off <= SPARK_SPRITE[1]) or (d.part == 'in play' and d.role in SPARKS_SEEN) or end


def frame_diffs(fa, fb, fbt=None):
    """each difference of one frame: the objects by role, the RNG words, the songs, the run order, the panels"""
    t, out = fa.fr['t'], []
    ra, rb = {o.role: o for o in fa.fr['objs']}, {o.role: o for o in fb.fr['objs']}
    for role in sorted(set(ra) | set(rb), key=role_name):
        oa, ob = ra.get(role), rb.get(role)
        if oa is None or ob is None:
            out.append(Diff(t, role, 'in play', 0, oa is not None, ob is not None))
            continue
        # (from its node's +8: its links compared as the run order; BN4's numbers as BN6's only against BN6's)
        fams = getattr(fa.canon, 'fams', None) if fb.game == 6 else None
        found = [(p, off - NODE, a, b) for p, off, a, b in cells(fa, fb, 'obj', named(fa.canon, fa.game, oa, fams, fa.fr.get('reserved', ())),
                                                                 named(fb.canon, fb.game, ob), 8)]
        alliance = getattr(fa.canon, 'alliance', None) if fb.game == 6 else None
        ca = coll_as_bn6(oa.coll, alliance) if alliance and oa.coll is not None else oa.coll
        for part, xa, xb in (('ai', oa.ai, ob.ai), ('coll', ca, ob.coll)):
            if (xa is None) != (xb is None):
                found.append((part, 0, xa is not None, xb is not None))
            elif xa is not None:
                found += cells(fa, fb, part, xa, xb)
        out += [Diff(t, role, part, off, a, b) for part, off, a, b in found]
    if fa.fr['rng'] != fb.fr['rng'] and not replayed_world(fa, fb):
        out.append(Diff(t, 'battle', 'rng', 0, fa.fr['rng'], fb.fr['rng']))
    # (the family's own draws, each given BN4's: one BN6 made with none of BN4's to give it, or none of its code's place, is
    # the family's difference, never the world's)
    for e in fb.fr['events'] if fb.game == 6 and fa.game == 4 else ():
        if e.kind == DRAW and e.family and e.flags & (UNMATCHED | UNMAPPED):
            out.append(Diff(t, FAMILY_DRAWS, 'draw', 0, None, f'{e.family:#x} drawn as it is'))
    sd = sounds_diff(fa, fb, fbt, t)   # (each by its clock: legit.json's bn4 sounds)
    if sd:
        out.append(Diff(t, 'battle', 'sounds', 0, sd[0], sd[1]))
    oa, ob = [fa.ptr(x) for x in fa.fr['order']], [fb.ptr(x) for x in fb.fr['order']]
    if fb.game == 6 and fa.game == 4:   # (BN6's MegaMan's companion, BN6's alone: legit.json's bn4 allowed)
        ob = [x for x in ob if not (isinstance(x, tuple) and x[0] in companions(fb))]
        # (a hit's spark in play in one game alone, its own art's life: legit.json's bn4 allowed, the hit's spark)
        ra = {role_name(o.role) for o in fa.fr['objs'] if is_spark(o.role) and o.role in SPARKS_SEEN}
        rb = {role_name(o.role) for o in fb.fr['objs'] if is_spark(o.role) and o.role in SPARKS_SEEN}
        oa = [x for x in oa if not (isinstance(x, tuple) and x[0] in ra - rb)]
        ob = [x for x in ob if not (isinstance(x, tuple) and x[0] in rb - ra)]
    if oa != ob:
        out.append(Diff(t, 'battle', 'run order', 0, oa, ob))
    # (MegaMan's pending form: BN4's AI data +0x10 at the player block's pending record +0, BN6_CROSS_PENDING's form, by
    # src/dev/xdiff_bn4_trace.c's XD4_PENDING; the record's other bytes BN6's own)
    pa, pb = fa.fr.get('player', b''), fb.fr.get('player', b'')
    if fa.game == 4 and pa and pb and pa[PLAYER_PENDING] != pb[PLAYER_PENDING]:
        out.append(Diff(t, 'player', 'pending form', 0, pa[PLAYER_PENDING], pb[PLAYER_PENDING]))
    tabs = getattr(fa.canon, 'panels', None) if fb.game == 6 else None
    for i, (pa, pb) in enumerate(zip(fa.fr['panels'], fb.fr['panels'])):
        pa = panel_as_bn6(i, pa, tabs, fa.fr['panels']) if tabs else pa
        out += [Diff(t, f'panel {i % 8},{i // 8}', part, off, a, b) for part, off, a, b in cells(fa, fb, 'panel', pa, pb)]
    return out


def said(d, tb):
    """a difference as a line, each side by its game"""
    fmt = lambda v: f'{v:#x}' if isinstance(v, int) and not isinstance(v, bool) else str(v)
    who = role_name(d.role) if isinstance(d.role, tuple) else d.role
    return f't{d.t} {who} {d.part}+{d.off:#04x}: BN4 {fmt(d.a)} | BN{tb["game"]} {fmt(d.b)}'


def family_roles(trace, canon):
    """the roles of a BN4 family moved into BN6's engine (canon.fams: their enemies by BN6's ids) and of the objects their
    code spawned (a spawn event whose chain passed through the family's code, xdiff_events.h's family): the family's own
    records, whose differences fail the spec where the world's (MegaMan, the engine, the RNG, the panels) stop it"""
    ids = {v for f in getattr(canon, 'fams', ()) for v in f['ids'].values()}
    out = set()
    for fr in trace['frames']:
        by_addr = {o.addr: o for o in fr['objs']}
        for o in fr['objs']:
            if o.kind == 'T1' and isinstance(o.role, tuple) and o.role[1] in ids:
                out.add(o.role)
        for e in fr['events']:
            if e.kind == SPAWN and e.family and e.object in by_addr:
                out.add(by_addr[e.object].role)
    return out


# Both RNG words against BN6's where BN6's draws are replayed from BN4's: BN6 steps them three times a frame (its main
# loop's 0x080050FC and 0x08005100 and 0x0800031C: bn6.h BN6_RNG_PRIMARY and BN6_RNG_SECONDARY; 1473 each in billy_v1's
# 1200 fight frames, from 0x08005101, 0x08005105 and 0x08000321) and BN4 never does (its 177 draws there none of a
# frame's own: its callers' counts 42, 42, 30, 30, 15, 15, 2, 1), so the words part at the fight's first frame and stay
# apart whatever either battle does; with the replay every draw of the family's code reads BN4's word (its own draws
# compared one by one, below), so the words are read only there
FAMILY_DRAWS = "the family's draws"   # (the role of a draw of the family's that the replay could not give BN4's word)


def replayed_world(fa, fb):
    """whether BN6's battle here replays its draws from BN4's (its trace's draw events marked so), the words left uncompared"""
    return fa.game == 4 and fb.game == 6 and any(e.kind == DRAW and e.flags & (REPLAYED | UNMATCHED | UNMAPPED)
                                                 for fr in fb.trace['frames'][:64] for e in fr['events'])


def bn4_renderer(fa, fb, fam, last):
    """the family's renderer places read as its own, as BN5's are (legit.json, maps, renderer; tools/xdiff_masks.py
    renderer_ranks, whose reading this is with the family's roles given): the renderer hands each object it draws its tile
    place (its sprite state's +0x08) and palette slot (+0x15's high nibble) from running counts over all it draws before
    it, so what else each game draws (MegaMan's own art, BN6's thunder's own art, BN6's opening effect) moves the numbers:
    a family object drawn this frame reads its rank among those the family's drawn objects take, in the order they ran;
    one not drawn keeps its last rank where its place stayed; one never drawn none. Into each object's `over`"""
    for f in (fa, fb):
        own = [o for o in f.fr['objs'] if o.role in fam]
        drawn = [o for o in own if o.o8(0) & 2 and o.rec[SPRITE[o.kind] + NODE + 3] & 2 and u32(o.rec, SPRITE[o.kind] + NODE + CACHE)]
        run = {}
        for n, a in enumerate(f.fr['order']):
            for o in drawn:
                if o.addr - NODE <= a < o.addr - NODE + SIZE[o.kind]:
                    run.setdefault(o.role, n)
        drawn.sort(key=lambda o: (run.get(o.role, len(f.fr['order'])), o.kind, o.slot))
        for k in (0, 1):
            taken = []
            for o in drawn:
                if places(o)[k] not in taken and not (k and places(o)[k] == NO_PALETTE):
                    taken.append(places(o)[k])
            for o in own:
                raw, key, rank, sp = places(o)[k], (f.game, o.role, k), None, SPRITE[o.kind] + NODE
                if k and raw == NO_PALETTE:
                    continue
                if o in drawn:
                    rank = taken.index(raw)
                    last[key] = (raw, rank)
                elif key in last and last[key][0] == raw:
                    rank = last[key][1]
                elif key not in last:
                    raw = NO_PALETTE if k else NO_PLACE
                if k:
                    o.over[sp + 0x15] = (rank if rank is not None else raw) << 4 | o.rec[sp + 0x15] & 0xF
                else:
                    v = rank if rank is not None else raw if raw == NO_PLACE else 0x8000 | raw
                    o.over[sp + 8], o.over[sp + 9] = v & 0xFF, v >> 8


def unexplained(fa, fb, ta, tb, used, fbt=None):
    """a frame's differences no BN4_LACKS mask explains (the masks applied counted into used)"""
    diffs = frame_diffs(fa, fb, fbt)
    for d in diffs:
        d.mask = lacks(d, ta, tb)
        if d.mask:
            used[d.mask] = used.get(d.mask, 0) + 1
    return [d for d in diffs if not d.mask]


def allowed_lag(spec):
    """the fight frames BN6's whole battle runs behind BN4's: the spec's "lag", only where legit.json's bn4 allowed names
    exactly that lag for the battle (its reason and both games' code paths there: BN6's engine runs no state while the
    opening is paused, BN4's does); 0 none. Every record of BN6's battle is compared at t + lag against BN4's at t, never a
    field or a role apart; any other lag is refused"""
    lag = spec.get('lag', 0)
    if not lag:
        return 0
    ok = [a for a in bn4_allowed() if a.get('whose') == 'battle' and a.get('lag') == lag]
    if not ok:
        raise SystemExit(f'lag {lag}: no allowed difference of tests/xdiff/legit.json (bn4) names it')
    return lag


def bn4_allowed():
    """legit.json's bn4 allowed differences (each with its reason and both games' code)"""
    return json.load(open(os.path.join(ROOT, 'tests', 'xdiff', 'legit.json'))).get('bn4', {}).get('allowed', [])


def allowed_bit(d, fam):
    """the allowed difference of legit.json's bn4 a family's difference is (a bit of its object BN6 has and BN4 not, the
    whole difference that bit), None where none: never on another's records, never more than its bit"""
    if d.role not in fam:
        return None
    for a in bn4_allowed():   # (a range of one kind and type's own object: the thunder's sprite state)
        if 'offsets' not in a or a.get('part') != d.part or not isinstance(d.role, tuple):
            continue
        lo, hi = (int(x, 0) for x in a['offsets'])
        if d.role[0] == a['kind'] and d.role[1] == int(a['type'], 0) and lo <= d.off <= hi:
            return a['name']
    if not isinstance(d.a, int) or not isinstance(d.b, int) or isinstance(d.a, bool):
        return None
    for a in bn4_allowed():
        if 'bit' not in a or a.get('part') != d.part or int(a['offset'], 0) != d.off:
            continue
        bit = int(a['bit'], 0)
        if d.a ^ d.b == bit and d.b & bit:
            return a['name']
    return None


def compare(spec, ta, tb, canon, show=6):
    """('PASS'|'FAIL'|'INCONCLUSIVE', the frame of it, lines): against BN6's, the first frame with a difference no
    BN4_LACKS mask explains in the family's own records (family_roles) fails it; one in the world's alone (MegaMan, the
    engine, the RNG, the songs, the panels) makes it INCONCLUSIVE from there (the battle no longer the same battle for the
    family: the BN5 differential's stop rule), the family's still compared on and its first difference said. A control
    (two BN4 traces) fails at its first difference of any kind"""
    bt, used, n = {f['t']: f for f in tb['frames']}, {}, 0
    against = tb['game'] == 6
    fam = family_roles(ta, canon) | family_roles(tb, canon) | {FAMILY_DRAWS} if against else set()
    # (allowed_lag: BN6's whole battle, every record of it, against BN4's of `lag` fight frames before)
    lag = allowed_lag(spec) if against else 0
    stop, lines, ranks = None, [], {}
    SPARKS_SEEN.clear()
    for fra in ta['frames']:
        frb = bt.get(fra['t'] + lag)
        if frb is None:
            continue
        n += 1
        fa = Frame(ta, fra, canon)
        fb = Frame(tb, frb, canon)
        if against:
            bn4_renderer(fa, fb, fam, ranks)
        frt = bt.get(fra['t'])   # (BN6's frame of the same fight frame: what the battle's timer drives, legit.json's bn4 clocks)
        open_ = unexplained(fa, fb, ta, tb, used, Frame(tb, frt, canon) if frt and lag else fb)
        world = [d for d in open_ if d.role not in fam] if against else []
        ft = Frame(tb, frt, canon) if frt and lag else None
        for d in world:   # (legit.json's bn4 clocks: what the fight's frames count, at the same fight frame)
            d.mask = ("the opening's animation count, by the fight's frames" if opening_count(d, fa, ft) else
                      "the hit's spark, each engine's own" if spark_own(d) else
                      "the paralysis's flash, its gate a state's and its phase the timer's" if paralysis_flash(d, fa, fb) else None)
            if d.mask:
                used[d.mask] = used.get(d.mask, 0) + 1
        world = [d for d in world if not d.mask]
        for d in world:   # (legit.json's bn4 allowed of the world's: MegaMan's own sprite, BN6's companion of his)
            d.mask = allowed_world(d, fb)
            if d.mask:
                used[d.mask] = used.get(d.mask, 0) + 1
        world = [d for d in world if not d.mask]
        mine = [d for d in open_ if d.role in fam] if against else open_
        for d in mine:   # (legit.json's bn4 allowed bits of the family's own, each counted)
            d.mask = allowed_bit(d, fam)
            if d.mask:
                used[d.mask] = used.get(d.mask, 0) + 1
        mine = [d for d in mine if not d.mask]
        SPARKS_SEEN.update({o.role for o in fa.fr['objs'] if is_spark(o.role)} & {o.role for o in fb.fr['objs'] if is_spark(o.role)})
        if world and stop is None:
            stop = fra['t']
            lines += [f'  INCONCLUSIVE from t{stop}: the world parts there (no map or mask explains it), the family compared on:'] + \
                [f'    {said(d, tb)}' for d in world[:show]] + ([f'    ... {len(world) - show} more'] if len(world) > show else [])
        if mine:
            whose = 'the family\'s first' if against else 'first'
            lines += [f'  {n} fight frames compared; {whose} unexplained difference at t{fra["t"]}:'] + \
                [f'    {said(d, tb)}' for d in mine[:show]] + ([f'    ... {len(mine) - show} more'] if len(mine) > show else [])
            return 'FAIL', fra['t'], lines
    if stop is not None:
        return 'INCONCLUSIVE', stop, lines + [f'  {n} fight frames compared: the family\'s own records the same in all of them']
    lines = [f'  {n} fight frames compared (t{ta["frames"][0]["t"]}-t{ta["frames"][-1]["t"]}), masks applied: ' +
             (', '.join(f'{k} x{v}' for k, v in sorted(used.items())) or 'none')]
    if len(ta['frames']) + lag != len(tb['frames']):
        lines.append(f'  the traces are {len(ta["frames"])} and {len(tb["frames"])} fight frames long' + (f' (BN6\'s {lag} more)' if lag else ''))
        return 'FAIL', min(len(ta['frames']), len(tb['frames'])), lines
    return 'PASS', None, lines


def rng_twins():
    """legit.json's bn4 rng twins as the spec's twin= items (src/dev/xdiff_spec.c): each BN6's and BN4's bytes the same over
    its length but where both hold a BL (each to its own game's routine), checked against both ROMs (.build/xdiff/rom)"""
    rows = json.load(open(os.path.join(ROOT, 'tests', 'xdiff', 'legit.json'))).get('bn4', {}).get('rng_twins', [])
    roms = {g: open(os.path.join(ROOT, '.build', 'xdiff', 'rom', f'bn{g}.gba'), 'rb').read() for g in (4, 6)} if rows else {}
    items = []
    for t in rows:
        a6, a4, n = (int(t[k], 0) for k in ('bn6', 'bn4', 'len'))
        b6, b4 = roms[6][a6 - 0x08000000:a6 - 0x08000000 + n], roms[4][a4 - 0x08000000:a4 - 0x08000000 + n]
        k = 0
        while k + 1 < n:
            h6, h4 = b6[k] | b6[k + 1] << 8, b4[k] | b4[k + 1] << 8
            if h6 >> 11 == 0x1E and h4 >> 11 == 0x1E and k + 3 < n:   # (a BL's two halves)
                k += 4
                continue
            if h6 != h4:
                raise SystemExit(f'legit.json\'s bn4 twin BN6 {a6:#x} / BN4 {a4:#x} differs at +{k:#x}: {h6:04x} | {h4:04x}')
            k += 2
        items.append(f'twin={a6:08X}:{a4:08X}:{n:X}')
    return items


def spec_text(spec, out, game):
    """the spec's items as src/dev/xdiff_spec.c takes them, its battle BN4's (game: bn4:trace, bn4:bn6)"""
    keys = ','.join(f'{t}:{k}:{n}' for t, k, n in spec.get('keys', []))
    parts = [f'game={game}', f'foes={spec["foes"]}', f'mm={spec.get("mm", "22")}', f'hp={spec.get("hp", 100)}',
             f'frames={spec.get("frames", 1200)}', f'seed={spec.get("seed", "12345678:9ABCDEF0")}', f'out={out}']
    # (rng=replay: BN6's draws given BN4's, keyed as BN5's are, xdiff_events.h; poke=, call=: a mutation's break of the subject)
    extra = ([f'rng={spec["rng"]}'] if spec.get('rng') and game == 'bn4:bn6' else []) + \
        ([f'lag={spec["lag"]}'] if spec.get('lag') and game == 'bn4:bn6' else []) + \
        (rng_twins() if spec.get('rng') == 'replay' and game == 'bn4:bn6' else []) + \
        [f'poke={p}' for p in spec.get('poke', [])] + [f'call={c}' for c in spec.get('call', [])]
    return ';'.join(parts + ([f'keys={keys}'] if keys else []) + (['safe=1'] if spec.get('safe') else []) + extra)


def dev_args(spec):
    """the host build's arguments a spec asks for (its "dev": --dev bn4families, BN4's families moved into BN6's engine)"""
    return ['--dev', ','.join(spec['dev'])] if spec.get('dev') else []


def one(name, spec, a, canon, run_xdiff, work):
    """one BN4 spec run and compared: its verdict and kind. run_xdiff(text) runs the host build's --xdiff (tools/xdiff.py)"""
    runs = os.path.join(work, 'runs')
    control = spec.get('kind') == 'control'
    if control:
        paths = [os.path.join(runs, f'{name}{k}.bn4') for k in ('', '_again')]
        # (the second run's spec the first's but for its "again" items: the control broken on purpose, which must FAIL)
        for k, sp in (('', spec), ('_again', dict(spec, **spec.get('again', {})))) if not a.no_run else ():
            run_xdiff(spec_text(sp, f'/src/.build/xdiff/runs/{name}{k}', 'bn4:trace'))
    else:
        paths = [os.path.join(runs, f'{name}.{g}') for g in ('bn4', 'bn6')]
        if not a.no_run:
            run_xdiff(spec_text(spec, f'/src/.build/xdiff/runs/{name}', 'bn4:bn6'), dev_args(spec))
    ta, tb = read_trace(paths[0]), read_trace(paths[1])
    if not control:   # (BN4's moved families' ids as BN6's, their recipe's: bn4_families)
        fams = bn4_families()
        canon = Bn4Canon(canon, fams, [int(x.split("@")[0], 16) for x in spec.get("foes", "").split(",") if x])
        canon.fams = fams
        rom4 = os.path.join(work, 'rom', 'bn4.gba')
        rom4b = open(rom4, 'rb').read() if os.path.exists(rom4) else b''
        rom6 = os.path.join(work, 'rom', 'bn6.gba')
        canon.sprites = bn4_sprites(rom4b)
        rom6b = open(rom6, 'rb').read() if os.path.exists(rom6) else b''
        canon.alliance = bn4_alliance(rom4b, rom6b)
        canon.panels = panel_tables(rom4b, rom6b)
    for tr in (ta, tb):
        assign_roles(tr, canon)
    if not control:
        reserved_roles(ta)
    verdict, at, lines = compare(spec, ta, tb, canon, a.show)
    print('\n'.join(lines))
    kind = 'control' if control else 'bn4'
    what = 'against BN6\'s' if not control else 'fought twice, the second as "again" changes it, which must part' \
        if spec.get('again') else 'fought twice, its traces the same'
    if spec.get('again'):   # (the comparator's own test: the change must be found, at the frame it shows)
        verdict = 'PASS' if verdict == 'FAIL' else 'FAIL'
    print(f'{name}: {verdict}{f" at t{at}" if at is not None else ""} ({kind}: BN4\'s battle {what})')
    return verdict, kind
