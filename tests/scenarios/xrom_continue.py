"""A quit on a layer of an older net, then CONTINUE with its ROM still
there: the same layer of the same area, drawn as its own game's, HP and
zenny as they were (a run saved in BN4's Town Area went on in it, the
checks of 5 November 2026 by hand)."""

# (seed 5: its first act an older net's area with either ROM; an act's second layer, no random battles)
START = '--scene emu --seed 5 --run-depth 2 --dev quiet'


def course(game, name):
    return dict(
        START=START, NEEDS=(game,),
        GUARDS=f"A quit on a layer of {game.upper()}'s {name}, then CONTINUE: the same layer, still {game.upper()}'s {name}",
        STEPS=[
            ('do', 'wait 150; mash A 300; wait 20'),                    # the arrival's words paged
            ('repeat', 'mash A 60; wait 30', 'chat=closed'),
            ('until', 'screen=map', 600),
            ('expect', f'area={name}'),
            ('expect', f'areafrom={game}'),
            ('keep', 'layer area areafrom acts hp zenny'),
            ('restart', '--scene title'),
            ('repeat', 'press START; wait 40', 'title=menu'),
            ('expect', 'cursor=continue'),
            ('repeat', 'press A; wait 40', 'scene=emu'),
            ('until', 'screen=map', 1200),
            ('same', 'layer area areafrom acts hp zenny'),
        ])


MANY = {'bn5': course('bn5', 'End Area'), 'bn4': course('bn4', 'Town Area')}
