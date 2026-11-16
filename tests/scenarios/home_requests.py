"""The requests (docs/world/home/jobs.md): the visit's three posted on
AsterLand's Request BBS and taken there or from the one who asks, one at a
time; counted in the Net; settled at the next visit by the asker, paid
once (issue #122: a jack-in and out paid the lab's 1000 zenny again) or
not at all where it was not done."""

ROOM = 'room'
FREE = ('until', 'held=none', 600)
PAGED = ('repeat', 'press A; wait 20', 'chat=closed', 40)
# (out of Lan's room and house into the town, and into AsterLand: home_town.py's ways)
OUT = [
    ('do', 'place -62 80 3; wait 10; hold DOWN 24'),               # (the top of the room's stairs, where a run sets Lan down)
    ('until', "place=Lan's House", 600), FREE,
    ('do', 'place -78 55 1; wait 10; hold DOWN 30'),
    ('until', 'place=Central Town', 600), FREE,
]
ASTER = OUT + [
    ('do', 'place -156 -40 7; wait 10; hold UP+LEFT 62'),
    ('until', 'place=AsterLand', 600), FREE,
]
# (... and through the Cyber Academy to class 6-1, where the NetBattle club's member asks)
CLUB = OUT + [step for spot, hold, at in (
    ('92 -232 7', 40, '02:06'), ('24 16 7', 30, '02:04'), ('280 -40 7', 30, '02:05'), ('-208 -40 7', 30, '02:00'),
) for step in (('do', f'place {spot}; wait 10; hold UP+LEFT {hold}'), ('until', f'at={at}', 600), FREE)] + [
    ('do', 'place -148 18 7; wait 10'), ('repeat', 'press A; wait 30', 'chat=open', 4),
]
# (the lab's man walks the east pavement: met at his walk's end, where he turns)
LAB_MAN = [('do', 'place 132 212 7; wait 10'), ('repeat', 'press A; wait 30', 'chat=open', 24)]
# (Lan's HP after an act, its arrival's words paged; R's "MegaMan, jack
# out?" opens on No: Left, A, and Lan stands at his PC)
ARRIVED = [('until', 'chat=open', 900), ('repeat', 'mash A 60', 'chat=closed', 20), FREE]
JACK_OUT = [
    ('do', 'press R; wait 40'),
    ('repeat', 'press A; wait 30', 'choice=2/2'),
    ('do', 'press LEFT; wait 20'),
    ('repeat', 'press A; wait 40', "place=Lan's Room", 20), FREE,
]
# (... and back: his front door from the pavement, the stairs up, R at his PC)
JACK_IN = [
    ('do', 'place -28 216 7; wait 10; hold UP 40'),
    ('until', "place=Lan's House", 600), FREE,
    ('do', 'place 16 -108 7; wait 10; hold UP+LEFT 40'),
    ('until', "place=Lan's Room", 600), FREE,
    ('do', 'place -40 -8 7; wait 10; press R; wait 40'),
    ('until', "place=Lan's HP", 900),
    ('repeat', 'mash A 60', 'held=none', 20),
]


def home(seed, depth, dev):
    return f'--scene home --seed {seed} --run-depth {depth} --dev {dev}'


MANY = {
    'bbs-take-one': dict(
        GUARDS="AsterLand's Request BBS at a run's first visit: a post read and taken is the run's request, held; the board takes no second, and the lab's man in town offers none.",
        START=ROOM,
        STEPS=ASTER + [
            ('expect', 'request=none held 0 settled 0'),
            ('do', 'place -72 -36 7; wait 10'),
            ('repeat', 'press A; wait 50', 'choice=1/2', 20),      # the board, a post read out, "Take this request?"
            ('repeat', 'press A; wait 50', 'request~taken', 6),
            ('expect', 'request~held 1 settled 0'),
            ('keep', 'request'),
            ('do', 'press A; wait 50; press A; wait 50; press A; wait 50; press A; wait 50; press A; wait 50'),   # another post: one at a time
            ('same', 'request'),
            ('repeat', 'press B; wait 50', 'screen=town', 20), FREE,
            ('same', 'request'),
            ('do', 'place -4 96 3; wait 10; hold DOWN+RIGHT 40'),
            ('until', 'place=Central Town', 600), FREE,
        ] + LAB_MAN + [PAGED, FREE, ('same', 'request')],
    ),
    'asker-take': dict(
        GUARDS="The lab's man in town offers his request in person: Yes takes it, and it is the run's, held.",
        START=ROOM,
        STEPS=OUT + LAB_MAN + [
            ('repeat', 'press A; wait 30', 'choice=1/2', 20),
            ('do', 'press A; wait 40'),
            PAGED, FREE,
            ('expect', 'request~taken'),
            ('expect', 'request~held 1 settled 0'),
        ]),
    'asker-decline': dict(
        GUARDS="The lab's man's offer declined: no request is held, and he offers it again.",
        START=ROOM,
        STEPS=OUT + LAB_MAN + [
            ('repeat', 'press A; wait 30', 'choice=1/2', 20),
            ('do', 'press RIGHT; wait 20'),
            ('expect', 'choice=2/2'),
            ('do', 'press A; wait 40'),
            PAGED, FREE,
            ('expect', 'request=none held 0 settled 0'),
        ] + LAB_MAN + [
            ('repeat', 'press A; wait 30', 'choice=1/2', 20),
        ]),
    'club-take': dict(
        GUARDS="The NetBattle club's member in class 6-1 offers her request in person: Yes takes it.",
        START=ROOM,
        STEPS=CLUB + [
            ('repeat', 'press A; wait 30', 'choice=1/2', 20),
            ('do', 'press A; wait 40'),
            PAGED, FREE,
            ('expect', 'request~taken got 0'),
            ('expect', 'request~held 1 settled 0'),
        ]),
    'counted-kept-paid': dict(
        GUARDS="A request's whole course: taken, its battles counted in the Net until it is done, home after the act's guardian, kept through a quit and CONTINUE there, and paid by its asker in class 6-1.",
        # (three wins in ten seconds each, by the autopilot with one-hit enemies; the act's guardian falls the same way)
        START='CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --dev job=2,god,onehit',
        STEPS=[
            ('expect', 'request~quick taken got 0 of 3'),
            ('until', 'request~got 1 of 3', 6000),
            ('until', 'request~quick done got 3 of 3', 12000),
            ('until', 'where=town', 12000),
            ('until', 'chat=open', 900),                                # Lan's HP's arrival words ...
            ('until', 'held=none', 900),                                # ... paged, MegaMan free: the run saved there
            ('keep', 'request'),
            ('restart', '--scene title'),                               # (no autopilot from here: Lan's own errands)
            ('repeat', 'press START; wait 40', 'title=menu'),
            ('expect', 'cursor=continue'),
            ('repeat', 'press A; wait 40', 'scene=emu'),
            ('until', "place=Lan's HP", 1200),
            ('repeat', 'mash A 60', 'held=none', 20),
            ('same', 'request'),
            ('expect', 'bugfrags=0'),
        ] + JACK_OUT + CLUB + [
            PAGED, FREE,
            ('expect', 'bugfrags>0'),
            ('expect', 'request~settled 1'),
        ]),
    'paid-once': dict(
        GUARDS="Issue #122: a finished request is settled by its asker at the next visit, its zenny paid; after a jack-in at Lan's PC and a jack-out he pays nothing more.",
        START=home(7, 10, 'job=3,jobstate=2'),
        STEPS=ARRIVED + [
            ('expect', 'request~explore done'),
            ('expect', 'request~held 1 settled 0'),
            ('expect', 'courier=waiting'),
            ('expect', 'zenny=0'),
        ] + JACK_OUT + OUT + LAB_MAN + [
            PAGED, FREE,
            ('expect', 'zenny=1000'),
            ('expect', 'request~settled 1'),
        ] + JACK_IN + JACK_OUT + OUT + LAB_MAN + [
            PAGED, FREE,
            ('expect', 'zenny=1000'),
        ]),
    'failed-pays-nothing': dict(
        GUARDS="A request not done is settled with a word and no pay.",
        START=home(7, 10, 'job=3,jobstate=3'),
        STEPS=ARRIVED + [
            ('expect', 'request~explore failed'),
        ] + JACK_OUT + OUT + LAB_MAN + [
            PAGED, FREE,
            ('expect', 'zenny=0'),
            ('expect', 'request~settled 1'),
        ]),
}
