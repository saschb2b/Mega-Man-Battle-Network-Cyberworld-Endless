"""Issue #697: with SlipRunr on the NaviCust, a slide into the guardian's
arena carried MegaMan past the staging, which a slide does not hear, onto
the guardian's own spot: he met him there and stood on his Guardian Data.
The arena's door ends a slide as a wall does: MegaMan steps up to his
place beside the guardian, and after the battle takes his data as ever."""

# (onehit: any buster shot ends his fight; quiet: no random battle on the
# bridge; sliprun: SlipRunr's slide, as if on the board)
START = '--scene emu --seed 3 --run-depth 3 --guardian 1 --dev onehit,quiet,sliprun'

STEPS = [
    ('do', 'wait 150; mash A 300; wait 20'),              # the layer's arrival paged
    ('do', 'by arena; wait 60'),                          # on his way in: MegaMan's last words there,
    ('repeat', 'press A; wait 40', 'chat=closed'),        # ... paged
    ('do', 'by arena; wait 5; hold AHEAD+B 4'),           # a slide in
    ('until', 'card=title', 600),
    ('expect', 'met=beside'),                              # at his place, not on the guardian's spot
    ('until', 'chat=open', 600),                           # his words before the battle
    ('repeat', 'press A; wait 30', 'phase=custom', 40),
    ('repeat', 'press START; wait 10; press A; wait 60', 'phase=fight'),
    ('repeat', 'mash B 40; press UP; mash B 40; press DOWN; mash B 40; press DOWN; mash B 40; press UP', 'phase=results', 30),
    ('repeat', 'press A; wait 20', 'screen=map', 40),     # its RESULT paged
    ('repeat', 'wait 10', 'guardian=HeatMan beaten', 60),
    ('repeat', 'press A; wait 30', 'data=waiting', 40),   # his last word paged, his data seen
    ('repeat', 'press A; wait 30', 'screen=reward-pick', 40),
    ('expect', 'hp=200/200'),
    ('repeat', 'wait 30', 'pickready=yes'),
    ('repeat', 'press A; wait 30', 'data=taken', 40),
    ('until', 'exitpad=open', 120),
]
