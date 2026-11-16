"""What MegaMan carries from battle to battle: the HP a battle left him
and what its RESULT paid are the map's and the next battle's first (no
heal between them), and the tally and the layer's battles add up."""

# (hp=200/150: the run begins hurt, 150 of 200; onehit: short battles the
# autopilot wins)
START = 'CYBERWORLD_AUTOPILOT=1 CYBERWORLD_STATE_POS=1 --scene emu --seed 2 --run-depth 2 --dev onehit,hp=200/150'

STEPS = [
    ('expect', 'hp=150/200'),
    ('until', 'chat=open', 1200),             # the arrival's words, paged by the autopilot,
    ('until', 'held=none', 1200),             # and MegaMan free
    ('do', 'battle'),
    ('until', 'phase=custom', 1200),
    ('expect', 'hp=150/200'),                 # into the battle as he walked
    ('until', 'phase=results', 3000),
    ('expect', 'result=won'),
    ('until', 'held=none', 1200),             # on the map again, its reward paid
    ('expect', 'battles=1'),
    ('keep', 'hp zenny viruses'),
    ('do', 'battle'),
    ('until', 'phase=custom', 1200),
    ('same', 'hp zenny'),                     # ... and into the next as the first left him
    ('expect', 'battles=2'),
    ('until', 'phase=results', 3000),
    ('same', 'viruses'),                      # (counted as he is back on the map)
    ('until', 'held=none', 1200),
    ('expect', 'viruses>2'),
    ('expect', 'lastbattle=won'),
]
