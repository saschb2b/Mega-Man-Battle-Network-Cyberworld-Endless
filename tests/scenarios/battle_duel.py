"""ProtoMan's duel (docs/game/rival.md), taken at his question and fought
to Chaud's verdict, and what the record says after: a race won under his
time (a win in the record, the first clearance), a race won too slowly
(a loss), the no-hit race lost by a hit, the third rung's netbattle with
ProtoMan himself in act 3 (the second clearance), his question
declined, a duel won and then a CONTINUE (fought once, counted once),
and MegaMan deleted in a race (the summary says where, and the record counts the
loss, which the verdict never reached)."""

# (the layer's duel talk opened once the arrival's words are paged; LEFT
# before each A: his question stands on No)
TAKE = [
    ('do', 'wait 150; mash A 300; wait 20'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('expect', 'duel=waiting'),
    ('until', 'chat=open', 1200),             # --talk duel: ProtoMan's terms
    ('repeat', 'press LEFT; press A; wait 30', 'battle=duel', 30),
    ('expect', 'duel=on'),
    ('until', 'phase=custom', 600),
]
OK = [('repeat', 'press START; wait 10; press A; wait 60', 'phase=fight')]
# (onehit: a buster shot along each row deletes the squad)
SHOOT = 'mash B 40; press UP; mash B 40; press DOWN; mash B 40; press DOWN; mash B 40; press UP'
RESULT = [('repeat', 'press A; wait 20', 'screen=map', 40), ('repeat', 'wait 10', 'duel=fought', 60)]
# Chaud's verdict paged
VERDICT = [('until', 'chat=open', 600), ('repeat', 'press A; wait 30', 'chat=closed', 40)]


def start(dev, duels=0, seed=1, depth=2):
    return f'--scene emu --seed {seed} --run-depth {depth} --dev {dev},duels={duels} --talk duel:700'


MANY = {
    'won': dict(START=start('onehit,god'), STEPS=[
        ('expect', 'duelswon=0'),
        ('expect', 'clearance=0'),
        *TAKE,
        ('expect', 'rung=0'),
        *OK,
        ('repeat', SHOOT, 'phase=results', 30),
        ('expect', 'result=won'),
        *RESULT,
        ('expect', 'verdict=beat'),
        ('expect', 'duelswon=1'),
        ('expect', 'duelslost=0'),
        ('expect', 'clearance=1'),            # his respect's first rung: the TagChip system
        *VERDICT,
        ('expect', 'duel=fought'),            # once a layer
    ]),
    'slow': dict(START=start('onehit,god'), STEPS=[
        *TAKE,
        *OK,
        ('do', 'wait 3000'),                  # his time runs out first
        ('repeat', SHOOT, 'phase=results', 30),
        ('expect', 'result=won'),
        *RESULT,
        ('expect', 'verdict=slow'),
        ('expect', 'duelswon=0'),
        ('expect', 'duelslost=1'),            # the battle won, the race lost
        ('expect', 'clearance=0'),
        *VERDICT,
    ]),
    'hit': dict(START=start('onehit,hp=900', 1, 2), STEPS=[
        ('expect', 'duelswon=1'),
        *TAKE,
        ('expect', 'rung=1'),                 # the second rung: under his time, and no hit taken
        *OK,
        ('keep', 'hp'),
        ('repeat', 'wait 30', 'hp<900', 200),  # a hit taken
        ('repeat', SHOOT, 'phase=results', 30),
        *RESULT,
        ('expect', 'verdict=hit'),            # under his time, and still lost
        ('expect', 'duelswon=1'),
        ('expect', 'duelslost=1'),
        *VERDICT,
    ]),
    # (issue #20: a CONTINUE met him again, and each duel won anew counted in the record)
    'won-then-continue': dict(START=start('onehit,god'), STEPS=[
        *TAKE,
        *OK,
        ('repeat', SHOOT, 'phase=results', 30),
        *RESULT,
        *VERDICT,
        ('expect', 'duelswon=1'),
        ('restart', '--scene title --dev onehit,god,quiet'),
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('expect', 'cursor=continue'),
        ('repeat', 'press A; wait 40', 'scene=emu'),
        ('until', 'screen=map', 2400),
        ('expect', 'duel=fought'),            # once a layer, across a CONTINUE too
        ('expect', 'duelswon=1'),
        ('expect', 'clearance=1'),
    ]),
    'declined': dict(START=start('onehit,god'), STEPS=[
        ('do', 'wait 150; mash A 300; wait 20'),
        ('repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('until', 'chat=open', 1200),         # --talk duel: ProtoMan's terms
        ('repeat', 'press A; wait 30', 'chat=closed', 40),   # A alone: his question stands on No
        ('do', 'wait 120'),
        ('expect', 'screen=map'),
        ('expect', 'duel=waiting'),           # he waits; nothing in the record
        ('expect', 'duelswon=0'),
        ('expect', 'duelslost=0'),
        ('expect', 'verdict=none'),
    ]),
    'netbattle': dict(START=start('onehit,god', 2, 1, 8), STEPS=[
        ('expect', 'clearance=1'),
        *TAKE,
        ('expect', 'rung=2'),                 # no race: ProtoMan himself
        *OK,
        ('repeat', SHOOT, 'phase=results', 30),
        *RESULT,
        ('expect', 'verdict=beat'),
        ('expect', 'duelswon=3'),
        ('expect', 'clearance=2'),
        *VERDICT,
    ]),
    'deleted': dict(START=start('fragile', 0, 2), STEPS=[
        *TAKE,
        *OK,
        ('until', 'doing=gameover|scene=title', 6000),
        ('repeat', 'press A; wait 60', 'scene=title', 30),
        ('expect', 'ended=deleted'),
        ('expect', "cause=in ProtoMan's duel"),
        ('repeat', 'press A; wait 40', 'page=records'),
        ('repeat', 'press A; wait 40', 'title=start'),
        ('restart', '--scene emu --seed 2 --run-depth 2 --dev quiet'),
        ('expect', 'duelslost=1'),            # the record has it for the next run
        ('expect', 'duelswon=0'),
        ('expect', 'chaud=known'),
    ]),
}
