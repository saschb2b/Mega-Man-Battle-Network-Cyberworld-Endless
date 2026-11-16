"""The net's locks and set pieces, each as a player meets it: a Security
Cube asks its P-Code and opens once a Navi has told it, a toll's cube
opens for zenny and stays for none, Rush lies in his gap for a RushFood,
a Link Navi's obstacle goes for the right Cross alone, a teleport's pad
leads to its twin, an arrow lane carries MegaMan to its end, an invisible
path holds under him to its Mystery Data."""

# (CYBERWORLD_STATE_POS: the state says what holds MegaMan, `held none` once he is free)
RUN = 'CYBERWORLD_STATE_POS=1 --scene emu --run-depth 2'
CUBE = f'{RUN} --seed 1 --dev quiet'          # RoboDog Comp: a Security Cube, a blue Mystery Data beside it
TOLL = f'{RUN} --seed 6 --dev quiet'          # ... a toll's
GAP = f'{RUN} --seed 5 --dev quiet'           # Seaside Area: Rush's gap, one panel
GEYSER = f'{RUN} --seed 2 --net-biome 1 --dev quiet,pieces=40'   # ... a geyser of cyberwater
TELEPORT = f'{RUN} --seed 2 --dev quiet'      # Central Area: a teleport pair
LANE = f'{RUN} --seed 1 --net-biome 7 --dev quiet,pieces=32'     # the Cybeast Nest: an arrow lane
PATH = f'{RUN} --seed 3 --dev quiet,pieces=64'                   # Seaside Area: an invisible path

ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),                    # the arrival's words paged
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('expect', 'held=none'),
    ('expect', 'locks>=0'),
]
PAGED = ('repeat', 'press A; wait 30', 'held=none')             # a chat's boxes to its end, MegaMan free again
# (A at the lock, twice: on this seed the first takes the Mystery Data that stands beside the cube)
AT_LOCK = [('do', 'by block; wait 10; press A; wait 40'), PAGED, ('do', 'by block; wait 10; press A; wait 40'), PAGED]

MANY = {
    # a Security Cube before its P-Code is told: it asks, and stands
    'cube-asks-its-code': dict(START=CUBE, STEPS=ARRIVE + [
        ('expect', 'locks=1'),
        ('expect', 'pcode=untold'),
        *AT_LOCK,
        ('expect', 'locks=1'),
    ]),
    # ... a Navi on the layer knows it: told, the cube takes it and opens
    'cube-opens-for-its-code': dict(START=CUBE, STEPS=ARRIVE + [
        ('do', 'by teller; wait 10; press A; wait 40'),
        ('expect', 'chat=open'),
        PAGED,
        ('expect', 'pcode=told'),
        *AT_LOCK,
        ('expect', 'locks=0'),
    ]),
    # a toll's cube: Yes pays its zenny and it opens
    'toll-paid': dict(START=f'{TOLL} --talk zenny:10', STEPS=ARRIVE + [
        ('expect', 'locks=1'),
        ('keep', 'zenny'),
        ('do', 'by block; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=1/2'),
        ('do', 'press A; wait 30'),
        PAGED,
        ('expect', 'locks=0'),
        ('expect', 'zenny<10000'),
    ]),
    # ... No keeps the zenny and the cube
    'toll-declined': dict(START=f'{TOLL} --talk zenny:10', STEPS=ARRIVE + [
        ('keep', 'zenny'),
        ('do', 'by block; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=1/2'),
        ('do', 'press RIGHT; wait 10'),
        ('expect', 'choice=2/2'),
        ('do', 'press A; wait 30'),
        PAGED,
        ('expect', 'locks=1'),
        ('same', 'zenny'),
    ]),
    # ... and with no zenny a Yes opens nothing
    'toll-not-afforded': dict(START=TOLL, STEPS=ARRIVE + [
        ('expect', 'zenny=0'),
        ('do', 'by block; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=1/2'),
        ('do', 'press A; wait 30'),
        PAGED,
        ('expect', 'locks=1'),
        ('expect', 'zenny=0'),
    ]),
    # Rush's gap with RushFood held: A at its edge, Rush eats one and lies in it; over him, the island's Mystery Data
    'rush-gap-bridged': dict(START=f'{GAP} --talk rushfood:10', STEPS=ARRIVE + [
        ('keep', 'rushfood bluetaken'),
        ('do', 'by gap; wait 10; press A; wait 60'),
        ('until', 'chat=open', 600),
        PAGED,
        ('moved', 'rushfood', -1),
        ('do', 'hold AHEAD 60; press A; wait 40'),              # across Rush's back, A at the data on the island
        PAGED,
        ('moved', 'bluetaken', 1),
    ]),
    # ... and without: no Rush, and the gap holds MegaMan on his side
    'rush-gap-without-food': dict(START=GAP, STEPS=ARRIVE + [
        ('expect', 'rushfood=0'),
        ('keep', 'bluetaken'),
        ('do', 'by gap; wait 10; press A; wait 60'),
        PAGED,
        ('do', 'hold AHEAD 60; press A; wait 40'),
        PAGED,
        ('same', 'bluetaken'),
    ]),
    # a teleport's pad: stepped on, MegaMan stands on its twin, then walks free
    'teleport-leads-to-its-twin': dict(START=TELEPORT, STEPS=ARRIVE + [
        ('do', 'by teleport; wait 2'),
        ('expect', 'on=teleport1'),
        ('until', 'on=teleport2', 300),
        ('until', 'held=none', 300),
    ]),
    # a Link Navi's obstacle, a geyser of cyberwater: MegaMan alone names who could clear it, and it stands
    'obstacle-needs-a-cross': dict(START=GEYSER, STEPS=ARRIVE + [
        ('expect', 'locks=1'),
        ('do', 'by block; wait 10; press A; wait 40'),
        ('expect', 'chat=open'),
        PAGED,
        ('expect', 'locks=1'),
    ]),
    # ... HeatMan's Cross is not the one (the setup's fifth number: the Cross a run brings)
    'obstacle-keeps-to-its-cross': dict(START=f'{GEYSER} --setup endless,0,0,0,1', STEPS=ARRIVE + [
        ('do', 'by block; wait 10; press A; wait 40'),
        PAGED,
        ('expect', 'locks=1'),
    ]),
    # ... ElecMan's shorts it out
    'obstacle-cleared-by-its-cross': dict(START=f'{GEYSER} --setup endless,0,0,0,2', STEPS=ARRIVE + [
        ('expect', 'locks=1'),
        ('do', 'by block; wait 10; press A; wait 40'),
        ('repeat', 'press A; wait 30', 'held=none', 30),
        ('expect', 'locks=0'),
    ]),
    # an arrow lane: a step onto it and it carries MegaMan to its far end, no button held
    'lane-carries': dict(START=LANE, STEPS=ARRIVE + [
        ('do', 'by lane; wait 10'),
        ('expect', 'on=floor'),
        ('repeat', 'hold AHEAD 10', 'on=lane', 6),
        ('expect', 'held~conveyor'),
        ('until', 'on=floor', 600),
        ('until', 'held=none', 300),
    ]),
    # an invisible path: the Navi beside it hints at it, it holds under MegaMan, and its pad's Mystery Data is his
    'invisible-path-walked': dict(START=PATH, STEPS=ARRIVE + [
        ('do', 'by hinter; wait 10; press A; wait 40'),
        ('expect', 'chat=open'),
        PAGED,
        ('keep', 'bluetaken'),
        ('do', 'by path; wait 10'),
        ('repeat', 'hold AHEAD 10', 'on=path', 6),
        ('repeat', 'hold AHEAD 20', 'on=floor', 12),
        ('do', 'hold AHEAD 30; press A; wait 40'),
        ('expect', 'chat=open'),
        *[PAGED, ('do', 'wait 40'), PAGED],
        ('moved', 'bluetaken', 1),
    ]),
}
