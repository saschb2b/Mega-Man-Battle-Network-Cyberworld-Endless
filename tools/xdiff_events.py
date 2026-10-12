"""xdiff_events.py: what each core's hooks logged in the differential's traces (src/dev/xdiff_events.h; issue #82), for
tools/xdiff.py: each event's place in BN5's numbers (a family's code through what the install moved), the family's calls
compared by their call sites (its spawns, sounds, effects, setAttack, a panel reserved, its draws), the draws the replay
could not match, the objects each spawn made, and the engine's own bits of an object read in BN6's terms from BN5's
records: a panel reserved (BN6 marks the object, BN5 does not) and the blink's phase (the two hide a flashing object on
opposite frames). Each translation is the family's own record read as BN6 has it, never a mask (docs/DEVTOOLS.md, The
differential)."""
from xdiff_snap import (BLINK, DRAW, KEPT, MOOD, NODE, PLAYER_NAVI, REPLAYED, RESERVE, SPAWN, SPRITE_SET, SOUND, EFFECT, ATTACK, UNMAPPED,
                        UNMATCHED, Diff)

OBJ_RESERVED = 0x20   # an object's +0: BN6 sets it as the object first reserves a panel (BN6_OBJ_RESERVED)
OBJ_VISIBLE = 0x02    # ... drawn this frame: the blink hides a flashing object (BN5_BLINK_FLASHING, BN6_BLINK_FLASHING)
NAMES = {DRAW: 'draw', SPAWN: 'spawn', SOUND: 'sound', EFFECT: 'effect', ATTACK: 'setAttack', RESERVE: 'reserve', BLINK: 'blink',
         SPRITE_SET: 'sprite', MOOD: 'mood'}
NAVI_MOOD = PLAYER_NAVI + 0x0E   # his battle NaviStats' mood in the player block


def site(trace, a):
    """an address of the family's code as BN5 has it (BN6's copy through what the install moved), bit 0 clear; 0 none"""
    if not a:
        return 0
    a &= ~1
    if trace['game'] == 6:
        for frm, to, ln in trace['moved']:
            if to <= a < to + ln:
                return frm + (a - to)
    return a


def object_role(fr, addr):
    """the role of the object at addr in a frame's record (None: none in play there)"""
    return next((o.role for o in fr['objs'] if o.addr == addr), None)


# ---- the engine's bits read in BN6's terms (translations of BN5's own records) ----

class Translations:
    """BN5's reservations and blinks, frame by frame: an object that has reserved a panel reads BN6's bit (BN6 sets it
    at its first reservation and never clears it while the object lives, 0x0801BB36); an object the blink ran on reads
    its visible bit in BN6's phase (BN5 hides it where its flash timer's bit 1 is clear, 0x080137AE; BN6 where it is
    set, 0x0801695A); MegaMan's mood less what BN5's gains raised it by (BN5 raises it as a hit he lands is no counter,
    0x08017018 calling 0x080127FA; BN6's hit pass has no such call, 0x0801A228-0x0801A256, until the Souls track ports
    BN5's DarkChip rule). And in both games, an object spawned while the hooks watched that has set up no sprite since holds
    the slot's last occupant's sprite state (Obj.leftover), compared from its sprite's set-up on (xdiff_snap.obj_cells)."""

    def __init__(self):
        self.reserved = set()   # BN5's roles that have reserved a panel
        self.spawned = {5: set(), 6: set()}   # each game's roles the hooks saw spawn
        self.drawn = {5: set(), 6: set()}     # ... and set up a sprite since
        self.gained = 0                       # what BN5's mood gains raised MegaMan's mood by so far (BN6 has none)

    def apply(self, fa, fb):
        """BN5's frame `fa` (its objects' over) read in BN6's terms, each game's leftover sprite states told; the counts of
        bytes read so, by translation"""
        for f in (fa, fb):
            for e in f.fr['events']:
                role = object_role(f.fr, e.object)
                if role is not None and e.kind in (SPAWN, SPRITE_SET):
                    (self.spawned if e.kind == SPAWN else self.drawn)[f.game].add(role)
            for o in f.fr['objs']:
                o.leftover = o.role in self.spawned[f.game] and o.role not in self.drawn[f.game]
        n = {'reservation': 0, 'blink': 0, 'mood': 0}
        blinked = set()
        for e in fa.fr['events']:
            role = object_role(fa.fr, e.object)
            if e.kind == RESERVE and role is not None:
                self.reserved.add(role)
            elif e.kind == BLINK and role is not None:
                blinked.add(role)
            elif e.kind == MOOD and e.a == 0:
                self.gained += e.b
        # (BN5's mood as BN6's: less what its gains raised it by, where it is neither Full Synchro nor dark, whose rules both share)
        player = fa.fr.get('player')
        if player and self.gained and player[NAVI_MOOD] not in (0, 0xFF):
            fa.fr['player_over'] = {NAVI_MOOD: max(player[NAVI_MOOD] - self.gained, 1)}
            n['mood'] += 1
        for o in fa.fr['objs']:
            b = o.over.get(NODE, o.rec[NODE])
            if o.role in self.reserved:
                b |= OBJ_RESERVED
                n['reservation'] += 1
            if o.role in blinked:
                b ^= OBJ_VISIBLE
                n['blink'] += 1
            if b != o.rec[NODE]:
                o.over[NODE] = b
        return n


# ---- the family's calls by their call sites ----

def ran_for(fr, e):
    """the role of the object an event's code ran for: the running object (a spawn's, a draw's, a sound's, an effect's,
    setAttack's), the object acted on (a reservation's, a sprite's); None where none in play"""
    return object_role(fr, e.b if e.kind in (SPAWN, ATTACK) else e.object) if (e.b if e.kind in (SPAWN, ATTACK) else e.object) else None


def foreign(trace, role):
    """the object is one the trace attributed as not the family's (MegaMan's, a native's, the engine's): BN5's code a family
    moved (an object type of its engine's, a helper) runs for its own objects as well, where BN6 runs the family's copy for
    the family's objects alone"""
    return role is not None and role in trace.get('whose', {}) and trace['whose'][role] is not None


def own(f, e):
    """the event is the family's: its code led to it, for an object of the family's (or none)"""
    return bool(e.family) and not foreign(f.trace, ran_for(f.fr, e))


def family_calls(f):
    """The calls the family's code made in a frame (its events whose call chain passes through its code, for its own
    objects: own()), in the order made, in BN6's numbers: (what, its kind or N, its number, the family's call site as BN5
    has it)"""
    canon, game, out = f.canon, f.game, []
    for e in f.fr['events']:
        if not own(f, e) or e.kind == BLINK:
            continue
        at = site(f.trace, e.family)
        if e.kind == SPAWN:
            what = canon.t3(game, e.a) if e.sub == 3 else e.a
        elif e.kind == EFFECT:
            what = canon.fx(game, e.a)
        elif e.kind == ATTACK:
            # (a chip's family, the chip writer's for setAttack0, as BN6's of its chip: xdiff_snap's chip_action)
            o = next((x for x in f.fr['objs'] if x.addr == e.b), None) if game == 5 else None
            fam = canon.chip_action(o.ai, e.a) if o is not None else None
            what = fam if fam is not None else canon.action(game, False, e.a)
        elif e.kind == SOUND:
            what = canon.snd(game, e.a)
        else:
            what = 0
        out.append((NAMES[e.kind], e.sub if e.kind in (SPAWN, ATTACK, DRAW) else 0, what, f'{at:#x}'))
    return out


def call_diffs(fa, fb):
    """The family's calls of a frame that are not the same in both, by their place in each frame's list"""
    ca, cb, t, out = family_calls(fa), family_calls(fb), fa.fr['t'], []
    for k in range(max(len(ca), len(cb))):
        a = ca[k] if k < len(ca) else None
        b = cb[k] if k < len(cb) else None
        if a != b:
            out.append(Diff(t, 'family', 'call', k, a, b))
    return out


# ---- the replay's draws: those with no twin in the other game's frame ----

def matched_draws(tb):
    """the reference's draws the subject's replay took (their numbers + 1, as the events' ref)"""
    return {e.ref for fr in tb['frames'] for e in fr['events'] if e.kind == DRAW and e.flags & REPLAYED}


def draw_diffs(fa, fb, matched):
    """A frame's draws with no twin: the reference's kept and never replayed, the subject's unmatched or unmapped; the
    family's where its code led to it (they fail), else the battle's (the engine's own draws legit.json's rng names are
    explained by its mask)"""
    t, out = fa.fr['t'], []
    for e in fa.fr['events']:
        if e.kind == DRAW and e.flags & KEPT and e.ref not in matched:
            out.append(Diff(t, 'family' if own(fa, e) else 'battle', 'draw', e.sub, ('BN5', e.caller, site(fa.trace, e.family)), None))
    for e in fb.fr['events']:
        if e.kind == DRAW and e.flags & (UNMATCHED | UNMAPPED):
            out.append(Diff(t, 'family' if own(fb, e) else 'battle', 'draw', e.sub, None, ('BN6', e.caller, site(fb.trace, e.family))))
    return out


def engine_draw(d, legit):
    """the draw is the engine's own, made by one game alone where legit.json's rng names it (its caller's range)"""
    side = d.a if d.a is not None else d.b
    game = 5 if side[0] == 'BN5' else 6
    return any(o['game'] == game and int(o['from'], 0) <= side[1] & ~1 < int(o['to'], 0) for o in legit['rng']['only']['draws'])


# ---- the objects' spawns ----

def spawns_of(trace):
    """each role's spawn, where the trace logged it: {role: (its event, the role of the object running as it was
    spawned, or None)}; and into trace['effect_calls'] each effect object's call of the effect spawner (the effect event
    its spawn came of: its caller, the effect's id)"""
    out, trace['effect_calls'] = {}, {}
    for fr in trace['frames']:
        last = None
        for e in fr['events']:
            if e.kind == EFFECT:
                last = e
            elif e.kind == SPAWN and e.object:
                role = object_role(fr, e.object)
                if role is not None and role not in out:
                    out[role] = (e, object_role(fr, e.b) if e.b else None)
                    if last is not None and e.sub == 4:
                        trace['effect_calls'][role] = last
                last = None
    return out


def engine_chose(trace, role):
    """the effect object's id was the engine's code's to give: the effect spawner's caller (BN5_SPAWN_EFFECT's,
    BN6_SPAWN_EFFECT's) not the family's code, but a routine of its game's engine (one BN6 runs as its twin)"""
    e = trace.get('effect_calls', {}).get(role)
    return e is not None and (e.caller & ~1) != (e.family & ~1)


def spawned_by(trace, role):
    """whose code spawned an object: 'family' where the family's code led to its spawn for an object of the family's (or
    none: foreign()), else the role of the object that ran as it was spawned; None where neither is known (in play before
    the hooks, or spawned outside the runner)"""
    e, running = trace.get('spawns', {}).get(role, (None, None))
    if e is None:
        return None
    return 'family' if e.family and not foreign(trace, running) else running
