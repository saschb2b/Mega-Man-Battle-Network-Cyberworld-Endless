"""bn5_navi_twins.py: the Navis' own pairs of BN5 Team Colonel's routines with their twins in BN6 Cybeast Gregar (USA),
and their audits, beside tools/bn5_twins.py's (the families'), read by tools/xnavirecipe.py (issue #82; docs/ROM_DATA.md,
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
         0x0802B804: 'XN_STUB_DS'}

# BN5's panel types as BN6's (tests/xdiff/legit.json, maps, panel_type): its lava and sea BN6's 13 and 14 where
# src/emu/xpanels.c brings them in, its holy BN6's 5; a Navi's code that sets a panel's type or compares one read from a
# panel (its +2) takes BN6's number
PANEL_TYPES = {0x8: 0xD, 0x9: 0x5, 0xA: 0xE}
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
}
# BN5's RAM a Navi's code names, and BN6's in its place (the RAM plan: src/emu/free_space.h, bn6_navis.h)
RAM_TWINS = {
    # Colonel's table, two 8-byte entries by side (BN5_COLONEL_TABLE): BN6_COLONEL_TABLE, zeroed as each battle is set up
    0x02034000: (0x0203FF60, 0x10, "Colonel's table (BN6_COLONEL_TABLE)"),
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
)

# Each DIFF pair's audit (verdict, why, conditions), as tools/bn5_twins.py's AUDITS; the record is docs/ROM_DATA.md's
# (The Navis' engine twins)
AUDITS = {
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
                                     "+0x70: the collision layout); BN5's alone the block at 0x08018002-0x0801802E, a dark hole "
                                     "spawned where its panel's side's bit 0x10 or 0x20 of BattleState +0x5C is set (0x080CABEA "
                                     "reads it through 0x08001336, 0x080CABCA spawns the third kind's 0x30), which BN5 sets to 0x0C "
                                     "as each battle is set up (0x08008F64, its setter's one caller): none of the two bits in a "
                                     "battle of a run", ()),
    (0x080C3808, 0x080C5D18): ('ok', "the companion overlay spawned (the first kind's 0x56, its parameters from r4: its kind, 0 and "
                                     "the two bytes of the companion's r2, 0 from the triples' caller, BN5 0x0800ECFE / BN6 "
                                     "0x08010DEE): the same steps but BN6 copies its Navi's +0x16 as a halfword (its alliance "
                                     "and +0x17) where BN5 copies the byte, and BN5 writes 0 to +0x60, which BN6's spawn leaves as "
                                     "its allocator zeroed it (0x080032B2): BN6's overlay reads +0x60 as a flag (0: its masks "
                                     "decide), +7 as its animation's offset and +5 as whether it takes its Navi's palette (0: it "
                                     "does), each 0 as BN5's overlay has them", ()),
}
