"""xchiprecipe_bn4_navi.py: what a Navi chip of BN4's asks of tools/xchiprecipe_bn4.py past a support chip's unit
(docs/devtools/chip-recipe-generator.md, The BN4 path, A Navi chip's unit; docs/multirom/bn4-chips.md, The build plan's
unit 6).

BN4 runs a Navi chip from its support state's one table (BN4_CHIP_ROUTINES): its routine spawns a fourth-kind object of
the Navi's own (GutsMan's 0x10, FireMan's 0x0F, AquaMan's 0x16; 0x42 for six of them, by the record's third parameter),
which steps MegaMan out (BN4's fourth-kind 0x0C), waits, calls the spawner of the Navi as his chip shows him (a first-kind
type of his own, not the type he fights as: GutsMan's 0x14), waits for that Navi's word that he is done, and steps MegaMan
in. BN6 has that object once for every Navi chip (its fourth-kind 0x10, BN6_NAVI_OBJECT: the same four states, a routine of
BN6_NAVI_ROUTINES by the chip's subfamily where BN4's object names its Navi's spawner) and steps MegaMan out by its own
object, which knows his Crosses and Beast Out. So a Navi chip's unit is the chip's Navi alone: his first-kind type's
region moved, its entry his spawner at BN6's grown Navi routines' NUMBER0 + BN4's subfamily (src/emu/xchips_code.h
XC_NAVI), run by BN6's Navi object under BN6's Navi family, as BN5's Navi chips are (tools/xchiprecipe.py's 'navi' units).
What his attack spawns is the Navi's own objects, which the Navi's recipe moved already (src/emu/xnavi_bn4.c): the unit's
calls into those blocks go to the Navi's copy, one copy run by both, and his battle sprite is the Navi's list 2 number.

Imported by tools/xchiprecipe_bn4.py alone. Numbers only: BN4's and BN6's addresses, read from both ROMs and each said why."""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import xnavirecipe_bn4_objects as C  # noqa: E402
import xrecipe_bn4 as X4  # noqa: E402
import xrecipe_thumb as T  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NAVIS_C = os.path.join(ROOT, 'src', 'emu', 'xnavi_bn4.c')
SPAWN4_1 = 0x080032B8        # BN4_SPAWN_1 (r0 the type)
NAVI_OBJECT6 = 0x080E2B28    # BN6_NAVI_OBJECT: BN6's fourth-kind 0x10, the Navi chips' one object, to:
NAVI_OBJECT6_END = 0x080E2C6C   # ... its spawner (BN6_NAVI_OBJECT_SPAWN)
# The dispatch's first-kind numbers the chips' Navis take: past BN5's own types' (0x5C the highest a BN5 track names), below
# BN4's families' and Navis' (0xE0 on)
TYPES1 = (0xA0, 0xC0)
SPRITE_FIRST = 0x40          # list 2's numbers of BN4's Navis, a seat each (src/emu/xnavi_sprites.h XNAVI_SPRITE_BN4)
# The spawner's shape, GutsMan's (0x080B9B1C): the Navi spawned, his panel (r0, r1), element (r2), parent and side (r5), power
# (r6) and the word he clears when he is done (r7, a byte set to 1): what BN6's Navi object hands a routine of
# BN6_NAVI_ROUTINES (0x080E2BC8-0x080E2BE8: r0, r1 its panel, r2 its element, r4 its parameters, r6 its power, r7 its own
# +0x18, r5 MegaMan), register for register
SHAPE_XY = ('push', 'push', 'movs', 'bl', 'adds3', 'pop', 'bcc', 'strb', 'strb', 'strb', 'str', 'ldrb', 'strb', 'str', 'str',
            'movs', 'strb', 'pop')
# ... and RollAro's (0x080C83D4): no element and no parent, its panel twice, and its side from r3 (strb r3,[r0,#0x16]), which
# BN4's object hands it (ldrb r3,[r5,#0x16] before the call, 0x080E9048) and BN6's object does not: its r3 is the routine's
# own address (0x080E2BCE). The unit's entry is then a wrapper of the installer's that loads r3 from the user BN6 hands in
# r5 (XChipUnit.side: src/emu/xchips_code.c side_stage)
SHAPE_SIDE = ('push', 'push', 'movs', 'bl', 'adds3', 'pop', 'bcc', 'strb', 'strb', 'strb', 'strb', 'strb', 'str', 'str',
              'movs', 'strb', 'pop')
SIDE_STORE = 'strb r3, [r0, #0x16]'
# BN4's Navi chip objects read against BN6's Navi object (NAVI_OBJECT6), each by hand in both ROMs: BN4's fourth-kind type ->
# why BN6's object serves in its place. One not here is a decision
OBJECTS = {
    0x10: "GutsMan's (0x080E31E8): its four states are BN6's Navi object's instruction for instruction (MegaMan stepped out and "
          "0x1E frames, the Navi's spawner called and his word waited for, 0x1E frames, MegaMan stepped in and 0x1E frames) but "
          "for three things. It steps MegaMan out by BN4's fourth-kind 0x0C (0x080E2D26, by his side), BN6 by its first-kind "
          "0x2D (0x080E266E, by his object): each engine's own, MegaMan's forms each game's. It hands the Navi bit 8 of r4 set "
          "where the user's AI data's +0x15 is 3 or more (0x080E3284-0x080E3296), which BN6's object does not (r4 the "
          "parameters alone): that byte is 0 for MegaMan (in every frame of bn4_chip_gutsman's BN4 trace; a Navi's AI data holds "
          "his version there), and the chip's GutsMan never reads his +5 (his region read whole). And its word for the Navi is its +5, BN6's its +0x18: "
          "the Navi keeps the address he is given",
    0x64: "GutPnch's (0x080E90CC): GutsMan's object (0x10) state for state, its timers and its step out and in the same "
          "calls (0x080E2D26), but for two things. Its state that calls the Navi's spawner hands him r4 the parameters whole "
          "([r5,#4], as BN6's object does) and no bit 8 by the user's AI data, with r3 his side, which the spawner "
          "(0x080C85C2) overwrites before it reads it. And its last state plays sound 0xB0 as it steps MegaMan in "
          "(0x080E91BC: movs r0,#0xb0), which GutsMan's does not and BN6's object does not: STEP_IN",
    0x63: "RollAro's (0x080E8FA0): GutPnch's object (0x64) state for state without its sound as MegaMan steps in, so "
          "GutsMan's but that it hands its Navi the parameters whole, and his side in r3 (ldrb r3,[r5,#0x16] at 0x080E9048), "
          "which his spawner stores: SHAPE_SIDE",
}
# A sound BN4's object plays as it steps MegaMan in, which BN6's Navi object does not: BN4's fourth-kind type -> (the movs that
# names it, the sound), checked against the instruction and its call. The unit carries the sound's slot (XChipUnit.step_in):
# the installer aims BN6's object's last state at a stub of the unit's home that plays it for this unit's number alone
# (src/emu/xchips_code.c step_in_stage)
STEP_IN = {0x64: (0x080E91BC, 0xB0)}
PLAY4 = 0x08000560   # BN4's PlaySoundEffect (r0 the song)
# The frames a BN4 Navi chip object waits (each `movs r0, #N; strh r0, [r5, #0x20]` of its states, in address order), BN6's
# Navi object's the control, read each run
TIMERS6 = (0x1E, 0x1E, 0x1E)


def navi_blocks():
    """the blocks BN4's Navis moved (src/emu/xnavi_bn4.c, in install order): (BN4's start, end, the copy's place as the C
    file names it, the Navi)"""
    with open(NAVIS_C, encoding='utf-8') as fh:
        text = fh.read()
    out = []
    for name, body in re.findall(r'static const XvBlock (\w+)_blocks\[\] = \{(.*?)\};', text, re.S):
        for lo, hi, base, off in re.findall(r'\{ 0x([0-9A-F]{8}), 0x([0-9A-F]{8}), (XN_BN4_CODE) \+ 0x([0-9A-F]+),', body):
            out.append((int(lo, 16), int(hi, 16), base, int(off, 16), name))
    return out


def navi_sprites():
    """each moved BN4 Navi's battle sprite: BN4's list 2 number -> (BN6's list 2 number, the Navi), the one room's rows of
    src/emu/xnavi_bn4.c"""
    with open(NAVIS_C, encoding='utf-8') as fh:
        text = fh.read()
    out = {}
    for name, body in re.findall(r'static const XvSprite (\w+)_sprites\[\] = \{(.*?)\};', text, re.S):
        for num, to in re.findall(r'\{ 2, 0x([0-9A-F]{2}), 2, 0x([0-9A-F]{2}), XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE \}', body):
            out[int(num, 16)] = (int(to, 16), name)
    return out


def t1_of(b4, t):
    """the first-kind type whose code holds `t`: (type, lo, hi), else None"""
    types = [b4.u32(X4.BN4['t1'] + 4 * i) & ~1 for i in range(X4.BN4['nt1'])]
    starts = sorted(set(types))
    lo = max((a for a in starts if a <= t), default=None)
    if lo is None or lo == starts[-1]:
        return None
    tys = [k for k, a in enumerate(types) if a == lo]
    return (tys[0] if len(tys) == 1 else None), lo, starts[starts.index(lo) + 1]


def region_walks(rom, lo, hi):
    """every routine of a type's region, from its routine and its jump tables: {entry: walk}"""
    todo, seen = [lo], {}
    while todo:
        e = todo.pop() & ~1
        if e in seen or not lo <= e < hi:
            continue
        w = T.walk(rom, e, region=(lo, hi))
        seen[e] = w
        todo += [t for t in w.bls.values() if lo <= t < hi]
        for v in w.lits.values():
            if v and lo <= v < hi and not v & 3:
                todo += [x for x in X4.table_at(rom, v, lo, hi) if lo <= (x & ~1) < hi]
    return seen


def timers(walks):
    """the frames an object's states wait: each movs r0, #N stored to its +0x20 by the next instruction, in address order"""
    out = []
    for e, w in sorted(walks.items()):
        addrs = sorted(w.insns)
        for a, b in zip(addrs, addrs[1:]):
            i, j = w.insns[a], w.insns[b]
            if i.form == 'movs' and i.f[0] == 0 and j.form == 'strh' and tuple(j.f) == (0, 5, 0x20):
                out.append((a, i.f[1]))
    return tuple(n for _, n in sorted(set(out)))


def spawner_of(b4, f, ty4, lo4, hi4):
    """the spawner of the chip's Navi its object calls: a routine in a first-kind type's region that spawns that type (BN4's
    first-kind spawner, r0 the type), named by a call of the object's or, where the object serves several Navis, by its
    table at the record's third parameter"""
    walks = region_walks(b4, lo4, hi4)
    def spawns(t):
        hit = t1_of(b4, t)
        if hit is None or hit[0] is None:
            return None
        w = T.walk(b4, t, region=hit[1:])
        return hit if any(s == SPAWN4_1 and T.back_value(w, a, 0)[0] == hit[0] for a, s in w.bls.items()) else None
    called = sorted({t for w in walks.values() for t in w.bls.values() if spawns(t)})
    if len(called) == 1:
        return called[0], spawns(called[0]), walks
    tabled = [[x & ~1 for x in X4.table_at(b4, v, lo4, hi4)] for w in walks.values() for v in w.lits.values() if v and lo4 <= v < hi4 and not v & 3]
    tabled = [t for t in tabled if t and all(spawns(x) for x in t)]
    idx = {r[0x12] for _, r in f.chips}
    if len(tabled) == 1 and len(idx) == 1 and not next(iter(idx)) & 3 and next(iter(idx)) // 4 < len(tabled[0]):
        t = tabled[0][next(iter(idx)) // 4]
        f.notes.append(f'its object serves {len(tabled[0])} Navis by a table of their spawners: its chips\' third parameter {next(iter(idx)):#x}, {t:08X}')
        return t, spawns(t), walks
    f.decide('entry', f'its object (the fourth kind\'s {ty4:#x}, {lo4:08X}-{hi4:08X}) names no one spawner of a first-kind type')
    return None, None, walks


def begin(b4, b6, f, ty4):
    """a Navi chip's unit: its object read against BN6's Navi object, its Navi's first-kind type's region the unit's block,
    its spawner the unit's entry"""
    types = [b4.u32(C.T4_TYPES + 4 * i) & ~1 for i in range(C.NT4)]
    starts = sorted(set(types))
    lo4 = types[ty4]
    hi4 = starts[starts.index(lo4) + 1]
    spawner, hit, walks = spawner_of(b4, f, ty4, lo4, hi4)
    if spawner is None:
        return False
    if ty4 not in OBJECTS:
        f.decide('object', f'its object (the fourth kind\'s {ty4:#x}, {lo4:08X}-{hi4:08X}) is not read against BN6\'s Navi object yet (OBJECTS)')
    else:
        f.notes.append(f'its object, the fourth kind\'s {ty4:#x} ({lo4:08X}-{hi4:08X}), not moved: BN6\'s Navi object in its place')
    f.step_in = 0
    if ty4 in STEP_IN:
        at, song = STEP_IN[ty4]
        i, nxt = T.decode(b4, at), T.decode(b4, at + 2)
        if not lo4 <= at < hi4 or i.text != f'movs r0, #{song:#x}' or nxt.form != 'bl' or nxt.target != PLAY4:
            f.decide('object', f'its object\'s sound as it steps MegaMan in is not movs r0,#{song:#x} and its call at {at:08X} (STEP_IN)')
        else:
            f.step_in = song
            f.notes.append(f'its object plays {song:#x} as it steps MegaMan in ({at:08X}), BN6\'s Navi object none: the unit\'s step_in')
    t6 = timers(region_walks(b6, NAVI_OBJECT6, NAVI_OBJECT6_END))
    if t6 != TIMERS6:
        sys.exit(f'xchiprecipe_bn4: BN6\'s Navi object at {NAVI_OBJECT6:08X} waits {t6}, not what this tool reads')
    t4 = timers(walks)
    if t4 != TIMERS6:
        f.decide('object', f'its object waits {tuple(hex(x) for x in t4)} frames, BN6\'s Navi object {tuple(hex(x) for x in TIMERS6)}')
    sw = T.walk(b4, spawner)
    forms = tuple(sw.insns[a].form for a in sorted(sw.insns))
    f.side = forms == SHAPE_SIDE and SIDE_STORE in [sw.insns[a].text for a in sw.insns]
    if f.side:
        f.notes.append(f'his spawner ({spawner:08X}) takes his side in r3, which BN6\'s Navi object does not hand: the unit\'s entry a wrapper that loads it')
    elif forms != SHAPE_XY:
        f.decide('entry', f'his spawner ({spawner:08X}) is not the shape BN6\'s Navi object calls (GutsMan\'s, 0x080B9B1C): a shim to write')
    ty1, lo1, hi1 = hit
    f.entry, f.ty, f.lo, f.hi = spawner, ty1, lo1, hi1
    f.regions, f.roots = [(lo1, hi1, ('1', ty1))], [spawner, lo1]
    f.moved1 = {ty1: dict(lo=lo1, hi=hi1, routine=lo1)}
    f.navi_blocks, f.navi_calls = navi_blocks(), {}
    f.notes.append(f'the chip\'s Navi, the first kind\'s {ty1:#x} ({lo1:08X}-{hi1:08X}), which his spawner ({spawner:08X}) spawns: moved')
    return True


def navi_block(f, t):
    """the Navi's moved block that holds `t` (the first to install it): the copy's place as the C file names it, else None"""
    for lo, hi, base, off, name in getattr(f, 'navi_blocks', ()):
        if lo <= t < hi:
            return f'{base} + 0x{off + t - lo:X}', name
    return None


def aim_calls(f):
    """its calls into code a BN4 Navi's recipe moved: to that copy (the Navi's own objects, one copy run by both)"""
    for t in sorted(f.calls):
        hit = navi_block(f, t)
        if hit:
            f.aim[t] = (hit[0], f'{hit[1]}\'s copy of BN4\'s {t:08X} (src/emu/xnavi_bn4.c): the Navi\'s own object\'s code, one copy run by both')


def spawns1(f, alloc):
    """the spawn of the first-kind type it moves: the dispatch's number of the chips' Navis (TYPES1), the movs that names it
    changed, its call to the dispatch's spawner; xrecipe_bn4.spawns_of's decision answered"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != SPAWN4_1:
                continue
            ty, chain = T.back_value(w, site, 0)
            if ty not in getattr(f, 'moved1', {}) or len(chain) != 1 or chain[0][1] != 'movs':
                continue
            m = f.moved1[ty]
            if 'as' not in m:
                m['as'] = alloc.types1
                alloc.types1 += 1
                if m['as'] >= TYPES1[1]:
                    f.decide('type', 'the chips\' first-kind dispatch numbers (TYPES1) are spent')
                f.types.append((1, m['as'], ty))
            f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=ty, now=m['as'],
                               why=f'the chip\'s Navi\'s type {ty:#x} (the first kind): moved, the dispatch\'s {m["as"]:#x}'))
            f.spawn_calls[t] = X4.SPAWNERS4[t][1]
            f.decisions = [d for d in f.decisions if d != ('type', f'{site:08X}: it spawns a 1 type {ty:#x} which it does not move')]


def battle_sprite(f, site, chain, num):
    """his battle sprite (list 2): the moved Navi's own number there, which the Navis' rooms fill (src/emu/xnavi_bn4_room.h);
    True where answered"""
    hit = navi_sprites().get(num)
    if hit is None:
        f.decide('sprite', f'{site:08X}: his battle sprite 2:{num:#x} is no moved BN4 Navi\'s (src/emu/xnavi_bn4.c)')
        return True
    to, name = hit
    f.seat = to - SPRITE_FIRST
    f.notes.append(f'his battle sprite 2:{num:#x} ({site:08X}): {name}\'s, list 2\'s {to:#x} (seat {f.seat})')
    f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=2, rn=0, was=num, now=to,
                       why=f'his battle sprite: BN4\'s list 2 {num:#x}, the moved Navi\'s {to:#x} in BN6 ({name}\'s)'))
    return True
