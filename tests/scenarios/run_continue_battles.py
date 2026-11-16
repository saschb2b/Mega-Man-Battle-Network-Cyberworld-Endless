"""0.22.0: a CONTINUE replayed the layer's first battle. A layer's random
battles are rolled by its seed and the count of those fought on it, which
began at nothing again at a CONTINUE, so its first battle came round once
more. The count comes back with the run: one battle fought, a quit on
the map, CONTINUE, and the layer goes on with its second."""

START = 'CYBERWORLD_AUTOPILOT=1 CYBERWORLD_STATE_POS=1 --scene emu --seed 3 --run-depth 2 --dev onehit'

STEPS = [
    ('expect', 'battles=0'),
    ('until', 'chat=open', 1200),             # the arrival's words, paged by the autopilot,
    ('until', 'held=none', 1200),             # and MegaMan free
    ('do', 'battle'),
    ('until', 'phase=results', 4000),
    ('until', 'held=none', 1200),             # on the map again, free: a quit here saves
    ('expect', 'battles=1'),
    ('keep', 'battles viruses zenny layer'),
    ('restart', 'CYBERWORLD_AUTOPILOT=1 CYBERWORLD_STATE_POS=1 --scene title --dev onehit,quiet'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'screen=map', 2400),
    ('same', 'battles viruses zenny layer'),
    ('until', 'held=none', 1200),
    ('do', 'battle'),
    ('until', 'phase=custom', 1200),
    ('expect', 'battles=2'),                  # the layer's next, not its first again
]
