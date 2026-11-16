"""The run's opening gift: the Mr.Prog by the first layer's arrival offers
one of three (two HPMemory, a chip, a NaviCust program), on the pick's
own screen, which takes no A until it has been seen; each pick is taken
once, and the Prog has nothing more to give. After a run that ended
before its first guardian he adds an HPMemory."""


def gift(row, got):
    return dict(
        START='--scene emu --seed 3 --dev quiet',
        STEPS=[
            ('expect', 'layer=1'),
            ('expect', 'hp=100/100'),
            ('do', 'wait 150'),
            ('repeat', 'press A; wait 30', 'screen=reward-pick', 40),   # the arrival's words, then his
            ('expect', 'pick=gift'),
            ('expect', 'offers~hpmemory:2 chip:'),
            ('expect', 'offers~program:'),
            ('expect', 'pickrows=3'),         # no skip: the gift waits for its pick
            ('expect', 'pickrow=0'),
            ('expect', 'pickready=no'),       # an A pressed through his lines takes nothing
            ('do', 'press A; wait 20'),
            ('expect', 'screen=reward-pick'),
            ('do', 'press B; wait 20'),       # nor does B leave it
            ('expect', 'screen=reward-pick'),
            ('do', '; '.join(['press DOWN; wait 10'] * row) if row else 'wait 10'),
            ('expect', f'pickrow={row}'),
            ('repeat', 'wait 30', 'pickready=yes'),
            ('do', 'press A; wait 30'),
            ('expect', 'screen=map'),         # taken: "MegaMan got:"
            ('repeat', 'press A; wait 30', 'chat=closed', 20),
            ('expect', got),
            ('do', 'wait 30; press A; wait 60'),   # the Prog again: his thanks, no second gift
            ('repeat', 'press A; wait 30', 'chat=closed', 20),
            ('expect', 'screen=map'),
            ('expect', got),
        ],
    )


# a run that ended before its first guardian: the next run's Prog adds an HPMemory
EARLY = [
    ('do', 'wait 150; mash A 300; wait 20'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('do', 'foes 01@52 01@41 01@63'),
    ('until', 'phase=custom', 1200),
    ('repeat', 'press START; wait 10; press A; wait 60', 'phase=fight'),
    ('until', 'doing=gameover|scene=title', 3000),
    ('repeat', 'press A; wait 60', 'scene=title', 30),
    ('expect', 'ended=deleted'),
    ('restart', '--scene emu --seed 3 --dev quiet'),
    ('do', 'wait 150'),
    ('repeat', 'press A; wait 30', 'screen=reward-pick', 40),
    ('expect', 'pick=gift'),
    ('expect', 'hp=120/120'),                 # his extra, given with his words
    ('expect', 'offers~hpmemory:2 chip:'),
    ('repeat', 'wait 30', 'pickready=yes'),
    ('do', 'press A; wait 30'),
    ('repeat', 'press A; wait 30', 'chat=closed', 20),
    ('expect', 'hp=160/160'),
]

MANY = {
    'hpmemory': gift(0, 'hp=140/140'),
    # (a chip and a program leave his HP as it was; what the Pack and the
    # NaviCust then hold is the layer scenarios' to read)
    'chip': gift(1, 'hp=100/100'),
    'program': gift(2, 'hp=100/100'),
    'after-an-early-loss': dict(START='--scene emu --seed 5 --run-depth 2 --dev fragile', STEPS=EARLY),
}
