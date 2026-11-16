"""Mystery Data of each colour, from the A to what MegaMan is left with:
a green and a blue one taken, once, a purple one locked until an
Unlocker opens it, the Unlocker spent and its chip in the Pack; and what
was taken stays taken through a quit and a CONTINUE."""

# (an act's middle layer of all three colours, MegaMan put beside each;
# a run's second layer starts from nothing: 100 HP, no zenny, 30 chips)
LAYER = 'CYBERWORLD_STATE_POS=1 --scene emu --seed 3 --run-depth 2 --dev quiet'
HOLDS = 'hp zenny bugfrags chips programs unlockers regmemory scrtdata'
# (what a Mystery Data can hold: a chip, zenny, BugFrags, HPMemory, a RegUP, an Unlocker, ScrtData)
GOT = 'zenny>0|bugfrags>0|chips>30|hp>100|unlockers>0|regmemory>4|scrtdata>0'

ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),                    # the arrival's words paged
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('expect', 'held=none'),
]
# (a chat's boxes to its end, and MegaMan's word after it where he has one: a RegUP's)
PAGED = [('repeat', 'press A; wait 30', 'held=none'), ('do', 'wait 40'), ('repeat', 'press A; wait 30', 'held=none')]


def taken(colour):
    """A `colour` Mystery Data opened with A: counted taken, something in
    MegaMan's hands."""
    return ARRIVE + [
        ('keep', f'{colour}taken'),
        ('do', f'by {colour}; wait 10; press A; wait 40'),
        ('expect', 'chat=open'),
        *PAGED,
        ('moved', f'{colour}taken', 1),
        ('expect', GOT),
    ]


MANY = {
    'green': dict(START=LAYER, STEPS=taken('green')),
    # ... and gone: an A where it stood gives nothing more (no one else stands in reach of this one)
    'blue': dict(START=LAYER, STEPS=taken('blue') + [
        ('keep', f'bluetaken {HOLDS}'),
        ('do', 'press A; wait 60'),
        *PAGED,
        ('same', f'bluetaken {HOLDS}'),
    ]),
    # a purple one without an Unlocker: MegaMan says it is locked, and it stays
    'purple-locked': dict(START=LAYER, STEPS=ARRIVE + [
        ('expect', 'unlockers=0'),
        ('keep', HOLDS),
        ('do', 'by purple; wait 10; press A; wait 40'),
        ('expect', 'chat=open'),
        *PAGED,
        ('expect', 'purpletaken=0'),
        ('same', HOLDS),
    ]),
    # ... with one: the question's Yes opens it, the Unlocker spent, its chip in the Pack
    'purple-unlocked': dict(START=f'{LAYER} --talk keys:10', STEPS=ARRIVE + [
        ('expect', 'unlockers=1'),
        ('keep', 'chips'),
        ('do', 'by purple; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=1/2'),
        ('do', 'press A; wait 30'),
        *PAGED,
        ('expect', 'purpletaken=1'),
        ('expect', 'unlockers=0'),
        ('moved', 'chips', 1),
    ]),
    # ... and its No keeps both
    'purple-kept-shut': dict(START=f'{LAYER} --talk keys:10', STEPS=ARRIVE + [
        ('keep', HOLDS),
        ('do', 'by purple; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=1/2'),
        ('do', 'press RIGHT; wait 10'),
        ('expect', 'choice=2/2'),
        ('do', 'press A; wait 30'),
        *PAGED,
        ('expect', 'purpletaken=0'),
        ('same', HOLDS),
    ]),
    # taken, then a quit and a CONTINUE: still taken, what it held still held
    'taken-through-a-continue': dict(START=LAYER, STEPS=ARRIVE + [
        ('do', 'by green; wait 10; press A; wait 40'),
        *PAGED,
        ('expect', 'greentaken=1'),
        ('keep', f'greentaken {HOLDS}'),
        ('restart', 'CYBERWORLD_STATE_POS=1 --scene title'),
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('expect', 'cursor=continue'),
        ('repeat', 'press A; wait 40', 'scene=emu'),
        ('until', 'screen=map', 1200),
        ('until', 'held=none', 600),
        ('same', f'greentaken {HOLDS}'),
    ]),
}
