#!/usr/bin/env python3
"""xchiprecipe_bn4.py [--check] [--report] [--unit KIND:SUB:NAME ...]: BN4 Red Sun's chips' code as units for BN6's battle
engine, made from BN4's ROM and BN6 Gregar's, as tools/xchiprecipe.py makes BN5's (docs/devtools/chip-recipe-generator.md,
The BN4 path; docs/multirom/bn4-chips.md, The build plan's units 3 and 5).

BN4 runs a chip by a state of MegaMan's (BN4_PLAYER_STATES by the record's family): its support state (family 0x0C,
BN4_SUPPORT_RUN) runs its support chips and its Navi chips from one table of routines by subfamily (BN4_CHIP_ROUTINES),
where BN6 has two. A unit here is one of those routines: a spawner of a fourth-kind object, in that object's type's region,
of the shape BN6's support routines have (the element's place apart). The unit is that region (the type's routine, its
states, the spawner), walked with tools/xrecipe_bn4.py's parts as a moved family's objects are: every routine it reaches
paired with BN6's twin (tools/bn4_twins.py, the Navis' pairs, and TWINS below; else a BN6 routine of the same body), a
third- or fourth-kind type it spawns that BN6 has no twin of moved with it through the second-level dispatch, its
immediates BN6's (the element's place, the collision data's offsets, a panel's fields, its sounds and sprites). Its entry
is the spawner's copy, at BN6's grown support routines' NUMBER0 + its subfamily (src/emu/xchips_code.h XC_SUPPORT; BN5's
are at 0x2A + theirs), which its chips' records take under BN6's support family (the registry's XREG_MOVED rows, their
`sub`: tools/xreg_bn4.py MOVED, checked here).

  python3 tools/xchiprecipe_bn4.py           write src/emu/xchips_bn4.c (UNITS, each whole) and .build/xchiprecipe/bn4_report.md
  python3 tools/xchiprecipe_bn4.py --check   write nothing: fail where src/emu/xchips_bn4.c is not what it would write
  python3 tools/xchiprecipe_bn4.py --report --unit support:0x19:ShotStar   derive more units for the report alone
                                                                  (misc:0x02:Boomer a misc routine, attack:0x1e:Spreads a family)

Local only: both ROMs. Nothing of either is written: addresses, counts and CRC32s."""
import argparse
import os
import re
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bn4_twins  # noqa: E402
import bn5_twins  # noqa: E402
import xnavirecipe_bn4 as N  # noqa: E402
import xchiprecipe_bn4_navi as NV  # noqa: E402
import xnavirecipe_bn4_objects as C  # noqa: E402
import xrecipe_bn4 as X4  # noqa: E402
import xrecipe_thumb as T  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_C = os.path.join(ROOT, 'src', 'emu', 'xchips_bn4.c')
WORK = os.path.join(ROOT, '.build', 'xchiprecipe')
REGISTRY = os.path.join(ROOT, 'src', 'core', 'xchips_registry_bn4.c')
EMU = os.path.join(ROOT, 'src', 'emu')

# ---- both games' support chips (src/emu/bn4_chips.h, bn6_chips.h; docs/rom/bn4.md, Its chips' code) ----
ROUTINES4, NROUTINES4 = 0x080220E0, 0x4E   # BN4_CHIP_ROUTINES: its support state's routines by subfamily, Navi chips among them
ROUTINES6, NROUTINES6 = 0x0802CCB4, 0x2A   # BN6_CHIP_ROUTINES, BN6_CHIP_ROUTINE_COUNT
RECORDS4, NRECORDS4, RECORD = 0x080197EC, 389, 0x2C   # BN4_CHIP_RECORDS: +0xB its family, +0xC its subfamily
FAMILY4_SUPPORT, FAMILY6_SUPPORT = 0x0C, 0x15
# ... and its misc chips: MegaMan's state 0x20 (BN4_MISC_RUN) runs a routine by subfamily and ends the attack at once, BN6's
# family 0x1C (BN6_MISC_RUN) the same instruction for instruction; a 'misc' unit is one of BN4's routines there, whatever
# its shape, at BN6's grown misc routines' NUMBER0 + its subfamily (src/emu/xchips_bn4.h XC4_MISC)
MISC4, NMISC4, MISC6, NMISC6 = 0x080EC9CC, 0x12, 0x080ED730, 7   # BN4_MISC_ROUTINES, BN6_MISC_ROUTINES
FAMILY4_MISC, FAMILY6_MISC = 0x20, 0x1C
# ... and its Navi chips, the support state's too: a 'navi' unit is the chip's Navi alone, run by BN6's Navi object at BN6's
# grown Navi routines' NUMBER0 + its subfamily (tools/xchiprecipe_bn4_navi.py)
FAMILY4 = {'support': FAMILY4_SUPPORT, 'misc': FAMILY4_MISC, 'navi': FAMILY4_SUPPORT}
# ... and its attack families: a chip's family (its record's +0xB) is a state of MegaMan's (BN4_PLAYER_STATES by its number),
# run with his object in r5 as BN6's RunAIAttack runs a family of its table (BN6_ATTACK_FAMILIES), but that BN4's state makes
# r7 the attack's variables itself (his AI data's +0x70, BN6's +0xA0). An 'attack' unit is one of BN4's states whole, every
# subfamily of it, at a number of BN6's grown attack families the chips of BN4 have for their own (ATTACK_NUMBERS: 0xD0 on,
# src/emu/xattack_grow.h XATTACK_BN4), which its chips' records take as their family, their subfamily BN4's own
STATES4, NSTATES4 = 0x080EAEFC, 116   # BN4_PLAYER_STATES, BN4_PLAYER_STATE_COUNT
ATTACK_FIRST, ATTACK_END = 0xD0, 0x100   # (XATTACK_BN4, XATTACK_BN4_END)
# BN4's family -> its number, fixed for good as a unit is written (a saved state's CurAction names it), in the units' order
ATTACK_NUMBERS = {0x1E: 0xD0, 0x22: 0xD1, 0x34: 0xD3, 0x26: 0xD6, 0x59: 0xD5, 0x62: 0xD4, 0x47: 0xD7, (0x09, 'SeekBom'): 0xD2}
# An attack family whose state throws by its subfamily through a jump table of spawners (the bombs', BN4's family 9, ten
# subfamilies and the chips of three lines of work): a unit of it moves the state and the entries of the subfamilies it
# names, each the spawner of a third-kind type moved with it, and takes those subfamilies' chips alone; an entry no chip of
# the unit names returns (BN6's null routine). Another subfamily's unit copies the state again, at a number of its own
# (ATTACK_NUMBERS by the family and the unit's name): 288 bytes, and no unit's table is another's to write.
# The unit's name -> (the state's table, its entries, the subfamilies)
ATTACK_SUBS = {'SeekBom': (0x080EB4C4, 10, (9,))}
# A third-kind type several units spawn, moved once, by the first of them in UNITS: MegaMan's arm (BN4_T3_ARM, the
# third kind's 0x02, with its kinds' table, TABLES), every attack unit's; ShotStar's star (BN4_T3_STAR), Meteors'. A unit
# after its owner copies the type's spawner alone (a routine of
# 0x26 bytes in the type's code, a block of its own), its spawn given the owner's number of the dispatch and listed as
# shared (XV_TYPE_SHARED: the dispatch runs the owner's copy for it too); the kinds it spawns are rows of the owner's copy
# of the table (TABLES' kinds: every unit's): BN4's type -> (its spawner, why)
SHARED3 = {0x02: (0x080CD6E0, "MegaMan's arm's spawner (BN4_ARM_SPAWN): the arm itself, the third kind's 0x02, is the first attack "
                              "unit's copy, shared"),
           0x56: (0x080D877C, "the star's spawner with its panel, element and power handed on (BN4_T3_STAR): ShotStar's unit "
                              "moved the star; Meteors' controller spawns its kind 6 (movs r4,#6 at 0x080EAE56)"),
           0x32: (0x080D3430, "the boomerang's spawner (BN4_BOOMER_SPAWN): Boomer's unit moved the boomerang; PitRng's launcher "
                              "throws three (0x080E9A24)")}
# A chip of a written unit that still sits out (its row XREG_PENDING), by BN4's id, each its reason
ROWS_WAIT = {192: 'DrkSpred, a DarkChip of BN4\'s (the dark chips\' session\'s: its kind 0xF of the shot\'s table and its price)',
             47: 'SandRing, the generator line\'s (its records\' kinds, the ring\'s rows 1-3 of table 080DAFB0 and its spec): the family '
                 'is moved for ThunSoul\'s charged shot, whose ring is kind 0 (the attack\'s +0xC, which its power attack zeroes)'}
# A Soul's power attack (BN4_POWER_ATTACKS by its id, BN4_FORM_LOADOUTS' charged shot) that sets the attack's variables and
# returns an attack unit's family, as BN6's power attacks do (r7 the variables, r0 the family back): copied with the unit, a
# block of its own, its return given the unit's number (POWER_RETURN); the Souls' engine runs it from BN6's grown power
# attacks (src/emu/soulbn4.c, xchips_bn4_moved_at). By BN4's family: (the power attack's id, its Soul)
POWERS4, NPOWERS4 = 0x0800CA7C, 108   # BN4_POWER_ATTACKS, BN4_POWER_ATTACK_COUNT
SOUL_POWERS = {0x47: ((0x58, 'ThunSoul'),)}
SPAWN4_4, SPAWN6_4 = 0x08003344, 0x080033AC   # BN4_SPAWN_4, BN6's fourth-kind spawner (r0 the type)
# BN6's support routines' shape, Invisibl's (0x080E8886): the object spawned and its panel, element, parent, side, r6 and r7
# written; BN4's the same instruction for instruction but for the element's place (ELEMENT4, ELEMENT6: the scan's immediate)
# (the walker's form of a conditional branch is 'bcc', whatever its condition)
SHAPE = ('push', 'push', 'movs', 'bl', 'adds3', 'pop', 'bcc', 'strb', 'strb', 'strb', 'str', 'ldrb', 'strb', 'str', 'str', 'pop')
SHAPE6 = 0x080E8886
# The unit's number in BN6's grown support routines (XC_SUPPORT, 256): BN6's own 0-0x29, BN5's at 0x2A + its subfamily (to
# 0x7B), BN4's at NUMBER0 + its subfamily
NUMBER0 = 0x80
# The generic states of a support chip's object (the chip's name shown and the time freeze's turn taken, then its own, then
# its end), which every such type's tables name: BN6's of the same place in its own types' tables (Invisibl's 0x5D:
# 0x080E8828, 0x080E8848), each pair read in both ROMs (AUDITS)
TWINS = {
    0x08008BD8: (0x0800B916, 'chips'),   # its state 0 (+8): its turn asked for
    0x08008EEE: (0x0800BD34, 'chips'),   # its state 8 (+8): its turn given back, the object freed
    0x08008BF8: (0x0800B94C, 'chips'),   # its step 0 (+9): the other side's turn waited out (the same body)
    0x08008C2A: (0x0800B9B0, 'chips'),   # its step 4 (+9): the chip's name shown
    0x08008EB8: (0x0800BC88, 'chips'),   # its step 12 (+9): its own work done, its state 8
    0x08009BAC: (0x0800C9F8, 'chips'),   # a panel broken where it stands free (PanlSht's shot's own panel)
    0x0800FC48: (0x08016168, 'chips'),   # a panel of the other side's found in the object's row (Meteors' aim)
    0x08009E38: (0x0800CC52, 'chips'),   # a panel drawn as another type and side this frame (the panel changer's blink)
    # ... and what an attack state of MegaMan's calls (the 'attack' units):
    0x0800BA66: (0x0800FDB6, 'chips'),   # his counter window opened as his attack begins
    0x0800C9FC: (0x08011714, 'chips'),   # a chip's attack ended (BN4_CHIP_ATTACK_END, BN6_CHIP_ATTACK_END)
    0x0800D7CA: (0x08013774, 'chips'),   # a battle NaviStats byte of the object in r5 (BN4_NAVI_BYTE; BN6_NAVI_BY_OBJECT's get)
    0x0801288C: (0x08019F8C, 'chips'),   # an object's element set, its own and its collision data's
    # a collision region spawned (BN4's 0x080CD7E2, BN6's 0x080C6BDA: the families' pair, audited) and its +0 given bit 0x10,
    # which runs it inside a chip's time freeze: BN6's the same twenty bytes after its own spawner (a Navi chip's hit's)
    0x080CD81E: (0x080C6C16, 'chips'),
    # ... and an object that stands as its side's obstacle (PropBom's bomb):
    0x0800B230: (0x0800F614, 'chips'),   # its side's obstacle slot taken (BN4_OBSTACLE_ADD, BN6_OBSTACLE_ADD)
    0x0800B272: (0x0800F656, 'chips'),   # ... and given up (BN4_OBSTACLE_REMOVE, BN6_OBSTACLE_REMOVE)
    # ... and what a moved object the actors' runner runs calls and names (TimeBomb's bomb, ACTOR_RUN4):
    0x0801393E: (0x0801AD12, 'chips'),   # an actor's step begun: its statuses (BN4_ACTOR_STEP, BN6_ACTOR_STEP)
    0x08013F3E: (0x0801B394, 'chips'),   # its state run from its table (BN4_ACTOR_RUN, BN6_ACTOR_RUN)
    0x08010276: (0x080166AE, 'chips'),   # the engine's generic state 3 (bn4_twins.GENERIC4, GENERIC6)
    0x080112C4: (0x08017CC0, 'chips'),   # an obstacle's state 5, pushed (BN6's own bomb's table's 0x080CE928 + 0x14)
}
# A moved third-kind object that is an actor: its update hands its state table to the actors' runner in r0 (TimeBomb's
# bomb, 0x080D6CB6). BN6's runner numbers the engine's states 0-7 and an object's own from 8 where BN4's numbers 0-5 and
# from 6 (bn4_twins.GENERIC4, as a family's: tools/xrecipe_bn4.py), so the unit brings the table in BN6's layout
# (XVirusFamily.states, written at its home's XC4_HOME_STATES, which the update's pool word names: a number row), and
# its code's CurAction written or compared from 6 becomes 8 on
ACTOR_RUN4 = 0x08013F3E
HOME_STATES = 0x380   # (XC4_HOME_STATES)
AUDITS = {
    (0x0801393E, 0x0801AD12): ('ok', "an actor's step begun, each engine's own: its statuses' timers and what they end, read from "
                                     "its collision data; BN6's own bomb's update calls BN6's (0x080CE90A) where BN4's bomb's "
                                     "calls BN4's (0x080D6CB2). A moved actor runs BN6's, as every moved family does: the "
                                     "statuses are the engine's rules, not the object's"),
    (0x08013F3E, 0x0801B394): ('ok', "the actors' runner, each engine's own: r0 the object's state table, its CurAction (+9) the "
                                     "entry run, after the engine's own tests of a time freeze and its statuses; BN4's numbers "
                                     "its generic states 0-5 and an object's own from 6, BN6's 0-7 and from 8, so the table "
                                     "handed in is the unit's in BN6's layout (ACTOR_RUN4) and its own states' numbers shift"),
    (0x08010276, 0x080166AE): ('ok', "the engine's generic state 3 (struck), each game's own body: the entry of the bomb's BN4 "
                                     "table, which no code reads once the unit's table in BN6's layout is the one handed to "
                                     "the runner; BN6's generic 3 there, as in every moved family's states"),
    (0x080112C4, 0x08017CC0): ('ok', "an obstacle's state 5, pushed (a dispatcher of three steps on +0xA): step 0 the same but "
                                     "the slide's speed (0x80000 in BN4, 0x60000 in BN6), step 1 BN6's counts its moves, step 2 "
                                     "the same; the slot 5 of BN6's own bomb's table (0x080CE928) and its cube's kin "
                                     "(0x08017E26, BN4 0x0801143C). A pushed bomb slides by BN6's rule: the engine's, as a "
                                     "pushed foe's (bn4_chip_magbolt's notes); no TimeBomb spec pushes one"),
    (0x0800B230, 0x0800F614): ('ok', "an object put in its side's obstacle slots (r0 it, r1 the side, r2 which pair): the same "
                                     "instructions but the slots' place in the battle state, +0x68 in BN4 and +0xA0 in BN6, each "
                                     "engine's own state; the one a new object pushes out has its HP zeroed in both"),
    (0x0800B272, 0x0800F656): ('ok', "the object in r5 taken out of the six obstacle slots: the same instructions but the slots' "
                                     "place in the battle state (+0x68, +0xA0)"),
    (0x08008BD8, 0x0800B916): ('ok', "a support chip's object's state 0: its side's turn of the time freeze asked for and the state "
                                     "set to 4; BN6's the same place in its types' tables (Invisibl's 0x5D, 0x080E8828), with BN6's "
                                     "own bookkeeping of its freeze (its flag 0x40, its side's word at 0x0200F3B8), which BN6's "
                                     "engine reads and BN4's has none of: BN6's object's protocol, as bn4_chip_invis runs it"),
    (0x08008EEE, 0x0800BD34): ('ok', "a support chip's object's state 8: its turn given back (the other side's waiting object set "
                                     "going, the freeze ended) and the object freed, instruction for instruction BN6's with BN6's "
                                     "own clearing of its side's word (+0xC of its freeze's record) added"),
    (0x08008C2A, 0x0800B9B0): ('ok', "a support chip's object's step 4: the chip's name shown for its side (the chip's id at +0x30, "
                                     "its power at +0x2C and +0x32 where its record shows one) and the step moved on by 4 or 8 on "
                                     "the same tests; BN6's shows it by its own text routine and plays its sound 0x173 "
                                     "(tests/xdiff/legit.json's bn4 section, BN6's sound as a support chip's name shows)"),
    (0x08009BAC, 0x0800C9F8): ('ok', "a panel (r0, r1) of a floor (0x10) with none of its occupants' flags broken (type 1, its flags' "
                                     "low bits 1, the crack's sound), 1 back, else 0; BN6's the same at its offsets (flags +0x14, "
                                     "type +2 and its drawn type +6) and its own sound, but BN4's test of flag 0x20000 first, "
                                     "which only BN4's type 5 gives (BN4_PANEL_FLAGS_OF 0x30010; BN6's occupants' mask lacks that "
                                     "bit likewise): no BN6 panel type's word has it, so the test never holds in BN6"),
    (0x0800FC48, 0x08016168): ('ok', "a panel found in the object's row for the side across (its side ^ 1, its row, a required and a "
                                     "refused word of panel flags by its side, eight bytes a side, handed to BN4's 0x0800A1AE, BN6's "
                                     "0x0800D086; where none, again with a second pair of words): BN6's the same instruction for "
                                     "instruction, a routine no code of BN6's calls, and its two tables (0x080161A4, 0x080161B4) the "
                                     "same eight words as BN4's (0x0800FC84, 0x0800FC94), read in both ROMs; the search pairs only "
                                     "routines BN6 calls"),
    (0x08009E38, 0x0800CC52): ('ok', "a panel (r0 column, r1 row) marked to be drawn this frame as type r2 on side r3, the panel "
                                     "changer's blink before it sets the type: BN4's in a byte table of its own (0x02037B36, row * 8 + "
                                     "column: the type | 0x80 in the low bits, the side | 0x80 0x28 further), BN6's in the panel "
                                     "itself (its +8 the type, +9 the side, +0xD 1); each game's panel draw reads its own and shows "
                                     "the type for the frame, the type BN6's number where the changer's kinds' table is read in BN6's "
                                     "terms (TABLES), BN4's Hole BN6's 16 (src/emu/xpanels.h)"),
    (0x0800BA66, 0x0800FDB6): ('ok', "MegaMan's counter window as a chip's attack begins, each game's own rule for one flag: BN4 writes "
                                     "0x10 to his collision data's counter byte (+0xB, BN4_COLL_COUNTER) through 0x0800AFBC where the "
                                     "battle's kind (the battle state's +0xF) is 0x46 or over; BN6 writes 0x10 to its counter byte "
                                     "(+0xD, BN6_COLL_COUNTER) through 0x0800E9DC unless his AI data's kind is 2 and a link battle's "
                                     "bit 8 is clear. Each family of BN6's that is a twin of BN4's calls it at the same place (the "
                                     "spreader's 0x080EDF1A after his animation, as BN4's 0x080EC790): BN6's engine's rule for its own "
                                     "MegaMan, whose hit pass reads BN6's byte"),
    (0x0800C9FC, 0x08011714): ('ok', "a chip's attack ended. BN4: his AI requests' low six bits cleared (0x0800BFC0), the attack's "
                                     "lock-out (+5) kept at his AI data's +0x3A, +9 to +0xB, +0x30 and +0x32 cleared (0x0800BDAA), his "
                                     "collision flag 0x400000 cleared, his animation 0, his CurAction 6 and the attack's first u16 0. "
                                     "BN6: his animation 0, then by the attack's kind (its +0x1C, which BN6's own chip use sets: 2 a "
                                     "chip's) the lock-out kept at his AI data's +0x19, the requests cleared (0x0800FFEE), the flag "
                                     "0x400000 cleared, his CurAction 8 and the first u16 0: BN6's engine's end for an attack its own "
                                     "chip use began, as BN6's twin families call it at the same place (the spreader's 0x080EDFC0 for "
                                     "BN4's 0x080EC83C). The lock-out's two places are the comparison's megaman_lockout"),
    (0x0801288C, 0x08019F8C): ('ok', "an object's element set (r0): BN4 writes it to the object's +0x17 and its collision data's +2; BN6 "
                                     "to the object's +0xE, its low four bits to the collision data's +2 and its high four (BN6's second "
                                     "wheel's flags, none in a BN4 chip's element byte but the icon's the comparison names) to its +0x19"),
}
# The dispatch's numbers the chips' moved types take, by kind. The fourth kind's from 0x91 up, past BN5's 145 types (its own
# numbers in the tables) and below BN4's families' and Navis' (0xE0 on), each checked against every other recipe
# (numbers_clear). The third kind's are BN4's own type numbers: the chips' units have a page of the dispatch to themselves
# (src/emu/xvirus_dispatch.h, BN4's chips' page; XV_TYPE_MOVED4), since the tables' MOVED is one for every game and BN5's
# own numbers, which its Navis and chips come to need, are all that was left there (the first twelve units took such
# numbers until 11 November 2026). So a type is one unit's (PAGE3) unless every unit of a kind shares it (SHARED3), a
# call to BN4's third-kind spawner goes to the page's (PAGE_SPAWNERS), and no immediate changes.
TYPES4 = (0x91, 0xE0)
PAGE_SPAWNERS = {'XV_SPAWN_3': 'XV_SPAWN4_3', 'XV_SPAWN_3_LAST': 'XV_SPAWN4_3_LAST', 'XV_SPAWN_3_FIRST': 'XV_SPAWN4_3_FIRST'}
# BN4's Navis' track's third-kind numbers of the tables once its own 0xCC-0xFF are spent (tools/xrecipe_bn4.py type_number
# reads this tuple at the take, from the lowest up): BN5's own numbers that no track of BN5's moves and no Navi of BN5's
# not yet moved will (tools/bn5_twins.py dispatch_reserved names those; numbers_reserved here and build.py lint refuse one
# of them in this tuple). Its second half is the 41 numbers the chips' units held or had ahead before their page.
TYPES3_NAVIS = (0x48, 0x47, 0x46, 0x3F, 0x32, 0x2E, 0x2D, 0x2C, 0x28, 0x1A, 0x13, 0x11, 0x0F, 0x0A, 0x08, 0x06, 0x03, 0x01,
                0xCA, 0xC9, 0xC5, 0xC4, 0xC3, 0xC2, 0xB9, 0xB4, 0xAE, 0xAD, 0xA9, 0xA3, 0xA2, 0xA1, 0xA0, 0x9F, 0x9E, 0x9C,
                0x96, 0x91, 0x8E, 0x80, 0x7D, 0x7A, 0x77, 0x76, 0x75, 0x73, 0x6E, 0x67, 0x65, 0x57, 0x55, 0x54, 0x53, 0x52,
                0x50, 0x4E, 0x4C, 0x4B, 0x49)
# The units written, in install order: (kind, BN4's subfamily, a name); CANDIDATES for the report alone. Append-only, one
# unit a line: a unit's home, its dispatch numbers, its effects' ids and its sprites' list numbers follow its place here,
# so a unit put before another renumbers every one after it. Where two branches add units, the one merged first keeps
# its place and the other's go after, then the file is written again (never joined by hand)
UNITS = (
    ('support', 0x26, 'PanlSht'),
    ('support', 0x19, 'ShotStar'),
    ('misc', 0x0B, 'Repair'),
    ('misc', 0x00, 'Counter'),
    ('misc', (0x08, 0x09, 0x0A), 'WhitWeb'),
    ('misc', 0x02, 'Boomer'),
    ('misc', 0x01, 'MokoRus'),
    ('misc', 0x05, 'SidBmbo'),
    ('attack', 0x1E, 'Spreads'),
    ('attack', 0x22, 'FlmLine'),
    ('attack', 0x26, 'Thunder'),
    ('attack', 0x34, 'TwnFng'),
    ('attack', 0x59, 'MagBolt'),
    ('attack', 0x62, 'PropBom'),
    ('attack', 0x47, 'ThunSoul'),
    ('attack', 0x09, 'SeekBom'),
    ('misc', 0x11, 'Meteors'),
    ('support', 0x11, 'TimeBomb'),
    ('misc', 0x0C, 'Ligtnin'),
    ('support', (0x0B, 0x3B), 'Hole'),
    # (a Navi chip's unit, tools/xchiprecipe_bn4_navi.py: installed after BN4's Navis, src/emu/xchips_code.c)
    ('navi', 0x05, 'GutsManChip'),
    ('misc', 0x0D, 'PitRng'),
    ('navi', 0x36, 'GutPnch'),
)
CANDIDATES = ()
# BN4's sounds a unit plays that BN6 has no song of (xrecipe.song_sig), each copied into a slot of BN6's grown song table
# fixed for it (the owner's call 6, docs/multirom/bn4-chips.md: src/emu/song_table.h, slots from SONG_GROWN_FIRST up in
# the order the units took them; src/audio/xsong.c XSOUNDS, the same numbers; xsong_install_at as the unit installs), one
# slot a sound whichever unit plays it. A sound not here is a decision: the next slot
SOUND_SLOT0, SOUND_SLOT_END = 0x1DA, 0x400   # (src/emu/song_table.h SONG_GROWN_FIRST, SONG_GROWN_END)
# (the sessions moving units side by side each take theirs from a range of their own, so no two give out one slot:
# 0x1DA-0x1FF the generator's units, 0x200-0x23F the spreads' and lines', 0x240-0x27F the larger units', 0x280-0x2BF the
# dark chips', 0x2C0-0x2FF the Navi chips', 0x300-0x30F the BN4 Souls')
SOUND_SLOTS = {0x1C6: 0x1DA, 0xBF: 0x1DB,   # (PanlSht's shot's)
               0x146: 0x1DC,                # (ShotStar's star's fall)
               0xBE: 0x1DD,                 # (Counter's hit)
               0xF7: 0x1DE,                 # (WhitWeb's web)
               0xAB: 0x1DF,                 # (Boomer's boomerang)
               0xC7: 0x1E0,                 # (MokoRus's Moko)
               0x8E: 0x1E1,                 # (SidBmbo's bamboo)
               0xB0: 0x1E2, 0xE3: 0x1E3, 0xE4: 0x1E4,   # (TimeBomb's bomb set down, its count's tick and its last ticks)
               0x105: 0x280, 0x106: 0x281,  # (the panel changer's: DrkStage's, Hole's and DrkLine's)
               0x107: 0x282, 0x124: 0x29F,  # (the changer's as it sets its panels: SOUND_ADDS)
               # (the attack families' units' from 0x200 up to 0x23F, a block of their own: the units are moved side by side,
               # each session a block)
               0x8D: 0x200,                 # (the spread chips' shot)
               0xAD: 0x201,                 # (Thunder's ball's launch)
               0xF8: 0x202, 0xB9: 0x203,    # (MagBolt's bolt and its hum)
               0x133: 0x204, 0xC8: 0x205,   # (PropBom's bomb's propeller and its burst as it is struck)
               0x90: 0x206,                 # (the bombs' state's throw)
               0xA4: 0x207,                 # (GutPnch's fist's punch)
               # (the sweep's third line, units-c's, its slots from 0x240 up)
               0x120: 0x240, 0x13E: 0x241,  # (AquaUp's geyser, GreenWd's tower: the rows of 0x080DF674)
               0x143: 0x242,                # (Ligtnin's bolt)
               # (the Souls' line's from 0x300 up, a block of its own: ThunSoul's ring, SandRing's too)
               0x91: 0x300}
# (BN4's 0xB0 is a Navi chip's Navi as he appears too, GutsMan's at 0x080B9A3E: TimeBomb's slot, one song one slot)
# A call several of BN4's sounds reach, each by a branch that makes r0 one (T.back_value follows one path): read by hand,
# each sound copied at its number plus the call's add, which its shim adds to r0 (XvSoundCall.add), SOUND_SLOTS holding
# each that number
SOUND_ADDS = {
    0x080E447A: dict(sounds=(0x107, 0x124), add=0x17B,
                     why="the panel changer's sound as it sets its panels (0x080E4460-0x080E447A): none for its kinds 9 and 0xA "
                         "(Hole's, DrkLine's: beq past the call), 0x124 for 8 and 0xD (poison: movs r0,#0x25; adds r0,#0xff at "
                         "0x080E4476), 0x107 for the rest (movs r0,#8; adds r0,#0xff at 0x080E446A), DrkStage's Holes 0xC"),
}
# ... and a sound a BN4 family plays too, at the family's fixed slot (tools/xrecipe_bn4.py SOUND_SLOTS, xsong.c's XSOUNDS:
# one slot a sound whichever track copies it), a byte a movs names
FAMILY_SLOTS = {0x70: 0x3A,                 # (ShotStar's star's burst: Walla's)
                0x9A: 0x39,                 # (Repair's: Walla's)
                0x9F: 0x3D, 0xBC: 0x3F,
                0x9D: 0x36,                 # (FlmLine's flames: CirKill's)
                0xA6: 0x38}                 # (TwnFng's fangs: Elmperor's)     # (the spread shot's bubbles' and fire's bursts, by its kinds' rows: Shrimpy's and Spikey's)
# The kinds' tables a moved object reads (as tools/bn4_twins.py TABLES: its size, a row's size, the kinds the unit spawns,
# where a row keeps its sprite's number, the sprite's list and the load that reads it): copied with the unit, each spawned
# kind's sprite BN6's of the same bytes or copied at a number of the chips' (its byte in the copy set); and, where a row
# holds one of BN4's numbers BN6 numbers otherwise, bytes: a row's offset -> {BN4's value: BN6's}, each set in the copy
# (table_bytes; bytes_why says what the values are and where each pair was read)
TABLES = {
    0x080D8580: dict(size=42, entry=6, kinds=(1, 6), sprite=(None, 0), list=3, loads={0x080D85E4},
                     why="the star's kinds (the third kind's 0x56: 6 bytes each, its sprite's number in list 3, its animation, its "
                         "collision's two arguments), read by its init (0x080D85D2, by +4); ShotStar's object spawns kind 1 "
                         "(0x080E5D3E: movs r4,#1), Meteors' controller kind 6 (0x080EAE56: movs r4,#6; SHARED3)"),
    0x080E423C: dict(size=14 * 0x14, entry=0x14, kinds=(), sprite=(None, 0), list=3, loads=set(),
                     bytes={0: {2: 2, 4: 4, 6: 6, 7: 7, 9: 5, 0xB: 16}},
                     bytes_why="+0 a row's panel type: BN4's plain 2, poison 4, grass 6 and ice 7 BN6's numbers too, its holy 9 "
                               "BN6's 5 (BN4_PANEL_HOLY: BN6's 5's word), its Hole 0xB the engine's 16 (XPANEL_HOLE); its 5, 8 and "
                               "0xA (rows 5, 1 and 4) BN6 has none of, rows no moved chip's parameter names (docs/rom/bn4.md, The "
                               "panel changer and the Hole panel)",
                     why="the panel changer's kinds (BN4_PANEL_CHANGER_KINDS: 14 rows of 0x14 bytes, the panel type, the shape, "
                         "the blink, the frames and the panel search's flags by side, the same words in both games), read by its "
                         "init (0x080E43B0's literal), the row the chip's first parameter (Hole 9, DrkLine 0xA) or DrkStage's "
                         "object's two (0xC and 0xD)"),
    0x080D31EC: dict(size=0x48, entry=12, kinds=(), sprite=(None, 0), list=3, loads=set(),
                     why="the boomerang's flights (the third kind's 0x32: six rows of three words, numbers alone, no sprite), in the "
                         "bytes before its routine"),
    0x08012618: dict(size=0x18, entry=8, kinds=(), sprite=(None, 0), list=3, loads=set(),
                     why="the panels' patterns (rows of two words, a mask of the field's panels each: numbers alone), which the "
                         "blasts' spawner (0x080E4ABC) reads by its argument's low seven bits; the bomb asks rows 1 and 2, a "
                         "side's whole area (its words at 0x080D6E64). BN6 holds the same bytes for 0x50 at 0x08019C34; "
                         "copied with the unit, three rows"),
    # (the arm, BN4_T3_02_KINDS: tools/bn4_twins.py TABLES has it with Roll's kind; here the kinds the chips' states spawn,
    # each state's own bytes of them by subfamily read by hand, a row's sprite its list's offset and its number)
    0x080CD524: dict(size=0xD4, entry=4, kinds=(0x0A, 0x06, 0x0E, 0x32, 0x0C, 0x23, 0x24, 0x25, 0x2F), sprite=(0, 1), list=None, loads={0x080CD628},
                     why="the arm's kinds (the third kind's 0x02: 53 of 4 bytes, its sprite's list offset and number, its "
                         "palette, its +0xC), read by its init (0x080CD61E, by +4); the spread state spawns 0x0A, 0x06, 0x0E "
                         "and 0x32 by subfamily (its bytes at 0x080EC814), FlmLine's 0x0C (0x080ECAF6), MagBolt's 0x23 + its "
                         "subfamily, 0x23 again for MagShok's 3 (0x080EE05E-0x080EE068)"),
    0x080DF674: dict(size=0x18, entry=12, kinds=(0, 1), sprite=(0, 1), list=3, loads={0x080DF6C4}, sound=(0x0A, {0x080DF762}),
                     why="the geyser's and the tower's rows (the third kind's 0x92, AquaUp's and GreenWd's strike: 12 bytes each, its "
                         "sprite's list's offset and number, three animations with their frames, its collision's argument, and at "
                         "+0xA the sound it plays, a u16), read by its init (0x080DF6B8, by +4); the sweep spawns kind 0 on a "
                         "cracked panel and kind 1 on grass (0x080DF8DC, 0x080DF940)"),
}
# A kinds' table of a moved object whose rows name an effect and a sound by a byte each (no sprite): copied with the unit as
# TABLES' are, each kind's effect BN6's of the same look or moved (its byte the id), its sound a byte BN6 plays the same song
# at (a family's slot, FAMILY_SLOTS, or BN6's own number); the kinds are the parameters of the unit's chips that play; the
# sites that spawn an effect or play a sound by a row's byte (effect_sites, sound_sites: no immediate there to change)
ROW_TABLES = {
    0x080E4A00: dict(size=0x0A, entry=2, kinds=(1,), effect=0, sound=1,
                     effect_sites={0x080E4A78}, sound_sites={0x080E4A84},
                     why="the blasts' kinds (the fourth kind's 0x24: five of 2 bytes, the effect it draws on each panel of its "
                         "list and its sound, 0 none), read each time it draws one (0x080E4A70, by +4); TimeBomb's bomb spawns "
                         "kind 1 (0x080D6E2E: movs r4,#1), the burst 0x00 and no sound; 0, 2, 3 and 4 are other chips', left BN4's"),
    0x080CF3DC: dict(size=0x44, entry=4, kinds=(4, 5, 6, 7, 8, 9, 0x0A, 0x0B), effect=2, sound=3,
                     effect_sites={0x080CF55E}, sound_sites={0x080CF56A},
                     why="the spread shot's kinds (the third kind's 0x12: 17 of 4 bytes, its burst's shape, its collision's "
                         "argument, its burst's effect, its sound), read by its init (0x080CF452, by +4, the chip's first "
                         "parameter): 4-6 Bubbler, Bub-V and BublSide, 7 BubSprd, 8-0xA HeatShot, Heat-V and HeatSide, 0xB HeatSprd; "
                         "0-3 are Spreader's (a run's by BN6's own) and 0xC-0x10 other families' and DrkSpred's, left BN4's"),
    # (a table whose rows name a sound alone, `effect` None; `queue` where that sound's slot is past a byte (SOUND_SLOTS): the
    # sites that play it by the row's byte called through the unit's shim that gives r0 the slot, every kind moved its one sound)
    0x080DAFB0: dict(size=12, entry=3, kinds=(0,), effect=None, sound=2, effect_sites=set(), sound_sites={0x080DB05C}, queue=True,
                     why="the ring's kinds (the third kind's 0x6D: 4 of 3 bytes, its speed, a flag, its sound), read by its init "
                         "(0x080DB01A, by +4), the state passing the attack's +0xC (0x080EDCE2): ThunSoul's charged shot kind 0 "
                         "(its power attack 0x58 zeroes +0xC; seen in both traces); the rest SandRing's (ROWS_WAIT)"),
}
for _v, _t in ROW_TABLES.items():
    TABLES[_v] = dict(size=_t['size'], entry=_t['entry'], kinds=(), sprite=(None, 0), list=3, loads=set(), why=_t['why'])

# The numbers the chips' copies take, each from the top of its range down (the late families' way, tools/xrecipe_bn4.py):
# effect ids 0xCF-0xB2 (below BN5's chips' 0xD0 on, above BN4's Navis' from 0xA0 up), list 5's 0xEF-0xC0, list 4's 0x9F-0x80,
# list 3's from 0x70 up to 0xDF (past BN6's own 105, below BN5's chips' 0xE4 on), and list 1's 0x7F-0x40 (past BN5's
# families' 0x20-0x36, below BN4's families' 0xC0 on and BN5's chips' from 0xFF down). The bottoms of three of them are
# BN4's Navis' track's since 11 November 2026 (its own ranges spent at MetalMan; tools/xrecipe_bn4.py EFFECT_MORE,
# LIST5_MORE, LIST4_MORE): effect id 0xB1 (with 0xE0-0xEC, which no recipe named; 0xB0 is BN5's Meddy's, xnavi_bn5.c), list 5's
# 0xB0-0xBF, list 4's 0x70-0x7F
CHIP_EFFECTS, CHIP_LIST5, CHIP_LIST4, CHIP_LIST3, CHIP_LIST1 = (0xCF, 0xB2), (0xEF, 0xC0), (0x9F, 0x80), (0x70, 0xE0), (0x7F, 0x40)
ROOM_SPRITE = 0x100
# An immediate of a unit's code that names a place of BN4's engine BN6 keeps elsewhere, read by hand in both games' code:
# BN4's address -> (the instruction's form, its register, BN4's number, BN6's, why)
IMMS = {
    0x0800C728: dict(form='THUMB_ADDS', rd=0, rn=0, was=0x68, now=0xA0,
                     why="the battle state's obstacles by side: BN4 +0x68, BN6 +0xA0 (BN4_OBSTACLE_ADD 0x0800B230 and BN6_OBSTACLE_ADD "
                         "0x0800F614, the same routine but for that adds; three words a side, 0xC apart in both)"),
    # Counter's walk of the other side's players (0x080CFE1A-0x080CFE2C: its side ^ 1 ^ MegaMan's, lsls #2, adds #0x44, then a
    # word each): BN6's players a word every 0x10 bytes a side from +0xD0 (tools/xrecipe_bn4.py PLAYERS4, PLAYERS6), the side's
    # own four 4 apart as BN4's three
    0x080CFE26: dict(form='THUMB_LSLS', rd=0, rn=0, was=2, now=4,
                     why="a side's players in the battle state: BN4 a word a side's first from +0x44, BN6 0x10 bytes a side from +0xD0"),
    0x080CFE28: dict(form='THUMB_ADDS', rd=0, rn=0, was=0x44, now=0xD0,
                     why="a side's players in the battle state: BN4 +0x44, BN6 +0xD0"),
    # The spread family's state (0x1E): whose arm (the battle NaviStats' Navi, 0 MegaMan) and MegaMan's animation
    0x080EC796: dict(form='THUMB_MOVS', rd=1, rn=0, was=0x23, now=0x29,
                     why="the battle NaviStats' Navi, 0 MegaMan (his arm's animation by it): BN4 +0x23 (BN4_NAVI_NAVI), BN6 +0x29 "
                         "(BN6_NAVI_SIDE_NAVI)"),
    0x080EC788: dict(form='THUMB_MOVS', rd=0, rn=0, was=0x0E, now=0x0B,
                     why="MegaMan's animation as he fires a spread chip: BN4's 0x0E, BN6's 0x0B of its own sprite of him (BN6's twin "
                         "family 0x25 sets it at 0x080EDF14, through 0x0800F2B6, which stores it and 0xFF as BN4's state does)"),
    # FlmLine's state (0x22): whose arm, and MegaMan's two animations (each art's number of the animation of the same frames:
    # BN4's 0x0D one frame of 2, BN6's 0x0A; BN4's 0x11 frames of 3 and 2, BN6's 0x0D; both ROMs' art read)
    0x080ECAEA: dict(form='THUMB_MOVS', rd=1, rn=0, was=0x23, now=0x29,
                     why="the battle NaviStats' Navi, 0 MegaMan (his arm's animation by it): BN4 +0x23 (BN4_NAVI_NAVI), BN6 +0x29 "
                         "(BN6_NAVI_SIDE_NAVI)"),
    0x080ECAD4: dict(form='THUMB_MOVS', rd=0, rn=0, was=0x0D, now=0x0A,
                     why="MegaMan's animation as he readies FlmLine: BN4's 0x0D, BN6's 0x0A of its own sprite of him (one frame of "
                         "2 in each art)"),
    0x080ECB14: dict(form='THUMB_MOVS', rd=0, rn=0, was=0x11, now=0x0D,
                     why="MegaMan's animation as he fires FlmLine: BN4's 0x11, BN6's 0x0D of its own sprite of him (frames of 3 "
                         "and 2 in each art)"),
    # PropBom's state (0x62): whose bomb's height (its table by the Navi), and MegaMan's animation (Thunder's throw)
    0x080EE58A: dict(form='THUMB_MOVS', rd=1, rn=0, was=0x23, now=0x29,
                     why="the battle NaviStats' Navi, 0 MegaMan (the bomb's height by it): BN4 +0x23 (BN4_NAVI_NAVI), BN6 +0x29 "
                         "(BN6_NAVI_SIDE_NAVI)"),
    0x080EE55C: dict(form='THUMB_MOVS', rd=0, rn=0, was=0x12, now=0x0E,
                     why="MegaMan's animation as he throws PropBom: BN4's 0x12, BN6's 0x0E of its own sprite of him (frames of 1, 2, "
                         "2 and 3 in each art)"),
    # MagBolt's state (0x59): whose arm, and MegaMan's animation (BN4's 0x0D, BN6's 0x0A: FlmLine's ready pose)
    0x080EE052: dict(form='THUMB_MOVS', rd=1, rn=0, was=0x23, now=0x29,
                     why="the battle NaviStats' Navi, 0 MegaMan (his arm's animation by it): BN4 +0x23 (BN4_NAVI_NAVI), BN6 +0x29 "
                         "(BN6_NAVI_SIDE_NAVI)"),
    0x080EE044: dict(form='THUMB_MOVS', rd=0, rn=0, was=0x0D, now=0x0A,
                     why="MegaMan's animation as he holds MagBolt out: BN4's 0x0D, BN6's 0x0A of its own sprite of him (one frame of "
                         "2 in each art)"),
    # Thunder's state (0x26) and TwnFng's (0x34): MegaMan's animation, BN4's 0x12 (frames of 1, 2, 2 and 3), BN6's 0x0E of the
    # same frames in its own art of him
    0x080ECE3C: dict(form='THUMB_MOVS', rd=0, rn=0, was=0x12, now=0x0E,
                     why="MegaMan's animation as he throws Thunder: BN4's 0x12, BN6's 0x0E of its own sprite of him (frames of 1, 2, "
                         "2 and 3 in each art)"),
    0x080ED4EA: dict(form='THUMB_MOVS', rd=0, rn=0, was=0x12, now=0x0E,
                     why="MegaMan's animation as he throws TwnFng: BN4's 0x12, BN6's 0x0E of its own sprite of him (frames of 1, 2, "
                         "2 and 3 in each art)"),
    0x080CFE38: dict(form='THUMB_LDRB', rd=1, rn=1, was=0x0B, now=0x0D,
                     why="a player's collision data's counter byte: BN4 +0xB (BN4_COLL_COUNTER), BN6 +0xD (BN6_COLL_COUNTER)"),
    # The Moko's hit, struck and pushed back: BN4's struck pass makes it of the hit's bit 0x01 with a push bit (0xFC:
    # 0x0801360C-0x08013622, status 0x100 set and the flinch's 0x4 cleared), BN6's of bit 0x40 with a push bit (0x3C:
    # 0x0801AC06-0x0801AC2E, the same two stores); with BN4's 0x21 BN6 pushes and does not strike (status 0x10)
    0x080D23E0: dict(form='THUMB_MOVS', rd=3, rn=0, was=0x21, now=0x61,
                     why="its hit's bits (the collision setup's r3): struck and pushed is bit 0x01 with a push bit in BN4 "
                         "(BN4_STRUCK_PUSHED), bit 0x40 with one in BN6 (BN6_STRUCK_PUSHED)"),
    # MagBolt's bolt's hit lands with the pull's (the state's hit of bits 0x08 each frame, 0x080EE10C's word): BN4's struck pass
    # reads the two together, 0x0B, as struck and pushed, which BN6's reads of bit 0x40 (the Moko's row above)
    0x080DBCD2: dict(form='THUMB_MOVS', rd=3, rn=0, was=0x03, now=0x43,
                     why="its hit's bits (the collision setup's r3): with the pull's push bit, struck and pushed is bit 0x01 in "
                         "BN4 (BN4_STRUCK_PUSHED), bit 0x40 in BN6 (BN6_STRUCK_PUSHED)"),
    # GutPnch's punch, struck and pushed back a panel (the region's bits, r7 of BN4's 0x080CD81E): the Moko's row above
    0x080C855A: dict(form='THUMB_MOVS', rd=7, rn=0, was=0x21, now=0x61,
                     why="its punch's hit's bits (the collision region's r7): struck and pushed is bit 0x01 with a push bit in BN4 "
                         "(BN4_STRUCK_PUSHED), bit 0x40 with one in BN6 (BN6_STRUCK_PUSHED)"),
}
# A routine of BN4's the project pairs with BN6's own for the families (tools/bn4_twins.py TWINS) that a chip's unit moves
# instead, with the object type it spawns: BN6's own object draws BN6's art, which a family's rule allows by its foe and a
# chip's battle has none for. BN4's spawner -> why
UNIT_MOVES = {
    0x080D2156: "the thunder's spawner (BN4_SPAWN_THUNDER): Billy's recipe runs BN6's own third kind's 0x2A for it, drawn from "
                "BN6's art; Thunder1-3's unit moves BN4's, its sprite copied, so the chip's ball is BN4's in every battle",
}
# A routine of BN4's engine a unit copies where the project pairs it with BN6's (as tools/bn4_twins.py COPIED, for the unit
# alone): the unit's name -> BN4's routine -> why
UNIT_COPIED = {
    'TimeBomb': {0x08111888: "a remainder (swi 6; adds r0,r1,#0; bx lr: r0 mod r1), which its placement draws a panel by "
                             "(0x080E4D54); BN6 has the three instructions only inline (0x08010C00, searched), so copied: no "
                             "word of it names a place"},
    'MagBolt': {0x08012D9A: "the collision data's panels set from its place, with the direction it moved by (BN4's 0x0800AF90, "
                            "copied too): BN6's twin 0x0801A04C asks 0x0800E994, which answers a neighbouring panel alone (5 "
                            "else), where BN4's answers every panel; the bolt is put on its panel from none (4 in BN4, 5 in BN6)"},
    'Ligtnin': {0x08014446: "a third-kind object's sprite each frame (BN4_T3_SPRITE_STEP: unless paused, a new animation set up where "
                            "its +0x10 is not its +0x11, else the sprite updated): BN6's nearest (0x0801BBF4) also updates the sprite "
                            "in the frame its animation is set up, a frame ahead of BN4's (AquaUp's geyser's count at its +0x91, "
                            "bn4_chip_aquaup1 at t44); copied, as BN4's Navis copy it (tools/xnavirecipe_bn4.py), its pause test "
                            "BN6's"},
}
# BN4's misc run's wait (bn4_chips.h BN4_MISC_WAIT): after a subfamily's routine, cmp r0,#SUB; bne; movs r0,#FRAMES; strh into
# the misc state's count, where every other subfamily's attack ends at once. BN6's run waits for its own 0x14 alone, so a unit
# of that subfamily is given the frames (XChipUnit.wait: the installer's wrapper)
MISC_WAIT4 = 0x080EC9AC


def misc_wait(b4, sub):
    """the frames BN4's misc run waits after subfamily `sub`'s routine, read from its code: 0 none"""
    cmp_, bne, movs, strh = (b4.u16(MISC_WAIT4 + 2 * k) for k in range(4))
    if cmp_ & 0xFF00 != 0x2800 or bne & 0xFF00 != 0xD100 or movs & 0xFF00 != 0x2000 or strh != 0x8238:
        sys.exit('xchiprecipe_bn4: BN4\'s misc run\'s wait is not at MISC_WAIT4 as it was read')
    return movs & 0xFF if cmp_ & 0xFF == sub else 0


# A call into another object type's code whose BN6 twin the search found by its body alone: the two types compared to their
# depth by the differential or by hand before the pair is trusted (the census's lesson: docs/multirom/bn4-chips.md, The six
# "on BN6's code" are not): BN4's routine -> why BN6's twin serves. One not here is a decision
TYPE_TWINS = {}
# A panel's fields a moved object reads after BN4's getPanel (bn4.h BN4_PANEL_*): BN4's offset -> BN6's
PANEL4TO6_FIELDS = {0x01: 0x03}

# ---- where they go (src/emu/xchips_bn4_code.h; src/emu/free_space.plan FREE_BN4_CHIPS) ----
EMU_FREE = 0x08800000
X4C_AT = EMU_FREE + 0x0EF000
X4C_HOMES, X4C_HOME_SIZE, X4C_UNITS = X4C_AT, 0x400, 96
X4C_CODE = X4C_HOMES + X4C_HOME_SIZE * X4C_UNITS
X4C_ART = X4C_AT + 0x30000
X4C_END = X4C_AT + 0x50000


class Alloc(X4.Alloc):
    """the chips' numbers and room: their code and art in their own region, their types' numbers the chips' own"""

    def __init__(self):
        super().__init__()
        # (the art has no place of its own: the room's rows, c_unit; `art` only counts what the units draw)
        self.code, self.code_end, self.art, self.art_end, self.room = X4C_CODE, X4C_ART, X4C_ART, X4C_ART + 0x10000000, 'BN4\'s chips'
        self.types = {1: 0x100, 3: 0x100, 4: TYPES4[0]}
        self.page3 = {}           # (a third-kind type of the chips' page: the unit that moves it)
        self.late_effect, self.late_list5, self.late_list4, self.list3 = CHIP_EFFECTS[0], CHIP_LIST5[0], CHIP_LIST4[0], CHIP_LIST3[0]
        self.late_list1 = CHIP_LIST1[0]
        self.list3_sprites = {}   # (BN4's list 3 number copied: the copy)
        self.types1 = NV.TYPES1[0]   # (the chips' Navis' first-kind dispatch numbers)
        self.shared3 = {}         # (a third-kind type of SHARED3 moved: its number of the dispatch, its owner)


# A literal of a unit's code that is a number, not an address, kept (as tools/bn4_twins.py NUMBERS, read by hand)
NUMBERS = {
    0x04000000: "a panel's flag, the thunder's search for its target by its side (BN4 0x080D21E8 after its panel's flags, "
                "0x08009A86; BN6's own thunder tests the same bit, 0x080CAE60)",
    0x08000000: "... the other side's (BN4 0x080D2208; BN6's 0x080CAE66)",
    0x0405FF11: "PropBom's bomb's blast's hit word (BN4 0x080DED2C, r4 of the hit area's spawn 0x080CD7E2): its bits, its "
                "reach and its frames, bytes of a number as MagBolt's pull's",
}


def install(pairs):
    bn4_twins.NUMBERS.update(NUMBERS)
    pairs.twins.update({a: b for a, (b, _) in TWINS.items()})
    pairs.twin_src.update({a: 'project:' + s for a, (_, s) in TWINS.items()})
    bn4_twins.AUDITS.update(AUDITS)


def chips_of(b4, kind, sub):
    """BN4's chip records that run the unit: (id, its record)"""
    out = []
    for i in range(1, NRECORDS4):
        r = b4.bytes(RECORDS4 + RECORD * i, RECORD)
        if r[0xB] == FAMILY4[kind] and r[0xC] == sub and any(c != 0xFF for c in r[0:4]):
            out.append((i, r))
    return out


def family_chips(b4, fam):
    """BN4's chip records of an attack family, every subfamily's: (id, its record)"""
    out = []
    for i in range(1, NRECORDS4):
        r = b4.bytes(RECORDS4 + RECORD * i, RECORD)
        if r[0xB] == fam and any(c != 0xFF for c in r[0:4]):
            out.append((i, r))
    return out


def shape_of(b4, b6, f, entry):
    """the unit's routine: BN6's support routines' shape (SHAPE6 the control), the fourth-kind type it spawns"""
    w, w6 = T.walk(b4, entry), T.walk(b6, SHAPE6)
    forms = lambda x: tuple(x.insns[a].form for a in sorted(x.insns))
    if forms(w6) != SHAPE:
        sys.exit(f'xchiprecipe_bn4: BN6\'s support routine at {SHAPE6:08X} is not the shape this tool reads')
    if forms(w) != SHAPE or [t for _, t in sorted(w.bls.items())] != [SPAWN4_4]:
        f.decide('entry', f'its routine ({entry:08X}) is not the common spawner (BN6\'s {SHAPE6:08X}): another kind of unit')
        return None
    site = next(iter(w.bls))
    ty, chain = T.back_value(w, site, 0)
    if ty is None or len(chain) != 1 or chain[0][1] != 'movs':
        f.decide('entry', f'its routine ({entry:08X}) spawns a type made otherwise')
        return None
    return ty


def type_number(f, alloc, kind, ty=None):
    """the dispatch's number of a kind for a chip's moved type: the third kind's BN4's own, on the chips' page (one unit's:
    a second unit that moves it a decision, SHARED3 its answer); the fourth kind's next"""
    if kind == 3:
        who = alloc.page3.setdefault(ty, f.name)
        if who != f.name:
            f.decide('type', f'its third-kind type {ty:#x} is {who}\'s on the chips\' page: one copy a type (SHARED3)')
        return ty
    n = alloc.types[kind]
    alloc.types[kind] += 1
    if kind != 4 or n >= TYPES4[1]:
        f.decide('type', f'the chips\' dispatch numbers of the {kind} kind are spent ({n:#x} wanted)')
        return 0
    return n


def derive(b4, b6, pairs, kind, sub, name, alloc, slot):
    # (a pair of the families' a chip's unit does not take: the type moved, UNIT_MOVES; the pairs' memo of it dropped)
    for a in UNIT_MOVES:
        pairs.twins.pop(a, None)
        pairs.memo.pop(a, None)
    f = X4.Family(0, name, 0)
    f.late = True   # (its copies' numbers from the top of the chips' ranges down: CHIP_EFFECTS, CHIP_LIST5, CHIP_LIST4)
    # (a unit of several subfamilies: their routines in one block, the first's the unit's entry, the rest rows that share it)
    subs = tuple(sub) if isinstance(sub, (tuple, list)) else (sub,)
    sub = subs[0]
    f.subs = subs
    f.kind, f.sub, f.number, f.slot = kind, sub, NUMBER0 + sub, slot
    f.b4, f.b6, f.home = b4, b6, X4C_HOMES + X4C_HOME_SIZE * slot
    f.update, f.type_table, f.st, f.own, f.dead, f.copied = None, 0, 0, [], [], []
    f.shared = []   # (the types it spawns as an earlier unit's copy: kind, the dispatch's number, BN4's type, the owner)
    f.chips = [c for x in subs for c in chips_of(b4, kind, x)] if kind in FAMILY4 else family_chips(b4, sub) if kind == 'attack' else []
    if kind == 'attack' and name in ATTACK_SUBS:
        f.chips = [(i, r) for i, r in f.chips if r[0xC] in ATTACK_SUBS[name][2]]
    f.wait = misc_wait(b4, sub) if kind == 'misc' else 0
    if f.wait:
        f.notes.append(f'BN4\'s misc run waits {f.wait:#x} frames after its routine ({MISC_WAIT4:08X}): its entry the installer\'s wrapper')
    if kind == 'attack':
        if not 6 <= sub < NSTATES4 or sub in FAMILY4.values():
            f.decide('unit', f'an attack unit {sub:#x}: no state of MegaMan\'s a chip\'s family names')
            return f
        f.number = ATTACK_NUMBERS.get((sub, name), ATTACK_NUMBERS.get(sub, 0))
        if not ATTACK_FIRST <= f.number < ATTACK_END:
            f.decide('unit', f'its family {sub:#x} has no number of BN6\'s grown attack families (ATTACK_NUMBERS: the next from {ATTACK_FIRST:#x})')
    elif kind not in FAMILY4 or not 0 <= sub < (NMISC4 if kind == 'misc' else NROUTINES4):
        f.decide('unit', f'a {kind} unit {sub:#x}: the support and misc states\' routines and the attack states alone are read')
        return f
    f.regions, f.regions_roots, f.moved4, f.powers = [], [], {}, []
    saved_copied = dict(bn4_twins.COPIED)
    bn4_twins.COPIED.update(UNIT_COPIED.get(name, {}))
    if kind == 'attack':
        # (the state's code: its routine to the next state's, its phases' table and their routines in it)
        states = sorted({b4.u32(STATES4 + 4 * i) & ~1 for i in range(NSTATES4)})
        f.entry = b4.u32(STATES4 + 4 * sub) & ~1
        f.lo, f.hi = f.entry, states[states.index(f.entry) + 1] if f.entry != states[-1] else f.entry
        if f.hi - f.lo > 0x800 or f.hi == f.lo:
            f.decide('entry', f'its state ({f.entry:08X}) ends at no state\'s start')
            return f
        f.regions, f.roots = [(f.lo, f.hi, 'state')], [f.entry]
        f.notes.append(f'MegaMan\'s state {sub:#x} ({f.lo:08X}-{f.hi:08X}), the family of its chips\' records: moved')
        f.unreached = {}
        if name in ATTACK_SUBS:
            table, n, mine = ATTACK_SUBS[name]
            entries = [b4.u32(table + 4 * x) & ~1 for x in range(n)]
            for x in mine:
                hit = X4.t3_of(b4, entries[x])
                if hit is None or hit[0] is None:
                    f.decide('entry', f'its subfamily {x:#x}\'s entry ({entries[x]:08X}) lies in no third-kind type\'s code')
                    continue
                ty, lo, hi = hit
                f.regions.append((lo, hi, ty))
                f.roots += [entries[x], lo]
                f.moved3[ty] = dict(lo=lo, hi=hi, routine=lo)
                f.notes.append(f'its subfamily {x:#x}\'s entry ({entries[x]:08X}) lies in the third-kind {ty:#x}\'s code ({lo:08X}-{hi:08X}): moved')
            f.unreached = {e: x for x, e in enumerate(entries) if e not in [entries[y] for y in mine]}
        for pid, soul in SOUL_POWERS.get(sub, ()):
            p = b4.u32(POWERS4 + 4 * pid) & ~1 if pid < NPOWERS4 else 0
            bn4_twins.COPIED[p] = f'{soul}\'s power attack {pid:#x}, which returns the family'
            f.roots.append(p)
            f.powers.append((pid, p, soul))
            f.notes.append(f'{soul}\'s power attack {pid:#x} ({p:08X}), which returns the family: copied with it')
    elif kind == 'support':
        # (each subfamily's routine the common spawner of a fourth-kind type of its own, each type moved, what they share
        # once: DrkStage's and Hole's objects both spawn the panel changer)
        f.entries = [b4.u32(ROUTINES4 + 4 * x) & ~1 for x in subs]
        f.entry, f.roots = f.entries[0], []
        for e in f.entries:
            hit = C.t4_of(b4, e)
            ty = shape_of(b4, b6, f, e)
            if ty is None or hit is None or hit[0] != ty:
                f.decide('entry', f'its routine ({e:08X}) does not lie in the region of the type it spawns')
                return f
            lo, hi = hit[1:]
            if e == f.entry:
                f.ty, f.lo, f.hi = ty, lo, hi
            f.regions.append((lo, hi, ('4', ty)))
            f.roots += [e, lo]
            f.moved4[ty] = dict(lo=lo, hi=hi, routine=lo)
            f.notes.append(f'its fourth-kind {ty:#x} ({lo:08X}-{hi:08X}), which its routine ({e:08X}) spawns: moved')
    elif kind == 'navi':
        # (a Navi chip: its routine the common spawner of its object, which BN6's Navi object stands in for; the unit the chip's
        # Navi's first-kind type, its entry his spawner)
        f.object_entry = b4.u32(ROUTINES4 + 4 * sub) & ~1
        ty = shape_of(b4, b6, f, f.object_entry)
        if ty is None or not NV.begin(b4, b6, f, ty):
            return f
    else:
        f.entry = b4.u32(MISC4 + 4 * sub) & ~1
        hit = X4.t3_of(b4, f.entry)
        if hit is not None and hit[0] is not None:
            # (a routine in a third-kind type's code, Counter's in its hit's: the type moved, its region the unit's block)
            ty, lo, hi = hit
            f.entries = [b4.u32(MISC4 + 4 * x) & ~1 for x in subs]
            if any(not lo <= e < hi for e in f.entries):
                f.decide('entry', f'its subfamilies\' routines do not all lie in the third-kind {ty:#x}\'s code')
            f.regions, f.roots = [(lo, hi, ty)], f.entries + [lo]
            f.moved3[ty] = dict(lo=lo, hi=hi, routine=lo)
            f.notes.append(f'its routine ({f.entry:08X}) lies in the third-kind {ty:#x}\'s code ({lo:08X}-{hi:08X}), which it spawns: moved')
        elif t4_with(b4, f.entry) is not None and t4_with(b4, f.entry)[0] is not None:
            # (a routine in a fourth-kind type's code, Meteors' after its controller's spawner: the type moved, its region to
            # the routine's end the unit's block)
            ty, lo, hi = t4_with(b4, f.entry)
            f.regions, f.roots = [(lo, hi, ('4', ty))], [f.entry, lo]
            f.moved4 = {ty: dict(lo=lo, hi=hi, routine=lo)}
            f.notes.append(f'its routine ({f.entry:08X}) lies in the fourth-kind {ty:#x}\'s code ({lo:08X}-{hi:08X}), which it spawns: moved')
        elif hit is not None or C.t4_of(b4, f.entry) is not None:
            f.decide('entry', f'its routine ({f.entry:08X}) lies in an object type\'s code this tool does not move with a misc unit yet')
            return f
        else:
            # (a routine of BN4's engine by itself: copied, a block of its own)
            bn4_twins.COPIED[f.entry] = 'a misc chip\'s routine, the unit\'s entry'
            f.roots = [f.entry]
            f.notes.append(f'its routine ({f.entry:08X}), a routine of BN4\'s by itself: copied')
    # (a call into code a BN4 Navi's recipe moved goes to that copy: no type moved again for it, NV.aim_calls)
    saved_moved3 = X4.moved3_of
    X4.moved3_of = lambda b, g, t, p: None if NV.navi_block(g, t) else saved_moved3(b, g, t, p)
    # (a type an earlier unit moved, SHARED3: its spawner copied, its spawn the owner's number; one the unit does not spawn is
    # dropped after the walk)
    for ty, (spawner, why) in sorted(SHARED3.items()):
        if ty in alloc.shared3:
            bn4_twins.COPIED[spawner] = why
            lo, hi = X4.t3_of(b4, spawner)[1:]
            f.moved3[ty] = {'lo': lo, 'hi': hi, 'routine': lo, 'as': alloc.shared3[ty][0]}
            f.shared.append((3, alloc.shared3[ty][0], ty, alloc.shared3[ty][1]))
    try:
        roots = list(f.roots)
        while roots:
            X4.walk_code(b4, f, roots, set(), pairs)
            NV.aim_calls(f)
            roots = sorted((C.moved4(b4, f, pairs) | moved3(b4, f, pairs)) - set(f.walks))
            # (a call noted before its target's type was moved with the unit: its own code now, walked)
            late = [t for t in f.calls if X4.own_region(f, t)]
            for t in late:
                del f.calls[t]
            roots = sorted(set(roots) | set(late))
    finally:
        X4.moved3_of = saved_moved3
        bn4_twins.COPIED.clear()
        bn4_twins.COPIED.update(saved_copied)
    # (a shared type the unit does not spawn after all: its spawner not among its calls)
    for row in [s for s in f.shared if SHARED3[s[2]][0] not in f.walks]:
        f.shared.remove(row)
        del f.moved3[row[2]]
    site_imms(f)
    if kind == 'attack':
        megaman_ai(f)
        power_returns(f)
    X4.side_sites(f)
    X4.scan(f)
    actors(b4, f, pairs)
    X4.battle_players(f)
    saved = X4.type_number
    X4.type_number = type_number
    try:
        X4.spawns_of(f, alloc)
        C.spawns4(f, alloc)
        NV.spawns1(f, alloc)
    finally:
        X4.type_number = saved
    f.spawn_calls = {t: PAGE_SPAWNERS.get(name, name) for t, name in f.spawn_calls.items()}
    for k, a, t in f.types:   # (the first unit to move a shared type owns it)
        if k == 3 and t in SHARED3 and t not in alloc.shared3:
            alloc.shared3[t] = (a, f.name)
    f.notes += [f'its {X4.KIND_NAMES[k]}-kind {t:#x}: {who}\'s copy, the dispatch\'s {a:#x}, shared (its spawner copied)' for k, a, t, who in f.shared]
    X4.pair_calls(f, pairs)
    shallow_twins(b4, f)
    panel_fields(f)
    X4.queued_sites(f)
    saved_tables = dict(bn4_twins.TABLES)
    saved_late = X4.LATE_EFFECTS, X4.LATE_LIST5, X4.LATE_LIST4
    bn4_twins.TABLES.update(TABLES)
    X4.LATE_EFFECTS, X4.LATE_LIST5, X4.LATE_LIST4 = CHIP_EFFECTS, CHIP_LIST5, CHIP_LIST4
    try:
        X4.blocks_of(b4, f, alloc)
        X4.words_of(f, pairs)
        # (a jump table's entry no chip of the unit names, ATTACK_SUBS: BN6's null routine)
        for e, x in sorted(getattr(f, 'unreached', {}).items()):
            f.decisions = [d for d in f.decisions if not (d[0] == 'table' and d[1].startswith(f'{e | 1:08X} '))]
            f.words.append((e | 1, X4.NULLSUB6, f'its subfamily {x:#x}\'s entry, which no chip of the unit names: BN6\'s null routine'))
        sprites_of(b4, b6, f, alloc)
        table_sprites(b4, b6, f, alloc)
        table_bytes(b4, f)
        f.sounds = []
        row_tables(b4, b6, f, alloc)
        X4.effects_of(b4, b6, f, alloc)
        pattern_effects(f)
        effect_calls(b4, b6, f, alloc)
        # (an effect by a row's byte of a kinds' table: the row's byte is BN6's id, row_tables)
        by_row = [f'spawned at {s:08X} ' for t in ROW_TABLES.values() for s in t['effect_sites']]
        f.decisions = [d for d in f.decisions if not (d[0] == 'effect' and any(x in d[1] for x in by_row))]
    finally:
        bn4_twins.TABLES.clear()
        bn4_twins.TABLES.update(saved_tables)
        X4.LATE_EFFECTS, X4.LATE_LIST5, X4.LATE_LIST4 = saved_late
    # (an effect a unit before it moved is that unit's row alone: the units install in one step in UNITS' order, the effect's
    # entry written by the first, and the installer finds a row's sprite in the unit's own rows or a family's, never in
    # another unit's (xvirus.c sprite_of); its code's immediate names the id all the same)
    f.effects = [(e4, e6) for e4, e6 in f.effects if alloc.effect_owner.get(e4) == f.name]
    # (... and an effect it is the first to move whose sprite an earlier unit listed: the installer finds a row's sprite in
    # the unit's own rows, so it lists the sprite too, at the same number of BN6's list, a row of the room of its own)
    for e4, e6 in f.effects:
        row = b4.bytes(X4.BN4['effects'] + 4 * e4, 2)
        have = alloc.effect_sprites.get((row[0] // 4, row[1]))
        if have is not None and have[1] != f.name and have[0] not in f.attack_sprites:
            f.attack_sprites.append(dict(have[0]))
            f.notes.append(f'its effect {e4:#x}\'s sprite {row[0] // 4}:{row[1]:#x}, which {have[1]} lists: listed by it too, the same number '
                           f'({have[0]["to_list"]}:{have[0]["to_index"]:#x}), as its effect\'s row needs its sprite among its own')
    sounds_of(b4, b6, f)
    if f.ranges:
        f.decide('unit', 'it needs a shim the chips\' emitter writes none of yet (a NameID range)')
    f.notes += [f'{at:08X}: {why}' for at, why in f.sides]
    rows_check(b4, f)
    f.imms.sort(key=lambda x: x['at'])
    seen, out = set(), []
    for i in f.imms:
        if i['at'] not in seen:
            seen.add(i['at'])
            out.append(i)
    f.imms = out
    return f


def t4_with(b4, t):
    """the fourth-kind type whose code holds `t`, as tools/xnavirecipe_bn4_objects.py's t4_of, and for the table's last type,
    whose region no next type ends: (type, its routine, the end of the routine at `t`), `t` within 0x100 bytes of the type's"""
    hit = C.t4_of(b4, t)
    if hit is not None:
        return hit
    types = [b4.u32(C.T4_TYPES + 4 * i) & ~1 for i in range(C.NT4)]
    lo = max(a for a in types if T.is_code_ptr(a | 1, b4))
    if not lo <= t < lo + 0x100:
        return None
    w = T.walk(b4, t)
    hi = max(max(a + i.size for a, i in w.insns.items()), max((x + 4 for x in w.lits), default=0))
    tys = [k for k, a in enumerate(types) if a == lo]
    return (tys[0] if len(tys) == 1 else None), lo, hi


def moved3(b4, f, pairs):
    """its calls into a third-kind type's code that reach that type's spawn through a wrapper (tools/xrecipe_bn4.py's moved3_of
    reads the spawner alone), where no pair of the project's nor TYPE_TWINS answers for BN6's: the type moved with the unit,
    its region one of its blocks; the new roots"""
    roots = set()
    for t in sorted(f.calls):
        hit = None if t in X4.SPAWNERS4 or t in pairs.twins or t in TYPE_TWINS or t in f.aim else X4.t3_of(b4, t)
        if hit is None or hit[0] is None or hit[0] in f.moved3:
            continue
        ty, lo, hi = hit
        seen, todo, spawns = set(), [t], False
        while todo and not spawns:   # (the routine called and what it calls in the type's region: a spawn of the type)
            e = todo.pop()
            if e in seen:
                continue
            seen.add(e)
            w = T.walk(b4, e, region=(lo, hi))
            spawns = any(s in X4.SPAWNERS4 and T.back_value(w, a, 0)[0] == ty for a, s in w.bls.items())
            todo += [s for s in w.bls.values() if lo <= s < hi]
        if not spawns:
            continue
        f.moved3[ty] = dict(lo=lo, hi=hi, routine=lo)
        f.regions.append((lo, hi, ty))
        f.notes.append(f'its third-kind {ty:#x} ({lo:08X}-{hi:08X}), spawned through {t:08X}: no BN6 type proved its twin, moved')
        del f.calls[t]
        roots |= {t, lo}
    return roots


def power_returns(f):
    """each Soul's power attack copied with the unit (SOUL_POWERS): BN6's power attacks' shape (the variables at r7 set, a
    movs r0 of the family, its return), the family's number given its movs"""
    for pid, p, soul in f.powers:
        w = f.walks.get(p)
        ins = [w.insns[a] for a in sorted(w.insns)] if w else []
        last = ins[-2:] if len(ins) >= 2 else []
        if len(last) != 2 or last[0].form != 'movs' or last[0].f[:2] != (0, f.sub) or not T.is_return(last[1]) or \
                any(i.form == 'bl' for i in ins):
            f.decide('power', f'{soul}\'s power attack {pid:#x} ({p:08X}) is not the shape read: its family\'s movs, then its return')
            continue
        f.imms.append(dict(at=sorted(w.insns)[-2], form='THUMB_MOVS', rd=0, rn=0, was=f.sub, now=f.number,
                           why=f'{soul}\'s power attack {pid:#x}: the family it returns, BN4\'s {f.sub:#x}, BN6\'s grown {f.number:#x}'))


def site_imms(f):
    """the immediates of its code IMMS names, each checked against the instruction there"""
    forms = {'THUMB_ADDS': ('adds8', lambda i: (i.f[0], i.f[0], i.f[1])), 'THUMB_MOVS': ('movs', lambda i: (i.f[0], 0, i.f[1])),
             'THUMB_LSLS': ('lsls', lambda i: (i.f[0], i.f[1], i.f[2])), 'THUMB_LDRB': ('ldrb', lambda i: (i.f[0], i.f[1], i.f[2]))}
    for e, w in sorted(f.walks.items()):
        for a, i in sorted(w.insns.items()):
            m = IMMS.get(a)
            if m is None or any(x['at'] == a for x in f.imms):
                continue
            form, read = forms[m['form']]
            want = (m['rd'], m['rd'] if m['form'] == 'THUMB_ADDS' else 0 if m['form'] == 'THUMB_MOVS' else m['rn'], m['was'])
            if i.form != form or read(i) != want:
                f.decide('immediate', f'{a:08X}: IMMS names {m["form"]} r{m["rd"]}, #{m["was"]:#x} there, the code holds {i.text}')
                continue
            f.imms.append(dict(at=a, form=m['form'], rd=m['rd'], rn=m['rn'], was=m['was'], now=m['now'], why=m['why']))


def shallow_twins(b4, f):
    """a call into another object type's code paired by the search alone (TYPE_TWINS)"""
    for t, tw, how, cls, sites, audited in f.pairs:
        if how != 'search' or isinstance(tw, str) or tw is None:
            continue
        hit = X4.t3_of(b4, t) or C.t4_of(b4, t)
        if hit is not None and t not in TYPE_TWINS:
            f.decide('type', f'{t:08X} -> {tw:08X}: a routine of another object type\'s code ({hit[1]:08X}-{hit[2]:08X}) paired by its body '
                             f'alone: its type compared to its depth first (TYPE_TWINS), or moved')


def panel_fields(f):
    """a panel's side read after BN4's getPanel: BN6's offset (PANEL4TO6_FIELDS; its type's, +0, is the scan's)"""
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


def sounds_of(b4, b6, f):
    """the sounds its code plays (BN4_PLAY_SOUND, r0 the sound): each BN6's of the same number where it is the same song (how
    it sounds, xrecipe.song_sig); else BN4's song copied into its slot of BN6's grown song table (SOUND_SLOTS) and the call
    made through a shim of the unit's home that makes r0 the slot, a number past what a movs names (XvSoundCall), the
    instructions that made BN4's number left as they are"""
    ids, added = {}, set()
    for e, w in f.walks.items():
        for site, t in w.bls.items():
            if t == X4.BN4['sound'] and site in SOUND_ADDS:
                added.add(site)
            elif t == X4.BN4['sound'] and not any(site in x.get('sound', (0, ()))[1] for x in TABLES.values()):
                ids.setdefault(T.back_value(w, site, 0)[0], []).append(site)
    by_row = {s for t in ROW_TABLES.values() for s in t['sound_sites']}
    odd_sites = [s for s in ids.pop(None, []) if s not in by_row]
    if odd_sites:
        f.decide('sound', f'a sound played at {", ".join(f"{s:08X}" for s in odd_sites)} made otherwise')
    odd = sorted(v for v in ids if v is not None and X4.song_sig(b4, X4.BN4['songs'], v) != X4.song_sig(b6, X4.SONGS6, v))
    same = sorted(v for v in ids if v is not None and v not in odd)
    if same:
        f.notes.append(f'its sounds {", ".join(f"{v:#x}" for v in same)}: BN6\'s of the same numbers, the same songs')
    f.sounds = getattr(f, 'sounds', [])
    table_sounds(b4, b6, f)
    for v in odd:
        if v in FAMILY_SLOTS:
            slot = FAMILY_SLOTS[v]
            f.sounds.append((v, slot))
            for e, w in sorted(f.walks.items()):
                for site, t in sorted(w.bls.items()):
                    chain = T.back_value(w, site, 0)[1] if t == X4.BN4['sound'] and site in ids[v] else None
                    if chain is None:
                        continue
                    if len(chain) != 1 or chain[0][1] != 'movs':
                        f.decide('sound', f'its sound {v:#x} at {site:08X} named otherwise than by a movs ({chain})')
                        continue
                    f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=v, now=slot,
                                       why=f'its sound {v:#x}: BN4\'s copied into BN6\'s slot {slot:#x}, a BN4 family\'s too'))
            f.notes.append(f'its sound {v:#x}: BN4\'s song at slot {slot:#x}, the families\' slot for it (FAMILY_SLOTS)')
            continue
        slot = SOUND_SLOTS.get(v)
        if slot is None:
            f.decide('sound', f'its sound {v:#x} is no song of BN6\'s and has no slot (SOUND_SLOTS: the next of the grown table\'s)')
            continue
        if not SOUND_SLOT0 <= slot < SOUND_SLOT_END:
            f.decide('sound', f'its sound {v:#x}\'s slot {slot:#x} is none of the grown table\'s')
            continue
        f.sounds.append((v, slot))
        player = b4.u16(X4.BN4['songs'] + 8 * v + 4)
        for site in sorted(ids[v]):
            f.queue_calls.append(dict(at=site, slot=slot, why=f'its sound {v:#x}: BN4\'s song at the grown table\'s slot {slot:#x}'))
        f.notes.append(f'its sound {v:#x} ({", ".join(f"{s:08X}" for s in sorted(ids[v]))}): BN4\'s song copied into the grown song table\'s '
                       f'slot {slot:#x}, on BN4\'s player {player}')
    for site in sorted(added):
        x = SOUND_ADDS[site]
        for v in x['sounds']:
            slot = SOUND_SLOTS.get(v)
            if slot != v + x['add'] or not SOUND_SLOT0 <= slot < SOUND_SLOT_END:
                f.decide('sound', f'its sound {v:#x} at {site:08X} (SOUND_ADDS) needs its slot {v + x["add"]:#x}, SOUND_SLOTS gives {slot}')
                continue
            if X4.song_sig(b4, X4.BN4['songs'], v) == X4.song_sig(b6, X4.SONGS6, v):
                f.notes.append(f'its sound {v:#x} at {site:08X}: BN6\'s of the same number the same song, copied all the same (SOUND_ADDS)')
            f.sounds.append((v, slot))
            player = b4.u16(X4.BN4['songs'] + 8 * v + 4)
            f.notes.append(f'its sound {v:#x} ({site:08X}, one of the call\'s: SOUND_ADDS): BN4\'s song copied into the grown song table\'s '
                           f'slot {slot:#x}, on BN4\'s player {player}')
        f.queue_calls.append(dict(at=site, slot=x['add'], add=True,
                                  why=f'its sounds {", ".join(f"{v:#x}" for v in x["sounds"])}: BN4\'s numbers plus {x["add"]:#x}, '
                                      f'their slots of the grown table ({x["why"]})'))


def table_sounds(b4, b6, f):
    """the sounds its objects play by a row of a kinds' table (TABLES' sound: the row's u16, loaded and handed to BN4's
    PlaySoundEffect at the sites named, a number of any size): BN6's of the same number where it is the same song, else BN4's
    song at its slot of the grown table (SOUND_SLOTS), the row's two bytes in the table's copy set to it"""
    named = {v for w in f.walks.values() for v in w.lits.values() if v in TABLES and TABLES[v].get('sound')}
    sites = {s for w in f.walks.values() for s, t in w.bls.items() if t == X4.BN4['sound']}
    for v in sorted(named):
        t = TABLES[v]
        if not t['sound'][1] <= sites:
            f.decide('sound', f'its table {v:08X}\'s sound is played at a site its code does not hold ({t["sound"][1] - sites})')
        for k in t['kinds']:
            at = v + t['entry'] * k + t['sound'][0]
            snd = b4.u16(at)
            if X4.song_sig(b4, X4.BN4['songs'], snd) == X4.song_sig(b6, X4.SONGS6, snd):
                f.notes.append(f'its table {v:08X}\'s kind {k}: its sound {snd:#x}, BN6\'s of the same number, the same song')
                continue
            slot = SOUND_SLOTS.get(snd)
            if slot is None or not SOUND_SLOT0 <= slot < SOUND_SLOT_END:
                f.decide('sound', f'{at:08X}: its table\'s kind {k} plays {snd:#x}, no song of BN6\'s, which has no slot (SOUND_SLOTS)')
                continue
            if (snd, slot) not in f.sounds:
                f.sounds.append((snd, slot))
            f.bytes += [(at + j, snd >> 8 * j & 0xFF, slot >> 8 * j & 0xFF,
                         f'its table\'s kind {k}: its sound {snd:#x} in BN4, the grown table\'s slot {slot:#x} in BN6 (byte {j})')
                        for j in range(2) if snd >> 8 * j & 0xFF != slot >> 8 * j & 0xFF]
            f.notes.append(f'its table {v:08X}\'s kind {k}: its sound {snd:#x}, BN4\'s song copied into the grown song table\'s slot {slot:#x}')


def registry_rows():
    """BN4's rows of the chip registry: BN4's id -> (BN6's id, how, sub)"""
    rows = {}
    with open(REGISTRY, encoding='utf-8') as fh:
        for m in re.finditer(r'\{\s*(\d+),\s*(\d+),\s*XREG_(\w+),\s*(\d+),\s*(0x[0-9A-Fa-f]+|\d+),', fh.read()):
            rows[int(m.group(1))] = (int(m.group(2)), m.group(3), int(m.group(5), 0))
    return rows


def rows_check(b4, f):
    """its chips' rows: each XREG_MOVED with the unit's number its subfamily (tools/xreg_bn4.py MOVED); a dark chip's
    (its record's alignment dark, or a DarkChip by its effect flags) among the registry's dark rows, which the price's
    hook reads (docs/game/meta/darkchips.md: 20 max HP a battle one is used in)"""
    rows = registry_rows()
    with open(REGISTRY, encoding='utf-8') as fh:
        m = re.search(r'xreg4_dark_ids\[\] = \{([^}]*)\}', fh.read())
    dark = {int(x) for x in re.findall(r'\d+', m.group(1))} if m else set()
    f.rows = []
    for i, r in f.chips:
        if (r[0x15] == 2 or r[9] & 0x20) and i in rows:
            if rows[i][0] in dark:
                f.notes.append(f'its chip {i} is a dark chip of BN4\'s (row {rows[i][0]}): a battle it is used in costs 20 max HP')
            elif rows[i][1] != 'PENDING':   # (a Navi's DS whose row waits: no price asked yet, the unit serves his V1)
                f.decide('row', f'its chip {i} is a dark chip and its row {rows[i][0]} is not among xreg4_dark_ids: no price (tools/xreg_bn4.py)')
        if i not in rows:
            f.notes.append(f'its chip {i} has no row of the registry (the run\'s by name, or none)')
            continue
        bn6, how, sub = rows[i]
        if i in ROWS_WAIT:
            if how != 'PENDING':
                f.decide('row', f'its chip {i} waits (ROWS_WAIT) but its row (id {bn6}) is XREG_{how}')
            f.notes.append(f'its chip {i} (id {bn6}) sits out: {ROWS_WAIT[i]}')
            continue
        f.rows.append((i, bn6))
        want = NUMBER0 + r[0xC]   # (its own subfamily's routine: the unit's, or one of the rows that share its code)
        if f.kind == 'navi' and how == 'PENDING':
            # (a Navi's SP waits for BN4's computed powers, his DS for the dark chips' cost: the unit serves the V1 meanwhile)
            f.notes.append(f'its chip {i} (id {bn6}) waits: its row is XREG_PENDING')
            continue
        if f.kind == 'attack':
            # (an attack family's chips: the unit's number, its family; a Program Advance of it is formed, never dealt, its
            # row XREG_PA until BN4's recipes are listed; a chip of it another session moves waits, ROWS_WAIT)
            want = f.number
            if (how == 'PA' and sub == 0xFF) or i in ROWS_WAIT:
                f.notes.append(f'its chip {i} (id {bn6}) sits out: ' + ('a Program Advance, unformed until BN4\'s recipes are listed' if how == 'PA' else ROWS_WAIT[i]))
                f.rows.pop()
                continue
        if how == 'PA' and sub == want:
            # (a Program Advance whose row names the unit, tools/xreg_bn4.py PAS: formed by BN4's recipes in BN6's grown list)
            f.notes.append(f'its chip {i} (id {bn6}) is a Program Advance, formed by BN4\'s recipes (src/core/xchips_pas_bn4.c)')
            continue
        if how != 'MOVED' or sub != want:
            f.decide('row', f'its chip {i}\'s row (id {bn6}) is XREG_{how} with subfamily {sub:#x}, not XREG_MOVED with {want:#x}: '
                            f'tools/xreg_bn4.py MOVED')


# A sprite load whose number is made on more than one path (a movs on each, the walk's back_value seeing the last): the other
# paths' movs, each copied and given the copy as the first is. BN4's load -> the other movs
SPRITE_PATHS = {0x080DAFF4: (0x080DAFE6,)}   # (the ring's init: 3:0x37, its kinds but 3's; 3:0x3A kind 3's, 0x080DAFEE)


def sprites_of(b4, b6, f, alloc):
    """the sprites its moved objects load (BN4's sprite load, r1 the list's offset, r2 the number, each a movs): BN4's copied
    at a number of the chips' in that list (the movs given it), one copy a sprite whichever unit draws it"""
    for e, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != X4.BN4['sprite_load'] or any(site in x['loads'] for x in TABLES.values()):
                continue
            lst, _ = T.back_value(w, site, 1)
            num, chain = T.back_value(w, site, 2)
            if lst is None or lst & 3 or num is None or len(chain) != 1 or chain[0][1] != 'movs':
                f.decide('sprite', f'{site:08X}: a sprite loaded by numbers made otherwise ({lst}, {num})')
                continue
            if f.kind == 'navi' and lst // 4 == 2 and NV.battle_sprite(f, site, chain, num):
                continue
            # (BN4's sprite copied, as the families' are, whether or not BN6 has one of its bytes: the differential names a
            # sprite by the recipe's rows, and an object in play in a fight's frame is compared by its sprite's frames)
            c = list3_copy(b4, f, alloc, num, lst // 4)
            tw, how = (c['to_index'], 'copied') if c else (None, '')
            if tw is None:
                f.decide('sprite', f'{site:08X}: its sprite {lst // 4}:{num:#x} is no sprite of BN6\'s list {lst // 4}, and no number of the chips\' is left for a copy')
                continue
            f.notes.append(f'its sprite {lst // 4}:{num:#x} ({site:08X}): BN6\'s {lst // 4}:{tw:#x}, {how}')
            if tw != num:
                f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=2, rn=0, was=num, now=tw,
                                   why=f'its object\'s sprite: list {lst // 4} {num:#x} in BN4, {tw:#x} in BN6 ({how})'))
            for at in SPRITE_PATHS.get(site, ()):
                i = w.insns.get(at)
                if i is None or i.form != 'movs' or i.f[0] != 2:
                    f.decide('sprite', f'{at:08X}: SPRITE_PATHS names a movs r2 of {site:08X}\'s load there, the code holds {i.text if i else "none"}')
                    continue
                c = list3_copy(b4, f, alloc, i.f[1], lst // 4)
                if c is None:
                    f.decide('sprite', f'{at:08X}: its sprite {lst // 4}:{i.f[1]:#x}: no number of the chips\' is left for a copy')
                    continue
                f.notes.append(f'its sprite {lst // 4}:{i.f[1]:#x} ({site:08X}, by {at:08X}): BN6\'s {lst // 4}:{c["to_index"]:#x}, copied')
                f.imms.append(dict(at=at, form='THUMB_MOVS', rd=2, rn=0, was=i.f[1], now=c['to_index'],
                                   why=f'its object\'s sprite on its other path: list {lst // 4} {i.f[1]:#x} in BN4, {c["to_index"]:#x} in BN6 (copied)'))


def list_number(alloc, lst):
    """the chips' next number of a list (3 up from CHIP_LIST3, 1, 4 and 5 down their ranges), None spent"""
    if lst == 3:
        n, ok = alloc.list3, alloc.list3 < CHIP_LIST3[1]
        alloc.list3 += ok
    elif lst == 5:
        n, ok = alloc.late_list5, alloc.late_list5 >= CHIP_LIST5[1]
        alloc.late_list5 -= ok
    elif lst == 4:
        n, ok = alloc.late_list4, alloc.late_list4 >= CHIP_LIST4[1]
        alloc.late_list4 -= ok
    elif lst == 1:
        n, ok = alloc.late_list1, alloc.late_list1 >= CHIP_LIST1[1]
        alloc.late_list1 -= ok
    else:
        return None
    return n if ok else None


def list3_copy(b4, f, alloc, num, lst=3):
    """BN4's sprite `num` of list 3 (or `lst`: 1, 4, 5) copied at a number of the chips' in that list, once for every unit: the
    copy, None unread or spent"""
    if (lst, num) in alloc.list3_sprites:
        s, who = alloc.list3_sprites[lst, num]
        if who != f.name:
            f.notes.append(f'its sprite {lst}:{num:#x}: list {lst} {s["to_index"]:#x}, as {who} copied it')
        return s
    p, raw = X4.sprite(b4, X4.BN4['sprites'], lst, num)
    to = list_number(alloc, lst) if raw is not None else None
    if to is None:
        return None
    s = dict(list=lst, index=num, to_list=lst, to_index=to, at=alloc.art, room=X4.rounded(len(raw), ROOM_SPRITE), lz=bool(p & 0x80000000))
    alloc.art += s['room']
    alloc.list3_sprites[lst, num] = (s, f.name)
    f.attack_sprites.append(s)
    if alloc.art > alloc.art_end:
        f.decide('room', f'its sprite 3:{num:#x} ends at {alloc.art:08X}, past the chips\' art')
    return s


def table_sprites(b4, b6, f, alloc):
    """the sprites of the kinds' tables its objects read (TABLES): each spawned kind's BN6's of the same bytes, else copied
    (list 3 alone yet), the row's byte in the table's copy set to BN6's number"""
    named = {v for w in f.walks.values() for v in w.lits.values() if v in TABLES}
    for v in sorted(named):
        t = TABLES[v]
        for k in t['kinds']:
            at = v + t['entry'] * k + t['sprite'][1]
            num = b4.u8(at)
            # (the list a row's own where the table keeps it, its offset: the arm's)
            lst = t['list'] if t['sprite'][0] is None else b4.u8(v + t['entry'] * k + t['sprite'][0]) // 4
            c = list3_copy(b4, f, alloc, num, lst)
            tw, how = (c['to_index'], 'copied') if c else (None, '')
            if tw is None:
                f.decide('sprite', f'{at:08X}: its table\'s kind {k} draws {lst}:{num:#x}, which BN6 has none of and the generator copies none of')
                continue
            f.notes.append(f'its table {v:08X}\'s kind {k}: sprite {lst}:{num:#x} in BN4, {tw:#x} in BN6 ({how})')
            if tw != num:
                f.bytes.append((at, num, tw, f'its table\'s kind {k}: list {lst} {num:#x} in BN4, {tw:#x} in BN6 ({how})'))


EFFECT_PATTERN4 = 0x0801457E   # (bn4_navis.h BN4_EFFECT_PATTERN: an effect over a pattern of panels, r2 its id; BN6_EFFECT_PATTERN)


def pattern_effects(f):
    """the effects its code spawns over a pattern of panels (BN4_EFFECT_PATTERN, r2 the id, a movs: PropBom's blast): BN6's
    twin spawns the id it is handed, so the movs is given the id the unit's own spawn of that effect was given (effects_of's
    rows); an id the unit spawns nowhere itself is a decision, one read from a kinds' row is that table's (row_tables)"""
    given = {x['was']: x['now'] for x in f.imms if x['why'].startswith('its effect: ')}
    seen = {int(m.group(1), 16) for n in f.notes for m in [re.match(r'its effect (0x[0-9a-f]+): ', n)] if m}
    for w in f.walks.values():
        for site, t in sorted(w.bls.items()):
            if t != EFFECT_PATTERN4:
                continue
            v, chain = T.back_value(w, site, 2)
            if v is None or len(chain) != 1 or chain[0][1] != 'movs':
                continue   # (an id by a row's byte of a kinds' table: BN6's already, row_tables; the spread shot's burst)
            if v not in seen:
                f.decide('effect', f'an effect over a pattern at {site:08X}: its id {v:#x} is none the unit spawns itself')
            elif v in given:
                f.imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=2, rn=0, was=v, now=given[v],
                                   why=f'its effect over a pattern of panels: {v:#x} in BN4, {given[v]:#x} in BN6 (as its own spawn of it)'))


EFFECT_PAIRS = 4   # (src/emu/xvirus_recipe.h XV_EFFECT_PAIRS: the pairs an effect call's shim holds)


def effect_calls(b4, b6, f, alloc):
    """the effects its code spawns by a byte of the object's own: BN4_SPAWN_EFFECT after ldrb r4,[r5,#4 to 7], a byte of the
    word the object's spawn kept of its caller's r4. No immediate names the id, so the ids that reach the load are read
    where they are made: that byte of r4 at each call of the type's spawner in the unit's own code, each an immediate (one
    that is not, or a type no call of the unit spawns, stays effects_of's decision, so that an id nobody read cannot
    pass to BN6's own effect of that number). Where BN6 has another id for one (its twin of the same look, else the copy
    a unit moved), the call goes through a shim of the unit's home that gives r4 BN6's id (XvEffectCall: a row)"""
    f.effect_calls = []
    types = [(4, ty, x) for ty, x in f.moved4.items()] + [(3, ty, x) for ty, x in f.moved3.items() if 'lo' in x]
    for w in f.walks.values():
        for site, t in sorted(w.bls.items()):
            v, chain = T.back_value(w, site, 4) if t == X4.BN4['effect'] else (0, [])
            if v is not None or len(chain) != 1 or chain[0][1] != 'ldrb':
                continue
            load = b4.u16(chain[0][0])   # (ldrb rd,[rn,#imm]: 0x7800 | imm << 6 | rn << 3 | rd)
            at = load >> 6 & 0x1F
            own = [(k, ty) for k, ty, x in types if x['lo'] <= site < x['hi']]
            if load & 0xF800 != 0x7800 or load & 7 != 4 or load >> 3 & 7 != 5 or not 4 <= at <= 7 or len(own) != 1:
                continue
            kind, ty = own[0]
            # (the type's spawners: the unit's routines that hand BN4's spawn of the kind this type's number; their callers)
            spawners = {e for e, x in f.walks.items() for s, to in x.bls.items()
                        if X4.SPAWNERS4.get(to, (0,))[0] == kind and T.back_value(x, s, 0)[0] == ty}
            made = [T.back_value(x, s, 4)[0] for x in f.walks.values() for s, to in sorted(x.bls.items()) if to in spawners]
            if not made or any(m is None for m in made):
                continue
            ids, pairs = sorted({m >> 8 * (at - 4) & 0xFF for m in made}), []
            for e4 in ids:
                e6 = X4.effect_twin(b4, b6, e4)
                if e6 is None:
                    e6 = X4.move_effect(b4, f, alloc, e4)
                if e6 is not None and e6 != e4:
                    pairs.append((e4, e6))
            if len(pairs) > EFFECT_PAIRS or any(b in ids for _, b in pairs):
                continue
            f.decisions = [d for d in f.decisions if not (d[0] == 'effect' and f'spawned at {site:08X} ' in d[1])]
            why = (f'its {X4.KIND_NAMES[kind]}-kind {ty:#x}\'s effect, its +{at:#x}: ' +
                   ', '.join(f'{a:#x}' for a in ids) + f' from its spawns ({len(made)} in the unit\'s code)')
            if pairs:
                f.effect_calls.append(dict(at=site, pairs=pairs, why=why + ': ' + ', '.join(f'{a:#x} BN6\'s {b:#x}' for a, b in pairs)))
            f.notes.append(why + (': through a shim that gives r4 ' + ', '.join(f'{b:#x} for {a:#x}' for a, b in pairs) if pairs
                                  else ': BN6\'s ids the same'))


def row_tables(b4, b6, f, alloc):
    """the effects and sounds its moved objects' kinds' tables name by a byte (ROW_TABLES), for the kinds its chips play: an
    effect BN6's of the same look or moved (X4.move_effect), a sound BN6's own of its number where it is the same song, else
    a BN4 family's slot for it (FAMILY_SLOTS, a byte), its song copied with the unit; the row's bytes in the copy set"""
    named = {v for w in f.walks.values() for v in w.lits.values() if v in ROW_TABLES}
    made = {}   # (BN4's effect: BN6's id and how, once a unit)
    for v in sorted(named):
        t = ROW_TABLES[v]
        for k in t['kinds']:
            row = v + t['entry'] * k
            if t['effect'] is None:
                row_sound(b4, b6, f, v, t, k)
                continue
            e4 = b4.u8(row + t['effect'])
            if e4 not in made:
                e6, how = X4.effect_twin(b4, b6, e4), 'the same look'
                if e6 is None:
                    e6, how = X4.move_effect(b4, f, alloc, e4), 'moved'
                made[e4] = (e6, how)
                if e6 is not None:
                    f.notes.append(f'its effect {e4:#x} (its table {v:08X}\'s): BN6\'s {e6:#x}, ' +
                                   ('BN6\'s own of the same look' if how != 'moved' else 'moved (no effect of BN6\'s draws its sprite\'s bytes)'))
            e6, how = made[e4]
            if e6 is None:
                continue
            if e6 != e4:
                f.bytes.append((row + t['effect'], e4, e6, f'its table\'s kind {k:#x}\'s effect: {e4:#x} in BN4, {e6:#x} in BN6 ({how})'))
            s4 = b4.u8(row + t['sound'])
            if not s4 or X4.song_sig(b4, X4.BN4['songs'], s4) == X4.song_sig(b6, X4.SONGS6, s4):
                continue
            slot = FAMILY_SLOTS.get(s4)
            if slot is None or slot > 0xFF:
                f.decide('sound', f'{row + t["sound"]:08X}: its table\'s kind {k:#x}\'s sound {s4:#x} is no song of BN6\'s and has no slot a byte names (FAMILY_SLOTS)')
                continue
            if (s4, slot) not in f.sounds:
                f.sounds.append((s4, slot))
                f.notes.append(f'its sound {s4:#x} (its table {v:08X}\'s): BN4\'s song at slot {slot:#x}, the families\' slot for it (FAMILY_SLOTS)')
            f.bytes.append((row + t['sound'], s4, slot, f'its table\'s kind {k:#x}\'s sound: BN4\'s {s4:#x} at BN6\'s slot {slot:#x}'))


def row_sound(b4, b6, f, v, t, k):
    """a kinds' table's row that names a sound alone (ROW_TABLES' `effect` None): BN6's own where the same song, else its slot of
    the grown song table (SOUND_SLOTS) played by the sites through the unit's shim (`queue`), its song copied with the unit"""
    s4 = b4.u8(v + t['entry'] * k + t['sound'])
    if not s4 or X4.song_sig(b4, X4.BN4['songs'], s4) == X4.song_sig(b6, X4.SONGS6, s4):
        return
    slot = SOUND_SLOTS.get(s4)
    if slot is None or not t.get('queue') or any(b4.u8(v + t['entry'] * x + t['sound']) != s4 for x in t['kinds']):
        f.decide('sound', f'its table {v:08X}\'s kind {k:#x}\'s sound {s4:#x}: no slot of the grown table, or its kinds\' sounds differ')
        return
    if (s4, slot) not in f.sounds:
        f.sounds.append((s4, slot))
    for site in sorted(t['sound_sites']):
        if not any(x['at'] == site for x in f.queue_calls):
            f.queue_calls.append(dict(at=site, slot=slot, why=f'its table {v:08X}\'s kind {k:#x}\'s sound {s4:#x}: BN4\'s song at the grown '
                                                                f'table\'s slot {slot:#x}'))
    f.notes.append(f'its sound {s4:#x} (its table {v:08X}\'s kind {k:#x}): BN4\'s song at the grown table\'s slot {slot:#x}, played '
                   f'through the unit\'s shim')


# MegaMan's AI data as an attack state of his names it: the attack's variables alone, BN4's +0x70 (0x50 bytes), BN6's +0xA0
# (tools/xnavirecipe_bn4.py AI_BLOCKS: a Soul Navi's states name them so too); another field of it a decision until both
# games' use of it is read (MEGAMAN_AI: BN4's offset -> BN6's)
MEGAMAN_AI_VARS = (0x70, 0x50, 0xA0)
MEGAMAN_AI = {}


def megaman_ai(f):
    """an attack state's pointers into MegaMan's AI data (his object's +0x58): the attack's variables' made by an immediate
    given BN6's offset; a load or store of another field BN6's place for it (MEGAMAN_AI), else a decision"""
    lo, n, to = MEGAMAN_AI_VARS
    for e, w in sorted(f.walks.items()):
        reg = X4.own_region(f, e)
        if not reg or reg[2] != 'state':
            continue
        for a, form, rd, rn, imm, off in N.ai_tracker(w, {} if e == f.entry else {7: lo}):
            if lo <= off < lo + n:
                if form.startswith('MEM:') and imm != off:
                    continue   # (through a pointer to the variables: its offset in them, the same in both games)
                now = imm + to - lo
                if form.startswith('MEM:') or form == 'THUMB_ADDS3' or now > 0xFF:
                    f.decide('AI data', f'{a:08X}: the attack\'s variables (+{off:#x}) named by a form the installer does not rewrite')
                    continue
                f.imms.append(dict(at=a, form=form, rd=rd, rn=rn, was=imm, now=now,
                                   why=f'MegaMan\'s AI data\'s +{off:#x} in BN4, +{off + to - lo:#x} in BN6: the attack\'s variables'))
            elif form.startswith('MEM:') and off in MEGAMAN_AI and imm == off:
                if MEGAMAN_AI[off] != off:
                    f.imms.append(dict(at=a, form=X4.FORMS[form[4:]], rd=rd, rn=rn, was=imm, now=MEGAMAN_AI[off],
                                       why=f'MegaMan\'s AI data\'s +{off:#x} in BN4, +{MEGAMAN_AI[off]:#x} in BN6'))
            else:
                f.decide('AI data', f'{a:08X}: MegaMan\'s AI data\'s +{off:#x}, which MEGAMAN_AI gives no BN6 place')


def table_bytes(b4, f):
    """the bytes of a kinds' table read in BN6's terms (TABLES' bytes: a row's offset -> BN4's value -> BN6's, each pair read in
    both ROMs), set in the table's copy; a value the map has none of left as BN4's and noted, its row a kind no moved chip
    names (the map's why says which)"""
    named = {v for w in f.walks.values() for v in w.lits.values() if v in TABLES}
    for v in sorted(named):
        t = TABLES[v]
        for off, m in sorted(t.get('bytes', {}).items()):
            for row in range(t['size'] // t['entry']):
                at = v + t['entry'] * row + off
                was = b4.u8(at)
                if was not in m:
                    f.notes.append(f'its table {v:08X}\'s row {row:#x}: +{off:#x} {was:#x}, none of BN6\'s, left as BN4\'s ({t["bytes_why"]})')
                elif m[was] != was:
                    f.bytes.append((at, was, m[was], f'its table\'s row {row:#x}: +{off:#x} {was:#x} in BN4, {m[was]:#x} in BN6'))


def report_unit(f):
    L = [f'## {f.name} ({f.kind} {f.sub:#x}, BN6\'s {f.kind} {"family" if f.kind == "attack" else "routine"} {f.number:#x})', '']
    L.append('chips: ' + ', '.join(str(i) for i, _ in f.chips))
    if getattr(f, 'blocks', None):
        L.append(f'blocks: ' + ', '.join(f'{lo:08X}-{hi:08X}' for lo, hi, _, _ in f.blocks) + f' ({sum(hi - lo for lo, hi, _, _ in f.blocks)} bytes)')
        L.append(f'walked: {len(f.walks)} routines; types moved: ' + ', '.join(f'{k}:{t:#x} as {a:#x}' for k, a, t in f.types))
        L += ['', 'calls:'] + [f'- {t:08X} -> {tw if isinstance(tw, str) else f"{tw:08X}" if tw else None} ({how}, {cls}{", audited" if aud else ""}; {len(sites)} sites)'
                               for t, tw, how, cls, sites, aud in f.pairs]
        L += ['', 'words:'] + [f'- {a:08X} -> {b:08X} ({why})' for a, b, why in f.words]
        L += ['', 'numbers:'] + [f'- {lit:08X}: {was:08X} -> {now:08X} ({why})' for lit, was, now, why in f.numbers]
        L += ['', 'immediates:'] + [f'- {i["at"]:08X} {i["form"]} {i["was"]:#x} -> {i["now"]:#x}: {i["why"]}' for i in f.imms]
    L += ['', 'notes:'] + [f'- {x}' for x in f.notes]
    L += ['', f'decisions open: {len(f.decisions)}'] + [f'- [{k}] {d}' for k, d in f.decisions]
    return '\n'.join(L) + '\n'


HEADER = '''/* xchips_bn4.c: BN4's chips' code as units for BN6's battle engine, written by tools/xchiprecipe_bn4.py from the two
 * ROMs (python3 tools/xchiprecipe_bn4.py; --check says whether this is what it writes). Do not edit: change the generator
 * (or tools/bn4_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/multirom/bn4-chips.md):
 * addresses in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA), counts and the CRC32s of the ranges the install copies from
 * the player's ROM; nothing of either game's. The units are the generator's, whole (no decision open), in install order;
 * src/emu/xchips_code.c installs them, and the registry's XREG_MOVED rows of BN4's (src/core/xchips_registry_bn4.c) name
 * the chips that run them. */
#include "xchips_bn4.h"

#include "bn4.h"
#include "bn6.h"
#include "rom.h"
#include "xchips_bn4_art.h"
#include "xnavi_recipe.h"
'''
ROWS = [0]        # (the next row of the art's room, counted over the units as their rows are written: c_file)
ROWS_MAX = 512    # (XC4_ROWS: the rows a slot's start can tell apart)
KIND_C = {'support': 'XCHIP_SUPPORT', 'misc': 'XCHIP_MISC', 'attack': 'XCHIP_ATTACK', 'navi': 'XCHIP_NAVI'}


def place(at, base, name):
    return f'{name} + 0x{at - base:X}'


def c_unit(f):
    n = re.sub(r'[^a-z0-9]', '', f.name.lower())
    L = [f'/* ---- {f.name}: BN4\'s {f.kind} {f.sub:#x} ({f.entry | 1:08X}) as BN6\'s {f.number:#x}; its chips '
         f'{", ".join(f"{i} ({b})" for i, b in f.rows)} ---- */']
    L += [f'/* {x} */' for x in f.notes]
    L += ['', f'static const XvBlock {n}_blocks[] = {{']
    L += [f'\t{{ 0x{lo:08X}, 0x{hi:08X}, {place(at, X4C_CODE, "XC4_CODE")}, 0x{crc:08X}u }},' for lo, hi, at, crc in f.blocks]
    L += ['};', f'static const XvData {n}_data[] = {{']
    L += [f'\t{{ 0x{lo:08X}, 0x{hi:08X}, XV_{k} }},' for lo, hi, k in f.data]
    L += ['};', f'static const XvPair {n}_calls[] = {{']
    L += [X4.pair_text(t, tw, how, cls, audited) for t, tw, how, cls, sites, audited in f.pairs]
    L += ['};', f'static const XvPair {n}_words[] = {{']
    L += [f'\t{{ 0x{a:08X}, 0x{b:08X} }},   /* {why} */' for a, b, why in f.words]
    L += ['};', f'static const XvImm {n}_imms[] = {{']
    L += [f'\t{{ 0x{i["at"]:08X}, {i["form"]}, {i["rd"]}, {i["rn"]}, 0x{i["was"]:X}, 0x{i["now"]:X} }},   /* {i["why"]} */' for i in f.imms]
    L.append('};')
    if f.numbers:
        L.append(f'static const XvNumber {n}_numbers[] = {{')
        L += [f'\t{{ 0x{lit:08X}, 0x{was:08X}, 0x{now:08X} }},   /* {why} */' for lit, was, now, why in f.numbers]
        L.append('};')
    if f.states:
        # (its moved actor's state table, BN6's layout: { address, its own, no clone, not entered })
        L.append(f'static const XvRef {n}_states[] = {{')
        L += [f'\t{{ 0x{x:08X}, {"true" if who == "OWN" else "false"}, 0, false }},   /* {k}: {"its own" if who == "OWN" else "BN6\'s"} */'
              for k, (who, x) in enumerate(f.states)]
        L.append('};')
    L.append(f'static const XvType {n}_types[] = {{')
    # (the third kind's by BN4's own number on the chips' page, XV_TYPE_MOVED4; the fourth kind's at a number of the tables)
    for k, a, t in f.types:
        where = "the chips' page" if k == 3 else f"the dispatch's {a:#x}"
        L.append(f'\t{{ {k}, 0x{a:02X}, XV_TYPE_MOVED{4 if k == 3 else ""}, 0x{t:02X} }},   /* BN4\'s {X4.KIND_NAMES[k]}-kind {t:#x}, moved, {where} */')
    for k, a, t, who in f.shared:
        where = '' if k == 3 else f": the dispatch's {a:#x}"
        L.append(f'\t{{ {k}, 0x{a:02X}, XV_TYPE_SHARED{4 if k == 3 else ""}, 0x{t:02X} }},   /* BN4\'s {X4.KIND_NAMES[k]}-kind {t:#x}, {who}\'s copy{where} */')
    L.append('};')
    if f.attack_sprites:
        L.append(f'static const XvSprite {n}_sprites[] = {{')
        # (its art's rows: none installed, each a row of the room the sprites are put into as an object loads one, its number
        # over every unit's rows in UNITS' order: src/emu/xchips_bn4_art.h)
        for x in f.attack_sprites:
            L.append(f'\t{{ {x["list"]}, 0x{x["index"]:02X}, {x["to_list"]}, 0x{x["to_index"]:02X}, XC4_ROOM_ROW({ROWS[0]}), 0x{x["room"]:X} }},'
                     f'   /* BN4\'s {x["list"]}:{x["index"]:#x}, the room\'s row {ROWS[0]}{" (LZ77 in BN4)" if x["lz"] else ""} */')
            ROWS[0] += 1
        L.append('};')
    if f.effects:
        L.append(f'static const XvId {n}_effects[] = {{ ' + ', '.join(f'{{ 0x{a:02X}, 0x{b:02X} }}' for a, b in f.effects) + ' };')
    if f.bytes:
        L.append(f'static const XvByte {n}_bytes[] = {{')
        L += [f'\t{{ 0x{at:08X}, 0x{was:02X}, 0x{now:02X} }},   /* {why} */' for at, was, now, why in f.bytes]
        L.append('};')
    if f.sides:
        # (its calls to BN4's direction rule, each through a shim that numbers left and right by the object's side: XvSide)
        L.append(f'static const uint32_t {n}_sides[] = {{')
        L += [f'\t0x{at:08X},   /* {why} */' for at, why in f.sides]
        L.append('};')
    if f.queue_calls:
        L.append(f'static const XvSoundCall {n}_sound_calls[] = {{')
        L += [f'\t{{ 0x{x["at"]:08X}, 0x{x["slot"]:X}{", true" if x.get("add") else ""} }},   /* {x["why"]} */' for x in f.queue_calls]
        L.append('};')
    if getattr(f, 'effect_calls', None):
        L.append(f'static const XvEffectCall {n}_effect_calls[] = {{')
        L += [f'\t{{ 0x{x["at"]:08X}, {len(x["pairs"])}, {{ ' + ', '.join(f'{{ 0x{a:02X}, 0x{b:02X} }}' for a, b in x['pairs']) +
              f' }} }},   /* {x["why"]} */' for x in f.effect_calls]
        L.append('};')
    if f.sounds:
        L.append(f'static const XcSong {n}_songs[] = {{ ' + ', '.join(f'{{ 0x{v:X}, 0x{slot:02X} }}' for v, slot in f.sounds) + ' };')
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    fields = [f'.{a} = {n}_{a}, .n{a} = {count(a)}' for a, have in (('blocks', True), ('data', True), ('calls', True), ('words', True),
                                                                   ('imms', True), ('numbers', f.numbers), ('states', f.states), ('types', True), ('sides', f.sides),
                                                                   ('sound_calls', f.queue_calls), ('effect_calls', getattr(f, 'effect_calls', None)),
                                                                   ('sprites', f.attack_sprites),
                                                                   ('effects', f.effects), ('bytes', f.bytes)) if have]
    L += ['', f'#define {n.upper()}_CODE {{ .name = "{f.name} (BN4\'s)", .game = XROM_BN4_RED_SUN_US, \\',
          f'\t.home = XC4_SHIMS_HOME({place(f.home, X4C_HOMES, "XC4_HOMES")}), \\']
    L += [f'\t{x}, \\' for x in fields[:-1]] + [f'\t{fields[-1]} }}']
    songs = f'{n}_songs, {count("songs")}' if f.sounds else 'NULL, 0'
    # (a Navi chip's object's sound as it steps MegaMan in, tools/xchiprecipe_bn4_navi.py STEP_IN: its slot, XChipUnit.step_in)
    step_in = SOUND_SLOTS.get(getattr(f, 'step_in', 0), 0)
    if getattr(f, 'step_in', 0) and not step_in:
        sys.exit(f'xchiprecipe_bn4: {f.name}\'s step-in sound {f.step_in:#x} has no slot (SOUND_SLOTS)')
    rows = [f'\t{{ {KIND_C[f.kind]}, 0x{f.sub:02X}, 0x{f.number:02X}, 0x{f.entry | 1:08X}, {place(f.home, X4C_HOMES, "XC4_HOMES")}, '
            f'{n.upper()}_CODE, {songs}, {getattr(f, "seat", -1) + 1}, 0' + (f', 0x{f.wait:02X}' if f.wait or step_in else '') +
            (f', 0x{step_in:X}' if step_in else '') + ' },']
    # (its other subfamilies: their routines in its block, each a row that shares its code, a home of its own unused)
    for k, x in enumerate(f.subs[1:], 1):
        rows.append(f'\t{{ {KIND_C[f.kind]}, 0x{x:02X}, 0x{NUMBER0 + x:02X}, 0x{f.entries[k] | 1:08X}, '
                    f'{place(f.home + X4C_HOME_SIZE * k, X4C_HOMES, "XC4_HOMES")}, {{ .name = "{f.name} (BN4\'s), its subfamily {x:#x}", '
                    f'.game = XROM_BN4_RED_SUN_US }}, NULL, 0, 0, SHARE }},')
    return '\n'.join(L), rows


def c_file(units):
    parts, rows = [HEADER], []
    ROWS[0] = 0
    for f in units:
        t, own = c_unit(f)
        parts.append(t)
        rows += [x.replace('SHARE', str(len(rows) + 1)) for x in own]   # (a sharing row names its unit's row, 1-based)
    if ROWS[0] > ROWS_MAX:
        sys.exit(f'xchiprecipe_bn4: {ROWS[0]} rows of art, past the room\'s {ROWS_MAX}')
    parts.append('const XChipUnit xchips_bn4[] = {\n' + '\n'.join(rows) + '\n};\n'
                 'const int xchips_bn4_count = (int)(sizeof xchips_bn4 / sizeof *xchips_bn4);\n')
    return '\n'.join(parts)


def actors(b4, f, pairs):
    """its moved objects that are actors (ACTOR_RUN4): each one's state table in BN6's layout (f.states: the engine's
    generic 0-7 BN6's, a slot the object fills with a routine of the engine's its twin, its own from 8), the pool word that
    names BN4's table aimed at the unit's (a number row), and its own code's CurAction from 6 shifted (X4.curaction, which
    X4.scan runs on a family's own region alone)"""
    f.states = []
    for e, w in sorted(f.walks.items()):
        for site in sorted(s for s, t in w.bls.items() if t == ACTOR_RUN4):
            reg = X4.own_region(f, e)
            tab, chain = T.back_value(w, site, 0)
            if f.states or reg is None or reg[2] is None or tab is None or len(chain) != 1 or chain[0][1] != 'ldr_lit' or tab not in f.tables:
                f.decide('actor', f'{site:08X}: the actors\' runner called with a table this tool does not rebuild '
                                  f'({"a second actor" if f.states else "no table of its region read from its pool"})')
                continue
            ents = f.tables[tab]
            states = [('BN6', X4.GENERIC6[k] | 1) for k in range(8)]
            for k, x in enumerate(ents):
                if X4.own_region(f, x & ~1):
                    ref = ('OWN', x | 1)
                else:
                    tw = pairs.twin(x & ~1)[0]
                    if tw is None or (k < 6 and x & ~1 == bn4_twins.GENERIC4[k] and tw != X4.GENERIC6[k]):
                        f.decide('actor', f'{x:08X}: its state {k} is no routine of its own and has no twin that serves')
                        continue
                    ref = ('BN6', tw | 1)
                if k < 6:
                    states[k] = ref
                else:
                    states.append(ref)
            if len(states) > 16 or any(r[0] != 'OWN' for r in states[8:]):
                f.decide('actor', f'{tab:08X}: its state table has {len(states)} states in BN6\'s layout, or one past the generic not its own')
            f.states = states
            lit = w.insns[chain[0][0]].lit
            f.numbers.append((lit, tab, f.home + HOME_STATES, 'its state table: the unit\'s in BN6\'s layout, at its home\'s XC4_HOME_STATES'))
            f.notes.append(f'its third-kind {reg[2]:#x} is an actor ({site:08X}: the actors\' runner with its table {tab:08X}, '
                           f'{len(ents)} states): its table in BN6\'s layout, {len(states)} states, its own from 8')
            for e2, w2 in sorted(f.walks.items()):
                if X4.own_region(f, e2) == reg:
                    addrs = sorted(w2.insns)
                    for k, a in enumerate(addrs):
                        X4.curaction(f, w2, addrs, k, w2.insns[a])


def numbers_reserved():
    """no number of TYPES3_NAVIS, and none of TYPES4's run, is one a track of BN5's moves or a Navi of BN5's not yet moved will
    (tools/bn5_twins.py dispatch_reserved); the same check as build.py lint's, which needs no ROM"""
    held = bn5_twins.dispatch_reserved(3)
    bad = [f'TYPES3_NAVIS\'s {n:#04x} is {held[n]}' for n in TYPES3_NAVIS if n in held]
    held = bn5_twins.dispatch_reserved(4)
    bad += [f'TYPES4\'s {n:#04x} is {held[n]}' for n in range(*TYPES4) if n in held]
    if bad:
        sys.exit('xchiprecipe_bn4: a dispatch number of BN4\'s is BN5\'s own: ' + '; '.join(bad) +
                 ' (take it out of the tuple; a Navi that held it takes another and runs again)')


def numbers_clear(units):
    """no other recipe names a dispatch number the chips' moved types take (an XvType row of its kind)"""
    for n in sorted(os.listdir(EMU)):
        if not n.endswith('.c') or n == os.path.basename(OUT_C):
            continue
        with open(os.path.join(EMU, n), encoding='utf-8') as fh:
            text = fh.read()
        for f in units:
            for k, a, t in f.types:
                if k != 3 and re.search(rf'\{{ {k}, 0x{a:02X}, ', text, re.I):
                    sys.exit(f'xchiprecipe_bn4: {n} names the dispatch\'s {k}:{a:#x}, which {f.name}\'s moved type takes: another number')


def attack_numbers_clear(units):
    """the attack units' numbers: each within BN4's chips' range of BN6's grown attack families as src/emu/xattack_grow.h has it
    (XATTACK_BN4 to XATTACK_BN4_END, ATTACK_FIRST and ATTACK_END here), one family a number, and none a family another recipe
    puts in the grown table (BN5's chips' units' rows, the Souls' families, a Navi's)"""
    with open(os.path.join(EMU, 'xattack_grow.h'), encoding='utf-8') as fh:
        head = fh.read()
    have = {n: int(v, 0) for n, v in re.findall(r'#define (XATTACK_\w+)\s+(0x[0-9A-Fa-f]+)', head)}
    if (have.get('XATTACK_BN4'), have.get('XATTACK_BN4_END')) != (ATTACK_FIRST, ATTACK_END) or have.get('XATTACK_SOULS_END', 0x100) > ATTACK_FIRST:
        sys.exit(f'xchiprecipe_bn4: src/emu/xattack_grow.h does not give BN4\'s chips the families {ATTACK_FIRST:#x}-{ATTACK_END - 1:#x}')
    if len(set(ATTACK_NUMBERS.values())) != len(ATTACK_NUMBERS) or any(not ATTACK_FIRST <= n < ATTACK_END for n in ATTACK_NUMBERS.values()):
        sys.exit('xchiprecipe_bn4: ATTACK_NUMBERS gives a number twice or one out of BN4\'s chips\' range')
    mine = {f.number: f.name for f in units if f.kind == 'attack'}
    for n in sorted(os.listdir(EMU)):
        if not n.endswith('.c') or n == os.path.basename(OUT_C):
            continue
        with open(os.path.join(EMU, n), encoding='utf-8') as fh:
            text = fh.read()
        # (a chips' unit's row: its kind, its game's family, BN6's number; a Soul's or a Navi's family: from, to)
        named = {int(x, 16) for x in re.findall(r'\{ XCHIP_ATTACK, 0x[0-9A-Fa-f]{2}, 0x([0-9A-Fa-f]{2}),', text)}
        named |= {int(x, 16) for x in re.findall(r'\{ 0x[0-9A-Fa-f]{2}, 0x([0-9A-Fa-f]{2}), (?:0x08[0-9A-Fa-f]{6}u \}|OWN\()', text)}
        for num in sorted(named & set(mine)):
            sys.exit(f'xchiprecipe_bn4: {n} names the grown attack family {num:#x}, which {mine[num]} takes: another number')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--check', action='store_true')
    ap.add_argument('--report', action='store_true')
    ap.add_argument('--unit', nargs='*', default=[])
    a = ap.parse_args()
    numbers_reserved()
    b4, b6 = X4.find_roms()
    pairs = X4.Pairs4(b4, b6)
    C.install(pairs)
    install(pairs)
    alloc = Alloc()
    more = tuple((k, int(s, 0), n) for k, s, n in (u.split(':') for u in a.unit))
    units, slot = [], 0
    for k, s, n in UNITS + CANDIDATES + more:
        units.append(derive(b4, b6, pairs, k, s, n, alloc, slot))
        slot += len(s) if isinstance(s, (tuple, list)) else 1   # (a row a subfamily: each its home's slot)
    os.makedirs(WORK, exist_ok=True)
    with open(os.path.join(WORK, 'bn4_report.md'), 'w', encoding='utf-8') as fh:
        fh.write('# BN4\'s chips\' units (tools/xchiprecipe_bn4.py)\n\n' + '\n'.join(report_unit(f) for f in units))
    for f in units:
        print(f'{f.name}: {len(f.decisions)} decisions open')
    whole = units[:len(UNITS)]
    if any(f.decisions for f in whole):
        sys.exit('xchiprecipe_bn4: a unit of UNITS has decisions open (.build/xchiprecipe/bn4_report.md): nothing written')
    if sum(len(f.subs) for f in whole) > X4C_UNITS:
        sys.exit('xchiprecipe_bn4: more units than homes')
    numbers_clear(whole)
    attack_numbers_clear(whole)
    text = c_file(whole)
    if a.check:
        with open(OUT_C, encoding='utf-8') as fh:
            if fh.read() != text:
                sys.exit('xchiprecipe_bn4: src/emu/xchips_bn4.c is not what the generator writes')
        print('xchiprecipe_bn4: src/emu/xchips_bn4.c is what the generator writes')
    elif not a.report:
        with open(OUT_C, 'w', encoding='utf-8') as fh:
            fh.write(text)
        print(f'xchiprecipe_bn4: {len(whole)} units written')


if __name__ == '__main__':
    main()
