"""Issue #669: MegaMan deleted in a liberation's battle, the next run's
battles ran by a liberation's rules (three turns, the Custom screen by
itself, TIME UP!). A liberation's rules end with its battle."""

# (a dark layer, its board played by the autopilot; fragile: the first hit
# deletes MegaMan, in the board's first battle)
START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 5 --run-depth 5 --dev dark,fragile'

STEPS = [
    ('until', 'rules=liberation', 4000),      # the board's first battle, a liberation's
    ('until', 'scene=title', 8000),           # deleted in it: the run over
    ('expect', 'title=summary'),
    ('repeat', 'press A; wait 40', 'title=start'),              # the summary's two pages
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=new'),
    ('repeat', 'press A; wait 40', 'title=setup|title=leaving|scene=emu'),
    ('repeat', 'press START; wait 60', 'scene=emu'),            # the setup's START: jack in
    ('until', 'screen=battle', 20000),        # the next run, from Lan's room to its first battle
    ('expect', 'rules=plain'),
]
