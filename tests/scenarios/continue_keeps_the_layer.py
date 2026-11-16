"""A quit on a layer, then CONTINUE: the same layer, HP and zenny (0.22.0's
six fixes of a run that came back changed were each found by hand)."""

START = '--scene emu --seed 3 --run-depth 2 --dev quiet'

STEPS = [
    ('do', 'wait 150; mash A 300; wait 20'),                    # the arrival's words paged
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('keep', 'layer area hp zenny bugfrags'),
    ('restart', '--scene title'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'screen=map', 1200),
    ('same', 'layer area hp zenny bugfrags'),
]
