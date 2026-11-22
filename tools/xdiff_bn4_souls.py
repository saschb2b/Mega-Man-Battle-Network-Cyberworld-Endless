"""xdiff_bn4_souls.py: a BN4 Soul's rules for tools/xdiff_bn4.py (docs/multirom/bn4-souls.md, The comparator's BN4 Souls).

A spec whose BN4 MegaMan holds a Soul (souls4) and whose BN6 MegaMan holds its move (souls6) compares two engines that keep a
Soul, and a charged buster, in different places and by different clocks. Each rule here reads one such difference as the
same state, from both games' code, and names it by its entry of tests/xdiff/legit/bn4/souls.json (its why and evidence
there); a difference no rule reads stays the spec's. The rules check the numbers they read as one another, so a Soul's
wrong turns, form, power attack, charge or level still parts:

  held          the battle's other actors held through BN4's unison by its actors' runner (time stopped: BN4_TIME_STOP): its held
                routine's first frame sets +0x18 to 4 and copies +0x36 and +0x3E to +0x30 and +0x32, kept after; BN6, crossing
                before the fight, holds none
  unison var    BN4's attack's variables' +5, 2 as it enters its unison (0x08013B1C-0x08013B22), BN6's crossing none: until his
                next state, where his attack's power attack writes it in both
  unison        BN4's unison (family 0x0D) runs as his state on the fight's clock, its last frame the fight's first; BN6's
                crossing ends before the fight. That frame's differences of MegaMan, his pending form, the unison's effect,
                the run order and his panel; and its three sounds, begun before the fight, said at the frame after
  ready clock   his ready state's first wait after it, begun two of BN4's fight frames after BN6's: his phase, its set-up
                byte and its count read against BN4's two frames on, until BN4's wait is done
  power         a moved Soul's power attack by each game's number (its loadout's, AI data +5 to +8)
  name          his NameID: BN6's by his form (the proxy's), BN4's one id in every form
  soul          the Soul's state: BN4's in his AI data (+0x18, +0x19) and NaviStats (+0x24), BN6's in its Souls block, held
                to each other (BN6's Soul the move of BN4's form, its turns BN4's left)
  pending       BN4 clears the pending form at each turn's start, BN6's record keeps the form taken
  form frames   BN4's timed form's frames, written for every form and counted for DarkSoul's alone; BN6 none
  shots         BN4's count of a form's charged shots, to 5; BN6 none
  presses       BN4's count of B's presses toward GutsSoul's rapid buster (+0x11 its window, +0x12 the presses, under six);
                BN6 counts none
  charge        the buster's charge: BN4's counters (+0x30, +0x32), levels (+0x0A, +0x0B: 0, 1, 3), button (+9) and the
                levels kept at the release (+0x14, +0x15); BN6's counter (+0x1B, held at the charge time), level (+0x1D: 0,
                1, 2), button (+0x1E) and request bits (+0x44's 0x20000 and 0x40000); each engine's own glow object"""
import os
import re

import shards
from xdiff_snap import NODE, OBJ_ACTION, ROOT

PLAYER_SOULS = 0x78            # (xdiff_snap.PLAYER_SOULS: BN6's Souls block, side 0, in the player block)
SOUL_ID, SOUL_TURNS = 0x00, 0x02   # (BN6_SOUL_ID, BN6_SOUL_TURNS: src/emu/bn6_forms.h)
BN4_NAVI_FORM = 0x24           # (bn4.h BN4_NAVI_FORM: the battle NaviStats' form, the BN4 player block's +0x24)
BN4_UNITE = 0x0D               # (bn4.h BN4_FAMILY_UNITE: MegaMan's state for the unison)
READY4 = 6                     # (BN4's MegaMan's ready state: bn4_twins.GENERIC4's own first)
HELD_FLAG, HELD = 0x18, 4      # (an actor's held routine, BN4 0x080104DC, BN6's twin 0x08016BFC: +0x18 4 on its first held frame,
HELD_COPY, HELD_FROM = range(0x30, 0x34), 6   # ... +0x36 and +0x3E copied to +0x30 and +0x32)
VARS5, UNISON_VAR5 = 0xA5, 2   # (his attack's variables' +5 at BN6's +0xA0 + 5; BN4's 2 as the unison begins, 0x08013B1C)
UNISON_SOUNDS = (0x129, 0x157, 0x158)   # (its phase 0's, 0x080EBABE; the picture's, 0x080E369A; its phase 1's, 0x080EBB32)
BN6_FORM_PROXY, BN6_FORM_NAME_BASE = 8, 0x1AB
MEGAMAN = ('T1', 0x1A0)
READY = (0x0A, 0x0B, 0x20)     # (an object's phase, its set-up byte, its phase's count)
READY_AHEAD = 2                # (BN6's ready state ahead of BN4's, in BN4's fight frames)
POWERS = (5, 6, 7, 8)          # (BN6_AI_POWER_CHIP, _A, _CHARGE, _C)
# BN4's AI data's bytes the trace lays where BN6 has no field there (src/dev/xdiff_bn4_trace.c's leftovers, in order from
# BN6_AI_SPARE: BN4's +5, +8, +9, +0x2C, +0x2D, ...): their places in BN6's units
SHOTS4, BUTTON4, FRAMES4 = 0xF0, 0xF2, 0xF3
TURNS_USED4, TURNS4 = 0x18, 0x19
COUNT_CHIP4, COUNT4 = 0x30, 0x32          # (BN4's counters, u16, at their own places)
LEVEL_CHIP4, LEVEL4 = 0x0A, 0x0B          # (BN4's levels)
KEPT_CHIP4, KEPT4 = 0x14, 0x15            # (... kept as B is let go, 0x0800BE48-0x0800BE4E)
TIME4 = 0xFE                              # (BN4's charged shot's charge time, its cache +0x36 at BN6_AI_SPARE_LAST + 2)
COUNT6, LEVEL6, BUTTON6, REQUEST6 = 0x1B, 0x1D, 0x1E, 0x46
LEVEL_AS6 = {0: 0, 1: 1, 3: 2}
WINDOW4, PRESSES4 = 0x11, 0x12            # (BN4's rapid buster's count, 0x0800BE5A-0x0800BE7A: a press sets the window to 10)
WINDOW6, PRESSES6 = 0x19, 0x18            # (... BN6's in his side's Souls block: BN6_SOUL_WINDOW, BN6_SOUL_PRESSES, soulrapid.c)
REQUESTS, RAPID4, CHARGED6, RAPID_AT = 0x44, 0x20, 0x02, 6   # (... at six asked: BN4's request 0x20, BN6's its charged B's 2)
CHAOS6_NONE = 0xFF                        # (BN6's +0x11, its chip's chaos power, 0xFF none: BN6_AI_POWER_CHIP's note)
GLOW4, GLOW6, GLOW_SLOT = ('T4', 0x05), ('T4', 0x08), 0x58


def rules():
    """tests/xdiff/legit/bn4/souls.json: each rule's name by its key"""
    return {r['key']: r['name'] for r in shards.load('legit').get('bn4', {}).get('souls', [])}


def bn4_forms():
    """BN4's moved Souls' forms by BN6's Soul number (src/emu/soulbn4.h SOUL_BN4_FIRST, soulbn4.c's forms)"""
    head = open(os.path.join(ROOT, 'src', 'emu', 'soulbn4.h')).read()
    body = open(os.path.join(ROOT, 'src', 'emu', 'soulbn4.c')).read()
    first = int(re.search(r'#define SOUL_BN4_FIRST (\d+)', head).group(1))
    forms = [int(x, 0) for x in re.search(r'forms\[SOULS_BN4\] = \{ ([^}]*) \}', body).group(1).split(',')]
    return {first + k: f for k, f in enumerate(forms)}


def c_entries(path, name):
    """how many entries the C array `name` of `path` (from src/) holds: its initializer's top-level items"""
    body = open(os.path.join(ROOT, 'src', *path.split('/'))).read()
    m = re.search(r'\b' + re.escape(name) + r'\[\] = \{(.*?)\n?\};', body, re.S)
    inner = re.sub(r'/\*.*?\*/', '', m.group(1), flags=re.S)
    if '{' in inner:
        return inner.count('{')
    return len([x for x in inner.split(',') if x.strip()])


def colonel_powers():
    """how many of soulcode_bn5.c's powers are Team Colonel's, those before BN4's in the grown table (Team ProtoMan's come
    after BN4's room, src/emu/soulpower.c): the recipe's npowers_colonel, its initializer's last number but for its overlays'
    pair (Team ProtoMan's, NULL, 0 where none)"""
    body = open(os.path.join(ROOT, 'src', 'emu', 'soulcode_bn5.c')).read()
    m = re.search(r'const SoulCode soulcode_bn5 = \{[^}]*?, (\d+),(?: (?:NULL, 0|overlays, N\(overlays\)),)?\s*\};', body, re.S)
    return int(m.group(1)) if m else c_entries('emu/soulcode_bn5.c', 'powers')


def bn6_power(form):
    """BN6's number of the charged shot of BN4's Soul of `form`, as src/emu/soulpower.c's tables() lays the grown table out:
    BN6's own (BN6_POWER_ATTACK_COUNT), then Team Colonel's moved ones (soulcode_bn5.c's powers, colonel_powers), their empty ones
    (soulpower.c's empty_powers) and their Chaos Unison's (chaos_powers), then BN4's moved Souls' in soulbn4.c's order. Read
    from the sources, so a Soul another track adds before them moves the number here as it does in the engine; None where
    the form is none of the moved ones"""
    head = open(os.path.join(ROOT, 'src', 'emu', 'bn6_attacks.h')).read()
    count = int(re.search(r'#define BN6_POWER_ATTACK_COUNT\s+(\d+)', head).group(1))
    k = {f: n for n, f in enumerate(bn4_forms().values())}.get(form)
    if k is None:
        return None
    return count + colonel_powers() + c_entries('emu/soulpower.c', 'empty_powers') + \
        c_entries('emu/soulpower.c', 'chaos_powers') + k


def power_map(rom4):
    """BN4's power attack -> BN6's for the moved Souls: each one's charged shot (BN4_FORM_LOADOUTS' +3 by its form, 0x080184F0)
    at bn6_power's number"""
    if not rom4:
        return {}
    return {rom4[0x184F0 + 4 * f + 3]: bn6_power(f) for f in bn4_forms().values()}


def form_frames(rom4):
    """BN4_FORM_FRAMES by form (0x08018088, a u16 each)"""
    return lambda form: int.from_bytes(rom4[0x18088 + 2 * form:0x18088 + 2 * form + 2], 'little') if rom4 else None


class Souls4:
    """the rules over one spec's two traces"""

    def __init__(self, ta, tb, rom4, lag):
        self.names, self.forms, self.powers, self.frames = rules(), bn4_forms(), power_map(rom4), form_frames(rom4)
        self.a, self.b, self.lag = {f['t']: f for f in ta['frames']}, {f['t']: f for f in tb['frames']}, lag
        self.unison_last = None   # (BN4's last frame of its unison)
        self.since_unison = False  # (BN4's MegaMan in no state but his ready one since it: unison var)
        self.held_from = {}        # (each actor's +0x36-+0x39 at BN4's last frame of its unison, which the held routine copied)

    @staticmethod
    def mm(fr):
        return next((o for o in fr['objs'] if isinstance(o.role, tuple) and o.role[:2] == MEGAMAN), None)

    def bn6_soul(self, fb):
        p = fb.fr.get('player', b'')
        return p[PLAYER_SOULS + SOUL_ID] if len(p) > PLAYER_SOULS + SOUL_TURNS else 0

    def united(self, fa, fb):
        """BN6's MegaMan in the proxy with the move of BN4's form, BN4's MegaMan in that form"""
        soul, pa = self.bn6_soul(fb), fa.fr.get('player', b'')
        return soul in self.forms and len(pa) > BN4_NAVI_FORM and pa[BN4_NAVI_FORM] == self.forms[soul]

    def in_unison(self, fa):
        o = self.mm(fa.fr)
        return o is not None and o.rec[NODE + OBJ_ACTION] == BN4_UNITE

    def see(self, fa):
        """a frame of BN4's looked at, in order"""
        o = self.mm(fa.fr)
        if self.in_unison(fa):
            self.unison_last = fa.fr['t']
            self.since_unison = True
            for x in fa.fr['objs']:
                if isinstance(x.role, tuple) and x.role[:2] != MEGAMAN:
                    self.held_from[x.role] = {k: x.rec[NODE + k + HELD_FROM] for k in HELD_COPY}
        elif o is not None and o.rec[NODE + OBJ_ACTION] != READY4:
            self.since_unison = False

    def mask(self, d, fa, fb):
        """the rule's name that reads d as the same state, None where none"""
        for key, rule in (('unison', self.unison), ('held', self.held), ('unison var', self.unison_var), ('ready clock', self.ready), ('power', self.power), ('name', self.name),
                          ('soul', self.soul), ('pending', self.pending), ('form frames', self.form_frames),
                          ('shots', self.shots), ('presses', self.presses), ('charge', self.charge)):
            if key in self.names and rule(d, fa, fb):
                return self.names[key]
        return None

    def checks(self, fa, fb, diff):
        """the Soul's state held across the engines: BN6's Soul the move of BN4's form, its turns BN4's left; each part
        as a difference of MegaMan's player block (which no rule reads) where it does not hold"""
        p6, pa, o = fb.fr.get('player', b''), fa.fr.get('player', b''), self.mm(fa.fr)
        if 'soul' not in self.names or len(p6) <= PLAYER_SOULS + SOUL_TURNS or len(pa) <= BN4_NAVI_FORM or o is None or o.ai is None \
                or self.in_unison(fa):
            return []
        soul, form = p6[PLAYER_SOULS + SOUL_ID], pa[BN4_NAVI_FORM]
        out = []
        if self.forms.get(soul, 0) != form and (soul in self.forms or form in self.forms.values()):
            out.append(diff(fa.fr['t'], 'player', 'soul', 0, form, soul))
        if soul in self.forms:
            left = (o.ai[TURNS4] - o.ai[TURNS_USED4]) & 0xFF if o.ai[TURNS_USED4] != 0xFF else 0
            if left != p6[PLAYER_SOULS + SOUL_TURNS]:
                out.append(diff(fa.fr['t'], 'player', 'soul turns', SOUL_TURNS, left, p6[PLAYER_SOULS + SOUL_TURNS]))
        return out

    # ---- the rules ----

    def unison(self, d, fa, fb):
        if self.in_unison(fa):
            o = self.mm(fa.fr)
            at = (o.rec[NODE + 0x12], o.rec[NODE + 0x13]) if o else None   # (his panel: an object's +0x12, +0x13)
            if (isinstance(d.role, tuple) and d.role[:2] == MEGAMAN) or d.role == 'player' or d.part in ('run order', 'in play') \
                    or (at is not None and d.role == f'panel {at[0]},{at[1]}'):
                return True
            # (another object held through it: its record BN6's of the fight frame before, its byte the same there)
            before = self.b.get(fa.fr['t'] + self.lag - 1)
            ob = next((x for x in before['objs'] if x.role == d.role), None) if before and isinstance(d.role, tuple) else None
            return ob is not None and d.part == 'obj' and isinstance(d.a, int) and not isinstance(d.a, bool) and \
                ob.rec[NODE + d.off] == d.a
        # (its sounds, begun before the fight, said at the frame after its last)
        return d.part == 'sounds' and self.unison_last is not None and fa.fr['t'] == self.unison_last + 1 and \
            isinstance(d.a, list) and set(d.a) <= set(UNISON_SOUNDS) and not d.b

    def held(self, d, fa, fb):
        if self.unison_last is None or not isinstance(d.role, tuple) or d.role[:2] == MEGAMAN or d.part != 'obj':
            return False
        oa = next((x for x in fa.fr['objs'] if x.role == d.role), None)
        first = next((x for x in self.first_b() if x.role == d.role), None)
        if oa is None or first is None:
            return False
        if d.off == HELD_FLAG:   # (at the unison's frame: held, BN4's 4 where BN6's runs it as ever)
            return self.in_unison(fa) and d.a == HELD and d.b == first.rec[NODE + d.off]
        # (its +0x30-+0x33 BN4's copies of its own +0x36-+0x39, as they are or as they were when its time stopped last (a push
        # since changes them), BN6's as it began)
        copied = (oa.rec[NODE + d.off + HELD_FROM], self.held_from.get(d.role, {}).get(d.off))
        return d.off in HELD_COPY and oa.rec[NODE + d.off] in copied and d.b == first.rec[NODE + d.off]

    def first_b(self):
        return self.b[min(self.b)]['objs'] if self.b else []

    def unison_var(self, d, fa, fb):
        ai = self.ai_bytes(d, fa, fb)
        return ai is not None and self.since_unison and d.off == VARS5 and ai[0][VARS5] == UNISON_VAR5 and ai[1][VARS5] == 0

    def ready(self, d, fa, fb):
        if self.unison_last is None or not (isinstance(d.role, tuple) and d.role[:2] == MEGAMAN) or d.part != 'obj' or d.off not in READY:
            return False
        later = self.a.get(fa.fr['t'] + READY_AHEAD)
        o, lo = self.mm(fa.fr), self.mm(later) if later else None
        if o is None or lo is None or o.rec[NODE + 0x0A] or lo.rec[NODE + OBJ_ACTION] != o.rec[NODE + OBJ_ACTION]:
            return False   # (BN4's first wait done, or the state another two frames on)
        return lo.rec[NODE + d.off] == d.b

    def power(self, d, fa, fb):
        return isinstance(d.role, tuple) and d.role[:2] == MEGAMAN and d.part == 'ai' and d.off in POWERS and \
            self.powers.get(d.a) == d.b and self.united(fa, fb)

    def name(self, d, fa, fb):
        return isinstance(d.role, tuple) and d.role[:2] == MEGAMAN and d.part == 'obj' and d.off == 0x28 and d.a == MEGAMAN[1] & 0xFF and \
            d.b == (BN6_FORM_NAME_BASE + BN6_FORM_PROXY) & 0xFF and self.united(fa, fb)

    def ai_bytes(self, d, fa, fb):
        """(BN4's AI data, BN6's) of MegaMan where d is his AI data's, None else"""
        if not (isinstance(d.role, tuple) and d.role[:2] == MEGAMAN and d.part == 'ai'):
            return None
        oa, ob = self.mm(fa.fr), self.mm(fb.fr)
        return (oa.ai, ob.ai) if oa and ob and oa.ai and ob.ai else None

    def word_bytes(self, d, fa, fb, explain):
        """d a byte, or a word read whole (a pointer's look), each byte of it explained by explain(offset, BN4's AI, BN6's)"""
        ai = self.ai_bytes(d, fa, fb)
        if ai is None:
            return False
        a, b = ai
        n = 4 if isinstance(d.a, tuple) or isinstance(d.b, tuple) else 1
        return all(a[k] == b[k] or explain(k, a, b) for k in range(d.off, min(d.off + n, len(a), len(b))))

    def soul(self, d, fa, fb):
        # (BN4's turns and their count at its own places, BN6's 0 there: its Souls block holds them, checks())
        return self.united(fa, fb) and self.word_bytes(d, fa, fb, lambda k, a, b: k in (TURNS_USED4, TURNS4) and b[k] == 0 or
                                                       self.charge_byte(k, a, b))

    def pending(self, d, fa, fb):
        return d.role == 'player' and d.part == 'pending form' and d.a == 0xFF and d.b == BN6_FORM_PROXY and self.united(fa, fb)

    def form_frames(self, d, fa, fb):
        ai = self.ai_bytes(d, fa, fb)
        if ai is None or d.off not in (FRAMES4, FRAMES4 + 1) or not self.united(fa, fb):
            return False
        a, b = ai
        return int.from_bytes(a[FRAMES4:FRAMES4 + 2], 'little') == self.frames(fa.fr['player'][BN4_NAVI_FORM]) and b[d.off] == 0

    def shots(self, d, fa, fb):
        ai = self.ai_bytes(d, fa, fb)
        return ai is not None and d.off == SHOTS4 and 0 < ai[0][SHOTS4] <= 5 and ai[1][SHOTS4] == 0 and self.united(fa, fb)

    def presses(self, d, fa, fb):
        ai = self.ai_bytes(d, fa, fb)
        if ai is None or d.off not in (WINDOW4, PRESSES4, REQUESTS) or not self.united(fa, fb):
            return False
        a, b = ai
        p6 = fb.fr.get('player', b'')
        if len(p6) <= PLAYER_SOULS + WINDOW6:
            return False
        if d.off == REQUESTS:   # (the rapid buster asked: BN4's 0x20, BN6's 2 beside what else each holds, his count at six)
            return bool(a[REQUESTS] & RAPID4) and p6[PLAYER_SOULS + PRESSES6] >= RAPID_AT and \
                (a[REQUESTS] & ~RAPID4) | CHARGED6 == b[REQUESTS]
        kept = p6[PLAYER_SOULS + (WINDOW6 if d.off == WINDOW4 else PRESSES6)]
        return a[d.off] == kept and b[d.off] == (CHAOS6_NONE if d.off == WINDOW4 else 0)

    def charge_byte(self, k, a, b):
        """a byte of MegaMan's AI data where the engines keep the charge apart, each reading as the other"""
        count, time = int.from_bytes(a[COUNT4:COUNT4 + 2], 'little'), int.from_bytes(a[TIME4:TIME4 + 2], 'little')
        level = LEVEL_AS6.get(a[LEVEL4])
        same = b[COUNT6] == min(count, time) and level == b[LEVEL6] and a[BUTTON4] == b[BUTTON6] and \
            int.from_bytes(a[COUNT_CHIP4:COUNT_CHIP4 + 2], 'little') == 0 and a[LEVEL_CHIP4] == 0
        if k in (COUNT6, LEVEL6, BUTTON6):
            return same and a[k] == 0
        if k in (COUNT4, COUNT4 + 1, LEVEL4, BUTTON4):
            return same and b[k] == 0
        if k == REQUEST6:   # (BN6's key handler's request for B's charge (0x04) or A's (0x02), while that button charges)
            return a[k] == 0 and b[k] == {0: 0, 1: 0x02, 2: 0x04}.get(b[BUTTON6], -1)
        if k in (KEPT_CHIP4, KEPT4):   # (BN4's levels kept as B is let go, BN6 none)
            return b[k] == 0 and a[k] in (0, 1, 3)
        return False

    def charge(self, d, fa, fb):
        if self.word_bytes(d, fa, fb, self.charge_byte):
            return True
        # (each engine's glow: BN4's in play while his charge has a level, BN6's the whole battle; his AI data's word for it)
        if isinstance(d.role, tuple) and d.role[:2] == GLOW4 and d.part == 'in play' and d.a and not d.b:
            return True
        if isinstance(d.role, tuple) and d.role[:2] == MEGAMAN and d.part == 'ai' and d.off == GLOW_SLOT:
            return (d.a == ('raw', 0) or (isinstance(d.a, tuple) and d.a[0].startswith('T4:0x5#'))) and \
                isinstance(d.b, tuple) and d.b[0].startswith('T4:0x8#')
        if d.part == 'run order' and isinstance(d.a, list) and isinstance(d.b, list):
            return [x for x in d.a if not (isinstance(x, tuple) and x[0].startswith('T4:0x5#'))] == d.b
        return False
