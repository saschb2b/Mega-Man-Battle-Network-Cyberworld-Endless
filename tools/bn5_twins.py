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

# Object types of BN5's a moved routine spawns through the second-level dispatch, each moved (BN5's routine, its art by its
# bytes) and never aimed at BN6's type of the same number: one decision per type, global (issue #82's shared allocations: a
# type is moved unless proven the same in BN6, its sprites by their bytes too; at a merge, moved wins). (kind, type): why.
# The Souls' (tools/xsoul.py, which opens a decision for a type it spawns that is not here): BN6's routine at each number
# another body (tools/xrecipe_pairs.py's deep comparison), so none could be a twin
MOVE_TYPES = {
    (1, 0x05): "the arm's flash, the image object (BN5 0x080B9B81): the allocations' amendment's, one object for every track, "
               "BN5's kinds table (0x080B9A5C, 5 bytes a kind: its sprite's list and number first) moved with it: BN6's "
               "0x080BA549 runs the same code on its own table 0x080BA444, laid out otherwise (the two agree on kinds 0-4 "
               "alone). The Souls' spawns (their code's and MegaMan's buster's in a Soul, through the moved spawner 0x080B9CAE)",
    (1, 0x54): "NumberSoul's companion (BN5 0x080C33AD, spawned by its helper at 0x080C34AC): its sprite by its own table "
               "(0x080C33A8: list 4's 0x01, which BN6 has no sprite of the same bytes of: copied, list 0's); BN6's type 0x54 "
               "is 0x080C5751, another body",
    (3, 0x05): "NumberSoul's charged shot's bomb (BN5 0x080C5015, spawned at 0x080C534C): list 3's 0x0A (no BN6 sprite of "
               "its bytes: copied), BN5's sounds 0x70 and 0x115; BN6's type 0x05 is 0x080C6E21, another body",
    (3, 0x17): "KnightSoul's charged shot's ball (BN5 0x080C7651, spawned at 0x080C7778): list 3's 0x05 (no BN6 sprite of "
               "its bytes: copied); BN6's type 0x17 is 0x080C863D, another body",
    (3, 0x81): "ToadSoul's charged shot's hit (BN5 0x080D6789, spawned at 0x080D6894); BN6's type 0x81 is 0x080D6DF1, "
               "another body",
    (3, 0x12): "the obstacle strike's shot (BN5 0x080C6C81, spawned at 0x080C6E14 by 0x080C6E0E for the strike, 0x080CAB44): its "
               "numbers by its parameter from its own table (0x080C6CB4, 6 bytes each); BN6's type 0x12 is 0x080C81B7, the same "
               "steps over its own table, other numbers (its hit's element among them)",
    (3, 0x30): "ColonelSoul's obstacle strike (BN5 0x080CA91D, spawned at 0x080CABD0 by 0x080CABCA as an obstacle on the "
               "enemy's area turns, its states' block 0x08018002-0x0801802E): it fires at the side's Colonel's table's first "
               "word (0x080CAA56); BN6's type 0x30 is 0x080CB571, another body",
    (4, 0x28): "ShadowSoul's object of the fourth kind (BN5 0x080E3619, spawned at 0x080E36E2); BN6's type 0x28 is "
               "0x080E45F9, another body",
}
