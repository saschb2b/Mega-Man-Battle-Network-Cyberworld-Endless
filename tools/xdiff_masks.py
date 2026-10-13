"""xdiff_masks.py: whose a difference is, and the masks that explain it, for tools/xdiff.py (issue #82):
each role attributed from both traces (the family's, MegaMan's, a native's, the engine's), the renderer's places
of the family's objects read in the family's own order, and each mask of tests/xdiff/legit.json applied by its
place and its rule, never to the family's own records (docs/DEVTOOLS.md, The differential)."""
from xdiff_snap import (NODE, SIZE, SPRITE, OBJ_NAME_ID, OBJ_ACTION, OBJ_TYPE, OBJ_EFFECT, RNG_XOR, MEGAMAN_BUSTER, PLAYER_NAVI, NAVI_FORM,
                        PLAYER_SOULS, PLAYER_UNITE, Diff, Frame, icon_flag, role_name, u16, u32)
from xdiff_events import engine_chose, engine_draw, spawned_by, spawns_of
from xdiff_snap import DRAW

SPARK = 0x04   # the fourth kind's type of the engine's spark (BN6_SPAWN_SPARK's, sprite list 5's 5)
STEP_FAMILY = 0x10   # the step's attack family, the same in both: not a chip's
# the engine's spark spawner in each game (BN5 0x080E09B4, BN6_SPAWN_SPARK: the same 24 bytes but the allocator's call),
# which object_spawnCollisionEffect (BN5 0x08016C04, BN6 0x0801A0D4) calls where an attack struck: its spawn's caller
# (the allocator's return, BN5 0x080E09BF, BN6 0x080E1C0B). A spark spawned there is the hit's, its kind and place the
# attack's, its look each game's own (BN6's init runs its sprite a step on, 0x080E1BD0 bl 0x080026C4, which BN5's
# 0x080E0988 does not)
SPARK_SPAWNER = {5: (0x080E09B4, 0x18), 6: (0x080E1C00, 0x18)}
# The engine's hit area's routine (the third kind's 0x03: BN5 0x080C4DB8, BN6 0x080C6B20), its states run from there: a
# spawn whose outermost caller returns into it is the hit area's
HIT_AREA = {5: (0x080C4DB8, 0x080C4DC6), 6: (0x080C6B20, 0x080C6B2E)}


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


def spark_spawned(trace, role):
    """the object was spawned by the engine's spark spawner (SPARK_SPAWNER: its spawn's caller there)"""
    e = trace.get('spawns', {}).get(role, (None, None))[0]
    at, size = SPARK_SPAWNER[trace['game']]
    return e is not None and at <= e.caller & ~1 < at + size


def chip_running(f, role, canon):
    """the object of this role (the family's: a moved Navi) runs a chip as it spawns (its CurAction a family of BN6's
    numbers, not the step's and none moved: the chip's family from BN6's record, src/emu/xnavi_bn5.c's CURACTIONS), the
    spawn made by that family's code, no family's on the stack (spawned_by)"""
    o = f.by_role.get(role)
    if o is None or o.kind != 'T1' or not o.o16(OBJ_NAME_ID):
        return False
    a = canon.action(f.game, False, o.o8(OBJ_ACTION))
    return a > STEP_FAMILY and a not in canon.attack_families.values()


def whose(f, role, canon, known, depth=0):
    """'megaman' (his object), "megaman's" (what he owns: an object whose +0x4C names his or one of his, or an
    attack of his side with no object over it: his buster's shots), 'native' (an enemy both games have, each
    running its own code: a spec's natives) and "native's" (what it owns, and a spec's native attacks and effects
    that no object owns), 'engine' (an attack or effect no object owns, of a type the family's code does not
    spawn), 'panels' (the engine's spark over an actor, which BN5's lava spawns as it cools under it, and BN6's as
    src/emu/xpanels.c brings lava in: the code under test as the family's is, its presentation each game's own),
    'spark' (the engine's spark where an attack struck: spawned by the engine's spark spawner, which
    object_spawnCollisionEffect calls with the attack's hit effect or element, run by the engine's own code, the fourth
    kind's type 4 the install never moves, and drawn from each game's own sprite: its kind, place and owner the
    attack's, compared, its look each game's), 'effect' (an effect object, the fourth kind's type 0, the engine's code
    gave its id where the family's attack acted: a routine of its game's BN6 runs as its twin, the same bytes, called
    the effect spawner; its id, place and owner compared, its sprite each game's), "chip's" (what a chip the family's
    Navi uses spawns, BN6's chip family by BN6's record: its look each game's own, as MegaMan's chips', the rest
    compared: chip_running), or None (the family's: its enemy, what it owns, its attacks and the effects its own code
    gives). An object owned by another is its owner's, as the owner's own first frame had it (known)."""
    w, o = whose_of(f, role, canon, known, depth), f.by_role.get(role)
    if w is None and o is not None and o.kind == 'T4' and o.o8(OBJ_TYPE) == SPARK and spark_spawned(f.trace, role):
        return 'spark'
    if w is None and o is not None and o.kind == 'T4' and o.o8(OBJ_TYPE) == 0 and engine_chose(f.trace, role):
        return 'effect'
    return w


def whose_of(f, role, canon, known, depth):
    """whose() but for the engine's spark where an attack struck and the engine's effect for the family's attack"""
    if role_name(role).startswith(tuple(f'T1:{v:#x}#' for v in canon.megaman.values())):
        return 'megaman'
    if role[0] == 'T1' and role[1] in canon.natives:
        return 'native'
    o = f.by_role.get(role)
    if o is None or depth > 4:
        return None
    if o.kind == 'T1' and o.o16(OBJ_NAME_ID):
        return None
    if f'{role[0]}:{role[1]:#04x}' in canon.megaman_types if isinstance(role[1], int) else False:
        return "megaman's"   # (a spec's chip's objects whose spawn no hook sees: its megaman_types)
    p = f.ptr(o.o32(0x4C))
    # (the spark of a hit the family's code led to, its body's as its attacks': whose() names it the spark)
    if o.kind == 'T4' and o.o8(OBJ_TYPE) == SPARK and spawned_by(f.trace, role) == 'family' and spark_spawned(f.trace, role):
        return None
    # (a spark over an actor, not where an attack struck: the lava's spark, legit.json's masks, spawned by the actors'
    # pre-step, no family code on its way; before whose code spawned it, which is the actor's own step: MegaMan's on
    # lava would make it his)
    if o.kind == 'T4' and o.o8(OBJ_TYPE) == SPARK and len(p) == 3 and p[1] == 'obj' and p[0].startswith('T1:'):
        return 'panels'
    # (the spark of a hit spawned by the engine's object_spawnCollisionEffect as any attack struck, an element's among them:
    # its look each game's own, the rest of it the attack's: whose() names it the spark, legit.json's masks)
    if o.kind == 'T4' and o.o8(OBJ_TYPE) == SPARK and spark_spawned(f.trace, role):
        return None
    # (a spark where an attack struck, spawned by the engine's hit area, the third kind's 0x03 (BN6's own, audited:
    # tools/xchiprecipe.py, TYPE_AUDITS; often gone by the next frame) other than through object_spawnCollisionEffect: the
    # engine's, whose attack it marks)
    if o.kind == 'T4' and o.o8(OBJ_TYPE) == SPARK:
        e = f.trace.get('spawns', {}).get(role, (None, None))[0]
        lo, hi = HIT_AREA[f.game]
        if e is not None and lo <= (e.outer & ~1) < hi:
            return 'engine'
    # (whose code spawned it, where the hooks saw its spawn: the family's code, or the object that ran as it was spawned)
    by = spawned_by(f.trace, role)
    if by == 'family':
        return None
    if by is not None and by != role:
        w = known[by] if by in known else whose(f, by, canon, known, depth + 1)
        if w is None and chip_running(f, by, canon):
            return "chip's"
        return {'megaman': "megaman's", 'native': "native's"}.get(w, w)
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
    return f.canon.action(f.game, True, o.o8(OBJ_ACTION), o.ai) if o else None


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
NO_PLACE = 0x7FFF   # a family object's tile place before the renderer first draws it (none)
CACHE = 0x24        # a sprite state's frame copied for its place (0: none, the renderer passed it over)
SHARED = 0x4000     # a family object's tile place where BN5's renderer shares it with another's of the same sprite


def renderer_ranks(fa, fb, last):
    """the family's renderer places read as the family's own (legit.json, maps, renderer): the renderer hands each
    object it draws its tile place and palette slot in the order it draws, from running counts over everything drawn
    before it, so a family object drawn this frame has each read as its rank among those the family's objects drawn
    take, in the order they ran (two sharing one, as a frame or palette already loaded, share its rank; slot 15, none
    taken, stays). One not drawn keeps the place it was last drawn at, and the rank it had there (last: each game's
    role's last); one whose place moved undrawn keeps its place as it is, marked (0x8000) so it never equals a rank; one
    never drawn has no place the renderer gave it (what its sprite state holds is its slot's last occupant's or its
    set-up's): none in both (NO_PLACE, NO_PALETTE). Drawn is the renderer's own test: the object shown (its +0x00 bit
    0x02), its sprite's state shown (its +0x03 bit 0x02, BN5 0x081C7F9A-0x081C7FA6 / BN6 0x081D6532-0x081D653E), as an
    object can hide its sprite alone, and its frame copied or kept this frame (its cache set: the renderer clears an
    undrawn sprite's cache, BN6 0x081D653A-0x081D6544, an object visible yet passed over, NumberMan's companion, among
    them). A family object whose place in BN5 is one an object not the family's draws this frame (the same frame or
    palette, which the renderer keeps where it is: BN5's MegaMan's own art, the Dark Soul's buster's arm and muzzle,
    3:0x03 and 3:0x06, which BN6's MegaMan draws from its own) shares it there: that place not ranked in either game,
    read as SHARED (a palette slot as NO_PALETTE) in both."""
    others = [o for o in fa.fr['objs'] if whose_role(o.role, fa, fb) != 'family' and o.o8(0) & 2]
    shared = []
    for k in (0, 1):
        held = {places(o)[k] for o in others} - ({NO_PALETTE} if k else set())
        shared.append({o.role for o in fa.fr['objs'] if whose_role(o.role, fa, fb) == 'family' and places(o)[k] in held})
    for f in (fa, fb):
        fam = [o for o in f.fr['objs'] if whose_role(o.role, fa, fb) == 'family']
        drawn = [o for o in fam if o.o8(0) & 2 and o.rec[SPRITE[o.kind] + NODE + 3] & 2 and u32(o.rec, SPRITE[o.kind] + NODE + CACHE)]
        # (in the order the objects ran, the same in both games: each place or slot ranked by the first of the family's to
        # take it, so one given again to a family object, as a frame or palette already loaded by another drawn before it,
        # shares that one's rank, whatever else of each game's own drew between them)
        run = {}
        for n, a in enumerate(f.fr['order']):
            for o in drawn:
                if o.addr - NODE <= a < o.addr - NODE + SIZE[o.kind]:
                    run.setdefault(o.role, n)
        drawn.sort(key=lambda o: (run.get(o.role, len(f.fr['order'])), o.kind, o.slot))
        for k in (0, 1):
            taken = []
            for o in drawn:
                if o.role not in shared[k] and places(o)[k] not in taken and not (k and places(o)[k] == NO_PALETTE):
                    taken.append(places(o)[k])
            for o in fam:
                raw, key, rank = places(o)[k], (f.game, o.role, k), None
                sp = SPRITE[o.kind] + NODE
                if o.role in shared[k]:
                    if k:
                        o.over[sp + 0x15] = NO_PALETTE << 4 | o.rec[sp + 0x15] & 0xF
                    else:
                        o.over[sp + 8], o.over[sp + 9] = SHARED & 0xFF, SHARED >> 8
                    last.pop(key, None)
                    continue
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


PROXY = 8                    # BN6_FORM_PROXY: the form BN5's Souls stand behind in BN6's engine


def united(f):
    """MegaMan united with a Soul in this game's frame: BN5's form byte one of the Souls (legit.json, maps, form), BN6's the
    proxy with a Soul in the Souls' block (maps, souls)"""
    p = f.fr.get('player', b'')
    if len(p) < PLAYER_UNITE:
        return False
    form = p[PLAYER_NAVI + NAVI_FORM]
    return form in f.canon.form if f.game == 5 else form == PROXY and p[PLAYER_SOULS] != 0


def his_panel(d, fa, fb):
    """the difference is the panel BN6's MegaMan stands on, both games' MegaMan united"""
    if not (united(fa) and united(fb)) or not isinstance(d.role, str) or not d.role.startswith('panel '):
        return False
    mm = next((o for o in fb.fr['objs'] if o.kind == 'T1' and fb.canon.is_megaman(6, o.o16(OBJ_NAME_ID))), None)
    return mm is not None and d.role == f'panel {mm.o8(0x12)},{mm.o8(0x13)}'


def form_left(f):
    """BN6's MegaMan has left a form earlier in this battle (his form byte not 0 in a frame before, 0 in this one or after)"""
    tr = f.trace
    if 'form_left' not in tr:
        seen, tr['form_left'] = False, set()
        for fr in tr['frames']:
            p = fr.get('player', b'')
            if p and p[PLAYER_NAVI + NAVI_FORM]:
                seen = True
            elif seen:
                tr['form_left'].add(fr['t'])
    return f.fr['t'] in tr['form_left']


def in_ranges(ranges, game, caller):
    return any(int(r[f'bn{game}'], 0) <= caller & ~1 < int(r[f'bn{game}'], 0) + int(r['len'], 0) for r in ranges)


def per_frame(legit, game, caller):
    """a draw's caller is one of the routines each game draws from once a frame whatever its battle does (legit.json, rng,
    per_frame: the game state's and the main loop's), or the camera's shake, which draws each frame it shakes from each
    game's own word (rng, shake)"""
    return in_ranges(legit['rng'].get('per_frame', []) + legit['rng'].get('shake', []), game, caller)


def stepped(x, y, n):
    """y is x stepped n times by the games' RNG step (x is y stepped -n times where n is negative)"""
    if n < 0:
        x, y, n = y, x, -n
    for _ in range(n):
        x = rng_step(x)
    return x == y


def held_clock(d, fa, fb):
    """d is the RNG words' difference of the first traced frame after the battle's clock held for a number of frames each
    game's own (a turn's change, a unison's crossing): of the draws since the last traced frame, all but the per-frame ones
    (per_frame) the same in number on each word in both, and each word the other game's stepped by the per-frame draws one
    made more than the other"""
    if d.part != 'rng' or not isinstance(d.a, tuple) or not isinstance(d.b, tuple):
        return False
    legit, n, shakes = fa.canon.legit, {}, {5: 0, 6: 0}
    for f in (fa, fb):
        for e in f.fr['events']:
            if e.kind == DRAW and e.flags:   # (since the words were seeded: a draw before it has no flag, src/dev/xdiff_events.c)
                key = (f.game, e.sub >> 1, per_frame(legit, f.game, e.caller))
                n[key] = n.get(key, 0) + 1
                shakes[f.game] += in_ranges(legit['rng'].get('shake', []), f.game, e.caller)
    # (the camera shaken as long in both: as many of its draws, each game's from its own word)
    if shakes[5] != shakes[6] or any(n.get((5, w, False), 0) != n.get((6, w, False), 0) for w in (0, 1)):
        return False
    return all(stepped(d.a[w], d.b[w], n.get((6, w, True), 0) - n.get((5, w, True), 0)) for w in (0, 1))


def in_both(d, fa, fb):
    """the difference's object is one both games had in play at some frame (its role in both traces): an end a frame apart,
    not an object one game alone spawned (an effect of another id: its role another)"""
    return isinstance(d.role, tuple) and all(d.role in f.trace.get('whose', {}) for f in (fa, fb))


ATTACK_CHIP = 0xB4   # MegaMan's AI data: AttackVars +0x14, the chip used (legit.json, maps, chip_id)


def his_icon_flag(d, fb):
    """the bits BN6 has and BN5 not are BN6's hit flag for the icon element of the chip BN6's MegaMan uses (his AttackVars
    +0x14): BN6's chip use ORs it into the attack element (0x08012722), BN5's sets none (0x0800FEA4); his chip's objects carry
    it where they take the element (their +0x0E, their collision data's +0x19)"""
    a = 0 if d.a is None else d.a
    if not isinstance(a, int) or not isinstance(d.b, int) or a & ~d.b:
        return False
    d = Diff(d.t, d.role, d.part, d.off, a, d.b)
    mm = next((o for o in fb.fr['objs'] if o.kind == 'T1' and fb.canon.is_megaman(6, o.o16(OBJ_NAME_ID))), None)
    chip = u16(mm.ai, ATTACK_CHIP) if mm is not None and mm.ai is not None else None
    return chip is not None and d.b & ~d.a == icon_flag(fb.canon, chip) != 0


SUPPORT_FAMILY = 0x15   # the support chips' family: MegaMan's CurAction as one is used (BN6_CHIP_SUPPORT_FAMILY)
ATTACK_PHASE = 0xA1    # MegaMan's AI data: AttackVars +1, the attack's phase
SUPPORT_LAG = 12        # the frames after the family's end in which BN6's MegaMan may still lag BN5's by its one frame


def megaman_in(f):
    return next((o for o in f.fr['objs'] if o.kind == 'T1' and f.canon.is_megaman(f.game, o.o16(OBJ_NAME_ID))), None)


def support_lag(d, fa, fb, prev):
    """BN6's support family ends its attack a frame after BN5's (BN6 0x080ED12A: AttackVars +1 set, the end at its next frame,
    0x080ED130; BN5 0x080EC448 ends it at once): from BN5's end MegaMan's own byte in BN6's frame is the one BN5's frame
    before had (his object's in BN6's numbers, his AI data's), within SUPPORT_LAG frames of BN5's CurAction 0x15"""
    if d.role != ('T1', next(iter(fa.canon.megaman.values())), 0) or d.part not in ('obj', 'ai') or not prev:
        return False
    frames = fa.trace['frames']
    k = next((i for i, fr in enumerate(frames) if fr is fa.fr), None)
    if not k or not any(megaman_action(Frame(fa.trace, frames[j], fa.canon)) == SUPPORT_FAMILY for j in range(max(0, k - SUPPORT_LAG), k)):
        return False
    before = megaman_in(prev[0])
    if before is None:
        return False
    if d.part == 'ai' and d.off == ATTACK_PHASE and d.b == 1 and megaman_action(fb) == SUPPORT_FAMILY:
        return True   # (its mark of the routine run, AttackVars +1, in its extra frame)
    if d.part == 'ai':
        return before.ai is not None and before.ai[d.off] == d.b
    v = before.o8(d.off)
    return (fa.canon.action(5, True, v) if d.off == OBJ_ACTION else v) == d.b


def spawned_now(d, prev):
    """the difference's object new this frame: its role in neither game's frame before"""
    return not prev or not any(o.role == d.role for f in prev for o in f.fr['objs'])


WHEN = {
    'in both': lambda d, fa, fb, prev: in_both(d, fa, fb),
    'support lag': support_lag,
    'spawned now': lambda d, fa, fb, prev: spawned_now(d, prev),
    'icon flag': lambda d, fa, fb, prev: his_icon_flag(d, fb),
    'hand of ten': lambda d, fa, fb, prev: hand_of_ten(d, fa),
    'his panel': lambda d, fa, fb, prev: his_panel(d, fa, fb),
    'held clock': lambda d, fa, fb, prev: held_clock(d, fa, fb),
    'united': lambda d, fa, fb, prev: united(fa) and united(fb),
    'not united': lambda d, fa, fb, prev: not (united(fa) and united(fb)),
    'form left': lambda d, fa, fb, prev: form_left(fb),
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
    # (an effect object's role is its type 0 with its effect id: identity())
    if 'types' in m and (not isinstance(d.role, tuple) or
                         (d.role[1][0] if isinstance(d.role[1], tuple) else d.role[1]) not in [int(x, 0) for x in m['types']]):
        return False
    if 'families' in m and megaman_action(fb) not in [int(x, 0) for x in m['families']]:
        return False
    if 'effects' in m and (not isinstance(d.role, tuple) or not isinstance(d.role[1], tuple) or d.role[1][1] not in [int(x, 0) for x in m['effects']]):
        return False
    if 'whose' in m and who(d, fb, fa.canon, fa) not in m['whose']:
        return False
    if 'when' in m and not WHEN[m['when']](d, fa, fb, prev):
        return False
    if 'songs' in m:
        songs = {int(x, 0) for x in m['songs']}
        if not (set(d.a) - set(d.b) <= songs and not set(d.b) - set(d.a)):
            return False
    if 'song_for' in m:   # (BN5's song in place of BN6's: [BN5's, BN6's], the rest the same)
        a5, b6 = (int(x, 0) for x in m['song_for'])
        if sorted(b6 if x == a5 else x for x in d.a) != sorted(d.b):
            return False
    return True


PARTS = {}   # id(masks) -> (masks, {part: its masks in their order}): mask_applies' first test, made once a list


def by_part(masks, part):
    """the masks of `masks` whose place includes `part`, in their order (the rest never apply to its differences)"""
    held = PARTS.get(id(masks))
    if held is None or held[0] is not masks:
        index = {}
        for m in masks:
            for p in m['part'] if isinstance(m['part'], list) else [m['part']]:
                index.setdefault(p, []).append(m)
        held = PARTS[id(masks)] = (masks, index)
    return held[1].get(part, ())


def mask_of(d, masks, fa, fb, prev):
    """the mask (or masks, where each explains some of a byte's bits) that explains d; None where none does, and
    always where d is on the family's own records (what the engine writes there is read as BN6's in the snapshot,
    never masked). A mask's bits with "direction": "bn6" explain a bit only where BN6 has it and BN5 not ("bn5" the other way); a
    byte only BN6's layout has (BN5's None) is read as 0 by a mask that says "bn5_none": 0."""
    if who(d, fb, fa.canon, fa) == 'family':
        return None
    masks = by_part(masks, d.part)
    if d.a is None and isinstance(d.b, int) and any(m.get('bn5_none') == 0 and mask_applies(m, d, fa, fb, prev) for m in masks):
        d = Diff(d.t, d.role, d.part, d.off, 0, d.b)
    bits, names = 0, []
    for m in masks:
        if not mask_applies(m, d, fa, fb, prev):
            continue
        if 'bits' not in m:
            return m['name']
        mb = int(m['bits'], 0)
        if m.get('direction') == 'bn6' and (not isinstance(d.b, int) or (d.a ^ d.b) & mb & ~d.b):
            continue
        if m.get('direction') == 'bn5' and (not isinstance(d.a, int) or (d.a ^ d.b) & mb & ~d.a):
            continue
        bits |= mb
        names.append(m['name'])
    if names and isinstance(d.a, int) and isinstance(d.b, int) and not (d.a ^ d.b) & ~bits:
        return ' + '.join(names)
    return None
