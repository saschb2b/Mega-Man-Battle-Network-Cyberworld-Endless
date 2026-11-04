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

from xdiff_snap import (NODE, OBJ_ACTION, OBJ_EFFECT, OBJ_NAME_ID, OBJ_TYPE, RESERVE, ROOT, SIZE, SPAWN, Diff, Frame, assign_roles, is_ptr,
                        read_trace, role_name, u32)

XV_FIRST_ID = 0x200   # (src/emu/xvirus_recipe.h: the moved families' enemy ids from here)


def bn4_families():
    """BN4's families moved into BN6's engine, from their recipe (src/emu/xvirus_bn4.c, the generator's numbers): each its
    BN4 object type (.src_ai: its first-kind type, BN4 having no AI index) and its six BN4 enemy ids with BN6's for them
    (.names, .first_id: Billy's 13-18 BN6's 0x2C0-0x2C5)"""
    text = open(os.path.join(ROOT, 'src', 'emu', 'xvirus_bn4.c')).read()
    # (its effects: its XvId rows, BN4's id and BN6's, `static const XvId NAME_effects[] = { { 0x16, 0x98 }, ... }`)
    effects = {k: {int(a, 0): int(b, 0) for a, b in re.findall(r'\{ (0x[0-9A-Fa-f]+), (0x[0-9A-Fa-f]+) \}', rows)}
               for k, rows in re.findall(r'static const XvId (\w+)_effects\[\] = \{(.*?)\};', text, re.S)}
    out = []
    for src, first, names in re.findall(r'\.src_ai = (0x[0-9A-Fa-f]+),.*?\.first_id = XV_FIRST_ID \+ (0x[0-9A-Fa-f]+),.*?'
                                        r'\.names = \{ ([^}]*) \}', text, re.S):
        ids = [int(x, 0) for x in names.split(',')]
        out.append(dict(type=int(src, 0), ids={v: XV_FIRST_ID + int(first, 0) + k for k, v in enumerate(ids)}, effects={}))
    # (each family's effects by its name's rows: one family yet, Billy's; several matched to theirs by order)
    for f, rows in zip(out, effects.values()):
        f['effects'] = rows
    return out


XV_ART = 0x08B80000   # (src/emu/xvirus_recipe.h: the families' art in BN6's free space, xdiff_snap.XV_CODE's end)


def bn4_sprites(rom4):
    """the sprites BN4's moved families copied (src/emu/xvirus_bn4.c's XvSprite rows: BN4's list and number, BN6's copy's
    place and room): each (BN4's sprite's address, BN6's copy's, its room), BN4's from its sprite lists (src/core/rom.c's
    BN4 layout, sprite_lists) in its ROM; a pointer into either read by its offset in the sprite (its frames' data)"""
    text = open(os.path.join(ROOT, 'src', 'emu', 'xvirus_bn4.c')).read()
    rom_c = open(os.path.join(ROOT, 'src', 'core', 'rom.c')).read()
    m = re.search(r'\[XROM_BN4_RED_SUN_US\].*?\.sprite_lists = (0x[0-9A-Fa-f]+)', rom_c, re.S)
    if not m or not rom4:
        return []
    lists, out = 0x08000000 + int(m.group(1).rstrip('uU'), 0), []
    for lst, idx, off, room in re.findall(r'\{ (\d+), 0x([0-9A-Fa-f]+), \d+, 0x[0-9A-Fa-f]+, XV_ART \+ 0x([0-9A-Fa-f]+), 0x([0-9A-Fa-f]+) \}', text):
        base = u32(rom4, u32(rom4, lists - 0x08000000 + 4 * int(lst)) - 0x08000000 + 4 * int(idx, 16)) & 0x7FFFFFFF
        out.append((base, XV_ART + int(off, 16), int(room, 16)))
    return out


def sprite_ptr(sprites, game, v):
    """a pointer into a moved family's sprite (bn4_sprites) as ('sprite', which, its offset); None where it is none"""
    for i, (b4, b6, room) in enumerate(sprites):
        base = b4 if game == 4 else b6 if game == 6 else None
        if base is not None and base <= v < base + room:
            return ('sprite', i, v - base)
    return None


class Bn4Canon:
    """canon (xdiff_snap.Canon) with BN4's moved families' ids read as BN6's (bn4_families), where a BN4 trace is compared
    with BN6's; everything else canon's own"""

    def __init__(self, canon, fams):
        self.canon, self.ids = canon, {k: v for f in fams for k, v in f['ids'].items()}
        self.effects = {k: v for f in fams for k, v in f['effects'].items()}

    def nid(self, game, v):
        return self.ids.get(v, self.canon.nid(game, v)) if game == 4 else self.canon.nid(game, v)

    def fx(self, game, v):
        """an effect's id as BN6's: a moved family's BN4 effects as its recipe moved them (xvirus_bn4.c's XvId rows: Billy's
        0x16 and 0x15 BN6's 0x98 and 0x99, its move's flashes, from BN4's effects table 0x080E1698). Any BN4 effect of that
        number is read so; one the engine spawned would then meet BN6's own of its number and differ, never hide one"""
        return self.effects.get(v, v) if game == 4 else self.canon.fx(game, v)

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


def bn4_as_bn6(rec, fams):
    """an object's record (a T1's, from its node) with BN4's numbers as BN6's: its type, its CurAction"""
    types = {f['type'] for f in fams}
    if rec[NODE + OBJ_TYPE] in types:
        rec[NODE + OBJ_TYPE] = 0
    if rec[NODE + OBJ_ACTION] >= 6:
        rec[NODE + OBJ_ACTION] += 2
    return rec


# A collision data's alliance words (+0x30 and +0x34 in BN6's layout, BN4's +0x58 and +0x5C), each read by the same
# routine from its game's table by alliance and region (BN4 0x08012EF6 from 0x08012660, its 122 words; BN6 0x0801A0BA
# from BN6_COLL_MASKS, 0x08019C7C, 178): BN4's word read as BN6's entry at its place in BN4's table, where every place of
# it gives the same (BN6's first 122 are BN4's, 98 of them with bit 0x80: read from both ROMs, bn4_alliance)
BN4_MASKS, BN4_MASK_WORDS, BN6_MASKS = 0x08012660, 122, 0x08019C7C
COLL_MASKS = (0x30, 0x34)


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


def coll_as_bn6(coll, alliance):
    """a BN4 collision data (in BN6's layout) with its alliance words (bn4_alliance) and its last move's direction as
    BN6's"""
    out = bytearray(coll)
    for at in COLL_MASKS:
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
    if o.kind == 'T1':
        at = NODE + OBJ_NAME_ID
        rec[at:at + 2] = canon.nid(game, rec[at] | rec[at + 1] << 8).to_bytes(2, 'little')
        if game == 4 and fams is not None:
            rec = bn4_as_bn6(rec, fams)
    if game == 4 and fams is not None and o.role in reserved:
        rec[NODE] |= OBJ_RESERVED
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
                view = sprite_ptr(sprites, fa.game, wa) or fa.ptr(wa), sprite_ptr(sprites, fb.game, wb) or fb.ptr(wb)
                if view[0] != view[1]:
                    out.append((part, k, view[0], view[1]))
                k += 4
                continue
        a, b = ba[k] if k < len(ba) else None, bb[k] if k < len(bb) else None
        if a != b:
            out.append((part, k, a, b))
        k += 1
    return out


def frame_diffs(fa, fb):
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
    if fa.fr['rng'] != fb.fr['rng']:
        out.append(Diff(t, 'battle', 'rng', 0, fa.fr['rng'], fb.fr['rng']))
    sa, sb = sorted(s for _, s in fa.fr['sounds']), sorted(s for _, s in fb.fr['sounds'])
    if sa != sb:
        out.append(Diff(t, 'battle', 'sounds', 0, sa, sb))
    oa, ob = [fa.ptr(x) for x in fa.fr['order']], [fb.ptr(x) for x in fb.fr['order']]
    if oa != ob:
        out.append(Diff(t, 'battle', 'run order', 0, oa, ob))
    for i, (pa, pb) in enumerate(zip(fa.fr['panels'], fb.fr['panels'])):
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


def unexplained(fa, fb, ta, tb, used):
    """a frame's differences no BN4_LACKS mask explains (the masks applied counted into used)"""
    diffs = frame_diffs(fa, fb)
    for d in diffs:
        d.mask = lacks(d, ta, tb)
        if d.mask:
            used[d.mask] = used.get(d.mask, 0) + 1
    return [d for d in diffs if not d.mask]


def allowed_lag(spec):
    """the fight frames the family's own timeline may run later in BN6's engine: the spec's "lag", only where legit.json's
    bn4 allowed names exactly that lag for the family (its reason and both games' code paths there: BN6's engine hands a
    virus its first own state a fight frame later than BN4's); 0 none. Any other lag is refused, so a family off by any
    other amount fails"""
    lag = spec.get('lag', 0)
    if not lag:
        return 0
    ok = [a for a in bn4_allowed() if a.get('whose') == 'family' and a.get('lag') == lag]
    if not ok:
        raise SystemExit(f'lag {lag}: no allowed difference of tests/xdiff/legit.json (bn4) names it')
    return lag


def bn4_allowed():
    """legit.json's bn4 allowed differences (each with its reason and both games' code)"""
    return json.load(open(os.path.join(ROOT, 'tests', 'xdiff', 'legit.json'))).get('bn4', {}).get('allowed', [])


def allowed_bit(d, fam):
    """the allowed difference of legit.json's bn4 a family's difference is (a bit of its object BN6 has and BN4 not, the
    whole difference that bit), None where none: never on another's records, never more than its bit"""
    if d.role not in fam or not isinstance(d.a, int) or not isinstance(d.b, int) or isinstance(d.a, bool):
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
    fam = family_roles(ta, canon) | family_roles(tb, canon) if against else set()
    # (allowed_lag: the family's own records against BN6's of `lag` fight frames later, the world's of the same frame)
    lag = allowed_lag(spec) if against else 0
    stop, lines = None, []
    for fra in ta['frames']:
        frb = bt.get(fra['t'])
        if frb is None:
            continue
        n += 1
        fa = Frame(ta, fra, canon)
        open_ = unexplained(fa, Frame(tb, frb, canon), ta, tb, used)
        world = [d for d in open_ if d.role not in fam] if against else []
        if lag:
            frl = bt.get(fra['t'] + lag)
            mine = [d for d in unexplained(fa, Frame(tb, frl, canon), ta, tb, {}) if d.role in fam] if frl else []
        else:
            mine = [d for d in open_ if d.role in fam] if against else open_
        for d in mine:   # (legit.json's bn4 allowed bits of the family's own, each counted)
            d.mask = allowed_bit(d, fam)
            if d.mask:
                used[d.mask] = used.get(d.mask, 0) + 1
        mine = [d for d in mine if not d.mask]
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
    if len(ta['frames']) != len(tb['frames']):
        lines.append(f'  the traces are {len(ta["frames"])} and {len(tb["frames"])} fight frames long')
        return 'FAIL', min(len(ta['frames']), len(tb['frames'])), lines
    return 'PASS', None, lines


def spec_text(spec, out, game):
    """the spec's items as src/dev/xdiff_spec.c takes them, its battle BN4's (game: bn4:trace, bn4:bn6)"""
    keys = ','.join(f'{t}:{k}:{n}' for t, k, n in spec.get('keys', []))
    parts = [f'game={game}', f'foes={spec["foes"]}', f'mm={spec.get("mm", "22")}', f'hp={spec.get("hp", 100)}',
             f'frames={spec.get("frames", 1200)}', f'seed={spec.get("seed", "12345678:9ABCDEF0")}', f'out={out}']
    # (rng=replay: BN6's draws given BN4's, keyed as BN5's are, xdiff_events.h; poke=, call=: a mutation's break of the subject)
    extra = ([f'rng={spec["rng"]}'] if spec.get('rng') and game == 'bn4:bn6' else []) + \
        ([f'lag={spec["lag"]}'] if spec.get('lag') and game == 'bn4:bn6' else []) + \
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
        canon = Bn4Canon(canon, fams)
        canon.fams = fams
        rom4 = os.path.join(work, 'rom', 'bn4.gba')
        rom4b = open(rom4, 'rb').read() if os.path.exists(rom4) else b''
        rom6 = os.path.join(work, 'rom', 'bn6.gba')
        canon.sprites = bn4_sprites(rom4b)
        canon.alliance = bn4_alliance(rom4b, open(rom6, 'rb').read() if os.path.exists(rom6) else b'')
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
