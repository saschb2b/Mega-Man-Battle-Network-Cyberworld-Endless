"""xnavirecipe_bn4_companion.py: what ThunderMan adds to tools/xnavirecipe_bn4.py's walk of a BN4 Navi (docs/multirom/bn4-navis.md,
ThunderMan built): a companion his init sets out (a first-kind object of its own type, moved through the first kind's dispatch),
a generic state his table names through a wrapper of his own, his objects' sprite of list 5, a sound past a byte named by a
movs and an adds, and the routines his objects call that the families' pairs lack.

Imported by tools/xnavirecipe_bn4.py alone. Numbers only: BN4's and BN6's addresses, read from both ROMs and each said why."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_navis as N  # noqa: E402
import bn4_twins  # noqa: E402
import xrecipe_bn4 as X4  # noqa: E402
import xrecipe_thumb as T  # noqa: E402

# BN4's routines his code calls with a BN6 twin the families' pairs did not find (tools/bn4_twins.py's way: BN4 -> BN6, its
# source), each body read in both ROMs (tools/xrecipe_thumb.py's walk)
TWINS = {
    # a side's count of enemies standing (BattleState +4 + its side) less one, the families' dying's (BN4_DYING): BN6's
    # 0x0800A10C the same eight instructions (ldrb [r5,#0x16]; adds #4; [eToolkit +0x18]; ldrb, subs #1, strb; mov pc,lr)
    0x08007990: (0x0800A10C, 'thunderman'),
    # a panel broken where it stands free, else cracked (his 0x71's, at 080DB6B0): BN6's 0x0800CA8C (audited, AUDITS)
    0x08009BF0: (0x0800CA8C, 'thunderman'),
}
AUDITS = {
    (0x08009BF0, 0x0800CA8C): ('ok', "a panel (r0, r1) of a floor (0x10) broken (type 1, its flags' low bits 1, the crack's sound) "
                                     "where none of its occupants' flags is set, else cracked (type 3, flags + 0x40); BN6's the "
                                     "same at its offsets (flags +0x14, type +2 and its drawn type +6) and its own sound (0x97), "
                                     "but: BN4's test of flag 0x20000 first (no change), which only BN4's type 5 gives and no BN6 "
                                     "panel has (GutsMan's crack's audit); BN4's occupants' mask 0x0F0A0080 where BN6's is "
                                     "0x0F880080 (BN6 counts 0x00800000 an occupant's, BN4's break does not); and BN4 returns 0 "
                                     "after a crack where BN6 returns 1, which his one call (080DB6B0) drops (movs r0,#1 next). "
                                     "The mask's bit is the differential's to show: his panels' flags are compared each frame"),
    # (the symbol map's pairs by their calls alone, bn6f's names: each body read in both ROMs, and judged where they are called)
    (0x0801466A, 0x0801BE28): ('ok', "called by his companion's and his cloud's (the third kind's 0x6A) updates only while the battle "
                                     "is not running (0x08007A8C's bit 0 clear, BN6's 0x0800A0F0 the same body), in its place: "
                                     "BN4's blinks the object (its +0 bit 2 by the clock's bit 1, 0x080019C0) while [BattleState "
                                     "+0x3C]+6 has 0x20, BN6's (bn6f's nullsub_10) does nothing. Interim, assumed inert: that bit's "
                                     "setter unread; the differential compares his objects' +0 in the paused opening"),
    (0x080406A4, 0x0800A03C): ('ok', "called by his cloud's update (0x080DAB54) and the third kind's 0x6F's (0x080DB2F8, "
                                     "0x080DB314) before they move: BN4's tests its event flag 0x1187 (0x0802509C, eToolkit +0x48's "
                                     "flags), BN6's (bn6f's battle_isPaused) BattleSettings +0xA. Interim, assumed: neither set in a "
                                     "run's battle (flag 0x1187's meaning unread); the differential shows both answer 0 in its battle"),
}
# the engine's routines his code calls with no BN6 twin, copied with him (tools/xnavirecipe_bn4.py NAVI_COPIED's way)
COPIED = {
    0x0800AD54: "the can-move check's kin (its flags against 0x1040, then its panel's type): BN4_CAN_MOVE's body (0x0800AD2A, "
                "0x5040) with another mask, no BN6 routine of its body; copied, its panel's type read at BN6's +2 (BN4's +0)",
    0x080D97F8: "an object's x moved by r0 toward its side's front (its +0x36 by +0x16): a leaf in the third kind's 0x69's block, "
                "its fields BN6's the same (its spawn's +0x34 a word, +0x16 its side); copied",
    0x0801052C: "the families' dying (BN4_DYING), his companion's jump table's +8: its collision data let go (BN6's 0x080198CE, "
                "LAYOUT), its object freed and its side's count less one (0x08007990, BN6's 0x0800A10C); no BN6 routine of its "
                "body (BN6's virus object keeps its own); copied",
}
# its pools' words that look like addresses and are numbers (bn4_twins.NUMBERS' way)
NUMBERS = {
    0x04050301: "a collision region's spawn word (object_spawnCollisionRegion's r4, as 0x04050601 and 0x0405FF01): BN6's own "
                "attacks pass the same (its pools at 0x080E4A44 and 0x08101400)",
}


def install(pairs):
    """his pairs, audits, copies and numbers into the shared tables, for the Navis' walk (after the families')"""
    pairs.twins.update({a: b for a, (b, _) in TWINS.items()})
    pairs.twin_src.update({a: 'project:' + s for a, (_, s) in TWINS.items()})
    bn4_twins.AUDITS.update(AUDITS)
    bn4_twins.NUMBERS.update(NUMBERS)


def wraps_generic(b4, x, k):
    """whether state `x` is only a call of BN4's generic Navi state k (push {lr}; bl; pop {pc}): that state, BN6's of its place
    in his table (ThunderMan's 1, 0x080C38CC)"""
    w = T.walk(b4, x & ~1, limit=0x10)
    forms = [w.insns[a].form for a in sorted(w.insns)]
    return forms == ['push', 'bl', 'pop'] and list(w.bls.values()) == [N.NAVI_GENERIC[k] & ~1]


# His init's companion's form (ThunderMan's, 0x080C3868-0x080C3874): after the collision setup, r0 = hi << 16 | a byte of a table
# of his block by his version (+4), r1 his version, then the call of his spawner, a routine that spawns a first-kind type
FORM = ('movs', 'lsls', 'ldr_lit', 'ldrb', 'ldrb_r', 'orrs', 'bl')


def companion_of(b4, f, w):
    """his init's companion (FORM after the collision setup's call): its spawner a root of his walk, its type's region his (a
    first-kind type moved through the dispatch), f.companion4 what the installer's shim needs (xnavi_install.c companion_set);
    his init's call of his update noted (BN6's actor init makes none, 0x08017092)"""
    f.companion4, f.moved1 = None, {}
    addrs = sorted(w.insns)
    setup = [s for s, t in w.bls.items() if t == X4.BN4['setup_coll']]
    for site, t in sorted(w.bls.items()):
        if t == f.update:
            f.notes.append(f'{site:08X}: its init runs its update once (its state 0 in its spawn\'s frame); BN6\'s actor init does not')
            drop(f, site, t)
    if len(setup) != 1:
        return
    k = addrs.index(setup[0]) + 1
    run = [w.insns[a] for a in addrs[k:k + len(FORM)]]
    forms = tuple(i.form for i in run)
    if forms != FORM:
        return
    sp = run[-1].target
    sw = T.walk(b4, sp, limit=0x40)
    spawn = [(a, X4.SPAWNERS4[s]) for a, s in sw.bls.items() if s in X4.SPAWNERS4]
    ty = T.back_value(sw, spawn[0][0], 0)[0] if len(spawn) == 1 and spawn[0][1][0] == 1 else None
    hi, table = run[0].f[1], b4.u32(run[2].lit)
    if ty is None or run[1].f[2] != 16 or run[3].f[1:] != (5, 4) or table is None:
        f.decide('companion', f'{run[-1].at:08X}: its init\'s companion in a form the generator does not read')
        return
    lo, end = X4.region_of(b4, ty)
    f.moved1[ty] = dict(lo=lo, hi=end, routine=lo)
    f.regions.append((lo, end, ty))
    f.roots += [sp, lo]
    f.companion4 = dict(spawner=sp, table=table, hi=hi, ty=ty)
    drop(f, run[-1].at, sp)
    f.notes.append(f'{run[-1].at:08X}: its companion, the first-kind {ty:#x} ({lo:08X}-{end:08X}) its spawner {sp:08X} sets out with r0 '
                   f'{hi:#x} << 16 | its table {table:08X}\'s byte by its version: begun by BN6\'s actor init\'s companion word '
                   '(0x08017064), a shim of its home (XnCompanion4)')


def drop(f, site, t):
    f.decisions = [d for d in f.decisions if d != ('init', f'{site:08X}: its init calls {t:08X}, which BN6\'s Navi init makes nothing of')]


def spawns1_of(f, alloc):
    """his code's spawns of the first-kind types it moves (f.moved1): at a number of the dispatch's of their own, as
    xrecipe_bn4.spawns_of moves the third kind's"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            kind, name = X4.SPAWNERS4.get(t, (None, None))
            ty, chain = T.back_value(w, site, 0)
            if kind != 1 or ty not in f.moved1 or len(chain) != 1 or chain[0][1] != 'movs':
                continue
            f.decisions = [d for d in f.decisions if d != ('type', f'{site:08X}: it spawns a 1 type {ty:#x} which it does not move')]
            m = f.moved1[ty]
            if 'as' not in m:
                m['as'] = alloc.types[1]
                alloc.types[1] += 1
                f.types.append((1, m['as'], ty))
            f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=ty, now=m['as'],
                               why=f'its first-kind object\'s type {ty:#x}: moved, the dispatch\'s {m["as"]:#x}'))
            f.spawn_calls[t] = name


def list5_sprites(b4, b6, f, alloc):
    """his objects' loads of a list 5 sprite (r1 0x14, r2 its number, each a movs): BN6's of the same bytes, else copied into
    list 5 at a number of its own (the families' list 5 range, an effect's sprite the same entry)"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != X4.BN4['sprite_load']:
                continue
            lst, c1 = T.back_value(w, site, 1)
            num, c2 = T.back_value(w, site, 2)
            if lst != 0x14 or num is None or len(c2) != 1 or c2[0][1] != 'movs':
                continue
            msg = f'{site:08X}: a moved object\'s sprite {lst}:{num}, which the generator moves none of (list 4 alone, by a movs)'
            tw = X4.bn6_sprite_twin(b4, b6, 5, num)
            how = 'BN6\'s of the same bytes'
            if tw is None:
                s = alloc.effect_sprites.get((5, num), (None,))[0]
                if s is None:
                    p, raw = X4.sprite(b4, X4.BN4['sprites'], 5, num)
                    if raw is None or alloc.list5 >= X4.LIST5_END:
                        continue
                    s = dict(list=5, index=num, to_list=5, to_index=alloc.list5, at=alloc.art,
                             room=X4.rounded(len(raw), X4.ROOM_EFFECT), lz=bool(p & 0x80000000))
                    f.attack_sprites.append(s)
                    alloc.effect_sprites[(5, num)] = (s, f.name)
                    alloc.list5 += 1
                    alloc.art += s['room']
                tw, how = s['to_index'], 'copied'
            f.decisions = [d for d in f.decisions if d != ('sprite', msg)]
            if tw != num:
                f.imms.append(dict(at=c2[0][0], form='THUMB_MOVS', rd=2, rn=0, was=num, now=tw,
                                   why=f'its object\'s sprite: list 5 {num:#x} in BN4, {tw:#x} in BN6 ({how})'))
            f.notes.append(f'{site:08X}: its object\'s sprite 5:{num:#x}, BN6\'s 5:{tw:#x} ({how})')


def wide_movs(f, pool):
    """his sounds past a byte named by movs r0,#lo and adds r0,#0xFF (BN4's 0x143 so): where the pool's slot fits a movs, the
    movs its slot and the adds 0"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != X4.BN4['sound']:
                continue
            v, chain = T.back_value(w, site, 0)
            if v is None or v not in pool or len(chain) != 2 or chain[0][1] != 'adds8' or chain[1][1] != 'movs' \
                    or chain[0][2] != 0xFF or pool[v] > 0xFF:
                continue
            f.decisions = [d for d in f.decisions if not (d[0] == 'sound' and d[1].startswith(f'its sound {v:#x} at {site:08X} named otherwise'))]
            why = f'its sound {v:#x} (movs and adds #0xff) at the pool\'s {pool[v]:#x}'
            f.imms.append(dict(at=chain[1][0], form='THUMB_MOVS', rd=0, rn=0, was=chain[1][2], now=pool[v], why=why))
            f.imms.append(dict(at=chain[0][0], form='THUMB_ADDS', rd=0, rn=0, was=0xFF, now=0, why=why))


def c_companion(f, n):
    """its companion's shim's numbers as C (XnCompanion4), or ''"""
    c = f.companion4
    if not c:
        return ''
    return (f'static const XnCompanion4 {n}_companion = {{ OWN(0x{c["spawner"] | 1:08X}), 0x{c["table"]:08X}, 0x{c["hi"]:02X} }};   '
            f'/* its init\'s companion: the first-kind {c["ty"]:#x} */\n')


def copied_entries(f):
    """the entries of his moved objects' jump tables out of their regions that are routines copied with him (COPIED: his
    companion's dying, BN4_DYING): walked as his own, so the table's word names the copy, not a twin"""
    out = set()
    for x in list(f.outs):
        if x & ~1 in COPIED:
            out.add(x & ~1)
            del f.outs[x]
    return out


def palette_interim(b4, f):
    """his palette by version where BN4's step is not 0 (BN4_NAVI_PALETTE: its byte of 0x0800B9AC times the version's of
    0x0800B9B1, ThunderMan's 2 x 1, 2, 1, 2): BN6's actor init gives a Navi its table's 0x0800F354 by its version times the
    step, 0 for versions 0-3 whatever the step (0x0800F346), so no step of BN6's gives his; palette 0 as an interim, said so"""
    steps = [d for d in f.decisions if d[0] == 'struct1' and 'its palette step' in d[1]]
    if not steps:
        return
    f.decisions = [d for d in f.decisions if d[0] != 'struct1' or not ('its palette step' in d[1] or 'gives no palette' in d[1])]
    f.s1['palette'] = 0
    pals = [2 * b4.u8(0x0800B9B1 + v) for v in range(4)]
    f.interim.append(f'its palette by version is BN4\'s {pals} (BN4_NAVI_PALETTE\'s step 2), which BN6\'s actor init has no step for '
                     '(0 for every Navi version 0-3, 0x0800F346): palette 0 until a shim sets it at its spawn')
