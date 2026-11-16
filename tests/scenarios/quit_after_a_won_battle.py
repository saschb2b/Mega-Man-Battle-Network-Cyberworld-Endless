"""A quit as a won battle's RESULT pays, then CONTINUE: the battle stays won,
its zenny and the layer kept (0.22.0: BN6 holds MegaMan from the RESULT to
the map, the quit's save waited for him free, and the run went back to its
last checkpoint)."""

START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 2'

STEPS = [
    ('until', 'screen=battle', 8000),
    ('until', 'zenny>0', 8000),               # its reward paid: the RESULT is up
    ('keep', 'layer area zenny'),
    ('restart', '--scene title'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'screen=map', 2400),
    ('same', 'layer area zenny'),
]
