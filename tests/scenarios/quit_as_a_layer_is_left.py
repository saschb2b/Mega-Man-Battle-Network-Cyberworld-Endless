"""A quit as a layer's exit is taken, then CONTINUE: the next layer, and
what the layer left had earned (the fuzz of quits' first find: from the
exit's warp to the arrival's checkpoint a quit kept the layer before from
its last checkpoint, its battles and their zenny lost)."""

START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 2'

STEPS = [
    ('until', 'layer>=3', 20000),             # the exit taken: its warp under way
    ('expect', 'saves=1'),                    # (layer 2's checkpoint alone: layer 3's not saved yet)
    ('expect', 'zenny>0'),                    # (a battle of layer 2's paid)
    ('keep', 'layer area zenny'),
    ('restart', '--scene title'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'screen=map', 2400),
    ('same', 'layer area zenny'),
]
