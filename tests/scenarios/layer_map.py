"""The layer's map on SELECT (issue #840): a press opens it and it stays
open frames later, the next press closes it, a battle's start closes it,
and held a while it shows while held and goes as SELECT is let go, as it
always did."""

RUN = 'CYBERWORLD_STATE_POS=1 --scene emu --dev quiet --seed 3 --run-depth 2'

PAGED = ('repeat', 'press A; wait 30', 'held=none', 30)         # a chat's boxes to its end, MegaMan free again
ARRIVE = [('do', 'wait 240'), PAGED, ('do', 'wait 30'), PAGED, ('expect', 'screen=map'), ('expect', 'map=closed')]

MANY = {
    # a press opens it, and it stays open; the next press closes it
    'press-opens-and-closes': dict(START=RUN, STEPS=ARRIVE + [
        ('do', 'press SELECT; wait 120'),
        ('expect', 'map=open'),
        ('do', 'hold RIGHT 40; wait 20'),                       # MegaMan walks under it
        ('expect', 'map=open'),
        ('do', 'press SELECT; wait 10'),
        ('expect', 'map=closed'),
    ]),
    # held, it shows while held and goes as SELECT is let go
    'hold-shows-while-held': dict(START=RUN, STEPS=ARRIVE + [
        ('do', 'hold SELECT 60'),
        ('expect', 'map=open'),
        ('do', 'wait 10'),
        ('expect', 'map=closed'),
    ]),
    # opened by a press, a battle's start closes it, and it stays closed on the map after
    'battle-closes-it': dict(START=RUN, STEPS=ARRIVE + [
        ('do', 'press SELECT; wait 30'),
        ('expect', 'map=open'),
        ('do', 'battle'),
        ('until', 'screen=battle', 600),
        ('expect', 'map=closed'),
    ]),
}
