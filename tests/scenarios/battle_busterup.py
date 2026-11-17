"""BusterUp in a run's battle: dealt, picked, used, and MegaMan's buster
a level stronger for the rest of the battle, as BN6's own (its misc
routine 0x0A adds the record's 1 to the battle's Attack, up to level 10;
a shot deals the level: 1 a shot, then 2). With BN4's ROM beside BN6's it
did nothing (a player's report, "Some chips just do nothing, like
Busterup"): BN6's misc family's routines were grown for BN4's chips with
7 of BN6's 23, so BusterUp's (and SandWrm, FireHit, GolmHit, JustcOne,
ColArmy and the rest past the seventh) ran a routine that does nothing."""

# (a run's first layer, MegaMan free on it; BN6's own area whatever the ROMs; no battle but the step's)
START = 'layer --seed 15 --net-biome 0 --dev god,quiet,hp=2000'

STEPS = [
    ('do', 'chip 175 * 29; foes 01@52'),              # BusterUp * in the folder's first 29; a Mettaur in MegaMan's row
    ('until', 'drawn~175*', 900),                     # the Custom screen: the hand dealt
    ('repeat', 'press A; wait 10', 'handpicks=1'),
    ('do', 'press START; wait 10; press A'),          # to OK, and OK
    ('until', 'hand=1', 600),
    ('expect', 'nextchip=175'),
    ('until', 'custom>=20', 900),                     # the fight well begun
    ('do', 'status paralysis 3000'),                  # the Mettaur held in MegaMan's row
    ('expect', 'busterattack=1'),
    ('expect', 'enemyhp=40'),
    ('repeat', 'press B; wait 30', 'enemyhp<40'),     # a shot: 1
    ('expect', 'enemyhp=39'),
    ('repeat', 'press A; wait 30', 'hand=0'),         # BusterUp used
    ('expect', 'busterattack=2'),
    ('repeat', 'press B; wait 30', 'enemyhp<39'),     # a shot: 2
    ('expect', 'enemyhp=37'),
    ('repeat', 'press B; wait 30', 'enemyhp<37'),     # and still 2
    ('expect', 'enemyhp=35'),
]

MANY = {
    'bn6': dict(START=START, STEPS=STEPS, GUARDS='BusterUp with BN6\'s ROM alone: the buster 1, then 2 a shot'),
    'bn4': dict(START=START, STEPS=STEPS, NEEDS=('bn4',),
                GUARDS='BusterUp with BN4\'s ROM beside BN6\'s: the buster 1, then 2 a shot (BN6\'s misc routines grown whole)'),
}
