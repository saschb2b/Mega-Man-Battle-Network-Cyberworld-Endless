"""Issue #696: a liberation's board left with a teammate in play left the
player in the teammate's body: Lan walked the next run's town in
EraseMan's sprite (a player on X), or HeatMan's, whose jack-in at Lan's
PC then stood still (agergo100 on the issue). The player's spawn takes
his own body again wherever no board is on: on the next layer, and as
the next run's Lan, whose jack-in reaches Lan's HP. (Colonel's Soul
held, --dev souls=1: the board's team MegaMan and Colonel; R puts
Colonel in play.)"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from liberation_board import BRIEFED  # noqa: E402

BOARD = '--scene emu --seed 5 --run-depth 5 --dev dark,souls=1'
COLONEL = BRIEFED + [
    ('expect', 'body=own'),
    ('do', 'press R; wait 10'),
    ('repeat', 'press A; wait 20', 'inplay=Colonel', 40),     # his word as he first comes into play
    ('repeat', 'press A; wait 20', 'board=move', 40),
    ('expect', 'body!=own'),
]
FREE = ('until', 'held=none', 600)

MANY = {
    'next-layer': dict(
        GUARDS="The board left with Colonel in play (the dev menu's Next layer): MegaMan on the next layer in his own body.",
        NEEDS=('bn5',),
        START=BOARD,
        STEPS=COLONEL + [
            ('do', 'hold SELECT 4; hold SELECT+R 4; wait 6; ' + 'press DOWN; ' * 7 + 'press A; wait 6'),
            ('until', 'layer=6', 1200),
            ('until', 'where=layer', 1200),
            ('expect', 'board=none'),
            ('expect', 'body=own'),
        ]),
    'next-run': dict(
        GUARDS="The board left with Colonel in play by the reset's chord, then NEW GAME: Lan in his room in his own body, and "
               "R at his PC jacks in to Lan's HP.",
        NEEDS=('bn5',),
        START=BOARD,
        STEPS=COLONEL + [
            ('do', 'hold A+B+SELECT+START 10; wait 30'),
            ('until', 'scene=title', 600),
            ('repeat', 'press START; wait 40', 'title=menu', 12),
            ('repeat', 'press UP; wait 10', 'cursor=new', 4),
            ('repeat', 'press A; wait 40', 'title=confirm', 6),            # NEW GAME over the saved run asks
            ('repeat', 'press LEFT; wait 10; press A; wait 40', 'title=setup|title=leaving|scene=emu', 6),   # Yes
            ('repeat', 'press START; wait 60', 'scene=emu'),               # the setup's START, where it opens
            ('until', "place=Lan's Room", 2000),
            ('do', 'wait 40; mash A 460; wait 30'),                        # Dad's call paged
            FREE,
            ('expect', 'body=own'),
            ('do', 'place -40 -8 7; wait 10; press R; wait 40'),           # R on the PC's cells
            ('until', "place=Lan's HP", 900),
            ('expect', 'body=own'),
        ]),
}
