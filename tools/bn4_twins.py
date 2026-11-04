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
)

# The generic states BN4's families' state tables open with (0-5, as BN5's: tools/xrecipe_family.py GENERIC5), each by
# its place there; BN6's of the same place stand in for them (its 6 and 7, frozen and bubbled, BN4 has none of)
GENERIC4 = {0: 0x0800FCAC, 1: 0x0801013A, 2: 0x0801014C, 3: 0x08010276, 4: 0x080104AC, 5: 0x080105BC}

# (BN4, BN6) -> (verdict, why): a pair whose bodies differ judged against what BN4's code asks of it (Billy's code audit,
# read from both games' code); any other DIFF pair a family reaches stays an open decision in the generator's report
AUDITS = {
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
    (0x080CD7E2, 0x080C6BDA): ('ok', "a collision region spawned (a third-kind object: its panel, element, damage and words): "
                                     "BN6's, the element at BN6's +0xE where BN4's +0x17, its side copied with the flip"),
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
    0x080B552A: "a panel of its side but its own at random, its row's first (its table of panels' masks by side): no BN6 routine "
                "of its body (BN6's families each keep their own); copied, its table with it",
    0x080B56A4: "a panel of its side but its own at random (its table of panels' masks by side): BN6's nearest (0x08015C48) "
                "takes the panels an attack object holds too (its masks 0x0F8800A0 where BN4's 0x3F8800A0); copied, its table "
                "with it",
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
    0x04080080: "a panel's flags (its panel's, 0x08009A86, BN6's 0x0800C8F8) CirKill waits on: 0x04000000 the enemy's alliance, "
                "0x00080000 and 0x80 as bn6f's PanelData flags have them (an occupant's, a panel struck)",
    0x0405FF01: "a collision region's spawn word (object_spawnCollisionRegion's r4: its hit's kinds), which BN6's own attacks "
                "pass the same (eight of its pools)",
    0x0A800000: "the flags its hits left it, tested (BN4_COLL_FROM_COLLISION): BN6's bits at FlagsFromCollision (+0x70), "
                "where the thunder's update tests the same mask, 0xFF800000, in both games",
}
GET_PANEL = 0x08009ABC   # BN4's getPanel (BN6's 0x0800C90A): back r0 the panel, its type at +0 (BN6's +2)
# BN4's panel types as BN6's, where a copied routine compares one (bn4.h BN4_PANEL_*; docs/rom/bn4.md, Panel types): a
# number BN6's, or None where BN6 has no type of its meaning (the compare never holds in BN6, as in BN6's own routine)
PANEL4TO6 = {
    # BN4's 0xA, the holding panel (BN4_PANEL_HOLDING: a non-floating object's move stopped as it arrives, 0x0801033E; its
    # can-move refused while it stands there, 0x0800AD42): no type of BN6's holds, and BN6's can-move (0x0800E5E2, the same
    # flags test, 0x5040) has no panel test at all; its flags word 0x10210 BN6's conveyors' (9-12), which push instead
    0xA: None,
}

# A family's update that does work of its own between BN4's pre-step and its state runner: the update run as its pre-step
# (the installer's shim, after BN6's own pre-step, before its state: the place BN4 runs it), its calls to the pre-step,
# the runner and the present aimed at BN6's family nullsub: update -> why
PRESTEP = {
    0x080C2808: "Elmperor's: unless time stops (battle_isTimeStop, BN6's 0x0800A098), 0x080C286C: its collision data's poison "
                "timer (+6, BN6's +8) held at 0xFF, poison panels never striking it, and while its +0x60 is set 1 HP back "
                "every few frames (object_addHP)",
    0x080BD10C: "CirKill's: its +0x64 and +0x68 (the objects it spawned) let go once that object is no longer in play "
                "(its +0 byte 0, as BN6's objects' first byte)",
}

# A family's generic state of its own (its state table's entry 0-5 another routine than BN4's): a wrapper of the generic
# state, run in BN6's place, its call to BN4's generic state paired with BN6's: entry -> why
OWN_STATES = {
    0x080C2850: "Elmperor's state 2 (deletion's): on its first frame its companion (+0x6C) told to end (0x080E770C) and let go, "
                "then BN4's generic state 2, BN6's in its place",
}

# Tables of BN4's ROM out of every region a moved object's code names by a literal, copied whole with it as raw data:
# address -> its size, an entry's, the kinds its families spawn (bn4_twins.TYPES' spawns' r4), the places of a kind's sprite
# (list offset, number) and the loads of them (the sprite load whose r1 and r2 come from an entry), why
TABLES = {
    0x080CE5B0: dict(size=0x130, entry=0x10, kinds=(10, 11, 12), sprite=(5, 6), loads=(0x080CE718,),
                     why="the third kind's 0x0B's kinds (19 of 16 bytes: its collision's set-up, element, hit effect, sprite, "
                         "speed), read by its init (0x080CE704, r4 its kind); CirKill spawns kinds 10-12 by its version "
                         "(0x080BD3E0-0x080BD3E4), as BN5's records of its own 0x0B are read"),
}

# What a family keeps of BN4's for now, named over its recipe beside the generator's own interims: family -> lines
INTERIMS = {
    'Elmperor': (
        "its panel types (its attack's, 0x080DC780: 6, 7, 8, 4, 0xA, set by BN6's object_setPanelType) and the panel flags it "
        "reads its element by (0x080C2948: 0x400, 0x800, 0x1000, 0x100, 0x200) kept as BN4's numbers: BN4's panel types are "
        "not mapped yet (the same interim as Billy's panel type 0xA; BN6's flag 0x200 is its conveyors')",
        "a panel its attack changes keeps its new type: BN4 gives it 0xBE frames (+0x12) to change back, which BN6's panels "
        "count for its conveyors alone",
        "its aura (the fourth kind's 0x4C, its sprite drawn again in its element's colours through BN4's tile buffer 0x02037810) "
        "not drawn: its spawn answered 0 (bn4_twins.AUDITS); its HP's return goes on",
    ),
}
