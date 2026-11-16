"""Central Town in a run (docs/world/home/): out of Lan's room and house
and back in, its people spoken to, R away from the PC, AsterLand's counter
(an order bought once a visit, refused without the zenny), its SubChip
seller, its Chip Trader and the Number Trader's sign, the Cyber Academy's
way to class 6-1 and back."""

# (a new run in Lan's room, Dad's call paged; the spots are BN6's maps' own,
# the same every run: build.py's HOME_OUT, HOME_ASTER and HOME_ACADEMY walk
# them. Not with CYBERWORLD_STATE_POS: in the town its state works out the
# arrow's way each time, a quarter of a second a step)
ROOM = 'room'
FREE = ('until', 'held=none', 600)
# (down the room's stairs, across the house and out of its front door)
OUT = [
    ('do', 'place -62 80 3; wait 10; hold DOWN 24'),               # (the top of the room's stairs, where a run sets Lan down)
    ('until', "place=Lan's House", 600), FREE,
    ('do', 'place -78 55 1; wait 10; hold DOWN 30'),
    ('until', 'place=Central Town', 600), FREE,
]
# (... and through AsterLand's door)
ASTER = OUT + [
    ('do', 'place -156 -40 7; wait 10; hold UP+LEFT 62'),
    ('until', 'place=AsterLand', 600), FREE,
]
# (... or through the Cyber Academy's gate: its foyer, the 1F hallway, the
# stairs to the 2F hallway and the door into class 6-1; BN6's map each, its
# group and number the state's `at`)
ACADEMY = OUT + [step for spot, hold, at in (
    ('92 -232 7', 40, '02:06'), ('24 16 7', 30, '02:04'), ('280 -40 7', 30, '02:05'), ('-208 -40 7', 30, '02:00'),
) for step in (('do', f'place {spot}; wait 10; hold UP+LEFT {hold}'), ('until', f'at={at}', 600), FREE)]
# (the town's people as a run's start stands them, town_folk.c: each from
# the cell below; its two walkers, the lab's man down the east pavement and
# the one along the north road, met at their walks' ends, where each turns)
FOLK = ((108, -132), (100, -116), (60, -160), (-188, -60), (-108, -100), (-164, 132))
WALKERS = ('132 212 7', '84 -228 5')
PAGED = ('repeat', 'press A; wait 20', 'chat=closed', 40)


def talk(x, y):
    """Lan set below (x, y) facing it, A, and the talk paged to its end."""
    return [('do', f'place {x} {y + 16} 7; wait 10'), ('repeat', 'press A; wait 30', 'chat=open', 4), PAGED, FREE]


def meet(spot):
    """Lan set at a walker's turn facing up his walk, A until he is there to answer, and the talk paged."""
    return [('do', f'place {spot}; wait 10'), ('repeat', 'press A; wait 30', 'chat=open', 24), PAGED, FREE]


def dev(switches):
    return f'{ROOM} --dev {switches}'


MANY = {
    'out-and-back': dict(
        GUARDS="Lan's room, its stairs, his house, the front door and the town, and the same way back up to his room: each door leads where it stands.",
        START=ROOM,
        STEPS=OUT + [
            ('expect', 'hour=morning'),
            ('do', 'place -28 216 7; wait 10; hold UP 40'),              # his front door, from the pavement
            ('until', "place=Lan's House", 600), FREE,
            ('do', 'place 16 -108 7; wait 10; hold UP+LEFT 40'),        # the stairs up
            ('until', "place=Lan's Room", 600), FREE,
        ]),
    'people': dict(
        GUARDS="Central Town's eight people at a run's start, its two walkers among them: each answers A with a talk that pages to its end and leaves Lan free.",
        START=ROOM,
        STEPS=OUT + [step for x, y in FOLK for step in talk(x, y)] + [step for spot in WALKERS for step in meet(spot)] +
        [('expect', 'place=Central Town')],
    ),
    'r-away-from-the-pc': dict(
        GUARDS="R in the town, on its statue's and its other jack-in point and in the street: MegaMan says where the PC is, and Lan jacks in nowhere.",
        START=ROOM,
        STEPS=OUT + [step for spot in ('96 -16', '-124 -12', '0 100') for step in (
            ('do', f'place {spot} 7; wait 10; press R; wait 40'),
            ('expect', 'talk=director'),
            PAGED, FREE,
            ('expect', 'place=Central Town'),
        )],
    ),
    'l-says-the-way': dict(
        GUARDS="L in the town: MegaMan's word on the way, paged, Lan free.",
        START=ROOM,
        STEPS=OUT + [
            ('do', 'press L; wait 40'),
            ('expect', 'talk=director'),
            PAGED, FREE,
        ]),
    'aster-order': dict(
        GUARDS="AsterLand's Order Service: an order costs its price and puts its chip in the pack, the visit's order is made, and the clerk takes no second one.",
        START=dev('zenny=20000'),
        STEPS=ASTER + [
            ('expect', 'order=open'),
            ('keep', 'chips'),
            ('do', 'place 23 -40 7; wait 10'),
            ('repeat', 'press A; wait 40', 'screen=shop'),
            ('repeat', 'press A; wait 40', 'choice=1/2'),           # "You'd like to buy ...?", on Yes
            ('repeat', 'press A; wait 40', 'order=made', 20),
            ('repeat', 'press B; wait 40', 'screen=town', 20),
            PAGED, FREE,
            ('expect', 'zenny=19000'),
            ('expect', 'chips=31'),
            ('do', 'press A; wait 40'),                                 # the clerk again: his word, no list
            ('expect', 'chat=open'),
            PAGED, FREE,
            ('expect', 'screen=town'),
            ('expect', 'zenny=19000'),
        ]),
    'aster-order-no-zenny': dict(
        GUARDS="AsterLand's Order Service with no zenny: nothing is ordered, the pack and the visit's order as they were.",
        START=ROOM,
        STEPS=ASTER + [
            ('expect', 'zenny=0'),
            ('keep', 'chips order zenny'),
            ('do', 'place 23 -40 7; wait 10'),
            ('repeat', 'press A; wait 40', 'choice=1/2', 20),       # "You'd like to buy ...?", on Yes
            ('expect', 'screen=shop'),
            ('do', 'press A; wait 40'),                                 # Yes: not enough zenny
            ('repeat', 'press B; wait 40', 'screen=town', 20),
            FREE,
            ('same', 'chips order zenny'),
        ]),
    'aster-subchip': dict(
        GUARDS="AsterLand's SubChip seller: an Unlocker and a MiniEnrg bought are held and paid for.",
        START=dev('zenny=1000'),
        STEPS=ASTER + [
            ('expect', 'subchips~minienrg 0'),
            ('do', 'place -4 -36 7; wait 10'),
            ('repeat', 'press A; wait 40', 'screen=shop'),
            ('repeat', 'press A; wait 40', 'unlockers=1', 20),  # the list's first: an Unlocker, 600 zenny
            ('do', 'press A; wait 40; press DOWN; wait 10; press DOWN; wait 10'),   # "I bought ..." paged; MiniEnrg, 100
            ('repeat', 'press A; wait 40', 'subchips~minienrg 1', 20),
            ('do', 'press A; wait 40'),
            ('repeat', 'press B; wait 40', 'choice=1/2'),           # "Want to leave?", on Yes
            ('do', 'press A; wait 40'),
            ('until', 'screen=town', 600), FREE,
            ('expect', 'zenny=300'),
            ('expect', 'subchips=minienrg 1 fullenrg 0 sneakrun 0 untrap 0'),
            ('expect', 'unlockers=1'),
        ]),
    'aster-subchip-no-zenny': dict(
        GUARDS="AsterLand's SubChip seller with no zenny: nothing is sold.",
        START=ROOM,
        STEPS=ASTER + [
            ('keep', 'subchips unlockers zenny'),
            ('do', 'place -4 -36 7; wait 10'),
            ('repeat', 'press A; wait 40', 'screen=shop'),
            ('repeat', 'press A; wait 40', 'choice=1/2'),           # "Unlocker, right?", on Yes
            ('do', 'press A; wait 40; press A; wait 40'),               # Yes: not enough zenny
            ('repeat', 'press B; wait 40', 'choice=1/2'),           # "Want to leave?"
            ('do', 'press A; wait 40'),
            ('until', 'screen=town', 600), FREE,
            ('same', 'subchips unlockers zenny'),
        ]),
    'aster-chip-trader': dict(
        GUARDS="AsterLand's Chip Trader: three chips of the pack in, one out, and it asks to go again; No leaves, the pack two chips fewer.",
        START=dev('pack=12'),
        STEPS=ASTER + [
            ('expect', 'chips=42'),                                      # (the folder's thirty and twelve spare)
            ('do', 'place 94 46 7; wait 10'),
            ('repeat', 'press A; wait 40', 'choice=1/2'),           # "Insert 3 BtlChips?", on Yes
            ('repeat', 'press A; wait 40', 'screen=trader'),
            ('do', 'wait 30; press A; wait 30; press A; wait 30; press A; wait 30'),
            ('repeat', 'press A; wait 40', 'choice=1/2', 4),        # "Add these 3?"
            ('do', 'press A; wait 90'),
            ('repeat', 'press A; wait 40', 'choice=1/2', 20),       # the chip it gave, then "Try again?"
            ('do', 'press RIGHT; wait 20'),
            ('expect', 'choice=2/2'),
            ('do', 'press A; wait 60'),
            PAGED, FREE,
            ('expect', 'screen=town'),
            ('expect', 'chips=40'),
        ]),
    'aster-number-trader-off': dict(
        GUARDS="AsterLand's Number Trader is off: its check reads a sign, and no screen of BN6's opens.",
        START=ROOM,
        STEPS=ASTER + [
            ('do', 'place 116 44 7; wait 10; press A; wait 40'),
            ('expect', 'chat=open'),
            ('expect', 'screen=town'),
            PAGED, FREE,
            ('expect', 'screen=town'),
        ]),
    'academy-class-6-1': dict(
        GUARDS="The Cyber Academy on a club day: the gate, the foyer, both hallways and class 6-1, its two club members spoken to, and each way back out to the town; R inside has MegaMan point the way out.",
        START=ROOM,
        STEPS=ACADEMY + [
            ('expect', 'place=Cyber Academy'),
            ('do', 'place -130 14 7; wait 10; press A; wait 30'),      # the club's other member: a tip
            ('expect', 'chat=open'),
            PAGED, FREE,
            ('do', 'press R; wait 40'),
            ('expect', 'talk=director'),
            PAGED, FREE,
            ('expect', 'at=02:00'),
        ] + [step for spot, at in (('-132 144 3', '02:05'), ('-252 152 3', '02:04'), ('-252 152 3', '02:06'), ('-60 312 3', '01:00'))
             for step in (('do', f'place {spot}; wait 10; hold DOWN+RIGHT 30'), ('until', f'at={at}', 600), FREE)] + [
            ('expect', 'place=Central Town'),
        ]),
    'after-an-act': dict(
        GUARDS="The town after act 2: R's jack-out from Lan's HP sets Lan at his PC, the afternoon's town outside, its plaza's Mr.Prog and the lab's man answer, and AsterLand's order of the new visit is open.",
        START='--scene home --run-depth 7 --seed 3',
        STEPS=[
            ('until', 'chat=open', 900), ('repeat', 'mash A 60', 'chat=closed', 20), FREE,
            ('do', 'press R; wait 40'),
            ('repeat', 'press A; wait 30', 'choice=2/2'),           # BN6's "MegaMan, jack out?" opens on No
            ('do', 'press LEFT; wait 20'),
            ('repeat', 'press A; wait 40', "place=Lan's Room", 20), FREE,
        ] + OUT + [
            ('expect', 'hour=afternoon'),
            ('expect', 'order=open'),
        ] + meet(WALKERS[0]) + meet(WALKERS[1]) + [
            ('expect', 'place=Central Town'),
        ]),
    'aster-out': dict(
        GUARDS="AsterLand's door leads out to the town before it, and R inside has MegaMan point the way out.",
        START=ROOM,
        STEPS=ASTER + [
            ('do', 'press R; wait 40'),
            ('expect', 'talk=director'),
            PAGED, FREE,
            ('expect', 'place=AsterLand'),
            ('do', 'place -4 96 3; wait 10; hold DOWN+RIGHT 40'),
            ('until', 'place=Central Town', 600), FREE,
        ]),
}
