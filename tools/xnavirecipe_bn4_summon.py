"""xnavirecipe_bn4_summon.py: a BN4 Navi's summon as tools/xnavirecipe_bn4.py derives it (docs/multirom/bn4-navis.md, Roll
built). Roll's MegaMan's state 47 (0x080EFD07) calls BN4_SPAWN_ENEMY (0x080F00F4) with r0 an enemy id from her version's
list: a free panel of her side by BN4_FREE_PANELS (its masks by side in the routine's pool) and one draw of the RNG, then the
enemy's record (BN4_ENEMY_RECORD) and its first-kind type spawned there (BN4_SPAWN_1, its +4 the version, +5 2: BN4's generic
state 0 takes its summoned appear, 0x0801002A), its fields from the record, its sound. BN6 spawns an enemy into a fight under
way by BN6_SPAWN_ENEMY_1 (r0 its id, r1 and r2 its panel, r3 its side: its AI data's +2 1, which BN6's appear state
0x08016380 reads to take its own summoned appear, 0x080164A0), so the routine is not copied: the call is aimed at a shim of
her code room (src/emu/xnavi_init.c, xvirus_shim_summon; XnSummon) that picks the panel as BN4's does (her copy of BN4_FREE_PANELS, the same
masks, one draw of BN6's GetRNG twin), gives the id BN6's (a moved family's by its recipe, a native by its name: SUMMON_NATIVES),
spawns it by BN6's routine and plays the sound at its pool's slot.

Imported by tools/xnavirecipe_bn4.py alone. Numbers only: BN4's and BN6's addresses, read from both ROMs and each said why."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import xrecipe_bn4 as X4  # noqa: E402
import xrecipe_thumb as T  # noqa: E402

# BN4_SPAWN_ENEMY (bn4_navis.h): its routine, its pool's end (its masks' table, two words a side, at SUMMON_MASKS), its sound's
# movs, the free panels it asks (BN4_FREE_PANELS, copied: xnavirecipe_bn4.NAVI_COPIED)
SUMMON4, SUMMON_END, SUMMON_MASKS, SUMMON_SOUND_AT, FREE_PANELS4 = 0x080F00F4, 0x080F01B8, 0x080F01A8, 0x080F0196, 0x0800A244
# the shim's room in the Navi's code room (src/emu/xnavi_init.c SUMMON_ROOM: its code and pool)
SHIM_ROOM = 0x100
# a Navi's lists of the ids it summons: its BN4 type -> its table of pointers by version (each list a count, then the ids), read
# by its routine that picks the next (Roll's 0x080BC3B4: its literal 0x080BC3D8, [r5 +4] the version)
SUMMON_LISTS = {0x1D: 0x080BC3D8}
# The ids a summon gives that no moved family's recipe names: BN4's id -> (BN6's, why). A family BN6 has by its name is not
# moved (docs/multirom/bn4.md, Only what is new visits): BN6's own stands in, its versions in BN6's order (V1, V2, V3, SP)
SUMMON_NATIVES = {
    0x01: (0x01, 'Mettaur (BN4\'s 40 HP), BN6\'s own Mettaur'),
    0x02: (0x02, 'Mettaur2, BN6\'s own Mettaur2'),
    0x03: (0x03, 'Mettaur3, BN6\'s own Mettaur3'),
    0x04: (0x04, 'MettaurSP, BN6\'s own MettaurSP'),
}


# The fourth-kind types a Navi's code spawns that BN6 has no twin of, moved with it (the dispatch's fourth kind, XV_SPAWN_4):
# BN4's type -> (its block: its routine, then its spawner; why)
MOVED4 = {
    0x17: (0x080E3A98, 0x080E3AE6, "a flash where it was spawned (r1-r3): 30 frames, every 8th the effect 0x2E, its parameter "
                                   "(r4's high byte) a draw & 3 (BN4_SPAWN_EFFECT); spawned by its own spawner "
                                   "(0x080E3AD8: movs r0,#0x17, BN4_SPAWN_4) in MegaMan's states 46 and 47, Roll's warp and "
                                   "summon; no BN6 type of its body (0x080042C8's 0x00-0x5F compared)"),
}


def split_regions(f):
    """MegaMan's block without BN4_SPAWN_ENEMY, so a call of it is a call out (aimed at the shim), not walked"""
    out = []
    for lo, hi, name in f.regions:
        if name == 'MegaMan' and lo <= SUMMON4 < hi:
            out += [(lo, SUMMON4, name), (SUMMON_END, hi, name)]
        else:
            out.append((lo, hi, name))
    f.regions = out
    if f.ty in SUMMON_LISTS:
        f.roots.append(FREE_PANELS4)   # (the shim's, walked and copied with it: NAVI_COPIED's)
        f.moved4 = {}
        for ty, (lo, hi, _) in MOVED4.items():
            f.moved4[ty] = dict(lo=lo, hi=hi, routine=lo)
            f.regions.append((lo, hi, ty))
            f.roots.append(lo)
            f.notes.append(f'its fourth-kind {ty:#x} moved ({lo:08X}-{hi:08X}): ' + MOVED4[ty][2])


def summon_ids(b4, f, families):
    """the ids its lists name (SUMMON_LISTS), each with BN6's: a moved family's by its recipe (its .names and .first_id), else
    SUMMON_NATIVES', else 0 (the shim spawns none: an interim, said)"""
    v = SUMMON_LISTS.get(f.ty)
    ids = []
    for k in range(4 if v else 0):
        p = b4.u32(v + 4 * k)
        ids += [b4.u8(p + 1 + j) for j in range(b4.u8(p))]
    out = []
    for i in sorted(set(ids)):
        fam = next(((x, i - x.first) for x in families if x.first <= i < x.first + 6), None)
        if fam:
            out.append((i, fam[0].first_id + fam[1], f'{fam[0].name}\'s, moved (its recipe\'s ids)'))
        elif i in SUMMON_NATIVES:
            out.append((i, *SUMMON_NATIVES[i]))
        else:
            r = b4.bytes(X4.BN4['stats'] + X4.BN4['stat_size'] * i, X4.BN4['stat_size'])
            out.append((i, 0, f'its type {r[2]:#x} moved by no recipe and no native of BN6\'s by name: the shim spawns none (an interim)'))
    return out


def summon(b4, f, alloc, pool, families):
    """where its code calls BN4_SPAWN_ENEMY: the call aimed at the shim (its room taken from the code room), its masks, ids
    and sound for the installer (f.summon)"""
    f.summon = None
    if SUMMON4 not in f.calls:
        return
    alloc.code = X4.rounded(alloc.code, 4)
    at = alloc.code
    alloc.code += SHIM_ROOM
    f.aim[SUMMON4] = (f'XV_CODE + 0x{at - X4.XV_CODE:X}', 'BN4_SPAWN_ENEMY: the summon\'s shim (xnavi_init.c), BN6\'s '
                                                         'BN6_SPAWN_ENEMY_1 by the id BN6 numbers it')
    w = T.walk(b4, SUMMON4, region=(SUMMON4, SUMMON_END))
    snd = T.decode(b4, SUMMON_SOUND_AT)
    masks = [b4.u32(SUMMON_MASKS + 4 * k) for k in range(4)]
    if FREE_PANELS4 not in w.bls.values() or snd.form != 'movs' or SUMMON_MASKS not in w.lits.values():
        f.decide('summon', f'{SUMMON4:08X}: not BN4_SPAWN_ENEMY\'s body as read (its free panels, masks and sound)')
        return
    sound = snd.f[1]
    # (its bytes dead where a block holds them, MegaMan's state 48's pool past them: the call goes to the shim, xnavi_recipe.c)
    f.dead.append((SUMMON4, w))
    ids = summon_ids(b4, f, families)
    f.summon = dict(at=at, masks=masks, ids=ids, sound=pool.get(sound, X4.SOUND_SLOTS.get(sound, sound)), sound4=sound)
    f.notes.append(f'its summon (BN4_SPAWN_ENEMY) by a shim at {at:08X}: ' + ', '.join(f'{a:#x} as {b:#x} ({why})' for a, b, why in ids) +
                   f'; its sound {sound:#x} at {f.summon["sound"]:#x}')


def c_summon(f, n):
    """its summon as an XnSummon (src/emu/xnavi_recipe.h): (the arrays' text, its XNavi line), or ('', None)"""
    s = getattr(f, 'summon', None)
    if not s:
        return '', None
    rows = ''.join(f'\t{{ 0x{a:02X}, 0x{b:03X} }},   /* {why} */\n' for a, b, why in s['ids'])
    text = f'static const XnSummonId {n}_summon_ids[] = {{\n{rows}}};\n'
    masks = ', '.join(f'0x{m:08X}' for m in s['masks'])
    text += (f'static const XnSummon {n}_summon = {{ XV_CODE + 0x{s["at"] - X4.XV_CODE:X}, OWN(0x{FREE_PANELS4 | 1:08X}), {{ {masks} }}, '
             f'{n}_summon_ids, (int)(sizeof {n}_summon_ids / sizeof *{n}_summon_ids), 0x{s["sound"]:X} }};   /* (BN4_SPAWN_ENEMY\'s '
             f'masks by side, {SUMMON_MASKS:08X}; its sound {s["sound4"]:#x}) */\n')
    return text, f'\t.summon = &{n}_summon,   /* (its summon: xnavi_init.c) */'

