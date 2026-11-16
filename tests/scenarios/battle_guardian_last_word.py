"""0.22.0: the A that paged a guardian's last word took his Guardian Data
the moment it appeared, from a panel away, before it was seen. A pressed
in step through his last word and on reaches nothing for the data's first
second; pressed after it, it takes the data: its boxes, the program
draft's first offer, the exit open."""

# (the autopilot to his arena, where the run is saved at the door; then a
# CONTINUE without it, the buttons this course's own. onehit: any buster
# shot ends his fight; god: he cannot delete MegaMan meanwhile)
START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --guardian 1 --dev onehit,quiet'

# (an A every ten frames, as a player pages a box)
A = 'press A 4'

STEPS = [
    ('until', 'battle=guardian', 4000),
    ('restart', '--scene title --guardian 1 --dev onehit,god,quiet'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'chat=open', 2400),             # at the door again: his card, his words
    ('repeat', 'press A; wait 30', 'phase=custom', 40),
    ('repeat', 'press START; wait 10; press A; wait 60', 'phase=fight'),
    ('repeat', 'mash B 40; press UP; mash B 40; press DOWN; mash B 40; press DOWN; mash B 40; press UP', 'phase=results', 30),
    ('repeat', 'press A; wait 20', 'screen=map', 40),         # its RESULT paged
    ('repeat', 'wait 10', 'guardian=HeatMan beaten', 60),
    ('repeat', A, 'data=fresh', 60),          # A in step through his last word, until his data shows
    ('expect', 'chat=closed'),
    ('do', '; '.join([A] * 4)),               # ... and on through its first second
    ('expect', 'data=fresh'),
    ('expect', 'chat=closed'),                # no A has reached it
    ('repeat', A, 'chat=open', 30),           # seen for a second: A takes it
    ('expect', 'data=waiting'),
    ('repeat', 'press A; wait 30', 'screen=reward-pick', 40),
    ('expect', 'pick=draft'),
    ('expect', 'hp=200/200'),
    ('repeat', 'wait 30', 'pickready=yes'),
    ('repeat', 'press A; wait 30', 'data=taken', 40),
    ('until', 'exitpad=open', 120),
]
