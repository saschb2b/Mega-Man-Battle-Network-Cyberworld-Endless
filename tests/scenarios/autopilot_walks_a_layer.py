"""A run's first layer walked by the autopilot to the next: its battles
fought, its exit found and taken (the Engine code row's autopilot run)."""

START = 'CYBERWORLD_AUTOPILOT=1 layer --seed 3'

STEPS = [
    ('expect', 'layer=1'),
    ('until', 'layer>=2', 20000),
    ('until', 'screen=map', 1200),
]
