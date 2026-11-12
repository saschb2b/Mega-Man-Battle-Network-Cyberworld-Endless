"""bn4_twins.py: BN4 Red Sun's routines and their twins in BN6 Cybeast Gregar (USA), as this project paired them for
moving BN4's virus families (docs/multirom/families.md; docs/rom/bn4.md, Virus families): numbers only, each pair's two
bodies compared by tools/xrecipe_thumb.py when tools/xrecipe_bn4.py derives a family (SAME, LAYOUT: but for the offsets
of loads and stores, or DIFF), a DIFF pair taken only where its audit (AUDITS) says it serves what BN4's code asks of it,
as tools/bn5_twins.py's are for BN5."""

# (BN4, BN6, where the pair was found): a routine's entry in each game. 'research' the engine survey's (issue #82's BN4
# phase: the 21 families' calls out, each located by bn6f's symbol map in BN4 and in Gregar and compared by its body;
# 40 SAME, 5 LAYOUT, 13 DIFF there); the generator compares each again before it takes one
TWINS = (
    (0x08000560, 0x080005CC, 'research'), (0x08000AAA, 0x08000C72, 'research'), (0x08001B16, 0x08001532, 'research'),
    (0x0800283C, 0x080026A4, 'research'), (0x0800287C, 0x080026E4, 'research'), (0x08002DEC, 0x08002D80, 'research'),
    (0x08002E10, 0x08002DA4, 'research'), (0x08002E56, 0x08002DEA, 'research'), (0x08002E96, 0x08002E3C, 'research'),
    (0x08002FAE, 0x08002F5C, 'research'), (0x08002FE2, 0x08002F90, 'research'), (0x080033D4, 0x08003458, 'research'),
    (0x08007948, 0x0800A098, 'research'), (0x08007A12, 0x0800A18E, 'research'), (0x08007BE4, 0x0800A704, 'research'),
    (0x08007C20, 0x0800A754, 'research'), (0x080090B8, 0x0800BF16, 'research'), (0x08009A86, 0x0800C8F8, 'research'),
    (0x08009E8C, 0x0800CC66, 'research'), (0x08009EAC, 0x0800CC86, 'research'), (0x08009FC4, 0x0800CE64, 'research'),
    (0x0800A0D8, 0x0800CFDC, 'research'), (0x0800A10E, 0x0800D012, 'research'), (0x0800A192, 0x0800D06A, 'research'),
    (0x0800A1AE, 0x0800D086, 'research'), (0x0800AB20, 0x0800E258, 'research'), (0x0800AB3E, 0x0800E276, 'research'),
    (0x0800AB64, 0x0800E29C, 'research'), (0x0800AB74, 0x0800E2AC, 'research'), (0x0800AB88, 0x0800E2C0, 'research'),
    (0x0800AB8A, 0x0800E2C2, 'research'), (0x0800ABBA, 0x0800E2EC, 'research'), (0x0800ACA6, 0x0800E456, 'research'),
    (0x0800AE30, 0x0800E708, 'research'), (0x0800F83C, 0x08015D80, 'research'), (0x0800F902, 0x08015E46, 'research'),
    (0x0801013A, 0x080165B8, 'research'), (0x08010276, 0x080166AE, 'research'), (0x080104AC, 0x08016B02, 'research'),
    (0x080105BC, 0x08016CE8, 'research'), (0x08012496, 0x08019892, 'research'), (0x080129AC, 0x08019FB4, 'research'),
    (0x08012D62, 0x0801A018, 'research'), (0x08012D9A, 0x0801A04C, 'research'), (0x08012DC0, 0x0801A074, 'research'),
    (0x08012DC8, 0x0801A07C, 'research'), (0x08012F44, 0x0801A0D4, 'research'), (0x08012F82, 0x0801A152, 'research'),
    (0x08012F8C, 0x0801A15C, 'research'), (0x08012F96, 0x0801A166, 'research'), (0x080143CA, 0x0801BB46, 'research'),
    (0x08014446, 0x0801BBF4, 'research'), (0x0802608C, 0x080302A8, 'research'), (0x080B3878, 0x0800EC48, 'research'),
    (0x080CD7E2, 0x080C6BDA, 'research'), (0x080D520C, 0x080CD6F2, 'research'), (0x080D528E, 0x080CD774, 'research'),
    (0x080E18EE, 0x080E1932, 'research'),
    # Billy's code audit's (the field-by-field reading of its states, .build/bn4-billy): its aims, each compared again
    (0x080D2156, 0x080CADBC, 'billy'), (0x0800AFBC, 0x0800E9DC, 'billy'), (0x080143A8, 0x0801BB1C, 'billy'),
    (0x08044AEC, 0x080011EC, 'billy'), (0x080E514E, 0x0810A952, 'billy'), (0x080BE634, 0x0810A952, 'billy'),
    (0x080190D0, 0x08021AA4, 'billy'), (0x08009ABC, 0x0800C90A, 'billy'), (0x080B6568, 0x0800EBD4, 'billy'),
    # Spikey's and Shrimpy's code audit's (their states and their attacks', .build/bn4-spikey): each compared again
    (0x0800A17A, 0x0800D048, 'spikey'), (0x0800A0A2, 0x0800CFA6, 'spikey'), (0x08010560, 0x080D0B94, 'spikey'),
    # the thunder's (the third kind's 0x2A, BN4's and BN6's of the same code but for an offset or two): the routines their
    # init, update and type routine call in the same places, which Spikey's and Shrimpy's attacks call too
    (0x080143FC, 0x0801BBAC, 'thunder'), (0x08012F74, 0x0801A140, 'thunder'), (0x080129FC, 0x0801A00E, 'thunder'),
    (0x08009EE4, 0x0800CCBE, 'thunder'),
    # Walla's, Elmperor's and CirKill's (bn6f's names located in BN4 and in Gregar, .build/bn4-fam2; each body read), and their
    # attacks'
    (0x08012DB2, 0x0801A066, 'families-2'), (0x0800AE10, 0x0800E6E8, 'families-2'), (0x0801014C, 0x080165C2, 'families-2'),
    (0x08009DA0, 0x0800CBD8, 'families-2'), (0x08009DC4, 0x0800CC0A, 'families-2'), (0x0800ABCA, 0x0810A952, 'families-2'),
    (0x080164B4, 0x0810A952, 'families-2'), (0x080E407A, 0x0810A952, 'families-2'), (0x080E76EA, 0x080011EC, 'families-2'),
    (0x080E770C, 0x0810A952, 'families-2'), (0x080CDCE4, 0x0801156A, 'families-2'),
    # the after-state's (BN4_VIRUS_PRESENT, copied with each family: COPIED), its calls BN6's twin makes in the same places
    # (0x08016E64): the sprite update (BN4_SPRITE_UPDATE, BN6's 0x080026C4) and the struck word read (BN4's +0x68, BN6's +0x40)
    (0x0800285C, 0x080026C4, 'after-state'), (0x08012FB0, 0x0801A180, 'after-state'),
    # GutsMan's (tools/xnavirecipe_bn4.py, each body read): GetRNG (BN4_GET_RNG, the same LCG step on BN6's word, one of three
    # BN6 routines of its body: bn4.h's), the status a hit gives (BN4_COLL_STATUS_GIVEN +0xE, BN6's +0x10: the one BN6 routine
    # of its three instructions with BN6's offset, found by its bytes), a side's player (BN4_FIND_PLAYER, BN6's own search)
    (0x08001B00, 0x0800151C, 'gutsman'), (0x08013212, 0x0801A258, 'gutsman'), (0x0800C456, 0x080103BC, 'gutsman'),
    # ... its own panel a floor (flag 0x10 of its flags, BN4's +4, BN6's +0x14), one of two BN6 routines of its outer body
    # (0x0800CC66's inner test is the panel's place); a panel cracked (BN4_PANEL_CRACK)
    (0x08009ECC, 0x0800CCA6, 'gutsman'), (0x08009AEC, 0x0800C938, 'gutsman'),
    # FireMan's (docs/multirom/bn4-navis.md, FireMan built; each body read): his companion's sprite setters (the same bodies at
    # BN6's places), his objects' collision setters, the pause test his copied sprite step calls
    (0x08002E20, 0x08002DB4, 'fireman'), (0x08002F54, 0x08002F02, 'fireman'), (0x08012886, 0x08019F86, 'fireman'),
    (0x08012F7A, 0x0801A146, 'fireman'), (0x080078EC, 0x0800A03C, 'fireman'),
    # WindMan's (tools/xnavirecipe_bn4.py, each body read): his flinch's and paralysis' tails (BN4's generic Navi states 3 and
    # 4, BN6's Navi states of those places) and his wind's spawn (the fourth kind's 0x41 of his side, BN6's routine of the
    # same steps); his companions' sprite steps are FireMan's pairs above
    (0x08010E88, 0x080174FE, 'windman'), (0x08010F08, 0x080175B8, 'windman'), (0x080E68F8, 0x080E666C, 'windman'),
    # Mettaur's (docs/multirom/bn4-navis.md, Roll built: her summon's; each body read): the appear queue its turn waits in
    # (BN4's one queue of +0xE bytes, BN6's queues of pointers by r0: each call made through a shim that gives r0 the appear's
    # queue, 0, QUEUED), and its held chips' attacks, dead while it holds none
    (0x08007F04, 0x0800AA06, 'mettaur'), (0x08007F1C, 0x0800AA1A, 'mettaur'), (0x08007F46, 0x0800AA40, 'mettaur'),
    (0x080E45F6, 0x0810A952, 'mettaur'), (0x080E8ADA, 0x0810A952, 'mettaur'),
    # ... its generic state 0 and BN4's summoned appear copied with it (SUMMON_APPEAR): their calls out
    (0x0800FE00, 0x08016396, 'mettaur'), (0x0800FCDE, 0x08016396, 'mettaur'), (0x0800FF26, 0x080164A0, 'mettaur'),
    (0x0800B018, 0x0800EAFA, 'mettaur'), (0x0800B026, 0x0800EB08, 'mettaur'), (0x08007E46, 0x080011EC, 'mettaur'),
    (0x08015ABC, 0x0801DC7C, 'mettaur'),
    # Melody's (docs/multirom/bn4-navis.md, Roll's summons; each body read): its collision's region changed (its states'), BN6's
    # routine of the same steps at BN6's offsets
    (0x08012ED0, 0x0801A082, 'melody'),
    # VolGear's (each body read): its held chips' attacks, each behind its held chip's test (0x080B52E4 at 0x080B6362 and
    # 0x080B6384; its +0x2A against 0xFFFF at 0x080B63BE), dead while it holds none
    (0x080E50AE, 0x0810A952, 'volgear'), (0x080B9704, 0x0810A952, 'volgear'), (0x080B3C1A, 0x0810A952, 'volgear'),
    # MetalMan's (docs/multirom/bn4-navis.md, MetalMan built; each body read): a panel broken where it stands free (the chips'
    # generator's pair, tools/xchiprecipe_bn4.py TWINS), his MetalGears' obstacle bookkeeping (added, removed, their lifetime)
    (0x08009BAC, 0x0800C9F8, 'metalman'), (0x0800B230, 0x0800F614, 'metalman'), (0x0800B272, 0x0800F656, 'metalman'),
    (0x0800B28E, 0x0800F672, 'metalman'),
    # BurnerMan's (each body read): a hit's push word set
    (0x08013444, 0x0801A4D0, 'burnerman'),
)

# The generic states BN4's families' state tables open with (0-5, as BN5's: tools/xrecipe_family.py GENERIC5), each by
# its place there; BN6's of the same place stand in for them (its 6 and 7, frozen and bubbled, BN4 has none of)
GENERIC4 = {0: 0x0800FCAC, 1: 0x0801013A, 2: 0x0801014C, 3: 0x08010276, 4: 0x080104AC, 5: 0x080105BC}

# (BN4, BN6) -> (verdict, why): a pair whose bodies differ judged against what BN4's code asks of it (Billy's code audit,
# read from both games' code); any other DIFF pair a family reaches stays an open decision in the generator's report
AUDITS = {
    (0x08009AEC, 0x0800C938): ('ok', "a panel cracked (r0, r1): a floor (0x10) not yet cracked (0x40) made cracked (type 3, flags + 0x40), "
                                     "a cracked one broken (type 1), the crack's sound, 1 back; BN6's the same at its offsets (flags "
                                     "+0x14, type +2 and its drawn type +6) and its own sound, but BN4's test of flag 0x20000 first "
                                     "(no crack), which only BN4's type 5 gives (BN4_PANEL_FLAGS_OF 0x30010): no BN6 panel type's word "
                                     "has it (BN6's table at 0x081D7E1C), so the test never holds in BN6. Flags 0x10 and 0x40 are "
                                     "the same in both tables (types 0-4 the same words)"),
    (0x0800C456, 0x080103BC): ('ok', "a side's player object (r0 the side): BN4's the battle state's player where its side is r0's, else "
                                     "the first of the other side's three; BN6's the first of the side's four whose name is a "
                                     "MegaMan's (0x0800F29C): MegaMan for his side in a battle of one player a side, GutsMan's aim"),
    (0x080D2156, 0x080CADBC): ('ok', "Billy's attack spawned: BN6's spawner in its own third-kind 0x2A block, the same registers in; "
                                     "BN6's 0x2A runs Billy's thunder state by state for Billy's parameters (BN4's would write its "
                                     "element into BN6's flip at +0x17): BN6's object, not BN4's moved"),
    (0x0800AFBC, 0x0800E9DC): ('ok', "an object's counter time (its window to be countered): BN6's sets the same timer, its own fields"),
    (0x080143A8, 0x0801BB1C): ('ok', "the panel it moves to reserved: BN6's own reserve of the future panel (+0x14, +0x15)"),
    (0x08012D9A, 0x0801A04C): ('ok', "its collision data's panels set from its place: BN6's keeps r6 too and writes BN6's offsets"),
    (0x08044AEC, 0x080011EC): ('ok', "BN4's test of a subsystem's byte (0x020015E1) no BN6 battle has: BN6's routine that returns 0, "
                                     "BN4's path in a plain battle (to be checked on the guest core)"),
    (0x080E514E, 0x0810A952): ('ok', "a held chip's attack spawned: dead in BN6 while its +0x2A stays 0xFFFF (it holds none): BN6's "
                                     "family nullsub"),
    (0x080BE634, 0x0810A952): ('ok', "a held chip used: dead in BN6 while its +0x2A stays 0xFFFF (it holds none): BN6's family nullsub"),
    (0x08009ABC, 0x0800C90A): ('ok', "getPanel: each game's veneer (ldr r2,=; mov lr,pc; bx r2, its pool a word further in BN4's) to "
                                     "its routine in IWRAM, back r0 the panel at column r0, row r1 (its type BN4's +0, BN6's +2)"),
    (0x080B6568, 0x0800EBD4): ('ok', "enemies looked up by a NameID range: BN6's object_getEnemyByNameRange, through a shim that gives "
                                     "BN6's ids as its bounds and r3 its side (1, a virus's), which BN4's takes for itself"),
    (0x0800A17A, 0x0800D048): ('ok', "the row of the enemies' player (MegaMan; its own where there is none): BN6's, which takes the row "
                                     "he is headed for where his own is off rows 1-3 (mid-move), where BN4's gives that row as it is"),
    (0x080143FC, 0x0801BBAC): ('ok', "an object's sprite each frame (none while the battle is paused, the object hidden or held, or time "
                                     "stopped; its animation changed where it was asked): BN6's of the same steps, its pause's test a "
                                     "call that sets the flags where BN4's returns r0"),
    (0x080129FC, 0x0801A00E): ('ok', "its collision's hits resolved each frame: BN6's runs that work from IWRAM through this thunk "
                                     "(0x0300754D), BN4's in ROM; each thunder's update, BN6's but for an offset, calls it first"),
    (0x08009EE4, 0x0800CCBE): ('ok', "the panels its collision region covers lit (its panel, its region's kind, its side): BN6's reads "
                                     "its own offsets and turns the side by the object's flip (+0x17), 0 on BN4's objects moved (their "
                                     "element BN6's +0xE)"),
    # (the engine's routines Spikey's and Shrimpy's attacks, moved, call that BN4's other code reached none of: each the same
    # steps as BN6's in its own layout, read side by side)
    (0x0800ACA6, 0x0800E456): ('ok', "an object's facing for its sprite (its side): BN6's turns it by the flip (+0x17), 0 on BN4's "
                                     "objects moved (their element BN6's +0xE)"),
    (0x08012496, 0x08019892): ('ok', "its collision data taken from the pool (eToolkit +0x30): BN6's, 0xA8 bytes each where BN4's "
                                     "are 0x90, zeroed and marked in BN6's layout"),
    (0x080129AC, 0x08019FB4): ('ok', "its collision data set up (its region, its kinds, its owner): BN6's, its fields at BN6's "
                                     "offsets (the owner +0x38 where BN4's +0x60)"),
    (0x08012D62, 0x0801A018): ('ok', "its collision data presented each frame (the flags its hits left it, BN4's +0x54, BN6's "
                                     "+0x70; the frame's fields cleared): BN6's, in BN6's layout, its last step through IWRAM"),
    (0x08012F44, 0x0801A0D4): ('ok', "the effect its hits make (the flags its hits left it tested, its hit effect: BN4's +7, "
                                     "BN6's +9): BN6's, the same steps in BN6's layout"),
    # (Walla's, Elmperor's and CirKill's code and their attacks', read from both games' code, .build/bn4-fam2)
    (0x0801014C, 0x080165C2): ('ok', "the generic state 2 (deletion's) Elmperor's own state 2 wraps: BN6's in its place, as every "
                                     "family's state 2 is BN6's; BN6's first runs its own tests (+0x40000000, 0x0800A03C)"),
    (0x08009DA0, 0x0800CBD8): ('ok', "a panel highlighted where the attack will strike (its column r0, row r1, a valid panel's): "
                                     "BN6's object_highlightPanel sets the panel's +1, where BN4 keeps a byte a panel at 0x02037ABE"),
    (0x08009DC4, 0x0800CC0A): ('ok', "a panel's type set (r0 column, r1 row, r2 type) where the panel is not a hole: BN6's "
                                     "object_setPanelType; BN4's also gives it 0xBE frames (+0x12) to change back, which BN6's panels "
                                     "do not count but for its conveyors (an interim: the panel keeps its type)"),
    (0x0800ABCA, 0x0810A952): ('ok', "Walla's Recov80 or Recov150 used (a held chip, its +0x2A 0x6E or 0x70): dead in BN6 while its "
                                     "+0x2A stays 0xFFFF (it holds none): BN6's family nullsub"),
    (0x080164B4, 0x0810A952): ('ok', "Walla's held chip spent after its recovery: dead while its +0x2A stays 0xFFFF: BN6's family "
                                     "nullsub"),
    (0x080E407A, 0x0810A952): ('ok', "Walla's held chip's attack spawned (behind 0x080B52E4's test of +0x2A): dead while it holds "
                                     "none: BN6's family nullsub"),
    (0x080E76EA, 0x080011EC): ('ok', "Elmperor's aura spawned (the fourth kind's 0x4C: its sprite copied in its panel's colours "
                                     "through BN4's tile buffer 0x02037810, which has no twin mapped): answered 0, BN6's routine that "
                                     "returns 0, so its +0x6C stays 0 (an interim: the aura not drawn; its HP's return goes on)"),
    (0x080E770C, 0x0810A952): ('ok', "Elmperor's aura told to end (its +8 set 8): reached only where +0x6C holds one, never while "
                                     "the aura is answered 0: BN6's family nullsub"),
    (0x080CDCE4, 0x0801156A): ('ok', "an object's hit taken from its HP (r0 0: off its +0x24, -1 back where none is left, else a white "
                                     "flash): BN6's basicHandleBattleObjectDamage, its r0 0 path the same, its sound BN6's (0x85, BN4's "
                                     "0xC8)"),
    (0x08012F7A, 0x0801A146): ('ok', "its collision data's hit kind (BN4's +7, BN6's +9) given its object's element (BN4's +0x17, "
                                     "BN6's +0xE): BN6's keeps the element's low four bits, where BN6's actor init ORs its "
                                     "struct1's flags into +0xE's high ones (0x08016FA0); FireMan's moved objects' +0xE is the "
                                     "element alone (their spawns' element store, rewritten to +0xE), so both store the same"),
    (0x080078EC, 0x0800A03C): ('ok', "the battle's pause byte (BN4's [eToolkit +0x40] +9, BN6's [eToolkit +0x3C] +0xA) back in r0, "
                                     "BN6's with its flags set by a tst its callers branch on (BN4's callers test r0 themselves): "
                                     "the after-steps' pause tests, the pair 0x080143FC / 0x0801BBAC's"),
    # (WindMan's, read from both games' code)
    (0x08010E88, 0x080174FE): ('ok', "the generic Navi flinch WindMan's own state 3 wraps (his wind, +0x60, told to end first, "
                                     "0x080E69BC): BN6's Navi flinch in its place, as every Navi's state 3 is BN6's"),
    (0x08010F08, 0x080175B8): ('ok', "the generic Navi paralysis WindMan's own state 4 wraps (his wind told to end first): BN6's "
                                     "Navi paralysis in its place, as every Navi's state 4 is BN6's"),
    (0x080E68F8, 0x080E666C): ('ok', "a side's wind spawned (the fourth kind's 0x41, r0 back; none where the side's slot holds one "
                                     "of level 1 or more): the slot BN4's battle state +0x88 + 4 x side (its level 8 past it, "
                                     "0x080E69E8), BN6's +0xC0 (0x080E677C), each game's 0x41 its own code; BN6's also gives its "
                                     "wind's +0x4C its spawner (0x080E6692), which BN4's leaves as the slot had it"),
    # (Mettaur's, read from both games' code)
    (0x08007F04, 0x0800AA06): ('ok', "the appear queue's head tested (1 back where it is the object): BN4's one queue, 8 bytes of "
                                     "objects' +0xE at [[r10+0x18]+0x58]; BN6's queue r0 of four, 8 words of objects at 0x0800AAAE's "
                                     "place, the appear's 0 (0x080163D6), through QUEUED's shim"),
    (0x08007F1C, 0x0800AA1A): ('ok', "the object put at the appear queue's end where it is not in it (1 back where it was put): "
                                     "BN4's by its +0xE, BN6's by its pointer in queue r0, through QUEUED's shim (0, the queue "
                                     "BN4's appear and Mettaurs share, as BN4's one queue is both)"),
    (0x08007F46, 0x0800AA40): ('ok', "the object taken out of the appear queue and the rest moved up (BN4's 0x08007F6A, BN6's "
                                     "0x0800AA64, the same steps on bytes and words): r0 the queue, through QUEUED's shim"),
    (0x0801013A, 0x080165B8): ('ok', "the generic state 1 Mettaur's own state 1 wraps (its first own state begun: CurAction 6, BN6's "
                                     "8, +0xA 0): BN6's in its place, as every family's state 1 is BN6's; BN4's also clears +0's bit "
                                     "2, which BN6's leaves"),
    (0x080104AC, 0x08016B02): ('ok', "the generic state 4 Mettaur's own state 4 wraps (its hit's reaction until it ends, then its "
                                     "state back from +0x5C): BN6's in its place, as every family's state 4 is BN6's; BN6's calls "
                                     "its own two routines where BN4's calls one (0x0800C13C; BN6's 0x0800F394, 0x080101C4)"),
    (0x0800FE00, 0x08016396): ('ok', "BN4's plain appear (BN4_APPEAR), which its copied generic state 0 runs at +5 0: BN6's "
                                     "(BN6_APPEAR), as every family's state 0 runs it, the same steps but for BN4's battle kind "
                                     "0x44, its sound and step 2's end (docs/rom/bn4.md, A virus's appear)"),
    (0x0800FCDE, 0x08016396): ('ok', "BN4's appear of a battle whose settings' +6 has bit 0x20 (read at 0x0800FCBA, its copy reading "
                                     "BN6's settings there): BN6's plain appear, which BN6's generic state 0 runs for every virus a "
                                     "battle begins with, whatever its settings"),
    (0x0800FF26, 0x080164A0): ('ok', "BN4's appear of +5 1, which no spawn of the family gives (BN6_SPAWN_ENEMY_1's 1 made 2 by the "
                                     "summon's shim): BN6's appear of an enemy spawned into a fight under way in its place"),
    (0x0800B018, 0x0800EAFA): ('ok', "its collision data's word +0x1C set from r0 and its flag 8 set: BN6's, the word at BN6's +0x28 "
                                     "(COLL4TO6) and its flag by BN6's twin (0x08012F82 / 0x0801A152)"),
    (0x0800B026, 0x0800EB08): ('ok', "... the word cleared and the flag cleared: BN6's, at BN6's +0x28 (0x08012F8C / 0x0801A15C)"),
    (0x08007E46, 0x080011EC): ('ok', "BN4's battle kind ([[r10+0x18]+0xF]) its summoned appear's step 2 tests against 0x44 alone "
                                     "(0x08010114), both ways then showing the HP of a virus of the enemies' side (0x08010118-0x08010122; "
                                     "BN6's battle state +0xD and the object's +0x16 the same): 0 back, BN6's routine that returns 0, "
                                     "as no battle of BN6's is BN4's 0x44"),
    (0x08015ABC, 0x0801DC7C): ('ok', "an enemy's HP shown (r0, r1 its offset: an entry of BN4's 0x0203DC90, BN6's 0x0203EB50): BN6's, "
                                     "which BN6's own appears call in the same place (0x08016598), BN6's also giving its NameIDs "
                                     "0xEB and 0x49-0x4E a kind of their own"),
    (0x080E45F6, 0x0810A952): ('ok', "Mettaur's held chip's attack spawned (the fourth kind's 0x20; its state 9's case by the chip, "
                                     "0x080B399C): dead while its +0x2A stays 0xFFFF (it holds none): BN6's family nullsub"),
    (0x080E8ADA, 0x0810A952): ('ok', "Mettaur's held chip's other attack spawned (0x080B3C1A, its state 9's case 0x080B3A2A): dead "
                                     "while it holds none: BN6's family nullsub"),
    (0x08012ED0, 0x0801A082): ('ok', "its collision's region changed (r1 what it is, r2 what it strikes, r3 its setup's arg): "
                                     "BN6's, the setup's tail (0x08019FB4 from 0x08019FDC), its fields at BN6's offsets (+0xE, "
                                     "+0x2E, +0x30, +0x34; BN4's +0xC, +0x1E, +0x58, +0x5C); BN6's stores the second word at its "
                                     "+0x34 where BN4's stores it at r4's +0x5C, r4 the region's number (0x08012EEC: a write to the "
                                     "BIOS, which keeps nothing), so BN4's +0x5C keeps the setup's: Melody passes r2 2 at each of its "
                                     "calls, as its init's setup does, so BN6's +0x34 is written what it holds. BN6's also ORs the "
                                     "time stop's bit into +0x30 (0x0801A0A0), as both setups do and BN4's change does not: its calls "
                                     "are its states', which no spec reaches with time stopped (assumed)"),
    (0x080CD7E2, 0x080C6BDA): ('ok', "a collision region spawned (a third-kind object: its panel, element, damage and words): "
                                     "BN6's, the element at BN6's +0xE where BN4's +0x17, its side copied with the flip"),
    (0x080E50AE, 0x0810A952): ('ok', "VolGear's held chip's attack spawned (the fourth kind's 0x29, from 0x080B636E behind "
                                     "0x080B52E4's test of +0x2A): dead while its +0x2A stays 0xFFFF (it holds none): BN6's "
                                     "family nullsub"),
    (0x080B9704, 0x0810A952): ('ok', "VolGear's held chip used, by Billy's routine (0x080B54D0's numbers, then 0x080E514E's "
                                     "attack: from 0x080B638C behind 0x080B52E4's test): dead while its +0x2A stays 0xFFFF: BN6's "
                                     "family nullsub"),
    # (MetalMan's, read from both games' code)
    (0x08009BAC, 0x0800C9F8): ('ok', "a panel (r0, r1) of a floor (0x10) with none of its occupants' flags broken (type 1, its flags' "
                                     "low bits 1, the crack's sound), 1 back, else 0; BN6's the same at its offsets (flags +0x14, "
                                     "type +2 and its drawn type +6) and its own sound, but BN4's test of flag 0x20000 first, "
                                     "which only BN4's type 5 gives: no BN6 panel type's word has it (the chips' generator's "
                                     "audit of the same pair, tools/xchiprecipe_bn4.py AUDITS)"),
    (0x0800B230, 0x0800F614): ('ok', "an obstacle added to its side's list in the battle state (six words from BN4's +0x68, BN6's "
                                     "+0xA0, the first free one taken): the same steps but for that adds, each game's own list"),
    (0x0800B272, 0x0800F656): ('ok', "an obstacle taken out of that list (its word cleared where it is the object): the same steps "
                                     "but for the list's place, BN4's +0x68, BN6's +0xA0"),
    (0x0800B28E, 0x0800F672): ('ok', "an obstacle's lifetime (+0x20) counted down, at 0 its collision's region cleared and its HP "
                                     "(+0x24) 0, its last 0xB4 frames blinking: BN6's the same steps with the battle's pause tested "
                                     "(0x0800A03C) after the time stop both test, so its count stands while the battle is paused, "
                                     "where BN4's counts on (BN4's states never run while paused: the families' lag, legit.json)"),
    (0x08013444, 0x0801A4D0): ('ok', "a hit's push word (r0 | r1 << 8) set in its collision data: the same steps at each layout's "
                                     "place (BN4_COLL_PUSH +0x74, BN6_COLL_PUSH +0x12)"),
    (0x080B3C1A, 0x0810A952): ('ok', "VolGear's held chip 0x85 used, by Mettaur's routine (its other attack, 0x080E8ADA: from "
                                     "0x080B63F0, past its +0x2A compared with 0xFFFF at 0x080B63BE): dead while it holds none: "
                                     "BN6's family nullsub"),
}

# BN6's own third-kind types a family's call aims at (its pair's twin the type's spawner) whose code is BN4's but for a
# number BN6's own users need: copied whole for BN4's families with that number BN4's (tools/xrecipe_bn4_copy.py), run
# through the second-level dispatch as a moved type at a number of its own, BN6's own type left as it is: type -> its
# spawner, the immediates changed (each checked against BN6's code), why (docs/rom/bn4.md, The thunder's copy)
COPIED6 = {
    0x2A: dict(spawner=0x080CADBC, imms=(
        dict(at=0x080CACD6, rd=3, was=1, now=0,
             why="its collision set up (0x08019FB4) with r3 0, as BN4's init (0x080D208C) passes, where BN6's passes 1"),),
        why="the thunder Billy's attack spawns (0x080D2156, its pair BN6's spawner 0x080CADBC): BN6's init passes r3 = 1 to its "
            "collision setup (movs r3,#1 at 0x080CACD6, bl 0x08019FB4), BN4's 0 (movs r3,#0 at 0x080D208C, bl 0x080129AC, its "
            "twin), and the hit pass ORs it into the struck's hit bits, so a struck the paralysis does not take starts its "
            "struck reaction in BN6 and not in BN4 (billy_v1's comparison found it; with 0 there Billy's own records match "
            "BN4's through all 1200 frames). BN6's own two calls of the spawner (0x080EDB48, 0x08108464, by a scan of its "
            "BLs) keep BN6's 1, so BN6's 0x2A is never patched: a copy of it whose init passes 0 serves BN4's families"),
}

# BN4's routines its families' code calls that BN6 has no twin of, copied with the family (each a block of its own, its
# literal pool with it, the code between zeroed): entry -> why
COPIED = {
    0x0800AFB0: "the direction from its panel to another (r0-r3, 0-4) into its collision data's +0xA (BN6's +0xC): BN6's "
                "0x0800E994 answers neighbours alone (5 else), BN4's every panel",
    0x0800AF90: "... the direction itself, a leaf",
    0x080B52E4: "a held chip's test (+0x2A + 1): a leaf, BN6's +0x2A the same field",
    0x080B54D0: "a held chip's numbers read for its attack (getChip, BN6's twin): self-contained",
    0x0800AD2A: "the can-move check (its flags, then its panel's type): no BN6 routine of its body; ported, its panel's type read "
                "at BN6's +2 (BN4's +0)",
    0x0800AD7E: "the can-move check's third form, the same steps as 0x0800AD2A's with its own mask (the pool word at 0x0800B130 "
                "where 0x0800AD2A's reads 0x0800B128; 0x0800AD54 the second, 0x0800B12C): MagTect's shake's gate; copied with it, its "
                "panel's type read at BN6's +2",
    0x080B552A: "a panel of its side but its own at random, its row's first (its table of panels' masks by side): no BN6 routine "
                "of its body (BN6's families each keep their own); copied, its table with it",
    0x080B56A4: "a panel of its side but its own at random (its table of panels' masks by side): BN6's nearest (0x08015C48) "
                "takes the panels an attack object holds too (its masks 0x0F8800A0 where BN4's 0x3F8800A0); copied, its table "
                "with it",
    0x08010736: "a virus's after-state (BN4_VIRUS_PRESENT, its type routine's tail after each state): BN6's twin "
                "(0x08016E64, BN6_VIRUS_PRESENT, at BN6's virus type's tail, 0x0810A284) skips it while the battle is paused "
                "(0x08016E84-0x08016E90), BN4's never; copied, run after its state 0 (its appear) and its hook 5 (its spawn "
                "done) while BN6's skips it for the pause alone (XVirusFamily.present, xvirus_shim.h's paused present), so it "
                "animates from its spawn through its appear as in BN4",
}
# A copied rule whose answer BN6 numbers otherwise, by the object's side: each call of a family's code to it made a call
# to a shim of its home that calls the rule's copy and numbers its answer BN6's way (src/emu/xvirus_shim.h, a direction;
# XVirusFamily.sides): entry -> why
SIDED = {
    0x0800AF90: "BN4's direction rule (BN4_DIRECTION): right 4 and left 3 by the screen, which BN6's collision data holds by "
                "the object's side, as BN6's rule numbers them (0x0800E994, r4 its +0x16: right 4 and left 3 on side 0, 3 and "
                "4 on side 1; BN6's collision panels' update 0x0801A04C passes it) and BN6's code reads them (0x0800E498, by "
                "+0xC from 0x0800E468): its 3 and 4 swapped on side 1, up, down and none as they are (cirkill_v1 parted at t43 "
                "on its +0xC, BN6's copy writing BN4's 3 for a move to the left on side 1, where BN6's rule gives 4)",
}
# a copied routine's data past its pool, copied with it as it is (raw): entry -> (start, end, what)
COPIED_DATA = {
    0x080B552A: (0x080B5584, 0x080B5594, "its masks: two words a side, BN6's bits (bn6f's PanelData flags)"),
    0x080B56A4: (0x080B56D4, 0x080B56E4, "its masks: two words a side, BN6's bits (bn6f's PanelData flags)"),
}

# BN4's init's calls BN6's virus init (0x08016F56) does its own of, and so not the family's to run: entry -> why
INIT_ENGINE = {
    0x080167C0: "BN4's note of the enemy in one of three slots (0x0203BE40, its enemy record's +8 and +9 as an offset): BN6's "
                "virus init keeps its own (Billy's code audit took BN6's init whole)",
}
# the ObjectFlags1 an init sets, as BN6's virus init sets them by its struct2's flags (+3): the bit there each takes
INIT_FLAGS = {0x08000000: 0x10, 0x02000000: 0x08, 0x20: 0x04, 0x10: 0x02, 0x20000: 0x01}

# A hook entered past its routine's push {lr} (XvRef.entered: its init's or update's own step after the engine's work,
# which BN6's init or dispatcher does), read from its code: entry -> why (no register but r5 live there)
ENTRIES = {
    0x080BA48A: "Spikey's update past its pre-step, state and present (BN6's dispatcher runs each): its animation 2 back to "
                "0 at its last frame",
    0x080BA13E: "Shrimpy's update past its pre-step, state and present (BN6's dispatcher runs each): its animation 1 back "
                "to 0 at its last frame",
    0x080B8C08: "Melody's init from its own step on (INIT_AGAIN): its +0x2E by its version (0x080B8C38's byte, 30 each), its "
                "collision data, its set-up with region 0x35, +8 4 and its update's call (a nullsub: its update runs as its "
                "pre-step)",
    0x080BEBF6: "MagTect's update past its pre-step, state and present (BN6's dispatcher runs each): its shake (0x080BF064): "
                "unless the battle is paused (0x08007948) or the can-move check says not (0x0800AD7E, copied), every fourth frame "
                "of its +0x20 its sprite's offsets +0x36 and +0x3E moved to a random pair of its table's signed bytes, kept at "
                "+0x64 and +0x68",
    0x080BA11A: "Shrimpy's init past BN6's virus init's work (its sprite, its collision and its float, struct2's flag 4): "
                "its first way, up or down, at random (+7), then +8 4, as BN6's init sets it after, and its update's call (a "
                "nullsub: its tail is hook 3)",
}

# Its pools' words that look like addresses and are numbers, kept: value -> why
NUMBERS = {
    0x2A800000: "the flags its hits left it, tested as 0x0A800000 is, with 0x20000000: a word of both games' collision masks' "
                "tables (BN4's 0x0801268C, BN6's BN6_COLL_MASKS' 0x08019CA8)",
    0x0B800000: "a panel's flags it may not have (object_checkPanelParameters, BN6's 0x0800CC86 the same body): the support and "
                "alliance bits, BN6's own code naming the same mask (0x0800E6D4)",
    0x03800000: "a panel's flags it may not have (object_checkPanelParameters' r3, BN6's 0x0800CC86 the same body): 0x0B800000's "
                "bits but 0x08000000 (GutsMan's objects, 0x080D07C4)",
    0x04050601: "a collision region's spawn word (object_spawnCollisionRegion's r4), as 0x0405FF01: BN6's own attacks pass the same "
                "(five of its pools; GutsMan's states, MegaMan's 0x080F052C and 0x080F0606)",
    0x04080080: "a panel's flags (its panel's, 0x08009A86, BN6's 0x0800C8F8) CirKill waits on: 0x04000000 the enemy's alliance, "
                "0x00080000 and 0x80 as bn6f's PanelData flags have them (an occupant's, a panel struck)",
    0x0A050A01: "a collision region's spawn word (object_spawnCollisionRegion's r4, read by the third kind's 3's init, the same "
                "code in both: +4 its region kind 1, +5 its hit kind 0x0A, +6 and +7 the collision set-up's 5 and 0x0A): BN6's own "
                "attacks pass each byte the same, hit kind 0x0A in 0x06050A01 (six of its pools) and 0x15050A01 (two), which BN4 "
                "passes too, and 0x0A at +7 in 0x0A050101, 0x0A050701 and 0x0A05FF01 (BigHamr's, 0x080D6B1C)",
    0x0405FF01: "a collision region's spawn word (object_spawnCollisionRegion's r4: its hit's kinds), which BN6's own attacks "
                "pass the same (eight of its pools)",
    0x04050301: "a collision region's spawn word (MagTect's: its ldr r4 at 0x080BEF94, then bl 0x080CD7E2, object_spawnCollisionRegion, "
                "whose twin BN6's 0x080C6BDA takes r4 the same), the form of 0x04050601; BN6's ROM holds the word too (0x080E4A44, "
                "0x08101400)",
    0x0405FF08: "a collision region's spawn word (MagTect's: its ldr r4 at 0x080BF052, then bl 0x080CD7E2, object_spawnCollisionRegion, "
                "BN6's twin 0x080C6BDA), the form of 0x0405FF01",
    0x04050401: "a collision region's spawn word (Kilby's: its ldr r4 at 0x080BC946, then bl 0x080CD7E2, object_spawnCollisionRegion, "
                "whose twin BN6's 0x080C6BDA takes r4 the same), the form of 0x04050601 and 0x0405FF01; BN6's ROM holds the word "
                "too (0x080E4A4C)",
    0x04050101: "a collision region's spawn word (BurnerMan's flame's: its ldr r4 at 0x080D4006, then its spawner of the third "
                "kind 0x7C), the form of 0x04050601 and 0x04050301",
    0x08000000: "the flags its hits left it, tested for one bit (SparkMan's object, 0x080D16E8-0x080D16EC: its collision data's "
                "+0x54, BN6's +0x70): a bit of 0x0A800000 below, a word of both games' collision masks' tables alike",
    0x0A800000: "the flags its hits left it, tested (BN4_COLL_FROM_COLLISION): BN6's bits at FlagsFromCollision (+0x70), "
                "where the thunder's update tests the same mask, 0xFF800000, in both games",
}
# Tables of BN4's ROM a copied routine names whose BN6 twin holds the same entries, its literal given BN6's: address -> (BN6's,
# why), each compared entry by entry
DATA_TWINS = {
    0x080125B0: (0x08019B78, "the panels' patterns (24 pointers to lists of signed column and row offsets, 0x7F ending each), "
                             "0x08009EF6's and BN6's 0x0800CCD4's: every pattern the same, compared list by list"),
}
GET_PANEL = 0x08009ABC   # BN4's getPanel (BN6's 0x0800C90A): back r0 the panel, its type at +0 (BN6's +2)
# BN4's panel types as BN6's, where a copied routine compares one (bn4.h BN4_PANEL_*; docs/rom/bn4.md, Panel types): a
# number BN6's, or None where BN6 has no type of its meaning (the compare never holds in BN6, as in BN6's own routine)
PANEL4TO6 = {
    # BN4's 0xA, the holding panel (BN4_PANEL_HOLDING: a non-floating object's move stopped as it arrives, 0x0801033E; its
    # can-move refused while it stands there, 0x0800AD42): no type of BN6's holds, and BN6's can-move (0x0800E5E2, the same
    # flags test, 0x5040) has no panel test at all; its flags word 0x10210 BN6's conveyors' (9-12), which push instead
    0xA: None,
    # BN4's cracked (3) and grass (6): BN6's numbers and flags words (src/emu/bn4.h, Its panel types: 1 hole, 2 plain, 3
    # cracked, 4 poison, 6 grass and 7 ice are BN6's, by the word each gives in BN4_PANEL_FLAGS_OF and BN6_PANEL_FLAGS_OF),
    # compared by AquaUp's and GreenWd's sweep (0x080DF8B0, 0x080DF914: tools/xchiprecipe_bn4.py's Ligtnin)
    3: 3, 6: 6,
}

# A family's update that does work of its own between BN4's pre-step and its state runner: the update run as its pre-step
# (the installer's shim, after BN6's own pre-step, before its state: the place BN4 runs it), its calls to the pre-step,
# the runner and the present aimed at BN6's family nullsub: update -> why
PRESTEP = {
    0x080C2808: "Elmperor's: unless time stops (battle_isTimeStop, BN6's 0x0800A098), 0x080C286C: its collision data's poison "
                "timer (+6, BN6's +8) held at 0xFF, poison panels never striking it, and while its +0x60 is set 1 HP back "
                "every few frames (object_addHP)",
    0x080B8C3E: "Melody's: the effect its hits make (BN4_HIT_EFFECT, BN6's 0x0801A0D4), then, where the flags its hits left it "
                "(its collision's +0x54, BN6's +0x70) hold 0x0F800000, its collision's region cleared (0x08012DC0, BN6's "
                "0x0801A074) and its +0x68 made 1",
    0x080BD10C: "CirKill's: its +0x64 and +0x68 (the objects it spawned) let go once that object is no longer in play "
                "(its +0 byte 0, as BN6's objects' first byte)",
}

# A family's generic state of its own (its state table's entry 0-5 another routine than BN4's): a wrapper of the generic
# state, run in BN6's place, its call to BN4's generic state paired with BN6's: entry -> why
OWN_STATES = {
    0x080B3658: "Mettaur's state 1: on its first frame (+0xA 0) into the appear queue (its turn's, QUEUED) and +7 0, then BN4's "
                "generic state 1, BN6's in its place",
    0x080B366E: "Mettaur's state 2 (deletion's): on its first frame out of the appear queue, then BN4's generic state 2, BN6's",
    0x080B3680: "Mettaur's state 4: BN4's generic state 4, BN6's in its place, then where its CurAction is 6 (its turn) and +0x60 "
                "set, out of the appear queue and in at its end again, +0x60 and +7 0",
    0x080B36A2: "Mettaur's state 5: on its first frame +7 0, then BN4's generic state 5, BN6's in its place",
    0x080C2850: "Elmperor's state 2 (deletion's): on its first frame its companion (+0x6C) told to end (0x080E770C) and let go, "
                "then BN4's generic state 2, BN6's in its place",
}

# Tables of BN4's ROM out of every region a moved object's code names by a literal, copied whole with it as raw data:
# address -> its size, an entry's, the kinds its families spawn (bn4_twins.TYPES' spawns' r4), the places of a kind's sprite
# (list offset, number) and the loads of them (the sprite load whose r1 and r2 come from an entry), why
TABLES = {
    0x080CCC94: dict(size=8, entry=2, kinds=(2, 3), sprite=(0, 1), loads=(0x080CCCCC,),
                     why="the first kind's 0x5C's kinds (a Navi's companions: 2 bytes each, its sprite's list offset and "
                         "number), read by its init (0x080CCCBC, r0 its kind by +4); WindMan spawns kinds 2 and 3 at his "
                         "spawn (0x080C328C, 0x080C32A0)"),
    0x080CD524: dict(size=0xD4, entry=4, kinds=(7,), sprite=(0, 1), loads=(0x080CD628,),
                     why="the third kind's 0x02's kinds (53 of 4 bytes, to its block: its sprite's list offset and number, its "
                         "palette, its +0xC), read by its init (0x080CD61E, r4 its kind by +4); Roll's MegaMan's state 44 spawns "
                         "kind 7 (0x080EF9B0), list 3's 0xF (tools/xnavirecipe_bn4_objects.py list_tables)"),
    0x080CFB8C: dict(size=0x24, entry=4, kinds=(0, 1, 2, 3, 4, 5), sprite=(None, 0), loads=(0x080CFBFE,),
                     why="the third kind's 0x16's kinds (9 of 4 bytes, to its routine: its sprite's number in list 4, its +0x10, "
                         "its step's frames, its crack), read by its init (0x080CFBF2, r4 its kind by +4); Mettaur spawns its kind "
                         "by its version (its +4, 0x080B3ACE), its shockwave's sprite list 4 by a movs (None: no list in the table)"),
    0x080CE5B0: dict(size=0x130, entry=0x10, kinds=(10, 11, 12), sprite=(5, 6), loads=(0x080CE718,),
                     why="the third kind's 0x0B's kinds (19 of 16 bytes: its collision's set-up, element, hit effect, sprite, "
                         "speed), read by its init (0x080CE704, r4 its kind); CirKill spawns kinds 10-12 by its version "
                         "(0x080BD3E0-0x080BD3E4), as BN5's records of its own 0x0B are read"),
}

# TABLES' kind read by the families' generator alone (tools/xrecipe_bn4.py tables_of): FireMan's recipe moves the same third
# kind 0x1A with its table's sprite byte BN4's (tools/xnavirecipe_bn4.py, his spawns' kinds no constant), his to settle with
# his spec (docs/multirom/bn4-navis.md)
FAMILY_TABLES = {
    0x080D013C: dict(size=0x12, entry=6, kinds=(0, 1, 2), sprite=(None, 0), loads=(0x080D019E,),
                     why="the third kind's 0x1A's kinds (3 of 6 bytes, before its routine: its sprite's number in list 4, its "
                         "+0x20, its element at +4), read by its init (0x080D018C, r4 its kind by +4: its literal 0x080D0230); "
                         "VolGear spawns its kind by its version >> 1 (0x080B6876), its fire's sprite list 4 by a movs (None: "
                         "no list in the table), the three kinds' the same 0x0A as its third kind 0x19's (0x080CFFAE)"),
}

# A family moved for a summon alone (tools/xrecipe_bn4.py SUMMONED): its generic state 0 run as its own, BN4's
# (BN4_STATE_GENERIC_0) copied with BN4's summoned appear (BN4_SUMMONED_APPEAR, its +5 2, which the summon's shim writes after
# BN6's spawn), so a summoned virus arrives as BN4's does; its other appears paired with BN6's (AUDITS): region -> its end, why
SUMMON_APPEAR = {
    0x0800FCAC: (0x0800FCDE, "BN4's generic state 0: its appear by its +5 (0 the plain, 1, 2 or more the summoned) and the "
                             "battle settings' +6 bit 0x20"),
    0x0801002A: (0x0801013A, "BN4's summoned appear, its table (0x0801003C) and its three steps"),
}

# An init whose own step comes before its collision's set-up, or whose set-up names a region BN6's virus init gives none of
# (tools/xrecipe_bn4.py init_again): its init from that step run again at its spawn's end as hook 5 (entered: ENTRIES), BN6's
# init's work done; its tail's call to take a collision data aimed at a routine of BN6's that gives back the one BN6's init
# took (and nothing its set-up does not write again), its set-up BN6's run again with its own arguments: init -> its entry,
# those aims (BN4 -> (BN6, why)), why
INIT_AGAIN = {
    0x080B8BDC: dict(at=0x080B8C08, aims={
        0x08012496: (0x0801A074, "its collision data as BN6's init took it: BN6's 0x0801A074 gives back its +0x54 and clears its "
                                 "+1, which BN6's set-up after it writes 1 again (0x08019FD6)")},
        why="Melody's (BN4's 0x080B8BDC): its +0x2E from 0x080B8C38 by its version (30, where BN6's and BN4's spawns give 10, "
            "BN6's 0x0800774C), which the set-up copies into its collision's +7 (BN4's +5), and its set-up with region 0x35 (BN6's "
            "init's 1 with bit 2 in its alliance word, BN6_COLL_MASKS' 0x35 at its +0x30), BN6's set-up (0x08019FB4) run again: "
            "its other fields written as BN6's init wrote them (its owner, side, element, panel, damage and region 2), and "
            "nothing BN6's init wrote after it (+0x18, +0x3C's flags)"),
}

# BN4's appear queue's routines (bn4_battle.h BN4_QUEUE_*), one queue where BN6's (BN6_QUEUE_*) take its number in r0: each
# call of a family's code to one made a call to a shim of its home that makes r0 that number and goes on to BN6's twin
# (XvSoundCall, its slot the queue's number; xvirus.c's r0 calls): BN4's routine -> the queue
QUEUED = {0x08007F04: 0, 0x08007F1C: 0, 0x08007F46: 0}

# What a family keeps of BN4's for now, named over its recipe beside the generator's own interims: family -> lines
INTERIMS = {
    'Elmperor': (
        "its panel types 8 and 0xA (its attack's, 0x080DC780: 6, 7, 8, 4, 0xA, set by BN6's object_setPanelType) and the "
        "panel flags 0x1000 and 0x200 it reads its element by (0x080C2948: 0x400, 0x800, 0x1000, 0x100, 0x200) kept as BN4's "
        "numbers: 8 is BN6's volcano (an eruption every 140 frames) by its number and its word (0x11010), which is BN5's "
        "lava's too, and which of them BN4's 8 is stays unread; 0xA is BN4's holding panel, none in BN6 "
        "(bn4_twins.PANEL4TO6), where BN6's 0xA is a conveyor and its flag 0x200 the conveyors' (0x10210): the attack's "
        "fifth panel pushes in BN6, and Elmperor standing on a conveyor reads that element. 6, 7 and 4 (grass, ice, "
        "poison) and their flags 0x400, 0x800 and 0x100 are BN6's own numbers and words (BN4_PANEL_FLAGS_OF against "
        "BN6_PANEL_FLAGS_OF), decided",
        "a panel its attack changes keeps its new type: BN4 gives it 0xBE frames (+0x12) to change back, which BN6's panels "
        "count for its conveyors alone",
        "its aura (the fourth kind's 0x4C, its sprite drawn again in its element's colours through BN4's tile buffer 0x02037810) "
        "not drawn: its spawn answered 0 (bn4_twins.AUDITS); its HP's return goes on",
    ),
}
