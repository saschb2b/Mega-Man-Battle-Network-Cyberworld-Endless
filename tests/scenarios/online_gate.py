"""The Net Square's gate in Lan's HP (docs/world/home/square-gate.md):
MegaMan's question, a Yes into the Square of a server of this machine and
home again with the run as it was, a No that stays home, and the visits
that end at the gate, each said there with the run going on: a server
that does not answer, one under maintenance, a full Square."""

import scenarios_online as online

# (the keeper's first questions answered, as a player's network.ini has them: the gate asks "To the Net Square,Lan?")
ASKED = {'network.ini': 'identity={identity}\nname={name}\nnamed=1\nselected=0\nconsent=1\nserver={server}\n',
         'netpass.ini': 'stamps=1\n'}
NOWHERE = {'network.ini': 'identity={identity}\nname={name}\nnamed=1\nselected=0\nconsent=1\nserver={nowhere}\n',
           'netpass.ini': 'stamps=1\n'}
HOME = '--scene home --run-depth 7 --seed 3 --dev quiet'
# (Lan's HP's arrival words paged, then onto the gate: its question)
ARRIVE = [
    ('do', 'wait 300'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('keep', 'hp zenny bugfrags spares place'),
    ('do', 'place 16 72 3; wait 10'),
    ('repeat', 'wait 4', 'choice=2/2', 90),            # "To the Net Square,Lan?", the cursor on No
]

MANY = {
    'yes-and-home': dict(START=HOME, SERVER=True, FILES=ASKED, STEPS=ARRIVE + [
        ('expect', 'visit=home'),
        *online.yes(),
        *online.soon('visit=square', '', 900),
        ('expect', 'screen=square'),
        ('expect', 'part=plaza'),
        ('expect', 'peers=0'),
        ('expect', 'link=up'),
        *online.soon('held=none', '', 600),
        *online.home_again(),                          # R asks to head home: Yes, and "We're home,Lan!"
        ('expect', 'visitended=left'),
        ('expect', 'where=town'),
        ('expect', 'visits=1'),
        ('same', 'hp zenny bugfrags spares place'),
    ]),
    'no-stays-home': dict(START=HOME, SERVER=True, FILES=ASKED, STEPS=ARRIVE + [
        ('repeat', 'press A; wait 10', 'chat=closed'),   # No
        ('do', 'wait 120'),
        ('expect', 'visit=home'),
        ('expect', 'visits=0'),
        ('same', 'hp zenny bugfrags spares place'),
    ]),
    'no-server': dict(START=HOME, FILES=NOWHERE, STEPS=ARRIVE + [
        *online.yes(),
        ('until', 'visitended=unreachable', 1800),
        ('expect', 'visit=home'),
        ('expect', 'where=town'),
        ('until', 'chat=open', 300),                   # the keeper's word on it
        ('repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('same', 'hp zenny bugfrags spares place'),
    ]),
    'maintenance': dict(START=HOME, SERVER=dict(settings=dict(maintenance='until 14:00 UTC')), FILES=ASKED, STEPS=ARRIVE + [
        *online.yes(),
        ('until', 'visitended=maintenance', 1800),
        ('expect', 'visit=home'),
        ('until', 'chat=open', 300),                   # the keeper's word, with the host's line
        ('repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('same', 'hp zenny bugfrags spares place'),
    ]),
    # (a Square of one place: the second player is turned away)
    'square-full': dict(START=online.HOME, SERVER=dict(settings=dict(capacity=1, player_cap=1)), GAMES={'beta': online.HOME},
                        FILES={'netpass.ini': online.SEEN_SQUARE}, STEPS=online.course(
        online.into_square(),
        online.paged('beta'),
        ('@beta', 'keep', 'hp zenny bugfrags spares place'),
        ('@beta', 'do', 'place 16 72 3; wait 10'),
        ('@beta', 'until', 'visitended=full', 1800),
        ('@beta', 'expect', 'visit=home'),
        ('@beta', 'repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('@beta', 'same', 'hp zenny bugfrags spares place'),
        ('expect', 'visit=square'),
        ('expect', 'peers=0'),
    )),
}
