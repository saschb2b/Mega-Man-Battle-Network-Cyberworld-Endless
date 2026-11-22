"""A Chip Trader's trades with MegaMan hurt: BN6's save after each holds
the game's RAM masked some frames, and the run's logic leaves it alone
until it unmasks. BN6 sums the RAM before its mask and again after: a
byte written meanwhile (the HP-full flag the run kept from MegaMan's
masked HP, the dev tools' flags) failed the sum, and BN6 masked and saved
it all again under a new key, the run writing into that one too (issue
#734's sequel). One mask a trade; his HP, the zenny and the layer as
they were."""

TRADES = 9
START = 'CYBERWORLD_STATE_POS=1 --scene emu --seed 14 --run-depth 4 --dev quiet,library=1,pack=120,hp=200/100'

ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('expect', 'held=none'),
    ('do', 'wait 600'),                                      # (hp=200/100 holds his HP through the run's first 600 frames)
    ('expect', 'hp=100/200'),
]


def trade():
    """One trade from BN6's trader screen, its save checked once the chip
    is named: one mask, nothing of the run's moved."""
    return [
        ('keep', 'savemasks hp zenny layer'),
        ('repeat', 'press A; wait 30', 'doing=menu'),       # Yes: BN6's trader screen
        ('do', 'wait 60'),
        ('repeat', 'press A; wait 40', 'choice=1/2'),       # three chips marked, then "Is this OK? > Yes"
        ('do', 'press A; wait 30'),
        ('until', 'doing=map', 900),
        ('do', 'wait 40'),                                   # BN6's save, its RAM masked a few frames
        ('expect', 'savemasked=no'),
        ('moved', 'savemasks', 1),                           # the bug: masked and saved again, the run's write failing its sum
        ('same', 'hp zenny layer'),
        ('repeat', 'press A; wait 30', 'choice=1/2'),       # the chip named; "Try again? > Yes"
    ]


STEPS = ARRIVE + [
    ('do', 'by trader; wait 10'),
    ('repeat', 'press A; wait 30', 'choice=1/2'),
]
for k in range(TRADES):
    STEPS += trade()
STEPS += [
    ('repeat', 'press RIGHT; wait 10', 'choice=2/2'),       # "Try again? Yes > No"
    ('do', 'press A; wait 30'),
    ('repeat', 'press A; wait 30', 'held=none', 30),
    ('expect', 'screen=map'),
    ('expect', 'hp=100/200'),
]
