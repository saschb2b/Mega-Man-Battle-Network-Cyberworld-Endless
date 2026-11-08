"""xnavirecipe_bn4_objects.py: what BN4's Navis' moved objects ask of tools/xnavirecipe_bn4.py's walk past the families'
machinery (docs/multirom/bn4-navis.md, ThunderMan built): the routines they call that the families' pairs lack (their twins,
audits and copies), their sprites of list 5, a sound past a byte named by a movs and an adds, the routines their jump tables
name that are copied with them. A Navi's init and the companions it spawns are tools/xnavirecipe_bn4_init.py's.

Imported by tools/xnavirecipe_bn4.py alone. Numbers only: BN4's and BN6's addresses, read from both ROMs and each said why."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_navis as N  # noqa: E402
import bn4_twins  # noqa: E402
import xrecipe_bn4 as X4  # noqa: E402
import xrecipe_thumb as T  # noqa: E402

# the fourth kind's types (bn4.h BN4_T4_TYPES, 130) and its spawner (BN4_SPAWN_4)
T4_TYPES, NT4, SPAWN4 = 0x08003F78, 130, 0x08003344

# BN4's routines his code calls with a BN6 twin the families' pairs did not find (tools/bn4_twins.py's way: BN4 -> BN6, its
# source), each body read in both ROMs (tools/xrecipe_thumb.py's walk)
TWINS = {
    # a side's count of enemies standing (BattleState +4 + its side) less one, the families' dying's (BN4_DYING): BN6's
    # 0x0800A10C the same eight instructions (ldrb [r5,#0x16]; adds #4; [eToolkit +0x18]; ldrb, subs #1, strb; mov pc,lr)
    0x08007990: (0x0800A10C, 'thunderman'),
    # a panel broken where it stands free, else cracked (his 0x71's, at 080DB6B0): BN6's 0x0800CA8C (audited, AUDITS)
    0x08009BF0: (0x0800CA8C, 'thunderman'),
    # SearchMan's hiding (docs/multirom/bn4-navis.md, SearchMan built): an object made invisible for r0 frames (its collision
    # data's +0x1A, flag 4, its +0 bit 2 cleared), and shown again (+0x1A 0, flag 4 cleared): BN6's 0x080101AE and 0x080101C4
    # the same bodies at BN6's +0x26 (its invisibility's timer, BN6's 0x08010162 against BN4's 0x0800C0DA): LAYOUT
    0x0800C126: (0x080101AE, 'searchman'),
    0x0800C13C: (0x080101C4, 'searchman'),
}
# the collision data's offsets his code names that the families' (xrecipe_bn4.COLL4TO6) lack: BN4's -> BN6's, each from both
# games' code
COLL4TO6 = {
    # the invisibility's timer: BN4's 0x0800C0DA (from the Navi and player runners, 0x08013EF2 and 0x08013BE4) and BN6's
    # 0x08010162 (from the status step, 0x0801B12E) the same body, 0xFFFF for ever, else less 1 a frame and flag 4 cleared
    # under 0 (sound 0xB0 and 0x94 as it ends); a hit clears it (BN4's 0x0800C110, BN6's 0x08010198)
    0x1A: 0x26,
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
    0x0800A2E4: "SearchMan's: the panels of the rows 3 to 1 and the columns from his side's edge whose flags (0x08009A86, BN6's "
                "0x0800C8F8) have r2 and none of r3, into r7's bytes (column | row << 4), r1's flags met first counted; no BN6 "
                "routine of its body (a search of BN6 by its bytes); copied, its fields none but the object's side",
    0x080DF332: "SearchMan's shot's test: its collision data's hits word (BN4's +0x54, BN6's +0x70) against the other side's "
                "bits (0x0A800000 a side 1 object's, 0x05800000 a side 0 one's, BN6's alliance bits the same: BN4_COLL_MASKS); "
                "a leaf in the third kind's 0x5F's block, no BN6 routine of its body; copied, its +0x54 BN6's +0x70",
}
# effects his code spawns by an id ORed with a flip (r4 = movs id | bit 9 drawn: BN4_SPAWN_EFFECT's r4 high byte its flags):
# the effect's site -> (the movs of its id, the orrs, why); the id given BN6's as xrecipe_bn4.effects_of gives a movs's
FLIPPED_EFFECTS = {
    0x080D8840: (0x080D8830, 0x080D8832, "his shot's hit (the third kind's 0x57): r4 8 ORed with the RNG's bit 1 shifted to bit 9, "
                                         "BN4_SPAWN_EFFECT's flip, the id its low byte"),
}
# the battle's flags (BN4's 0x08007A8C, BN6's 0x0800A2F0 the same body) where a moved object no state runs gates its update on
# their bit 0, the fight's: BN4's state runner runs the Navi's states through the paused opening, so his first own state and
# that object's first gated frame are the fight's first in BN4; BN6's runs none there (0x0801B1C4's pause test), his first
# own state a frame into the fight. Aimed at the dispatch's XV_FIGHT_FLAGS (bit 0 from the fight's second frame,
# src/emu/xvirus_dispatch.c), the object starts with his first own state as in BN4. By the Navi's BN4 type, its sites read
FIGHT_FLAGS4 = 0x08007A8C
FIGHT_GATED = {
    0x32: "ThunderMan's companion's update (0x080C3B42) and his cloud's (0x080DAB26): each gated on bit 0, else the paused "
          "blink alone; thunderman_v1's cloud a step ahead at the battle's offset before (+7 2 against 1 at t1)",
}

# its pools' words that look like addresses and are numbers (bn4_twins.NUMBERS' way)
NUMBERS = {
    0x04050301: "a collision region's spawn word (object_spawnCollisionRegion's r4, as 0x04050601 and 0x0405FF01): BN6's own "
                "attacks pass the same (its pools at 0x080E4A44 and 0x08101400)",
    0x05800000: "SearchMan's shot's test (0x080DF332): the hits word's bits of a side 0 object's foes (0x0A800000 a side 1 "
                "one's), as BN4_COLL_MASKS' and BN6_COLL_MASKS' alliance bits",
    0x0405FF09: "a collision region's spawn word (SearchMan's shot's, 0x080D7D56, r4 of 0x080CD7E2: BN6's 0x080C6BDA), as "
                "0x0405FF01",
}


def install(pairs):
    """his pairs, audits, copies and numbers into the shared tables, for the Navis' walk (after the families')"""
    pairs.twins.update({a: b for a, (b, _) in TWINS.items()})
    pairs.twin_src.update({a: 'project:' + s for a, (_, s) in TWINS.items()})
    bn4_twins.AUDITS.update(AUDITS)
    bn4_twins.NUMBERS.update(NUMBERS)
    for k, v in COLL4TO6.items():
        X4.COLL4TO6.setdefault(k, v)


def t4_of(b4, t):
    """the fourth-kind type whose code holds `t` (its routine to the next type's): (type, lo, hi), else None"""
    types = [b4.u32(T4_TYPES + 4 * i) & ~1 for i in range(NT4)]
    starts = sorted(set(a for a in types if T.is_code_ptr(a | 1, b4)))
    lo = max((a for a in starts if a <= t), default=None)
    if lo is None or lo == starts[-1] or t - lo > 0x1000:
        return None
    tys = [k for k, a in enumerate(types) if a == lo]
    return (tys[0] if len(tys) == 1 else None), lo, starts[starts.index(lo) + 1]


def moved4(b4, f, pairs):
    """his calls into a fourth-kind type's spawner BN6 has no twin of (SearchMan's scope, 0x2E): the type moved with him, its
    region one of his (walked from its routine), the call his own; the new roots"""
    roots = set()
    for t in sorted(f.calls):
        if t in X4.SPAWNERS4 or pairs.twin(t)[0] is not None:
            continue   # (a spawner with a twin of BN6's: GutsMan's debris, BN4_SPAWN_DEBRIS)
        hit = t4_of(b4, t)
        if hit is None or hit[0] is None:
            continue
        ty, lo, hi = hit
        w = T.walk(b4, t, region=(lo, hi))
        if not any(s == SPAWN4 and T.back_value(w, a, 0)[0] == ty for a, s in w.bls.items()):
            continue
        moved = getattr(f, 'moved4', {})
        f.moved4 = moved
        if ty not in moved:
            moved[ty] = dict(lo=lo, hi=hi, routine=lo)
            f.regions.append((lo, hi, ('4', ty)))
            f.notes.append(f'its fourth-kind {ty:#x} ({lo:08X}-{hi:08X}): no BN6 type of its body, moved with him')
        del f.calls[t]
        roots |= {t, lo}
    return roots


def spawns4(f, alloc):
    """each spawn of a fourth-kind type he moves: the dispatch's number of its own (alloc.types[4], distinct across the Navis),
    the movs that names it changed, its call to the dispatch's spawner; xrecipe_bn4.spawns_of's decision answered"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != SPAWN4:
                continue
            ty, chain = T.back_value(w, site, 0)
            if ty not in getattr(f, 'moved4', {}) or len(chain) != 1 or chain[0][1] != 'movs':
                continue
            m = f.moved4[ty]
            if 'as' not in m:
                m['as'] = alloc.types[4]
                alloc.types[4] += 1
                f.types.append((4, m['as'], ty))
            f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=ty, now=m['as'],
                               why=f'its object\'s type {ty:#x} (the fourth kind): moved, the dispatch\'s {m["as"]:#x}'))
            f.spawn_calls[t] = X4.SPAWNERS4[t][1]
            f.decisions = [d for d in f.decisions if d != ('type', f'{site:08X}: it spawns a 4 type {ty:#x} which it does not move')]


def flipped_effects(b4, b6, f, alloc):
    """his effects of FLIPPED_EFFECTS: the id's movs read, BN6's effect of its look (xrecipe_bn4.effect_twin) given it; the
    decision effects_of left open answered"""
    for site, (at, orr, why) in FLIPPED_EFFECTS.items():
        if not any(site in w.bls for w in f.walks.values()):
            continue
        i, o = T.decode(b4, at), T.decode(b4, orr)
        if i.form != 'movs' or i.f[0] != 4 or o.form != 'orrs' or o.f[0] != 4:
            continue
        v = i.f[1]
        e6, how = X4.effect_twin(b4, b6, v), 'the same look'
        if e6 is None:
            e6, how = X4.move_effect(b4, f, alloc, v), 'moved'
        if e6 is None:
            continue
        f.decisions = [d for d in f.decisions if not (d[0] == 'effect' and d[1].startswith(f'an effect spawned at {site:08X} '))]
        if e6 != v:
            f.imms.append(dict(at=at, form='THUMB_MOVS', rd=4, rn=0, was=v, now=e6, why=f'its effect: {v:#x} in BN4, {e6:#x} in BN6 '
                               f'({how}; {why})'))
        f.notes.append(f'its effect {v:#x} at {site:08X} (an id ORed with a flip): BN6\'s {e6:#x} ({how})')


# the tables of a moved fourth-kind type's kinds its init reads its sprite from (its +4, r4's low byte at its spawn): table ->
# its type, its entries' size, the sprite's list offset's and number's places, the kinds his code spawns (each site read), why;
# the table, before the type's routine, copied with its region (its lo moved down to it)
T4_SPRITE_TABLES = {
    0x080E589C: dict(ty=0x31, entry=4, sprite=(0, 1), kinds=(0, 1, 2),
                     why="the fourth kind's 0x31's kinds (4 bytes each: its sprite's list offset and number, its animation, its "
                         "palette), read by its init (0x080E58E2, by +4); SearchMan's states spawn kind 0 (0x080F13C4), the "
                         "third kind's 0x60 kinds 1 and 2 (0x080D972A, 0x080D97E4: movs, then its side ORed in above the byte)"),
}


def t4_regions(f):
    """a moved fourth-kind type whose kinds' table lies before its routine (T4_SPRITE_TABLES): its region from the table on"""
    for v, t in T4_SPRITE_TABLES.items():
        m = getattr(f, 'moved4', {}).get(t['ty'])
        if m is None or m['lo'] <= v:
            continue
        f.regions = [(v, r[1], r[2]) if r[2] == ('4', t['ty']) else r for r in f.regions]
        m['lo'] = v


def t4_sprite_tables(b4, f, alloc):
    """the kinds' sprites of T4_SPRITE_TABLES his moved fourth-kind types load, copied (list 4), each kind's byte the copy's; the
    sprite decision art_of left at the type's load answered"""
    for v, t in T4_SPRITE_TABLES.items():
        m = getattr(f, 'moved4', {}).get(t['ty'])
        if m is None:
            continue
        for k in t['kinds']:
            at = v + t['entry'] * k
            lst, num = b4.u8(at + t['sprite'][0]), b4.u8(at + t['sprite'][1])
            s = X4.list4_sprite(b4, f, alloc, num) if lst == 0x10 else None
            if s is None:
                f.decide('sprite', f'{at:08X}: its table\'s kind {k} draws {lst // 4}:{num:#x}, which the generator moves none of')
                continue
            f.bytes.append((at + t['sprite'][1], num, s['to_index'], f'its fourth-kind {t["ty"]:#x}\'s kind {k}: list 4 {num:#x} in '
                                                                    f'BN4, {s["to_index"]:#x} copied'))
        loads = {site for e, w in f.walks.items() if m['lo'] <= e < m['hi'] for site, x in w.bls.items() if x == X4.BN4['sprite_load']}
        f.decisions = [d for d in f.decisions if not (d[0] == 'sprite' and any(d[1].startswith(f'{a:08X}:') for a in loads))]
        f.notes.append(f'its fourth-kind {t["ty"]:#x}\'s kinds by its table {v:08X}: {", ".join(map(str, t["kinds"]))}')


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
    movs its slot and the adds 0; where it is past a byte (SearchMan's 0x105, 0x109, 0x128, 0x12F), the call aimed at the shim
    that makes r0 the slot (f.wide_calls, which wide_sounds makes XvSoundCalls of), its movs and adds left as BN4's"""
    f.wide_calls = []
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != X4.BN4['sound']:
                continue
            v, chain = T.back_value(w, site, 0)
            if v is None or v not in pool or len(chain) != 2 or chain[0][1] != 'adds8' or chain[1][1] != 'movs' \
                    or chain[0][2] != 0xFF:
                continue
            f.decisions = [d for d in f.decisions if not (d[0] == 'sound' and d[1].startswith(f'its sound {v:#x} at {site:08X} named otherwise'))]
            if pool[v] > 0xFF:
                f.wide_calls.append(dict(at=site, slot=pool[v], why=f'its sound {v:#x} (movs and adds #0xff) at the pool\'s '
                                                                    f'{pool[v]:#x}, past a byte'))
                continue
            why = f'its sound {v:#x} (movs and adds #0xff) at the pool\'s {pool[v]:#x}'
            f.imms.append(dict(at=chain[1][0], form='THUMB_MOVS', rd=0, rn=0, was=chain[1][2], now=pool[v], why=why))
            f.imms.append(dict(at=chain[0][0], form='THUMB_ADDS', rd=0, rn=0, was=0xFF, now=0, why=why))


def copied_entries(f):
    """the entries of his moved objects' jump tables out of their regions that are routines copied with him (COPIED: his
    companion's dying, BN4_DYING): walked as his own, so the table's word names the copy, not a twin"""
    out = set()
    for x in list(f.outs):
        if x & ~1 in COPIED:
            out.add(x & ~1)
            del f.outs[x]
    return out


def fight_gates(f):
    """his objects' gates on the battle's flags aimed at XV_FIGHT_FLAGS (FIGHT_GATED, by his BN4 type): a fight frame later in
    BN6, with his first own state"""
    if f.ty in FIGHT_GATED and FIGHT_FLAGS4 in f.calls:
        f.aim[FIGHT_FLAGS4] = ('XV_FIGHT_FLAGS', 'the battle\'s flags, bit 0 from the fight\'s second frame: his objects no state '
                                                 'runs begin with his first own state, a frame into BN6\'s fight (FIGHT_GATED)')
