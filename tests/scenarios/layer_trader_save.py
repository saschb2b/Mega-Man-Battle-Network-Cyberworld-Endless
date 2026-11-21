"""A Chip Trader's trades on a layer whose vault is open: each trade ends
in BN6's own save, which holds the game's RAM masked for some frames as it
writes the cartridge; no reward pick opens there, nor after, and the
vault's pick still opens from the vault alone (issue #734)."""

# a Chip Trader and a collector's vault on one layer; library=1 opens the
# vault, so its pick is made with the layer; pack=120 chips to trade. The
# ninth trade's save masks with a key that reads as the vault's pick's flags
TRADES = 9
START = 'CYBERWORLD_STATE_POS=1 --scene emu --seed 14 --run-depth 4 --dev quiet,library=1,pack=120'

ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('expect', 'held=none'),
]


def trade():
    """One trade from BN6's trader screen: three chips in, the screen gone,
    BN6's save, the chip named and "Try again?" asked."""
    return [
        ('repeat', 'press A; wait 30', 'doing=menu'),       # Yes: BN6's trader screen
        ('do', 'wait 60'),
        ('repeat', 'press A; wait 40', 'choice=1/2'),       # three chips marked, then "Is this OK? > Yes"
        ('do', 'press A; wait 30'),
        ('until', 'doing=map|screen=reward-pick', 900),
        ('do', 'wait 40'),                                   # BN6's save, its RAM masked a few frames
        ('expect', 'screen!=reward-pick'),                  # the bug: a reward pick opened over it
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
    ('do', 'wait 120'),
    ('expect', 'screen!=reward-pick'),                       # nor once the trader's chat has closed
    # the vault's own pick still opens from its own talk
    ('do', 'by vault; wait 10'),
    ('repeat', 'press A; wait 30', 'pickat=1/3', 30),
    ('expect', 'pick=vault'),
]
