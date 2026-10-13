#!/usr/bin/env python3
"""xchiprecipe.py [--check] [--report] [--unit NAME ...]: the code of BN5's chips moved into BN6's battle engine, as recipes
made from the two ROMs (issue #82's C3; docs/MULTIROM.md, The chips' code; docs/DEVTOOLS.md, The chip recipe generator).

A chip of BN5's runs by its record's family (its attack family's routine, the CurAction its user takes) or, for a support or
a Navi chip, by a routine its family calls by its subfamily. Where BN6 has no code of a chip's, this moves BN5's: a unit is
one such family or routine, its code every routine it reaches that BN6 has no twin of (walked instruction by instruction,
tools/xrecipe_thumb.py), its calls out paired with BN6's twins (tools/xrecipe_pairs.py, the project's pairs and audits in
tools/bn5_twins.py), the objects it spawns BN6 has no twin of moved with it through the second-level dispatch, its
literals, immediates and data each in BN6's terms, as tools/xrecipe.py does for a virus family (its Deriver: the same
walk, the same checks). Each unit takes a number of BN6's: a family of the grown attack families' table (0x60 on,
src/emu/xattack_grow.h), or an entry of the grown support or Navi routines' tables (src/emu/xchips_code.h).

  python3 tools/xchiprecipe.py           write src/emu/xchips_bn5.c (the units' recipes) and src/core/xchips_moved.c (the chips
                                         that run them: BN6's id, family and subfamily), the units whole (no decision open)
  python3 tools/xchiprecipe.py --check   write nothing: fail where the two files are not what it would write
  python3 tools/xchiprecipe.py --report  every unit of UNITS, CANDIDATES and --unit, what it takes and what needs a decision,
                                         into .build/xchiprecipe/report.md

Local only: both ROMs. Nothing of either is written: the C files hold addresses, counts and CRC32s."""
import argparse
import json
import os
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import xrecipe as X  # noqa: E402
import xrecipe_thumb as T  # noqa: E402
from xrecipe_family import BN5, BN6, Deriver, Family, kind_name  # noqa: E402
import xrecipe_family as F  # noqa: E402
from xrecipe_pairs import Pairs  # noqa: E402
import bn5_objects as OB  # noqa: E402
import bn5_twins  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_C = os.path.join(ROOT, 'src', 'emu', 'xchips_bn5.c')
OUT_MOVED = os.path.join(ROOT, 'src', 'core', 'xchips_moved.c')
OUT_PAS = os.path.join(ROOT, 'src', 'core', 'xchips_pas.c')
WORK = os.path.join(ROOT, '.build', 'xchiprecipe')
REGISTRY = os.path.join(ROOT, 'src', 'core', 'xchips_registry.c')

# ---- each game's chip tables (src/emu/bn5.h, bn6_chips.h; docs/ROM_DATA.md, The chips' code) ----

BN5_FAMILY_TABLE, BN5_FAMILY_COUNT = 0x080EB514, 73           # its attack families 0x10-0x58 (RunAIAttack's, 0x0801857A)
BN5_SUPPORT_TABLE, BN5_NAVI_TABLE = 0x080297BC, 0x080298C8     # its support and Navi chips' routines by subfamily (0x080EC450, 0x080E1AC0's)
BN6_FAMILY_TABLE, BN6_FAMILY_COUNT = 0x080EBFA0, 79
BN5_RECORDS, BN6_RECORDS = 0x0801E210, 0x08021DA8
BN5_SUPPORT_FAMILY, BN5_NAVI_FAMILY = 0x15, 0x41
BN6_SUPPORT_FAMILY, BN6_NAVI_FAMILY = 0x15, 0x1B
BN5_SET_ATTACK = 0x0800F200                                     # setAttack0-6 (r0 the CurAction), 4 bytes apart

# Where moved code may come from (copied where BN6 has no twin): BN5's objects' code, its chips' families and routines,
# its virus code shared by families, and the engine's helpers the units call that BN6 has no twin of (each its why)
COPY_RANGES = ((0x080B0000, 0x08104AC0), (0x080E0000, 0x080F2000))
COPY_HELPERS = {
    0x08016634: "an object's element set (r0: its +0x0E and its collision data's +2, both the same place in BN6's layouts: "
                "legit.json's collision layout's first 0x14 bytes alike); BN6 has no routine of its body",
    0x080127E0: "MegaMan's mood raised (r0 his side, r1 the amount; RedFrut's 0x32): his NaviStats +0xE (BN6's the same place, "
                "bn5_twins' audit of 0x08010D18) plus the amount, at most 0xFE, where not 0; BN6 has no routine of its body",
    0x08012530: "the swords' reach (Z Saver's, CustSwrd's, MoonBld's, Katana's): the first panel ahead with a foe on it, each panel "
                "tested by its flags against the side's mask (0x080125B8, through 0x0800AFB0) and 0x0800B348's object test along "
                "the rows of 0x080125C0; BN6 has no routine of its body (none found by the pairs' search), its calls BN6's twins",
}
# Routines copied though BN6 has a twin of the same body, and object types moved though BN6's runs the same code: where the
# data they read is BN6's own and another (each found by the chips' differential, tests/xdiff)
FORCE_COPY = {
    0x0800E190: "the misc family's subfamily 3 (the plus chips used alone: DarkPlus's power stored in the next chip of the hand, "
                "BN5 0x0800E1C8-0x0800E1CE, its green flash, a puff else): BN6's 0x08010488 another body, the hand's branch and the "
                "flash gone (SAME_MOVED's DarkPlus)",
    0x080D32FE: "the moon blade's sweep (MoonBld's): BN6's twin 0x080D750A the same code, its hit's stat effect 0x18 by BN6's rule "
                "(HIT_STATS: a virus struck by it bugged, chip_moonbld1's CanladEX losing 1 HP at t78), BN5's to be given BN6's terms",
    0x080D3340: "the moon blade's hit (MoonBld's): BN6's twin 0x080D754C, the same, called from the sweep",
    0x080CA454: "the shield's spawner (Guard's, CrsShld's): BN6's twin 0x080CB050 spawns BN6's type 0x2B",
    0x080CA33C: "the shield's first state: BN6's twin 0x080CAF38 the same code, reading its own table of kinds (0x080CAED4), "
                "BN5's (0x080CA2D8) another sprite at CrsShld3's row",
    0x080093A2: "the Anti chips' trap sprung: BN5's own (its effect 0x2B where the trap was, and sound 0xA5); BN6's twin "
                "(0x0800ABC6) spawns its own effect 0x46 lower down",
    0x080C7614: "the shockwave's spawner (Guard's, the reflected wave): BN6's twin 0x080C8554 spawns BN6's type 0x16",
    0x080E3898: "the Anti chips' counter's second state (its actions by +9, 0x080E38AC): BN6's twin 0x080E4904 the same code, "
                "copied to reach its strike (0x080E38BC)",
    0x080E38BC: "the Anti chips' counter's strike (its actions' third, 0x080E38AC's): BN6's twin 0x080E4928 the same code, "
                "its strike (BN5 0x080E3900, BN6 0x080E4970) spawning the explosion through each game's own spawner (BN5 "
                "0x080E32EC, BN6 0x080E4328), not the same to its depth (FORCE_MOVED's (4, 0x2B))",
}
# Routines aimed at their BN6 twin though the twin's tables differ from BN5's in length (the same code else, the families'
# copy's choice of the same routine, proven by their specs): BN5's routine: (BN6's, why)
FORCE_TWIN = {
    0x080CCFD0: (0x080CD4A8, "BoyBomb's and BomBoy's bomb's update (the third kind's 0x3E, its states run by the obstacle runner "
                             "0x08017FCC / 0x0801B394): BN6's twin runs BN6's actions table, its two status states before the bomb's "
                             "own, as BN6's generic states number them (BN5's 0x080133C4's twin writes 8 where BN5's 6); BN5's own "
                             "table mixed both numberings at a paralysis (bomboy_v1_keys at t1037), the families' copy BN6's twin"),
}
FORCE_MOVED = {
    (3, 0x2B): "the shield: BN6's type 0x2B runs the same code but reads its kinds' sprites from its own table (BN6 0x080CAEEC), "
               "BN5's (0x080CA2F0) another at its last row (list 3 0x1B in BN5, 0 in BN6: CrsShld3's); chip_crsshld1_reflect read "
               "the two tables' places apart",
    (3, 0x16): "the shockwave: BN6's type 0x16 runs the same code but reads its kinds from its own table (BN6 0x080C8370), BN5's "
               "(0x080C7448) another: speed and reach by kind (chip_guard1_reflect: BN6's wave a panel ahead from its first)",
    (4, 0x2B): "the Anti chips' counter: BN6's type 0x2B runs the same code but spawns its explosion through its own spawner "
               "(BN6 0x080E4328, its hit by 0x080E4378), BN5's (0x080E32EC, by 0x080F625A) another below (chip_antifire: the "
               "explosion's spawn BN5's moved code's in BN5, BN6's own in BN6)",
}
# BN5's routines a unit calls whose work no battle a run fights does, aimed at the chips' stubs (src/emu/xchips_code.h,
# XC_STUBS) instead of copied: a panel claim's two spawners (AqWhirl's, DrilArm's and Guard's wave's). Each spawns the
# fourth kind's 0x83 (0x080EA020, the one object that uses BN5's panel-claim pool 0x0802E920-0x0802EAB4 and its RAM
# 0x02035760, 0x02034B80, 0x0203C51C-0x0203C59F: no other code calls the pool, and no other code spawns 0x83) only where
# 0x080090E8 answers 1, the battle's bit 0x40 (BattleState +0x32, 0x0802D510); else it answers r0 0, Z set, r1-r3 as
# they came, as the stub does. The moved spawner would call BN6's twin of that gate, 0x0800A8F8, which reads BN6's bit
# 0x40 (0x0802E09A); BN6 sets it in one place, 0x0802E112 at a battle's start (from 0x0800795E), for a link battle alone:
# the battle's type not 1, 6, 7 or 9, its settings without 0x200000, and either the link's state 0x0200AD04 set (written
# only by the link routines with what the cable answered, 0x0803F4F8) or its settings' bit 8 with event flag 0x1722 (the
# Beast Link Gate). No run's battle is one (no link, the director sets neither); BN5 sets its own bit in one place too,
# 0x0802D594 (from 0x08006D2A), where the battle's type is not 1 and MegaMan's +0x2A is 1 or more (docs/MULTIROM.md,
# The chips' code)
STUB_ZERO = 0                             # (XC_STUBS + 0: movs r0,#0; mov pc,lr)
STUBS = {
    0x080EA08C: (STUB_ZERO, "a panel claim's spawner: its gate (0x080090E8, the battle's bit 0x40) clear in every battle a run "
                            "fights, r0 0 as BN5 answers then"),
    0x080EA0BA: (STUB_ZERO, "a panel claim's other spawner, the same"),
}
# Helpers: BN5's code more than one unit calls, whose object only one copy may move (the dispatch moves a type once):
# installed once, before the units, from XC_HELPERS; a unit's call to its routine aimed at its copy. (BN5's routine, the
# project's name, why)
# (The muzzle flash, BN5's image object, the first kind's 0x05 that BN6 runs other code for, is no helper of the chips': BN5's
# engine objects are moved once for every track, its module whole, installed with the Navis' before the chips' code
# (tools/bn5_objects.py, IMAGE): a unit's call to its spawner, 0x080B9CAE, aimed at its copy, the kinds the units spawn fixed
# there, flash_kinds)
HELPERS = (
    ((0x080EBB96,), 'BusterArm', "MegaMan's arm for a buster-like chip (0x080EBB96 and the routine it calls, 0x080EBBA6: SideBub's, "
                              "SpShake's, Pulsar's, WoodNos's, CactBal's, BN5's plain buster's too): its muzzle flash by his "
                              "form, through the image object's spawner; one copy, so that BN5's own buster's calls are no unit's"),
    ((0x080D3286,), 'Shake', "the shake's spawner (the third kind's 0x68: SpShake's, and the misc family's ShakPar's), the shake's "
                             "code with it: the dispatch moves a type once, so one copy for both units"),
    # (the computed powers, POWERS: each a routine BN5's table of computed powers names, 0x0800E728 by the record's power - 1000)
    ((0x0800EA18,), 'PowerCustSwrd', "CustSwrd's power (1045): by the Custom gauge (BN5 0x0801A8D0)"),
    ((0x0800EB06, 0x0800EB0C, 0x0800EB12), 'PowerCusVolt', "CusVolt1-3's powers (1073-1075): by the Custom gauge, from 10, 30, 50"),
    ((0x0800E8D6,), 'PowerSP', "a Navi chip SP's power (1002-1021): by BN5's best deletion time of the Navi's SP (its battle's copy "
                               "0x0203E560 of the save's 0x02002670), which BN6 keeps none of for BN5's Navis: its literal aimed at a "
                               "table of none (XC_SP_TIMES, WORDS_TO), BN5's power for a save without one; its bit 0x40 test (a "
                               "Liberation Mission's in BN5) BN6's twin 0x0800A8F8, a link battle's, clear in a run's"),
    ((0x0800E9A8,), 'PowerDS', "a Navi chip DS's power (1023-1044): by the side's battle counter 3 (BN5 0x0800931C, BN6 0x0800AB3A)"),
)
# BN6's table of computed powers (a chip's power of 1000 and up: the routine by power - 1000, BN6 0x080109A4 with 45 of its
# own at 0x080109DC) grown to 256 at XC_POWERS: BN5's entry i at BN6's POWER_FIRST + i, its routine one of the helpers'
BN5_POWERS, BN5_POWER_COUNT = 0x0800E728, 76
POWER_FIRST = 0x80
# The bases a computed power's routine subtracts from its chip's record's power (its row in a table of its own), each a word
# of its pool: BN5's power 1000 + i is BN6's 1000 + POWER_FIRST + i in the moved records (src/core/xchips_moved.c,
# xchips_power_of), so the base is BN6's that much further (BN5's literal: why)
POWER_BASES = {
    0x0800EC2C: "the SP powers' base, 1001 (their rows 1-20, NumbrMn[SP]'s and on: 0x0800E8E2)",
    0x0800EC3C: "the DS powers' base, 1023 (their rows 0-21: 0x0800E9B4)",
}
# Tables of routines a family runs by its subfamily, by the unit's label: BN5's table: (its entries, why). An entry no
# subfamily of the unit's chips names (the registry's rows of its family moved, or waiting as a DarkChip or a Program
# Advance) is aimed at the Navi stub (XC_STUBS + 4: mov pc,lr) and not walked: BN6's own chips of the family (XREG_SAME)
# run BN6's own code, so those subfamilies never run from the copy (a call to one from the unit's code a decision)
SUB_TABLES = {
    'Bombs': {0x080EBD4C: (15, "the bombs' spawners by subfamily (0x080EBCF2: AttackVars +3)")},
    'Misc': {0x080EC818: (27, "the misc family's routines by subfamily (0x080EC7E4: AttackVars +3)")},
}
SUB_STUB = 4
# Calls aimed at the chips' stubs in one unit alone (by its label): the SP power's gate (0x080090E8, the battle's link bit)
# answering 1, so BN5's fixed powers of a link battle are taken (0x0800E95C) where BN5 reads its own deletion times
UNIT_STUBS = {
    # (ElemRage's flame's place: BN5's point 0x14 of MegaMan's sprite, (-1, 26) in every one of his rows, BN6's 0x14 another,
    # (20, 25), and none of BN6's (1, 26): the call answered with BN5's numbers, POINTS)
    'ElemRage': {0x08015164: (0x10, "MegaMan's point 0x14, (-1, 26) in each of BN5's rows (0x080151C4 on): BN5's numbers")},
    # (HolyDrem's shots' place: BN5's point 0x1D of MegaMan's sprite, (40, 20) in every one of his rows, BN6's 0x1D another, (120,
    # -4), and none of BN6's (40, 20): the call answered with BN5's numbers, x by his facing as 0x0800C6CE has it)
    'HolyDrem': {0x08015140: (0x18, "MegaMan's point 0x1D, (40, 20) in each of BN5's rows (0x080151C4 on), x by the facing "
                                    "(r2 side, r3 flip, as 0x0800C6CE): BN5's numbers")},
}
# The object types a helper moves, for every unit (the units spawn them through the dispatch, the helper's copy run)
HELPER_TYPES = {(3, 0x68): 'Shake'}
# Raw data whose extent no walk tells (a table read by a number made elsewhere): its bytes
# Routines a unit hands a table of routines to, in a register (its number): the table's every entry walked and moved or
# aimed at its twin, as a jump table's (the family deriver stops a table at its first entry far from it)
TABLE_ARGS = {
    0x08017FCC: 0,   # an obstacle's states (BN6 0x0801B394): r0 its actions' routines by CurAction (BoyBomb's bomb, 0x080CD024)
}
# Code in a block no walk reaches, no word, call or branch of the ROM naming any of its halfwords (searched): copied as it is,
# never run (the walk's 'unreached' decision taken)
DEAD_CODE = {
    0x080DA27C: "AqWhirl's: between 0x080DA27A's branch past it and its target 0x080DA28E",
    0x080CD346: "BoyBomb's bomb's: after 0x080CD344's branch past it to 0x080CD380 (its own loop's branches its only)",
}
# ROM data a unit's literal names that BN6 keeps none of, copied with the unit (moved as its block's): its bytes
DATA_COPY = {
    0x0801D27A: (0x120, "the Navi chips' SP and DS powers by time and by counter (13 rows of 11 u16), BN5's alone"),
}
# Literal words a unit's code holds that no BN6 run reads (their path a stub makes dead): kept as they are, by the unit
# CurActions a unit's object writes that index its own states' table, not the engine's (no status states before them): BN5's
# numbers kept, where the family deriver's rule would add BN6's two (BN5's site: why)
OWN_STATES = {
    0x080B9EE0: "KnightMan's object (the first kind's 0x06, KnightMn's): its +9 indexes its own states' table by 4 (0x080B9DA8's, "
                "four entries at 0x080B9DAC, its update 0x080B9D10's), 0xC its fourth; BN6's 0xE would read between two (chip_knightmn: "
                "BN6's battle frozen from t33, the Navi chip's object never let go)",
}
UNREAD_WORDS = {}
# Literal words of BN5's RAM a unit names that BN6's engine keeps at a place of the chips' own: (unit, BN5's word): (BN6's, why)
WORDS_TO = {
    ('PowerSP', 0x0203E560): (None, "BN5's best deletion times of its Navis' SPs (u16 frames by the Navi's AI index, 0x2C a side, "
                                    "0xFFFF none: the battle's copy of the save's 0x02002670), which BN6 keeps none of: XC_SP_TIMES, "
                                    "a table of none (src/emu/xchips_code.c)"),
}
# Sprite loads by an object's kind from a table of its own, each row its sprite (its list x 4 and number at two offsets, or
# its number alone in a list the code names): BN5's load site: (the table, a row's size, the list's offset or None, the
# number's offset, its rows, the list where the code names it); every row's sprite BN6's own where the same bytes, else
# BN5's copied and the row's number set to the copy's (table_art); a row of list 0xFF draws nothing
TABLE_SPRITES = {
    0x080C5EEC: (0x080C5DF4, 16, 5, 6, 12, None),   # the bubbles' kinds (SideBub's): 12 rows to the code at 0x080C5EB4
    0x080C49B0: (0x080C4800, 12, 5, 6, 31, None),   # the third kind's 0x00's kinds (Guard's): 31 rows to the code at 0x080C4974
    0x080C74D2: (0x080C7448, 4, None, 0, 15, 4),    # the shockwave's kinds (Guard's reflected wave): 15 rows, list 4's
    0x080CA350: (0x080CA2D8, 6, 0, 1, 10, None),
    0x080CDAB6: (0x080CDA78, 2, 0, 1, 1, None),   # the geyser's (the bombs' 0x5), its one kind: list 3's 2
    0x080D0C6C: (0x080D0C0C, 6, None, 0, 6, 3),   # the misc family's third kind at 0x080D0C30, by kind: list 3's 0x31 each    # the shield's kinds (Guard's, CrsShld's): 10 rows to its code at 0x080CA314
    0x080C4244: (0x080C4214, 2, 0, 1, 1, None),   # NumberMan's object (the first kind's 0x5C, NumbrMan's): its one kind, list 4's 6, its code from 0x080C4216
}
# Sprite loads of MegaMan's own sprite, whatever it is (0x0800DA6A's list and number: his body's), each game's own: BN5's
# load site: why
MEGAMAN_LOADS = {
    0x080BD86A: "HolyDrem's double of MegaMan (the first kind's 0x1C, its set-up 0x080BD834): MegaMan's own sprite where its user "
                "is his (0x0800D1B8's kind 2), its list and number 0x0800DA32's by his NaviStats +0x2C and +0x29 (BN6's twin "
                "0x0800FC9E, a Soul's by src/emu/soullook.c's hook), else its user's by its NameID (0x0800D188); BN6's own in BN6",
    0x080C3A2A: "Poltrgst's double of MegaMan (the first kind's 0x58, its set-up 0x080C39F4): as HolyDrem's (0x080BD86A), the same steps",
    0x080E3648: "the swords' afterimage (the fourth kind's 0x28): MegaMan's own sprite, its list and number 0x0800DA6A's, BN6's "
                "own in BN6 (his body's look, the mask 'MegaMan's own sprite')",
    0x080BA850: "BN5's family 0x49's afterimage (the first kind's 0x0A, at 0x080BA944): MegaMan's own sprite, its list and "
                "number 0x0800DA32's by his NaviStats +0x2C and +0x29, BN6's own in BN6",
}
# Effect ids a unit's code reads from a byte table of its own: BN5's spawn site: (the table, a row's size, the id's offset,
# its rows); each row's id BN6's twin of it or the chips' own (effect_of), set in its byte (u.bytes)
TABLE_EFFECTS = {
    0x080EE364: (0x080EE3D8, 1, 0, 3),   # Z Saver's three slashes' effects, 0x39-0x3B
    0x080EC016: (0x080EC110, 1, 0, 8),   # CustSwrd's slash by its kind, 0x16-0x1C and 0x28
    0x080EFC72: (0x080EFCC8, 1, 0, 2),   # Katana's two slashes, 0x17 and 0x16 (its +0x12 less one)
    0x080DAE4E: (0x080DAE74, 3, 2, 4),   # ElcReel's four sparks' rows (x, y, effect: 0x64, 0x65)
    0x080CEF66: (0x080CEEBC, 1, 0, 4),   # the seeds' splash by kind (0x080CEE70: r2, on as r4 at 0x080CEF5E): 0x29, 0x29, 0x54, 0x55
    0x080CEFA0: (0x080CEEBC, 1, 0, 4),   # ... the same, its second spawn (0x080CEF7A)
    0x080C9454: (0x080C94D0, 1, 0, 4),   # the crack bombs' cracks by kind (+4 & 3), handed to 0x08018838 (EFFECT_CALLS): 0x50-0x53
    0x080C6DD6: (0x080C6C08, 6, 2, 20),  # the spread shot's burst by kind (its +0x60's row, byte 2; 20 rows to its code)
    0x080C6DBE: (0x080C6C08, 6, 2, 20),  # ... the same, handed to 0x08018838 (EFFECT_CALLS) where it bursts in a pattern
}
# Panel types an object sets from a table of its own (setPanelType's r2 a row's byte): BN5's call site: (the table, a row's
# size, the type's offset, its rows); a row of BN5's lava, sea or holy given BN6's number (u.bytes), 0xFF none
TABLE_PANELS = {
    0x080C4AAA: (0x080C4800, 12, 8, 31),   # the third kind's 0x00's kinds: rows 20 (sea) and 26 (lava)
    0x080CEEE0: (0x080CEEB8, 1, 0, 4),     # the seeds' panels by kind (0x080CEE8A: r2, on as r5): ice, sea, grass, lava
    0x080E2CBC: (0x080E2A34, 20, 0, 20),   # the panel changer's kinds (the fourth kind's 0x1F, by its +4): ElemRage's 0xB plain
}
# BN5's panel types BN6 has none of: its 5 the magnet panel (MagPanl's, its flags 0x10210: docs/ROM_DATA.md, Battlefields;
# ElemRage's Elec), which xpanels.c brings in no more than BN6 has it. A table's row of it left as BN5's where no moved unit
# reads that row (TABLE_PANELS: the panel changer's kind 4, MagPanl's alone), or given ABSENT_PANEL where a moved unit
# compares a panel's type with it (PANEL_LOOKUPS)
PANELS_ABSENT = {0x5: "the magnet panel"}
ABSENT_PANEL = 0xFE   # (a type no panel has, short of the 0xFF that ends a lookup)
# Rows a moved unit reads of TABLE_PANELS' tables, where rows of a type BN6 lacks stand among them: BN5's site: the rows
TABLE_PANEL_ROWS = {0x080E2CBC: {0xB: "ElemRage's (0x080EFE02: movs r4,#0xB)"}}
# Tables of panel types a unit compares a panel's with (BN5's table: a row's size, the type's offset, its rows, why): each
# row's BN5's lava, sea or holy given BN6's number, a type BN6 lacks ABSENT_PANEL, 0xFF (the end) kept
# Panel types set from a value no table holds, each its case: BN5's site: why it sets none BN6 numbers otherwise
PANEL_SITES = {
    0x080D9BA2: "ElemRage's flame (the third kind's 0x98): the type its +7's high nibble, the attack's AttackVars +0xF "
                "(0x080EFE22: its record's AttackParam4, 0, ORed with 4 at 0x080EFDF2): 0, so it sets none (0x080D9B9C)",
}
PANEL_LOOKUPS = {
    0x080EFECC: (4, 0, 6, "ElemRage's element by MegaMan's panel (0x080EFEA2: its type against each row's till 0xFF)"),
}
DATA_SIZES = {
    0x080C5DF4: 12 * 16,  # the bubbles' kinds (TABLE_SPRITES)
    0x080C4800: 31 * 12,  # the third kind's 0x00's kinds (TABLE_SPRITES, TABLE_PANELS)
    0x080C7448: 15 * 4,   # the shockwave's kinds (TABLE_SPRITES)
    0x080CA2D8: 10 * 6,   # the shield's kinds (TABLE_SPRITES)
    0x080E2A34: 20 * 20,  # the panel changer's kinds (TABLE_PANELS): 20 rows of 20 bytes, its code at 0x080E2BC4
}
# The muzzle flash's numbers a call makes otherwise than by one instruction (the number its r4's low byte), each read from
# the code: BN5's call site: (the numbers, how)
FLASH_NUMBERS = {
    0x080EBC7E: ((0x23, 0x2B, 0x33), "the buster arm's (0x080EBBA6, SideBub's, SpShake's, Pulsar's, WoodNos's, CactBal's): 0x23 "
                                     "by default, 0x2B or 0x33 by his form (AI data +0/+1) and NaviStats +0x2C/+0x4C"),
    0x080EF602: ((0x2F,), "DrilArm's: r4 0x12F, 0x0100012F where its AttackVars +3 is 1: the low byte 0x2F"),
    0x080EE28C: ((0x03, 0x0A, 0x32), "Z Saver's: r4's low byte 0x080EC16A()'s, the sword's flash by his form: 3, 0xA or 0x32"),
    0x080EBF0E: ((0x03, 0x0A, 0x32), "CustSwrd's: r4's low byte 0x080EC16A()'s, as Z Saver's"),
    0x080EFB9E: ((0x03, 0x0A, 0x32), "Katana's: r4's low byte 0x080EC16A()'s, as Z Saver's"),
    0x080ED4E4: ((0x03, 0x0A, 0x32), "Slasher's: r4's low byte 0x080EC16A()'s, as Z Saver's"),
    0x080EF4C8: ((0x2C, 0x2D, 0x2E), "MrkCan's: r4 0x080EF548()'s number << 8, + 0x2C + its subfamily (AttackVars +3: 0, 1, 2 for "
                                     "MrkCan1-3)"),
    0x080ECE76: ((0x0C, 0x17), "the Tornado family's: r4 0x0C, 0x17 for its subfamilies 2 and 3 (DarkTorn's, with a palette in its top "
                               "byte), 0x080ECE5C-0x080ECE70"),
    0x080EBCD0: ((0x04, 0x1A, 0x28, 0x2A), "the bombs': r4 its halfword table's (0x080EBD88) by subfamily, the low byte the flash: "
                                           "the played subfamilies' 4, 5, 0xA, 0xB (4), CannBall's 0x1A, SeaSeed's 0x28, LavaSeed's 0x2A"),
}

# Object types BN6's own serves though its code is not BN5's to its depth: the engine's objects every attack of both games
# spawns, their differences the engine's (each audited; docs/ROM_DATA.md, The chips' code)
TYPE_AUDITS = {
    (3, 0x03): "the engine's hit area (an attack's damage on a panel for its frames; spawned by BN5 0x080C4EAE, BN6 0x080C6C16): "
               "the same states, its collision data's hit word read at each game's own place (BN5 +0x68 at 0x080C4E42, BN6 +0x70 "
               "at 0x080C6BAA) and BN6's end state freeing its panel's reservation first (0x08016C8C calls 0x0801BB78), as BN6 "
               "does for every object",
    (4, 0x00): "the engine's effect (an effect id's sprite and animation, spawned by BN5 0x080DFFD2, BN6 0x080E1932): the same "
               "states, BN6's but for a call each frame (0x080E1952) that copies its owner's visibility where its second spawner "
               "(0x080E194A) set +0x60, which its first leaves clear; its id read from each game's own table (each effect a unit "
               "spawns compared, a decision where the entries differ)",
}

# ---- the places (src/emu/free_space.h, src/emu/xchips_code.h) ----

EMU_FREE = 0x08800000
FREE_CHIP_CODE = EMU_FREE + 0x600000
XC_AT = FREE_CHIP_CODE + 0x400            # FREE_CHIP_MOVED
XC_STUBS = XC_AT + 0x800                  # the stubs (STUBS), then
XC_SP_TIMES = XC_STUBS + 0x30             # ... the SP powers' deletion times, none (WORDS_TO), then
XC_HELPERS = XC_STUBS + 0x90              # ... the helpers' code (HELPERS), to
XC_POWERS = XC_AT + 0x1C00                # BN6's computed powers grown (256 words), then
XC_HOMES = XC_AT + 0x2000                 # a unit's veneers, XC_HOME_SIZE each
XC_HOME_SIZE, XC_UNITS = 0x400, 64
XC_CODE = XC_HOMES + XC_HOME_SIZE * XC_UNITS
XC_ART = FREE_CHIP_CODE + 0xA0000
XC_END = FREE_CHIP_CODE + 0x100000
CODE_STEP = 0x10
# Each unit's number in BN6, fixed by BN5's (the static model: the same at every boot and in every build, a state saved
# mid-attack names it): BN5's attack family F is BN6's 0x50 + F (0x60-0xA8, within src/emu/xattack_grow.h's XATTACK_CHIPS),
# BN5's support or Navi subfamily S the grown table's entry past BN6's own routines + S
FAMILY_BASE = 0x50
FIRST_SUPPORT, FIRST_NAVI = 0x2A, 0x1D     # (BN6_CHIP_ROUTINE_COUNT, BN6_NAVI_ROUTINE_COUNT)


def number_of(kind, n):
    if kind == 'helper':
        return 0
    return FAMILY_BASE + n if kind == 'attack' else (FIRST_SUPPORT if kind == 'support' else FIRST_NAVI) + n

# The units moved, in install order: (kind, BN5's family or subfamily, the project's name), each
# its number by BN5's (number_of); each whole (no decision open) and its chips' specs passing (tests/xdiff/chip_*.json).
UNITS = (
    ('attack', 0x1B, 'CrakOut'),
    ('attack', 0x58, 'HotBody'),
    ('support', 0x13, 'Jealousy'),
    ('support', 0x15, 'NrthWind'),
    ('support', 0x06, 'MetaGel'),
    ('support', 0x08, 'Blinder'),
    ('attack', 0x55, 'Skully'),
    ('attack', 0x4F, 'DrilArm'),
    ('support', 0x3D, 'RainyDay'),
    ('attack', 0x28, 'WoodNos'),
    ('attack', 0x48, 'CactBal'),
    ('attack', 0x51, 'SpShake'),
    ('attack', 0x52, 'Pulsar'),
    ('attack', 0x4E, 'MrkCan'),
    ('attack', 0x1C, 'SideBub'),
    ('attack', 0x47, 'AqWhirl'),
    ('attack', 0x34, 'ZSaver'),
    ('attack', 0x54, 'Katana'),
    ('attack', 0x23, 'ElcReel'),
    ('support', 0x3A, 'BoyBomb'),
    ('attack', 0x1F, 'Guard'),
    ('attack', 0x53, 'MoonBld'),
    ('attack', 0x27, 'CusVolt'),
    ('attack', 0x13, 'CustSwrd'),
    ('support', 0x0C, 'Anti'),
    ('attack', 0x49, 'Reaction49'),
    ('attack', 0x29, 'Slasher'),
    ('attack', 0x56, 'ElemRage'),
    ('attack', 0x12, 'Bombs'),
    ('attack', 0x1A, 'Misc'),
    ('attack', 0x19, 'Vulcan'),
    ('attack', 0x25, 'Spreads'),
    ('support', 0x2A, 'DarkCirc'),
    ('support', 0x1B, 'DrkSonic'),
    ('navi', 0x00, 'NumbrMan'),
    ('navi', 0x05, 'KnightMan'),
    ('navi', 0x0A, 'Colonel'),
    ('navi', 0x0B, 'ShadowMan'),
    ('navi', 0x0C, 'TomahawkMan'),
    ('navi', 0x0D, 'ToadMan'),
    ('support', 0x20, 'BlakWing'),
    ('attack', 0x22, 'Tornado'),
    ('support', 0x23, 'HolyDrem'),
    ('support', 0x31, 'Poltrgst'),
)
# The next units, derived for the report alone (--report) until what each needs is in: its decisions there, and the
# chips' differential's finding where a spec ran it (docs/MULTIROM.md, The chips' code: what remains)
CANDIDATES = (('support', 0x3B, 'DarkInvs'),)


# ---- the registry's rows: which chips a unit plays ----

def registry_rows():
    import re
    with open(REGISTRY, encoding='utf-8') as f:
        text = f.read()
    rows = []
    for a, b, how in re.findall(r'^\s*\{\s*(\d+),\s*(\d+),\s*XREG_(\w+)', text, re.M):
        rows.append((int(a), int(b), how))
    return rows


def record(rom, base, i):
    return rom.bytes(base + 0x2C * i, 0x2C)


# ---- a unit ----

class Unit:
    pass


def entry_of(b5, kind, n):
    if kind == 'helper':
        return n[0] if isinstance(n, tuple) else n
    if kind == 'attack':
        return b5.u32(BN5_FAMILY_TABLE + 4 * (n - 0x10)) & ~1
    table = BN5_SUPPORT_TABLE if kind == 'support' else BN5_NAVI_TABLE
    return b5.u32(table + 4 * n) & ~1


class ChipDeriver(Deriver):
    """the family deriver's walk and checks, for a chip's code: no family region, every routine without a twin in BN5's
    objects', chips' and virus code copied (COPY_RANGES), or one of COPY_HELPERS"""

    helper_at = {}   # (BN5's helper routines and their copies, once the helpers are derived)

    join_pool_gaps = False   # (a unit's gaps joined by its own walk: gap_roots, gap_spans, the routines in them its own)

    unplayed = {}   # (SUB_TABLES' entries of subfamilies the unit plays none of: BN5's routine, why)

    def kind_of(self, f, t, pointer=False):
        # (a pointer's target classed as a call's: an object's routine whose twin is not the same code to its depth is copied
        # whichever names it, below)
        t &= ~1
        if t in f.own:
            return 'own', None
        mod = next((x for key, x in self.navi_modules.items() if key in self.navi_routing and t in x[3]), None)
        if mod is not None:
            # (a routine of the code of a type it spawns as a Navi's copy, XV_TYPE_SHARED: that copy's, in the Navi's recipe; a
            # unit's direct call to it, as the type's own code calls it, one copy for both)
            return 'engine', (mod[2] + t - mod[0], 'the Navis\' copy', 'SAME', None)
        m = OB.module_of(t)
        if m is not None:
            # (BN5's engine objects moved once for every track, installed with the Navis' before the chips' code: their copy
            # run, its type for the unit's spawns too, tools/bn5_objects.py)
            f.needs.add((m['kind'], m['type']))
            return 'engine', (OB.objects_at(t), 'the moved objects', 'SAME', None)
        if t in self.unplayed:
            return 'engine', (XC_STUBS + SUB_STUB, 'a stub', 'STUB', self.unplayed[t])
        if t in self.helper_at:
            return 'engine', (self.helper_at[t], 'a helper', 'HELPER', 'its copy, installed once for every unit')
        if t in UNIT_STUBS.get(f.name, {}):
            off, why = UNIT_STUBS[f.name][t]
            return 'engine', (XC_STUBS + off, 'a stub', 'STUB', why)
        if t in STUBS:
            return 'engine', (XC_STUBS + STUBS[t][0], 'a stub', 'STUB', STUBS[t][1])
        if t in FORCE_COPY:
            f.notes.append(f'copies {t:08X}: {FORCE_COPY[t]}')
            return 'foreign', (None, 'forced', None, None)
        if t in FORCE_TWIN:
            f.notes.append(f'{t:08X} -> {FORCE_TWIN[t][0]:08X}: {FORCE_TWIN[t][1]}')
            return 'engine', (FORCE_TWIN[t][0], 'forced', 'SAME', None)
        tw = self.pairs.twin(t)
        audit = self.pairs.audit(t, tw[0]) if tw[0] else None
        if audit and audit[0] == 'copy' and any(lo <= t < hi for lo, hi in COPY_RANGES + ((0x08000000, 0x080B0000),)):
            # (a twin whose audit serves BN5's own body, every track's: copied with the unit's code, its calls through their twins)
            f.notes.append(f'copies {t:08X}: its audit serves BN5\'s own body ({audit[1]})')
            return 'foreign', tw
        if tw[0] and any(lo <= t < hi for lo, hi in COPY_RANGES) and not self.same(t, tw[0]):
            # (an object's routine whose twin is not the same code to its depth: a dispatcher alike, its states not; BN5's
            # copied, as a virus family's are)
            f.notes.append(f'copies {t:08X}: BN6\'s {tw[0]:08X} ({tw[1]}) is not the same code to its depth')
            return 'foreign', (None, 'not the same to its depth', None, None)
        if tw[0]:
            if any(lo <= t < hi for lo, hi in COPY_RANGES) and self.spawns_moved(t, forced=False):
                # (BN6's twin would spawn BN6's own of a type whose code is not BN5's: BN5's copied, spawning BN5's moved)
                f.notes.append(f'copies {t:08X}: BN6\'s {tw[0]:08X} would spawn BN6\'s own of a type whose code differs')
                return 'foreign', (None, 'spawns a moved type', None, None)
            if self.spawns_moved(t):
                f.notes.append(f'{t:08X} -> {tw[0]:08X}: BN6\'s twin spawns BN6\'s own of a type FORCE_MOVED (the differential says '
                               f'whether it matters: FORCE_COPY where it does)')
            return 'engine', tw
        if t in COPY_HELPERS or any(lo <= t < hi for lo, hi in COPY_RANGES):
            return 'foreign', tw
        return 'missing', tw

    def classify(self, f, blocks):
        """the family deriver's, and the aligned words of raw data that point into the unit's blocks (a table of pointers to
        byte lists: CrakOut's crack patterns, 0x080C7900) a pool's, moved with the code"""
        data = super().classify(f, blocks)
        # (a table of routines whose next word is a routine too: cut short where its entries lie far from it, the rest
        # copied as BN5's addresses or not at all, which BN6's engine would jump to: BoyBomb's bomb's actions were)
        for w in f.walks.values():
            for tab, ents in w.tables.items():
                nxt = self.b5.u32(tab + 4 * len(ents))
                if T.is_code_ptr(nxt, self.b5) and nxt < 0x08110000:
                    f.decide('table', f'{tab:08X}: a table of {len(ents)} routines, its next word {nxt:08X} a routine too (TABLE_ARGS)')
        # (unreached code in a gap between a load and its pool, copied raw: other routines' no walk of this unit reaches,
        # their bytes moved along and never run from here, a helper's shared pool's neighbours)
        for k in [k for k in f.decisions if k[0] == 'unreached']:
            lo, hi = int(k[1][:8], 16), int(k[1][9:17], 16)
            if any(glo <= lo and hi <= ghi for glo, ghi in getattr(self, 'gaps', ())):
                f.decisions.remove(k)
                f.notes.append(f'{k[1][:17]}: in a gap copied raw (other routines\', never run from this copy)')
        for k in [k for k in f.decisions if k[0] == 'unreached' and int(k[1][:8], 16) in DEAD_CODE]:
            f.decisions.remove(k)
            f.notes.append(f'{k[1][:17]}: dead code ({DEAD_CODE[int(k[1][:8], 16)]})')
        inside = lambda v: any(lo <= (v & ~1) < hi for lo, hi in blocks)
        out = []
        for a, b, k in data:
            if k != 'RAW':
                out.append((a, b, k))
                continue
            words = [x for x in range((a + 3) & ~3, b - 3, 4) if inside(self.b5.u32(x))]
            at = a
            for x in words:
                if at < x:
                    out.append((at, x, 'RAW'))
                out.append((x, x + 4, 'POOL'))
                at = x + 4
            if at < b:
                out.append((at, b, 'RAW'))
        return out

    def covered(self, f):
        """the family deriver's, but raw data (a table a literal names) ends where a routine of another's begins: a chip's code
        stands among other families' routines, not after its records, so a table runs 0x80 bytes at most to the next push
        {..., lr} or the next word its code reads"""
        cov = super().covered(f)
        for w in f.walks.values():
            for v in w.lits.values():
                if v in DATA_COPY:
                    for a in range(v, v + DATA_COPY[v][0]):
                        cov.setdefault(a, 'RAW')
        # (a table DATA_SIZES knows: its bytes all, where the walk took its start for raw data)
        for v, n in DATA_SIZES.items():
            if cov.get(v) == 'RAW':
                for k in range(n):
                    if cov.get(v + k, 'RAW') != 'RAW':
                        break
                    cov[v + k] = 'RAW'
        raw = sorted(a for a, k in cov.items() if k == 'RAW')
        for a in raw:
            if cov.get(a) != 'RAW' or a & 1:
                continue
            if self.b5.u16(a) & 0xFF00 == 0xB500 and cov.get(a - 2) == 'RAW' and cov.get(a - 1) == 'RAW':
                k = a
                while cov.get(k) == 'RAW':
                    del cov[k]
                    k += 1
        # (a gap between a load and its pool whole, other routines' code in it too: a helper's shared pool past them)
        for lo, hi in getattr(self, 'gaps', ()):
            for a in range(lo, hi):
                cov.setdefault(a, 'RAW')
        return cov

    def scan(self, f):
        """the family deriver's, but a CurAction OWN_STATES names kept as BN5's"""
        super().scan(f)
        for i in [i for i in f.imms if i['at'] in OWN_STATES and i['why'].startswith('CurAction')]:
            f.imms.remove(i)
            f.notes.append(f'{i["at"]:08X}: CurAction {i["was"]} kept: {OWN_STATES[i["at"]]}')

    def scan_words(self, f, blocks):
        """the family deriver's, and the words UNREAD_WORDS names kept as they are"""
        super().scan_words(f, blocks)
        for k in [k for k in f.decisions if k[0] == 'literal']:
            v = int(k[1][:8], 16)
            if (f.name, v) in UNREAD_WORDS:
                f.decisions.remove(k)
                f.notes.append(f'{v:08X} kept: {UNREAD_WORDS[(f.name, v)]}')
                f.words[v] = (v, UNREAD_WORDS[(f.name, v)], None)   # (named as it is: the installer's word, BN5's)
            elif (f.name, v) in WORDS_TO:
                f.decisions.remove(k)
                f.words[v] = (XC_SP_TIMES, WORDS_TO[(f.name, v)][1], None)

    def spawns_moved(self, a, forced=True, depth=6, seen=None):
        """whether the BN5 routine at `a`, a twin's, spawns (itself, or through the routines it calls and the objects it
        spawns, `depth` deep) an object BN6 has no twin of (or one FORCE_MOVED, where `forced`): BN6's twin would spawn
        BN6's own there"""
        seen = set() if seen is None else seen
        if a in seen or depth < 0 or not any(lo <= a < hi for lo, hi in COPY_RANGES):
            return False
        seen.add(a)
        w = T.walk(self.b5, a)
        for site, t in w.bls.items():
            if t in BN5['spawners']:
                v, _ = T.back_value(w, site, 0)
                if v is None or v > 0xFF:
                    return True
                tt = self.plain_twin(BN5['spawners'][t], v, forced)
                if tt['bn6'] is None or (tt['routine'] and self.spawns_moved(tt['routine'], forced, depth - 1, seen)):
                    return True
            elif self.spawns_moved(t, forced, depth - 1, seen):
                return True
        # (and its states', where it runs them from a table: an object's routine is its states' dispatcher, each state
        # its substates', a hop that takes no depth)
        return any(self.spawns_moved(x, forced, depth, seen) for ents in self.pairs.tables_of(self.b5, w) for x in ents)

    def find_tables(self, f, w):
        """the family deriver's, and tables whose entries lie anywhere in the ROM: a chip family's table of routines by its
        subfamily (BN5's misc family 0x1A's at 0x080EC818) names routines of objects' code far from it; a literal is such a
        table where it holds two code pointers or more one after another and the routine loads a word from it by a register
        it then calls or jumps to, or hands it to a routine that does (TABLE_ARGS: an obstacle's actions)"""
        out = super().find_tables(f, w)
        handed = set()
        for site, t in w.bls.items():
            if t in TABLE_ARGS:
                v, chain = T.back_value(w, site, TABLE_ARGS[t])
                if v is not None and chain and chain[-1][1] == 'ldr_lit':
                    handed.add(v)
        for lit, v in w.lits.items():
            if v is None or v & 3 or not self.b5.has(v, 8) or not (used_as_table(w, lit) or v in handed):
                continue
            ents = []
            for k in range(64):
                x = self.b5.u32(v + 4 * k)
                if not T.is_code_ptr(x, self.b5):
                    break
                ents.append(x)
            # (an object's states end in the engine's own routines, far from it: HotBody's aura's third state, BN5's
            # 0x0800325C, the family deriver's table stopped short of)
            if len(ents) >= 2 and len(ents) > len(out.get(v, ())):
                out[v] = ents
        return out

    def plain_twin(self, kind, t5, forced=True):
        """the family deriver's twin of an object type (the same code to its depth), FORCE_MOVED's moved (where `forced`),
        TYPE_AUDITS' BN6's own"""
        if (kind, t5) in TYPE_AUDITS:
            return dict(bn6=t5, routine=None, why='BN6\'s own, audited: ' + TYPE_AUDITS[(kind, t5)])
        out = self.same_type(kind, t5)
        if forced and (kind, t5) in FORCE_MOVED and out['routine']:
            return dict(bn6=None, routine=out['routine'], why='moved: ' + FORCE_MOVED[(kind, t5)])
        return out

    def type_twin(self, kind, t5):
        """plain_twin's, and moved where BN6's twin, the same code, would spawn BN6's own of a type whose code is not
        BN5's (MetaGel's thrower and its gel: the gel's sprite another in BN6); a type a helper moves, none of the unit's"""
        if (kind, t5) in HELPER_TYPES and self.helper_at and HELPER_TYPES[(kind, t5)] != getattr(self, 'label', None):
            return dict(bn6=None, routine=None, helper=HELPER_TYPES[(kind, t5)], why=f'moved by the helper {HELPER_TYPES[(kind, t5)]}')
        out = self.plain_twin(kind, t5)
        if out['bn6'] is not None and out['routine'] and self.spawns_moved(out['routine'], forced=False):
            return dict(bn6=None, routine=out['routine'], why='moved: BN6\'s twin would spawn BN6\'s own of a type whose code differs')
        return out

    def same(self, a5, a6, depth=3):
        """the pairs' deep_same, where a pair the project audited as serving (bn5_twins.AUDITS, 'ok') counts as the same: an
        object's end, its states' generic routines; and the sprites the two load by number each game's of the same bytes
        (the same code drawing another picture is not the same: CrakOut's crack, list 5's 0xE, another in BN6)"""
        audit = self.pairs.audit(a5, a6)
        if audit and audit[0] == 'ok':
            return True
        n5, w5 = T.norm(self.b5, a5)
        n6, w6 = T.norm(self.b6, a6)
        if n5 != n6 or not self.pairs.bls_agree(w5, w6):
            return False
        l5, l6 = sprite_loads(w5, 5), sprite_loads(w6, 6)
        if len(l5) != len(l6) or not all(same_sprite(self.b5, self.b6, x, y) for x, y in zip(l5, l6)):
            return False
        if not same_data(self.b5, self.b6, w5, w6):
            return False
        if depth <= 0:
            return True
        t5, t6 = self.pairs.tables_of(self.b5, w5), self.pairs.tables_of(self.b6, w6)
        return len(t5) == len(t6) and all(len(x) == len(y) and all(self.same(p, q, depth - 1) for p, q in zip(x, y))
                                          for x, y in zip(t5, t6))

    def same_type(self, kind, t5):
        """the family deriver's type_twin by `same`: BN6's type of the same number first, then any"""
        tab5, n5 = BN5['types'][kind]
        if t5 >= n5:
            return dict(bn6=None, routine=None, why=f'type {t5:#x} past BN5\'s {n5}')
        r5 = self.b5.u32(tab5 + 4 * t5)
        if not T.is_code_ptr(r5, self.b5):
            return dict(bn6=None, routine=None, why='moved: no BN6 type runs the same code')
        tab6, n6 = BN6['types'][kind]
        for t6 in [t5] + [t for t in range(n6) if t != t5]:
            if t6 >= n6:
                continue
            r6 = self.b6.u32(tab6 + 4 * t6)
            if r6 and T.is_code_ptr(r6, self.b6) and self.same(r5 & ~1, r6 & ~1):
                return dict(bn6=t6, routine=r5 & ~1, why=f'BN6\'s type {t6:#x} runs the same code')
        return dict(bn6=None, routine=r5 & ~1, why='moved: no BN6 type runs the same code')

    def walk_unit(self, f, roots):
        f.states5, f.hooks5 = [], []
        f.own = set(roots)
        self.walk_family(f, extra=roots)


DATA_BYTES = 16   # (the bytes of a table a routine's literal names that its twin's must share: same_data)


def same_data(b5, b6, w5, w6):
    """the data the two routines' literals name, in their order, the same bytes where it is plain data in each game's ROM
    (no word of it a pointer, which each game's own places tell apart): DrkSonic's sonic's times by kind, BN5 0x080D5498
    (60, 60, 60, 60, 120), its twin's 0x080D5B38 another table, the same code reading it (0x080D547E, 0x080D5B20)"""
    def data(rom, v):
        if v is None or v & 3 or not 0x08000000 <= v < 0x0A000000 or not rom.has(v, DATA_BYTES):
            return None
        b = rom.bytes(v, DATA_BYTES)
        words = [int.from_bytes(b[k:k + 4], 'little') for k in range(0, DATA_BYTES, 4)]
        return None if any(0x02000000 <= x < 0x0A000000 for x in words) else b
    v5 = [w5.lits[k] for k in sorted(w5.lits)]
    v6 = [w6.lits[k] for k in sorted(w6.lits)]
    for x, y in zip(v5, v6):
        dx, dy = data(b5, x), data(b6, y)
        if dx is not None and dy is not None and dx != dy:
            return False
    return True


def used_as_table(w, lit):
    """the literal's register indexed by a word load (ldr rd,[rX,rY] or [rY,rX]) within the next few instructions"""
    addrs = sorted(w.insns)
    for k, a in enumerate(addrs):
        i = w.insns[a]
        if i.lit != lit:
            continue
        r = i.f[0]
        for b in addrs[k + 1:k + 6]:
            j = w.insns[b]
            if j.form == 'ldr_r' and r in j.f[1:]:
                return True
    return False


def chip_family(f):
    """a Family-shaped record of a unit's walk (no region, no states, no records)"""
    f.region = (0, 0)
    f.st, f.s1, f.s2 = 0, 0, 0
    f.ids5 = [None] * 6
    return f


# The DarkChips whose attacks the moved code plays (BN5's ids), their rules S7's: those BN6's twin family cannot (the rest
# play over their twin's, the registry's XREG_DARK rows)
DARK_MOVED = {
    187: "DarkCirc: its own support routine (BN5's 0x2A), which no BN6 chip's runs",
    192: "DarkWide: its graphics BN5's (the spread family's subfamily 2)",
    195: "DarkMetr: its meteors' controller (the misc family's subfamily 0x11, Astroid's)",
    197: "DarkTorn: BN5's own Tornado family (0x22, its subfamily 3: its tornadoes by the side's battle counter 3, its flash 0x17), "
         "where BN6's twin family 0x2F draws its own flash (chip_darktorn)",
    196: "DarkDril: BN5's own drill (its family 0x4F's subfamily 1, moved for DrilArm1-3: BN6's twin family 0x2E winds up 9 frames "
         "longer, chip_drilarm1)",
    198: "DrkSonic: its own support routine (BN5's 0x1B)",
}

# BN5's chips the registry pairs by name with one of BN6's (XREG_SAME) whose code is BN5's own and BN6's another: where BN5's
# ROM is read, BN6's record of the name takes the moved family and subfamily, and BN5's parameters and effect flags (src/core/
# chips.c, records_make), as an own id's record does
SAME_MOVED = {
    190: "DarkPlus: BN5's misc family's subfamily 3 (0x0800E190) stores its power in the next chip of the hand where that chip "
         "is an attack (and flashes him green 24 frames); BN6's 290 runs BN6's subfamily 3 (0x08010488), which lacks the hand's "
         "branch and puffs (chip_darkplus)",
}


def played_subs(b5, n):
    """the subfamilies of BN5's attack family `n` the registry's rows play by its moved code (moved, or still waiting for
    it, a Program Advance's waiting for its rule, a DarkChip of DARK_MOVED, or one of BN6's names SAME_MOVED)"""
    return {record(b5, BN5_RECORDS, bn5)[0x0C] for bn5, _, how in registry_rows()
            if (how in ('MOVED', 'PENDING', 'PA') or (how == 'DARK' and bn5 in DARK_MOVED) or (how == 'SAME' and bn5 in SAME_MOVED))
            and record(b5, BN5_RECORDS, bn5)[0x0B] == n}


def unplayed_of(b5, kind, n, label):
    """SUB_TABLES' entries of the unit's subfamilies none of whose rows it plays: BN5's routine: why"""
    out, keep = {}, set()
    subs = played_subs(b5, n) if kind == 'attack' else set()
    for table, (rows, why) in SUB_TABLES.get(label, {}).items():
        for k in range(rows):
            x = b5.u32(table + 4 * k) & ~1
            if k in subs:
                keep.add(x)
            else:
                out[x] = f'{why}: subfamily {k:#x}, which no chip it plays has (BN6\'s own chips run BN6\'s code)'
    return {x: why for x, why in out.items() if x not in keep}


# The tracks whose copies of a moved type install before the chips' code (src/emu/xinstall.c, xinstall_core: the
# moved objects and the Navis, then the chips' code, then the families): a type tools/bn5_twins.py's TYPES gives one of
# them is spawned as its copy (XV_TYPE_SHARED), none of its code copied with a unit
BEFORE_CHIPS = ('navis',)


def moved_before_chips():
    """the moved types whose copy installs before the chips' code: (kind, BN5's type) -> the track"""
    return {key: rec[2] for key, rec in bn5_twins.TYPES.items() if rec[0] == 'moved' and rec[2] in BEFORE_CHIPS}


# The Navis' code room (src/emu/xnavi_recipe.h: XN_CODE, from XN_OBJECTS_END, to XN_ART)
XN_CODE, XN_ART = OB.OBJECTS_END, EMU_FREE + 0x5B0000
_navi_modules = None


def type_closure(b5, pairs, entry, lo, hi):
    """the routines an object type's code reaches within [lo, hi) from its routine: its calls and its tables' entries"""
    seen, todo = set(), [entry]
    while todo:
        a = todo.pop()
        if a in seen or not lo <= a < hi:
            continue
        seen.add(a)
        w = T.walk(b5, a)
        todo += [x & ~1 for x in w.bls.values()]
        todo += [x & ~1 for ents in pairs.tables_of(b5, w) for x in ents]
    return seen


def navi_modules(b5, pairs):
    """the Navis' copies of the types they move (src/emu/xnavi_bn5.c, tools/xnavirecipe.py's): each a block of a Navi's
    recipe that holds a moved type's routine, the routines of the type's code there its module: {(kind, type): (BN5's
    start, end, BN6's place, its routines)}"""
    global _navi_modules
    if _navi_modules is None:
        import re
        with open(NAVIS_C, encoding='utf-8') as f:
            text = f.read()
        _navi_modules = {}
        for name in re.findall(r'static const XvType (\w+)_types\[\]', text):
            blocks = [(int(a, 16), int(b, 16), XN_CODE + int(o, 16)) for a, b, o in
                      re.findall(r'\{ 0x([0-9A-F]{8}), 0x([0-9A-F]{8}), XN_CODE \+ 0x([0-9A-F]+), 0x[0-9A-F]+u \}',
                                 text[text.index(f'static const XvBlock {name}_blocks[]'):text.index(f'static const XvType {name}_types[]')])]
            body = re.search(r'static const XvType ' + name + r'_types\[\] = \{(.*?)\};', text).group(1)
            for k, t5 in re.findall(r'\{ (\d), 0x([0-9A-F]{2}), XV_TYPE_MOVED \}', body):
                tab, n = BN5['types'][int(k)]
                r5 = b5.u32(tab + 4 * int(t5, 16)) & ~1
                blk = next((x for x in blocks if x[0] <= r5 < x[1]), None)
                if blk:
                    _navi_modules[(int(k), int(t5, 16))] = blk + (type_closure(b5, pairs, r5, blk[0], blk[1]),)
    return _navi_modules


def derive(b5, b6, pairs, kind, n, label, alloc, number, helper_at=None):
    d = ChipDeriver(b5, b6, pairs, moved_before_chips())
    d.navi_modules = navi_modules(b5, pairs)
    # (a number its code keeps is not audited by bn5_twins.KEPT, the families' rule: the chips' units are proven frame for
    # frame by tools/xdiff.py instead, as the Navis' are)
    d.audit_numbers = False
    d.helper_at = dict(helper_at or {})
    d.unplayed = unplayed_of(b5, kind, n, label)
    d.label = label
    f = chip_family(Family(-1))
    f.name = label
    entry = entry_of(b5, kind, n)
    roots = [entry] + (list(n[1:]) if kind == 'helper' and isinstance(n, tuple) else [])
    # (the types it spawns as a Navi's copy, a first walk's: their routines its code calls go to that copy, navi_modules)
    d.navi_routing = set()
    if any(k in d.moved_before for k in d.navi_modules):
        d.walk_unit(f, roots)
        d.navi_routing = {k for k, v in f.types.items() if v.get('shared') and k in d.navi_modules}
        f = chip_family(Family(-1))
        f.name = label
    d.walk_unit(f, roots)
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t in d.unplayed:
                f.decide('subfamily', f'{site:08X} calls {t:08X}, an unplayed subfamily\'s routine (SUB_TABLES)')
    blocks = d.blocks_of(f)
    for _ in range(4 if kind != 'helper' else 0):   # (a helper's gaps copied raw, none walked: gap_spans below)
        more = gap_roots(b5, f, blocks)
        if not more:
            break
        roots += more
        f = chip_family(Family(-1))
        f.name = label
        d.walk_unit(f, roots)
        blocks = d.blocks_of(f)
    # (a gap no routine of a push {..., lr} starts in, a leaf's: its bytes copied as they are, raw, so one block holds the
    # load and its pool: the muzzle flash's, 0x080B9CD6-0x080B9D04)
    d.gaps = gap_spans(f, blocks)
    if d.gaps:
        blocks = d.blocks_of(f)
    f.data = d.classify(f, blocks)
    # (a table's word aimed at a twin whose body differs serves only where audited: the symbol map's pair of BN5's
    # 0x0800A2F0 was another routine, NrthWind's wind never ending)
    for v, (to, why, tw) in sorted(f.words.items()):
        if tw and tw[2] == 'DIFF':
            audit = pairs.audit(v & ~1, tw[0])
            if not (audit and audit[0] == 'ok'):
                f.decide('pair', f'a table\'s word {v:08X} -> {tw[0]:08X} ({tw[1]}): the bodies differ ({tw[3]}) and no audit says it serves')
    d.scan(f)
    d.scan_words(f, blocks)
    for w in f.walks.values():
        for lit, v in w.lits.items():
            if lit in POWER_BASES and lit not in f.numbers:
                f.numbers[lit] = (v, v + POWER_FIRST, f'{POWER_BASES[lit]}: BN6\'s records\' {v + POWER_FIRST} (POWER_BASES)')
    u = Unit()
    u.f, u.kind, u.n5, u.label, u.entry, u.number, u.b5 = f, kind, n, label, entry, number, b5
    u.blocks = []
    where = 'hcode' if kind == 'helper' else 'code'
    at = alloc[where]
    for lo, hi in blocks:
        at = X.rounded(at, CODE_STEP) + (lo & 3)
        u.blocks.append((lo, hi, at, zlib.crc32(b5.bytes(lo, hi - lo)) & 0xFFFFFFFF))
        at += hi - lo
    alloc[where] = at
    if kind == 'helper' and at > XC_POWERS:
        f.decide('room', f'the helpers\' code runs to {at:08X}, past XC_POWERS')
    u.data = f.data
    X.calls_of(u, d, blocks)
    u.clones = []
    u.held_out = []
    u.more_imms = []
    chip_actions(u, d)
    u.sprites, u.effects, u.bytes, u.navis = [], [], [], set()
    chip_effect_calls(u)
    navi_ai_calls(u, b5, b6)
    art_of(u, b5, b6, alloc)
    numbered_calls(u)
    chip_panels(u)
    chip_hit_stats(u)
    chip_points(u, b5, b6)
    chip_coll_sums(u)
    for key in [k for k, v in f.types.items() if v.get('helper')]:
        f.notes.append(f'spawns the {kind_name(key[0])} kind\'s {key[1]:#x}: {f.types.pop(key)["why"]}')
    for key in sorted(f.needs):
        # (a type of the moved objects its code reaches through their spawner: run as their copy, which the install checks is there)
        f.types.setdefault(key, dict(bn6=None, routine=None, shared='the moved objects',
                                     why='the moved objects\' (tools/bn5_objects.py), installed before the chips\' code: their copy run'))
    X.types_of(u)
    chip_sounds(u, b5, b6)
    X.conditions_of(u, pairs)
    u.imms = X.unique_imms(f.imms + u.more_imms)
    u.words = sorted((v, to) for v, (to, why, tw) in f.words.items())
    u.numbers = sorted((lit, was, now) for lit, (was, now, why) in f.numbers.items())
    return u


def far_of(i):
    """where an instruction reaches that its block must hold: a load's pool, a branch's target (the installer moves a block
    whole, its loads reading their own block's pools and its branches staying in it)"""
    if i.form == 'ldr_lit':
        return i.lit
    return i.target if i.form in ('b', 'bcc') else None


def gap_spans(f, blocks):
    """the gaps between a load and the pool it reads, or a branch and its target, where they still fall in two blocks"""
    def block(a):
        return next((k for k, (lo, hi) in enumerate(blocks) if lo <= a < hi), None)
    out = set()
    for w in f.walks.values():
        for a, i in w.insns.items():
            if far_of(i) is None:
                continue
            ka, kl = block(a), block(far_of(i))
            if ka is not None and kl is not None and ka != kl:
                out.add(tuple(sorted((blocks[ka][1], blocks[kl][0]))))
    return sorted(out)


def gap_roots(b5, f, blocks):
    """the routines between a load and the pool it reads, or a branch and its target, where they fall in two blocks (BN5
    put HotBody's pool past a routine of its own that no walk reaches, 0x080DA57E; CustSwrd's 0x080EC1D6 branches past
    0x080EC208 to its pop): walked as the unit's too, so one block holds them (far_of)"""
    def block(a):
        return next((k for k, (lo, hi) in enumerate(blocks) if lo <= a < hi), None)
    out = set()
    for w in f.walks.values():
        for a, i in w.insns.items():
            if far_of(i) is None:
                continue
            ka, kl = block(a), block(far_of(i))
            if ka is None or kl is None or ka == kl:
                continue
            lo, hi = sorted((blocks[ka][1], blocks[kl][0]))
            out |= {g for g in range(lo & ~1, hi, 2) if b5.u16(g) & 0xFF00 == 0xB500 and g not in f.walks}
    return sorted(out)


def chip_actions(u, d):
    """the CurActions a chip's code writes or compares (scan's virus rules are not a chip's): MegaMan's ready state, BN5's 6,
    is BN6's 8 (legit.json, maps, action); a family's number is its BN6 number where the unit moves it (a chip that starts
    another of BN5's families, one a unit of UNITS moves: Slasher's 0x49, its number given BN6's for it); anything else a
    decision"""
    f = u.f
    moved = {n for k, n, _ in UNITS if k == 'attack'}
    for e, w in f.walks.items():
        if 'virus' not in f.ctx.get(e, ()) or 'object' in f.ctx.get(e, ()):
            continue   # (an object's own routine, an entry of its states' table too: its CurAction its own phase, numbered alike)
        for a in sorted(w.insns):
            i = w.insns[a]
            if i.form == 'bl' and BN5_SET_ATTACK <= i.target < BN5_SET_ATTACK + 7 * 4 and not (i.target - BN5_SET_ATTACK) & 3:
                v, chain = T.back_value(w, a, 0)
                n = (i.target - BN5_SET_ATTACK) // 4
                if v in moved and len(chain) == 1 and chain[0][1] == 'movs':
                    f.decisions = [k for k in f.decisions if k != ('curaction', f'{a:08X}: setAttack{n} with the chip action {v:#x}')]
                    u.more_imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=0, rn=0, was=v, now=v + 0x50,
                                            why=f'setAttack{n} with BN5\'s family {v:#x}: its unit\'s BN6 number {v + 0x50:#x}'))
                    continue
                f.decide('curaction', f'{a:08X}: setAttack{n} with {v if v is None else hex(v)}: a chip that starts another attack')
            if i.form == 'strb' and i.f[1:] == (5, 9):
                v, chain = T.back_value(w, a, i.f[0])
                if v == 6 and len(chain) == 1 and chain[0][1] == 'movs':
                    u.more_imms.append(dict(at=chain[0][0], form='THUMB_MOVS', rd=i.f[0], rn=0, was=6, now=8,
                                            why='CurAction 6 (BN5\'s ready state) written: BN6\'s 8'))
                elif v is None or v >= 6:
                    f.decide('curaction', f'{a:08X}: CurAction written from {chain}')


# The song slots BN5's sounds take in BN6 where BN6 has another song at their number (issue #82's shared allocations: one
# BN5 sound one slot, whichever track copies it, the slot taken only where it still holds BN6's empty header; the
# sounds' slots 0x40-0x62, the fixed ones the plan's and its amendments', the chips' others from 0x52 down, each in the
# report): src/audio/xsong.c's xsong_install_at copies the song there as the units install, the immediates name the slot
SOUND_SLOTS = {
    0xBC: 0x55, 0xB7: 0x61, 0x6F: 0x54, 0xE4: 0x53, 0x70: 0x5F, 0xBA: 0x60, 0xC0: 0x62,   # (the plan's)
    0x105: 0x59, 0x115: 0x5A, 0x117: 0x5E, 0x119: 0x4D, 0x8E: 0x4C,                        # (its second amendment's)
    0xA6: 0x51, 0xC6: 0x50, 0xBD: 0x4F, 0x10E: 0x4E, 0x12E: 0x52,                          # (the chips' own, down: 0x4B next)
    0xE1: 0x5B,                                                                            # (the Navis' ToadMan's, theirs)
}


# Sounds a unit's code reads from a table of its own: BN5's call site: (the table, a row's size, the sound's offset, its
# rows, its width: 1 a byte, 2 a halfword); each row's sound BN6's own where the same song, else its slot (SOUND_SLOTS) set
# in its bytes, 0 none
TABLE_SOUNDS = {
    0x080E395A: (0x080E39E8, 2, 0, 5, 2),   # the Anti chips' counter's, by its kind (+0xE): none, 0x10F, 0x11D, 0x12E, 0xFF
    0x080CEE9A: (0x080CEEB0, 2, 0, 4, 2),   # the seeds' (the bombs' 0x9, 0xC-0xE), by kind: 0x118, 0xE1, 0x11B, 0x11C
    0x080C93B4: (0x080C9414, 1, 0, 4, 1),   # the crack bombs' bursts (0xA), by kind (+4 & 3): 0x70, 0x97, 0x90, 0x90
    0x080C6DE2: (0x080C6C08, 6, 4, 20, 2),  # the spread shot's burst by kind (its row's halfword at +4): 0xAF, 0x11D, 0x10F
    0x080D542A: (0x080D5474, 2, 0, 5, 2),   # DrkSonic's sonic by kind (+4): 0xA8, 0xA9, 0xAA, 0xAB, 0x14A
}


def table_sounds(u, b5, b6, site):
    """TABLE_SOUNDS' rows at `site`: each its sound in BN6's terms (u.bytes), its song copied where BN6's differs"""
    table, size, off, rows, width = TABLE_SOUNDS[site]
    for k in range(rows):
        at = table + size * k + off
        v = b5.u16(at) if width == 2 else b5.u8(at)
        if any(x[0] == at for x in u.bytes):
            continue   # (a table two sites read, its rows given once)
        if not v:
            continue
        same = X.song_sig(b5, BN5['songs'], v, True) == X.song_sig(b6, BN6['songs'], v, True)   # (its music player too: the chips' copy BN5's own)
        u.sounds.append((v, same))
        if same:
            continue
        slot = SOUND_SLOTS.get(v)
        if slot is None:
            u.f.decide('sound', f'{table:08X}\'s row {k}: its sound {v:#x} is another song in BN6 and has no slot (SOUND_SLOTS)')
            continue
        u.songs.append((v, slot))
        why = f'{table:08X}\'s row {k}: its sound {v:#x}, BN5\'s copied to BN6\'s slot {slot:#x}'
        u.bytes.append((at, v & 0xFF, slot, why))
        if v >> 8:
            u.bytes.append((at + 1, v >> 8, 0, why + ' (its high byte)'))


def chip_sounds(u, b5, b6):
    """the sounds its code starts: BN6's own where its song of the number is BN5's, else BN5's copied to its slot
    (SOUND_SLOTS), the immediates that name it given the slot (movs r,#n; or movs r,#n-0xFF and adds r,#0xFF: then the slot
    and 0); a table's (TABLE_SOUNDS) in its bytes"""
    f = u.f
    u.sounds, u.songs = [], []
    for v, sites in sorted(f.sounds.items(), key=lambda x: str(x[0])):
        if v is None:
            for s, _ in sites:
                if s in TABLE_SOUNDS:
                    table_sounds(u, b5, b6, s)
            if any(s not in TABLE_SOUNDS for s, _ in sites):
                f.decide('sound', f'a sound made otherwise at {", ".join(f"{s:08X}" for s, _ in sites if s not in TABLE_SOUNDS)}')
            continue
        same = X.song_sig(b5, BN5['songs'], v, True) == X.song_sig(b6, BN6['songs'], v, True)   # (its music player too: the chips' copy BN5's own)
        u.sounds.append((v, same))
        if same:
            continue
        slot = SOUND_SLOTS.get(v)
        if slot is None:
            f.decide('sound', f'its sound {v:#x} is another song in BN6 and has no slot of its own (SOUND_SLOTS)')
            continue
        u.songs.append((v, slot))
        for site, chain in sites:
            sound_imms(u, b5, chain, v, slot)


def sound_imms(u, b5, chain, v, slot):
    why = f'its sound {v:#x}: BN5\'s copied to BN6\'s slot {slot:#x}'
    movs = [(a, x) for a, form, x in chain if form == 'movs']
    adds = [(a, x) for a, form, x in chain if form == 'adds8']
    lo = v if v <= 0xFF else v - 0xFF
    if len(movs) != 1 or movs[0][1] != lo or (v > 0xFF and (len(adds) != 1 or adds[0][1] != 0xFF)) or (v <= 0xFF and adds):
        u.f.decide('immediate', f'{why}: made by {chain}, not movs (and adds #0xFF)')
        return
    a = movs[0][0]
    u.more_imms.append(dict(at=a, form='THUMB_MOVS', rd=(b5.u16(a) >> 8) & 7, rn=0, was=lo, now=slot, why=why))
    if adds:
        a = adds[0][0]
        u.more_imms.append(dict(at=a, form='THUMB_ADDS', rd=(b5.u16(a) >> 8) & 7, rn=0, was=0xFF, now=0, why=why + ' (its high part)'))


# BN5's panel types BN6 numbers otherwise: its lava and sea BN6's 13 and 14 where src/emu/xpanels.c brings them in
# (XPANEL_LAVA, XPANEL_SEA), its holy BN6's 5 (BN6_PANEL_HOLY); the panel getter (object_getPanelDataOffset) and setter
PANEL_TYPES = {0x8: 0xD, 0x9: 0x5, 0xA: 0xE}
PANEL_GETTER, SET_PANEL_TYPE = 0x0800AFC2, 0x0800B2A6
PANEL_SAME = 0x10   # (a panel's first 0x10 bytes laid out alike in both: legit.json's panel layout)


def X_u8(u, a):
    return u.b5.u8(a)


def bn5_twins_same_panels():
    import bn5_twins
    return bn5_twins.SAME_PANEL_TYPES


def panel_to6(off):
    """BN5's offset in a panel as BN6's (tests/xdiff/legit.json, layouts, panel: BN5's, BN6's, bytes), None where BN6 has none"""
    with open(os.path.join(ROOT, 'tests', 'xdiff', 'legit.json'), encoding='utf-8') as fh:
        for a, b, n in json.load(fh)['layouts']['panel']['segments']:
            if a <= off < a + n:
                return b + off - a
    return None


PANEL_FORMS = {'ldr': 'THUMB_LDR', 'str': 'THUMB_STR', 'ldrb': 'THUMB_LDRB', 'strb': 'THUMB_STRB', 'ldrh': 'THUMB_LDRH',
               'strh': 'THUMB_STRH'}


def chip_panels(u):
    """the panel types its code sets (setPanelType's r2) or compares with a panel's (its +2, after the getter): BN6's
    numbers for BN5's lava, sea and holy; and what it reads of a panel the getter gives: within its first 0x10 bytes (the
    getter's audit) as it is, past them at BN6's place (legit.json's panel layout: the flags BN5 +0x18, BN6 +0x14), else
    a decision"""
    f = u.f
    for w in f.walks.values():
        addrs = sorted(w.insns)
        for k, a in enumerate(addrs):
            i = w.insns[a]
            if i.form != 'bl':
                continue
            if i.target == SET_PANEL_TYPE:
                v, chain = T.back_value(w, a, 2)
                if v in PANEL_TYPES:
                    X.imm_for(u, chain, 2, v, PANEL_TYPES[v], f'a panel set to BN5\'s type {v:#x}: BN6\'s {PANEL_TYPES[v]:#x}')
            if i.target != PANEL_GETTER:
                continue
            regs = {0}
            for n, b in enumerate(addrs[k + 1:k + 12]):
                j = w.insns[b]
                if j.form in PANEL_FORMS and len(j.f) == 3 and j.f[1] in regs and j.f[2] >= PANEL_SAME:
                    to = panel_to6(j.f[2])
                    if to is None:
                        f.decide('panel', f'{b:08X}: a panel\'s +{j.f[2]:#x} read or written, which BN6 has no place for')
                    else:
                        u.more_imms.append(dict(at=b, form=PANEL_FORMS[j.form], rd=j.f[0], rn=j.f[1], was=j.f[2], now=to,
                                                why=f'a panel\'s +{j.f[2]:#x} in BN5, +{to:#x} in BN6 (legit.json, layouts, panel)'))
                if j.form == 'ldrb' and j.f[1:] == (0, 2):
                    for c in addrs[k + 2 + n:k + 5 + n]:
                        x = w.insns[c]
                        if x.form == 'cmp' and x.f[0] == j.f[0] and x.f[1] in PANEL_TYPES:
                            u.more_imms.append(dict(at=c, form='THUMB_CMP', rd=x.f[0], rn=0, was=x.f[1], now=PANEL_TYPES[x.f[1]],
                                                    why=f'a panel\'s type compared with BN5\'s {x.f[1]:#x}: BN6\'s {PANEL_TYPES[x.f[1]]:#x}'))
                if j.form == 'bl' or j.form in ('pop', 'bx'):
                    break
                if T.writes(j, 0) and j.form not in ('ldrb', 'ldrh', 'ldr'):
                    regs.discard(0)
    # (types set from a table's rows, TABLE_PANELS: each row's BN5's lava, sea or holy given BN6's number)
    for site in sorted({a for a, v in f.panel_types if v is None and a in TABLE_PANELS}):
        table, size, off, rows = TABLE_PANELS[site]
        for k in range(rows):
            at = table + size * k + off
            v = X_u8(u, at)
            if v in PANEL_TYPES:
                u.bytes.append((at, v, PANEL_TYPES[v], f'{table:08X}\'s row {k}: a panel set to BN5\'s type {v:#x}: BN6\'s {PANEL_TYPES[v]:#x}'))
            elif v in PANELS_ABSENT and site in TABLE_PANEL_ROWS and k not in TABLE_PANEL_ROWS[site]:
                f.notes.append(f'{table:08X}\'s row {k}: {PANELS_ABSENT[v]} (BN5\'s {v:#x}, which BN6 lacks), a row no moved unit reads')
            elif v != 0xFF and v not in bn5_twins_same_panels():
                u.f.decide('panel', f'{table:08X}\'s row {k}: a panel set to type {v:#x}, numbered otherwise in BN6')
    covers = lambda a: any(lo <= a < hi for lo, hi, *_ in u.blocks)
    for table, (size, off, rows, why) in sorted(PANEL_LOOKUPS.items()):
        if not covers(table):
            continue
        for k in range(rows):
            at = table + size * k + off
            v = X_u8(u, at)
            now = PANEL_TYPES.get(v, ABSENT_PANEL if v in PANELS_ABSENT else None)
            if now is not None:
                u.bytes.append((at, v, now, f'{why}: row {k}, BN5\'s type {v:#x} ' +
                                (f'BN6\'s {now:#x}' if v in PANEL_TYPES else f'({PANELS_ABSENT[v]}), which BN6 lacks: none')))
            elif v != 0xFF and v not in bn5_twins_same_panels():
                f.decide('panel', f'{table:08X}\'s row {k}: a panel compared with type {v:#x}, numbered otherwise in BN6')
    for a, v in f.panel_types:
        if v is None and a in PANEL_SITES:
            f.notes.append(f'{a:08X}: {PANEL_SITES[a]}')
    f.panel_types = [(a, v) for a, v in f.panel_types if v not in PANEL_TYPES and not (v is None and (a in TABLE_PANELS or a in PANEL_SITES))]


# The engine's hit area's spawners (r7: its hit's words, a byte each: +0x60, +0x64, +0x68 the hit's effect, +0x6C its
# parameter, the effect and parameter set into the hit's collision data +0x12, +0x13 by 0x080172CE, BN6 0x0801A4D0)
HIT_SPAWNERS = (0x080C4E72, 0x080C4EAE)
# The hit effects that change a stat of the struck, each game its own rule (docs/ROM_DATA.md): 0x18 and 0x19, BN5's
# 0x08011036 (a Navi's NaviStats +0x18 and +0x19, its bugs: the parameter's low nibble added where its 0x10 is set,
# taken away where its 0x20 is, else raised to where lower) and nothing for a virus; BN6's 0x080139F6 (the Navi's stat
# plus the parameter, at most 7) and 0x08013B20 (0x18 on a virus: its AI data +0x12 plus the parameter, at most 7, from
# its pre-step 0x0801AB40)
HIT_STATS = (0x18, 0x19)


def chip_hit_stats(u):
    """the hit words a unit's code gives the hit area: a stat effect's parameter in BN6's terms (BN5's add as the amount;
    its raise-to as 0, a virus untouched as BN5 leaves it, a Navi's stat no longer raised: docs/MULTIROM.md; its subtract a
    decision)"""
    f = u.f
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t not in HIT_SPAWNERS:
                continue
            # (each literal r7 is loaded from before the call, a branch's other value of it beside: the moon blade's)
            before = sorted((a for a in w.insns if site - 0x20 <= a < site), reverse=True)
            lits = [a for a in before if w.insns[a].form == 'ldr_lit' and w.insns[a].f[0] == 7]
            if not lits:
                continue
            lit = w.insns[lits[0]].lit
            v = w.lits.get(lit)
            if v is None:
                continue
            effect, p = (v >> 16) & 0xFF, v >> 24
            if effect not in HIT_STATS or not p:
                continue
            if p & 0x20:
                f.decide('hit', f'{site:08X}: its hit takes {p & 0xF} from stat {effect:#x} (BN5\'s 0x20), which BN6\'s rule cannot')
                continue
            now = (v & 0x00FFFFFF) | ((p & 0xF) << 24 if p & 0x10 else 0)
            f.numbers[lit] = (v, now, f'its hit\'s stat effect {effect:#x}, parameter {p:#x} by BN5\'s rule, {now >> 24:#x} by BN6\'s')


# The points of an object's sprite (BN5 0x08015140, BN6's twin 0x08018810: r0 its NameID, r1 the point's number, back (x, y)
# from each game's own rows, BN5's 30 points a row at 0x080151C4 on, BN6's 34 at 0x080188C0, MegaMan's first): a point a
# unit asks for is BN6's of the same number where MegaMan's rows give the same, else POINTS names BN6's (each checked)
POINT_OF = 0x08015140
POINT_ROW = 0x08015164   # (its inner part, called direct too: r0 and r1 the row's category and number, r2 the point)
POINT_ROWS = {'bn5': (0x080151C4, 0x3C), 'bn6': (0x080188C0, 0x44)}
POINTS = {
    0x1A: (0x1B, "CusVolt's volt's place (0x080DD58E, at his centre, 18 up, 40 pixels ahead): BN5's 0x1A (0, 18) on every row "
                 "of his; BN6's 0x1A another (28, 25), no BN6 code asking for it, its 0x1B the centre (2, 18), as every point of "
                 "his BN6 put 2 pixels further right (0x00 (2, 0) against (0, 0))"),
}


# Offsets into an object's collision data a unit's code adds as a number, the pointer made by adding the data's address
# (the object's +0x54) after: BN5's instruction: why; the number BN6's place for it (xrecipe_family's coll_to6, the
# layout legit.json pairs: BN5 +0x74-0x9F is BN6 +0x7C-0xA7), checked to be added to the +0x54 load within four
COLL_SUMS = {
    0x080CF6CC: "the Anti chips' trap (the third kind's 0x4D): the struck's damage by element, a halfword each from Fire on, "
                "BN5's +0x7C + 2 x its kind (0x080CF6C8-0x080CF6D2), BN6's from +0x84 (its own trap's 0x080CF844, from Fire)",
}


def chip_coll_sums(u):
    """COLL_SUMS: each instruction's number given BN6's place, where it is the unit's and adds to a +0x54 load"""
    f = u.f
    for w in f.walks.values():
        for a in sorted(set(w.insns) & set(COLL_SUMS)):
            i = w.insns[a]
            nxt = [w.insns[b] for b in sorted(w.insns) if a < b <= a + 8]
            coll = {j.f[0] for j in nxt if j.form == 'ldr' and j.f[2] == 0x54}
            to = F.coll_to6(i.f[1]) if i.form == 'adds8' else None
            if to is None or not any(j.form == 'adds' and i.f[0] in j.f[1:] and set(j.f[1:]) & coll for j in nxt):
                f.decide('collision', f'{a:08X}: a collision data\'s offset added (COLL_SUMS), not as expected ({i.text})')
                continue
            u.more_imms.append(dict(at=a, form='THUMB_ADDS', rd=i.f[0], rn=0, was=i.f[1], now=to,
                                    why=f'collision data +{i.f[1]:#x} in BN5, +{to:#x} in BN6: {COLL_SUMS[a]}'))


def chip_points(u, b5, b6):
    """the points of MegaMan's sprite its code asks for (POINT_OF's r1): BN6's own where its first row gives the same, BN6's
    of POINTS where it names one (its row checked), else a decision"""
    f = u.f
    def point(rom, game, k):
        row, size = POINT_ROWS[game]
        x, y = rom.u8(row + 2 * k), rom.u8(row + 2 * k + 1)
        return (x - 256 if x > 127 else x, y - 256 if y > 127 else y)
    for w in f.walks.values():
        for site, t in sorted(w.bls.items()):
            if t not in (POINT_OF, POINT_ROW) or t in UNIT_STUBS.get(f.name, {}):
                continue   # (a call the unit's stub answers: its numbers BN5's, UNIT_STUBS)
            reg = 1 if t == POINT_OF else 2
            v, chain = T.back_value(w, site, reg)
            if v is None:
                continue   # (a number read from a table: the muzzle flash's, its entry's byte 4; legit.json's mask)
            if v in POINTS:
                now, why = POINTS[v]
                if abs(point(b6, 'bn6', now)[0] - point(b5, 'bn5', v)[0]) > 2 or point(b6, 'bn6', now)[1] != point(b5, 'bn5', v)[1]:
                    f.decide('point', f'{site:08X}: point {v:#x} {point(b5, "bn5", v)} as BN6\'s {now:#x} {point(b6, "bn6", now)}')
                X.imm_for(u, chain, reg, v, now, f'MegaMan\'s point {v:#x}: {why}')
            elif point(b5, 'bn5', v) != point(b6, 'bn6', v):
                f.decide('point', f'{site:08X}: MegaMan\'s point {v:#x}, BN5\'s {point(b5, "bn5", v)}, BN6\'s {point(b6, "bn6", v)} (POINTS)')


# Routines whose argument is a number of a table a helper moves with it (BN5's numbers, BN5's table): the muzzle flash's
# spawner, its number r4's low byte, its table's entries each its sprite's list x 4 and number first
NUMBERED = {OB.IMAGE['spawner']: ('a muzzle flash', OB.IMAGE['kinds'], 5, 4)}


def other_movs(w, at, reg):
    """the numbers a register is given before `at` that a branch around it may leave (movs r4,#0x11, then movs r4,#0x37
    where the subfamily is 2: the spread family's flash at 0x080ED0BE-0x080ED0C6): each movs of it the walk meets going
    back from `at` past compares and branches, up to its last call or other write of it"""
    out = set()
    for a in sorted((x for x in w.insns if x < at), reverse=True)[:8]:
        i = w.insns[a]
        if i.form == 'bl' or is_return(i):
            break
        if i.form == 'movs' and i.f[0] == reg:
            out.add(i.f[1] & 0xFF)
        elif T.writes(i, reg):
            break
    return out


def is_return(i):
    return i.form == 'pop' and 'pc' in i.text or i.form == 'bx'


def numbered_calls(u):
    """each call of a NUMBERED routine: the numbers it may give (its r4's low byte by one instruction, or FLASH_NUMBERS), kept
    on the unit (u.flash) for its helper's table; else a decision"""
    f = u.f
    u.flash = set()
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t not in NUMBERED:
                continue
            what, table, size, reg = NUMBERED[t]
            v, chain = T.back_value(w, site, reg)
            if site in FLASH_NUMBERS:
                u.flash |= set(FLASH_NUMBERS[site][0])
            elif v is not None and len(chain) == 1:
                u.flash.add(v & 0xFF)
                u.flash |= other_movs(w, chain[0][0], reg)
            elif len(chain) >= 2 and chain[0][1] == 'adds8' and chain[1][1] == 'lsls' and (chain[1][2] or 0) >= 8:
                u.flash.add(chain[0][2])   # (its low byte an immediate added to a value shifted past it: ElcReel's 0x35)
            else:
                f.decide('number', f'{site:08X}: {what} of a number made otherwise ({chain}): FLASH_NUMBERS')


NAVIS_C = os.path.join(ROOT, 'src', 'emu', 'xnavi_bn5.c')


def objects_art(b5, alloc):
    """the sprites the moved objects' copy places (tools/bn5_objects.py, IMAGE_COPIED: the image object's kinds' sprites,
    copied by the Navis' install of the moved objects, before the chips' code), and the Navis' shared art (src/emu/xnavi_bn5.c,
    tools/xnavirecipe.py's, installed before the chips' code too): each number taken from the chips' numbering (alloc's
    `reserved`), and BN5's sprite at its number where its list is BN5's (alloc's `memo`: a unit that loads it draws that
    copy, one BN5 sprite one number)"""
    import re
    places = []
    for k, (lst, num, _) in sorted(OB.IMAGE_COPIED.items()):
        at = OB.IMAGE['kinds'] + 5 * k
        places.append((b5.u8(at) // 4, b5.u8(at + 1), lst, num))
    with open(NAVIS_C, encoding='utf-8') as f:
        navis = f.read()
    for l5, i5, lst, num in re.findall(r'\{ (\d), 0x([0-9A-F]{2}), (\d), 0x([0-9A-F]{2}), XN_SHARED_ART \+', navis):
        places.append((int(l5), int(i5, 16), int(lst), int(num, 16)))
    for l5, i5, lst, num in places:
        alloc['reserved'].add((lst, num))
        if l5 == lst:
            alloc['memo'].setdefault((l5, i5), num)
    # (and the Navis' effects: one BN5 effect one BN6 number, its entry the Navis' install's, its sprite their art)
    for body in re.findall(r'static const XvId \w+_effects\[\] = \{(.*?)\};', navis):
        for e5, e6 in re.findall(r'\{ 0x([0-9A-F]{2}), 0x([0-9A-F]{2}) \}', body):
            alloc['effects'].setdefault(int(e5, 16), int(e6, 16))


def shared_rows(units, b5):
    """the rows of a type's own tables a later track spawns it with (tools/bn5_twins.py's SHARED_ROWS), fixed in each copy of
    them the chips make: the one copy every track runs"""
    for at, was, now, why in bn5_twins.shared_row_bytes(b5, 'chips'):
        for u in units:
            if any(lo <= at < hi for lo, hi, _, _ in u.blocks) and not any(x[0] == at for x in u.bytes):
                u.bytes.append((at, was, now, why))


def flash_kinds(helpers, units):
    """the image object's kinds the units spawn (its spawner's numbers, numbered_calls), each one tools/bn5_objects.py fixes
    in the moved objects' copy (its sprite BN6's twin or BN5's copied there, at a number of the chips' for the kinds only the
    chips spawn): a kind it leaves with BN5's bytes a decision of the first unit that spawns it"""
    fixed = set(OB.IMAGE_TWIN) | set(OB.IMAGE_COPIED)
    for u in helpers + units:
        for k in sorted(u.flash):
            if k >= OB.IMAGE['count']:
                u.f.decide('number', f'the image object\'s kind {k:#x}, past its table')
            elif k not in fixed:
                u.f.decide('image', f'its image kind {k:#x}: unfixed in tools/bn5_objects.py (its sprite BN5\'s bytes in the moved copy)')
        fixed |= u.flash


# BN5's routine that sets up an object's sprite and collision from two packed words (r0: its sprite's list x 4 << 8 | its
# number, its animation << 16; r1 its collision's), BN6's twin 0x08011504 the same: a sprite loaded by number all the same
SPRITE_SETUP = 0x0800F198
SPRITE_SETUP6 = 0x08011504
SPRITE_LOAD6 = 0x080026E4   # (BN6's sprite load, BN5's 0x08002650: r1 the list's offset, r2 the number)
BN5_SPRITE_LISTS = 0x0803272C
# The grown sprite lists (src/emu/xvirus_grow.c: 1, 2, 3, 4 and 5, 256 numbers each) the chips take numbers of, from the
# top down, where the families take theirs from the bottom up (tools/xrecipe.py, LIST_NUMBERS; list 2's 0x20-0x2F the
# Navis'): one numbering, each slot the install checks to hold the fill first; a sprite BN6 keeps another of at the number
# BN5's code loads is copied there. List 3, the chips' attacks, grown for every track by the chips' (XV_LIST3)
CHIP_LISTS = (1, 2, 3, 4, 5)
CHIP_LIST_TOP = 0xFF
SPRITE_ROOM = 0x800


def sprite_loads(w, game):
    """the sprites a walk's code loads by number, in address order: (its list's offset, its number), None where made
    otherwise (the loader's r1 and r2, or the packed word SPRITE_SETUP takes in r0)"""
    load, setup = (BN5['sprite_load'], (SPRITE_SETUP, 0x08018926)) if game == 5 else (SPRITE_LOAD6, (SPRITE_SETUP6, 0x0801BE2A))
    out = []
    for site in sorted(w.bls):
        t = w.bls[site]
        if t == load:
            l, _ = T.back_value(w, site, 1)
            v, _ = T.back_value(w, site, 2)
            out.append((l, v))
        elif t in setup:
            v, _ = T.back_value(w, site, 0)
            out.append(None if v is None else ((v >> 8) & 0xFF, v & 0xFF))
    return out


def same_sprite(b5, b6, x, y):
    """two loads (sprite_loads') the same picture: each game's sprite of its list and number the same bytes"""
    if x is None or y is None or None in x or None in y:
        return x == y
    _, s5 = X.sprite(b5, BN5_SPRITE_LISTS, x[0] // 4, x[1])
    _, s6 = X.sprite(b6, BN6['sprites'], y[0] // 4, y[1])
    return bool(s5) and s5 == s6


# BN5's other routine that sets up an object's sprite from a packed word (r0: its list x 4 << 8 | its number, a palette's
# << 24), BN6's twin 0x0801BE2A: MrkCan's mark's (0x1024)
SPRITE_SETUPS = (SPRITE_SETUP, 0x08018926)


# BN5's afterimage's spawner (the fourth kind's 0x28, 0x080E36DC): its caller's r4 the packed word its sprite is loaded by
# ([r5+4] its list x 4, [r5+5] its number at 0x080E3648; a palette added to its top byte). A spawn of it with a word of the
# unit's own (DarkWide's wave's, 0x080CCAA8: list 4's 0x1D, the wave's own sprite) loads that sprite, its number set to BN6's
# as any load's (chip_darkwide: BN6's afterimage drew BN6's own 4:0x1D, another, where the wave drew the chips' copy); a word
# made otherwise is MegaMan's own sprite (the swords', MEGAMAN_LOADS' 0x080E3648)
AFTERIMAGE_SPAWN = 0x080E36DC


def afterimage_word(w, site):
    """the literal word the afterimage's spawner at `site` takes in r4 (a palette added to its top byte after it, an adds
    r4,r4,rX, left as it is): (the word, its literal's address), else (None, None)"""
    for a in sorted((x for x in w.insns if x < site), reverse=True)[:14]:
        i = w.insns[a]
        if i.form in ('b', 'bx') or T.is_return(i):
            break
        if not T.writes(i, 4):
            continue
        if i.form == 'adds' and i.f[0] == 4 and i.f[1] == 4:
            continue
        if i.form == 'ldr_lit':
            return w.lits.get(i.lit), i.lit
        break
    return None, None


def packed_loads(f):
    """the sprites a unit's code sets up through SPRITE_SETUPS, as the family deriver's loads (its list's offset, its number),
    and those its spawns of the afterimage name (AFTERIMAGE_SPAWN)"""
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t == AFTERIMAGE_SPAWN:
                v, lit = afterimage_word(w, site)
                if v is not None:
                    f.loads.setdefault((v & 0xFF, (v >> 8) & 0xFF), []).append((site, [(None, 'afterimage', lit)]))
                continue
            if t not in SPRITE_SETUPS:
                continue
            v, chain = T.back_value(w, site, 0)
            key = (None, None) if v is None else ((v >> 8) & 0xFF, v & 0xFF)
            f.loads.setdefault(key, []).append((site, chain))


def load_rewrite(u, site, chain, was, now, why):
    """a load's number set to BN6's: the immediate that makes it, or the packed word's low byte in its literal"""
    w = next((w for w in u.f.walks.values() if site in w.bls), None)
    if w is not None and w.bls[site] == AFTERIMAGE_SPAWN:
        lit = chain[0][2]
        word = w.lits[lit]
        u.f.numbers[lit] = (word, (word & ~0xFF00) | now << 8, why)
        return
    if w is not None and w.bls[site] in SPRITE_SETUPS:
        lit = w.insns[chain[0][0]].lit if chain and chain[0][1] == 'ldr_lit' else None
        if lit is None:
            u.f.decide('sprite', f'{why}: its packed word made otherwise ({chain})')
            return
        word = w.lits[lit]
        u.f.numbers[lit] = (word, (word & ~0xFF) | now, why)
        return
    X.imm_for(u, chain, 2, was, now, why)


# The grown effects' ids the chips take (src/emu/xvirus_grow.c, XV_EFFECTS_MAX: BN6's 108 first, the families' from 0x6C up),
# from 0xFF down (issue #82's shared allocations: 0xD0-0xFF the chips')
CHIP_EFFECT_TOP, CHIP_EFFECT_BOTTOM = 0xFF, 0xD0


def effect_of(u, b5, b6, alloc, e):
    """BN5's effect `e` as BN6 has it: BN6's effect of the same sprite (by its bytes), animation and palette where it has one,
    else an id of the chips' (from 0xFF down) whose entry names BN5's sprite, copied (the entry and its sprite installed with
    the unit that places the sprite: xvirus.c's effects_set takes an effect's sprite from its own unit's)"""
    if e in alloc['effects']:
        return alloc['effects'][e]
    r5 = b5.bytes(BN5['effects'] + 4 * e, 4)
    lst, v = r5[0] // 4, r5[1]
    for e6 in range(BN6['neffects']):
        q = b6.bytes(BN6['effects'] + 4 * e6, 4)
        if q[2:] == r5[2:] and q[0] == r5[0] and same_sprite(b5, b6, (r5[0], r5[1]), (q[0], q[1])):
            alloc['effects'][e] = e6
            return e6
    if alloc['effect_next'] < CHIP_EFFECT_BOTTOM:
        u.f.decide('effect', f'its effect {e:#x}: the chips\' effect ids are all taken')
        return None
    if lst not in CHIP_LISTS:
        u.f.decide('effect', f'its effect {e:#x} draws sprite {lst}:{v:#x}, a list not grown')
        return None
    e6 = alloc['effect_next']
    alloc['effect_next'] -= 1
    alloc['effects'][e] = e6
    place_sprite(u, b5, alloc, lst, v)
    owner = alloc['owner'][(lst, v)]
    owner.effects.append((e, e6))
    return e6


# Effect ids a unit's code makes as one immediate plus the object's kind (base + kind, its kinds `count`): BN5's spawn
# site: (the immediate's instruction, the form, count, why); BN6's ids for base..base+count-1 one run of the same entries
# (BN6's own where it has them in a row, else the chips' own, taken as a run), the immediate given the run's first
EFFECT_RANGES = {
    0x080BA538: (0x080BA526, 'THUMB_ADDS', 3, "Colonel's soldiers' bursts: 0x48 + its +5 (0x080BA524-0x080BA528), list 4's "
                                              "0x1F, its animations 0-2"),
    0x080BC15E: (0x080BC144, 'THUMB_MOVS', 1, "TomahawkMan's swing's: 0x45 (0x080BC144-0x080BC146, added to the side's high "
                                              "bytes)"),
}


def effect_range(u, b5, b6, alloc, site):
    """EFFECT_RANGES' run at `site` in BN6's ids, its immediate given the run's first"""
    at, form, count, why = EFFECT_RANGES[site]
    base = b5.u16(at) & 0xFF
    rows = [b5.bytes(BN5['effects'] + 4 * (base + k), 4) for k in range(count)]
    same = lambda e6, k: (b6.bytes(BN6['effects'] + 4 * e6, 4) == rows[k]
                          and same_sprite(b5, b6, (rows[k][0], rows[k][1]), (rows[k][0], rows[k][1])))
    run = next((e6 for e6 in range(BN6['neffects'] - count + 1) if all(same(e6 + k, k) for k in range(count))), None)
    if run is None and base in alloc['effects'] and all(alloc['effects'].get(base + k) == alloc['effects'][base] + k for k in range(count)):
        run = alloc['effects'][base]   # (a run another track's install numbered so, before the chips': the Navis')
    if run is None:
        if alloc['effect_next'] - count + 1 < CHIP_EFFECT_BOTTOM:
            u.f.decide('effect', f'{site:08X}: a run of {count} effects, the chips\' effect ids all taken')
            return
        run = alloc['effect_next'] - count + 1
        alloc['effect_next'] -= count
        for k in range(count):
            lst, v = rows[k][0] // 4, rows[k][1]
            if lst not in CHIP_LISTS:
                u.f.decide('effect', f'its effect {base + k:#x} draws sprite {lst}:{v:#x}, a list not grown')
                return
            alloc['effects'][base + k] = run + k
            place_sprite(u, b5, alloc, lst, v)
            alloc['owner'][(lst, v)].effects.append((base + k, run + k))
    if run != base:
        rd = (b5.u16(at) >> 8) & 7
        u.more_imms.append(dict(at=at, form=form, rd=rd, rn=0, was=base, now=run,
                                why=f'its effects {base:#x}-{base + count - 1:#x} in BN5, {run:#x}-{run + count - 1:#x} in BN6: {why}'))


def effect_imm(u, b5, chain, was, now):
    """the immediate the effect id's chain sets (movs, or adds8 of a byte), given BN6's id"""
    why = f'its effect: {was:#x} in BN5, {now:#x} in BN6'
    for a, form, v in chain:
        if form in ('movs', 'adds8') and v == was:
            u.more_imms.append(dict(at=a, form='THUMB_MOVS' if form == 'movs' else 'THUMB_ADDS', rd=(b5.u16(a) >> 8) & 7, rn=0, was=was,
                                    now=now, why=why))
            return
    u.f.decide('immediate', f'{why}: made by {chain}, not one instruction')


def table_art(u, b5, b6, alloc, site):
    """a kind table's sprites (TABLE_SPRITES): each row's BN6's own where the same bytes, else BN5's copied and the row's
    number byte set to the copy's (u.bytes)"""
    table, size, lo, no, rows, fixed = TABLE_SPRITES[site]
    for k in range(rows):
        at = table + size * k
        lst, v = (b5.u8(at + lo) // 4 if lo is not None else fixed), b5.u8(at + no)
        if lo is not None and b5.u8(at + lo) == 0xFF:
            continue
        _, s5 = X.sprite(b5, BN5_SPRITE_LISTS, lst, v)
        _, s6 = X.sprite(b6, BN6['sprites'], lst, v)
        if s5 and s5 == s6:
            continue
        if lst not in CHIP_LISTS or not s5:
            u.f.decide('sprite', f'{table:08X}\'s row {k}: its sprite {lst}:{v:#x}, BN6\'s another, ' + ('its list not grown' if s5 else 'BN5\'s not read'))
            continue
        to = place_sprite(u, b5, alloc, lst, v)
        u.bytes.append((at + no, v, to, f'{table:08X}\'s row {k}: its sprite list {lst} {v:#x} in BN5, {to:#x} in BN6 (BN5\'s copied there)'))


def place_sprite(u, b5, alloc, lst, v):
    """BN5's sprite `lst`:`v` at a number of the chips' in its grown list (from the top down), copied with the unit that
    places it first (the units install together); one the moved objects' copy places in that list (objects_art) its number
    there, copied by their install, before the chips' code"""
    to = alloc['memo'].get((lst, v))
    if to is None:
        _, s5 = X.sprite(b5, BN5_SPRITE_LISTS, lst, v)
        while (lst, alloc['lists'][lst]) in alloc['reserved']:
            alloc['lists'][lst] -= 1
        to = alloc['lists'][lst]
        alloc['lists'][lst] -= 1
        alloc['memo'][(lst, v)] = to
        room = X.rounded(len(s5), SPRITE_ROOM)
        u.sprites.append(dict(list=lst, index=v, to_list=lst, to_index=to, at=alloc['art'], room=room))
        alloc['owner'][(lst, v)] = u
        alloc['art'] += room
    return to


# The engine's routines that spawn an effect whose id they are handed (BN5's routine: the register): each call's id in
# BN6's terms as an effect spawn's (its immediate, or TABLE_EFFECTS by the call's site)
EFFECT_CALLS = {
    0x08018838: 2,   # effects over a pattern of panels (BN6's twin 0x0801BD3C): r2 the effect, r3 the pattern, r4 the side
}


def chip_effect_calls(u):
    """each call of EFFECT_CALLS' routines an effect site of the unit's (f.effects), its id the register's"""
    f = u.f
    for w in f.walks.values():
        for site, t in sorted(w.bls.items()):
            if t in EFFECT_CALLS:
                v, chain = T.back_value(w, site, EFFECT_CALLS[t])
                f.effects.setdefault(None if v is None else v & 0xFF, []).append((site, chain))


def shifted_effects(f):
    """an effect id made by a number added to a value shifted past its byte (r4 = x << 8 + n: Slasher's 0x16 at
    0x080ED536-0x080ED538, its high byte the side's): its id the number, its chain the immediate's (effect_imm's)"""
    insns = {a: i for w in f.walks.values() for a, i in w.insns.items()}
    for site, chain in list(f.effects.get(None, [])):
        last = insns.get(chain[-1][0]) if chain else None
        adds = [x for x in chain[:-1] if x[1] == 'adds8']
        if (last is None or last.form != 'lsls' or len(last.f) < 3 or last.f[2] < 8 or not adds
                or any(x[1] != 'adds8' for x in chain[:-1]) or sum(x[2] for x in adds) > 0xFF):
            continue
        f.effects[None].remove((site, chain))
        f.effects.setdefault(sum(x[2] for x in adds), []).append((site, chain[:-1]))
    if None in f.effects and not f.effects[None]:
        del f.effects[None]


# The Navis' battle sprites (issue #82's allocations: list 2's 0x20-0x2F the Navis', src/emu/xnavi_sprites.h): BN5's Navi of
# AI n (Team Colonel's six, 7-12) draws list 2's n in BN5 and 0x20 + n - 7 in BN6, the one copy the Navis' install makes
# (xnavi_sprite_install), which a unit's install asks for first (its `navis`)
NAVI_SIX = range(7, 13)
NAVI_SPRITE_FIRST = 0x20
# Routines a Navi chip's object calls with a Navi's AI index, reading a table by it (BN5's routine: (the register holding the
# table's kind, the kind the Navis' table is, the register holding the AI index, why)): BN5's Navi of AI n is BN6's
# XN_AI_OF(n) = 32 + n - 7 there (src/emu/xnavi_recipe.h), whose entries the Navis' install grows the tables with
NAVI_AI_CALLS = {
    0x0800ED06: (0, 1, 1, "a Navi's companion begun by its AI (the triple 0x0800ED1C's second table, BN6's twin 0x08010DF6 reading "
                          "BN6_NAVI_COMPANION_REF's, grown by the Navis: KnightMan's overlay, its kind BN6's 23)"),
    0x0800EEB6: (0, 1, 1, "a Navi's companion ended by its AI (the triple 0x0800EECC's second table, BN6's twin 0x08011044 reading "
                          "BN6_NAVI_COMPANION_END_REF's, grown by the Navis)"),
}
# The chips whose rows still sit out (XREG_PENDING) though their unit is written, and what for (BN5's ids): none since the Navi
# chips' SP and DS play (their code their V1's, their powers BN5's computed ones: chip_numbrmnsp, chip_numbrmnds and on)
ROWS_WAIT = {}


# The companion's triples' third tables, the same as their second in both games (BN5 0x0800ED20 and 0x0800ED24, 0x0800EED0 and
# 0x0800EED4; BN6 0x08010E10 and 0x08010E14, 0x08011060 and 0x08011064): a call with the third (BN5's Colonel object's, the Navi
# chip ColonelDS's, 0x080BA3D4: its kind 2) is one with the second, which the Navis grow in BN6 and the third not: its kind made
# the second's too. (BN5's triple, its second table's word, BN6's)
NAVI_KIND_ALSO = 2
NAVI_TRIPLES = {0x0800ED06: (0x0800ED1C, 0x08010E0C), 0x0800EEB6: (0x0800EECC, 0x0801105C)}


def navi_ai_calls(u, b5=None, b6=None):
    """each call of NAVI_AI_CALLS' routines with the Navis' table: its AI index BN6's for BN5's Navi (one of the six), the
    immediate that makes it; any other a decision"""
    f = u.f
    for w in f.walks.values():
        for site, t in sorted(w.bls.items()):
            if t not in NAVI_AI_CALLS:
                continue
            kreg, kind, reg, why = NAVI_AI_CALLS[t]
            k, kchain = T.back_value(w, site, kreg)
            also = k == NAVI_KIND_ALSO and b5 is not None and t in NAVI_TRIPLES and \
                b5.u32(NAVI_TRIPLES[t][0] + 4 * kind) == b5.u32(NAVI_TRIPLES[t][0] + 4 * k) and \
                b6.u32(NAVI_TRIPLES[t][1] + 4 * kind) == b6.u32(NAVI_TRIPLES[t][1] + 4 * k)
            if k != kind and not also:
                continue   # (another table: BN6's own of the same index)
            v, chain = T.back_value(w, site, reg)
            if v not in NAVI_SIX:
                f.decide('navi ai', f'{site:08X}: {why}, its AI index {v} made otherwise or none of the six\'s')
                continue
            X.imm_for(u, chain, reg, v, 32 + v - 7, f'{why}: BN5\'s AI index {v:#x}, BN6\'s {32 + v - 7:#x}')
            if also:
                X.imm_for(u, kchain, kreg, k, kind, f'{why}: its table {k}, the same as {kind} in both games, {kind} in BN6 (the Navis\' '
                                                    f'grown, NAVI_TRIPLES)')


def art_of(u, b5, b6, alloc):
    """what a unit draws: the sprites its code loads by number, each BN6's own where its list holds the same bytes at that
    number, else BN5's copied at a number of the chips' in its grown list (CHIP_LISTS, from the top down), its loads set
    to it; and its effects"""
    f = u.f
    packed_loads(f)
    for (l, v), sites in sorted(f.loads.items(), key=lambda x: str(x[0])):
        if l == 4 * 2 and v in NAVI_SIX:
            to = NAVI_SPRITE_FIRST + v - 7
            u.navis.add(v)
            for site, chain in sites:
                load_rewrite(u, site, chain, v, to, f'BN5\'s Navi {v}\'s battle sprite: list 2 {v:#x} in BN5, {to:#x} in BN6 (the Navis\')')
            continue
        if (l is None or v is None) and all(s in MEGAMAN_LOADS for s, _ in sites):
            f.notes.append(f'MegaMan\'s own sprite at {", ".join(f"{s:08X}" for s, _ in sites)}: {MEGAMAN_LOADS[sites[0][0]]}')
            continue
        if (l is None or v is None) and all(s in TABLE_SPRITES for s, _ in sites):
            for site in sorted({s for s, _ in sites}):
                table_art(u, b5, b6, alloc, site)
            continue
        if l is None or v is None:
            f.decide('sprite', f'a sprite {l if l is None else l // 4}:{v if v is None else hex(v)} loaded at '
                               f'{", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        lst = l // 4
        _, s5 = X.sprite(b5, BN5_SPRITE_LISTS, lst, v)
        _, s6 = X.sprite(b6, BN6['sprites'], lst, v)
        if s5 and s5 == s6:
            continue
        if lst not in CHIP_LISTS or not s5:
            f.decide('sprite', f'a sprite {lst}:{v:#x} loaded at {", ".join(f"{s:08X}" for s, _ in sites)}: BN6\'s of that number another, '
                               + ('its list not grown' if s5 else 'BN5\'s not read'))
            continue
        to = place_sprite(u, b5, alloc, lst, v)
        for site, chain in sites:
            load_rewrite(u, site, chain, v, to, f'its sprite: list {lst} {v:#x} in BN5, {to:#x} in BN6 (BN5\'s copied there)')
    shifted_effects(f)
    for e, sites in sorted(f.effects.items(), key=lambda x: str(x[0])):
        if e is None and all(s in EFFECT_RANGES for s, _ in sites):
            for site in sorted({s for s, _ in sites}):
                effect_range(u, b5, b6, alloc, site)
            continue
        if e is None and all(s in TABLE_EFFECTS for s, _ in sites):
            for site in sorted({s for s, _ in sites}):
                table, size, off, rows = TABLE_EFFECTS[site]
                for k in range(rows):
                    at = table + size * k + off
                    e5 = b5.u8(at)
                    if any(x[0] == at for x in u.bytes):
                        continue   # (a table two sites read, its rows given once)
                    r5, r6 = b5.bytes(BN5['effects'] + 4 * e5, 4), b6.bytes(BN6['effects'] + 4 * e5, 4) if e5 < BN6['neffects'] else b''
                    if r5 == r6:
                        continue
                    e6 = effect_of(u, b5, b6, alloc, e5)
                    if e6 is not None and e6 != e5:
                        u.bytes.append((at, e5, e6, f'{table:08X}\'s row {k}: its effect {e5:#x} in BN5, {e6:#x} in BN6'))
            continue
        if e is None:
            f.decide('effect', f'an effect id made otherwise at {", ".join(f"{s:08X}" for s, _ in sites)}')
            continue
        r5 = b5.bytes(BN5['effects'] + 4 * e, 4)
        r6 = b6.bytes(BN6['effects'] + 4 * e, 4) if e < BN6['neffects'] else b''
        if r5 == r6:
            continue   # (BN6's entry the same sprite's number, animation and palette: BN6's own, its art the engine's, as redrawn)
        e6 = effect_of(u, b5, b6, alloc, e)
        if e6 is None:
            continue
        for site, chain in sites:
            effect_imm(u, b5, chain, e, e6)


# ---- the C files ----

KIND_C = {'attack': 'XCHIP_ATTACK', 'support': 'XCHIP_SUPPORT', 'navi': 'XCHIP_NAVI', 'helper': 'XCHIP_HELPER'}


def addr_text(a):
    """an address the C file writes: the moved objects' or a Navi's copy by its place, else the number"""
    if XN_CODE <= (a & ~1) < XN_ART:
        return f'XN_CODE + 0x{a - XN_CODE:X}'
    return X.addr_text(a)


def c_name(label):
    import re
    return re.sub(r'[^a-z0-9]', '', label.lower())


def c_unit(u, slot):
    n = c_name(u.label)
    L = [f'/* ---- {u.label}: BN5\'s {u.kind} {u.n5:#x} ({u.entry | 1:08X}) as BN6\'s {u.number:#x} ---- */' if u.kind != 'helper' else
         f'/* ---- {u.label}: BN5\'s {u.entry | 1:08X}, a helper (installed once, before the units) ---- */', '']
    L.append(f'static const XvBlock {n}_blocks[] = {{')
    for lo, hi, at, crc in u.blocks:
        base = 'XC_HELPERS' if u.kind == 'helper' else 'XC_CODE'
        L.append(f'\t{{ 0x{lo:08X}, 0x{hi:08X}, {base} + 0x{at - (XC_HELPERS if u.kind == "helper" else XC_CODE):X}, 0x{crc:08X}u }},')
    L.append('};')
    if u.data:
        L.append(f'static const XvData {n}_data[] = {{')
        for lo, hi, k in u.data:
            L.append(f'\t{{ 0x{lo:08X}, 0x{hi:08X}, XV_{k} }},')
        L.append('};')
    L.append(f'static const XvPair {n}_calls[] = {{')
    for c in u.calls:
        if c['spawner']:
            L.append(f'\t{{ {X.BN5_SPAWN_NAME[c["bn5"]]}, {c["spawner"]} }},   /* the dispatch\'s spawner */')
        else:
            how = c['cls'] + (' (audited)' if c.get('audit') else '')
            L.append(f'\t{{ 0x{c["bn5"]:08X}, {addr_text(c["bn6"] or 0)} }},   /* {c["source"]}, {how} */')
    L.append('};')
    if u.words:
        L.append(f'static const XvPair {n}_words[] = {{')
        L += [f'\t{{ 0x{v:08X}, 0x{to:08X} }},' for v, to in u.words]
        L.append('};')
    if u.imms:
        L.append(f'static const XvImm {n}_imms[] = {{')
        for i in u.imms:
            L.append(f'\t{{ 0x{i["at"]:08X}, {i["form"]}, {i["rd"]}, {i["rn"]}, 0x{i["was"]:X}, 0x{i["now"]:X} }},   /* {i["why"]} */')
        L.append('};')
    if u.numbers:
        L.append(f'static const XvNumber {n}_numbers[] = {{')
        L += [f'\t{{ 0x{lit:08X}, 0x{was:08X}, 0x{now:08X} }},' for lit, was, now in u.numbers]
        L.append('};')
    if u.sprites:
        L.append(f'static const XvSprite {n}_sprites[] = {{')
        for x in u.sprites:
            L.append(f'\t{{ {x["list"]}, 0x{x["index"]:02X}, {x["to_list"]}, 0x{x["to_index"]:02X}, XC_ART + 0x{x["at"] - XC_ART:X}, 0x{x["room"]:X} }},')
        L.append('};')
    if u.effects:
        L.append(f'static const XvId {n}_effects[] = {{ ' + ', '.join(f'{{ 0x{a:02X}, 0x{b:02X} }}' for a, b in u.effects) + ' };')
    if u.bytes:
        L.append(f'static const XvByte {n}_bytes[] = {{')
        L += [f'\t{{ 0x{at:08X}, 0x{was:02X}, 0x{now:02X} }},   /* {why} */' for at, was, now, why in u.bytes]
        L.append('};')
    if u.types:
        L.append(f'static const XvType {n}_types[] = {{ ' + ', '.join(
            f'{{ {k}, 0x{t5:02X}, {"XV_TYPE_MOVED" if t6 is None else "XV_TYPE_SHARED" if t6 == "SHARED" else f"0x{t6:02X}"} }}'
            for k, t5, t6, _ in u.types) + ' };')
    if u.songs:
        L.append(f'static const XcSong {n}_songs[] = {{ ' + ', '.join(f'{{ 0x{v:X}, 0x{slot:02X} }}' for v, slot in u.songs) + ' };')
    count = lambda a: f'(int)(sizeof {n}_{a} / sizeof *{n}_{a})'
    fields = [f'.{a} = {n}_{a}, .n{a} = {count(a)}' for a, have in (('blocks', True), ('data', u.data), ('calls', True), ('words', u.words),
                                                                   ('imms', u.imms), ('numbers', u.numbers), ('sprites', u.sprites),
                                                                   ('effects', u.effects), ('types', u.types), ('bytes', u.bytes)) if have]
    L += ['', f'#define {n.upper()}_CODE {{ .name = "{u.label} (BN5\'s)", .game = XROM_BN5_COLONEL_US, \\']
    L += [f'\t{f}, \\' for f in fields[:-1]] + [f'\t{fields[-1]} }}']
    songs = f'{n}_songs, {count("songs")}' if u.songs else 'NULL, 0'
    navis = sum(1 << (v - NAVI_SIX[0]) for v in u.navis)
    return '\n'.join(L), (f'\t{{ {KIND_C[u.kind]}, 0x{u.n5 & 0xFF if u.kind != "helper" else 0:02X}, 0x{u.number:02X}, 0x{u.entry | 1:08X}, XC_HOMES + 0x{XC_HOME_SIZE * slot:X}, '
                          f'{n.upper()}_CODE, {songs}, 0x{navis:02X} }},')


HEADER_C = '''/* xchips_bn5.c: BN5's chips' code as units for BN6's battle engine, written by tools/xchiprecipe.py from the two ROMs
 * (python3 tools/xchiprecipe.py; --check says whether this is what it writes). Do not edit: change the generator (or
 * tools/bn5_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/MULTIROM.md, The chips' code):
 * addresses in BN5 Team Colonel (USA) and BN6 Cybeast Gregar (USA), counts and the CRC32s of the ranges the install copies
 * from the player's ROM; nothing of either game's. The units are the generator's, whole (no decision open), in install
 * order; src/emu/xchips_code.c installs them and src/core/xchips_moved.c names the chips that run them. */
#include "xchips_bn5.h"

#include "bn5.h"
#include "bn6.h"
#include "rom.h"
#include "xchips_code.h"
#include "xnavi_recipe.h"
'''


def c_file(units):
    parts, rows = [HEADER_C], []
    for slot, u in enumerate(units):
        text_, row = c_unit(u, slot)
        parts.append(text_)
        rows.append(row)
    parts.append('const XChipUnit xchips_bn5[] = {\n' + '\n'.join(rows) + '\n};\n'
                 'const int xchips_bn5_count = (int)(sizeof xchips_bn5 / sizeof *xchips_bn5);\n')
    return '\n'.join(parts)


HEADER_MOVED = '''/* xchips_moved.c: the chips whose code moved from BN5's (src/emu/xchips_bn5.c, issue #82's C3), written by
 * tools/xchiprecipe.py from the two ROMs and the registry (src/core/xchips_registry.c): each its id in BN6's tables and the
 * family and subfamily its record takes in BN6's engine (a moved attack family's number, BN5's subfamily under it; or the
 * support or Navi family with the moved routine's entry); and the computed powers whose routines moved (BN5's table's
 * entry, BN6's grown table's). Do not edit: write it again. Numbers alone. */
#include "xchips_moved.h"

#include <stddef.h>

const XChipMoved xchips_moved[] = {
'''


def moved_file(rows, powers):
    body = '\n'.join(f'\t{{ {bn6}, 0x{fam:02X}, 0x{sub:02X} }},   /* {name} */' for bn6, fam, sub, name in rows)
    pw = '\n'.join(f'\t{{ {i}, 0x{POWER_FIRST + i:02X} }},   /* {why} */' for i, why in powers)
    return (HEADER_MOVED + body + '\n};\nconst int xchips_moved_count = (int)(sizeof xchips_moved / sizeof *xchips_moved);\n\n'
            'const XChipMoved *xchips_moved_of(int bn6) {\n\tfor (int i = 0; i < xchips_moved_count; ++i)\n'
            '\t\tif (xchips_moved[i].bn6 == bn6) return &xchips_moved[i];\n\treturn NULL;\n}\n\n'
            'const XChipPower xchips_powers[] = {\n' + pw + '\n};\n'
            'const int xchips_powers_count = (int)(sizeof xchips_powers / sizeof *xchips_powers);\n\n'
            'int xchips_power_of(int bn5_power) {\n\tfor (int i = 0; i < xchips_powers_count; ++i)\n'
            '\t\tif (1000 + xchips_powers[i].bn5 == bn5_power) return 1000 + xchips_powers[i].bn6;\n\treturn 0;\n}\n')


# ---- the Program Advances: BN5's recipes of the registry's (XREG_PA) into BN6's list (src/core/xchips_pas.c) ----

BN5_PA_LIST, BN5_PA_LIST_REF = 0x08028020, 0x08025260   # BN5's Custom screen's list of its recipes (0x080251E0's), its word
PA_KIND_SAME, PA_KIND_IDS = 0, 4                       # a recipe's kind: n of one chip, codes in a row (0x080295C8); these ids (0x0802961A)
# Recipes of BN5's that BN6's list holds but its codes, or a recipe of BN6's before it, never let form (kept: BN5's recipe whole,
# which a deck of BN5's codes would form; they stay unformed, BN6's codes and recipes first: the decision of issue #82's last
# integration round, docs/MULTIROM.md)
PA_UNFORMED = {
    324: "three Vulcan2 of codes in a row, BN6's Vulcan2 D, F and L (BN6's record), none three in a row",
    325: "three Vulcan3, BN6's A, G and R, none three in a row",
    326: "three FireHit1 in a row, BN6's own FlmHook1's recipe, first in BN6's list (0x0802BB0E), which wins",
    327: "three FireHit2, BN6's FlmHook2's recipe, first",
    328: "three FireHit3, BN6's FlmHook3's recipe, first",
}


def pa_rows(b5):
    """BN5's recipes whose Program Advance the registry gives an id of its own (XREG_PA) and whose parts it registers: (BN5's
    entry, n, kind, BN6's PA id, BN6's part ids, the PA's name, the parts' names), in BN5's order, the kind-4 first as both
    games order theirs"""
    if b5.u32(BN5_PA_LIST_REF) != BN5_PA_LIST:
        sys.exit(f'xchiprecipe: BN5\'s Program Advances\' list is not at {BN5_PA_LIST:08X}')
    names = X.chip_names(b5, BN5['chip_names'], 424)
    reg = {bn5: (bn6, how) for bn5, bn6, how in registry_rows()}
    rows, k = [], 0
    while b5.u32(BN5_PA_LIST + 4 * k):
        p = b5.u32(BN5_PA_LIST + 4 * k)
        n, kind, pa = b5.u8(p), b5.u8(p + 1), b5.u16(p + 2)
        parts = [b5.u16(p + 4 + 2 * j) for j in range(n if kind == PA_KIND_IDS else 1)]
        if pa in reg and reg[pa][1] == 'PA' and all(x in reg for x in parts):
            rows.append((p, n, kind, reg[pa][0], [reg[x][0] for x in parts], names.get(pa, '?'), [names.get(x, '?') for x in parts], pa))
        k += 1
    return sorted(rows, key=lambda r: r[2] != PA_KIND_IDS)


HEADER_PAS = '''/* xchips_pas.c: BN5's Program Advances of the chip registry (XREG_PA), their recipes as BN5's list has them (0x08028020),
 * in BN6's chip ids, written by tools/xchiprecipe.py from BN5's ROM and the registry (src/core/xchips_registry.c), for BN6's
 * list grown with them (src/emu/xchips_code.c). Do not edit: write it again. Numbers alone. */
#include "xchips_pas.h"

const XChipPa xchips_pas[] = {
'''


def pas_file(rows):
    body = []
    for p, n, kind, pa, parts, name, part_names, pa5 in rows:
        ids = ', '.join(str(x) for x in parts + [0] * (4 - len(parts)))
        how = f'{n} {part_names[0]} of codes in a row' if kind == PA_KIND_SAME else ' + '.join(part_names)
        note = f'; never formed: {PA_UNFORMED[pa5]}' if pa5 in PA_UNFORMED else ''
        body.append(f'\t{{ 0x{p:08X}, {n}, {kind}, {pa}, {{ {ids} }} }},   /* {name}: {how}{note} */')
    return HEADER_PAS + '\n'.join(body) + '\n};\nconst int xchips_pas_count = (int)(sizeof xchips_pas / sizeof *xchips_pas);\n'


def powers_of(b5):
    """BN5's computed powers the registry's chips have whose routines a helper moves (POWERS): (BN5's entry, why)"""
    roots = {r: label for rs, label, _ in HELPERS for r in rs}
    used = {}
    for bn5, bn6, how in registry_rows():
        power = b5.u16(BN5_RECORDS + 0x2C * bn5 + 0x1A)
        if power >= 1000 and power - 1000 < BN5_POWER_COUNT:
            routine = b5.u32(BN5_POWERS + 4 * (power - 1000)) & ~1
            if routine in roots:
                used.setdefault(power - 1000, f'{roots[routine]}: BN5 {routine:08X}')
    return sorted(used.items())


def moved_rows(units, b5):
    """the registry's chips each unit plays: BN5's records of that family (an attack unit's: every subfamily, BN5's kept), or
    of the support or Navi family with that subfamily; each a chip the registry gives an id of its own, moved (XREG_MOVED),
    or a Program Advance (XREG_PA) or a DarkChip of DARK_MOVED whose code moved, waiting for its rule to play"""
    names = X.chip_names(b5, BN5['chip_names'], 424)
    rows = []
    reg = registry_rows()
    for u in units:
        if u.kind == 'helper':
            continue
        played = played_subs(b5, u.n5) if u.kind == 'attack' else None
        for bn5, bn6, how in reg:
            rec = record(b5, BN5_RECORDS, bn5)
            fam, sub = rec[0x0B], rec[0x0C]
            if u.kind == 'attack' and fam == u.n5:
                rows.append((bn5, bn6, u.number, sub, names.get(bn5, '?'), how, sub in played))
            elif u.kind != 'attack' and fam == (BN5_SUPPORT_FAMILY if u.kind == 'support' else BN5_NAVI_FAMILY) and sub == u.n5:
                rows.append((bn5, bn6, BN6_SUPPORT_FAMILY if u.kind == 'support' else BN6_NAVI_FAMILY, u.number, names.get(bn5, '?'),
                             how, True))
    # (BN6's own chips by name run BN6's code, a record of a subfamily the unit plays none of its twin's; a DarkChip outside
    # DARK_MOVED plays over its twin's)
    bad = [f'{name} ({bn6}, {how})' for bn5, bn6, fam, sub, name, how, played in rows
           if (how == 'PENDING' and bn5 not in ROWS_WAIT) or (how == 'RECORD' and played) or (how != 'PENDING' and bn5 in ROWS_WAIT)]
    keep = lambda bn5, how: how in ('MOVED', 'PA') or (how == 'DARK' and bn5 in DARK_MOVED) or (how == 'SAME' and bn5 in SAME_MOVED)
    return sorted((bn6, fam, sub, name) for bn5, bn6, fam, sub, name, how, _ in rows if keep(bn5, how)), bad


# ---- the report ----

def report(units, path):
    lines = ['# BN5\'s chips\' code as tools/xchiprecipe.py derives it', '']
    for u in units:
        f = u.f
        cls = {}
        for c in u.calls:
            k = 'dispatch' if c['spawner'] else 'none' if c['bn6'] is None else c['cls'] + ('+audit' if c.get('audit') else '')
            cls[k] = cls.get(k, 0) + 1
        moved = [f'{kind_name(k)} {t5:#x}' + ('' if t6 is None else ' (an earlier track\'s copy)' if t6 == 'SHARED' else f' as BN6\'s {t6:#x}')
                 for k, t5, t6, _ in u.types]
        head = f'BN5\'s {u.kind} {u.n5:#x} ({u.entry:08X}) as BN6\'s {u.number:#x}' if u.kind != 'helper' else f'BN5\'s {u.entry:08X}, a helper'
        lines += [f'## {u.label}: {head}', '',
                  f'- code: {len(u.blocks)} blocks, {sum(b - a for a, b, _, _ in u.blocks)} bytes: '
                  + ', '.join(f'{a:08X}-{b:08X}' for a, b, _, _ in u.blocks),
                  f'- calls {len(u.calls)} ({", ".join(f"{v} {k}" for k, v in sorted(cls.items()))}): '
                  + ', '.join(f'{c["bn5"]:08X}->{c["bn6"] or 0:08X}' for c in u.calls if not c['spawner']),
                  f'- immediates {len(u.imms)}; words {len(u.words)}; numbers kept {len(u.numbers)}; objects: {", ".join(moved) or "-"}; '
                  f'sounds ' + (', '.join(f'{v:#x}{"" if same else " (differs)"}' for v, same in u.sounds) or '-') +
                  f'; random draws {len(f.rng)}']
        if f.notes:
            lines.append('- notes: ' + '; '.join(dict.fromkeys(f.notes)))
        lines.append('- decisions: ' + ('none' if not f.decisions else ''))
        lines += [f'  - {k}: {d}' for k, d in f.decisions]
        lines.append('')
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8') as o:
        o.write('\n'.join(lines) + '\n')


def dispatch_disagrees(units):
    """each object type the units spawn through the dispatch, as they derive it (moved, its copy theirs or a track's before
    them, or BN6's of the same number: twin), against bn5_twins.TYPES, one decision per type for every track: where none is
    written or another, what to write; a moved type whose copy TYPES gives another track's than the one that installs it"""
    out = []
    for u in units:
        for kind, t5, t6, why in u.types:
            got = 'moved' if t6 is None or t6 == 'SHARED' else 'twin' if t6 == t5 else f'twin as {t6:#x}'
            want = bn5_twins.TYPES.get((kind, t5))
            if want is None or want[0] != got:
                out.append(f'{u.label}: the {kind_name(kind)} kind\'s {t5:#x} {got} ({why})' + (f', where it says {want[0]}' if want else ''))
            elif got == 'moved' and (t6 == 'SHARED') != (want[2] in BEFORE_CHIPS):
                out.append(f'{u.label}: the {kind_name(kind)} kind\'s {t5:#x} {"spawned as" if t6 == "SHARED" else "moved, where TYPES gives"} '
                           f'the {want[2]}\' copy')
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--check', action='store_true')
    ap.add_argument('--report', action='store_true')
    ap.add_argument('--unit', nargs='*', default=[], help='more units for the report: KIND:N:NAME (attack:0x4f:DrilArm)')
    a = ap.parse_args()
    b5, b6 = X.find_roms()
    pairs = Pairs(b5, b6, X.MATCH, X.WORK)
    if not pairs.map_ok:
        sys.exit(f'xchiprecipe: no symbol map in {X.MATCH}: build.py symbols --bn6f DIR makes it')
    alloc = dict(code=XC_CODE, hcode=XC_HELPERS, art=XC_ART, lists={k: CHIP_LIST_TOP for k in CHIP_LISTS}, memo={}, owner={},
                 effects={}, effect_next=CHIP_EFFECT_TOP, reserved=set())
    objects_art(b5, alloc)
    # (the helpers first, their copies' places for the units' calls; their art after the units', by what the units use)
    helpers, helper_at = [], {}
    for roots, label, _ in HELPERS:
        h = derive(b5, b6, pairs, 'helper', roots, label, alloc, 0, helper_at)
        helpers.append(h)
        # (every routine of its, for the units' calls: its root and those it calls)
        for e in h.f.walks:
            at = next((at + (e - lo) for lo, hi, at, _ in h.blocks if lo <= e < hi), None)
            if at is not None:
                helper_at[e] = at
    order = list(UNITS) + list(CANDIDATES) + [(k, int(n, 0), name) for k, n, name in (x.split(':') for x in a.unit)]
    units = []
    for k, (kind, n, label) in enumerate(order):
        units.append(derive(b5, b6, pairs, kind, n, label, alloc, number_of(kind, n), helper_at))
    flash_kinds(helpers, units[:len(UNITS)])
    units = helpers + units
    shared_rows(units, b5)
    report(units, os.path.join(WORK, 'report.md'))
    print(f'xchiprecipe: {len(units)} units derived, {sum(1 for u in units if not u.f.decisions)} with no decision open; '
          f'{os.path.join(WORK, "report.md")}')
    if a.report:
        return 0
    whole = units[:len(HELPERS) + len(UNITS)]
    for u in whole:
        if u.f.decisions:
            sys.exit(f'xchiprecipe: {u.label} has decisions open: ' + '; '.join(f'{k}: {d}' for k, d in u.f.decisions))
    undecided = dispatch_disagrees(whole)
    if undecided:
        sys.exit('xchiprecipe: the dispatch\'s decisions (tools/bn5_twins.py, TYPES) want: ' + '; '.join(undecided))
    rows, bad = moved_rows(whole, b5)
    if bad:
        sys.exit('xchiprecipe: the registry does not give these chips XREG_MOVED: ' + ', '.join(bad))
    out = {OUT_C: c_file(whole), OUT_MOVED: moved_file(rows, powers_of(b5)), OUT_PAS: pas_file(pa_rows(b5))}
    if a.check:
        same = all(open(p, encoding='utf-8').read() == t if os.path.exists(p) else False for p, t in out.items())
        print(f'xchiprecipe: the three files {"are" if same else "are NOT"} what the generator writes')
        return 0 if same else 1
    for p, t in out.items():
        with open(p, 'w', encoding='utf-8') as f:
            f.write(t)
    print(f'xchiprecipe: {", ".join(u.label for u in whole)} written into {os.path.relpath(OUT_C, ROOT)}; '
          f'{len(rows)} chips into {os.path.relpath(OUT_MOVED, ROOT)}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
