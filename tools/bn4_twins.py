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
    # Walla's, Elmperor's and CirKill's (bn6f's names located in BN4 and in Gregar, .build/bn4-fam2; each body read), and their
    # moved objects'
    (0x0800A17A, 0x0800D048, 'families-2'), (0x08012DB2, 0x0801A066, 'families-2'), (0x08012F74, 0x0801A140, 'families-2'),
    (0x080143FC, 0x0801BBAC, 'families-2'), (0x080129FC, 0x0801A00E, 'families-2'), (0x0800AE10, 0x0800E6E8, 'families-2'),
    (0x0801014C, 0x080165C2, 'families-2'), (0x08009DA0, 0x0800CBD8, 'families-2'), (0x08009DC4, 0x0800CC0A, 'families-2'),
    (0x08009EE4, 0x0800CCBE, 'families-2'), (0x08010560, 0x08016C8A, 'families-2'), (0x0800ABCA, 0x0810A952, 'families-2'),
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
    # (Walla's, Elmperor's and CirKill's code and their moved objects', read from both games' code, .build/bn4-fam2)
    (0x0800A17A, 0x0800D048): ('ok', "the row of the other side's player (battle_findPlayer, BN6's twin): BN6's "
                                     "object_getEnemyPlayerPanelY, his row where he stands on rows 1-3, his future row (+0x15) else, "
                                     "BN4's his row; the same while he stands on the field, where Walla and Spikey ask it"),
    (0x0800ACA6, 0x0800E456): ('ok', "object_getFlip: BN4's its alliance, BN6's its alliance XOR its flip (+0x17), which no moved "
                                     "family or object writes (BN6's spawn leaves it 0): the same"),
    (0x08012496, 0x08019892): ('ok', "object_createCollisionData: BN6's from its own pool of BN6's size (0xA8 where 0x90), zeroed as "
                                     "BN4's, +0x54 the object's"),
    (0x080129AC, 0x08019FB4): ('ok', "object_setupCollisionData, r1-r3 the same: BN6's writes BN6's offsets from the object's "
                                     "element (+0x0E, BN4's +0x17 moved there), its alliance and flip, its panel, +0x2E and +0x2C"),
    (0x080129FC, 0x0801A00E): ('ok', "object_removeCollisionData: BN6's the same walk of the region's panels in IWRAM "
                                     "(0x0300754D), its collision data's panels at BN6's offsets"),
    (0x08012D62, 0x0801A018): ('ok', "object_presentCollisionData: the frame's hits gathered (BN4's +0x54 into +0x80, BN6's +0x70 "
                                     "into +0x54) and cleared, unless time stops, then the panels' claims (IWRAM): BN6's for "
                                     "BN6's layout"),
    (0x08012F44, 0x0801A0D4): ('ok', "object_spawnCollisionEffect: the hit's effect spawned where its collision data was struck, "
                                     "BN6's reading BN6's offsets"),
    (0x080143FC, 0x0801BBAC): ('ok', "object_updateSprite: BN6's the same steps, its pause test BN6's (0x0800A03C, BN4's 0x080078EC) "
                                     "with its flags set by its own bne"),
    (0x0801014C, 0x080165C2): ('ok', "the generic state 2 (deletion's) Elmperor's own state 2 wraps: BN6's in its place, as every "
                                     "family's state 2 is BN6's; BN6's first runs its own tests (+0x40000000, 0x0800A03C)"),
    (0x08009DA0, 0x0800CBD8): ('ok', "a panel highlighted where the attack will strike (its column r0, row r1, a valid panel's): "
                                     "BN6's object_highlightPanel sets the panel's +1, where BN4 keeps a byte a panel at 0x02037ABE"),
    (0x08009DC4, 0x0800CC0A): ('ok', "a panel's type set (r0 column, r1 row, r2 type) where the panel is not a hole: BN6's "
                                     "object_setPanelType; BN4's also gives it 0xBE frames (+0x12) to change back, which BN6's panels "
                                     "do not count but for its conveyors (an interim: the panel keeps its type)"),
    (0x08009EE4, 0x0800CCBE): ('ok', "object_highlightCurrentCollisionPanels: BN6's reads its panel and region at BN6's offsets, its "
                                     "alliance XOR its flip (0)"),
    (0x08010560, 0x08016C8A): ('ok', "object_genericDestroy, a moved object's jump table's third entry: its collision data freed, "
                                     "the object freed; BN6's frees its sprite's data first (0x0801BB78)"),
    (0x0800ABCA, 0x0810A952): ('ok', "Walla's Recov80 or Recov150 used (a held chip, its +0x2A 0x6E or 0x70): dead in BN6 while its "
                                     "+0x2A stays 0xFFFF (it holds none): BN6's family nullsub"),
    (0x080164B4, 0x0810A952): ('ok', "Walla's held chip spent after its recovery: dead while its +0x2A stays 0xFFFF: BN6's family "
                                     "nullsub"),
    (0x080E407A, 0x0810A952): ('ok', "Walla's held chip's attack spawned (behind 0x080B52E4's test of +0x2A): dead while it holds "
                                     "none: BN6's family nullsub"),
    (0x080E76EA, 0x080011EC): ('ok', "Elmperor's aura spawned (the fourth kind's 0x4C: its sprite copied in its panel's colours "
                                     "through BN4's tile buffer 0x02037810, which has no twin mapped): answered 0, BN6's routine that "
                                     "returns 0, so its +0x6C stays 0 (an interim: the aura not drawn; its HP's return goes on)"),
    (0x080CDCE4, 0x0801156A): ('ok', "an object's hit taken from its HP (r0 0: off its +0x24, -1 back where none is left, else a white "
                                     "flash): BN6's basicHandleBattleObjectDamage, its r0 0 path the same, its sound BN6's (0x85, BN4's "
                                     "0xC8)"),
    (0x080E770C, 0x0810A952): ('ok', "Elmperor's aura told to end (its +8 set 8): reached only where +0x6C holds one, never while "
                                     "the aura is answered 0: BN6's family nullsub"),
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
}
GET_PANEL = 0x08009ABC   # BN4's getPanel (BN6's 0x0800C90A): back r0 the panel, its type at +0 (BN6's +2)
# BN4's panel types as BN6's, where a copied routine compares one: none known yet (an interim the report lists)
PANEL4TO6 = {}

# BN4's object types its families' code spawns and the decision for each (one a type for every family): 'moved' (its type's
# routine's region copied with the family, run through the second-level dispatch at a number of BN4's, xrecipe_bn4.TYPE_FIRST)
# where BN6's type of the number runs other code
TYPES = {
    (3, 0x3C): ('moved', "Walla's ring (its spawner 0x080D4D06): BN6's 0x3C (0x080CCF69) another object, its init a panel's "
                         "strike with effect 0x22, no motion; BN4's flies at a speed of its own (+0x40, +0x44)"),
    (3, 0x79): ('moved', "Elmperor's attack (its spawner 0x080DC78A), the panel it strikes changed to its element's type: "
                         "BN6's 0x79 (0x080D5CB1) another object, nearest none"),
    (3, 0x0B): ('moved', "CirKill's attack (its spawner 0x080CE80E): BN6's 0x0B (0x080C7919) another object; BN5's 0x0B, moved by "
                         "BN5's track, a kin of it (0.89 by body), not the same"),
}

# Literal words its families' code holds that look like addresses and are numbers, each read by hand: value -> why
KEPT = {
    0x0A800000: "a collision mask's bits (the alliance and support bits BN5's KEPT reads in BN6 the same, e.g. BN6's 0x080C7386), "
                "tested against what struck it (BN4's collision +0x54, BN6's +0x70)",
    0x2A800000: "... the same with 0x20000000, a word of the collision masks' tables of both games (BN4's 0x0801268C, BN6's "
                "BN6_COLL_MASKS' 0x08019CA8)",
    0x0B800000: "a panel's flags it may not have (object_checkPanelParameters, BN6's 0x0800CC86 SAME): the support and alliance "
                "bits, BN6's own code naming the same mask (0x0800E6D4)",
    0x04080080: "a panel's flags (its panel's, 0x08009A86, BN6's 0x0800C8F8) CirKill waits on: 0x04000000 the enemy's alliance, "
                "0x00080000 and 0x80 as BN6's panel flags have them (an occupant's, a panel struck)",
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
