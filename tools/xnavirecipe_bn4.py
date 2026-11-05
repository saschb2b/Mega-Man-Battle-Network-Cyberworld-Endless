#!/usr/bin/env python3
"""xnavirecipe_bn4.py [--check] [NAVI ...]: BN4 Red Sun's Navis' recipes, derived from BN4's ROM and BN6 Gregar's, as
tools/xrecipe_bn4.py derives BN4's virus families' and tools/xnavirecipe.py BN5's Navis' (docs/multirom/bn4-navis.md,
What tools/xnavirecipe.py needs for BN4; docs/devtools/recipe-generator.md).

A BN4 Navi is a first-kind object type of its own, as BN4's families are; its update runs BN4's Navi pre-step, the Navi
state runner with its state table (r6 its collision data, r7 its AI data, as BN6's status dispatcher 0x0801AF44 runs a
Navi's state: both read +0x54 and +0x58 so) and the present. In BN6 it runs as a Navi of BN6's one Navi object, by its
AI index in the grown Navi tables: BN6's Navi generic states 0-7 in its table's first eight, its own from 8, as BN5's
Navis run. This walks it as tools/xrecipe_bn4.py walks a family (its own block, the third-kind objects it spawns moved
with it, its calls paired with BN6's twins: tools/bn4_twins.py, the symbol map's, a BN6 routine of the same body), and
what a Soul Navi has more (docs/multirom/bn4-navis.md, How a BN4 Navi runs):

- MegaMan's own states its table names after its own (BN4_PLAYER_STATES; GutsMan's 7-10 MegaMan's 67-70), walked in
  MegaMan's block and copied as its own, with every routine of MegaMan's block they reach;
- the engine's routines a Soul Navi's states call that BN6 has no twin of (NAVI_COPIED: the state set, the attack's
  end), copied with it;
- its AI data's offsets as BN6 lays the AI data out (BN4_AI_NAVI +0x64, 12 bytes, at BN6's +0x80; the attack's variables
  +0x70, 0x50 bytes, at BN6's +0xA0): each `adds rN,#imm` that makes a pointer into its AI data (a register tracker from
  the state runner's r7 and +0x58 loads) given BN6's offset;
- its init's taking of its side's AI data (BN4_AI_TAKE) dropped: BN6's pool gives every Navi one (bn6_ai.h);
- its CurActions: BN4's generic 0-5 BN6's 0-5, its own from 6 BN6's from 8 (a compare, a store, the state set's r0);
- a panel's side read after BN4's getPanel (+1, BN6's +3), its palette from its init (BN4_NAVI_PALETTE), its objects' list 3
  sprites into list 4 (LIST_MOVES), and the sounds its objects play by a table's byte (SOUND_TABLES).

  python3 tools/xnavirecipe_bn4.py            derive every Navi of NAVIS: .build/xnavirecipe/bn4_report.md, what each
                                              takes and what stays open (no C file yet: none is whole)
  python3 tools/xnavirecipe_bn4.py --check    write nothing: exit 0 (no C file to hold to its generator yet)

Local only: both ROMs. Nothing of either is written: addresses, counts and CRC32s."""
import argparse
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_navis as N  # noqa: E402
import bn4_twins  # noqa: E402
import xrecipe_bn4 as X4  # noqa: E402
import xrecipe_thumb as T  # noqa: E402

ROOT = X4.ROOT
WORK = os.path.join(ROOT, '.build', 'xnavirecipe')
OUT_C = os.path.join(ROOT, 'src', 'emu', 'xnavi_bn4.c')

# The Navis derived, in the order of docs/multirom/bn4-navis.md (The order, by cost): (its first-kind type, the
# project's name, its V1's enemy id), its V2, V3 and SP after it (BN4_NAVI_FIRST_ID's rows)
NAVIS = ((0x0A, 'GutsMan', 132),)

# BN4's Navi machinery (src/emu/bn4_navis.h): the pre-step, the state runner, the present (BN4_VIRUS_PRESENT, the
# families' too), the dying, the type routine's after-step, the AI data's take: BN6's Navi object does their work
NAVI4 = dict(prestep=0x080135A8, runner=0x08013DB4, present=0x08012D62, dying=0x08010F60, after=0x080143FC, take=0x0800C09A,
             set_action=0x0800C9AC, attack_end=0x0800CA28, palette=0x0800B950)
# BN6's Navi generic states 0-7 (src/emu/bn6_navis.h BN6_NAVI_SPAWN_STATE to BN6_NAVI_BUBBLED), its table's first eight
GENERIC_NAVI6 = (0x08016381, 0x08017889, 0x080170C5, 0x080174FF, 0x080175B9, 0x080178B7, 0x08017689, 0x08017769)
# The engine's routines a Soul Navi's states call that BN6 has no twin of, copied with it (docs/multirom/bn4-navis.md,
# The engine's routines to port once): the reason each
NAVI_COPIED = {
    NAVI4['set_action']: 'the state set (BN4_SET_ACTION): r0 its CurAction, its sub-state and the attack\'s variables\' first u16 '
                         'cleared; no BN6 routine of its body (BN6\'s Navis set theirs inline); copied, its AI data\'s +0x70 BN6\'s +0xA0',
    NAVI4['attack_end']: 'a Soul Navi\'s attack\'s end (BN4_ATTACK_END): its requests\' bits, its flags, CurAction 6; no BN6 routine of '
                         'its body; copied, its CurAction BN6\'s 8',
    0x0800BDAA: 'the attack\'s end\'s clear of its AI data\'s +9, +0xA, +0xB, +0x30 and +0x32: no BN6 routine of its body; copied, '
                'its offsets kept (an interim: BN6\'s meaning of them for a Navi\'s AI data unread)',
    0x0800F734: 'a panel of its side at random (its table of panels\' masks by side, 0x0800F770, then the RNG), else its own: the '
                'callback GutsMan\'s decision hands his step (MegaMan\'s 67) in the attack\'s variables\' +0x1C; no BN6 routine of its '
                'body; copied, its table with it',
    0x0800FAD0: 'a free panel of its side at random (0x0800A244, then the RNG): no BN6 routine of its body; copied',
    0x0800A244: 'the free panels of a side (r0 the side, r2 and r3 the flags wanted and refused, r7 a buffer: a byte each, column | '
                'row << 4, r0 back their count): no BN6 routine of its body; copied, its panel\'s side read at BN6\'s +3 (BN4\'s +1)',
}
# A table of its moved objects' kinds whose bytes name a sound their init plays (BN4_PLAY_SOUND with r0 a byte of it, not a
# movs): its address -> its entries, their size, the kinds, the sound's place in an entry, the play it reaches, why; each
# kind's sound given BN6's number as a movs's would be (xrecipe_bn4.sounds_of: BN6's of the same song, else its slot)
SOUND_TABLES = {
    0x080D0A60: dict(entry=6, kinds=(0, 1, 2), sound=4, play=0x080D0B1A,
                     why="the third kind's 0x1E's kinds (6 bytes each: its speed, its +0x64, its animation, its collision's, its sound, "
                         "the status it gives), read by its init (0x080D0AAE, r7 the kind's by +4) to the code at 0x080D0A74"),
}
# A panel's fields a copied routine reads after BN4's getPanel (bn4_twins.GET_PANEL) past its type (xrecipe_bn4.panel_type's):
# BN4's offset -> BN6's (bn4.h BN4_PANEL_ALLIANCE, BN6's +3)
PANEL4TO6_FIELDS = {0x01: 0x03}
# the data a copied routine of NAVI_COPIED reads, copied with it as raw bytes (bn4_twins.COPIED_DATA's way): entry -> (lo,
# hi, why), read from the ROM
NAVI_COPIED_DATA = {
    0x0800F734: (0x0800F770, 0x0800F780, 'its table of panels\' masks by side (two words a side: 0x10, 0x0F8800A0; 0x30, 0x0F880080)'),
}
# The AI data's blocks a Soul Navi's code names (bn4.h BN4_AI_NAVI, BN4_AI_ATTACK_VARS) and BN6's in their place
# (bn6.h BN6_T1_AI_DATA: +0x80 its state, +0xA0 its attack's variables): (BN4's first, its size, BN6's first)
AI_BLOCKS = ((0x64, 0x0C, 0x80), (0x70, 0x50, 0xA0))
# The Navis' numbers (a decision of this generator's, docs/multirom/bn4-navis.md): BN4's Navi of V1 id n at seat
# (n - 128) / 4, a fixed place each (the static model, issue #82), its AI index BN4_NAVI_AI0 + seat in BN6's grown
# Navi tables (past BN5's Navis' 32-42 and the room BN5's Darkloids keep: 48 on), its ids four from
# BN4_NAVI_ID0 + 4 * seat, past every family's ids (XV_FIRST_ID + 6 * XV_FAMILIES = 0x380) and below XV_IDS (0x400)
BN4_NAVI_AI0, BN4_NAVI_ID0 = 48, 0x380
# Their room (the owner's call of 5 November 2026; src/emu/free_space.h FREE_BN4_NAVIS, xnavi_recipe.h XN_BN4_*): their homes
# (a seat each), their code and objects', then what they draw, to FREE_BRACKET
ROOM_HOMES = 0x08800000 + 0x720000
ROOM_CODE, ROOM_ART, ROOM_END = ROOM_HOMES + 0x10000, ROOM_HOMES + 0x30000, 0x08800000 + 0x790000
VERSIONS = 4
# The collision setup's r3 BN6's actor init gives every actor (0x08016FE8: movs r3,#3), the hit bits its body's hit ORs into
# the struck's (BN6_COLL_SETUP_ARG); a Navi whose init passes another keeps its own (XNavi.coll_arg). The struck's pre-step
# reads them alike in both games (BN4's Navi pre-step 0x080135E6, BN6's 0x0801AEB2): bit 1 its flag 4, bit 2 its flag 2, a
# heavier bit (BN4's 0xFC, BN6's 0x3C, 0x20 among them) its flag 0x10 or, with bit 1, its flag 0x100
COLL_ARG6 = 3
# Their battle sprites in BN6's list 2, the Navis' (src/emu/xnavi_sprites.h XNAVI_SPRITE_BN4: past BN5's 0x20-0x2A): a seat each
NAVI_SPRITE0 = 0x40
# Their sounds BN6 has no song of (the owner's call of 5 November 2026, docs/multirom/bn4.md call 11): a shared pool of BN6's
# empty song slots a Navi's take, its songs packed into its art's room at install and the pool's slots pointed at the songs
# of the Navi its battle holds (src/emu/xnavi_bn4_sounds.c). The pool: BN6's empty song slots nothing else takes (docs/rom/
# sound.md: 0x26-0x62 and fourteen past them, the families' 0x34-0x3F, BN5's 0x40-0x62 and the themes' 0x26-0x31 left
# alone), the five a movs or a byte can name (to 0xFF) first. A Navi's k-th sound its k-th slot: the ones its code names by
# a byte the low five
SOUND_POOL = (0x32, 0x33, 0x63, 0xD0, 0xE0, 0x104, 0x117, 0x148, 0x150, 0x155, 0x159, 0x15D, 0x167, 0x16B, 0x16E, 0x172)
SONG_STEP = 0x40


def ai_tracker(w, entry_regs):
    """the registers that point into its AI data, by offset, before each instruction (bn4_navis.track's rules: the state
    runner's r7, a +0x58 load from the object, adds of an immediate); and each instruction that makes such a pointer
    with an immediate: (its address, its form, rd, rn, BN4's offset it adds, the AI offset it makes)"""
    regs, makes, imm = dict(entry_regs), [], {}   # (imm: a register's movs, its address and value)
    for a in sorted(w.insns):
        i, f = w.insns[a], w.insns[a].form
        if f == 'adds' and len(i.f) == 3:
            rd, rs, rn = i.f
            base, num = (rs, rn) if rs in regs and rn in imm else (rn, rs) if rn in regs and rs in imm else (None, None)
            if base is not None:
                at, v = imm[num]
                makes.append((at, 'THUMB_MOVS', num, 0, v, regs[base] + v))
                regs[rd] = regs[base] + v
                imm.pop(rd, None)
                continue
        if f == 'movs':
            for r in range(8):
                if T.writes(i, r):
                    regs.pop(r, None)
            imm[i.f[0]] = (a, i.f[1])
            continue
        for r in range(8):
            if T.writes(i, r) or (f == 'bl' and r < 4):
                imm.pop(r, None)
        if f in T.MEM_FORMS and len(i.f) == 3:
            rd, rn, off = i.f
            if rn in regs and rn != rd:
                makes.append((a, 'MEM:' + f, rd, rn, off, regs[rn] + off))
            if f == 'ldr' and rn == 5 and off == 0x58:
                regs[rd] = 0
            elif f.startswith('ldr'):
                regs.pop(rd, None)
            continue
        if f == 'adds8' and i.f[0] in regs:
            makes.append((a, 'THUMB_ADDS', i.f[0], 0, i.f[1], regs[i.f[0]] + i.f[1]))
            regs[i.f[0]] += i.f[1]
            continue
        if f == 'adds3' and i.f[1] in regs:
            if i.f[2]:
                makes.append((a, 'THUMB_ADDS3', i.f[0], i.f[1], i.f[2], regs[i.f[1]] + i.f[2]))
            regs[i.f[0]] = regs[i.f[1]] + i.f[2]
            continue
        if f == 'hi_mov' and max(i.f) < 8 and i.f[1] in regs:
            regs[i.f[0]] = regs[i.f[1]]
            continue
        if f == 'bl':
            for r in (0, 1, 2, 3):
                regs.pop(r, None)
            continue
        for r in range(8):
            if T.writes(i, r):
                regs.pop(r, None)
    return makes


# the most each load or store's offset reaches (Thumb's imm5, scaled by its size)
MEM_MAX = {'ldr': 124, 'str': 124, 'ldrh': 62, 'strh': 62, 'ldrb': 31, 'strb': 31}


def mem_access(f, a, form, rd, rn, imm, off):
    """a load or store through a pointer into its AI data: where the pointer is the AI data's own (+0) and the field one of
    AI_BLOCKS, the offset BN6's (where it fits); a field out of the blocks noted for the trace's map (f.ai_fields)"""
    to = ai_offset6(off)
    if to is None:
        f.ai_fields.setdefault(off, []).append(a)
        return
    if off == imm and to != off:
        if to > MEM_MAX[form]:
            f.decide('AI data', f'{a:08X}: {form} of its AI data\'s +{off:#x} (BN6\'s +{to:#x}) past what the instruction reaches')
        else:
            f.imms.append(dict(at=a, form=X4.FORMS[form], rd=rd, rn=rn, was=imm, now=to,
                               why=f'its AI data\'s +{off:#x} in BN4, +{to:#x} in BN6 (AI_BLOCKS)'))


def ai_offset6(off):
    """BN6's offset for BN4's AI data offset `off` within a block of AI_BLOCKS, else None"""
    for lo, n, to in AI_BLOCKS:
        if lo <= off < lo + n:
            return to + off - lo
    return None


def ai_imms(f):
    """its AI data's pointers made by an immediate, given BN6's offset (AI_BLOCKS); an access out of the blocks a
    decision"""
    for e, w in sorted(f.walks.items()):
        entry = {7: 0} if e in f.state_entries else {}
        for a, form, rd, rn, imm, off in ai_tracker(w, entry):
            if form.startswith('MEM:'):
                mem_access(f, a, form[4:], rd, rn, imm, off)
                continue
            to = ai_offset6(off)
            if to is None:
                f.decide('AI data', f'{a:08X}: a pointer to its AI data\'s +{off:#x}, in no block BN6 lays out alike (AI_BLOCKS)')
                continue
            now = imm + to - off
            if form == 'THUMB_ADDS3' and now != imm:
                f.decide('AI data', f'{a:08X}: its AI data\'s +{off:#x} made by adds rd,rn,#imm3, which the installer\'s forms do not rewrite')
                continue
            if now > 0xFF:
                f.decide('AI data', f'{a:08X}: its AI data\'s +{off:#x} (BN6\'s +{to:#x}) made by an immediate too small for it')
                continue
            if now != imm:
                f.imms.append(dict(at=a, form=form, rd=rd, rn=rn, was=imm, now=now,
                                   why=f'its AI data\'s +{off:#x} in BN4, +{to:#x} in BN6 (AI_BLOCKS)'))


def panel_fields(f):
    """a panel's side read after BN4's getPanel, BN6's offset (PANEL4TO6_FIELDS)"""
    for e, w in sorted(f.walks.items()):
        addrs = sorted(w.insns)
        for k, a in enumerate(addrs):
            i = w.insns[a]
            if i.form != 'bl' or i.target != bn4_twins.GET_PANEL:
                continue
            for b in addrs[k + 1:k + 6]:
                j = w.insns[b]
                if j.form in ('ldrb', 'strb') and j.f[1] == 0 and j.f[2] in PANEL4TO6_FIELDS:
                    to = PANEL4TO6_FIELDS[j.f[2]]
                    f.imms.append(dict(at=b, form=X4.FORMS[j.form], rd=j.f[0], rn=0, was=j.f[2], now=to,
                                       why=f'a panel\'s side: BN4 +{j.f[2]:#x}, BN6 +{to:#x}'))
                if j.form == 'bl' or T.writes(j, 0):
                    break


# A side's player read inline from the battle state ([eToolkit +0x18], both games'): BN4's players a word each from +0x44
# (BN4_BATTLE_PLAYERS: MegaMan's side's first, the other's next, as BN4's getter 0x0800C456 reads them), BN6's a word every
# 0x10 bytes from +0xD0 by side (BN6_BATTLE_PLAYERS, BN6_BATTLE_PLAYER_STEP; its getter 0x080103BC). An index made by
# lsls #2 and adds #0x44 is given BN6's shift and base: BN4's index (its side ^ MegaMan's ^ 1) is the side wanted where
# MegaMan's side is 0, as in every battle a BN4 Navi is fought in (none a NetBattle's)
PLAYERS4, PLAYERS6 = (0x44, 2), (0xD0, 4)


def battle_players(f):
    """a side's player read from the battle state by an index (PLAYERS4): its shift and base BN6's (PLAYERS6)"""
    for e, w in sorted(f.walks.items()):
        addrs = sorted(w.insns)
        state = {}   # (a register holding eToolkit, then the battle state)
        for k, a in enumerate(addrs):
            i = w.insns[a]
            if i.form == 'hi_mov' and i.f[1] == 10 and i.f[0] < 8:
                state[i.f[0]] = 'toolkit'
                continue
            if i.form == 'ldr' and state.get(i.f[1]) == 'toolkit' and i.f[2] == 0x18:
                state[i.f[0]] = 'battle'
                continue
            if i.form == 'ldr_r' and state.get(i.f[1]) == 'battle' and k >= 2:
                sh, base = w.insns[addrs[k - 2]], w.insns[addrs[k - 1]]
                ro = i.f[2]
                if sh.form == 'lsls' and sh.f[:2] == (ro, ro) and base.form == 'adds8' and base.f[0] == ro and \
                        (base.f[1], sh.f[2]) == PLAYERS4:
                    why = 'a side\'s player read from the battle state: BN4 +0x44 a word each, BN6 +0xD0 0x10 bytes each'
                    f.imms.append(dict(at=sh.at, form='THUMB_LSLS', rd=ro, rn=ro, was=PLAYERS4[1], now=PLAYERS6[1], why=why))
                    f.imms.append(dict(at=base.at, form='THUMB_ADDS', rd=ro, rn=0, was=PLAYERS4[0], now=PLAYERS6[0], why=why))
                else:
                    f.decide('battle', f'{a:08X}: a word of the battle state read by an index BN4\'s players\' form does not make')
            for r in range(8):
                if T.writes(i, r):
                    state.pop(r, None)


def table_sounds(b4, b6, f):
    """its sounds named by a table's bytes (SOUND_TABLES): each kind's BN6's song of its number where the same, else its
    slot of BN6's (xrecipe_bn4.SOUND_SLOTS) as a byte of the table's copy, else the owner's call; the play's 'made otherwise'
    decision answered"""
    for v, t in SOUND_TABLES.items():
        if not any(t['play'] in w.bls for w in f.walks.values()):
            continue
        f.decisions = [d for d in f.decisions if not (d[0] == 'sound' and d[1] == f'a sound played at {t["play"]:08X} made otherwise')]
        for k in t['kinds']:
            at = v + t['entry'] * k + t['sound']
            snd = b4.bytes(at, 1)[0]
            if X4.song_sig(b4, X4.BN4['songs'], snd) == X4.song_sig(b6, X4.SONGS6, snd):
                f.notes.append(f'{at:08X}: its table\'s kind {k} plays {snd:#x}, BN6\'s of the same number the same song')
            elif X4.SOUND_SLOTS.get(snd, 0) > 0xFF:
                f.decide('sound', f'{at:08X}: its table\'s kind {k} plays {snd:#x} at the pool\'s {X4.SOUND_SLOTS[snd]:#x}, past a '
                                  f'byte: its shim (the owner\'s call of 5 November 2026) is built for a movs alone')
            elif snd in X4.SOUND_SLOTS:
                f.bytes.append((at, snd, X4.SOUND_SLOTS[snd], f'its table\'s kind {k}: its sound {snd:#x}, BN4\'s copied into BN6\'s slot '
                                                               f'{X4.SOUND_SLOTS[snd]:#x}'))
            else:
                f.decide('sound', f'{at:08X}: its table\'s kind {k} plays {snd:#x}, no song of BN6\'s and no slot (SOUND_SLOTS): ask')


def wide_sounds(f):
    """its sounds named by a movs whose pool slot is past a byte (the owner's call of 5 November 2026, docs/multirom/bn4.md
    call 11): the movs left as BN4's, its call to BN4_PLAY_SOUND aimed at a shim that makes r0 the slot (XvSoundCall)"""
    f.sound_calls = []
    wide = {i['at']: i for i in f.imms if i['form'] == 'THUMB_MOVS' and i['now'] > 0xFF and i['why'].startswith('its sound ')}
    for _, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != X4.BN4['sound']:
                continue
            v, chain = T.back_value(w, site, 0)
            if len(chain) == 1 and chain[0][0] in wide:
                i = wide[chain[0][0]]
                f.sound_calls.append(dict(at=site, slot=i['now'], why=f'its sound {v:#x} at the pool\'s {i["now"]:#x}, past a byte'))
    f.imms = [i for i in f.imms if i['at'] not in wide]


def set_action_imms(f):
    """its calls to the state set (r0 the CurAction, a movs): BN4's own from 6 BN6's from 8"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != NAVI4['set_action']:
                continue
            v, chain = T.back_value(w, site, 0)
            if v is None or len(chain) != 1 or chain[0][1] != 'movs':
                f.decide('curaction', f'{site:08X}: the state set with r0 from {chain}')
            elif v >= 6:
                f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=v, now=v + 2,
                                   why=f'CurAction {v} set (BN4_SET_ACTION\'s r0)'))


def navi_init(f):
    """its init's calls past what BN6's Navi init does by its records (the sprite, shadow, palette, facing, collision): a
    decision each, the AI data's take dropped (BN6's pool)"""
    w = T.walk(f.b4, f.init, region=(f.lo, f.hi))
    std = X4.INIT_STD | {NAVI4['take'], NAVI4['palette']}
    for site, t in sorted(w.bls.items()):
        if t == NAVI4['take']:
            f.notes.append(f'{site:08X}: its init takes its side\'s AI data (BN4_AI_TAKE): dropped, BN6\'s Navi init takes one from the pool')
        elif t not in std:
            f.decide('init', f'{site:08X}: its init calls {t:08X}, which BN6\'s Navi init makes nothing of')
    navi_palette(f, w)
    setup = [s for s, t in w.bls.items() if t == X4.BN4['setup_coll']]
    args = [T.back_value(w, setup[0], r)[0] for r in (1, 2, 3)] if len(setup) == 1 else None
    f.coll_arg = args[2] if args and args[2] is not None and args[2] != COLL_ARG6 else 0
    f.notes.append(f'its collision set up with {args}: BN6\'s actor init passes r3 {COLL_ARG6}, so its own '
                   f'{"is written over at its spawn\'s end (XNavi.coll_arg)" if f.coll_arg else "is the same"}')
    return w


def navi_palette(f, w):
    """its palette from its init's call of BN4's Navi palette (BN4_NAVI_PALETTE, r0 a number: its first table's byte for it
    times its second's by the version, +4): a step of 0 a palette of 0 for every version, as BN6's actor init gives a step
    of 0 (0x0800F334: its table by the version times AI data +3)"""
    sites = [s for s, t in w.bls.items() if t == NAVI4['palette']]
    if len(sites) != 1:
        return
    k, _ = T.back_value(w, sites[0], 0)
    pw = T.walk(f.b4, NAVI4['palette'], limit=0x10)
    lits = [v for _, v in sorted(pw.lits.items())]
    if k is None or len(lits) != 2:
        f.decide('struct1', f'{sites[0]:08X}: its palette from BN4_NAVI_PALETTE with r0 {k}')
        return
    step = f.b4.bytes(lits[0] + k, 1)[0]
    if step:
        f.decide('struct1', f'{sites[0]:08X}: its palette step {step} (BN4_NAVI_PALETTE\'s {k}): BN4\'s table by version against BN6\'s unread')
        return
    f.s1['palette'] = 0
    f.notes.append(f'its palette: BN4_NAVI_PALETTE\'s {k}, step 0 ({lits[0]:08X} + {k}): palette 0 for every version, BN6\'s step 0')


def derive(b4, b6, pairs, ty, name, first, alloc):
    f = X4.Family(ty, name, first)
    f.b4, f.navi_data, f.ai_fields = b4, {}, {}
    seat = (first - 128) // 4
    f.slot, f.ai, f.first_id = seat, BN4_NAVI_AI0 + seat, BN4_NAVI_ID0 + 4 * seat
    f.home = ROOM_HOMES + X4.XV_HOME_SIZE * seat
    f.lo, f.hi = X4.region_of(b4, ty)
    f.regions, f.roots, f.regions_roots = [(f.lo, f.hi, None), (N.PLAYER, N.PLAYER_END, 'MegaMan')], [], []
    tt, tw, ents = X4.type_table(b4, f)
    if not ents:
        f.decide('type', f'its routine ({f.lo:08X}) names no jump table of three')
        return f
    f.type_table, f.init, f.update = tt, ents[0] & ~1, ents[1] & ~1
    if ents[2] & ~1 != NAVI4['dying']:
        f.decide('type', f'its dying entry is {ents[2]:08X}, not BN4\'s Navis\' {NAVI4["dying"]:08X}')
    if [t for _, t in sorted(tw.bls.items())] != [NAVI4['after']]:
        f.decide('type', 'its type routine calls more than BN4\'s Navi after-step (BN6\'s Navi object does its work)')
    uw = T.walk(b4, f.update, region=(f.lo, f.hi))
    calls = [t for _, t in sorted(uw.bls.items())]
    if calls != [NAVI4['prestep'], NAVI4['runner'], NAVI4['present']]:
        f.decide('pre-step', f'its update calls {", ".join(f"{t:08X}" for t in calls)}, not BN4\'s Navi pre-step, runner and present alone')
    site = [s for s, t in uw.bls.items() if t == NAVI4['runner']]
    f.st = T.back_value(uw, site[0], 0)[0] if site else None
    states = X4.table_at(b4, f.st, f.lo, f.hi, 24) if f.st else []
    f.states = [('BN6', a) for a in GENERIC_NAVI6]
    for k in range(6):
        if k >= len(states) or states[k] != N.NAVI_GENERIC[k]:
            f.decide('states', f'its state {k} is its own, not BN4\'s generic Navi state (its own generic: the next Navis\')')
    f.own = states[6:]
    f.states += [('OWN', x | 1) for x in f.own]
    f.megaman = [k for k, x in enumerate(f.own, 6) if N.PLAYER <= x & ~1 < N.PLAYER_END]
    f.state_entries = {x & ~1 for x in f.own}
    f.s1 = X4.struct1_of(b4, f, f.init)
    navi_init(f)
    if f.s1['palette'] is not None:
        f.decisions = [d for d in f.decisions if not (d[0] == 'struct1' and d[1].endswith('gives no palette the generator reads'))]
    del f.walks[f.init]
    f.dead = [(f.lo, tw), (f.init, T.walk(b4, f.init, region=(f.lo, f.hi))), (f.update, uw)]
    saved = dict(bn4_twins.COPIED)
    bn4_twins.COPIED.update(NAVI_COPIED)
    try:
        roots = [x & ~1 for x in f.own] + f.roots
        while roots:
            X4.walk_code(b4, f, roots, {f.type_table, f.st}, pairs)
            roots = sorted(callbacks(b4, f) - set(f.walks))
    finally:
        bn4_twins.COPIED.clear()
        bn4_twins.COPIED.update(saved)
    # (MegaMan's routines its states reach are copied as routines, not his block whole)
    f.copied = sorted(set(f.copied) | {e for e in f.walks if N.PLAYER <= e < N.PLAYER_END})
    f.regions = [r for r in f.regions if r[2] != 'MegaMan']
    tables_of(b4, f)
    f.mm_bytes = sum(i.size for e, w in f.walks.items() if N.PLAYER <= e < N.PLAYER_END for i in w.insns.values())
    X4.side_sites(f)
    X4.scan(f)
    # (the state set's own store of r0 is its callers' CurAction: set_action_imms gives each call's)
    f.decisions = [d for d in f.decisions if not (d[0] == 'curaction' and d[1].startswith(f'{NAVI4["set_action"]:08X}:'))]
    ai_imms(f)
    panel_fields(f)
    battle_players(f)
    set_action_imms(f)
    X4.spawns_of(f, alloc)
    X4.pair_calls(f, pairs)
    navi_records(b4, f)
    saved = dict(bn4_twins.COPIED_DATA)
    bn4_twins.COPIED_DATA.update(f.navi_data)
    bn4_twins.COPIED_DATA.update(NAVI_COPIED_DATA)
    try:
        X4.blocks_of(b4, f, alloc)
    finally:
        bn4_twins.COPIED_DATA.clear()
        bn4_twins.COPIED_DATA.update(saved)
    X4.words_of(f, pairs)
    navi_art(b4, b6, f, alloc)
    X4.effects_of(b4, b6, f, alloc)
    pool = pool_of(b4, b6, f)
    saved = dict(X4.SOUND_SLOTS)
    X4.SOUND_SLOTS.update(pool)
    try:
        X4.sounds_of(b4, b6, f)
        table_sounds(b4, b6, f)
        wide_sounds(f)
    finally:
        X4.SOUND_SLOTS.clear()
        X4.SOUND_SLOTS.update(saved)
    songs_of(b4, f, pool, alloc)
    seen, out = set(), []
    for i in sorted(f.imms, key=lambda x: x['at']):
        if i['at'] not in seen:
            seen.add(i['at'])
            out.append(i)
    f.imms = out
    return f


def sound_ids(b4, f):
    """every sound its code plays: a movs's before BN4_PLAY_SOUND, a kind's byte of SOUND_TABLES"""
    ids = set()
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t == X4.BN4['sound']:
                v, _ = T.back_value(w, site, 0)
                if v is not None:
                    ids.add(v)
    for v, t in SOUND_TABLES.items():
        if any(t['play'] in w.bls for w in f.walks.values()):
            ids |= {b4.bytes(v + t['entry'] * k + t['sound'], 1)[0] for k in t['kinds']}
    return ids


def pool_of(b4, b6, f):
    """its sounds that are no song of BN6's and no family's (xrecipe_bn4.SOUND_SLOTS), each its slot of SOUND_POOL in order;
    one past what a movs can name (0xFF) played through a shim (wide_sounds), past the pool a decision"""
    odd = sorted(v for v in sound_ids(b4, f) if X4.song_sig(b4, X4.BN4['songs'], v) != X4.song_sig(b6, X4.SONGS6, v)
                 and v not in X4.SOUND_SLOTS)
    if len(odd) > len(SOUND_POOL):
        f.decide('sound', f'{len(odd)} sounds for the pool\'s {len(SOUND_POOL)}')
    pool = dict(zip(odd, SOUND_POOL))
    f.notes.append('its sounds in the pool: ' + ', '.join(f'{v:#x} at {slot:#x}' for v, slot in pool.items()))
    return pool


def songs_of(b4, f, pool, alloc):
    """its pool's songs, each packed into its art's room at install (xnavi_bn4_sounds.c), its bytes rounded up to SONG_STEP"""
    f.songs = []
    for v, slot in pool.items():
        room = X4.rounded(X4.song_bytes(b4, v, X4.BN4['songs']), SONG_STEP)
        f.songs.append(dict(song=v, slot=slot, at=alloc.art, room=room))
        alloc.art += room
    if alloc.art > alloc.art_end:
        f.decide('room', f'its songs end at {alloc.art:08X}, past {alloc.room}\'s art ({alloc.art_end:08X})')


def callbacks(b4, f):
    """the routines its walked code names by a literal (a Thumb pointer handed on, a callback: MegaMan's states pass
    theirs), in its own block or MegaMan's: walked and copied with it"""
    out = set()
    for w in f.walks.values():
        for v in w.lits.values():
            if v and v & 1 and T.is_code_ptr(v, b4) and (f.lo <= v & ~1 < f.hi or N.PLAYER <= v & ~1 < N.PLAYER_END
                                                         or v & ~1 in NAVI_COPIED):
                out.add(v & ~1)
    return out


def tables_of(b4, f):
    """the data its walked code names that no block holds: a table right past a copied routine of MegaMan's (its bytes to
    the next code walked), copied with that routine as raw bytes (bn4_twins.COPIED_DATA's way, NAVI_DATA), and its own
    tables just below its routine (the bytes from the lowest to its routine, a region of its own, raw)"""
    code = sorted({a for w in f.walks.values() for a in w.insns})
    ends = {}
    for e in f.copied:
        w = f.walks[e]
        ends[e] = max(max(a + i.size for a, i in w.insns.items()), max((x + 4 for x in w.lits), default=0))
    below = []
    for w in f.walks.values():
        for v in w.lits.values():
            if not v or v & 1 or v in bn4_twins.TABLES:
                continue
            if f.lo - 0x40 <= v < f.lo:
                below.append(v)
                continue
            if not N.PLAYER <= v < N.PLAYER_END:
                continue
            e = next((k for k, hi in ends.items() if hi == v), None)
            nxt = next((a for a in code if a > v), None)
            if e is None or nxt is None:
                f.decide('literal', f'{v:08X}: a table of MegaMan\'s block past no copied routine\'s end')
                continue
            f.navi_data[e] = (v, nxt, 'a table of MegaMan\'s block its copied routine reads, to the next code walked')
    if below:
        f.regions.append((min(below), f.lo, None))
        f.notes.append(f'its tables below its routine, {min(below):08X}-{f.lo:08X}: a region of its own, raw')


# A sprite of a list the families' room does not grow (3), loaded by its moved objects, copied into list 4 (which it does):
# its load's list offset made list 4's, as tools/xnavirecipe.py's NAVI_LIST_MOVES does BN5's (a list is read as the sprite
# is loaded, by its offset, and nowhere after)
LIST_MOVES = {3: 4}


def list_moves(b4, f, alloc):
    """its moved objects' loads of a sprite of LIST_MOVES' lists (r1 the list's offset, r2 its number, each a movs): the
    sprite copied into the list it moves to at a number of its own, both immediates BN6's"""
    done = set()
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != X4.BN4['sprite_load']:
                continue
            lst, c1 = T.back_value(w, site, 1)
            num, c2 = T.back_value(w, site, 2)
            if lst is None or lst & 3 or lst // 4 not in LIST_MOVES or num is None or len(c1) != 1 or len(c2) != 1 \
                    or c1[0][1] != 'movs' or c2[0][1] != 'movs':
                continue
            to = LIST_MOVES[lst // 4]
            p, raw = X4.sprite(b4, X4.BN4['sprites'], lst // 4, num)
            if raw is None:
                f.decide('sprite', f'{site:08X}: its sprite {lst // 4}:{num:#x} is not one the generator reads')
                continue
            sp = next((x for x in f.attack_sprites if (x['list'], x['index']) == (lst // 4, num)), None)
            if sp is None:
                room = X4.rounded(len(raw), X4.ROOM_ATTACK)
                sp = dict(list=lst // 4, index=num, to_list=to, to_index=alloc.list4, at=alloc.art, room=room, lz=bool(p & 0x80000000))
                f.attack_sprites.append(sp)
                alloc.list4 += 1
                alloc.art += room
            f.imms.append(dict(at=c1[0][0], form='THUMB_MOVS', rd=1, rn=0, was=lst, now=4 * to,
                               why=f'its object\'s sprite\'s list: {lst // 4} in BN4, copied into {to} (LIST_MOVES)'))
            f.imms.append(dict(at=c2[0][0], form='THUMB_MOVS', rd=2, rn=0, was=num, now=sp['to_index'],
                               why=f'its object\'s sprite: list {lst // 4} {num:#x} in BN4, list {to} {sp["to_index"]:#x} in BN6 (copied)'))
            done.add(site)
    return done


def table_kinds(f):
    """the kinds its code spawns of each moved type whose kinds' table bn4_twins.TABLES reads (the type's init loads it: its
    region holds the table's loads; r4 the kind, a constant, at each call of a spawner in that region): table -> kinds; a
    kind not constant a decision"""
    out = {}
    for v, tab in bn4_twins.TABLES.items():
        ty = next((t for t, m in f.moved3.items() if any(m['lo'] <= x < m['hi'] for x in tab['loads'])), None)
        if ty is None:
            continue
        m = f.moved3[ty]
        sp = {e for e, w in f.walks.items() if m['lo'] <= e < m['hi'] and any(t in X4.SPAWNERS4 for t in w.bls.values())}
        kinds = set()
        for e, w in sorted(f.walks.items()):
            for site, t in sorted(w.bls.items()):
                if t in sp:
                    k, _ = T.back_value(w, site, 4)
                    if k is None:
                        f.decide('sprite', f'{site:08X}: its third-kind {ty:#x} spawned with a kind (r4) not a constant')
                    else:
                        kinds.add(k & 0xFF)
        out[v] = tuple(sorted(kinds))
        f.notes.append(f'its third-kind {ty:#x}\'s kinds by its table {v:08X}: {", ".join(str(k) for k in out[v])} (its spawns\' r4)')
    return out


def navi_art(b4, b6, f, alloc):
    """what it draws, as xrecipe_bn4.art_of copies a family's (its moved objects' list 4 sprites, a table's by the kinds it
    spawns: table_kinds, not the family's that bn4_twins.TABLES names), its objects' list 3 sprites into list 4
    (LIST_MOVES), and its battle sprite in list 2, the Navis' list (BN4_NAVI_SPRITES), not list 1"""
    moved = list_moves(b4, f, alloc)
    kinds = table_kinds(f)
    saved = {v: t['kinds'] for v, t in bn4_twins.TABLES.items()}
    # (a list 4 sprite a family copied is his too, at its number: the mover finds it among the families installed before him,
    # xvirus.c sprite_of, and an effect of the same sprite is the same entry; left out of his own sprites)
    f.attack_sprites += [sp for (lst, _), sp in sorted(getattr(alloc, 'family_sprites', {}).items()) if lst == 4]
    try:
        for v, k in kinds.items():
            bn4_twins.TABLES[v]['kinds'] = k
        X4.art_of(b4, b6, f, alloc)
    finally:
        for v, k in saved.items():
            bn4_twins.TABLES[v]['kinds'] = k
    for sp in f.attack_sprites:
        if sp.get('shared') and any(sp['to_index'] in (i['now'],) and 'sprite' in i['why'] for i in f.imms) or \
                sp.get('shared') and any(b[2] == sp['to_index'] and 'list 4' in b[3] for b in f.bytes):
            f.notes.append(f'its sprite 4:{sp["index"]:#x} is {sp["shared"]}\'s copy (list 4 {sp["to_index"]:#x})')
    f.attack_sprites = [sp for sp in f.attack_sprites if not sp.get('shared')]
    f.decisions = [d for d in f.decisions if not (d[0] == 'sprite' and any(d[1].startswith(f'{x:08X}:') for x in moved))]
    alloc.list1 -= 1
    f.sprite.update(to_list=2, to_index=NAVI_SPRITE0 + f.slot)
    f.notes.append(f'its battle sprite (list {f.s1["list"]} {f.s1["index"]:#x}) at BN6\'s list 2 {NAVI_SPRITE0 + f.slot:#x} (xnavi_sprites.h '
                   'XNAVI_SPRITE_BN4), its arm points\' record the installer\'s')


def navi_records(b4, f):
    """its four enemy records (V1, V2, V3, SP): its type, its battle sprite (list 2, BN4_NAVI_SPRITES), each version"""
    st = X4.BN4['stats']
    f.recs = [b4.bytes(st + X4.BN4['stat_size'] * (f.first + v), X4.BN4['stat_size']) for v in range(VERSIONS)]
    for v, r in enumerate(f.recs):
        if r[2] != f.ty or r[5] != v or r[3] != 4 * f.s1['list'] or r[4] != f.s1['index']:
            f.decide('records', f'enemy id {f.first + v}\'s record names type {r[2]:#x}, sprite {r[3]}:{r[4]}, version {r[5]}')
    f.hp = [int.from_bytes(r[0:2], 'little') & 0xFFF for r in f.recs]
    f.element = min(int.from_bytes(r[0:2], 'little') >> 12 for r in f.recs)


def report_navi(f):
    L = [f'- region {f.lo:08X}-{f.hi:08X}, its routine\'s jump table {f.type_table:08X} (init {f.init:08X}, update {f.update:08X})',
         f'- state table {f.st:08X}: {len(f.own)} states its own from 6 (BN6\'s from 8); MegaMan\'s at {f.megaman}',
         f'- as AI {f.ai}, ids {f.first_id:#x}-{f.first_id + VERSIONS - 1:#x}; HP {f.hp}, element {f.element}',
         f'- {len(f.walks)} routines walked ({f.mm_bytes} bytes of MegaMan\'s), {len(f.copied)} copied as routines, '
         f'{len(f.imms)} immediates rewritten',
         f'- moved types: ' + ', '.join(f'3:{t:#x} as {a:#x}' for k, a, t in f.types),
         f'- blocks: ' + ', '.join(f'{lo:08X}-{hi:08X}' for lo, hi, at, _ in getattr(f, 'blocks', [])), '', '### Calls out', '']
    for t, tw, how, cls, sites, audited in getattr(f, 'pairs', []):
        L.append(f'- {t:08X} x{len(sites)}: ' + (f'{tw} ({how})' if isinstance(tw, str) else f'BN6 {tw:08X} ({how}, {cls}'
                                               f'{", audited" if audited else ""})' if tw else f'none ({how})'))
    L += ['', '### Immediates', ''] + [f'- {i["at"]:08X} {i["form"]} {i["was"]:#x} -> {i["now"]:#x}: {i["why"]}' for i in f.imms]
    L += ['', '### Bytes', ''] + [f'- {at:08X} {was:#x} -> {now:#x}: {why}' for at, was, now, why in f.bytes]
    L += ['', '### Its AI data\'s fields out of AI_BLOCKS (kept at their offsets: the engine\'s)', '']
    L += [f'- +{k:#x}: {", ".join(f"{a:08X}" for a in sorted(v))}' for k, v in sorted(f.ai_fields.items())]
    return L


def report(navis, path):
    L = ['# BN4\'s Navis as tools/xnavirecipe_bn4.py derives them', '']
    for f in navis:
        L += [f'## {f.name} (type {f.ty:#x}, enemy ids {f.first}-{f.first + VERSIONS - 1})', '']
        if hasattr(f, 'own'):
            L += report_navi(f)
        L += ['', f'### Open decisions ({len(f.decisions)})', ''] + [f'- **{k}**: {d}' for k, d in f.decisions]
        L += ['', '### Notes', ''] + [f'- {n}' for n in f.notes] + ['']
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8') as fh:
        fh.write('\n'.join(L))


def families_alloc(b4, b6, pairs):
    """the numbers BN4's families take, as tools/xrecipe_bn4.py's main derives them (BN6's copied types first, then each
    family in install order): their list 1 and list 4 numbers, effects and their sprites and the dispatch's types, which
    the Navis' continue past, their sprites and effects shared where a Navi draws the same (alloc.effects, effect_sprites);
    the code and art room the families' (the Navis' own room set after)"""
    alloc = X4.Alloc()
    for ty in sorted(bn4_twins.COPIED6):
        c = X4.xrecipe_bn4_copy.derive(b6, ty, alloc.code, ty)
        alloc.code = X4.rounded(c.room_end, X4.CODE_STEP)
        if not c.decisions:
            pairs.copies[c.spawner] = c
    alloc.family_sprites = {}
    for k, (ty, name, first) in enumerate(X4.FAMILIES):
        fam = X4.derive(b4, b6, pairs, ty, name, first, k, alloc)
        for sp in getattr(fam, 'attack_sprites', []):
            alloc.family_sprites.setdefault((sp['list'], sp['index']), dict(sp, shared=fam.name))
    return alloc


# BN6's Navi object's tables for a Navi of BN4's (src/emu/bn6_navis.h): the Navis' own pre-step (A), the empty hook A and
# hooks, the empty companion (BN5's Navis' without one), its ice block's size (BN6_ICE_SIZES' 2, a Navi of his size)
PRESTEP6, NULLSUB_NAVI6, NO_COMPANION6, ICE = 0x0801A9B9, 0x080F3BE9, 0x080114D5, 2
# the room's bases the C file names its places by (src/emu/xnavi_recipe.h): the families' generator's text names XV_CODE's and
# XV_ART's, given BN4's Navis' room's where they are in it
XV_CODE_AT, XV_ART_AT = X4.XV_CODE, X4.XV_ART

HEADER = '''/* xnavi_bn4.c: BN4 Red Sun's Navis' recipes, written by tools/xnavirecipe_bn4.py from BN4's and BN6's ROMs
 * (python3 tools/xnavirecipe_bn4.py; --check says whether this is what it writes). Do not edit: change the generator (or
 * tools/bn4_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/multirom/bn4-navis.md): addresses
 * in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA), counts and CRC32s. The Navis here are the generator's whole ones (no
 * decision open), each a Navi of BN6's Navi object at its seat's numbers (src/emu/xnavi_recipe.h XN_BN4_*), installed where
 * BN4's ROM is read, after BN4's families (their art's and the dispatch's numbers continue past theirs). */
#include "xnavi_bn4.h"

#include "bn4.h"
#include "bn6.h"
#include "rom.h"
#include "xvirus_dispatch.h"

#define OWN(a) { (a), true, 0 }
#define BN6(a) { (a), false, 0 }
'''


def rebase(text):
    """the families' generator's places (XV_CODE + n, XV_ART + n) named by BN4's Navis' room where they are in it"""
    def code(m):
        a = XV_CODE_AT + int(m.group(1), 16)
        return f'XN_BN4_CODE + 0x{a - ROOM_CODE:X}' if ROOM_CODE <= a < ROOM_ART else m.group(0)

    def art(m):
        a = XV_ART_AT + int(m.group(1), 16)
        return f'XN_BN4_ART + 0x{a - ROOM_ART:X}' if ROOM_ART <= a < ROOM_END else m.group(0)
    return re.sub(r'XV_ART \+ 0x([0-9A-F]+)', art, re.sub(r'XV_CODE \+ 0x([0-9A-F]+)', code, text))


def c_navi(f):
    """its recipe as an XNavi: the families' generator's arrays (xrecipe_bn4.c_recipe's), then its code as a family's and the
    Navi's tables' entries"""
    n = re.sub(r'[^a-z0-9]', '', f.name.lower())
    arrays = X4.c_recipe(f)[0]
    arrays = arrays[:arrays.index(f'\nstatic const XVirusFamily bn4_{n} = {{')]
    arrays = arrays.split('\n', 1)[1]
    head = (f'/* ---- BN4\'s {f.name} (its type {f.ty:#x} there, its enemy ids {f.first}-{f.first + VERSIONS - 1}), as Navi AI {f.ai} '
            f'with ids {f.first_id:#x}-{f.first_id + VERSIONS - 1:#x} ---- */')
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    if f.sound_calls:
        arrays += f'static const XvSoundCall {n}_sound_calls[] = {{\n' + ''.join(
            f'\t{{ 0x{c["at"]:08X}, 0x{c["slot"]:X} }},   /* {c["why"]} */\n' for c in f.sound_calls) + '};\n'
    L = [head, rebase(arrays), '', f'static const XNavi bn4_{n} = {{', '\t.code = {', f'\t\t.name = "{f.name} (BN4\'s)",',
         '\t\t.game = XROM_BN4_RED_SUN_US,', f'\t\t.src_ai = 0x{f.ty:02X},   /* (its first-kind type: BN4 has no AI index) */',
         f'\t\t.ai = {f.ai},', f'\t\t.first_id = XN_BN4_FIRST_ID + 0x{f.first_id - BN4_NAVI_ID0:X},',
         f'\t\t.home = XN_BN4_HOMES + 0x{f.home - ROOM_HOMES:X},']
    for field, there in (('blocks', True), ('data', True), ('calls', True), ('words', f.words), ('imms', True), ('ranges', f.ranges),
                         ('sides', f.sides), ('numbers', f.numbers), ('bytes', f.bytes), ('types', f.types), ('states', True)):
        if there:
            L.append(f'\t\t.{field} = {n}_{field}, .n{field} = {count(field)},')
    L.append('\t\t.hooks = { ' + ', '.join([f'BN6(0x{NULLSUB_NAVI6:08X})'] * 8) + ' },')
    rec = X4.BN4['stats'] + X4.BN4['stat_size'] * f.first
    L.append(f'\t\t.struct1 = 0x{rec:08X}, .struct2 = 0x{rec:08X},   /* (its V1\'s enemy record: BN4 has no struct1 or struct2) */')
    L.append('\t\t.names = { ' + ', '.join(f'0x{f.first + v:02X}' for v in range(VERSIONS)) + ' },')
    L.append(f'\t\t.ice = {ICE},')
    L.append(f'\t\t.traits = {{ .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = {"true" if f.s1["shadow"] else "false"}, '
             f'.palette = {f.s1["palette"]}, .flags = 0x{f.flags:02X} }},')
    L.append(f'\t\t.sprites = {n}_sprites, .nsprites = {count("sprites")},')
    if f.effects:
        L.append(f'\t\t.effects = {n}_effects, .neffects = {count("effects")},')
    if f.songs:
        L.append(f'\t\t.songs = {n}_songs, .nsongs = {count("songs")},   /* (its sounds in the pool: xnavi_bn4_sounds.c) */')
    if f.sound_calls:
        L.append(f'\t\t.sound_calls = {n}_sound_calls, .nsound_calls = {count("sound_calls")},')
    L += ['\t},', f'\t.versions = {VERSIONS},', f'\t.prestep = BN6(0x{PRESTEP6:08X}), .hook_a = BN6(0x{NULLSUB_NAVI6:08X}),',
          f'\t.companion = BN6(0x{NO_COMPANION6:08X}), .companion_end = BN6(0x{NO_COMPANION6:08X}), .hide = BN6(0x{NO_COMPANION6:08X}),',
          '\t.sprite_of = -1,   /* (its battle sprite its recipe\'s first: list 2 at its seat\'s number, XNAVI_SPRITE_BN4) */']
    if f.coll_arg:
        L.append(f'\t.coll_arg = 0x{f.coll_arg:02X},   /* (its init\'s collision setup\'s r3, BN6\'s actor init\'s {COLL_ARG6:#x}: the hit bits '
                 f'its body gives, read alike by both games\' pre-steps) */')
    L.append('};')
    return '\n'.join(L), f'&bn4_{n}'


def c_file(whole):
    parts, refs = [HEADER], []
    for f in whole:
        t, ref = c_navi(f)
        parts.append(t)
        refs.append(ref)
    parts.append('const XNavi *const xnavi_bn4[] = { ' + ', '.join(refs) + ' };\n'
                 'const int xnavi_bn4_count = (int)(sizeof xnavi_bn4 / sizeof *xnavi_bn4);\n')
    return '\n'.join(parts)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--check', action='store_true')
    ap.add_argument('navi', nargs='*')
    a = ap.parse_args()
    if a.check and not os.path.isdir(X4.ROM_DIR):
        print('xnavirecipe_bn4: no ROMs here: not checked')
        return 0
    b4, b6 = X4.find_roms()
    pairs = N.Pairs(b4, b6)
    alloc = families_alloc(b4, b6, pairs)
    alloc.code, alloc.code_end, alloc.art, alloc.art_end, alloc.room = ROOM_CODE, ROOM_ART, ROOM_ART, ROOM_END, 'the proposed room'
    navis = [derive(b4, b6, pairs, ty, name, first, alloc) for ty, name, first in NAVIS]
    text = c_file([f for f in navis if not f.decisions])
    if a.check:
        with open(OUT_C, encoding='utf-8') as fh:
            if fh.read() != text:
                print(f'xnavirecipe_bn4: {OUT_C} is not what the generator writes (python3 tools/xnavirecipe_bn4.py)')
                return 1
        return 0
    with open(OUT_C, 'w', encoding='utf-8') as fh:
        fh.write(text)
    navis = [f for f in navis if not a.navi or f.name in a.navi]
    path = os.path.join(WORK, 'bn4_report.md')
    report(navis, path)
    for f in navis:
        print(f'{f.name}: {len(f.decisions)} decisions open, {len(f.imms)} immediates, {len(getattr(f, "pairs", []))} calls out')
    print(f'wrote {OUT_C} ({sum(1 for f in navis if not f.decisions)} whole) and {path}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
