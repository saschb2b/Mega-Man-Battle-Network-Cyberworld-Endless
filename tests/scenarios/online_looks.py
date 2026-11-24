"""MegaMan's look in the Net Square (issue #854, protocol v30;
docs/online/square/looks.md): two peers in looks of their own seen by a
third, drawn in them; a look changed while there seen by the others; and
the plaza past its room (three looks besides MegaMan's own), the farthest
peer drawn in BN6's own colours until he comes nearer, no peer ever in a
stale slot's colours."""
import scenarios_online as online
from scenarios_online import soon

# (looks as the dev step sets them, "P,P,P,C,C,C", and as the state says them, "P.P.P/C.C.C")
BETA, GAMMA, DELTA, EPSILON, BETA2 = '1,1,255,3,3,255', '2,2,255,5,5,255', '3,3,255,7,7,255', '4,4,255,9,9,255', '5,5,255,1,1,255'


def said(look):
    p = look.split(',')
    return '.'.join(p[:3]) + '/' + '.'.join(p[3:])


SQUARE = dict(START=online.HOME, SERVER=True, FILES={'netpass.ini': online.SEEN_SQUARE})

MANY = {
    'looks-seen': dict(SQUARE, GAMES={'beta': online.HOME, 'gamma': online.HOME}, STEPS=online.course(
        online.into_square(),
        online.into_square('beta'),
        online.into_square('gamma'),
        ('@beta', 'do', f'look {BETA}; place 36 120 5; wait 10'),
        soon(f'looksent={said(BETA)}', 'beta'),
        ('@gamma', 'do', f'look {GAMMA}; place 36 88 5; wait 10'),
        ('do', 'place -4 120 1; wait 10'),
        soon(f'peerlooks~Beta {said(BETA)} drawn'),
        soon(f'peerlooks~Gamma {said(GAMMA)} drawn'),
        soon(f'peerlooks~Gamma {said(GAMMA)} drawn', 'beta'),
        ('expect', 'lookstale=0'),
        ('do', 'wait 20; shot'),
        # (a look changed in the Square: the others see it come)
        ('@beta', 'do', f'look {BETA2}; wait 10'),
        soon(f'peerlooks~Beta {said(BETA2)} drawn'),
        soon(f'peerlooks~Beta {said(BETA2)} drawn', 'gamma'),
        ('do', 'wait 20; shot'),
        ('expect', 'lookstale=0'),
    )),
    'looks-past-the-room': dict(SQUARE, GAMES={'beta': online.HOME, 'gamma': online.HOME, 'delta': online.HOME, 'epsilon': online.HOME},
                                STEPS=online.course(
        online.into_square(),
        online.into_square('beta'),
        online.into_square('gamma'),
        online.into_square('delta'),
        online.into_square('epsilon'),
        ('expect', 'part=plaza'),
        ('expect', 'lookroom=3'),
        ('@beta', 'do', f'look {BETA}; place 28 120 5; wait 10'),
        ('@gamma', 'do', f'look {GAMMA}; place 28 88 5; wait 10'),
        ('@delta', 'do', f'look {DELTA}; place 60 120 5; wait 10'),
        ('@epsilon', 'do', f'look {EPSILON}; place 92 120 5; wait 10'),
        ('do', 'place -4 120 1; wait 10'),
        # (four looks, room for three: the farthest drawn as MegaMan wears his, BN6's own)
        soon(f'peerlooks~Epsilon {said(EPSILON)} classic', '', 600),
        ('expect', f'peerlooks~Beta {said(BETA)} drawn'),
        ('expect', f'peerlooks~Gamma {said(GAMMA)} drawn'),
        ('expect', f'peerlooks~Delta {said(DELTA)} drawn'),
        ('do', 'wait 30; shot'),
        ('expect', 'lookstale=0'),
        # (MegaMan in a look of his own: BN6's own colours take one of the three, the two farthest in them)
        ('do', 'look 6,6,255,11,11,255; wait 10'),
        soon(f'peerlooks~Delta {said(DELTA)} classic', '', 300),
        ('expect', f'peerlooks~Epsilon {said(EPSILON)} classic'),
        ('expect', f'peerlooks~Beta {said(BETA)} drawn'),
        ('do', 'wait 30; shot'),
        ('expect', 'lookstale=0'),
        # (the farthest come nearest: his look comes back once the pick is made again, the farthest's goes)
        ('@epsilon', 'do', 'place 4 104 5; wait 10'),
        soon(f'peerlooks~Epsilon {said(EPSILON)} drawn', '', 300),
        ('expect', f'peerlooks~Delta {said(DELTA)} classic'),
        ('do', 'wait 30; shot'),
        ('expect', 'lookstale=0'),
    )),
}
