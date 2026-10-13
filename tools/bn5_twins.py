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
    # the element a collision data takes from its object (BN5 0x080D0CDA's call, the fire column's set-up; the chips' WoodNos's,
    # CactBal's and the misc family's objects' too, where the symbol map has none: BN6's of the same steps, a mask besides)
    (0x08016C3A, 0x0801A146, 'families'),
    # the chips' (tools/xchiprecipe.py): the support objects' generic end substate, where the symbol map placed a routine of
    # another body (BN6's 0x080EA56C); the same body at BN6's 0x0800BC88, the substate BN6's own support objects name
    (0x0800A2F0, 0x0800BC88, 'chips'),
    # an obstacle's pre-step (BoyBomb's bomb's), where the symbol map has none: BN6's of the same steps but the lava's call
    # (its audit below)
    (0x08017A70, 0x0801ADFA, 'chips'),
    # MegaMan's flash after a reaction (BN5's family 0x49's end), where the symbol map has none: BN6's of the same steps, the
    # flash timer at its own place in the collision data (BN5 +0x26, BN6 +0x24: legit.json's layout; its audit below)
    (0x0801742A, 0x0801A66C, 'chips'),
    # an object's HP recovered (RedFrut's fruit's), where the symbol map has none: BN6's of the same steps, AntiRecv by each
    # game's own id (its audit below)
    (0x0800C768, 0x0800E2FC, 'chips'),
    # the battle's events by bit (BN5's 0x0801A410 the bombs', HolyDrem's), where the symbol map has none: BN6's of the same
    # dispatch, each game's own handlers (its audit below)
    (0x0801A410, 0x0801DACC, 'chips'),
    # MegaMan's NaviCust bugs counted (the Tornado family's subfamily 2's, BN5 0x080ECEC0), where the symbol map has none: BN6's of
    # the same steps over the same fields of his battle NaviStats but one more, +0x63 (its audit below)
    (0x0800DC0C, 0x0800FE52, 'chips'),
)

# The BN5 rows of tools/bn6f_match.py's search that a twin above corrects: its search placed a bn6f function of another
# body there (by one reference each, 'calls'), where the routine is the twin's
WRONG_ROWS = (0x080131A0, 0x080142D0, 0x0801412A, 0x080141EC, 0x080142DC, 0x0800A2F0)

# Each DIFF pair's audit: (verdict, why, conditions). 'ok': BN6's routine does for BN5's code what BN5's did, where the
# conditions hold (each a check tools/xrecipe.py makes on the family's code: 'no_flip' it writes no object's +0x17,
# 'flags_free' the object flags it sets are none of 0x10000 and 0x80000000, 'no_attack6' it calls no setAttack6,
# 'panel_types' the panel types it sets are numbered alike in both games, 'held_bn6' its held chips are BN6's ids);
# 'no': a moved family's code must not be aimed at it; 'copy': BN6's differs where the family's code can see it, so BN5's
# own body is copied with the family's code (its own calls through their twins), as a routine of BN5's shared code is
AUDITS = {
    # the chips' (tools/xchiprecipe.py's HolyDrem and Poltrgst: their doubles of MegaMan)
    (0x0800DD8C, 0x0801002C): ('ok', "MegaMan's palette (r5 him): each game's own for his body (BN5 0x0800DE90, BN6 0x080100EC call it): "
                                     "BN5's by his Soul (0, 2 in Chaos Unison), mood and dark meter (dark 2 or 3 by 470, Full Synchro 4, "
                                     "else 4, 0, 3, 2 by the meter at 1000, 500, 470) and NaviStats +0x4C; BN6's by his form's table "
                                     "(0x080203EA), Full Synchro 4, else 0, with the Souls' read hooked to BN5's 0 or 2 (0x08010066, "
                                     "src/emu/soullook.c) and the mood's to BN5's dark 2 or 3 (0x0801006A, src/emu/soulmood.c). "
                                     "HolyDrem's and Poltrgst's double of him (the first kind's 0x1C, 0x58) draws his sprite by BN6's "
                                     "selector (MEGAMAN_LOADS), so it wants his palette as BN6 draws him: BN6's own, no meter "
                                     "coming into a run", ()),
    # the chips' (tools/xchiprecipe.py's Tornado: DarkTorn's family)
    (0x0800DC0C, 0x0800FE52): ('ok', "MegaMan's NaviCust bugs counted (his battle NaviStats +0x31 at 1, +0x13, +0x14, +0x16, +0x54, +0x24, "
                                     "+0x18, +0x19, +0x1A each one): BN6's counts +0x63 too, its Custom screen's bug's turns, which no "
                                     "battle of a run's gives him; called by the Tornado family's subfamily 2 alone (0x080ECEC0), which no "
                                     "chip of the registry plays (DarkTorn its 3)", ()),
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
    # (issue #82's A2b: a jump table's entry of Cactikil's and WuNote's objects of the first kind)
    (0x080138AE, 0x08016C4E): ('ok', "an object's end (its memory, collision data and place in its table let go, once): BN6's lets its "
                                     "panel reservation go first (0x0801BB78) as its object_genericDestroy does, and sets its AI data's "
                                     "+0x0E to 0xFF without BN5's call 0x08053BF4, which writes the object's HP into a Liberation "
                                     "Mission's record and only in that game mode (0x020022F0 +8 = 2), which no battle of BN6's is", ()),
    # the chips' code's (tools/xchiprecipe.py; issue #82's C3); the four after the first the Navis' survey's too (their
    # tools/bn5_navi_twins.py), the bodies compared again here
    (0x0800AFC2, 0x0800C90A): ('ok', "object_getPanelDataOffset: BN6 runs the same code from IWRAM (0x03007955: bn6f's "
                                     "_object_getPanelDataOffset, x and y in range, (y x 8 + x) x the panel's size), each game's "
                                     "panel at its own size (BN5 0x24 at 0x0203A100, BN6 0x20); a unit reads its type (+2) and "
                                     "alliance (+3), the same offsets in both, its types BN6's (tools/xchiprecipe.py checks both)", ()),
    (0x08017FCC, 0x0801B394): ('ok', "an obstacle's states (the third kind's that run as a virus: BoyBomb's bomb, DrkSonic's): the "
                                     "collision data's final damage and what hit it each game's (BN5 +0x78 and +0x68, BN6 +0x80 and "
                                     "+0x70: the collision layout); BN5's alone a dark hole spawned (0x08018002-0x0801802E) where its "
                                     "panel's side's bit 0x10 or 0x20 of BattleState +0x5C is set, which BN5 sets to 0x0C as each "
                                     "battle is set up (0x08008F64): none of the two in a battle of a run", ()),
    (0x0800931C, 0x0800AB3A): ('ok', "a side's battle counter read: BN5 keeps 8 a side at 0x0203E540, BN6 16 at 0x0203EAE0, each its own; "
                                     "counter 3 the one both games' flinch, paralysis and drag raise (the Navis' audit of its writer, "
                                     "0x08009326 / 0x0800AB46), the DS chips' power read by it", ()),
    (0x08017A70, 0x0801ADFA): ('ok', "an obstacle's pre-step (BN6_STEP_OBSTACLE_D's routine, BoyBomb's bomb's): BN5's steps its object on "
                                     "lava first (0x08017A9A: 0x08016D78, BN5_LAVA_STEP) where BN6's has no call, src/emu/xpanels.c "
                                     "hooking its next instruction (0x0801AE24) with BN5's rule: the same once lava is in, the rest the "
                                     "same but for the branches past the call (as the Navis' audit of 0x08017A10)", ()),
    (0x0800B144, 0x0800CA8C): ('ok', "a panel broken to a hole (Guard's obstacle's): the same steps on the panel's flags at each game's "
                                     "offset (BN5 +0x18, BN6 +0x14), each clearing its own bits as it sets the hole's type 1 (BN5's "
                                     "0x23F5F, BN6's 0x3F5F: BN5's 0x20000 its sea's), the same sound 0x97", ()),
    (0x0800B050, 0x0800C998): ('ok', "a panel cracked: the same steps on the panel's flags at each game's offset (BN5 +0x18, BN6 +0x14), "
                                     "each clearing its own bits as it sets the cracked type 3 (BN5's 0x23F0F, BN6's 0x3F0F, BN5's "
                                     "0x20000 its sea's); BN6 makes a panel's whole flags word again from its type as it updates it "
                                     "(IWRAM 0x030078DC)", ()),
    # (issue #82's A2b and the chips': the arm's position by an object's NameID, which BN5's temp attack object asks for its
    # owner's; Guard's point 6)
    (0x08015140, 0x08018810): ('ok', "an object's point (r1) by its NameID's arm-point record (the families' ids are past 0x200 and "
                                     "MegaMan's 0x1A0): BN6's records are 0x44 bytes (34 points) where "
                                     "BN5's 0x3C (30), BN6's first 22 points of MegaMan's the same bytes as BN5's (his form 0's record: "
                                     "0x08015C14 in BN5, 0x080191C8 in BN6), and BN6 answers (0, 7) for its filler ids 0xCD-0xFF, no "
                                     "chip's user; Guard asks for point 6", ()),
    # (Slasher's: a hit's reaction while it waits)
    (0x0800E2C2, 0x0801056A): ('ok', "MegaMan's reaction to a hit by its flags (r0): the same steps (AttackVars and the attack's "
                                     "place reset, the side's flash) and the same reactions by each game's own family numbers, "
                                     "BN5's 0x2C and 0x2D (0x080ED96F, 0x080EDBBF) BN6's 0x47 and 0x48 (0x080EFC4D, 0x080EFD7D, "
                                     "the same state tables' shape): 0x200 the first in both, BN5's other the second, BN6's 0x400 "
                                     "its second; Slasher calls it where the flags hold 0x200 or 0x400 (0x080ED420-0x080ED42A)", ()),
    (0x0801742A, 0x0801A66C): ('ok', "an object flashing for 120 frames: 0x78 into its collision data's flash timer (BN5 +0x26, BN6 "
                                     "+0x24, the layout's pair), then object_setFlag(0x200) (BN5 0x08016C42, BN6 0x0801A152): the "
                                     "same steps", ()),
    (0x0800C768, 0x0800E2FC): ('ok', "an object's HP recovered (r0 the amount, r1 whether AntiRecv may answer): the same steps (the "
                                     "recovery, effect 6, sound 0x8A), AntiRecv's trap read by each game's own chip id (BN5 0x93, "
                                     "BN6 0xBD: AntiRecv by the registry, XREG_SAME) and sprung by each game's own (BN5 0x080093A2, "
                                     "BN6 0x0800ABC6)", ()),
    (0x0801A410, 0x0801DACC): ('ok', "the battle's events by bit (r0 a mask of BattleState +0x44's bits): the same dispatch, each "
                                     "game's own sixteen handlers in order; the bombs' (0x40, the sixth: BN5 0x0801A8D6, BN6 "
                                     "0x0801DFEA) the same steps (its picture drawn, 0x0800184C / 0x080018D0 with the same "
                                     "numbers, then the event's bit, 0x080189C6 / 0x0801BEC2), BN5's alone asking the link bit "
                                     "first, clear in every battle of a run", ()),
    (0x080183FC, 0x0801B750): ('ok', "an object's states, struck (the third kind's that run as an obstacle's: the bombs' objects'): "
                                     "the collision data's final damage and what hit it each game's (BN5 +0x78 and +0x68, BN6 +0x80 "
                                     "and +0x70: the collision layout), the object's flags each game's own (BN5 0x10000 at "
                                     "0x08018462, BN6 0x8000 at 0x0801B78A, each by its own getter); BN5's alone a dark hole "
                                     "spawned (0x0801843E-0x0801845C: 0x080CABEA, its panel's side's bit of BattleState +0x5C, as "
                                     "0x08017FCC's), which no battle of a run sets", ()),
    (0x08010D18, 0x08013682): ('ok', "a side's battle NaviStats: each game's own array (BN5 0x60 a side at 0x0203C880, BN6 0x64 at "
                                     "0x0203CE00); RedFrut reads and writes +0xE, MegaMan's mood, the same place in both (BN5 "
                                     "0x080127C2, BN6 0x08015C1A; 0xFF Full Synchro in both's emotions' getters, BN5 0x08012768, "
                                     "BN6 0x08015B9C)", ()),
    (0x08012704, 0x08015B54): ('ok', "MegaMan's emotion (r0 his side): each game's own states by its own rules (BN5: 4 in a Soul, 5 "
                                     "at mood 0, 2 at 0xFF; BN6: 5 while his AI data +0x36 runs, 2 at 0xFF); RedFrut asks it 3 or "
                                     "less before Full Synchro (0x080D8238-0x080D823E): not in the worst, each game's own", ()),
    (0x080127CE, 0x08015BEC): ('ok', "MegaMan's mood set (r1, RedFrut's 0xFF: Full Synchro): each game's own rule for when it may "
                                     "be (BN5: not at mood 0; BN6: not while his AI data +0x32 or +0x36 runs, 0x0801445C, "
                                     "0x08014490), the same place, NaviStats +0xE", ()),
    (0x080E3290, 0x080E42CC): ('ok', "the explosion's burst (the fourth kind's 0x24, BoyBomb's, the Anti chips' counter's): its "
                                     "effect and sound by kind from each game's own table (BN5 0x080E324C, BN6 0x080E4288), the "
                                     "same rows but kind 2's effect, BN5's 0x36 and BN6's 0x45, each table's entry the same "
                                     "numbers (14 14 00 00: list 5's 0x14, animation 0), BN6's drawn from its own redrawn sprite", ()),
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

# Rows of a moved type's own tables that a later track's code spawns it with and its first copy's track does not: fixed in
# the one copy (TYPES: its owner's generator writes the bytes, tools/xnavirecipe.py, tools/xchiprecipe.py) to the numbers the
# later track gives what they name (its generator places it there and checks they agree, tools/xsoul.py): the row's place in
# BN5, what it names ('sprite': its list x 4 and number, two bytes; 'effect': an effect id), BN6's (list, number) or id, the
# track whose numbers they are, and why
SHARED_ROWS = (
    dict(at=0x080C6C52, kind='effect', to=0xC1, track='souls',
         why="the obstacle strike's shot's effect (the table 0x080C6C08's row 12, +2: the parameter the strike spawns the third "
             "kind's 0x12 with, BN5 0x080CAB40; the chips' Vulcan shots their own rows): BN5's 0x20, whose sprite BN6 has none "
             "of by its bytes, copied by the Souls at their 0xC1 (tools/xsoul.py's EFFECT_ORDER)"),
)


def shared_row_bytes(b5, track):
    """the bytes each of SHARED_ROWS the owner `track` of its type's copy writes: [(at, BN5's, BN6's, why)]"""
    out = []
    for r in SHARED_ROWS:
        if r['kind'] == 'sprite':
            l5, i5 = b5.u8(r['at']), b5.u8(r['at'] + 1)
            out += [(r['at'] + n, w, v, r['why']) for n, w, v in ((0, l5, 4 * r['to'][0]), (1, i5, r['to'][1])) if w != v]
        else:
            e5 = b5.u8(r['at'])
            if e5 != r['to']:
                out.append((r['at'], e5, r['to'], r['why']))
    return out


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
# tools/xrecipe.py, tools/xnavirecipe.py and tools/xchiprecipe.py derive each and refuse a recipe whose type is recorded
# otherwise or not at all. A moved type has one copy, the named track's: the one that installs first of those whose code
# spawns it (in install order: the moved objects and the Navis, the chips' code, the families; src/emu/xinstall.c,
# xinstall_core), whose copy fixes what every track's spawns read (its kinds' rows); a track after it spawns that
# copy (XV_TYPE_SHARED) and copies none of it.
# (kind, BN5's type): (decision, BN6's type or None, the track whose copy runs, why)
NO_TWIN = 'no BN6 type runs the same code (tools/xrecipe_family.py, type_twin: each type of the kind\'s table compared deep)'
TYPES = {
    # (the Navis')
    (3, 0x25): ('moved', None, 'families', "CanGard's reticle: BN6's 0x25 another object's code"),
    (1, 0x04): ('moved', None, 'navis', "ShadowMan's: " + NO_TWIN),
    (1, 0x05): ('moved', None, 'navis', "the image object (the Navis' kinds 3 and 0x25, WuNote's 0x30): BN6's type 5 reads "
                                        "a kinds table of its own (MOVE_TYPES), other sprites at other numbers; moved once "
                                        "for every track, its module whole (tools/bn5_objects.py), installed before any "
                                        "family or Navi; the chips' muzzle flash too (MegaMan's arm as a chip's attack "
                                        "takes it: the kinds the units spawn fixed in tools/bn5_objects.py)"),
    (1, 0x54): ('moved', None, 'navis', "NumberMan's companion: BN6's twin of its spawner spawns BN6's 0x55, no deep twin of BN5's 0x080C33AD"),
    (1, 0x56): ('twin', 0x56, 'navis', "the companion overlay: the same code (its spawner and end twins, 0x080C3808/0x080C5D18 and "
                                       "0x080C382C/0x080C5D38); BN5's kinds BN6 lacks take kinds past BN6's 19, their masks and sprites "
                                       "BN5's bytes (src/emu/xnavi_recipe.h, XnOverlay)"),
    (3, 0x02): ('moved', None, 'navis', "Colonel's: " + NO_TWIN),
    (3, 0x05): ('moved', None, 'navis', "NumberMan's: no BN6 type runs the same code (tools/xrecipe_family.py, type_twin: "
                                        "each type of the kind's table compared deep); the Navi chip's too (chips's, "
                                        "tools/xchiprecipe.py): the Navis' copy, installed before the chips', run for it"),
    (3, 0x07): ('moved', None, 'navis', "ShadowMan's: " + NO_TWIN),
    (3, 0x14): ('moved', None, 'navis', "KnightMan's wrecking ball: " + NO_TWIN),
    (3, 0x15): ('moved', None, 'navis', "KnightMan's: " + NO_TWIN),
    (3, 0x17): ('moved', None, 'navis', "KnightMan's: no BN6 type runs the same code (tools/xrecipe_family.py, type_twin: "
                                        "each type of the kind's table compared deep); the Navi chip's too (chips's, "
                                        "tools/xchiprecipe.py): the Navis' copy, installed before the chips', run for it"),
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
    (3, 0x81): ('moved', None, 'navis', "ToadMan's: no BN6 type runs the same code (tools/xrecipe_family.py, type_twin: "
                                        "each type of the kind's table compared deep); the Navi chip's too (chips's, "
                                        "tools/xchiprecipe.py): the Navis' copy, installed before the chips', run for it"),
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
    (3, 0x0B): ('moved', None, 'chips', "SideBub's: no BN6 type runs its code; Marina's water's records (ID_TABLES): BN6's "
                                        "0x0B another object's code (the chips' copy, installed before the families', "
                                        "fixes every kind's row, so the families spawn it)"),
    (3, 0x24): ('moved', None, 'chips', "the crack bombs' bomb (CrakBom's, ParaBom's, ResetBom's): BN6's 0x24 (0x080CA09D) "
                                        "other code, no BN6 type runs BN5's; Bugtank's thrown bomb: BN6's 0x24 another "
                                        "object's code (the chips' copy, installed before the families', fixes every "
                                        "kind's row, so the families spawn it)"),
    (3, 0x3E): ('moved', None, 'chips', "BoyBomb's bomb: no BN6 type runs its code; BomBoy's bomb: BN6's 0x3E another "
                                        "object's code (the chips' copy, installed before the families', fixes every "
                                        "kind's row, so the families spawn it)"),
    (3, 0x56): ('moved', None, 'chips', "Astroid's and DarkMetr's meteor (their controller's, the fourth kind's 0x46): "
                                        "BN6's 0x56 (0x080D0C2F) other code, no BN6 type runs BN5's; the fire column "
                                        "(MetFire's): BN6's 0x56 another sprite (list 3's 0x31, not the same bytes), its "
                                        "kinds 2 and 4 otherwise and only three kinds; moved with its table (ID_TABLES) "
                                        "(the chips' copy, installed before the families', fixes every kind's row, so the "
                                        "families spawn it)"),
    (3, 0x61): ('moved', None, 'families', "BigBrute's: BN6's 0x61 another object's code"),
    (3, 0x6A): ('moved', None, 'chips', "Pulsar's: no BN6 type runs its code; Batty's: BN6's 0x6A another object's code "
                                        "(the chips' copy, installed before the families', fixes every kind's row, so the "
                                        "families spawn it)"),
    (3, 0x6F): ('moved', None, 'families', "Flashy's and Eleogre's flash: BN6's 0x6F another object's code (Flashy's copy runs for both)"),
    (3, 0x84): ('moved', None, 'families', "WuNote's: BN6's 0x84 another object's code"),
    (3, 0x87): ('moved', None, 'families', "Draggin's: BN6's 0x87 another object's code"),
    (3, 0x8F): ('moved', None, 'chips', "SideBub's: no BN6 type runs its code; Marina's water: BN6's 0x8F another object's "
                                        "code (the chips' copy, installed before the families', fixes every kind's row, so "
                                        "the families spawn it)"),
    (3, 0xAC): ('moved', None, 'families', "Appley's: BN6's 0xAC another object's code"),
    (3, 0xC1): ('moved', None, 'families', "Skarab's: BN6's 0xC1 another object's code"),
    (4, 0x78): ('moved', None, 'families', "Drixol's: BN6's 0x78 another object's code"),
    (4, 0x90): ('moved', None, 'families', "Appley's: BN6's 0x90 another object's code"),
    # (twin: spawned by a routine of BN5's engine whose BN6 twin spawns BN6's own of the number, the same code but for each
    # game's layouts and numbers, the same sprites by their bytes)
    (3, 0x03): ('twin', 0x03, 'families', "the collision region (BN5 0x080C4E72's, BN6 0x080C6BDA's): its routines the same but for the "
                                          "collision data's flags at +0x70 where +0x68 and the audited object's end; no sprite"),
    (3, 0x3B): ('moved', None, 'chips', "the wide shot (the spread family's: SuprSpr's, DarkWide's; WideSht's plays BN6's "
                                        "own): no BN6 type runs BN5's code; Lark's (BN5 0x080CCA62's) too, which BN6's "
                                        "0x3B serves for its own kind (its routines the same but for its sprite's number, "
                                        "list 4's 0x1D in BN5, 0x37 in BN6, the same bytes, and the collision data's "
                                        "layout): moved at the merge, Lark's spawner copied to spawn the chips' copy"),
    (3, 0x4B): ('twin', 0x4B, 'families', "Handi's bomb (its spawner BN5 0x080CF37E / BN6 0x080CF4EC): its code BN6's with BN6's state "
                                          "numbering (its own two on, legit.json's t3_types), BN5's clearing of a battle state entry where "
                                          "no collision data is left to make aside; its sprite list 3's 0x23 the same bytes; the "
                                          "differential's handi specs the same frame for frame"),
    (4, 0x00): ('twin', 0x00, 'families', "the effect object (BN5 0x080DFFD2's, BN6 0x080E1932's): the same code but for each game's sprite "
                                          "routines; its sprite by its effect id's row, each game's art for an id (a family's own effects "
                                          "copied under ids of their own where BN5's differ: effect_to)"),
    (4, 0x11): ('twin', 0x11, 'families', "Cactikil's (BN5 0x080E1BFE's): its routines the same bodies; no sprite"),
    (4, 0x14): ('moved', None, 'chips', "the puff of a chip that does nothing (BN5 0x080E1EF4's; spawned by BN5 0x080E1F76, the misc "
                                       "family's subfamily 3 moved for DarkPlus): BN6's 0x14 (0x080E3038) the same instructions (tools/"
                                       "xrecipe_thumb.py compare: SAME) drawing its own art at list 5's 2, BN5's another picture: moved "
                                       "with BN5's art (the chips' rule: the same code drawing another picture is not the same)"),
    (4, 0x35): ('twin', 0x35, 'families', "the fire column's spark of flame (BN5 0x080E450E's, BN6 0x080E5736's): its routines the same "
                                          "bodies (code and type deep the same); no sprite of its own (it draws through the effect object)"),
    # (the chips': tools/xchiprecipe.py's units, their copies installed before the families')
    (1, 0x1C): ('moved', None, 'chips', "HolyDrem's double of MegaMan (BN5 0x080BD811): BN6's 0x1C (0x080BE0ED) other code, no BN6 "
                                        "type runs BN5's"),
    (1, 0x09): ('moved', None, 'chips', "DarkInvs's double of MegaMan (BN5 0x080BA6A9: his own sprite, flickering him 30 frames, then sound 0x143): " + NO_TWIN),
    (4, 0x18): ('moved', None, 'chips', "DarkInvs's object (BN5 0x080E2391: the double spawned, then MegaMan put under BN5's chip AI, src/emu/darkinvs.c): " + NO_TWIN),
    (1, 0x58): ('moved', None, 'chips', "Poltrgst's double of MegaMan (BN5 0x080C39D1): BN6's 0x58 (0x080C5F6D) other code, no BN6 "
                                        "type runs BN5's"),
    (1, 0x06): ('moved', None, 'chips', "the Navi chip KnightMn's KnightMan: BN6's 0x06 (0x080BA711) other code, no BN6 "
                                        "type runs BN5's; its battle sprite the Navis' (list 2's 0x20 on)"),
    (1, 0x07): ('moved', None, 'chips', "the Navi chip ShadoMan's ShadowMan: BN6's 0x07 (0x080BA8E9) other code, no BN6 "
                                        "type runs BN5's; its battle sprite the Navis' (list 2's 0x20 on)"),
    (1, 0x08): ('moved', None, 'chips', "the Navi chip ColonelDS's Colonel: BN6's 0x08 (0x080BAB29) other code, no BN6 "
                                        "type runs BN5's; its battle sprite the Navis' (list 2's 0x20 on)"),
    (1, 0x0A): ('moved', None, 'chips', "BN5's family 0x49's afterimage (MegaMan's own sprite where he stood: Slasher's, "
                                        "the player's 0x80000 reaction's): BN6's 0x0A (0x080BB031) other code, no BN6 type "
                                        "runs BN5's"),
    (1, 0x0F): ('moved', None, 'chips', "the Navi chip NumbrMan's NumberMan: BN6's 0x0F (0x080BBF79) other code, no BN6 "
                                        "type runs BN5's; its battle sprite the Navis' (list 2's 0x20 on)"),
    (1, 0x13): ('moved', None, 'chips', "the Navi chip TmhwkMan's TomahawkMan: BN6's 0x13 (0x080BC7C1) other code, no BN6 "
                                        "type runs BN5's; its battle sprite the Navis' (list 2's 0x20 on)"),
    (1, 0x17): ('moved', None, 'chips', "the Navi chip ToadMan's ToadMan: BN6's 0x17 (0x080BD409) other code, no BN6 type "
                                        "runs BN5's; its battle sprite the Navis' (list 2's 0x20 on)"),
    (1, 0x5C): ('moved', None, 'chips', "NumberMan's object at 0x080C4214 (the Navi chip NumbrMan's): BN6's 0x5C "
                                        "(0x080B9A5D) other code, no BN6 type runs BN5's; its battle sprite the Navis' "
                                        "(list 2's 0x20 on)"),
    (3, 0x00): ('moved', None, 'chips', "Guard's: no BN6 type runs its code"),
    (3, 0x12): ('moved', None, 'chips', "the spread shot (the Vulcan family's shots: InfVulc's): no BN6 type runs BN5's "
                                        "code to its depth (the pairs' search), BN6's own Spreader and Vulcans its own "
                                        "0x12"),
    (3, 0x16): ('moved', None, 'chips', "the shockwave (Guard's reflected wave): BN6's 0x16 the same code, but its kinds "
                                        "read from its own table (BN6 0x080C8370) where BN5's (0x080C7448) holds others: "
                                        "speed and reach by kind"),
    (3, 0x18): ('moved', None, 'chips', "CrakOut's crack: BN6's 0x18 the same code, another sprite at list 5's 0xE"),
    (3, 0x2B): ('moved', None, 'chips', "the shield (Guard's, CrsShld's): BN6's 0x2B the same code, its kinds' sprites "
                                        "another table, another picture at CrsShld3's row"),
    (3, 0x35): ('moved', None, 'chips', "CannBall's ball: BN6's 0x35 (0x080CC1A9) other code, no BN6 type runs BN5's"),
    (3, 0x42): ('moved', None, 'chips', "Geyser's spring: BN6's 0x42 (0x080CDE19) other code, no BN6 type runs BN5's"),
    (3, 0x43): ('moved', None, 'chips', "Geyser's burst: BN6's 0x43 (0x080CDFDD) other code, no BN6 type runs BN5's"),
    (3, 0x45): ('moved', None, 'chips', "MetaGel's gel: BN6's 0x45 draws list 3's 0, its animation 14 frames longer"),
    (3, 0x4A): ('moved', None, 'chips', "the seeds' seed (SeaSeed's, LavaSeed's): BN6's 0x4A (0x080CED5D) other code, no "
                                        "BN6 type runs BN5's"),
    (3, 0x4D): ('moved', None, 'chips', "Anti's: no BN6 type runs its code"),
    (3, 0x51): ('moved', None, 'chips', "Quake's rock: BN6's 0x51 (0x080CFF7D) other code, no BN6 type runs BN5's"),
    (3, 0x58): ('moved', None, 'chips', "ZSaver's: no BN6 type runs its code"),
    (3, 0x31): ('moved', None, 'chips', "the tornado (BN5 0x080CAD49, the Tornado family's: DarkTorn's): BN6's 0x31 (0x080CB7E9) "
                                        "other code, no BN6 type runs BN5's"),
    (3, 0x5A): ('moved', None, 'chips', "NrthWind's wind: no BN6 type runs its code"),
    (3, 0x5B): ('moved', None, 'chips', "Boxer's punch (its controller's, the fourth kind's 0x64, through 0x080D1508): "
                                        "BN6's 0x5B (0x080D1569) other code, no BN6 type runs BN5's"),
    (3, 0x5C): ('moved', None, 'chips', "ShadowMan's attack (the Navi chip ShadoMan's): BN6's 0x5C (0x080D1735) other "
                                        "code, no BN6 type runs BN5's; its battle sprite the Navis' (list 2's 0x20 on)"),
    (3, 0x5D): ('moved', None, 'chips', "AqWhirl's: no BN6 type runs its code"),
    (3, 0x5E): ('moved', None, 'chips', "CactBal's: no BN6 type runs its code"),
    (3, 0x68): ('moved', None, 'chips', "the shake (SpShake's, ShakPar's): no BN6 type runs its code; moved once, by the "
                                        "chips' helper (tools/xchiprecipe.py, HELPERS: Shake)"),
    (3, 0x69): ('moved', None, 'chips', "MoonBld's: no BN6 type runs its code"),
    (3, 0x71): ('moved', None, 'chips', "DrilArm's drill: no BN6 type runs its code"),
    (3, 0x78): ('moved', None, 'chips', "DrkSonic's sonic boom: BN6's 0x78 (0x080D58F9) other code, no BN6 type runs BN5's"),
    (3, 0x7E): ('moved', None, 'chips', "BlakWing's wing (BN5 0x080D6319): BN6's 0x7E (0x080D6899) other code, no BN6 type runs "
                                        "BN5's"),
    (3, 0x7F): ('moved', None, 'chips', "BlakWing's wing's second (BN5 0x080D64D5): BN6's 0x7F (0x080D69A9) other code, no BN6 "
                                        "type runs BN5's"),
    (3, 0x83): ('moved', None, 'chips', "HolyDrem's shot (BN5 0x080D6B49, list 3's 0x3E: SideBub's copy): BN6's 0x83 (0x080D7125) "
                                        "other code, no BN6 type runs BN5's"),
    (3, 0x88): ('moved', None, 'chips', "Skully's skull: no BN6 type runs its code"),
    (3, 0x89): ('moved', None, 'chips', "DarkCirc's circle: BN6's 0x89 (0x080D7DCD) other code, no BN6 type runs BN5's"),
    (3, 0x8A): ('moved', None, 'chips', "DarkCirc's circle's second (its 0x89's): BN6's 0x8A (0x080D805D) other code, no "
                                        "BN6 type runs BN5's"),
    (3, 0x8D): ('moved', None, 'chips', "RedFrut's fruit: BN6's 0x8D (0x080D8445) other code, no BN6 type runs BN5's"),
    (3, 0x90): ('moved', None, 'chips', "Voltz's spark: BN6's 0x90 (0x080D88D9) other code, no BN6 type runs BN5's"),
    (3, 0x92): ('moved', None, 'chips', "Voltz's bolt: BN6's 0x92 (0x080D8C71) other code, no BN6 type runs BN5's"),
    (3, 0x97): ('moved', None, 'chips', "RainyDay's: no BN6 type runs its code"),
    (3, 0x98): ('moved', None, 'chips', "ElemRage's flame: BN6's 0x98 (0x080D9CB5) other code, no BN6 type runs BN5's"),
    (3, 0x9B): ('moved', None, 'chips', "AqWhirl's: no BN6 type runs its code"),
    (3, 0x9D): ('moved', None, 'chips', "HotBody's fire: no BN6 type runs its code"),
    (3, 0xA4): ('moved', None, 'chips', "ElcReel's: no BN6 type runs its code"),
    (3, 0xA5): ('moved', None, 'chips', "the waves' wave (WavePit's, RedWave's, MudWave's): BN6's 0xA5 (0x080DB5BD) other "
                                        "code, no BN6 type runs BN5's"),
    (3, 0xA6): ('moved', None, 'chips', "WoodNos's: no BN6 type runs its code"),
    (3, 0xA7): ('moved', None, 'chips', "WoodNos's: no BN6 type runs its code"),
    (3, 0xA8): ('moved', None, 'chips', "Guard's: no BN6 type runs its code"),
    (3, 0xB1): ('moved', None, 'chips', "the misc family's third kind at 0x080DC6F4: BN6's 0xB1 (0x080DD13D) other code, "
                                        "no BN6 type runs BN5's"),
    (3, 0xB8): ('moved', None, 'chips', "CusVolt's: no BN6 type runs its code"),
    (3, 0xC0): ('moved', None, 'chips', "AqWhirl's: no BN6 type runs its code"),
    (4, 0x0A): ('moved', None, 'chips', "Blinder's flash: BN6's 0x0A not the same code to its depth"),
    (4, 0x1F): ('moved', None, 'chips', "the panel changer (ElemRage's, its kind 0xB: its panel plain again): BN6's 0x1F "
                                        "(0x080E3BE9) its own kinds' table, no BN6 type runs BN5's; the one unit that "
                                        "spawns it (a helper's once another does: MagPanl's, the seeds' rows)"),
    (4, 0x21): ('moved', None, 'chips', "MetaGel's thrower: BN6's twin spawns BN6's own gel"),
    (4, 0x23): ('moved', None, 'chips', "Blinder's light: no BN6 type runs its code"),
    (4, 0x24): ('twin', 0x24, 'chips', "BoyBomb's explosion: BN6's 0x24 the same code to its depth, its sprites the same "
                                       "bytes"),
    (4, 0x28): ('moved', None, 'chips', "the swords' afterimage (Z Saver's, MoonBld's, Katana's): no BN6 type runs its "
                                        "code"),
    (4, 0x2A): ('moved', None, 'chips', "the Anti chips' setter: BN6's 0x2A sets its trap for its kind 0 alone "
                                        "(0x080E48B4: kinds 1-7 nothing, BN6's trap 0x080CF95C), BN5's for its kinds 0-3, "
                                        "Fire, Aqua, Elec and Wood (0x080E3840), the trap BN5's third kind's 0x4D"),
    (4, 0x2B): ('moved', None, 'chips', "the Anti chips' counter (BN5 0x080E3878, BN6's 0x080E48E4): BN6's the same but "
                                        "for the explosion it spawns, BN5's spawner 0x080E32EC (its hit by 0x080F625A) not "
                                        "BN6's 0x080E4328 (its own 0x080E4378) to its depth: moved, its spawner with it "
                                        "(chip_antifire: BN5's explosion's spawn the moved code's, BN6's twin's another)"),
    (4, 0x36): ('twin', 0x36, 'chips', "Jealousy's hit on each foe: BN6's 0x36 the same code to its depth, drawing nothing "
                                       "of its own"),
    (4, 0x39): ('moved', None, 'chips', "NrthWind's time stop: no BN6 type runs its code"),
    (4, 0x3A): ('moved', None, 'chips', "NrthWind's gusts: BN6's 0x3A the same code, another sprite at its number"),
    (4, 0x44): ('moved', None, 'chips', "MrkCan's: no BN6 type runs its code"),
    (4, 0x46): ('moved', None, 'chips', "Astroid's and DarkMetr's meteors' controller: BN6's 0x46 (0x080E6D29) other code, "
                                        "no BN6 type runs BN5's"),
    (4, 0x4A): ('moved', None, 'chips', "DrkSonic's (its support routine's object): BN6's 0x4A would spawn BN6's own "
                                        "sonic, whose code differs"),
    (4, 0x54): ('moved', None, 'chips', "BlakWing's (its support routine's object, BN5 0x080E6A09): BN6's 0x54 (0x080E7BDD) "
                                        "not the same code to its depth"),
    (4, 0x55): ('moved', None, 'chips', "BlakWing's second (BN5 0x080E6AD9): BN6's 0x55 (0x080E7DD1) not the same code to its "
                                        "depth"),
    (4, 0x57): ('moved', None, 'chips', "HotBody's aura: no BN6 type runs its code"),
    (4, 0x59): ('moved', None, 'chips', "HolyDrem's (its support routine's object, BN5 0x080E6EE5): BN6's 0x59 (0x080E836D) other "
                                        "code, no BN6 type runs BN5's"),
    (4, 0x5E): ('moved', None, 'chips', "DarkCirc's (its support routine's object): BN6's 0x5E would spawn BN6's own "
                                        "circle, whose code differs"),
    (4, 0x64): ('moved', None, 'chips', "Boxer's (the Program Advance's): BN6's 0x64 (0x080E8E0D) other code, no BN6 type "
                                        "runs BN5's"),
    (4, 0x65): ('moved', None, 'chips', "CacDanc's (the Program Advance's): BN6's 0x65 (0x080E8F3D) other code, no BN6 "
                                        "type runs BN5's"),
    (4, 0x66): ('moved', None, 'chips', "ShakPar's (the Program Advance's): BN6's 0x66 (0x080E900D) other code, no BN6 "
                                        "type runs BN5's"),
    (4, 0x67): ('moved', None, 'chips', "BoyBomb's thrower: BN6's 0x67 would spawn BN6's own bomb, whose code differs"),
    (4, 0x72): ('moved', None, 'chips', "Poltrgst's (its support routine's object, BN5 0x080E8D05): BN6's 0x72 (0x080E9AD5) other "
                                        "code, no BN6 type runs BN5's"),
    (4, 0x73): ('moved', None, 'chips', "Poltrgst's thrower (BN5 0x080E8DE5: the field's obstacles thrown by 0x0800D5DC's twin, "
                                        "BN6's runners flying them): BN6's 0x73 (0x080E9BE1) other code, no BN6 type runs BN5's"),
    (4, 0x75): ('moved', None, 'chips', "RainyDay's: no BN6 type runs its code"),
    # (the Souls': tools/xsoul.py's, installed after every other track's, src/emu/xinstall.c; the rest of the types their
    # code spawns are other tracks' copies above, run for them: the image object, NumberSoul's companion (1, 0x54) and bomb
    # (3, 0x05), KnightSoul's ball (3, 0x17) and ToadSoul's hit (3, 0x81) the Navis', the obstacle strike's shot (3, 0x12)
    # and ShadowSoul's afterimage (4, 0x28) the chips')
    (3, 0x30): ('moved', None, 'souls', "the obstacle strike (BN5 0x080CA91D, spawned at 0x080CABD0 by 0x080CABCA as an obstacle "
                                        "on the side opposite a Colonel's turns, its states' block 0x08018002-0x0801802E): BN6's "
                                        "0x30 (0x080CB571) another body; a Navi Colonel's side and ColonelSoul's both arm it "
                                        "(BN6_COLONEL_SIDES, src/emu/xstrike.c)"),
    # (unreachable: spawned only behind BattleState+0x32 bit 0x40, which BN5's 0x0802D594 alone sets and BN6's engine never
    # does: issue #82's shared allocations, BN5's panel claims stubbed everywhere)
    (4, 0x83): ('unreachable', None, 'families', "the panel claim the fire column's spark asks for (BN5 0x080EA08C, behind 0x080090E8's "
                                                 "test of BattleState+0x32 bit 0x40): BN6's twin 0x080E5736 left to it"),
}
