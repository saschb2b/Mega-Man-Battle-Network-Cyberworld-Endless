"""An act's end and the way on: its guardian's Guardian Data taken, the
exit pad home to Lan's HP, and from its portal the next act's first
layer, its card naming the act, with what the run holds carried through
(HP, zenny, the Navi deleted) and no guardian's data owed there."""

START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --guardian 1 --dev onehit,quiet'

STEPS = [
    ('keep', 'area'),
    ('until', 'exitpad=open', 9000),          # act 1's guardian, his data taken
    ('keep', 'hp zenny navis'),
    ('until', 'where=town', 1200),            # his pad: home
    ('until', "place=Lan's HP", 600),
    ('same', 'hp zenny'),
    ('until', 'where=layer', 3000),           # a portal: the next act
    ('expect', 'layer=4'),
    ('repeat', 'wait 10', 'card=area', 60),
    ('expect', 'cardsays~Act 2'),
    ('same', 'hp zenny navis'),
    ('expect', 'battles=0'),
    ('expect', 'lastbattle=won'),
]
