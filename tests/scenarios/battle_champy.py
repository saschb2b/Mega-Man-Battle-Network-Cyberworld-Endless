"""BN6's Champy chips in a run's battle: FireHit1-3 (a Champy on each
card, its fiery punch at the first foe ahead) and FlmHook1, the Program
Advance three FireHit1 of codes in a row form. Each dealt, used at a
virus held on its panel, and its power taken off the virus's HP, with
BN6's ROM alone and with BN4's beside it. A player's report (Reddit,
17 November 2026: "any of the Champy chips" did nothing): where BN4's ROM
was read, BN6's misc family's routines were grown from 7 of its 23
(BN6_MISC_ROUTINE_COUNT), and every chip of a subfamily past 6 ran the
stub that does nothing: MegaMan vanished, no Champy came, no hit
(docs/rom/chips-code.md, BN6's misc family and its routines grown; the
fix BusterUp's, tests/scenarios/battle_busterup.py)."""

# (a run's first layer, MegaMan free on it; no battle but the step's, and HP enough to stand the foe while the chip is played)
START = 'layer --seed 15 --net-biome 0 --dev god,quiet,hp=2000'

# (BN6's Mettaur2 and RareMttr2, no element: 80 and 180 HP)
METTAUR2, RAREMTTR2 = ('02@52', 80), ('06@52', 180)

# (chip: its id, a code its record has, its power, the foe)
FIREHITS = {'firehit1': (107, 'D', 60, METTAUR2), 'firehit2': (108, 'R', 120, RAREMTTR2), 'firehit3': (109, 'A', 180, RAREMTTR2)}


def used(power, hp):
    """The chip in hand used at the foe, held paralysed on its panel, and its power off the foe's HP, once."""
    left = max(0, hp - power)
    return [
        ('until', 'custom>=20', 900),                     # the fight well begun (the Custom gauge a fifth full)
        ('do', 'status paralysis 600'),                   # held on its panel, in MegaMan's row
        ('expect', f'enemyhp={hp}'),
        ('repeat', 'press A; wait 30', 'hand=0'),         # used, once the fight has begun
        ('repeat', 'wait 20', f'enemyhp={left}', 20),     # struck: the stub of the bug left it whole
        ('do', 'wait 120'),
        ('expect', f'enemyhp={left}'),                    # one hit, and no more
    ]


def firehit(chip, code, power, foe, needs):
    foes, hp = foe
    return dict(
        START=START, NEEDS=needs,
        GUARDS=f"FireHit {chip} {code} in a run's battle{' with BN4' if needs else ''}: dealt, used, its {power} off the foe's HP",
        STEPS=[
            ('do', f'chip {chip} {code} 29; foes {foes}'),
            ('until', f'drawn~{chip}{code}', 900),        # the Custom screen: the hand dealt
            ('repeat', 'press A; wait 10', 'handpicks=1'),
            ('do', 'press START; wait 10; press A'),      # to OK, and OK
            ('until', 'hand=1', 600),
            ('expect', f'nextchip={chip}'),
            ('expect', f'nextpower={power}'),
        ] + used(power, hp))


def flmhook(needs):
    foes, hp = RAREMTTR2
    return dict(
        START=START, NEEDS=needs,
        GUARDS=f"FlmHook1, formed from FireHit1 D, E and F{' with BN4' if needs else ''}: used, its 300 off the foe's HP",
        STEPS=[
            # (the folder's first 5 D, then E to 15, F to 29: this seed's first hand E E F D D)
            ('do', f'chip 107 F 29; chip 107 E 15; chip 107 D 5; foes {foes}'),
            ('until', 'drawn~107', 900),
            ('expect', 'drawn=107E 107E 107F 107D 107D'),
            # (D, E, F picked in order: the fourth card, the second, the third)
            ('do', 'press RIGHT; wait 6; press RIGHT; wait 6; press RIGHT; wait 6; press A; wait 10; '
                   'press LEFT; wait 6; press LEFT; wait 6; press A; wait 10; press RIGHT; wait 6; press A; wait 30'),
            ('expect', 'handpicks=3'),
            ('do', 'press START; wait 10; press A'),
            ('until', 'hand=1', 600),
            ('expect', 'nextchip=326'),                   # the Program Advance formed: FlmHook1
            ('expect', 'nextpower=300'),
        ] + used(300, hp))


MANY = {}
for name, chip in FIREHITS.items():
    MANY[name] = firehit(*chip, ())
    MANY[f'{name}/bn4'] = firehit(*chip, ('bn4',))
MANY['flmhook1'] = flmhook(())
MANY['flmhook1/bn4'] = flmhook(('bn4',))
