"""xdiff_bn4_opening.py: a moved object's animation run through the paused opening, for tools/xdiff_bn4.py's compare
(legit.json's bn4 opening_animation; docs/devtools/differential.md, The BN4 trace; docs/multirom/bn4-navis.md, ThunderMan
built).

An object whose init sets its animation in the battle's set-up and whose after-state runs the sprite update every frame its
routine runs, with its +0 0x04 through the pause (ThunderMan's cloud), animates through the whole opening in both games.
The two openings are not the same length (each engine's banner, each game's Custom screen as the harness passes it), so
each game's object enters the fight at its own place in its animation. Compared by that place: the same animation of the
same frames, each game's frames run of it as far apart as the frames each ran since its spawn (its opening and the fight's
frames to the record), modulo the animation's frames; exactly from an animation set in the fight on.

Imported by tools/xdiff_bn4.py alone. Numbers only."""
import json
import os

from xdiff_snap import ROOT, SPAWN, SPRITE

ANIM, ANIM_SET, COUNT, ANIM_ART, ANIM_FRAME = 0x90, 0x11, 0x91, 0xA8, 0xAC   # (a sprite state's fields: tools/xdiff_bn4.py's)
FRAME_SIZE, FRAME_DURATION, FRAME_FLAGS, LAST = 20, 16, 18, 0x80
SPRITE_STATE = 0x2C   # (the sprite state's bytes from SPRITE: its animation to its frame's data, 0x90-0xBB)


def rule():
    """legit.json's bn4 opening_animation: its name and the (kind, BN4's type) of the objects it holds for; None where none"""
    r = json.load(open(os.path.join(ROOT, 'tests', 'xdiff', 'legit.json'))).get('bn4', {}).get('opening_animation')
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
