"""The jack-in at a run's start (docs/world/home/central-town.md,
lans-hp.md): R at Lan's PC, on its cells and a step short of them, takes
MegaMan to Lan's HP, and R away from it has him say where the PC is; in
Lan's HP the pink pad leads to the first layer, walked onto from the
corridor, from its left rim and stood on its pink middle (BN6's trigger
stops short of the pad's art, sessions 74 and 75)."""

ROOM = 'room'
FREE = ('until', 'held=none', 600)
PAGED = ('repeat', 'press A; wait 20', 'chat=closed', 40)
# (Lan set on his PC's cells and R: BN6's jack-in, Lan's HP's arrival words paged)
JACK_IN = [
    ('do', 'place -40 -8 7; wait 10; press R; wait 40'),
    ('until', "place=Lan's HP", 900),
    ('repeat', 'mash A 60', 'held=none', 20),
]
ARRIVED = [
    ('until', 'where=layer', 900),
    ('expect', 'layer=1'),
    ('until', 'chat=open', 900),                    # the act's card, then MegaMan's words
    ('repeat', 'mash A 60', 'held=none', 20),
    ('expect', 'layer=1'),
]


def pad(steps, guards):
    return dict(GUARDS=guards, START=ROOM, STEPS=JACK_IN + [('expect', 'portals=on 1 back 0'), ('do', steps)] + ARRIVED)


MANY = {
    'pc-on-its-cells': dict(
        GUARDS="R on the PC's cells in Lan's room: BN6's jack-in to Lan's HP, MegaMan on its blue pad, one portal lit, the first layer ahead.",
        START=ROOM,
        STEPS=JACK_IN + [
            ('expect', 'portals=on 1 back 0'),
            ('expect', 'ahead=layer 1'),
            ('expect', 'hp=100/100'),
        ]),
    'megaman-not-in-the-pet': dict(
        GUARDS="Issue #682: a new run whose jack-in answered \"MegaMan isn't in the PET...\". BN6's flag for it cleared with Lan free in his room is put "
               "back, and R at the PC jacks in; in Lan's HP it is clear, as BN6's jack-in leaves it, and its jack-out sets it.",
        START=ROOM,
        STEPS=[
            ('expect', 'megaman=in'),
            ('do', 'flags 0x171D 0x171D 0; wait 30'),
            ('expect', 'megaman=in'),
        ] + JACK_IN + [
            ('expect', "place=Lan's HP"),
            ('expect', 'megaman=out'),
            ('do', 'press R; wait 40'),
            ('repeat', 'press A; wait 30', 'choice=2/2'),           # BN6's "MegaMan, jack out?" opens on No
            ('do', 'press LEFT; wait 20'),
            ('repeat', 'press A; wait 40', "place=Lan's Room", 20), FREE,
            ('expect', 'megaman=in'),
        ]),
    'pc-a-step-short': dict(
        GUARDS="R a step in front of the PC's cells (two playtesters at the desk heard \"The PC's straight up,Lan!\"): Lan steps onto them and jacks in.",
        START=ROOM,
        STEPS=[
            ('do', 'place -40 10 7; wait 10; press R; wait 40'),
            ('until', "place=Lan's HP", 900),
        ]),
    'pc-away': dict(
        GUARDS="R in Lan's room away from the PC: MegaMan says where it is, and Lan stays in his room, free.",
        START=ROOM,
        STEPS=[
            ('do', 'place -40 20 7; wait 10; press R; wait 40'),
            ('expect', 'talk=director'),
            PAGED, FREE,
            ('do', 'wait 300'),
            ('expect', "place=Lan's Room"),
        ]),
    'pad-from-the-corridor': pad('place 171 0 1; wait 10; hold UP+RIGHT 25',
                                 "Lan's HP's pink pad walked onto from the corridor: the run's first layer."),
    'pad-from-its-left-rim': pad('place 171 -5 1; wait 10; hold RIGHT 12',
                                 "Lan's HP's pink pad walked onto from its left rim, where BN6's trigger does not reach (session 75): the first layer."),
    'pad-on-its-middle': pad('place 194 0 1; wait 10',
                             "MegaMan standing on the pink pad's middle, outside BN6's trigger (session 74, four runs without a warp): the first layer."),
}
