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
  step          his step out of an attack's lag, each engine's own: BN4's state 9 (its 7) by the panel, BN6's 0x10 by the
                direction, and the variables each sets, while both stand in it
  charge        the buster's charge: BN4's counters (+0x30, +0x32), levels (+0x0A, +0x0B: 0, 1, 3), button (+9) and the
                levels kept at the release (+0x14, +0x15); BN6's counter (+0x1B, held at the charge time), level (+0x1D: 0,
                1, 2), button (+0x1E) and request bits (+0x44's 0x20000 and 0x40000); each engine's own glow object"""
import os
import re

import shards
from xdiff_snap import NODE, OBJ_ACTION, ROOT, SPRITE, T1_MOVED, rom

PLAYER_SOULS = 0x78            # (xdiff_snap.PLAYER_SOULS: BN6's Souls block, side 0, in the player block)
SOUL_ID, SOUL_TURNS = 0x00, 0x02   # (BN6_SOUL_ID, BN6_SOUL_TURNS: src/emu/bn6_forms.h)
BN4_NAVI_FORM = 0x24           # (bn4.h BN4_NAVI_FORM: the battle NaviStats' form, the BN4 player block's +0x24)
BN4_UNITE = 0x0D               # (bn4.h BN4_FAMILY_UNITE: MegaMan's state for the unison)
READY4 = 6                     # (BN4's MegaMan's ready state: bn4_twins.GENERIC4's own first)
HELD_FLAG, HELD = 0x18, 4      # (an actor's held routine, BN4 0x080104DC, BN6's twin 0x08016BFC: +0x18 4 on its first held frame,
HELD_COPY, HELD_FROM = range(0x30, 0x34), 6   # ... +0x36 and +0x3E copied to +0x30 and +0x32)
VARS5, UNISON_VAR5 = 0xA5, 2   # (his attack's variables' +5 at BN6's +0xA0 + 5; BN4's 2 as the unison begins, 0x08013B1C)
UNISON_SOUNDS = (0x129, 0x157, 0x158)   # (its phase 0's, 0x080EBABE; the picture's, 0x080E369A; its phase 1's, 0x080EBB32)
UNISON_DECO_SOUNDS = (0xAD,)   # (... and the picture's decoration's: WindSoul's swirl, the third kind's 0x5A, 0x080D8C11's 0x121, BN6's
                               # number of it the wind's start's, sounds 045)
STEP4, STEP6 = 0x07, 0x10       # (his step's state, each engine's: BN4's 7 by BN4_STEP_ASK_START's movs r0,#7, the trace's 9 as it
                                 # reads BN4's own from 6 as BN6's from 8; BN6's 0x10 by BN6_STEP_ASK_START's movs r0,#0x10)
STEP_VARS = (0xA3, 0xAC, 0xB6, 0xB7, 0xB8, 0xB9, 0xCC, 0xCD, 0xCE, 0xCF)   # (... and his attack's variables it sets, at BN6's +0xA0:
                                 # BN4's +0x16 and +0x17 the panel, +0x18 its kind; BN6's +3 and +0x18 its kinds, +0xC the
                                 # step, +0x2C cleared)
WIND_WORD, WIND_ROLE = 0x5C, 'T4:0x41#'   # (WindSoul's wind at his AI data's +0x5C, BN4_WIND_ENTER)
BN6_FORM_PROXY, BN6_FORM_NAME_BASE = 8, 0x1AB
MEGAMAN = ('T1', 0x1A0)
READY = (0x0A, 0x0B, 0x20)     # (an object's phase, its set-up byte, its phase's count)
READY_AHEAD = 2                # (BN6's ready state ahead of BN4's, in BN4's fight frames)
POWERS = (5, 6, 7, 8)          # (BN6_AI_POWER_CHIP, _A, _CHARGE, _C)
# BN4's AI data's bytes the trace lays where BN6 has no field there (src/dev/xdiff_bn4_trace.c's leftovers, in order from
# BN6_AI_SPARE: BN4's +5, +8, +9, +0x2C, +0x2D, ...): their places in BN6's units
SHOTS4, BUTTON4, FRAMES4 = 0xF0, 0xF2, 0xF3
TURNS_USED4, TURNS4 = 0x18, 0x19
LOCK4, LOCK6 = 0x3A, 0x19                 # (his lock-out after a chip: BN4's u16 at +0x3A, BN6's byte at +0x19, megaman_lockout's)
COUNT4 = 0x32                             # (BN4's B counter, u16, at its own place)
COUNT_CHIP4 = 0xF5                        # (... its A counter, BN4's +0x30, u16: BN6's keys hold +0x30, so a leftover, after +0x2C)
TIME_CHIP4 = 0xFC                         # (BN4's chip charge's time, its cache +0x34 at BN6_AI_SPARE_LAST)
BUTTON_A, BUTTON_B = 1, 2
LEVEL_CHIP4, LEVEL4 = 0x0A, 0x0B          # (BN4's levels)
KEPT_CHIP4, KEPT4 = 0x14, 0x1A            # (... kept as B is let go, 0x0800BE48-0x0800BE4E; the second laid at BN6's +0x1A, where BN4
                                          # keeps its B and left's lock, src/dev/xdiff_bn4_trace.c KEPT_LEVEL_AT)
TIME4 = 0xFE                              # (BN4's charged shot's charge time, its cache +0x36 at BN6_AI_SPARE_LAST + 2)
COUNT6, LEVEL6, BUTTON6, REQUEST6 = 0x1B, 0x1D, 0x1E, 0x46
LEVEL_AS6 = {0: 0, 1: 1, 3: 2}
WINDOW4, PRESSES4 = 0x11, 0x12            # (BN4's rapid buster's count, 0x0800BE5A-0x0800BE7A: a press sets the window to 10)
WINDOW6, PRESSES6 = 0x19, 0x18            # (... BN6's in his side's Souls block: BN6_SOUL_WINDOW, BN6_SOUL_PRESSES, soulrapid.c)
REQUESTS, RAPID4, CHARGED6, RAPID_AT = 0x44, 0x20, 0x02, 6   # (... at six asked: BN4's request 0x20, BN6's its charged B's 2)
CHAOS6_NONE = 0xFF                        # (BN6's +0x11, its chip's chaos power, 0xFF none: BN6_AI_POWER_CHIP's note)
GLOW4, GLOW6, GLOW_SLOT = ('T4', 0x05), ('T4', 0x08), 0x58
SOUL_OBJECTS4 = (('T4', 0x44), ('T4', 0x12))   # (SrchSoul's scan, FireSoul's field of grass)
BLINK_TYPE, BLINK_SIDE, BLINKING = 0x08, 0x09, 0x0D   # (a panel's: BN6_PANEL_BLINK_TYPE, _SIDE, BN6_PANEL_BLINKING)
SETTER_TYPE = 0x04                        # (FireSoul's object's +4: the type it sets, BN4's 0x080E3436)
PANEL_TYPE, PANEL_TYPE_COPY, PANEL_FLAGS, PLAIN = 0x02, 0x06, 0x14, 2   # (BN6_PANEL_TYPE, its copy, the flags word; plain)
# A Soul's companion (NumSoul's, FireSoul's: BN4_SOUL_COMPANION's first-kind 0x57) as BN6's companion overlay, its type 0x56 of
# the kind SOUL_OVERLAY_BN4 (src/emu/soulcompanion.c), told as one by tools/xdiff_bn4.py's t1_moved
COMPANION = ('T1', T1_MOVED | 0x57)
COMPANION_TYPES = (0x57, 0x56)            # (its type, +1, in each)
SOUL_OVERLAY_BN4 = 61                     # (soul_room.h)
OVERLAY = 0x50                            # (BN6_T1_OVERLAY: MegaMan's overlay, BN4's companion kept at his AI +0x48)
SPAWN_FLAGS6 = 0x04                       # (+0: both spawns OR in 0x04 (BN6's 0x080C5D18 0x14); BN4's init clears it where the battle has
                                          # appeared, 0x080CC438-0x080CC43E, BN6's overlay keeps it)
PLACE = 0x08                              # (its sprite state's +8, u16: the renderer's tile place, given in the order things are drawn)
MODE4, DRAWN4 = 0x09, 8                   # (+9: BN4's companion's mode, 8 drawn, 0x080CC466; BN6's overlay has none)
MASKS6 = 0x54                             # (+0x54: BN6's overlay's masks by animation, its kind's)
WORD = 0x04                               # (+4, +5: BN4's its sprite's list offset and number; BN6's its kind and 0)
COMPANIONS4 = 0x18068                     # (BN4_FORM_COMPANIONS: a u16 by form)
OWN_ART = (0x18, 0x28)                    # (a sprite state's words into its sprite: xdiff_bn4.py's OWN_ART)
SPRITE_STATE = 0x2C                       # (its bytes: xdiff_bn4_opening.SPRITE_STATE)
COLL_MET = 0x68                           # (BN6_COLL_MET: the regions a collision data met, BN4's +0x4C at BN6's place in the trace)           # (a Soul's object spawned as he takes the form: SrchSoul's, tools/xchiprecipe_bn4.py SOUL_ENTERS)


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


def chip_power(rom4, form):
    """BN6's number of the chip charge of BN4's Soul of `form`, as src/emu/soulpower.c's bn4_chip_power gives it: BN5's empty
    power (soulpower.c's empty_powers, after Team Colonel's moved ones) whose charge times (BN5_CHARGE_TIMES, 0x0801CA68, ten
    bytes an id) are those of its own (BN4_FORM_LOADOUTS' +2, BN4_CHARGE_TIMES 0x080180B8), else BN4's own at the k-th's place
    past Team ProtoMan's (soul_room.h SOUL_POWER_BN4_MORE, two by k: FireSoul's); None where it has none"""
    chip = rom4[0x184F0 + 4 * form + 2]
    if chip == 0xFF:
        return None
    head = open(os.path.join(ROOT, 'src', 'emu', 'bn6_attacks.h')).read()
    count = int(re.search(r'#define BN6_POWER_ATTACK_COUNT\s+(\d+)', head).group(1))
    body = open(os.path.join(ROOT, 'src', 'emu', 'soulpower.c')).read()
    empty = [int(x, 0) for x in re.search(r'empty_powers\[\] = \{ ([^}]*) \}', body).group(1).split(',')]
    rom5 = rom(5)
    row4 = rom4[0x180B8 + 10 * chip:0x180B8 + 10 * chip + 10]
    for i, p in enumerate(empty):
        if rom5[0x1CA68 + 10 * p:0x1CA68 + 10 * p + 10] == row4:
            return count + colonel_powers() + i
    room = open(os.path.join(ROOT, 'src', 'emu', 'soul_room.h')).read()
    more = int(re.search(r'#define SOUL_POWER_BN4_MORE\s+(0x[0-9A-Fa-f]+)', room).group(1), 16)
    k = {f: n for n, f in enumerate(bn4_forms().values())}.get(form)
    return None if k is None else more + 2 * k


def power_map(rom4):
    """BN4's power attack -> BN6's for the moved Souls: each one's charged shot (BN4_FORM_LOADOUTS' +3 by its form, 0x080184F0)
    at bn6_power's number, its chip charge (+2) at chip_power's, and a plain shot of its own (+1: MetlSoul's) or his B and
    left's (ProtSoul's shield) at its own"""
    if not rom4:
        return {}
    out = {rom4[0x184F0 + 4 * f + 3]: bn6_power(f) for f in bn4_forms().values()}
    out.update({rom4[0x184F0 + 4 * f + 2]: chip_power(rom4, f) for f in bn4_forms().values() if chip_power(rom4, f) is not None})
    # (and its own plain shot, +1 where not BN4's 0: the second of its own past Team ProtoMan's, soulpower.c's MORE_OTHER)
    room = open(os.path.join(ROOT, 'src', 'emu', 'soul_room.h')).read()
    more = int(re.search(r'#define SOUL_POWER_BN4_MORE\s+(0x[0-9A-Fa-f]+)', room).group(1), 16)
    for k, f in enumerate(bn4_forms().values()):
        if rom4[0x184F0 + 4 * f + 1]:
            out[rom4[0x184F0 + 4 * f + 1]] = more + 2 * k + 1
        # (... or his B and left's, which his taking of the form sets (BN4_FORM_ENTERS, 0x0800E0A0: ldr r0,[r5,#0x58]; movs
        # r1,#power; strb r1,[r0,#0], ProtSoul's shield), the same place where he has no plain shot of his own)
        enter = int.from_bytes(rom4[0xE0A0 + 4 * f:0xE0A4 + 4 * f], 'little') & ~1
        at = enter - 0x08000000
        h = [int.from_bytes(rom4[at + 2 * i:at + 2 * i + 2], 'little') for i in range(3)] if 0 <= at < len(rom4) - 6 else []
        if not rom4[0x184F0 + 4 * f + 1] and h and h[0] == 0x6DA8 and h[1] & 0xFF00 == 0x2100 and h[2] == 0x7001:
            out[h[1] & 0xFF] = more + 2 * k + 1
    return out


def form_frames(rom4):
    """BN4_FORM_FRAMES by form (0x08018088, a u16 each)"""
    return lambda form: int.from_bytes(rom4[0x18088 + 2 * form:0x18088 + 2 * form + 2], 'little') if rom4 else None


class Souls4:
    """the rules over one spec's two traces"""

    def __init__(self, ta, tb, rom4, lag):
        self.names, self.forms, self.powers, self.frames = rules(), bn4_forms(), power_map(rom4), form_frames(rom4)
        self.rom4, self.trace_b = rom4, tb
        self.blinked = None         # (the panel type BN6's Soul object blinked the panels as, once seen: FireSoul's grass)
        self.companion_first = {}   # (the companion's animation, its sprite's and its +0x11, at its first difference read)
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
        for key, rule in (('unison', self.unison), ('soul object', self.soul_object), ('companion', self.companion), ('held', self.held), ('unison var', self.unison_var), ('ready clock', self.ready), ('power', self.power), ('name', self.name),
                          ('soul', self.soul), ('lockout', self.lockout), ('pending', self.pending), ('form frames', self.form_frames),
                          ('shots', self.shots), ('presses', self.presses), ('charge', self.charge), ('wind', self.wind),
                          ('step', self.step)):
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
        # (its sounds, begun before the fight, said at the frame after its last, beside BN6's of that frame)
        if d.part != 'sounds' or self.unison_last is None or fa.fr['t'] != self.unison_last + 1 or not isinstance(d.a, list):
            return False
        rest = list(d.a)
        for x in d.b or []:
            if x not in rest:
                return False
            rest.remove(x)
        deco = list(UNISON_DECO_SOUNDS)
        for x in list(rest):
            if x in deco:
                deco.remove(x)
                rest.remove(x)
        return set(rest) <= set(UNISON_SOUNDS)

    def soul_object(self, d, fa, fb):
        """a Soul's object spawned as he takes the form, BN4's inside its unison (BN4_FORM_STATE runs the form's routine as the form is
        written, before the fight's first frame), BN6's as he enters the proxy form, which its crossing ends nearer the fight: BN6's
        alone in play in the fight's first frames, BN4's gone before them"""
        mine = lambda x: isinstance(x, tuple) and tuple(x[:2]) in SOUL_OBJECTS4
        objs = lambda fr: [o for o in fr['objs'] if mine(o.role)]
        for o in objs(fb.fr):
            self.blinked = o.o8(SETTER_TYPE)
        # (a panel's blink, kept: BN6's of the object's that blinked it in the fight, its type and side (BN6_PANEL_BLINK_SET's
        # stores, kept after), BN4's cleared as its fight began, its blinks before it)
        raw = lambda v: v[1] if isinstance(v, tuple) and len(v) == 2 and v[0] == 'raw' and isinstance(v[1], int) else None
        if isinstance(d.role, str) and d.role.startswith('panel ') and d.part == 'panel' and d.off == BLINK_TYPE and \
                raw(d.a) is not None and raw(d.b) is not None and self.blinked is not None:
            # (the word read whole: its type and side bytes so, the rest alike)
            a, b = raw(d.a), raw(d.b)
            # (either game's kept where the other's has none: the setter's loop runs to column 0, which BN6's twin of the blink,
            # 0x0800CC52, leaves alone)
            kept, none = (b, a) if a & 0xFFFF == 0 else (a, b)
            return a & ~0xFFFF == b & ~0xFFFF and none & 0xFFFF == 0 and kept & 0xFF == self.blinked and (kept >> 8 & 0xFF) in (0, 1)
        if isinstance(d.role, str) and d.role.startswith('panel ') and d.part == 'panel' and d.a == 0 and self.blinked is not None:
            if d.off == BLINK_TYPE:
                return d.b == self.blinked
            if d.off == BLINK_SIDE:
                return d.b in (0, 1)
            if d.off == BLINKING:
                return d.b == 1 and bool(objs(fb.fr))
        if objs(fa.fr) or not objs(fb.fr):
            return False
        # (a panel it sets, BN4's set before the fight, BN6's as it ends: BN4's of its type, BN6's plain still, the type's own flags)
        if isinstance(d.role, str) and d.role.startswith('panel ') and d.part == 'panel' and self.blinked is not None:
            x, y = (int(v) for v in d.role.split(' ')[1].split(','))
            pa, pb = fa.fr['panels'][y * 8 + x], fb.fr['panels'][y * 8 + x]
            if len(pa) <= PANEL_FLAGS + 4 or len(pb) <= PANEL_FLAGS + 4 or (pa[PANEL_TYPE], pb[PANEL_TYPE]) != (self.blinked, PLAIN):
                return False
            return d.off in (PANEL_TYPE, PANEL_TYPE_COPY) or PANEL_FLAGS <= d.off < PANEL_FLAGS + 4 or d.off in (BLINK_TYPE, BLINK_SIDE)
        if d.part == 'in play':
            return mine(d.role) and d.a is False and d.b is True
        if d.part == 'run order' and isinstance(d.a, list) and isinstance(d.b, list):
            return [x for x in d.b if not (isinstance(x, tuple) and any(x[0].startswith(f'{k}:{t:#x}#') for k, t in SOUL_OBJECTS4))] == d.a
        # (MegaMan's word of the regions he met, BN6_COLL_MET: BN6's with the bits of the hit area the object lays as it ends, which
        # BN4's met before the fight; no bit of BN4's missing)
        if isinstance(d.role, tuple) and d.role[:2] == MEGAMAN and d.part == 'coll' and COLL_MET <= d.off < COLL_MET + 4:
            return isinstance(d.a, int) and isinstance(d.b, int) and d.a & ~d.b == 0
        return False

    def companion(self, d, fa, fb):
        """a Soul's companion, BN4's first-kind 0x57 and BN6's companion overlay of BN4's Souls' kind: its type, its word (BN4's
        sprite's list offset and number, BN6's kind and its palette's 0), its flags (the 0x04 both spawns set, BN6's kept), the renderer's tile place, BN4's mode
        (drawn) where BN6's overlay has none, BN6's masks by animation, its sprite's words by their offsets into the same
        bytes (its last set-up's CRC32 alike); and MegaMan's keeping of it, BN6's at his +0x50 and BN4's at his AI +0x48 (the
        trace's leftovers), each the companion's"""
        if not isinstance(d.role, tuple):
            return False
        num = lambda v: v[1] if isinstance(v, tuple) and len(v) == 2 and v[0] == 'raw' else v if isinstance(v, int) and not isinstance(v, bool) else None
        a, b = num(d.a), num(d.b)
        if d.role[:2] == COMPANION and d.part == 'obj':
            form = fa.fr.get('player', b'')[BN4_NAVI_FORM] if len(fa.fr.get('player', b'')) > BN4_NAVI_FORM else 0
            word = int.from_bytes(self.rom4[COMPANIONS4 + 2 * form:COMPANIONS4 + 2 * form + 2], 'little') if self.rom4 else 0
            if d.off == 0:
                return a is not None and b is not None and a ^ b == SPAWN_FLAGS6 and bool(b & SPAWN_FLAGS6)
            if d.off == 1:
                return (a, b) == COMPANION_TYPES
            if d.off == WORD:
                return word != 0 and (a, b) == (word & 0xFF, SOUL_OVERLAY_BN4)
            if d.off == WORD + 1:
                return word != 0 and (a, b) == (word >> 8, 0)
            if d.off == MODE4:
                return (a, b) == (DRAWN4, 0)
            if MASKS6 <= d.off < MASKS6 + 4:
                return a == 0 and b is not None and b != 0
            sp = SPRITE['T1']
            if sp + PLACE <= d.off < sp + PLACE + 2:
                return True
            if sp <= d.off < sp + SPRITE_STATE and a is not None:
                return self.companion_sprite(d, fa, fb, a, b)
            return False
        if d.role[:2] != MEGAMAN:
            return False
        mate = next((x for x in fa.fr['objs'] if isinstance(x.role, tuple) and x.role[:2] == COMPANION), None)
        if mate is None:
            return False
        if d.part == 'obj' and d.off == OVERLAY:
            return a == 0 and isinstance(d.b, tuple) and len(d.b) == 3 and str(d.b[0]).startswith(f'T1:{COMPANION[1]:#x}#')
        if d.part == 'ai' and b == 0 and a is not None:
            o = self.mm(fa.fr)
            at = o.ai.find(mate.addr.to_bytes(4, 'little')) if o is not None and o.ai is not None else -1
            return at >= 0 and at <= d.off < at + 4
        return False

    def companion_sprite(self, d, fa, fb, a, b):
        """its sprite's state at BN6's of BN4's same fight frame while its animation is the one its init set (his, before the
        fight), which runs by the fight's frames in both (each drawn mode's sprite update, BN4 0x080CC5E8 / BN6 0x080C5D0C,
        unless the time stop holds; FireMan's companion's the same, xdiff_bn4.py navi_opening_art): its words into its sprite
        by their offsets into the same bytes (its last set-ups' CRC32 alike), its other bytes as they are. Once he takes
        another, it follows his, BN6's a frame later as all of his: compared as ever, its words into its sprite by their
        offsets"""
        from xdiff_bn4 import arts_set
        sp, t = SPRITE['T1'], fa.fr['t']
        fs = self.b.get(t)
        oa = next((x for x in fa.fr['objs'] if x.role == d.role), None)
        ob = next((x for x in fs['objs'] if x.role == d.role), None) if fs else None
        if oa is None or ob is None:
            return False
        now = (oa.o8(sp), oa.o8(0x11))
        if self.companion_first.setdefault(d.role, now) != now:
            self.companion_first[d.role] = None   # (the opening's over for good: his next animations follow his at the lag)
        opening = self.companion_first[d.role] == now
        if not opening:
            ob = next((x for x in fb.fr['objs'] if x.role == d.role), None)
            if ob is None:
                return False
        if sp + OWN_ART[0] <= d.off < sp + OWN_ART[1] and (d.off - sp) % 4 == 0:
            tb = t if opening else fb.fr['t']
            art_a = arts_set(fa.trace).get(t, {}).get(d.role)
            art_b = arts_set(self.trace_b).get(tb, {}).get(d.role)
            vb = ob.o32(d.off) if opening else b
            return art_a is not None and art_b is not None and art_a[0] == art_b[0] and vb is not None and \
                a - oa.o32(sp + OWN_ART[0]) == vb - ob.o32(sp + OWN_ART[0])
        return opening and isinstance(d.a, int) and ob.o8(d.off) == a

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

    def wind(self, d, fa, fb):
        """WindSoul's wind's word: BN4's AI data's +0x5C naming the wind, BN6's 0 (it keeps the wind in the side's slot alone)"""
        a = d.a if isinstance(d.a, tuple) else ()
        return isinstance(d.role, tuple) and d.role[:2] == MEGAMAN and d.part == 'ai' and d.off == WIND_WORD and \
            len(a) == 3 and isinstance(a[0], str) and a[0].startswith(WIND_ROLE) and d.b in (('raw', 0), 0)

    def step(self, d, fa, fb):
        """his step out of his charged shot's lag (RollSoul's arrow, xchips_bn4.h XC4_STEP_OUT): each engine's own step state
        and the variables it sets, BN4's by its panel and BN6's by its direction, while both stand in it"""
        if not (isinstance(d.role, tuple) and d.role[:2] == MEGAMAN) or not self.united(fa, fb):
            return False
        oa, ob = self.mm(fa.fr), self.mm(fb.fr)
        if oa is None or ob is None or oa.rec[NODE + OBJ_ACTION] != STEP4 or ob.rec[NODE + OBJ_ACTION] != STEP6:
            return False
        return (d.part == 'obj' and d.off == OBJ_ACTION) or (d.part == 'ai' and d.off in STEP_VARS)

    def lockout(self, d, fa, fb):
        # (his lock-out after a chip, BN4's at +0x3A and BN6's at +0x19 (legit.json's bn4 megaman_lockout), where BN4's +0x19 holds
        # his Soul's turns, which checks() holds to BN6's Souls block: each engine's place of the one count)
        def explain(k, a, b):
            kept = a[LOCK4] | a[LOCK4 + 1] << 8
            return kept == b[LOCK6] and (k == TURNS4 or (k in (LOCK4, LOCK4 + 1) and b[k] == 0))
        return self.united(fa, fb) and self.word_bytes(d, fa, fb, explain)

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
        """a byte of MegaMan's AI data where the engines keep the charge apart, each reading as the other: BN4's two counters,
        levels and times (B's, A's chip charge), BN6's one counter, level and button"""
        u16 = lambda at: int.from_bytes(a[at:at + 2], 'little')
        def held(count, time, level, other_count, other_level):
            return b[COUNT6] == min(u16(count), u16(time)) and LEVEL_AS6.get(a[level]) == b[LEVEL6] and \
                a[BUTTON4] == b[BUTTON6] and u16(other_count) == 0 and a[other_level] == 0
        same = held(COUNT4, TIME4, LEVEL4, COUNT_CHIP4, LEVEL_CHIP4) if a[BUTTON4] != BUTTON_A else \
            held(COUNT_CHIP4, TIME_CHIP4, LEVEL_CHIP4, COUNT4, LEVEL4)
        if k in (COUNT6, LEVEL6, BUTTON6):
            return same and a[k] == 0
        if k in (COUNT4, COUNT4 + 1, LEVEL4, BUTTON4, COUNT_CHIP4, COUNT_CHIP4 + 1, LEVEL_CHIP4):
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
