"""A Bring request's chip chosen by the player (issue 842,
docs/world/home/jobs.md, The chip given): with two kinds of the request's
element in the Pack, the NetBattler at AsterLand opens BN6's own Chip
Trader screen for one over those alone; the second put in and Yes, he
takes that one and pays, the first still in the Pack. Left by B, nothing
is taken and the request stays held. Seed 7's request at layer 10 asks a
Wood chip; --dev pack=38 puts RiskyHoney1 B (37) and RiskyHoney2 C (38)
in the Pack, one of each."""

START = '--scene home --seed 7 --run-depth 10 --dev job=4,jobstate=2,pack=38'
FREE = ('until', 'held=none', 600)
BOTH = 'bringpack=37:B:1 38:C:1'
# (Lan's HP's arrival paged, jacked out, through the house and the town
# into AsterLand: home_requests.py's ways)
TO_ASTER = [
    ('until', 'chat=open', 900), ('repeat', 'mash A 60', 'chat=closed', 20), FREE,
    ('do', 'press R; wait 40'),
    ('repeat', 'press A; wait 30', 'choice=2/2'),
    ('do', 'press LEFT; wait 20'),
    ('repeat', 'press A; wait 40', "place=Lan's Room", 20), FREE,
    ('do', 'place -62 80 3; wait 10; hold DOWN 24'),
    ('until', "place=Lan's House", 600), FREE,
    ('do', 'place -78 55 1; wait 10; hold DOWN 30'),
    ('until', 'place=Central Town', 600), FREE,
    ('do', 'place -156 -40 7; wait 10; hold UP+LEFT 62'),
    ('until', 'place=AsterLand', 600), FREE,
    ('expect', 'request~bring done'),
    ('expect', BOTH),
]
# (the NetBattler by the board: his two boxes paged, and BN6's Chip
# Trader screen open, its list taking keys)
TO_SCREEN = TO_ASTER + [
    ('do', 'place -92 -24 7; wait 10; press A; wait 40'),
    ('repeat', 'press A; wait 40', 'doing=menu', 6),
    ('do', 'wait 60'),
]

MANY = {
    'bring-choice-pick': dict(
        GUARDS="Two kinds of the asked element in the Pack: the second chosen on BN6's Chip Trader screen is the one handed over, the first stays, and the request is paid.",
        START=START,
        STEPS=TO_SCREEN + [
            ('do', 'press DOWN; wait 10; press A; wait 60'),      # the second put in: MegaMan's question
            ('until', 'choice=1/2', 120),
            ('repeat', 'press A 10; wait 60', 'doing=map', 4),    # Yes
            ('until', 'chat=open', 600),                          # "Lan handed over:" ...
            ('repeat', 'press A; wait 40', 'chat=closed', 30), FREE,
            ('expect', 'request~settled 1'),
            ('expect', 'bringpack=37:B:1'),
        ]),
    'bring-choice-keep': dict(
        GUARDS="B out of the Chip Trader screen takes nothing: the request stays held, both chips in the Pack.",
        START=START,
        STEPS=TO_SCREEN + [
            ('repeat', 'press B; wait 60', 'doing=map', 4),
            ('until', 'chat=open', 600),                          # "I'll wait,then!"
            ('repeat', 'press A; wait 40', 'chat=closed', 10), FREE,
            ('expect', 'request~bring done'),
            ('expect', 'request~held 1 settled 0'),
            ('expect', BOTH),
        ]),
    'bring-one-kind': dict(
        GUARDS="One kind of the asked element in the Pack goes as before, with no screen: handed over and paid.",
        START=START.replace('pack=38', 'pack=37'),
        STEPS=[step if step != ('expect', BOTH) else ('expect', 'bringpack=37:B:1') for step in TO_ASTER] + [
            ('do', 'place -92 -24 7; wait 10; press A; wait 40'),
            ('repeat', 'press A; wait 40', 'chat=closed', 30), FREE,
            ('expect', 'doing=map'),
            ('expect', 'request~settled 1'),
            ('expect', 'bringpack=none'),
        ]),
}
