"""The Tournament (docs/game/tournament.md): act 3's arena holding a
bracket of three Navis, shown on BN4's screen; its rounds won one after
another, each paying its Navi chip, the final's Guardian Data and the way
home; a round lost ending the run; a quit in it back at the arena's door;
and no bracket with the setup's switch off, without BN4's ROM, or in the
act of the run's Glitch layer."""

# (a layer's arrival: its words paged)
ARRIVE = [('do', 'wait 150; mash A 300; wait 20'), ('repeat', 'mash A 60; wait 30', 'chat=closed')]
# (act 3's arena, the short net's, a bracket forced there; the older nets
# off: BN6's Navis alone, whatever BN4 would dress)
ARENA = '--scene emu --seed 6 --run-depth 9 --setup short,0,0,0,0,32 --dev quiet,tourney'
# (... played by the autopilot, every hit a win)
PLAYED = 'CYBERWORLD_AUTOPILOT=1 ' + ARENA + ',god,onehit'
BN4 = ('bn4',)

MANY = {
    'won': dict(
        NEEDS=BN4,
        GUARDS="A bracket won: BN4's screen, round 1 and round 2 each paying zenny and its Navi chip, the final as the act's "
               "guardian, his Guardian Data, and the exit home.",
        START=PLAYED,
        STEPS=[
            ('expect', 'bracket=ahead'),
            ('expect', 'round=1'),
            ('expect', 'roundchips=0'),
            ('keep', 'roundfoe'),
            ('repeat', 'wait 20', 'bracket=screen', 300),   # MegaMan stepped in: every entrant on BN4's screen
            ('until', 'bracket=round', 1200),
            ('same', 'roundfoe'),
            ('until', 'round=2', 12000),                    # round 1 won
            ('expect', 'roundchips=1'),
            ('expect', 'zenny>0'),
            ('until', 'round=3', 12000),                    # round 2 won
            ('expect', 'roundchips=2'),
            ('until', 'bracket=won', 12000),                # the final
            ('until', 'guardian~done', 6000),               # his Guardian Data taken
            ('until', 'where=town', 12000),                 # the act's exit: home
        ]),
    'hp-carried': dict(
        NEEDS=BN4,
        GUARDS="No heal between the rounds: round 2 begins with the HP round 1 left MegaMan.",
        START='CYBERWORLD_AUTOPILOT=1 ' + ARENA + ',onehit,hp=500',
        STEPS=[
            ('until', 'round=2', 12000),
            ('expect', 'hp!=500/500'),                      # hit in round 1
            ('keep', 'hp'),
            ('repeat', 'wait 10', 'doing=battle', 600),     # round 2's battle begun
            ('same', 'hp'),
        ]),
    'lost': dict(
        NEEDS=BN4,
        GUARDS="A round lost ends the run, as a guardian lost does.",
        START='CYBERWORLD_AUTOPILOT=1 ' + ARENA + ',fragile',
        STEPS=[
            ('expect', 'bracket=ahead'),
            ('until', 'bracket=round', 6000),
            ('until', 'scene=title', 12000),
            ('expect', 'title=summary'),
        ]),
    'door': dict(
        NEEDS=BN4,
        GUARDS="A quit after a round won comes back at the arena's door: round 1 again, the round's chip and zenny not kept.",
        START=PLAYED,
        STEPS=[
            ('keep', 'roundfoe'),
            ('until', 'round=2', 12000),
            ('expect', 'roundchips=1'),
            ('restart', 'CYBERWORLD_AUTOPILOT=1 --scene title --dev quiet,tourney,god,onehit'),
            ('repeat', 'press START; wait 40', 'title=menu'),
            ('expect', 'cursor=continue'),
            ('repeat', 'press A; wait 40', 'scene=emu'),
            ('repeat', 'wait 10', 'screen=map', 300),
            ('expect', 'layer=9'),
            ('expect', 'bracket=door|bracket=screen'),      # in the arena, not at the layer's start (ahead)
            ('expect', 'round=1'),
            ('same', 'roundfoe'),
            ('expect', 'roundchips=0'),
            ('expect', 'zenny=0'),
            ('repeat', 'wait 10', 'card=area', 100),        # the act's card again: the Navis met by then named
            ('expect', 'cardsays~1: DiveMan   2: TenguMan'),
            ('expect', 'cardsays~Final: ???'),
        ]),
    'card': dict(
        NEEDS=BN4,
        GUARDS="The act's card on its first layer lists the bracket in the guardian's place: round 1, round 2 and the "
               'final, ??? for a Navi never met.',
        START='--scene emu --seed 6 --run-depth 7 --setup short,0,0,0,0,32 --dev quiet,tourney',
        STEPS=[
            ('repeat', 'wait 10', 'card=area', 100),
            ('expect', 'cardsays~1: ???   2: ???'),
            ('expect', 'cardsays~Final: ???'),
            ('expect', 'bracket=none'),                     # the arena's layer holds it, not this one
        ]),
    'off': dict(
        NEEDS=BN4,
        GUARDS="The setup's Tournament switched off: act 3's arena holds its single guardian, the dev switch's force included.",
        START='--scene emu --seed 6 --run-depth 9 --setup short,0,0,0,0,34 --dev quiet,tourney',
        STEPS=[
            *ARRIVE,
            ('expect', 'layer=9'),
            ('expect', 'bracket=none'),
            ('expect', 'guardian~waiting'),
        ]),
    'without-bn4': dict(
        GUARDS="Without BN4's ROM no bracket stands, as its screen shows every bracket: the arena holds its guardian.",
        START=ARENA,
        STEPS=[
            *ARRIVE,
            ('expect', 'bracket=none'),
            ('expect', 'guardian~waiting'),
        ]),
    'glitch-act': dict(
        NEEDS=BN4,
        GUARDS="The act of the run's Glitch layer keeps its single guardian: no bracket there.",
        START='--scene emu --seed 2 --run-depth 9 --setup short,0,0,0,0,32 --dev quiet,tourney',
        STEPS=[
            *ARRIVE,
            ('expect', 'bracket=none'),
            ('expect', 'guardian~waiting'),
        ]),
}
