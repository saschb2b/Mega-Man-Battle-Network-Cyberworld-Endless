"""xdiff_bn4_opening.py: a moved object's animation run through the paused opening, for tools/xdiff_bn4.py's compare
(legit.json's bn4 opening_animation; docs/devtools/differential.md, The BN4 trace; docs/multirom/bn4-navis.md, ThunderMan
built); and a Navi's companion's animation its init set in the paused opening, run by the fight's frames until a state sets
it again (legit.json's bn4 companion_clock, CompanionClock; docs/multirom/bn4-navis.md, AquaMan built).

An object whose init sets its animation in the battle's set-up and whose after-state runs the sprite update every frame its
routine runs, with its +0 0x04 through the pause (ThunderMan's cloud), animates through the whole opening in both games.
The two openings are not the same length (each engine's banner, each game's Custom screen as the harness passes it), so
each game's object enters the fight at its own place in its animation. Compared by that place: the same animation of the
same frames, each game's frames run of it as far apart as the frames each ran since its spawn (its opening and the fight's
frames to the record), modulo the animation's frames; exactly from an animation set in the fight on.

Imported by tools/xdiff_bn4.py alone. Numbers only."""
import os

import shards

from xdiff_snap import ROOT, SPAWN, SPRITE, T1_MOVED, obj_cells

ANIM, ANIM_SET, COUNT, ANIM_ART, ANIM_FRAME = 0x90, 0x11, 0x91, 0xA8, 0xAC   # (a sprite state's fields: tools/xdiff_bn4.py's)
FRAME_SIZE, FRAME_DURATION, FRAME_FLAGS, LAST = 20, 16, 18, 0x80
SPRITE_STATE = 0x2C   # (the sprite state's bytes from SPRITE: its animation to its frame's data, 0x90-0xBB)


def rule():
    """legit.json's bn4 opening_animation: its name and the (kind, BN4's type) of the objects it holds for; None where none"""
    r = shards.load('legit').get('bn4', {}).get('opening_animation')
    return (r['name'], {(o['kind'], int(o['type'], 0)) for o in r['objects']}) if r else None


class Opening:
    """each held object's frames run before the fight in each game (the core frames from its spawn to its first fight
    frame's record: its routine ran in every one, its +0 0x04 through the pause), and whether an animation was set since"""

    def __init__(self, ta, tb, sprites, rom4):
        self.name, self.kinds = rule() or (None, set())
        self.sprites, self.rom4 = sprites, rom4
        self.runs = {ta['game']: self.opening(ta), tb['game']: self.opening(tb)}
        self.first, self.done = {}, set()

    def held(self, role):
        return isinstance(role, tuple) and len(role) == 3 and isinstance(role[1], int) and (role[0], role[1]) in self.kinds

    def opening(self, trace):
        fr = trace['frames'][0] if trace['frames'] else None
        out = {}
        for e in (fr.get('setup', []) + fr['events']) if fr else ():
            role = next((o.role for o in fr['objs'] if o.addr == e.object), None)
            if e.kind == SPAWN and self.held(role):
                out[role] = fr['f'] - e.frame - fr['t']
        return out

    def see(self, *frames):
        """each held object's animation in these frames against its first: one changed was set in the fight"""
        for f in frames:
            for o in f.fr['objs']:
                if self.held(o.role):
                    now = (o.o8(ANIM), o.o8(ANIM_SET))
                    if self.first.setdefault((f.game, o.role), now) != now:
                        self.done.add(o.role)

    def place(self, o, game):
        """(its animation, the frames run of it, the frames it runs in all) by BN4's bytes of its sprite (BN6's copy the
        same bytes, its pointers read by their offset in the copy), None where it is not in one"""
        art, frame = (self.bn4_address(o.o32(x), game) for x in (ANIM_ART, ANIM_FRAME))
        rom, BUS = self.rom4, 0x08000000
        g32 = lambda a: int.from_bytes(rom[a - BUS:a - BUS + 4], 'little') if a and BUS <= a and a + 4 <= BUS + len(rom) else None
        if g32(art) is None or g32(art + 4 * o.o8(ANIM)) is None:
            return None
        at, run, where = art + g32(art + 4 * o.o8(ANIM)), 0, None
        for _ in range(256):
            if not BUS <= at or at + FRAME_SIZE > BUS + len(rom):
                return None
            dur = rom[at - BUS + FRAME_DURATION]
            if at == frame and o.o8(COUNT) < dur:
                where = run + dur - 1 - o.o8(COUNT)
            run += dur
            if rom[at - BUS + FRAME_FLAGS] & LAST:
                break
            at += FRAME_SIZE
        return (o.o8(ANIM), where, run) if where is not None and run else None

    def bn4_address(self, v, game):
        for b4, b6, room, _, _ in self.sprites:
            base = b4 if game == 4 else b6
            if b4 is not None and base is not None and base <= v < base + room:
                return b4 + v - base
        return None

    def phase(self, d, fa, fb):
        """legit.json's bn4 opening_animation: a held object's sprite state where no animation was set in the fight and each
        game's place in its animation is as far from the other's as the frames each ran since its spawn, modulo its frames"""
        if not self.name or d.part != 'obj' or not self.held(d.role) or d.role in self.done:
            return None
        if not SPRITE[d.role[0]] <= d.off < SPRITE[d.role[0]] + SPRITE_STATE:
            return None
        oa = next((o for o in fa.fr['objs'] if o.role == d.role), None)
        ob = next((o for o in fb.fr['objs'] if o.role == d.role), None)
        ra, rb = self.runs.get(fa.game, {}).get(d.role), self.runs.get(fb.game, {}).get(d.role)
        pa, pb = (self.place(oa, fa.game) if oa else None), (self.place(ob, fb.game) if ob else None)
        if pa is None or pb is None or ra is None or rb is None or pa[0] != pb[0] or pa[2] != pb[2]:
            return None
        ran = (ra + fa.fr['t']) - (rb + fb.fr['t'])
        return self.name if (pa[1] - pb[1] - ran) % pa[2] == 0 else None


def clock_rule():
    """legit.json's bn4 companion_clock: its name and the BN4 first-kind types of the companions it holds for; None where none"""
    r = shards.load('legit').get('bn4', {}).get('companion_clock')
    return (r['name'], {int(o['type'], 0) for o in r['objects'] if o['kind'] == 'T1'}) if r else None


class CompanionClock:
    """legit.json's bn4 companion_clock: a held Navi's companion (a first-kind object of a type his recipe moves, its role
    T1_MOVED | its BN4 type) whose sprite update its mode runs through the after-step at every fight frame (whose pause
    test skips the paused opening in both games), not through its Navi's state runner: its sprite state compared at the same
    fight frame until it first steps with the battle's offset (its animation set up again from its Navi's state), exactly
    from then on"""

    def __init__(self):
        self.name, self.types = clock_rule() or (None, set())
        self.done = set()

    def held(self, role):
        return isinstance(role, tuple) and len(role) > 1 and role[0] == 'T1' and isinstance(role[1], int) and \
            bool(role[1] & T1_MOVED) and role[1] & ~T1_MOVED in self.types

    @staticmethod
    def state_cells(fa, fb, oa, ob):
        lo = SPRITE[oa.kind]
        return [c for c in obj_cells(fa, fb, oa, ob) if c[0] == 'obj' and lo <= c[1] < lo + SPRITE_STATE]

    def see(self, fa, fb):
        """each held companion whose sprite state is the same at the battle's offset (fa, fb): its clock the state's from here"""
        for oa in fa.fr['objs']:
            ob = fb.by_role.get(oa.role)
            if self.held(oa.role) and oa.role not in self.done and ob is not None and not self.state_cells(fa, fb, oa, ob):
                self.done.add(oa.role)

    def holds(self, d, fa, fbt):
        """its name where d is a held companion's sprite state before it stepped with the offset, BN6's of BN4's fight frame
        (fbt) the same there"""
        if not self.name or fbt is None or d.part != 'obj' or not self.held(d.role) or d.role in self.done:
            return None
        if not SPRITE['T1'] <= d.off < SPRITE['T1'] + SPRITE_STATE:
            return None
        oa, ob = fa.by_role.get(d.role), fbt.by_role.get(d.role)
        if oa is None or ob is None:
            return None
        return None if any(c[1] == d.off for c in self.state_cells(fa, fbt, oa, ob)) else self.name
