"""bn5_navi_twins.py: the Navis' own pairs of BN5 Team Colonel's routines with their twins in BN6 Cybeast Gregar (USA),
and their audits, beside tools/bn5_twins.py's (the families'), read by tools/xnavirecipe.py (issue #82; docs/rom/navis.md,
The Navis' engine twins). Numbers only: each pair's two bodies compared by tools/xrecipe_thumb.py, each DIFF judged for
what a Navi's moved code asks of it."""

# The Navis' generic states (BN5's state tables name them, 0-5) and their twins: BN5 numbers its Navis' own states from
# 6, BN6 from 8 (its frozen 6 and bubbled 7 between). State 1 has two forms in BN5: the Navis' (0x080142D0, most) and the
# viruses' (0x080133C4: ProtoMan, Colonel, KnightMan, ToadMan), each writing CurAction 6, their twins writing 8
NAVI_GENERIC5 = {0: (0x080131A0,), 1: (0x080142D0, 0x080133C4), 2: (0x08013D1C,), 3: (0x0801412A,), 4: (0x080141EC,), 5: (0x080142DC,)}
NAVI_GENERIC6 = {0: 0x08016380, 1: 0x08017888, 2: 0x080170C4, 3: 0x080174FE, 4: 0x080175B8, 5: 0x080178B6, 6: 0x08017688,
                 7: 0x08017768}
GENERIC_TWINS = {0x080131A0: 0x08016380, 0x080142D0: 0x08017888, 0x080133C4: 0x080165B8, 0x08013D1C: 0x080170C4,
                 0x0801412A: 0x080174FE, 0x080141EC: 0x080175B8, 0x080142DC: 0x080178B6}
# Their pre-steps (table 1): A (most), B (no knockback: ToadMan), C (ShadeMan and on), D (the bosses'), each BN6's of the
# same name (BN6 runs C's step 0x08010230 only out of battle mode 10, where BN5 always; BN5's C runs 0x08011150 too)
PRESTEPS = {0x08017680: 0x0801A9B8, 0x080176EC: 0x0801AA48, 0x08017758: 0x0801AAC0, 0x080177CC: 0x0801AB40}
# The Navis' empty hook, and the companion's empty start and end
NULLSUB5, NULLSUB6 = 0x080F2748, 0x080F3BE8
EMPTY_COMPANION5, EMPTY_COMPANION6 = 0x0800F194, 0x080114D4

# What BN6's engine lacks, aimed at the Navis' stubs (src/emu/xnavi_recipe.h): a panel claim's two spawners (their gate,
# BattleState +0x32 bit 0x40, set by BN5's 0x0802D594 alone, is clear in every battle a run fights: r0 0, as BN5 answers
# then), the DS version's chip AI's test (AIData +0xF0, set only by its entry: 0) and its entry and run (reached only by
# version 4, the DS, which no recipe installs: logged)
STUBS = {0x080EA08C: 'XN_STUB_ZERO', 0x080EA0BA: 'XN_STUB_ZERO', 0x0802C10A: 'XN_STUB_ZERO', 0x0802C114: 'XN_STUB_DS',
         0x0802B804: 'XN_STUB_DS',
         # the Dark Soul's chip AI's call at 0x0802BA7C, which BN5's ROM aims at data (0x081BC994: the end of a table of
         # pointers, then 0xFF fill; its DS twin at 0x0802B86A calls 0x0802BE52): behind 0x0802C016's count, which stays 0
         # (its two counters, AIData +0x110 and +0xF8, written by 0x0802BF52 alone, which nothing calls or names), dead: r0 0
         0x081BC994: 'XN_STUB_ZERO',
         # the attack setups by number (BN5's 0x0800F350, its table 0x0800F368; BN6's 0x080117BA another table): the Dark
         # Soul's chip AI calls it with 0x3E alone (0x0802BB90), the Navis' buster (BN5's 0x0800FAFE: AttackVars +8 the
         # buster's level + 1, +0xA 0, CurAction 0x16), which BN6 has no setup of: the stub makes the same, its family BN6's
         # number for BN5's 0x16 (FAMILIES)
         0x0800F350: 'XN_STUB_BUSTER',
         # BattleState +0x5C's bits set (BN5's 0x0800131E), which the moved Colonel calls alone (BN5_COLONEL_SIDE_SET, from his
         # spawn's end: 0x10 side 0, 0x20 side 1, read by BN5's obstacles' dark hole); BN6 numbers the word's bits otherwise
         # (its 0x10 the field's slide as a battle ends, BN5's 0x40) and reads no Colonel's: his bits kept in a field of the
         # Navis' own (BN6_COLONEL_SIDES)
         0x0800131E: 'XN_STUB_SIDES',
         # a chip's record by its id (BN5's 0x0801DF0C, BN6's twin 0x08021AA4 the same code), which the Dark Soul's chip AI
         # alone calls: its choice and approach read four bytes BN5 gives a meaning BN6's records do not hold (+9 bit 0,
         # +0xD the kind of approach, +0xE bits 0-1, +0x17 the region it aims from; BN6's +0x17 its lock-on type), its attack
         # (0x0802C098) BN6's own (+4, +0xA-+0xC, +0x10, +0x14: BN6's action for the chip): the Dark Soul's records, BN6's
         # with BN5's four bytes, by BN6's id (src/emu/xnavi_dark.c)
         0x0801DF0C: 'XN_STUB_CHIPDATA'}
# The one number each stub serves where the stubbed routine takes one (r0 at every call): the generator's check
STUB_ARGS = {0x0800F350: 0x3E}

# BN5's panel types as BN6's (tests/xdiff/legit.json, maps, panel_type): its lava and sea BN6's 13 and 14 where
# src/emu/xpanels.c brings them in, its holy BN6's 5; a Navi's code that sets a panel's type or compares one read from a
# panel (its +2) takes BN6's number
PANEL_TYPES = {0x8: 0xD, 0x9: 0x5, 0xA: 0xE}
# setPanelType's sites whose type (r2) the back walk does not reach, read by hand: site: (the type, why)
PANEL_SITES = {
    0x080C9AA8: (7, "BlizzardMan's ice: movs r2,#7 at 0x080C9A8C, kept over two calls by push {r0-r2} / pop (ice, 7 in both games)"),
    0x080C9AD6: (7, "BlizzardMan's ice: movs r2,#7 at 0x080C9A8C, kept over two calls by push {r0-r2} / pop (ice, 7 in both games)"),
}
# BN5's routines that give a panel's data (r0), whose +2 is its type: object_getPanelDataOffset
PANEL_GETTERS = (0x0800AFC2,)
SET_PANEL_TYPE = 0x0800B2A6
# BN5's companion overlay, the first kind's type 0x56 (BN6's BN6_OVERLAY_*: its init 0x080C5BD8, BN5's 0x080C36F0), and
# BN6's: each its spawner and end, its kinds' masks (a word a kind, naming a byte an animation: 0 drawn behind its Navi)
# and sprites (two bytes a kind: list offset and number). BN5's kinds BN6 lacks take numbers of their own past BN6's
# (src/emu/xnavi_recipe.h, XN_KIND_OF), their masks and sprites copied, and the companion routines that spawn and end
# them copied with their Navi
OVERLAY5 = dict(spawner=0x080C3808, end=0x080C382C, masks=0x080C36AC, sprites=0x080C36C4, kinds=6)
OVERLAY6 = dict(spawner=0x080C5D18, end=0x080C5D38, masks=0x080C5B44, sprites=0x080C5B90, kinds=19)
# The image object (the first kind's type 5: an image an attack shows, a sprite by its kind, +4), BN5's and BN6's: its
# spawner (r4 its parameters, the kind the low byte) and its kinds' tables (five bytes a kind: list offset, number,
# palette and two more; BN6 numbers its kinds otherwise, BN5's 3 and 0x25 BN6's other sprites)
IMAGE5 = dict(spawner=0x080B9CAE, kinds=0x080B9A5C)
IMAGE6 = dict(kinds=0x080BA444, count=52)

# BN5's companion routines (BN5_NAVI_COMPANION's and BN5_NAVI_COMPANION_END's words) that spawn or end what BN6 runs
# otherwise, copied with their Navi: an overlay of a kind BN6 lacks, Colonel's start (kind 3) and end, KnightMan's (kind
# 1); NumberMan's companion, the first kind's 0x54 (its spawner below, SPAWNS_OTHER), and its end (BN6's twin of the
# end, 0x080111DD, ends BN6's 0x55)
COMPANIONS = (0x0800EE56, 0x0800EFE4, 0x0800EE72, 0x0800F000, 0x0800EE66, 0x0800EFF2)

# BN5's routines whose twin in BN6 (the same bytes) spawns an object of the same type number that BN6 runs otherwise:
# copied with the Navi, so that its spawn goes through the dispatch with BN5's type moved (tools/xnavirecipe.py)
SPAWNS_OTHER = {
    # three of the third kind's 0x1D, BN5's falling rocks (KnightMan's wrecking ball's, from its 0x14 at 0x080C7252): BN6's
    # 0x1D (0x080C95E9) is another rock, set up otherwise (its height 0x68 where BN5's 0xA0, its sprite 4:5 where 4:0xE, its
    # collision 0x15/5/1 where 4/5/3) and claiming no panel (BN5's 0x080EA08C at 0x080C8150): knightman_v1 told them apart
    # at t613, the rocks' first frame
    0x080C825C: "BN5's falling rocks",
    # one of the fourth kind's 9, BN5's rock fragment: BN6's 9 (0x080E22DD) draws its sprite 4:5 where BN5's draws 4:0xE,
    # the falling rock's own (knightman_v1 told them apart at t685); and the routine a falling rock lands by (0x080C81D8),
    # which spawns two through it (0x080C8222)
    0x080E10F8: "BN5's rock fragment",
    0x080C8204: "BN5's rock fragments, two",
    # Colonel's: the third kind's 0x3C (0x080F8CF2: BN6's 0x3C, 0x080CCF69, strikes with effect 0x60, list 4's 0xA, where
    # BN5's 0x080CCAD5 with 0x47, list 4's 0x2C: colonel_v1_keys at t1003) and its 0x59 (0x080F9132: BN6's 0x080D11C5 no
    # deep twin of BN5's 0x080D1089, its sprite's frames apart: colonel_sp at t777)
    0x080CCCCE: "Colonel's 0x3C",
    0x080D12D6: "Colonel's 0x59",
    # the fourth kind's 0x38, which Colonel's 0x59 spawns two of as it strikes (0x080D12A2, 0x080D12AE): BN6's twin of the
    # spawner (0x080E5AE4, the same bytes) spawns BN6's 0x38 (0x080E5A19), whose palette (+0x95) and draws are another's
    # (colonel_sp at t1004)
    0x080E48BC: "the fourth kind's 0x38 (Colonel's strike's)",
    # NumberMan's companion, the first kind's 0x54 (from his companion's start, 0x0800EE66): BN6's twin of the spawner
    # (0x080C5A48, the same steps) spawns its 0x55, which runs otherwise (no deep twin of BN5's 0x080C33AD)
    0x080C34A6: "NumberMan's companion",
}

# Writes of an object's +9 (CurAction for an enemy) that are no Navi's state: a moved object of the third kind copying its
# owner's +9, its owner another such object, both BN5's code and numbers in either game (the generator's curaction check)
OWN_STATES = {
    # TomahawkMan's third kind's 0x23, spawned by his 0x1F (0x080C8444, r5 that object), its owner's state tracked
    0x080C9154: "TomahawkMan's 0x23 copies the state of its owner, his 0x1F",
}

# A Navi's setAttack whose CurAction its code makes otherwise than by a number of its own, each judged (the generator's
# curaction check): the site of the call, and why BN6's engine runs what BN5's does
CURACTIONS = {
    # the Dark Soul's chip AI: the chip's family from its record (0x0802C098: BN6's record of the chip's id by BN6's
    # GetChipData, so BN6's family for BN6's chips and the registry's for BN5's own, as the chip runs for MegaMan)
    0x0802C0D2: "the chip's family from its record (BN6's, by BN6's chip id)",
    # ... the Navis' buster, its setup the stub's (XN_STUB_BUSTER: BN6's number for BN5's family 0x16, FAMILIES)
    0x0802BB96: "the Navis' buster, its family the moved one's number (XN_STUB_BUSTER)",
    # ... its state 0xC (AIData +0xF2), which nothing of the Dark Soul's code sets (no 0xC stored there; BN5's DS twin's
    # 0x0802B95A alone): dead, its CurAction from BN6's setups by AIData +7 as the pair has it
    0x0802BC42: "dead: the chip AI's state 0xC, which nothing of the Dark Soul's sets",
    # the player's chip AI: the Navis' buster (as 0x0802BB96), and its state 0xC (AIData +0xF2), which nothing of its own sets
    0x0802B62C: "the player's chip AI: the Navis' buster (XN_STUB_BUSTER)",
    0x0802B6D2: "dead: the player's chip AI's state 0xC, which nothing of its own sets",
}

# BN5's sounds a Navi's code plays that BN6 has as another song, played as BN6's of the same number all the same: {sound:
# why}
SOUNDS_AS_BN6 = {
    # the buster's: BN5 plays 0x6A for MegaMan's buster and the Navis' alike (0x080EBAE2, 0x080EC51C); BN6's 0x6A is its
    # MegaMan's buster's, remade, which BN6's engine plays for his buster: the Navis' buster sounds as his, as in BN5
    0x6A: "the buster's sound, MegaMan's and the Navis' alike",
}

# Sounds a moved object starts from a table of its kinds (ldr r1,[r5,#0x60]; ldrh r0,[r1,#n]: its entry +n): (the table,
# its entries' size, the sound's offset, how many), each entry's sound checked to be the same song in both, else a
# decision
SOUND_TABLES = {
    # Colonel's 0x59: four entries of eight bytes at 0x080D1068, sounds 0x97 and 0xD9
    0x080D12C4: (0x080D1068, 8, 4, 4),
}

# Effects whose id is a variable added to a first (adds rX,#first; adds r4,r4,rX at the spawn): (how many, why)
EFFECT_RANGES = {
    # Colonel's slash: its pattern 0-2 (AttackVars +3, from 0x080F865A's three, 0x080F86B0), BN5's 0x48-0x4A (list 4's
    # 0x1F, animations 0-2)
    0x080F8B8C: (3, "Colonel's slash by his pattern"),
}
# BN5's routines no other code than a Navi's calls, BN6 having no twin: copied with it
COPIED = {
    # a panel within a pattern of the region table (0x0801629C) from a place and facing: Colonel's three (0x080F870C,
    # 0x080F8762, 0x080F87B8)
    0x08029C5A: "Colonel's pattern test",
    # BN5's chip AI, the Dark Soul's (BN5_CHIP_AI_DARK, 0x0802BA18, from its state 6; AIData +0xF0 its state): it plays the
    # chips of its foe's side's list (RAM_TWINS below), each as its own attack, after moving where the chip reaches its
    # target; the DS versions' twin of it (BN5_CHIP_AI_RUN) stays stubbed. Its routines, the machine's
    0x0802BA18: "the Dark Soul's chip AI", 0x0802B4B0: "the player's chip AI (its own side's lists)",
    0x0802BE52: "the chip AI: a chip of flag 1 first, its own side's", 0x0802BE58: "the chip AI: a chip of flag 1 first, its foe's", 0x0802BEEA: "the chip AI: the first of its own list",
    0x0802BEFE: "the chip AI: a sequence's entry, its own side's", 0x0802BF1A: "the chip AI: its own list turned",
    0x0802C0DA: "the chip AI: its own list shuffled", 0x0802BD96: "the chip AI: the next object of its foe's side",
    0x0802BDB8: "the chip AI: its next target", 0x0802BDE4: "the chip AI: an object of its foe's side",
    0x0802BE0A: "the chip AI: its target index on", 0x0802BE9E: "the chip AI: a chip of flag 2 first",
    0x0802BEEE: "the chip AI: the first of its foe's list", 0x0802BF02: "the chip AI: a sequence's entry",
    0x0802BEA4: "the chip AI: a chip of flag 2 first, its foe's (EXTRA_ROOTS)",
    0x0802BF20: "the chip AI: its foe's list turned", 0x0802C016: "the chip AI: a count (0: its counters never move)",
    0x0802C02A: "the chip AI: the other count (0)", 0x0802C098: "the chip AI: a chip's numbers written, its attack begun",
    0x0802C0E0: "the chip AI: its foe's list shuffled",
    # ... and its approach (0x08029BF0 begun with a chip and a target, 0x08029B40 under way): where a chip of each kind
    # reaches the target from, and the walk there
    0x08029B40: "the approach under way", 0x08029BF0: "the approach begun", 0x08029C04: "the approach",
    0x08029C1A: "the approach", 0x08029CB6: "the approach", 0x08029D00: "the approach", 0x08029E5E: "the approach",
    0x08029F42: "the approach", 0x0802A084: "the approach", 0x0802A188: "the approach", 0x0802A20C: "the approach",
    0x0802A290: "the approach", 0x0802A42A: "the approach", 0x0802A4DE: "the approach", 0x0802A5EC: "the approach",
    0x0802A672: "the approach", 0x0802A72C: "the approach", 0x0802A758: "the approach", 0x0802A7CE: "the approach",
    0x0802A8CE: "the approach", 0x0802A93E: "the approach", 0x0802AB22: "the approach", 0x0802AB86: "the approach",
    0x0802ACE2: "the approach", 0x0802AD32: "the approach", 0x0802ADF6: "the approach",
    # ... the places a chip reaches its target from, by the chip AI's own table of regions (0x08022AF8, 23 patterns; BN6's
    # table at 0x08019B78 is another): a pattern by number, and a random panel of one from the foe's side (BN6's twin of
    # the latter, 0x08015E46, keeps the side)
    0x08022AF0: "the chip AI's region by number", 0x08012C30: "a random panel of the chip AI's region, from the foe's side",
    # ... and BN5's random panel of a region itself (GetRandomRelativePanelFiltered): its candidates kept from sp+0x18,
    # where the fifth on overwrites its own saved place and on (sp+0x1C on), which BN6's twin (0x08015E58) mended (sp+0):
    # BN5's copied, so a region of more than four panels picks as BN5's does
    0x08012C48: "BN5's random panel of a region, its candidates over its saved place past four",
    # the buster's arm drawn (from the Navis' buster, BN5's family 0x16): BN5's image object's kind for the Navi by its AI
    # index (0x23 for the Dark Soul, palette 0xE), through the moved image object, where BN6's twin (0x080EC8A2, 0x080EC8B2)
    # spawns BN6's own type 5 with BN6's kinds and AI indexes
    0x080EBB96: "the buster's arm", 0x080EBBA6: "the buster's arm, by the Navi's AI index",
}
# A Navi's copied code that compares a Navi's AI index (AIData +1) with BN5's number of one of the moved Navis: the
# compare's immediate made BN6's number for it (src/emu/xnavi_recipe.h, XN_AI_OF), {site: why}
NAVI_AI_CMPS = {
    # the buster's arm (BN5's 0x080EBBA6, copied with the Dark Soul: BN6's twin, 0x080EC8B2, draws BN6's arms by BN6's AI
    # indexes): the Dark Soul's own arm, the image object's kind 0x23 with palette 0xE
    0x080EBC3C: "the buster's arm: the Dark Soul's",
}

# The image object's kinds a spawn of a Navi's copied code makes otherwise than by an immediate before it: {site: (the kinds
# it makes for the Navi that runs the copy, why)}
IMAGE_KINDS = {
    # the buster's arm (BN5's 0x080EBBA6, the Dark Soul's copy): r4 0x23 + its palette 0xE << 24 on the Dark Soul's branch
    # (AIData +1 0x16, NAVI_AI_CMPS); its others (0x2B, 0x33) other objects' alone, which never run this copy
    0x080EBC7E: ((0x23,), "the Dark Soul's buster's arm"),
}

# Routines of its game's a Navi's walk takes though nothing of it calls them: {Navi: {routine: why}}
EXTRA_ROOTS = {
    # the chip AI's search for a chip of flag 2, its foe's variant: its head sits between the own side's head (0x0802BE9E,
    # walked) and their common body (0x0802BEAC), which the walked head branches to: walked, its blocks one
    0x16: {0x0802BEA4: "between a walked head and its body",
           # BN5's chip AI for the player (BN5's MegaMan's idle states run it while his AI data's flag 0x200 stands: 0x080F04DC,
           # 0x080F0660; BN5's DarkInvs sets it, 0x0802C114): copied with the Dark Soul's, whose routines it shares, its own
           # side's heads before their common bodies
           0x0802B4B0: "the player's chip AI", 0x0802BE58: "between a walked head and its body"},
}

# BN5's attack families a Navi's code runs that BN6 has none of (its CurAction from 0x10, run from the families' table):
# each copied with its Navi and put in BN6's attack families grown (xattack_grow.h) at BN6's number for it, 0x50 + its
# BN5 number as the chips' are (issue #82's plan), where the entry still holds the stub. {Navi: {BN5 family: (its routine,
# its phases' routines, why)}}
FAMILIES = {
    # the Navis' buster (BN5's family 0x16: a muzzle of the image object's kind 0xD, sound 0x6A, a shot of BN5's 0x0800D928),
    # which the Dark Soul fires after it moves (its setup BN5's 0x3E, XN_STUB_BUSTER); BN6's 0x16 is MegaMan's charged shot
    # and no family of BN6's runs it
    0x16: {0x16: (0x080EC4A4, (0x080EC4C4, 0x080EC4DC, 0x080EC538), "the Navis' buster")},
}

# BN5's RAM a Navi's code names, and BN6's in its place (the RAM plan: src/emu/free_space.h, bn6_navis.h)
RAM_TWINS = {
    # Colonel's table, two 8-byte entries by side (BN5_COLONEL_TABLE): BN6_COLONEL_TABLE, zeroed as each battle is set up
    0x02034000: (0x0203FF60, 0x10, "Colonel's table (BN6_COLONEL_TABLE)"),
    # the chip AI's lists, 0xE0 bytes a side (BN5_DARK_LISTS: its chips, a count, its sequences): BN6_DARK_LISTS, zeroed as
    # each battle is set up
    0x02034C20: (0x0203FBE0, 0x1C0, "the chip AI's lists (BN6_DARK_LISTS)"),
}

# BN5's routines that set an object's sprite up from a packed word in r0: its number (bits 0-7), its list's offset (8-15),
# its animation (16-23), a flag (24-31) (0x0800F198 then its collision data from r1, 0x08018926 the sprite alone); BN6's
# twins read the word alike (0x08011504, 0x0801BE2A)
PACKED_SPRITES = (0x0800F198, 0x08018926)

# (BN5, BN6, source) pairs the families' table lacks, found for the Navis: each pair's bodies compared
# (tools/xrecipe_thumb.py) and, where they differ, audited below
TWINS = (
    # ToadMan's: its attack's element into its collision data (BN6 masks the second wheel's nibble off), its flinch's
    # status cleared (LAYOUT: the collision data's field at BN6's +0x1C, BN5's +0x1E)
    (0x08016C3A, 0x0801A146, 'navis'), (0x0801707C, 0x0801A284, 'navis'),
    # the companion overlay spawned and ended (OVERLAY5, OVERLAY6)
    (0x080C3808, 0x080C5D18, 'navis'), (0x080C382C, 0x080C5D38, 'navis'),
    # an enemy spawned mid-battle, its +5 and AIData +2 0 or 1 by its entry (BN5_CHAOS_FAIL_SPAWN the second): the same
    # body, its entries in the other order (the symbol map's pair for the first, 0x0800728C, is no twin)
    (0x08006AA4, 0x080076A0, 'navis'), (0x08006AAE, 0x08007696, 'navis'),
    # setAttack1 (BN6's of the same table, 0x08011680 + 4 n), and the attack setups by number (a table of routines that
    # write AttackVars and give CurAction: BN5's 0x0800F368, BN6's 0x080117D4)
    (0x0800F204, 0x08011684, 'navis'),
    # the count of a side's statuses its NaviStats hold, and the battle's handlers run by the bits of BattleState +0x44
    (0x0800DC0C, 0x0800FE52, 'navis'), (0x0801A410, 0x0801DACC, 'navis'),
)

# Each DIFF pair's audit (verdict, why, conditions), as tools/bn5_twins.py's AUDITS; the record is docs/rom/navis.md's
# (The Navis' engine twins)
AUDITS = {
    # (ShadeMan's: the engine's routines his drain and his bats' end call)
    (0x0800C6D8, 0x0800E2D8): ('ok', "an object's HP lowered by r0 (+0x24, at 0 the least): BN5's also counts it, where the object's AI data's "
                                     "first byte is 2, into its side's record of its own battle engine (0x02036770 + 0x1E8 a side, +0x28 less "
                                     "r0 << 7: 0x0802D4C4), after its Liberation rule raises r0 in a gate's battle (0x0800C72C, where "
                                     "0x080090E8's BattleState +0x32 bit 0x40, set by BN5's battle setup alone, stands); BN6's engine keeps "
                                     "no such record and lowers the HP alone: the same HP in both (shademan_v1_keys)", ()),
    (0x080E1534, 0x080E2718): ('ok', "an attack's end showing its user (BN6_SHOW_AFTER_HIDDEN's routine): BN5 tests the user's flags against "
                                     "0x80000004 (invisible or submerged), BN6 against 4, which src/emu/xsea.c makes see the submerged too "
                                     "(docs/rom/bn5-world.md, BN5's sea's submerge and hold); the rest the same, its calls their twins", ()),
    (0x0800E0B6, 0x08010332): ('ok', "a step's style by its side's battle NaviStats (the r2 of 0x0800F280's move): BN5's 4 where its +0x29 "
                                     "is 0, unless +0x2C is 8 (then 0), else a table's by +0x29 and +0x2B; BN6's 1 in the battle mode 9 "
                                     "(BattleSettings +3, 0x0802D234), else 4 where +0x29 is 0, else its table's: the Dark Soul's side is "
                                     "the foes' (1), whose battle NaviStats both games leave 0 in a battle of a run (BN5 0x0203C8E0, BN6 "
                                     "0x0203CE64; read in darksoul_v1), in no battle of mode 9: 4 in both",
                               ("the Dark Soul's side's NaviStats 0", "no battle of mode 9")),
    (0x0800DC0C, 0x0800FE52): ('ok', "how many of its side's statuses its battle NaviStats hold (+0x31 at 1, +0x13, +0x14, +0x16, +0x54's "
                                     "halfword, +0x24, +0x18, +0x19, +0x1A): BN6 counts +0x63 too; the Dark Soul's side's (the foes') "
                                     "battle NaviStats are 0 in both games (read in darksoul_v1): 0 in both",
                               ("the Dark Soul's side's NaviStats 0",)),
    (0x0800F204, 0x08011684): ('ok', "setAttack1: movs r2,#1, on to the common part setAttack4's audit judges (the same AttackVars set, "
                                     "BN6's clearing AttackVars +0x1D and +0x1E besides, as for BN6's own Navis)", ()),
    (0x0800F208, 0x08011688): ('ok', "setAttack2: movs r2,#2, on to the same common part (setAttack4's audit), the Dark Soul's chip AI's "
                                     "chips begun so with their family (CURACTIONS)", ()),
    (0x08015140, 0x08018810): ('ok', "an object's arm point (r0 its NameID, r1 the point, r2 its alliance, r3 its flip), which the moved image "
                                     "object reads by its kind's +4 (0x080B9C10, points 0-0x1C): BN6 answers (0, 7) for NameIDs 0xCD-0xFF "
                                     "(none of a Navi's: the moved Navis' ids are 0x1C4 on), finds list 0's record through MegaMan's form "
                                     "(0x080188B0), and its records are 0x44 bytes where BN5's 0x3C, a moved Navi's grown with BN5's 30 "
                                     "points first (src/emu/xnavi_sprites.c); its calls their twins", ()),
    (0x0800E2C2, 0x0801056A): ('ok', "an attack begun as a chip's (sub_801056A): the same steps but the chip action by AIData +0x44's "
                                     "bits 0x600: BN5's 0x2C for 0x200 else 0x2D, BN6's 0x47 for 0x200, 0x48 for 0x400, else 0x4B, each "
                                     "BN6's action the same routine as BN5's (0x080ED96F / 0x080EFC4D; 0x080EDBBF / 0x080EFD7D and "
                                     "0x080F0097, two copies): their numbers apart (the differential's action map), their code the same", ()),
    (0x08009326, 0x0800AB46): ('ok', "a side's battle counter raised (saturating at 0xFF): BN5 keeps 8 a side at 0x0203E540, BN6 16 at "
                                     "0x0203EAE0, each its own; counter 3, which ToadMan's flinch and paralysis raise, is the one both "
                                     "games' generic flinch, paralysis and drag raise (BN5 0x080141AC, 0x0801426A, 0x0801436E; BN6 "
                                     "0x08017576, 0x08017620, 0x08017700, 0x080177DC, 0x0801793E): the same counter", ()),
    (0x0800AFC2, 0x0800C90A): ('ok', "object_getPanelDataOffset: BN6 runs the same code from IWRAM (0x03007955), each game's panel at its "
                                     "own size (BN5 0x24, BN6 0x20); the Navis read its type (+2) and alliance (+3), at the same offsets "
                                     "in both, their types BN6's (PANEL_TYPES)", ()),
    (0x08016C3A, 0x0801A146): ('ok', "an object's element (+0x0E) copied into its collision data (+9): BN6 masks it with 0xF, its high "
                                     "nibble the second wheel's, which BN5's objects leave 0: the same byte", ()),
    (0x0800F210, 0x08011690): ('ok', "setAttack4: movs r2,#4, on to the common part setAttack0's audit judges (0x0800F200 / "
                                     "0x08011680): the same AttackVars set, BN6's routine of its own clearing AttackVars +0x1D and "
                                     "+0x1E besides (0x0801011A), as for BN6's own Navis", ()),
    (0x0800B050, 0x0800C998): ('ok', "a panel cracked (the falling rocks' hit, KnightMan's): the same steps on the panel's flags at "
                                     "each game's offset (BN5 +0x18, BN6 +0x14), each clearing its own bits as it sets the cracked "
                                     "type 3: BN5's 0x23F0F, BN6's 0x3F0F, BN5's 0x20000 its sea's (BN5_PANEL_FLAGS_OF's type 0xA), "
                                     "which BN6's panels hold only on xpanels.c's sea; BN6 makes a panel's whole flags word again "
                                     "from its type as it updates it (IWRAM 0x030078DC), where the bit is gone", ()),
    (0x08017A10, 0x0801AD9E): ('ok', "an obstacle's pre-step (BN6_STEP_OBSTACLE_C's routine, Colonel's objects'): BN5's steps its object "
                                     "on lava first (0x08017A3A: BN5_LAVA_STEP) where BN6's has no call, src/emu/xpanels.c hooking "
                                     "its next instruction (0x0801ADC8) with BN5's rule: the same once lava is in (xpanels_ready), the "
                                     "rest the same but for the branches past the call", ()),
    (0x08017FCC, 0x0801B394): ('ok', "an obstacle's states (the third kind's that run a virus's: Handi's bomb, Colonel's 0x3C and 0x59): "
                                     "the collision data's final damage and what hit it each game's (BN5 +0x78 and +0x68, BN6 +0x80 and "
                                     "+0x70: the collision layout); BN5's alone the block at 0x08018002-0x0801802E (as its three other "
                                     "obstacle states, 0x0801817C, 0x080182E8, 0x08018444): the dark hole, the third kind's 0x30 "
                                     "spawned (0x080CABCA) where the obstacle stands on a panel of the side opposite a Colonel whose "
                                     "side's bit of BattleState +0x5C is set and a panel beside it holds his side's (BN5_DARK_HOLE_TEST, "
                                     "0x080CAC42, 0x080CAC98). Colonel sets his bit as he is spawned, every battle he fights "
                                     "(BN5_COLONEL_SIDE_SET; the moved Colonel's write kept in BN6_COLONEL_SIDES, XN_STUB_SIDES, the "
                                     "differential comparing it with BN5's bit), so BN6's twin lacks a rule BN5 runs in his battles: "
                                     "reached by an obstacle on MegaMan's side (a chip's, a family's) beside a panel Colonel's side "
                                     "holds; BN5's block comes in at the four states' collision flags' reads for either Colonel's side "
                                     "with the core where BN5's Navis are (src/emu/xstrike.c, its soldier the Souls' moved code), "
                                     "colonel_strike holding a Navi Colonel's to BN5's (a RockCube before MegaMan, the soldier's three "
                                     "shots at him)", ()),
    (0x080C3808, 0x080C5D18): ('ok', "the companion overlay spawned (the first kind's 0x56, its parameters from r4: its kind, 0 and "
                                     "the two bytes of the companion's r2, 0 from the triples' caller, BN5 0x0800ECFE / BN6 "
                                     "0x08010DEE): the same steps but BN6 copies its Navi's +0x16 as a halfword (its alliance "
                                     "and +0x17) where BN5 copies the byte, and BN5 writes 0 to +0x60, which BN6's spawn leaves as "
                                     "its allocator zeroed it (0x080032B2): BN6's overlay reads +0x60 as a flag (0: its masks "
                                     "decide), +7 as its animation's offset and +5 as whether it takes its Navi's palette (0: it "
                                     "does), each 0 as BN5's overlay has them", ()),
}
