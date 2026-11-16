"""The Virus Battler (docs/world/level-design/virus-battler.md): his BtlrCard
given at a profile's first, his stake taken and refused, BN6's VIRUS LIST
and a round of the player's viruses won and lost, the pot taken after a
win and the whole series won with its first prize, a Rare virus deleted
joining the card, and no Battler with the setup's switch off."""

ARRIVE = [('do', 'wait 150; mash A 300; wait 20'), ('repeat', 'mash A 60; wait 30', 'chat=closed')]
# (his question starts on No: LEFT goes round to Yes)
YES = 'press LEFT; press A; wait 20'
NO = 'press A; wait 20'
# (Battler 1 on act 2's first layer; quiet: no random battles)
BATTLER = '--scene emu --seed 3 --run-depth 4 --dev quiet,battler=1'
# (... the card holding ErthDrgn beside Mettaur and Gunner: its V1 with Mettaur wins his three rounds)
STRONG = BATTLER + ',viruses=0x10000'
# (10000 zenny given, then his offer opened)
RICH = ' --talk zenny:600,battler:800'
ENTER = [
    *ARRIVE,
    ('until', 'zenny=10000', 900),
    ('until', 'chat=open', 900),
    ('repeat', YES, 'series=on', 40),             # the stake paid: BATTLE 1 START!
    ('expect', 'zenny=9500'),
    ('expect', 'pot=500'),
]
# (BN6's VIRUS LIST: Mettaur, its first entry, picked; then ErthDrgn, its
# fourteenth; OK; each placed on a panel of its own; "Fight with this set?" Yes)
DOWN_13 = '; '.join(['press DOWN 3; wait 8'] * 13)
PAIR = [
    ('until', 'vbscreen=list', 1200),
    ('repeat', 'press A; wait 30', 'vbpicked=1'),
    ('do', DOWN_13),
    ('repeat', 'press A; wait 30', 'vbpicked=2'),
    ('repeat', 'press A; wait 40', 'vbscreen=placing'),
    ('repeat', 'press A; wait 40', 'vbplaced=1'),
    ('do', 'press UP; wait 10'),
    ('repeat', 'press A; wait 40', 'vbplaced=2'),
    ('repeat', 'press A; wait 40', 'vbscreen=fight', 20),
    ('until', 'doing=map', 3000),                 # the viruses fight on their own
]
# (... Mettaur alone: RIGHT to OK)
ALONE = [
    ('until', 'vbscreen=list', 1200),
    ('repeat', 'press A; wait 30', 'vbpicked=1'),
    ('do', 'press RIGHT; wait 10'),
    ('repeat', 'press A; wait 40', 'vbscreen=placing'),
    ('repeat', 'press A; wait 40', 'vbplaced=1'),
    ('repeat', 'press A; wait 40', 'vbscreen=fight', 20),
    ('until', 'doing=map', 3000),
]
# (his word after a win, paged to TAKE THE POT / FIGHT ON)
CHOICE = [('repeat', 'press A; wait 30', 'potchoice=up', 30), ('do', 'wait 20')]
# (the dev menu's "Win this battle", its fifth row)
WIN = 'hold SELECT 4; hold SELECT+R 4; wait 6; press DOWN; press DOWN; press DOWN; press DOWN; press A; wait 60'

MANY = {
    'card-given': dict(
        GUARDS="A profile's first Virus Battler gives the BtlrCard, Mettaur and Gunner on it; his No costs nothing; the "
               'profile keeps the card over a quit.',
        START=BATTLER + RICH,
        STEPS=[
            *ARRIVE,
            ('expect', 'battler=1'),
            ('expect', 'btlrcard=none'),
            ('expect', 'stake=500'),
            ('expect', 'vbrounds=3'),
            ('until', 'chat=open', 1200),
            ('repeat', NO, 'chat=closed', 60),
            ('do', 'wait 60'),
            ('expect', 'btlrcard=held'),
            ('expect', 'virusdata=2'),
            ('expect', 'series=none'),
            ('expect', 'zenny=10000'),
            ('restart', '--scene title'),
            ('repeat', 'press START; wait 40', 'title=menu'),
            ('repeat', 'press A; wait 40', 'scene=emu'),
            ('until', 'screen=map', 2400),
            ('expect', 'btlrcard=held'),
            ('expect', 'virusdata=2'),
        ]),
    'too-little-zenny': dict(
        GUARDS="His Yes without his stake's zenny: no series, nothing taken.",
        START=STRONG + ' --talk battler:800',
        STEPS=[
            *ARRIVE,
            ('expect', 'zenny=0'),
            ('until', 'chat=open', 1200),
            ('repeat', YES, 'chat=closed', 60),
            ('do', 'wait 60'),
            ('expect', 'series=none'),
            ('expect', 'zenny=0'),
            ('expect', 'screen=map'),
        ]),
    'pot-taken': dict(
        GUARDS="A round won with the player's viruses, MegaMan's HP untouched, and the pot taken: the stake back and a "
               'stake more, his series over for the visit, his first prize still open.',
        START=STRONG + RICH + ',battler:5000',
        STEPS=[
            *ENTER,
            ('expect', 'vbround=1'),
            *PAIR,
            ('expect', 'vblast=won'),
            ('expect', 'vbwins=1'),
            ('expect', 'pot=1000'),
            ('expect', 'hp=100/100'),
            *CHOICE,
            ('repeat', 'press A; wait 30', 'series=over', 20),      # TAKE THE POT, the cursor's own
            ('repeat', 'press A; wait 30', 'chat=closed', 20),
            ('expect', 'zenny=10500'),
            ('expect', 'vbprize=open'),
            ('until', 'chat=open', 6000),                 # talked to again: his series is over
            ('repeat', 'press A; wait 30', 'chat=closed', 20),
            ('expect', 'series=over'),
            ('expect', 'zenny=10500'),
        ]),
    'series-won': dict(
        GUARDS="FIGHT ON after each win to his series' end: the pot a stake a round and the stake, paid whole, and his "
               'first prize in a run.',
        START=STRONG + RICH,
        STEPS=[
            *ENTER,
            *PAIR,
            ('expect', 'pot=1000'),
            *CHOICE,
            ('repeat', 'press DOWN; wait 10; press A; wait 30', 'vbround=2', 20),   # FIGHT ON
            ('expect', 'zenny=9500'),
            *PAIR,
            ('expect', 'vbwins=2'),
            ('expect', 'pot=1500'),
            *CHOICE,
            ('repeat', 'press DOWN; wait 10; press A; wait 30', 'vbround=3', 20),
            *PAIR,
            ('expect', 'vblast=won'),
            ('repeat', 'press A; wait 30', 'series=over', 40),      # his whole series: the pot, the prize
            ('repeat', 'press A; wait 30', 'chat=closed', 20),
            ('expect', 'zenny=11500'),
            ('expect', 'vbprize=taken'),
            ('expect', 'hp=100/100'),
        ]),
    'round-lost': dict(
        GUARDS="A round lost with Mettaur alone: the pot his, the stake gone, no Game Over and MegaMan's HP untouched.",
        START=BATTLER + ',viruses=0x2' + RICH,
        STEPS=[
            *ENTER,
            *ALONE,
            ('expect', 'vblast!=won'),
            ('expect', 'scene=emu'),
            ('repeat', 'press A; wait 30', 'series=over', 40),
            ('repeat', 'press A; wait 30', 'chat=closed', 20),
            ('expect', 'zenny=9500'),
            ('expect', 'hp=100/100'),
            ('expect', 'vbprize=open'),
        ]),
    'rare-joins-the-card': dict(
        GUARDS="With the card held, a Rare virus deleted for the first time puts its family on the card, and the profile "
               'keeps it.',
        START='--scene emu --seed 3 --run-depth 4 --dev quiet,viruses=0x2',
        STEPS=[
            *ARRIVE,
            ('expect', 'btlrcard=held'),
            ('expect', 'virusdata=2'),
            ('do', 'foes B@52'),                          # the second family's first Rare
            ('until', 'turn>=1', 1200),
            ('do', 'wait 90; press START; wait 10; press A'),
            ('until', 'custom>0', 600),
            ('do', WIN),
            ('repeat', 'press A; wait 40', 'doing=map', 40),
            ('expect', 'virusdata=3'),
        ]),
    'off': dict(
        GUARDS="The setup's Virus Battler switched off: none on the layer, where the dev switch would put one on every layer.",
        START=BATTLER + ' --setup endless,0,0,0,0,8',
        STEPS=[
            *ARRIVE,
            ('expect', 'layer=4'),
            ('expect', 'battler=none'),
        ]),
}
