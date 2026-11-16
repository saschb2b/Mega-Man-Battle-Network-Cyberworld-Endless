"""NEW GAME from the title: Lan's room, the jack-in from his PC and the run's
first layer, by the autopilot (the town's row's run through the jack-in)."""

START = 'CYBERWORLD_AUTOPILOT=1 --scene title'

STEPS = [
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=new'),
    ('repeat', 'press A; wait 40', 'title=setup|title=leaving|scene=emu'),
    ('repeat', 'press START; wait 60', 'scene=emu'),
    ('expect', 'where=town'),
    ('until', 'where=layer', 20000),
    ('expect', 'layer=1'),
]
