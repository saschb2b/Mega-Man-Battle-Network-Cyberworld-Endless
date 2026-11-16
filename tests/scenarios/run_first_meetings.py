"""0.22.0: first meetings went by the wrong thing. Chaud's first call went
by how deep a profile had gone, so a first run dropped in deep and
deleted before any call made his first call ever "it's Chaud again". The
profile remembers the call as it plays: new until heard, on whatever
layer the last run ended, and known from then on, in the next run too."""

# (act 2's first layer, which has no duel and no call; fragile: the first
# hit of three Mettaurs deletes MegaMan)
START = '--scene emu --seed 1 --run-depth 4 --dev fragile'

ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
]

STEPS = [
    ('expect', 'chaud=new'),
    ('expect', 'duel=none'),
    *ARRIVE,
    ('expect', 'chaud=new'),                  # nothing of his heard on this layer
    ('do', 'foes 01@52 01@41 01@63'),
    ('until', 'phase=custom', 1200),
    ('repeat', 'press START; wait 10; press A; wait 60', 'phase=fight'),
    ('until', 'doing=gameover|scene=title', 3000),
    ('repeat', 'press A; wait 60', 'scene=title', 30),
    ('expect', 'reached=4'),
    # the next run, on a duel's layer: his call is his first
    ('restart', '--scene emu --seed 1 --run-depth 2 --dev quiet'),
    ('expect', 'duel=waiting'),
    ('expect', 'chaud=new'),
    *ARRIVE,                                  # the arrival's words, then his call, paged
    ('repeat', 'mash A 60; wait 60', 'chaud=known', 20),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    # ... and the run after hears a later one
    ('restart', '--scene emu --seed 2 --run-depth 2 --dev quiet'),
    ('expect', 'chaud=known'),
]
