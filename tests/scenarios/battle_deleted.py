"""MegaMan deleted in a random battle: BN6's GAME OVER, then the title's
summary (RESULT: deleted, where, the layer reached, the viruses deleted,
the one of the battle that ended the run among them, which 0.22.0's
RESULT left out; then RECORDS), the title with no run to continue, and a
next run from Lan's PC whose first battle is plain, at full HP, its tally
at nothing (it began at the last run's last battle's count: the battle
MegaMan was deleted in was never ended, and the next run's first layer
took it for one just fought)."""

# (onehit: a buster shot deletes the Mettaur in MegaMan's row; fragile: the
# first hit of the two left deletes him)
START = '--scene emu --seed 1 --run-depth 2 --dev onehit,fragile'

STEPS = [
    ('do', 'wait 150; mash A 300; wait 20'),                    # the arrival's words paged
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('do', 'foes 01@52 01@41 01@63'),
    ('until', 'phase=custom', 1200),
    ('repeat', 'press START; wait 10; press A; wait 60', 'phase=fight'),
    ('repeat', 'mash B 30', 'down>=1'),
    ('expect', 'down=1'),
    ('until', 'doing=gameover|scene=title', 3000),              # a Mettaur's wave: deleted
    ('repeat', 'press A; wait 60', 'scene=title', 30),
    ('expect', 'title=summary'),
    ('expect', 'page=result'),
    ('expect', 'ended=deleted'),
    ('expect', 'cause=in RoboDog Comp'),
    ('expect', 'reached=2'),
    ('expect', 'viruses=1'),
    ('expect', 'navis=0'),
    ('expect', 'learned=none'),
    ('repeat', 'press A; wait 40', 'page=records'),
    ('expect', 'opened=nothing'),
    ('repeat', 'press A; wait 40', 'title=start'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=new'),                                   # the run is over: nothing to continue
    ('repeat', 'press A; wait 40', 'title=setup|title=leaving|scene=emu'),
    ('repeat', 'press START; wait 60', 'scene=emu'),            # the setup's START: jack in
    # the next run: Lan's room (Dad's call paged), R at his PC, Lan's HP's
    # pink pad (AGENTS.md's town row's places), the first layer
    ('until', 'where=town', 1200),
    ('do', 'wait 40; mash A 460; wait 30'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('do', 'place -40 -8 7; wait 10; press R; wait 500'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('do', 'place 171 0 1; wait 10; hold UP+RIGHT 25'),
    ('until', 'where=layer', 1200),
    ('expect', 'layer=1'),
    ('do', 'wait 150; mash A 300; wait 20'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('expect', 'hp=100/100'),
    ('expect', 'viruses=0'),                                    # (the last run's last battle counted here again)
    ('expect', 'zenny=0'),
    ('do', 'battle'),
    ('until', 'phase=custom', 1200),
    ('expect', 'rules=plain'),
    ('expect', 'battle=random'),
    ('expect', 'turn=1'),
    ('expect', 'battles=1'),
]
