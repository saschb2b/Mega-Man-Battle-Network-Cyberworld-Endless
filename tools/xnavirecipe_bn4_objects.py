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
    0x04050201: "a collision region's spawn word (AquaMan's third kind 0x27's, 0x080D1B60, its r4 to 0x080CD7E2): BN6's own "
                "attacks pass the same to its twin (0x080CABAC's to 0x080C6BDA)",
}


def install(pairs):
    """his pairs, audits, copies and numbers into the shared tables, for the Navis' walk (after the families')"""
    pairs.twins.update({a: b for a, (b, _) in TWINS.items()})
    pairs.twin_src.update({a: 'project:' + s for a, (_, s) in TWINS.items()})
    bn4_twins.AUDITS.update(AUDITS)
    bn4_twins.NUMBERS.update(NUMBERS)


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


def facing_effects(b4, b6, f, alloc):
    """his effects spawned with his side in r4's second byte (lsls r4,r0,#8 of 0x0800ACA6's, then adds r4,#id: AquaMan's
    0x080F459E): the id the adds's, BN6's of the same look or moved as xrecipe_bn4.effects_of gives a movs's, the adds given
    BN6's id; effects_of's decision for it answered"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != X4.BN4['effect']:
                continue
            v, chain = T.back_value(w, site, 4)
            if v is not None or len(chain) != 2 or chain[0][1] != 'adds8' or chain[1][1] != 'lsls':
                continue
            sh = w.insns[chain[1][0]]
            e4 = chain[0][2]
            if tuple(sh.f[::2]) != (4, 8) or e4 >= X4.BN4['neffects']:
                continue
            e6, how = X4.effect_twin(b4, b6, e4), 'the same look'
            if e6 is None:
                e6, how = X4.move_effect(b4, f, alloc, e4), 'moved'
            if e6 is None or e6 > 0xFF:
                continue
            f.decisions = [d for d in f.decisions if d != ('effect', f'an effect spawned at {site:08X} by an id made otherwise ({chain})')]
            if e6 != e4:
                f.imms.append(dict(at=chain[0][0], form='THUMB_ADDS', rd=4, rn=0, was=e4, now=e6,
                                   why=f'its effect (its side in r4\'s second byte): {e4:#x} in BN4, {e6:#x} in BN6 ({how})'))
            f.notes.append(f'{site:08X}: its effect {e4:#x} spawned with its side: BN6\'s {e6:#x} ({how})')
