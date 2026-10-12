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
    # issue #82's A2b: the collision data's tail its setup and its update end on (BN5 0x08016760 and 0x08016B96 / BN6
    # 0x08019FB4 and 0x0801A082 call it last)
    (0x080165E4, 0x08019F44, 'families'),
    # the element a collision data takes from its object (BN5 0x080D0CDA's call, the fire column's set-up)
    (0x08016C3A, 0x0801A146, 'families'),
)

# The BN5 rows of tools/bn6f_match.py's search that a twin above corrects: its search placed a bn6f function of another
# body there (by one reference each, 'calls'), where the routine is the twin's
WRONG_ROWS = (0x080131A0, 0x080142D0, 0x0801412A, 0x080141EC, 0x080142DC)

# Each DIFF pair's audit: (verdict, why, conditions). 'ok': BN6's routine does for BN5's code what BN5's did, where the
# conditions hold (each a check tools/xrecipe.py makes on the family's code: 'no_flip' it writes no object's +0x17,
# 'flags_free' the object flags it sets are none of 0x10000 and 0x80000000, 'no_attack6' it calls no setAttack6,
# 'panel_types' the panel types it sets are numbered alike in both games, 'held_bn6' its held chips are BN6's ids);
# 'no': a moved family's code must not be aimed at it; 'copy': BN6's differs where the family's code can see it, so BN5's
# own body is copied with the family's code (its own calls through their twins), as a routine of BN5's shared code is
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
    (0x080165E4, 0x08019F44): ('ok', "the collision data's tail (its damage word's top bits, +0x2E, as statuses): BN6's sets +0x0E 1 "
                                     "where BN5's 0 for bit 14, and hands bits 13 and 12 to 0x0801A4D0 where BN5's sets +0x10 0x20 "
                                     "and 0x30, and has no bit 11 (BN5's +0x12 and +0x13): as BN6's object_setupCollisionData ends "
                                     "on it (0x08019FB4's twin, audited above), which every family's setup already runs", ()),
    (0x08016C3A, 0x0801A146): ('ok', "an object's element (+0x0E) into its collision data's +9: BN6's keeps its low four bits (and "
                                     "#0xF), the same for every element of BN5's (0-6)", ()),
    # ('copy': BN5's own body served, copied with the family's code: issue #82's A2b, where the family's code changes its
    # region and BN6's twin's fix shows; marina_v1_keys)
    (0x08016B96, 0x0801A082): ('copy', "object_updateCollisionData: BN6's stores its second word through the collision data (BN5's "
                                       "through r4, the region number its caller passed: into the BIOS's ROM, where a write does "
                                       "nothing, so BN5's object keeps the target word its setup gave it) and ORs in the time stop's "
                                       "bit as BN6's setup does: BN5's own body copied, its two calls through their twins", ()),
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
    # (issue #82's A2b: the arm's position by an object's NameID, which BN5's temp attack object asks for its owner's)
    (0x08015140, 0x08018810): ('ok', "an object's arm position by its NameID (r0): BN6 answers (0, 7) at once for its obstacles' ids "
                                     "0xCD-0xFF (0x08018812-0x08018818), then as BN5's; the families' ids are past 0x200 and MegaMan's "
                                     "0x1A0", ()),
    # (issue #82's A2b: a jump table's entry of Cactikil's and WuNote's objects of the first kind)
    (0x080138AE, 0x08016C4E): ('ok', "an object's end (its memory, collision data and place in its table let go, once): BN6's lets its "
                                     "panel reservation go first (0x0801BB78) as its object_genericDestroy does, and sets its AI data's "
                                     "+0x0E to 0xFF without BN5's call 0x08053BF4, which writes the object's HP into a Liberation "
                                     "Mission's record and only in that game mode (0x020022F0 +8 = 2), which no battle of BN6's is", ()),
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

# Tables of numbers a family's code reads ids from, each read by hand (issue #82's A2b; docs/ROM_DATA.md, The recipe
# generator's reads of BN5): its address in BN5, its bytes, its entry's size, the entries its family's code reads (the rest
# dead in the copy: no code of the family reaches them), the fields of an entry that are ids (offset, kind: 'effect' an
# effect id, 'list' a sprite list's byte offset as struct1's, 'sprite' the number in that list), the calls whose id comes
# from it (the effect's spawner, a sprite's load), and why. The table is copied whole as raw data; each live entry's ids
# are given BN6's numbers as byte fix-ups (XvByte)
ID_TABLES = (
    dict(at=0x08108900, size=2, step=1, live=(0, 1), fields=((0, 'effect'),), sites=(0x081088C8,),
         why="Zomon's two effects (its index the word at sp, 0 or 1: BN5 0x081088B2-0x081088B6), spawned at 0x081088C8"),
    dict(at=0x0810B588, size=4, step=2, live=(0, 1), fields=((0, 'list'), (1, 'sprite')), sites=(0x0810B558,),
         why="Whirly's two sprites by r0 (0 its own battle sprite, 1 list 4's 0x08: its loader 0x0810B54A, called with r0 0 at "
             "0x0810B4CC and 1 at 0x0810B27E), each a list's byte offset then a number, as sprite_load takes them (0x0810B558)"),
    dict(at=0x080D0C0C, size=0x24, step=6, live=(0,), fields=((0, 'sprite'),), list=3, sites=(0x080D0C6C,),
         why="the kinds of BN5's fire column (the third kind's type 0x56: its +0x60 the table + 6 x its +4, 0x080D0C5A-"
             "0x080D0C64; +0 its sprite in list 3, the list its code's immediate, loaded at 0x080D0C6C), six kinds; MetFire "
             "spawns kind 0 alone (its spawner 0x080D0E2C with r4 0, BN5 0x08106B42-0x08106B48), the type moved with it "
             "(TYPES: BN6's draws another sprite)"),
    dict(at=0x080C5DF4, size=0xC0, step=0x10, live=(7, 8, 9, 10), fields=((5, 'list'), (6, 'sprite')), sites=(0x080C5EEC,),
         why="the records of BN5's third kind's type 0x0B, 16 bytes a kind (+4 the object's kind: its record at the table + 16 "
             "x it, 0x080C5EDA-0x080C5EE2; +5 and +6 the sprite it loads at 0x080C5EEC), 12 kinds; Marina's water objects (its "
             "third kind's 0x8F) spawn it with kinds 7-10 (by their version: the bytes at 0x080D879C, read at 0x080D8778-"
             "0x080D877C), and no other code of the family spawns one, so kinds 0-6 and 11 are dead in the copy"),
)

# Object types whose twin in BN6 runs the same code but reads a table by the object's kind (its +4, the spawn's r4) that
# BN6 laid out otherwise, so a family's kind names another object there: never a twin, so a family that spawns one moves
# BN5's code and table with it (through the second-level dispatch; the table's live entries in ID_TABLES, or for the
# moved objects' own modules in tools/bn5_objects.py)
MOVE_TYPES = {
    (1, 0x05): "the temp attack object: BN5's kinds at 0x080B9A5C, BN6's at 0x080BA444, 5 bytes a kind (its sprite's list and "
               "number first), read by 0x080B9CE0 / 0x080BA6D2 (the same code); the two agree on kinds 0-4 alone (BN5's 0x3A "
               "kinds, BN6's 0x34, reordered): moved once for every track as its module whole (tools/bn5_objects.py, IMAGE), "
               "its kinds fixed there",
}

# Numbers its code keeps as they are, read by hand in BN6 bit by bit (the generator's kept literals: a mask a word is tested
# with, the words a spawn's parameters are made of). A number the generator keeps that is not here is a decision
ALLIANCE_BITS = ("a collision data's words (+0x30, +0x34, and what hit it, BN5's +0x68, BN6's +0x70) and a panel's flags carry "
                 "one set of bits in both games: 0x00800000 a neutral support object, 0x01000000 an enemy's, 0x02000000 an ally's, "
                 "0x04000000 the enemy's alliance, 0x08000000 the ally's (bn6f's PanelData flags); the alliance tables the setup "
                 "reads them from are the same words in both but for bit 0x80 (BN5 0x08016364, BN6 0x08019C7C: tests/xdiff/"
                 "legit.json's alliance), and BN6's own code tests these masks the same way")
SPAWN_WORDS = ("the fourth word of a collision region's spawn (r4 of BN5 0x080C4E72, BN6 0x080C6BDA, a routine of the same body: "
               "its bytes the hit's parameters, read alike), which BN6's own code passes the same")
KEPT = {
    0x08000000: ALLIANCE_BITS + ": the ally's alliance (BN6's twin of BN5's shared 0x08104BBC, 0x0810A9F2, tests 0x0C000000, "
                                "the enemy's beside it: a BN5 virus is struck by the ally's side alone, so the two agree on its hits)",
    0x03800000: ALLIANCE_BITS + ": the three support objects (BN6 passes it to checkPanelParameters' twin 0x0800CC86 at "
                                "0x0800F546 and 0x080F57CE, and tests a panel's flags with it at 0x080C7386, 0x080F1F26)",
    0x06800000: ALLIANCE_BITS + ": a neutral and an ally's support object and the enemy's alliance (BN6 passes it to "
                                "checkPanelParameters' twin at 0x080CD72C and 0x080CD7AE as BN5 does)",
    0x0A800000: ALLIANCE_BITS + ": a neutral and an ally's support object and the ally's alliance (no code of BN6's names "
                                "this mask; its bits each as above)",
    0x0405FF00: SPAWN_WORDS + " (BN6 makes one of it at 0x080BEAC8, adding a number into its low byte as BN5's Powie ORs one)",
    0x0405FF10: SPAWN_WORDS + " (BN6's own three: 0x080C68E8, 0x080D922C, 0x0810F9A8)",
    0x04050101: SPAWN_WORDS + " (BN6's own, 0x080CC928, 0x080D1636 among them)",
    0x04050401: SPAWN_WORDS + " (none of BN6's: its bytes as 0x04050101's, its third byte 4)",
    0x0705FF01: SPAWN_WORDS + " (BN6's own, 0x080D757A, 0x080F0284 among them)",
    0x06050A01: SPAWN_WORDS + " (BN6's own, 0x080BD392, 0x080DC8F8 among them)",
}

# BN5's virus dispatcher's pre-step (BN5 0x08104550, BN6's twin 0x0810A29C: issue #82's A2b): before a virus's state each
# frame BN5 acts for two families' NameIDs, which BN6's dispatcher does for its own ids alone. Each case by its first
# instruction past the range test, as the installer ports it (src/emu/xvirus_shim.c: BN5's instructions, the collision
# data's offsets BN6's, the routine each calls BN5's own body likewise, no BN6 twin of it doing the same: BN6's role twins
# 0x0800EA22 and 0x0800EA4E clear its freeze and bubble bits and keep the slide's (0x300EE where BN5 0xFE) and write its
# collision data's +0 too)
PRESTEP = {
    0x0810455C: ('appley', 0x0800CD1E, "Appley's (ids 0x9D-0xA2): while its +0x60 and its collision data's +0x7C are 0, "
                                       "its pending hit's damage (+0x78) taken by BN5 0x0800CD1E (the statuses it leaves "
                                       "cleared, flags 0xFE), sound 0x6E where there is one"),
    0x08104588: ('dominerd', 0x0800CD44, "Dominerd's (ids 0x19-0x1E): while its +0x68 is 0 and a hit is pending (+0x78), "
                                         "taken by BN5 0x0800CD44 (flags 0x1FE, at least 1 left) unless the hit's flags "
                                         "(+0x68) hold 2; its +0x64 set"),
}

# Each object type of BN5's a track's objects come to be, and the decision for it (issue #82's plan for its tracks, binding
# for every track: one decision a type, whichever track's code spawns it; at a merge of two tracks' decisions, a moved one
# wins): 'moved' (its code copied, run through the second-level dispatch as BN5's number, src/emu/xvirus_dispatch.h)
# unless BN6's type of the same number runs the same code (Pairs.deep_same) and draws the same sprites by their bytes
# (type_sprites, art_same): then 'twin', BN6's number; 'unreachable' where only BN5's panel claims spawn it.
# tools/xrecipe.py and tools/xnavirecipe.py derive each and refuse a recipe whose type is recorded otherwise or not at all.
# (kind, BN5's type): (decision, BN6's type or None, the track that first spawned it, why)
NO_TWIN = 'no BN6 type runs the same code (tools/xrecipe_family.py, type_twin: each type of the kind\'s table compared deep)'
TYPES = {
    # (the Navis')
    (3, 0x25): ('moved', None, 'families', "CanGard's reticle: BN6's 0x25 another object's code"),
    (1, 0x04): ('moved', None, 'navis', "ShadowMan's: " + NO_TWIN),
    (1, 0x05): ('moved', None, 'navis', "the image object (the Navis' kinds 3 and 0x25, WuNote's 0x30): BN6's type 5 reads a kinds table of "
                                        "its own (MOVE_TYPES), other sprites at other numbers; moved once for every track, its module whole "
                                        "(tools/bn5_objects.py), installed before any family or Navi"),
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
    # (the families')
    # (moved: what a family's code spawns through the second-level dispatch; no BN6 type of the number runs the same code
    # and draws the same sprites)
    (1, 0x11): ('moved', None, 'families', "Cactikil's: BN6's 0x11 another object's code"),
    (1, 0x16): ('moved', None, 'families', "WuNote's: BN6's 0x16 another object's code"),
    (1, 0x2F): ('moved', None, 'families', "Appley's: BN6's 0x2F another object's code"),
    (3, 0x0B): ('moved', None, 'families', "Marina's water's records (ID_TABLES): BN6's 0x0B another object's code"),
    (3, 0x24): ('moved', None, 'families', "Bugtank's thrown bomb: BN6's 0x24 another object's code"),
    (3, 0x3E): ('moved', None, 'families', "BomBoy's bomb: BN6's 0x3E another object's code"),
    (3, 0x56): ('moved', None, 'families', "the fire column (MetFire's): BN6's 0x56 another sprite (list 3's 0x31, not the same bytes), its "
                                           "kinds 2 and 4 otherwise and only three kinds; moved with its table (ID_TABLES)"),
    (3, 0x61): ('moved', None, 'families', "BigBrute's: BN6's 0x61 another object's code"),
    (3, 0x6A): ('moved', None, 'families', "Batty's: BN6's 0x6A another object's code"),
    (3, 0x6F): ('moved', None, 'families', "Flashy's and Eleogre's flash: BN6's 0x6F another object's code (Flashy's copy runs for both)"),
    (3, 0x84): ('moved', None, 'families', "WuNote's: BN6's 0x84 another object's code"),
    (3, 0x87): ('moved', None, 'families', "Draggin's: BN6's 0x87 another object's code"),
    (3, 0x8F): ('moved', None, 'families', "Marina's water: BN6's 0x8F another object's code"),
    (3, 0xAC): ('moved', None, 'families', "Appley's: BN6's 0xAC another object's code"),
    (3, 0xC1): ('moved', None, 'families', "Skarab's: BN6's 0xC1 another object's code"),
    (4, 0x78): ('moved', None, 'families', "Drixol's: BN6's 0x78 another object's code"),
    (4, 0x90): ('moved', None, 'families', "Appley's: BN6's 0x90 another object's code"),
    # (twin: spawned by a routine of BN5's engine whose BN6 twin spawns BN6's own of the number, the same code but for each
    # game's layouts and numbers, the same sprites by their bytes)
    (3, 0x03): ('twin', 0x03, 'families', "the collision region (BN5 0x080C4E72's, BN6 0x080C6BDA's): its routines the same but for the "
                                          "collision data's flags at +0x70 where +0x68 and the audited object's end; no sprite"),
    (3, 0x3B): ('twin', 0x3B, 'families', "Lark's (BN5 0x080CCA62's): its routines the same but for its sprite's number (list 4's 0x1D in BN5, "
                                          "0x37 in BN6, the same bytes) and the collision data's layout"),
    (3, 0x4B): ('twin', 0x4B, 'families', "Handi's bomb (its spawner BN5 0x080CF37E / BN6 0x080CF4EC): its code BN6's with BN6's state "
                                          "numbering (its own two on, legit.json's t3_types), BN5's clearing of a battle state entry where "
                                          "no collision data is left to make aside; its sprite list 3's 0x23 the same bytes; the "
                                          "differential's handi specs the same frame for frame"),
    (4, 0x00): ('twin', 0x00, 'families', "the effect object (BN5 0x080DFFD2's, BN6 0x080E1932's): the same code but for each game's sprite "
                                          "routines; its sprite by its effect id's row, each game's art for an id (a family's own effects "
                                          "copied under ids of their own where BN5's differ: effect_to)"),
    (4, 0x11): ('twin', 0x11, 'families', "Cactikil's (BN5 0x080E1BFE's): its routines the same bodies; no sprite"),
    (4, 0x35): ('twin', 0x35, 'families', "the fire column's spark of flame (BN5 0x080E450E's, BN6 0x080E5736's): its routines the same "
                                          "bodies (code and type deep the same); no sprite of its own (it draws through the effect object)"),
    # (unreachable: spawned only behind BattleState+0x32 bit 0x40, which BN5's 0x0802D594 alone sets and BN6's engine never
    # does: issue #82's shared allocations, BN5's panel claims stubbed everywhere)
    (4, 0x83): ('unreachable', None, 'families', "the panel claim the fire column's spark asks for (BN5 0x080EA08C, behind 0x080090E8's "
                                                 "test of BattleState+0x32 bit 0x40): BN6's twin 0x080E5736 left to it"),
}
