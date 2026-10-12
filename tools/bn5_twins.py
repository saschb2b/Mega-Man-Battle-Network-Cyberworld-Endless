"""bn5_twins.py: BN5 Team Colonel's routines and their twins in BN6 Cybeast Gregar (USA), as this project paired them
(issue #82): numbers only, each pair's two bodies compared by tools/xrecipe_thumb.py (SAME, LAYOUT: but for the offsets
of loads and stores, or DIFF), each DIFF judged for what BN5's code asks of the routine (AUDITS). Read by
tools/xrecipe.py, which aims a moved family's calls at the twins, and by tools/bn6f_match.py, which writes them into its
BN5 table where its search placed none or the wrong function (docs/SYMBOLS.md, Twins of the project's). The record of
each audit is docs/ROM_DATA.md's (The engine's differing twins)."""

# (BN5, BN6, where the pair was found and checked): a routine's entry in each game. 'cangard' and 'handi' are the pairs
# of their recipes (src/emu/xvirus_families.c), each body compared as they were built and their families compared with
# BN5's own frame by frame since (tools/xdiff.py); 'research' the research's pairs the symbol map lacked (issue #82's
# engine survey); 'navis' the Navis' survey's; 'search' and 'near' tools/xrecipe.py's own finds (a BN6 routine of the
# same body, its tables' entries the same too; or the same body where its neighbours put it)
TWINS = (
    # CanGard's calls
    (0x08016C42, 0x0801A152, 'cangard'), (0x08016C4C, 0x0801A15C, 'cangard'), (0x08016C56, 0x0801A166, 'cangard'),
    (0x0800F29E, 0x08011714, 'cangard'), (0x0800F200, 0x08011680, 'cangard'), (0x08002C56, 0x08002DEA, 'cangard'),
    (0x0800CCD2, 0x0800E9DC, 'cangard'), (0x0800C676, 0x0800E276, 'cangard'), (0x080DFFD2, 0x080E1932, 'cangard'),
    (0x08008A1C, 0x0800A03C, 'cangard'), (0x08008A78, 0x0800A098, 'cangard'), (0x0800E140, 0x080103BC, 'cangard'),
    (0x080133CE, 0x080165C2, 'cangard'), (0x08008B6E, 0x0800A18E, 'cangard'), (0x08003178, 0x08003358, 'cangard'),
    (0x080186A8, 0x0801BBAC, 'cangard'), (0x080138EA, 0x08016C8A, 'cangard'), (0x0800B328, 0x0800CC66, 'cangard'),
    (0x08002650, 0x080026E4, 'cangard'), (0x08002DF4, 0x08002F90, 'cangard'), (0x08002C10, 0x08002DA4, 'cangard'),
    (0x08002610, 0x080026A4, 'cangard'), (0x08002BEC, 0x08002D80, 'cangard'), (0x0800C88E, 0x0800E456, 'cangard'),
    (0x08002DC0, 0x08002F5C, 'cangard'), (0x0800C6CA, 0x0800E2CA, 'cangard'), (0x0800C69C, 0x0800E29C, 'cangard'),
    (0x0800C6AC, 0x0800E2AC, 'cangard'), (0x0800325C, 0x08003458, 'cangard'), (0x08016126, 0x08019892, 'cangard'),
    (0x08016760, 0x08019FB4, 'cangard'), (0x08016C34, 0x0801A140, 'cangard'), (0x08016B88, 0x0801A074, 'cangard'),
    (0x08016B90, 0x0801A07C, 'cangard'), (0x08016B60, 0x0801A04C, 'cangard'), (0x08016B1A, 0x0801A018, 'cangard'),
    (0x080167B0, 0x0801A00E, 'cangard'), (0x08016C04, 0x0801A0D4, 'cangard'), (0x08000598, 0x080005CC, 'cangard'),
    # Handi's calls, and the engine's steps its jump table names
    (0x080014A6, 0x08001532, 'handi'), (0x08012DCC, 0x08015FDC, 'handi'), (0x080CF37E, 0x080CF4EC, 'handi'),
    (0x08104B1C, 0x0810A960, 'handi'), (0x08104E60, 0x0810AD80, 'handi'), (0x0810567C, 0x0810AA2A, 'handi'),
    (0x08104DA8, 0x0810ACC8, 'handi'), (0x08104DEA, 0x0810AD0A, 'handi'), (0x08104BFC, 0x0810AE94, 'handi'),
    # the research's
    (0x08013944, 0x08016CE8, 'research'), (0x0800C896, 0x0800E45E, 'research'), (0x0800D2E8, 0x0800F404, 'research'),
    (0x0800FE6A, 0x080126E4, 'research'), (0x0800B380, 0x0800CCBE, 'research'), (0x08012DA4, 0x08015FB4, 'research'),
    (0x080131A0, 0x08016380, 'research'), (0x08013824, 0x08016B02, 'research'), (0x080133C4, 0x080165B8, 'research'),
    # the Navis' survey's: their generic states (the symbol map placed other functions at four), setAttack2, the dying
    # state's two, a sound helper, the pre-step's two entries
    (0x080142D0, 0x08017888, 'navis'), (0x0801412A, 0x080174FE, 'navis'), (0x080141EC, 0x080175B8, 'navis'),
    (0x080142DC, 0x080178B6, 'navis'), (0x0800F208, 0x08011688, 'navis'), (0x08013D1C, 0x080170C4, 'navis'),
    (0x08013E02, 0x080171A6, 'navis'), (0x08000BC6, 0x08000CA6, 'navis'), (0x08017666, 0x0801A9A4, 'navis'),
    (0x0801766C, 0x0801A9AA, 'navis'),
)

# The BN5 rows of tools/bn6f_match.py's search that a twin above corrects: its search placed a bn6f function of another
# body there (by one reference each, 'calls'), where the routine is the twin's
WRONG_ROWS = (0x080131A0, 0x080142D0, 0x0801412A, 0x080141EC, 0x080142DC)

# Each DIFF pair's audit: (verdict, why, conditions). 'ok': BN6's routine does for BN5's code what BN5's did, where the
# conditions hold (each a check tools/xrecipe.py makes on the family's code: 'no_flip' it writes no object's +0x17,
# 'flags_free' the object flags it sets are none of 0x10000 and 0x80000000, 'no_attack6' it calls no setAttack6,
# 'panel_types' the panel types it sets are numbered alike in both games, 'held_bn6' its held chips are BN6's ids);
# 'no': a moved family's code must not be aimed at it
AUDITS = {
    # CanGard's (its recipe's, compared since frame by frame with BN5's own)
    (0x0800F29E, 0x08011714): ('ok', "the attack ended: BN6's also clears AttackVars +0x1B, clears AIData +0x44's bits 0x1000003F where "
                                     "BN5's clear 0x0001803F (each game's own bits), and writes CurAction 8 where BN5's 6 (BN6's two "
                                     "status states first); BN5's case of attack kind 6 (setAttack6, which BN6 lacks) aside", ('no_attack6',)),
    (0x0800F200, 0x08011680): ('ok', "setAttack0: movs r2,#0, on to the common part, BN6's a routine of its own (push, a call, pop) where "
                                     "BN5's returns by mov pc,lr: the same AttackVars set", ()),
    (0x0800CCD2, 0x0800E9DC): ('ok', "an object's counter time: BN5 skips a player of AI 0x17; no virus is one", ()),
    (0x080138EA, 0x08016C8A): ('ok', "an object's end: BN6's lets its panel reservation go first (0x0801BB78) and frees its collision data "
                                     "and memory by its own twins", ()),
    (0x08016126, 0x08019892): ('ok', "a collision data made: BN6's are 0xA8 bytes where BN5's 0xA0 (its own table at its own address)", ()),
    (0x08016760, 0x08019FB4): ('ok', "a collision data set up: BN6's keeps the region's high nibble apart at +0x19; its calls their twins", ()),
    (0x08016B1A, 0x0801A018): ('ok', "what hit an object cleared: BN6's one word at +0x70 where BN5 splits it by side (+0x6C, +0x70, and "
                                     "+0x18, +0x19: tests/xdiff/legit.json's rule)", ()),
    (0x080167B0, 0x0801A00E): ('ok', "an object's presence on its panels: BN6's runs the same code from IWRAM", ()),
    (0x08016C04, 0x0801A0D4): ('ok', "a hit's effect spawned: BN6's reads the hit's flags at +0x70 (BN5's +0x68), its calls their twins", ()),
    # the twelve the other families call (issue #82's capacity survey, open item 1)
    (0x08018654, 0x0801BB1C): ('ok', "object_reservePanel: the panel's reserver and count at BN6's +0x1C and +0x14 (BN5's +0x20, +0x18), "
                                     "and BN6 marks the object (+0 bit 0x20, which only BN6's own code reads)", ()),
    (0x0800D010, 0x0800ED90): ('ok', "the chip an object uses: a virus's path is the same ([r5,#0x2A], 0, 0, [r5,#0x1A]); BN6 adds a "
                                     "player's attack bonus on MegaMan's path alone", ('held_bn6',)),
    (0x0800FE6A, 0x080126E4): ('ok', "a chip's numbers written for its attack: BN6 ORs its chip record's icon element's hit flag in "
                                     "(0x080129E4): right for a BN6 chip id, which the family's held chips are", ('held_bn6',)),
    (0x08016B96, 0x0801A082): ('ok', "object_updateCollisionData: BN6's stores its second word through the collision data (BN5's through "
                                     "r4, a bug) and ORs in the time stop's bit as BN6's setup does", ()),
    (0x080133C4, 0x080165B8): ('ok', "on to the decide state: CurAction 8 where BN5's 6, BN6's two status states first", ()),
    (0x0801A5BC, 0x0801DC7C): ('ok', "an object put in a four-slot table (BN5 0x0203E5C0, BN6 0x0203EB50, each its own): BN6 marks its "
                                     "own ids 0x49-0x4E and 0xEB otherwise (0x1B); every other id the same (3)", ()),
    (0x0800B380, 0x0800CCBE): ('ok', "the panels of an object's collision region highlighted: BN6 turns them by its DirectionFlip (+0x17) "
                                     "too; the same where the family never flips it", ('no_flip',)),
    (0x0800C7CC, 0x0800E3BE): ('ok', "a hit's damage summed (its collision data's five, at BN6's +0x82 where BN5's +0x7A): BN5 doubles it "
                                     "in a Liberation Mission's late turns (0x0800901E), which BN6's battles never are", ()),
    (0x08013B04, 0x08016E64): ('ok', "an object's sprite animated while it lives: BN6 also holds it on flags 0x10000 and 0x80000000 and "
                                     "lets its own ids 0x49-0x4E animate at 0 HP; the same where the family sets neither flag",
                               ('flags_free',)),
    (0x0800AFF0, 0x0800C938): ('ok', "object_crackPanel: the panel's fields at BN6's offsets, BN5's sea bit (0x20000) out of its masks, "
                                     "its sounds by their twins", ()),
    (0x0800F2A6, 0x0801171C): ('ok', "the attack ended (the wait's form): as 0x0800F29E's twin", ('no_attack6',)),
    (0x0800B2A6, 0x0800CC0A): ('ok', "object_setPanelType: BN6's (in IWRAM) times its conveyors 9-12 where BN5 times its lava 8 and sea "
                                     "10; types 0-4, 6 and 7 are the same panels in both", ('panel_types',)),
}

# panel types both games number alike (docs/ROM_DATA.md, Battlefields and their panels' types)
SAME_PANEL_TYPES = (0, 1, 2, 3, 4, 6, 7)

# Tables of BN5's ROM its families' code names and their twins in BN6 (BN5, its bytes, BN6, how found): a literal naming
# a place in one is BN6's at the same distance
DATA_TWINS = (
    (0x0801629C, 32 * 4, 0x08019B78, "the panel patterns by region (BN6's 47, its first 32 the same patterns: docs/MULTIROM.md, "
                                     "the Navis' survey)"),
    (0x08005CD0, 0x80, 0x080065E0, 'a sine table (its 0x80 bytes the same)'),
    (0x08005D50, 0x80, 0x08006660, 'a cosine table (its 0x80 bytes the same)'),
)

# The second-level dispatch's decision for each BN5 object type a track's code spawns (issue #82's plan for its tracks):
# MOVED (BN5's routine for the type copied, src/emu/xvirus_dispatch.h) unless BN6 runs the same code for a type of its own
# and draws the same art (its sprites compared by their bytes); TWIN that BN6 type. One decision a type, whichever track's
# code spawns it; at a merge of two tracks' decisions, MOVED wins. tools/xnavirecipe.py refuses a recipe whose type is not
# decided here as it decides it (the families' entries are the families track's to add). (kind, BN5's type): (decision,
# BN6's type or None, the track that first spawned it, why)
NO_TWIN = 'no BN6 type runs the same code (tools/xrecipe_family.py, type_twin: each type of the kind\'s table compared deep)'
TYPES = {
    (3, 0x25): ('moved', None, 'families', "CanGard's: " + NO_TWIN),
    (1, 0x04): ('moved', None, 'navis', "ShadowMan's: " + NO_TWIN),
    (1, 0x05): ('moved', None, 'navis', "the image object: BN6's type 5 reads a kinds table of its own, other sprites at other numbers; moved "
                                        "once for every track, its module whole (tools/bn5_objects.py)"),
    (1, 0x54): ('moved', None, 'navis', "NumberMan's companion: BN6's twin of its spawner spawns BN6's 0x55, no deep twin of BN5's 0x080C33AD"),
    (1, 0x56): ('twin', 0x56, 'navis', "the companion overlay: the same code (its spawner and end twins, 0x080C3808/0x080C5D18 and "
                                       "0x080C382C/0x080C5D38); BN5's kinds BN6 lacks take kinds past BN6's 19, their masks and sprites "
                                       "BN5's bytes (src/emu/xnavi_recipe.h, XnOverlay)"),
    (3, 0x02): ('moved', None, 'navis', "Colonel's: " + NO_TWIN),
    (3, 0x05): ('moved', None, 'navis', "NumberMan's: " + NO_TWIN),
    (3, 0x07): ('moved', None, 'navis', "ShadowMan's: " + NO_TWIN),
    (3, 0x14): ('moved', None, 'navis', "KnightMan's wrecking ball: " + NO_TWIN),
    (3, 0x15): ('moved', None, 'navis', "KnightMan's: " + NO_TWIN),
    (3, 0x17): ('moved', None, 'navis', "KnightMan's: " + NO_TWIN),
    (3, 0x1D): ('moved', None, 'navis', "BN5's falling rocks: BN6's 0x1D another rock (its height, sprite 4:5 where BN5's 4:0xE, its collision; "
                                        "knightman_v1 at t613)"),
    (3, 0x1F): ('moved', None, 'navis', "TomahawkMan's: " + NO_TWIN),
    (3, 0x20): ('moved', None, 'navis', "TomahawkMan's: " + NO_TWIN),
    (3, 0x21): ('moved', None, 'navis', "TomahawkMan's: " + NO_TWIN),
    (3, 0x22): ('moved', None, 'navis', "TomahawkMan's: " + NO_TWIN),
    (3, 0x23): ('moved', None, 'navis', "TomahawkMan's: " + NO_TWIN),
    (3, 0x3C): ('moved', None, 'navis', "Colonel's strike: BN6's 0x3C strikes with effect 0x60, list 4's 0xA, where BN5's with 0x47, list 4's "
                                        "0x2C (colonel_v1_keys at t1003)"),
    (3, 0x59): ('moved', None, 'navis', "Colonel's: BN6's 0x080D11C5 no deep twin of BN5's 0x080D1089, its sprite's frames apart (colonel_sp "
                                        "at t777)"),
    (3, 0x7B): ('moved', None, 'navis', "ToadMan's: " + NO_TWIN),
    (3, 0x7C): ('moved', None, 'navis', "ToadMan's: " + NO_TWIN),
    (3, 0x81): ('moved', None, 'navis', "ToadMan's: " + NO_TWIN),
    (3, 0x8B): ('moved', None, 'navis', "NumberMan's: " + NO_TWIN),
    (3, 0x8C): ('moved', None, 'navis', "NumberMan's: " + NO_TWIN),
    (3, 0x93): ('moved', None, 'navis', "NumberMan's: " + NO_TWIN),
    (4, 0x09): ('moved', None, 'navis', "BN5's rock fragment: BN6's 9 draws its sprite 4:5 where BN5's 4:0xE (knightman_v1 at t685)"),
    (4, 0x3E): ('moved', None, 'navis', "the sea's marker (tools/bn5_objects.py, SEA_MARKER): BN6's 0x3E its invisibility's shadow, the "
                                        "same code drawing list 3's 0x00 where BN5's draws 0x2D, and nothing of BN6's spawns it"),
    (4, 0x38): ('moved', None, 'navis', "Colonel's strike's two: " + NO_TWIN + "; BN6's 0x38 sets another palette (+0x95) and draws "
                                        "otherwise (colonel_sp at t1004)"),
    (4, 0x8E): ('moved', None, 'navis', "NumberMan's: " + NO_TWIN),
}
