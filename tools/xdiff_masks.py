"""xdiff_masks.py: whose a difference is, and the masks that explain it, for tools/xdiff.py (issue #82):
each role attributed from both traces (the family's, MegaMan's, a native's, the engine's), the renderer's places
of the family's objects read in the family's own order, and each mask of tests/xdiff/legit.json applied by its
place and its rule, never to the family's own records (docs/DEVTOOLS.md, The differential)."""
from xdiff_snap import (NODE, SPRITE, OBJ_NAME_ID, OBJ_ACTION, OBJ_TYPE, OBJ_EFFECT, RNG_XOR, MEGAMAN_BUSTER, Frame,
                        role_name, u16)
from xdiff_events import engine_draw, spawned_by, spawns_of


# ---- masks: engine differences, each explained (legit.json) ----

def flashing(f, role):
    """the object of this role flashes after a hit (its collision data's flash timer), or the object that owns it
    (its +0x4C) does"""
    o = f.by_role.get(role)
    if o is None:
        return False
    if o.coll is None:
        p = f.ptr(o.o32(0x4C)) if o.kind != 'T1' or not o.o16(OBJ_NAME_ID) else ('raw',)
        owner_ = next((x for x in f.fr['objs'] if role_name(x.role) == p[0]), None) if len(p) == 3 else None
        return owner_ is not None and owner_.role != role and flashing(f, owner_.role)
    off = f.canon.layouts['collision']['flash_timer'][0 if f.game == 5 else 1]
    return u16(o.coll, off) != 0


def whose(f, role, canon, known, depth=0):
    """'megaman' (his object), "megaman's" (what he owns: an object whose +0x4C names his or one of his, or an
    attack of his side with no object over it: his buster's shots), 'native' (an enemy both games have, each
    running its own code: a spec's natives) and "native's" (what it owns, and a spec's native attacks and effects
    that no object owns), 'engine' (an attack or effect no object owns, of a type the family's code does not
    spawn), or None (the family's: its enemy, what it owns, its attacks and effects). An object owned by another
    is its owner's, as the owner's own first frame had it (known)."""
    if role_name(role).startswith(tuple(f'T1:{v:#x}#' for v in canon.megaman.values())):
        return 'megaman'
    if role[0] == 'T1' and role[1] in canon.natives:
        return 'native'
    o = f.by_role.get(role)
    if o is None or depth > 4:
        return None
    if o.kind == 'T1' and o.o16(OBJ_NAME_ID):
        return None
    # (whose code spawned it, where the hooks saw its spawn: the family's code, or the object that ran as it was spawned)
    by = spawned_by(f.trace, role)
    if by == 'family':
        return None
    if by is not None and by != role:
        w = known[by] if by in known else whose(f, by, canon, known, depth + 1)
        return {'megaman': "megaman's", 'native': "native's"}.get(w, w)
    p = f.ptr(o.o32(0x4C))
    if len(p) == 3 and p[1] in ('obj', 'ai'):
        up = next((x.role for x in f.fr['objs'] if role_name(x.role) == p[0]), None)
        if up is not None and up != role:
            w = known[up] if up in known else whose(f, up, canon, known, depth + 1)
            return {'megaman': "megaman's", 'native': "native's"}.get(w, w)
    if o.kind == 'T4' and o.o8(OBJ_TYPE) == 0:
        fx = canon.fx(f.game, o.o8(OBJ_EFFECT))
        return None if fx in canon.family_effects else "native's" if fx in canon.native_effects else 'engine'
    if o.kind == 'T3' and o.o8(0x16) == 0:
        return "megaman's"
    if o.kind == 'T3':
        t3 = canon.t3(f.game, o.o8(OBJ_TYPE))
        return None if t3 in canon.t3_type.values() else "native's" if t3 in canon.native_attacks else 'engine'
    return None


def attribute(trace, canon):
    """each role's whose(), as its first frame has it (where what owns it is still there), kept for its life"""
    trace['spawns'] = spawns_of(trace)
    trace['whose'] = {}
    for fr in trace['frames']:
        f = Frame(trace, fr, canon)
        for o in fr['objs']:
            if o.role not in trace['whose']:
                trace['whose'][o.role] = whose(f, o.role, canon, trace['whose'])


def whose_role(role, fa, fb):
    """whose a role is as both traces attribute it (attribute()): 'family' where the reference calls it the
    family's, where the two disagree, or where the subject alone has it and calls it the engine's (the subject's
    numbers are the transplant's own: a wrong id must not be its alibi); else what they agree on"""
    ws = {g: f.trace['whose'][role] for g, f in ((5, fa), (6, fb)) if f is not None and role in f.trace['whose']}
    if not ws or None in ws.values() or len(set(ws.values())) > 1 or ws == {6: 'engine'}:
        return 'family'
    return next(iter(ws.values()))


def who(d, fb, canon, fa=None):
    """whose a difference is: the family's (its calls, its draws), 'battle' (no object's: the RNG, the songs, the run
    order, a panel, the engine's draws) or its role's"""
    if d.role == 'family':
        return 'family'
    if d.role == 'player':
        return 'megaman'
    return whose_role(d.role, fa, fb) if isinstance(d.role, tuple) else 'battle'


def rng_step(x):
    return (((x << 1) | (x >> 31)) + 1 & 0xFFFFFFFF) ^ RNG_XOR


def megaman_action(f):
    o = next((x for x in f.fr['objs'] if x.kind == 'T1' and f.canon.is_megaman(f.game, x.o16(OBJ_NAME_ID))), None)
    return f.canon.action(f.game, True, o.o8(OBJ_ACTION)) if o else None


def buster_draw(d, fa, fb, prev):
    """the frame MegaMan's buster fires, and BN5's first word one step on from BN6's, the second the same"""
    (a1, a2), (b1, b2) = d.a, d.b
    return (a2 == b2 and a1 == rng_step(b1) and megaman_action(fb) == MEGAMAN_BUSTER and
            (not prev or megaman_action(prev[1]) != MEGAMAN_BUSTER))


def offsets_of(m, d):
    """the places a mask names, as object offsets: its own, its sprite's (from the sprite's state in the object), ranges A-B"""
    out = set()
    for x in m.get('offsets', []):
        a, _, b = x.partition('-')
        out |= set(range(int(a, 0), int(b or a, 0) + 1))
    if 'sprite_offsets' in m and isinstance(d.role, tuple):
        for x in m['sprite_offsets']:
            a, _, b = x.partition('-')
            out |= {SPRITE[d.role[0]] + k for k in range(int(a, 0), int(b or a, 0) + 1)}
    return out


def places(o):
    """an object's renderer's places: its tile place (its sprite's +0x08) and palette slot (+0x15's high nibble)"""
    sp = SPRITE[o.kind] + NODE
    return u16(o.rec, sp + 8), o.rec[sp + 0x15] >> 4


NO_PALETTE = 15     # the palette slot the renderer leaves where it takes none


def renderer_ranks(fa, fb, last):
    """the family's renderer places read as the family's own (legit.json, maps, renderer): the renderer hands each
    object it draws its tile place and palette slot in the order it draws, from running counts over everything drawn
    before it, so a family object drawn this frame has each read as its rank among those the family's objects drawn
    take (two sharing one, as a frame or palette already loaded, share its rank; slot 15, none taken, stays). One not
    drawn keeps the place it was last drawn at, and the rank it had there (last: each game's role's last); one never
    drawn, or whose place moved undrawn, keeps its place as it is, marked (0x8000) so it never equals a rank."""
    for f in (fa, fb):
        fam = [o for o in f.fr['objs'] if whose_role(o.role, fa, fb) == 'family']
        drawn = [o for o in fam if o.o8(0) & 2]
        for k in (0, 1):
            taken = sorted({places(o)[k] for o in drawn} - ({NO_PALETTE} if k else set()))
            for o in fam:
                raw, key, rank = places(o)[k], (f.game, o.role, k), None
                if k and raw == NO_PALETTE:
                    continue
                if o in drawn:
                    rank = taken.index(raw)
                    last[key] = (raw, rank)
                elif key in last and last[key][0] == raw:
                    rank = last[key][1]
                sp = SPRITE[o.kind] + NODE
                if k:
                    o.over[sp + 0x15] = (rank if rank is not None else raw) << 4 | o.rec[sp + 0x15] & 0xF
                else:
                    v = rank if rank is not None else 0x8000 | raw
                    o.over[sp + 8], o.over[sp + 9] = v & 0xFF, v >> 8


WHEN = {
    'flashing': lambda d, fa, fb, prev: any(flashing(f, d.role) for f in (fa, fb) + prev),
    'buster draw': buster_draw,
    'engine draw': lambda d, fa, fb, prev: engine_draw(d, fa.canon.legit),
}


def cell_match(m, d):
    """the difference is one of the mask's cells: (offset, BN5's value, BN6's; null where a game has no such byte)"""
    val = lambda x: None if x is None else int(x, 0)
    return any(int(o, 0) == d.off and val(a) == d.a and val(b) == d.b for o, a, b in m['cells'])


def mask_applies(m, d, fa, fb, prev):
    """whether mask m's place and conditions take in difference d (its bits aside)"""
    if d.part not in (m['part'] if isinstance(m['part'], list) else [m['part']]):
        return False
    if ('offsets' in m or 'sprite_offsets' in m) and d.off not in offsets_of(m, d):
        return False
    if 'cells' in m and not cell_match(m, d):
        return False
    if 'kinds' in m and (not isinstance(d.role, tuple) or d.role[0] not in m['kinds']):
        return False
    if 'types' in m and (not isinstance(d.role, tuple) or d.role[1] not in [int(x, 0) for x in m['types']]):
        return False
    if 'whose' in m and who(d, fb, fa.canon, fa) not in m['whose']:
        return False
    if 'when' in m and not WHEN[m['when']](d, fa, fb, prev):
        return False
    if 'songs' in m:
        songs = {int(x, 0) for x in m['songs']}
        if not (set(d.a) - set(d.b) <= songs and not set(d.b) - set(d.a)):
            return False
    return True


def mask_of(d, masks, fa, fb, prev):
    """the mask (or masks, where each explains some of a byte's bits) that explains d; None where none does, and
    always where d is on the family's own records (what the engine writes there is read as BN6's in the snapshot,
    never masked). A mask's bits with "direction": "bn6" explain a bit only where BN6 has it and BN5 not."""
    if who(d, fb, fa.canon, fa) == 'family':
        return None
    bits, names = 0, []
    for m in masks:
        if not mask_applies(m, d, fa, fb, prev):
            continue
        if 'bits' not in m:
            return m['name']
        mb = int(m['bits'], 0)
        if m.get('direction') == 'bn6' and (not isinstance(d.b, int) or (d.a ^ d.b) & mb & ~d.b):
            continue
        bits |= mb
        names.append(m['name'])
    if names and isinstance(d.a, int) and isinstance(d.b, int) and not (d.a ^ d.b) & ~bits:
        return ' + '.join(names)
    return None
