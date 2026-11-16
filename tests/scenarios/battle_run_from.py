"""A battle run from counts the viruses deleted in it (0.22.0: RESULT
counted too few, a battle run from none): one of three Mettaurs deleted,
then BN6's own "should we run?" from the Custom screen, turn after turn
until MegaMan gets away, and the run's tally is one more on the map."""

# (onehit: a buster shot deletes the Mettaur in MegaMan's row; god: the
# other two cannot delete him while he waits for the gauge)
START = '--scene emu --seed 1 --run-depth 2 --dev onehit,god'

# one turn's try: L opens the Custom screen once the gauge is full, L there
# asks, A is Yes, and two A's page what MegaMan says of it
TRY = 'press L; wait 120; press L; wait 100; press A; wait 100; press A; wait 60; press A; wait 180'

STEPS = [
    ('do', 'wait 150; mash A 300; wait 20'),                    # the arrival's words paged
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('expect', 'viruses=0'),
    ('do', 'foes 01@52 01@41 01@63'),
    ('until', 'phase=custom', 1200),
    ('repeat', 'press START; wait 10; press A; wait 60', 'phase=fight'),
    ('repeat', 'mash B 30', 'down>=1'),
    ('expect', 'foes=3'),
    ('expect', 'down=1'),
    ('repeat', TRY, 'screen=map', 20),
    ('expect', 'lastbattle=ran'),
    ('expect', 'viruses=1'),
    ('expect', 'zenny=0'),                                      # no RESULT, no reward
]
