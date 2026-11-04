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
}

# BN4's routines its families' code calls that BN6 has no twin of, copied with the family (each a block of its own, its
# literal pool with it, the code between zeroed): entry -> why
COPIED = {
    0x080B52E4: "a held chip's test (+0x2A + 1): a leaf, BN6's +0x2A the same field",
    0x080B54D0: "a held chip's numbers read for its attack (getChip, BN6's twin): self-contained",
    0x0800AD2A: "the can-move check (its flags, then its panel's type): no BN6 routine of its body; ported, its panel's type read "
                "at BN6's +2 (BN4's +0)",
}
GET_PANEL = 0x08009ABC   # BN4's getPanel (BN6's 0x0800C90A): back r0 the panel, its type at +0 (BN6's +2)
# BN4's panel types as BN6's, where a copied routine compares one: none known yet (an interim the report lists)
PANEL4TO6 = {}
