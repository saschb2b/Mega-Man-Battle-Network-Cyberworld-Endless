"""The shared dive's divers each in his own look (issue #878;
docs/online/shared-dive.md, Each in his own look): three games in three
looks dive the week's net, and on the first layer each draws the other two
in theirs, not in its own player's (every MegaMan had worn the local
look); a look changed on the layer reaches the others; none drawn in a
stale slot's colours."""
import os
import sys

import scenarios_online as online
from scenarios_online import soon

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from online_crew_view import CAST, dive  # noqa: E402

# (looks as the dev step sets them, "P,P,P,C,C,C", and as the state says them, "P.P.P/C.C.C")
ALPHA, BETA, GAMMA, BETA2 = '2,2,255,9,9,255', '4,4,255,5,5,255', '10,10,255,3,3,255', '3,3,255,6,6,255'


def said(look):
    p = look.split(',')
    return '.'.join(p[:3]) + '/' + '.'.join(p[3:])


def into(game, look):
    """`game` in the Square in `look`, then through the portal onto the dive's first layer."""
    return online.course(online.into_square(game), online.at(game, ('do', f'look {look}; wait 10')), dive(game))


MANY = {
    'each-his-own': dict(CAST, GAMES={'beta': online.HOME, 'gamma': online.HOME}, STEPS=online.course(
        into('', ALPHA), into('beta', BETA), into('gamma', GAMMA),
        ('sleep', 1.5),
        soon(f'divelooks~Beta {said(BETA)} drawn', '', 600),
        soon(f'divelooks~Gamma {said(GAMMA)} drawn', '', 300),
        soon(f'divelooks~Alpha {said(ALPHA)} drawn', 'beta', 300),
        soon(f'divelooks~Gamma {said(GAMMA)} drawn', 'beta', 300),
        soon(f'divelooks~Alpha {said(ALPHA)} drawn', 'gamma', 300),
        soon(f'divelooks~Beta {said(BETA)} drawn', 'gamma', 300),
        ('expect', 'divelookstale=0'),
        ('@beta', 'expect', 'divelookstale=0'),
        ('do', 'wait 30; shot'),
        ('@beta', 'do', 'wait 1; shot'),
        # (a look changed on the layer: the others see it come)
        ('@beta', 'do', f'look {BETA2}; wait 10'),
        soon(f'divelooks~Beta {said(BETA2)} drawn', '', 600),
        soon(f'divelooks~Beta {said(BETA2)} drawn', 'gamma', 300),
        ('do', 'wait 30; shot'),
        ('expect', 'divelookstale=0'),
    )),
}
