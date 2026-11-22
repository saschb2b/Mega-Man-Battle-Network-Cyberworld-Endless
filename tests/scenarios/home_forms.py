"""Issue #774's forms screen: R on the PET's menu opens it, CROSSSELECT's
five on the left and the other Crosses held as spares. A sixth Cross won
goes among the spares (BN6's first five held stay CROSSSELECT's); one
swapped in from them is CROSSSELECT's in BN6's order, kept by the run
through a quit and CONTINUE (run.cross_active), a seventh won after it
waits among the spares too, and CROSSSELECT in the next battle lists the
five chosen and no other (docs/game/meta/forms-screen.md)."""

# (the setup's HeatCross, Gregar's other four by their flags: five held)
SWAP_START = '--scene emu --seed 1 --run-depth 2 --setup short,0,0,0,1 --dev onehit,god'
GREGAR = 'HeatCross,ElecCross,SlashCross,EraseCross,ChargeCross'
CHOSEN = 'HeatCross,SlashCross,EraseCross,ChargeCross,SpoutCross'

ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
]
FORMS = [
    ('repeat', 'press START; wait 40', 'screen=pet', 6),
    ('do', 'wait 20; press R; wait 40'),
    ('expect', 'screen=forms'),
]
LEAVE = [('repeat', 'press B; wait 40', 'screen=map', 6)]

MANY = {
    'swap': dict(
        START=SWAP_START,
        STEPS=[
            *ARRIVE,
            ('do', 'flags 0xE3 0xE6 1; wait 4'),
            ('expect', f'crossesfive={GREGAR}'),
            *FORMS,
            ('expect', f'forms~five {GREGAR} spare none'),
            *LEAVE,
            ('do', 'flags 0xE7 0xE7 1; wait 4'),                   # a sixth: SpoutCross, Falzar's first
            ('expect', f'crossesfive={GREGAR}'),
            *FORMS,
            ('expect', f'forms~five {GREGAR} spare SpoutCross at HeatCross'),
            ('do', 'press DOWN; wait 8; press A; wait 12'),          # ElecCross taken up, the cursor to the spares
            ('expect', 'forms~at SpoutCross taken ElecCross'),
            ('do', 'press A; wait 20'),                              # ... swapped with SpoutCross
            ('expect', f'forms~five {CHOSEN} spare ElecCross'),
            ('expect', f'crossesfive={CHOSEN}'),
            *LEAVE,
            ('restart', '--scene title'),                            # quit, and CONTINUE: the five are the run's
            ('repeat', 'press START; wait 40', 'title=menu'),
            ('expect', 'cursor=continue'),
            ('repeat', 'press A; wait 40', 'scene=emu'),
            ('until', 'screen=map', 1200),
            ('expect', f'crossesfive={CHOSEN}'),
            ('do', 'flags 0xE8 0xE8 1; wait 4'),                   # a seventh: TomahawkCross, among the spares
            ('expect', f'crossesfive={CHOSEN}'),
            ('do', 'foes 01@52'),
            ('until', 'phase=custom', 1200),
            ('do', 'wait 60; press UP; wait 40'),                    # CROSSSELECT
            ('expect', f'crosslist={CHOSEN}'),
        ]),
    # (a Cross won from a guardian, the five held before him CROSSSELECT's still)
    'won': dict(
        START='CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --guardian 6 --dev onehit,quiet',
        STEPS=[
            ('do', 'flags 0xE2 0xE6 1; wait 4'),
            ('expect', f'crossesfive={GREGAR}'),
            ('until', 'guardian=SpoutMan beaten', 8000),
            ('until', 'data=taken', 3000),
            ('expect', f'crossesheld={GREGAR},SpoutCross'),
            ('expect', f'crossesfive={GREGAR}'),
        ]),
}
