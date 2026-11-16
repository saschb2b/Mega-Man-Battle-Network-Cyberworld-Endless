"""Issue #688: every new run began with the last run's MegaMan. A new game
begins from the state the boot keeps in the data folder; on the
reporter's device that state held a played game, so each NEW GAME kept
its HP, zenny, SubChips, RegUPs, NaviCust and Crosses, and its pack
(blank chips in the folder, "damaged chip data" at its edit), until the
app's data was deleted. How it came to hold one is not found. A kept
state that is no new game's is made again: here the boot is given the
run's own checkpoint as its kept state (--dev bootplayed), and the new
run holds nothing of that run."""

FREE = ('until', 'held=none', 600)
ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),                    # the arrival's words paged
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
]

START = '--scene emu --seed 3 --run-depth 2 --dev quiet --talk zenny:300'

STEPS = ARRIVE + [
    ('repeat', 'mash A 60; wait 30', 'zenny>9000', 20), FREE,  # the layer's 10000 zenny taken; the quit saves the run with them
    ('restart', '--scene title --dev bootplayed'),
    ('repeat', 'press START; wait 40', 'title=menu', 12),
    ('repeat', 'press UP; wait 10', 'cursor=new', 4),
    ('repeat', 'press A; wait 40', 'title=confirm', 6),            # NEW GAME over the saved run asks
    ('repeat', 'press LEFT; wait 10; press A; wait 40', 'scene=emu', 6),   # Yes
    ('until', "place=Lan's Room", 2000),
    ('do', 'wait 40; mash A 460; wait 30'),                     # Dad's call paged
    FREE,
    ('expect', 'zenny=0'),
    ('expect', 'hp=100/100'),
    ('expect', 'chips=30'),
    ('expect', 'programs=0'),
    ('expect', 'subchips=minienrg 0 fullenrg 0 sneakrun 0 untrap 0'),
]
