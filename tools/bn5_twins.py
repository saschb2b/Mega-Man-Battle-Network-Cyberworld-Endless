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
    # the chips' (tools/xchiprecipe.py): the support objects' generic end substate, where the symbol map placed a routine of
    # another body (BN6's 0x080EA56C); the same body at BN6's 0x0800BC88, the substate BN6's own support objects name
    (0x0800A2F0, 0x0800BC88, 'chips'),
    # an object's element into its collision data (WoodNos's, CactBal's and the misc family's objects'), where the symbol map
    # has none: BN6's of the same steps, a mask besides (its audit below)
    (0x08016C3A, 0x0801A146, 'chips'),
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
)

# The BN5 rows of tools/bn6f_match.py's search that a twin above corrects: its search placed a bn6f function of another
# body there (by one reference each, 'calls'), where the routine is the twin's
WRONG_ROWS = (0x080131A0, 0x080142D0, 0x0801412A, 0x080141EC, 0x080142DC, 0x0800A2F0)

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
    # the chips' code's (tools/xchiprecipe.py; issue #82's C3); the four after the first the Navis' survey's too (their
    # tools/bn5_navi_twins.py), the bodies compared again here
    (0x0800AFC2, 0x0800C90A): ('ok', "object_getPanelDataOffset: BN6 runs the same code from IWRAM (0x03007955: bn6f's "
                                     "_object_getPanelDataOffset, x and y in range, (y x 8 + x) x the panel's size), each game's "
                                     "panel at its own size (BN5 0x24 at 0x0203A100, BN6 0x20); a unit reads its type (+2) and "
                                     "alliance (+3), the same offsets in both, its types BN6's (tools/xchiprecipe.py checks both)", ()),
    (0x08016C3A, 0x0801A146): ('ok', "an object's element (+0x0E) copied into its collision data (+9): BN6 masks it with 0xF, its high "
                                     "nibble BN6's hit flag by a chip's icon element (legit.json's mask), which BN5's objects leave 0: "
                                     "the same element", ()),
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
    (0x08015140, 0x08018810): ('ok', "an object's point (r1) by its NameID's arm-point record: BN6's records are 0x44 bytes (34 points) where "
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

# The second-level dispatch's decision for each of BN5's object types a moved unit's code spawns through it (issue #82's
# shared allocations): MOVED, BN5's own code run for the type in BN6's engine, unless the type is proven the same in BN6,
# its code to its depth and what it draws by its bytes: TWIN, BN6's type of that number run in its place. One decision
# per type, whichever track's code spawns it, written here by the track that met it first (at a merge MOVED wins);
# tools/xchiprecipe.py fails where a unit it writes spawns a type that has none here or another. (kind, BN5's type):
# (decision, track, why)
DISPATCH = {
    (1, 0x05): ('MOVED', 'chips', "the muzzle flash (MegaMan's arm as a chip's attack takes it): BN6's 0x05 not the same code (its "
                                  "forms' and Beast Out's own tests, its own table's numbers); moved once, its spawner a helper of the "
                                  "chips' (tools/xchiprecipe.py, HELPERS)"),
    (1, 0x06): ('MOVED', 'chips', "the Navi chip KnightMn's KnightMan: BN6's 0x06 (0x080BA711) other code, no BN6 type runs BN5's; its sprite the "
                                  "Navi track's (NAVI_SPRITES_WAIT)"),
    (1, 0x07): ('MOVED', 'chips', "the Navi chip ShadoMan's ShadowMan: BN6's 0x07 (0x080BA8E9) other code, no BN6 type runs BN5's; its sprite the "
                                  "Navi track's (NAVI_SPRITES_WAIT)"),
    (1, 0x08): ('MOVED', 'chips', "the Navi chip ColonelDS's Colonel: BN6's 0x08 (0x080BAB29) other code, no BN6 type runs BN5's; its sprite the "
                                  "Navi track's (NAVI_SPRITES_WAIT)"),
    (1, 0x0A): ('MOVED', 'chips', "BN5's family 0x49's afterimage (MegaMan's own sprite where he stood: Slasher's, the player's "
                                  "0x80000 reaction's): BN6's 0x0A (0x080BB031) other code, no BN6 type runs BN5's"),
    (1, 0x0F): ('MOVED', 'chips', "the Navi chip NumbrMan's NumberMan: BN6's 0x0F (0x080BBF79) other code, no BN6 type runs BN5's; its sprite the "
                                  "Navi track's (NAVI_SPRITES_WAIT)"),
    (1, 0x13): ('MOVED', 'chips', "the Navi chip TmhwkMan's TomahawkMan: BN6's 0x13 (0x080BC7C1) other code, no BN6 type runs BN5's; its sprite the "
                                  "Navi track's (NAVI_SPRITES_WAIT)"),
    (1, 0x17): ('MOVED', 'chips', "the Navi chip ToadMan's ToadMan: BN6's 0x17 (0x080BD409) other code, no BN6 type runs BN5's; its sprite the "
                                  "Navi track's (NAVI_SPRITES_WAIT)"),
    (1, 0x5C): ('MOVED', 'chips', "NumberMan's object at 0x080C4214 (the Navi chip NumbrMan's): BN6's 0x5C (0x080B9A5D) other code, no BN6 type runs BN5's; its sprite the "
                                  "Navi track's (NAVI_SPRITES_WAIT)"),
    (3, 0x00): ('MOVED', 'chips', "Guard's: no BN6 type runs its code"),
    (3, 0x05): ('MOVED', 'chips', "NumberMan's attack (the Navi chip NumbrMan's): BN6's 0x05 (0x080C6E21) other code, no BN6 type runs BN5's; its sprite the "
                                  "Navi track's (NAVI_SPRITES_WAIT)"),
    (3, 0x0B): ('MOVED', 'chips', "SideBub's: no BN6 type runs its code"),
    (3, 0x12): ('MOVED', 'chips', "the spread shot (the Vulcan family's shots: InfVulc's): no BN6 type runs BN5's code to its "
                                  "depth (the pairs' search), BN6's own Spreader and Vulcans its own 0x12"),
    (3, 0x16): ('MOVED', 'chips', "the shockwave (Guard's reflected wave): BN6's 0x16 the same code, but its kinds read from its own "
                                  "table (BN6 0x080C8370) where BN5's (0x080C7448) holds others: speed and reach by kind"),
    (3, 0x17): ('MOVED', 'chips', "KnightMan's attack (the Navi chip KnightMn's): BN6's 0x17 (0x080C863D) other code, no BN6 type runs BN5's; its sprite the "
                                  "Navi track's (NAVI_SPRITES_WAIT)"),
    (3, 0x18): ('MOVED', 'chips', "CrakOut's crack: BN6's 0x18 the same code, another sprite at list 5's 0xE"),
    (3, 0x24): ('MOVED', 'chips', "the crack bombs' bomb (CrakBom's, ParaBom's, ResetBom's): BN6's 0x24 (0x080CA09D) other code, no BN6 type runs BN5's"),
    (3, 0x2B): ('MOVED', 'chips', "the shield (Guard's, CrsShld's): BN6's 0x2B the same code, its kinds' sprites another table, "
                                  "another picture at CrsShld3's row"),
    (3, 0x35): ('MOVED', 'chips', "CannBall's ball: BN6's 0x35 (0x080CC1A9) other code, no BN6 type runs BN5's"),
    (3, 0x3B): ('MOVED', 'chips', "the wide shot (the spread family's: SuprSpr's, DarkWide's; WideSht's plays BN6's own): no "
                                  "BN6 type runs BN5's code"),
    (3, 0x3E): ('MOVED', 'chips', "BoyBomb's bomb: no BN6 type runs its code"),
    (3, 0x42): ('MOVED', 'chips', "Geyser's spring: BN6's 0x42 (0x080CDE19) other code, no BN6 type runs BN5's"),
    (3, 0x43): ('MOVED', 'chips', "Geyser's burst: BN6's 0x43 (0x080CDFDD) other code, no BN6 type runs BN5's"),
    (3, 0x45): ('MOVED', 'chips', "MetaGel's gel: BN6's 0x45 draws list 3's 0, its animation 14 frames longer"),
    (3, 0x4A): ('MOVED', 'chips', "the seeds' seed (SeaSeed's, LavaSeed's): BN6's 0x4A (0x080CED5D) other code, no BN6 type runs BN5's"),
    (3, 0x4D): ('MOVED', 'chips', "Anti's: no BN6 type runs its code"),
    (3, 0x51): ('MOVED', 'chips', "Quake's rock: BN6's 0x51 (0x080CFF7D) other code, no BN6 type runs BN5's"),
    (3, 0x56): ('MOVED', 'chips', "Astroid's and DarkMetr's meteor (their controller's, the fourth kind's 0x46): BN6's 0x56 (0x080D0C2F) other code, no BN6 type runs BN5's"),
    (3, 0x58): ('MOVED', 'chips', "ZSaver's: no BN6 type runs its code"),
    (3, 0x5A): ('MOVED', 'chips', "NrthWind's wind: no BN6 type runs its code"),
    (3, 0x5B): ('MOVED', 'chips', "Boxer's punch (its controller's, the fourth kind's 0x64, through 0x080D1508): BN6's 0x5B (0x080D1569) other code, no BN6 type runs BN5's"),
    (3, 0x5C): ('MOVED', 'chips', "ShadowMan's attack (the Navi chip ShadoMan's): BN6's 0x5C (0x080D1735) other code, no BN6 type runs BN5's; its sprite the "
                                  "Navi track's (NAVI_SPRITES_WAIT)"),
    (3, 0x5D): ('MOVED', 'chips', "AqWhirl's: no BN6 type runs its code"),
    (3, 0x5E): ('MOVED', 'chips', "CactBal's: no BN6 type runs its code"),
    (3, 0x68): ('MOVED', 'chips', "the shake (SpShake's, ShakPar's): no BN6 type runs its code; moved once, by the chips' helper "
                                  "(tools/xchiprecipe.py, HELPERS: Shake)"),
    (3, 0x69): ('MOVED', 'chips', "MoonBld's: no BN6 type runs its code"),
    (3, 0x6A): ('MOVED', 'chips', "Pulsar's: no BN6 type runs its code"),
    (3, 0x71): ('MOVED', 'chips', "DrilArm's drill: no BN6 type runs its code"),
    (3, 0x78): ('MOVED', 'chips', "DrkSonic's sonic boom: BN6's 0x78 (0x080D58F9) other code, no BN6 type runs BN5's"),
    (3, 0x81): ('MOVED', 'chips', "ToadMan's attack (the Navi chip ToadMan's): BN6's 0x81 (0x080D6DF1) other code, no BN6 type runs BN5's; its sprite the "
                                  "Navi track's (NAVI_SPRITES_WAIT)"),
    (3, 0x88): ('MOVED', 'chips', "Skully's skull: no BN6 type runs its code"),
    (3, 0x89): ('MOVED', 'chips', "DarkCirc's circle: BN6's 0x89 (0x080D7DCD) other code, no BN6 type runs BN5's"),
    (3, 0x8A): ('MOVED', 'chips', "DarkCirc's circle's second (its 0x89's): BN6's 0x8A (0x080D805D) other code, no BN6 type "
                                  "runs BN5's"),
    (3, 0x8D): ('MOVED', 'chips', "RedFrut's fruit: BN6's 0x8D (0x080D8445) other code, no BN6 type runs BN5's"),
    (3, 0x8F): ('MOVED', 'chips', "SideBub's: no BN6 type runs its code"),
    (3, 0x90): ('MOVED', 'chips', "Voltz's spark: BN6's 0x90 (0x080D88D9) other code, no BN6 type runs BN5's"),
    (3, 0x92): ('MOVED', 'chips', "Voltz's bolt: BN6's 0x92 (0x080D8C71) other code, no BN6 type runs BN5's"),
    (3, 0x97): ('MOVED', 'chips', "RainyDay's: no BN6 type runs its code"),
    (3, 0x98): ('MOVED', 'chips', "ElemRage's flame: BN6's 0x98 (0x080D9CB5) other code, no BN6 type runs BN5's"),
    (3, 0x9B): ('MOVED', 'chips', "AqWhirl's: no BN6 type runs its code"),
    (3, 0x9D): ('MOVED', 'chips', "HotBody's fire: no BN6 type runs its code"),
    (3, 0xA4): ('MOVED', 'chips', "ElcReel's: no BN6 type runs its code"),
    (3, 0xA5): ('MOVED', 'chips', "the waves' wave (WavePit's, RedWave's, MudWave's): BN6's 0xA5 (0x080DB5BD) other code, no BN6 type runs BN5's"),
    (3, 0xA6): ('MOVED', 'chips', "WoodNos's: no BN6 type runs its code"),
    (3, 0xA7): ('MOVED', 'chips', "WoodNos's: no BN6 type runs its code"),
    (3, 0xA8): ('MOVED', 'chips', "Guard's: no BN6 type runs its code"),
    (3, 0xB1): ('MOVED', 'chips', "the misc family's third kind at 0x080DC6F4: BN6's 0xB1 (0x080DD13D) other code, no BN6 type runs BN5's"),
    (3, 0xB8): ('MOVED', 'chips', "CusVolt's: no BN6 type runs its code"),
    (3, 0xC0): ('MOVED', 'chips', "AqWhirl's: no BN6 type runs its code"),
    (4, 0x0A): ('MOVED', 'chips', "Blinder's flash: BN6's 0x0A not the same code to its depth"),
    (4, 0x1F): ('MOVED', 'chips', "the panel changer (ElemRage's, its kind 0xB: its panel plain again): BN6's 0x1F (0x080E3BE9) "
                                  "its own kinds' table, no BN6 type runs BN5's; the one unit that spawns it (a helper's once "
                                  "another does: MagPanl's, the seeds' rows)"),
    (4, 0x21): ('MOVED', 'chips', "MetaGel's thrower: BN6's twin spawns BN6's own gel"),
    (4, 0x23): ('MOVED', 'chips', "Blinder's light: no BN6 type runs its code"),
    (4, 0x24): ('TWIN', 'chips', "BoyBomb's explosion: BN6's 0x24 the same code to its depth, its sprites the same bytes"),
    (4, 0x28): ('MOVED', 'chips', "the swords' afterimage (Z Saver's, MoonBld's, Katana's): no BN6 type runs its code"),
    (4, 0x2A): ('MOVED', 'chips', "the Anti chips' setter: BN6's 0x2A sets its trap for its kind 0 alone (0x080E48B4: kinds 1-7 "
                                  "nothing, BN6's trap 0x080CF95C), BN5's for its kinds 0-3, Fire, Aqua, Elec and Wood (0x080E3840), "
                                  "the trap BN5's third kind's 0x4D"),
    (4, 0x2B): ('MOVED', 'chips', "the Anti chips' counter (BN5 0x080E3878, BN6's 0x080E48E4): BN6's the same but for the "
                                  "explosion it spawns, BN5's spawner 0x080E32EC (its hit by 0x080F625A) not BN6's 0x080E4328 (its "
                                  "own 0x080E4378) to its depth: moved, its spawner with it (chip_antifire: BN5's explosion's spawn "
                                  "the moved code's, BN6's twin's another)"),
    (4, 0x36): ('TWIN', 'chips', "Jealousy's hit on each foe: BN6's 0x36 the same code to its depth, drawing nothing of its own"),
    (4, 0x39): ('MOVED', 'chips', "NrthWind's time stop: no BN6 type runs its code"),
    (4, 0x3A): ('MOVED', 'chips', "NrthWind's gusts: BN6's 0x3A the same code, another sprite at its number"),
    (4, 0x44): ('MOVED', 'chips', "MrkCan's: no BN6 type runs its code"),
    (4, 0x46): ('MOVED', 'chips', "Astroid's and DarkMetr's meteors' controller: BN6's 0x46 (0x080E6D29) other code, no BN6 type runs BN5's"),
    (4, 0x4A): ('MOVED', 'chips', "DrkSonic's (its support routine's object): BN6's 0x4A would spawn BN6's own sonic, whose "
                                  "code differs"),
    (4, 0x57): ('MOVED', 'chips', "HotBody's aura: no BN6 type runs its code"),
    (4, 0x5E): ('MOVED', 'chips', "DarkCirc's (its support routine's object): BN6's 0x5E would spawn BN6's own circle, whose "
                                  "code differs"),
    (4, 0x64): ('MOVED', 'chips', "Boxer's (the Program Advance's): BN6's 0x64 (0x080E8E0D) other code, no BN6 type runs BN5's"),
    (4, 0x65): ('MOVED', 'chips', "CacDanc's (the Program Advance's): BN6's 0x65 (0x080E8F3D) other code, no BN6 type runs BN5's"),
    (4, 0x66): ('MOVED', 'chips', "ShakPar's (the Program Advance's): BN6's 0x66 (0x080E900D) other code, no BN6 type runs BN5's"),
    (4, 0x67): ('MOVED', 'chips', "BoyBomb's thrower: BN6's 0x67 would spawn BN6's own bomb, whose code differs"),
    (4, 0x75): ('MOVED', 'chips', "RainyDay's: no BN6 type runs its code"),
}
