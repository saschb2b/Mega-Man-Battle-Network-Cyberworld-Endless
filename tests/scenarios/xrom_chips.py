"""The older nets' chips in a run's battle: a chip of BN5's or BN4's held
in the folder, dealt on BN6's Custom screen, picked, used, and its power
taken off the foe's HP; and BN6's AquaNdl striking the foes moved in from
BN5 and BN4 (issue #523, 0.21.2: it found its targets by BN6's own names,
and its needles fell on nothing)."""

# (a run's first layer, MegaMan free on it; BN6's own area whatever the ROMs; no battle but the step's, and HP enough
# to stand a late version's shot while the chip is played)
START = 'layer --seed 15 --net-biome 0 --dev god,quiet,hp=2000'

# (the target: BN5's CanGard at its last version, which never leaves its panel: 280 HP, no element)
TARGET, TARGET_HP = '205@52', 280

# (chip: its id in BN6's tables, a code its record has, its power: one hit along MegaMan's row)
BN5 = {'WideSht1': (204, 'L', 70), 'Spreader': (207, 'C', 30), 'Pulsar1': (414, 'F', 70), 'Voltz1': (444, 'K', 100)}
BN4 = {'HeatShot': (588, 'B', 40), 'Bubbler': (591, 'P', 50), 'Thunder1': (647, 'B', 40)}


def deal(chip, code, foes):
    """The chip in the folder's first 29 entries, a battle of `foes`, its first hand dealt, one picked and OK."""
    return [
        ('do', f'chip {chip} {code} 29; foes {foes}'),
        ('until', f'drawn~{chip}{code}', 900),            # the Custom screen: the hand dealt
        ('expect', f'drawn= {chip}{code} {chip}{code} {chip}{code} {chip}{code} {chip}{code}'.replace('= ', '=')),
        ('expect', 'handpicks=0'),
        ('repeat', 'press A; wait 10', 'handpicks=1'),
        ('do', 'press START; wait 10; press A'),          # to OK, and OK
        ('until', 'hand=1', 600),                         # the chip in hand
        ('expect', f'nextchip={chip}'),
        ('until', 'custom>=20', 900),                     # the fight well begun (the Custom gauge a fifth full)
    ]


def played(game, name, chip, code, power):
    return dict(
        START=START, NEEDS=tuple({game, 'bn5'}),   # (the target is BN5's)
        GUARDS=f"{game.upper()}'s {name} in a run's battle: dealt, picked, used, its {power} off the foe's HP",
        STEPS=deal(chip, code, TARGET) + [
            ('expect', f'nextchipfrom={game}'),
            ('expect', f'nextpower={power}'),
            ('expect', f'enemyhp={TARGET_HP}'),
            ('repeat', 'press A; wait 30', 'hand=0'),     # used, once the fight has begun
            ('repeat', 'wait 20', f'enemyhp={TARGET_HP - power}', 20),
            ('do', 'wait 120'),
            ('expect', f'enemyhp={TARGET_HP - power}'),   # one hit, and no more
        ])


def needles(game, foe, name, foes, hp):
    return dict(
        START=START, NEEDS=(game,),
        GUARDS=f"Issue #523: AquaNdl thrown at {game.upper()}'s {foe}: its needles land ({name}, {hp} HP)",
        STEPS=deal(61, 'J', foes) + [
            ('expect', f'enemynames={name}'),
            ('expect', 'olderfoes=1'),
            ('expect', f'enemyhp={hp}'),
            ('do', 'status paralysis 600'),               # held on its panel: the needles fall where it stood as they were thrown
            ('repeat', 'press A; wait 30', 'hand=0'),
            ('repeat', 'wait 20', f'enemyhp<{hp}', 20),   # struck
        ])


MANY = {f'bn5/{name.lower()}': played('bn5', name, *chip) for name, chip in BN5.items()}
MANY.update({f'bn4/{name.lower()}': played('bn4', name, *chip) for name, chip in BN4.items()})
MANY.update({
    'aquandl/bn5-virus': needles('bn5', 'Bugtank', 'Bugtk3EX', '217@52', 290),
    'aquandl/bn5-navi': needles('bn5', 'KnightMan', 'KnightMn', '1D4@52', 600),
    'aquandl/bn4-virus': needles('bn4', 'Walla', 'WaldonEX', '2D7@52', 260),
    'aquandl/bn4-navi': needles('bn4', 'GutsMan', 'GutsM', '384@52', 500),
})
