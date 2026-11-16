"""The Glitch layer (docs/game/glitch-layer.md): its warp on an act's first
layer taken and refused, the layer behind it at each stage of its coming
apart, the copy at its heart refused, fought, deleted and paid, the layer
left by its exit for the run's own next, and none of it with the setup's
Glitch switched off."""

# (a layer's arrival: its words paged; the talks --talk opens come after)
ARRIVE = [('do', 'wait 150; mash A 300; wait 20'), ('repeat', 'mash A 60; wait 30', 'chat=closed')]
# (a question of the run's own starts on No: LEFT goes round to Yes)
YES = 'press LEFT; press A; wait 20'
NO = 'press A; wait 20'
# (the Custom screen left by its OK once it has opened, START on it: the gauge runs once the fight does)
OK = [('do', 'wait 90; press START; wait 10; press A'), ('until', 'custom>0', 600)]
# (the dev menu's "Win this battle", its fifth row; the cursor stays there for the next opening)
MENU = 'hold SELECT 4; hold SELECT+R 4; wait 6; '
WIN = MENU + 'press DOWN; press DOWN; press DOWN; press DOWN; press A; wait 60'
WIN_AGAIN = MENU + 'press A; wait 120'
# (the run quit and continued with the autopilot, which walks to the exit pad)
CONTINUE = [
    ('restart', 'CYBERWORLD_AUTOPILOT=1 --scene title --dev quiet'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'screen=map', 2400),
]
# (act 2's first layer, the warp forced there; quiet: no random battles)
WARP = '--scene emu --seed 3 --run-depth 4 --dev glitch,quiet'
# (a run begun on a Glitch layer)
LAYER = '--scene emu --seed 3 --run-depth 4 --dev glitch,quiet,side=4'
# (BN6's flags of a layer's Mystery Data taken, mapslot.h's MAPSLOT_MD_FLAG: the stage reads them)
MD = 0x1400

# (its stages held by glitchstage=N: what each does to the layer's battles,
# and whether the floor flips into another area's look, from stage 2)
BATTLES = ('own', 'own', 'mixed', 'deeper', 'deeper')


def stage(n):
    steps = [
        *ARRIVE,
        ('expect', 'sidelayer=glitch'),
        ('expect', f'glitchstage={n}'),
        ('expect', 'glitchlooks>=1'),
        ('expect', f'glitchbattles={BATTLES[n]}'),
        ('expect', 'glitched=on'),
        # (the stages' words, said one after another from stage 2, each paged)
        ('repeat', 'mash A 60; wait 30', f'glitchsaid={n}', 30),
        ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ]
    if n >= 2:
        steps.append(('repeat', 'wait 10', 'glitchlook!=own', 600))   # a flip: another area's look on the floor
        steps.append(('repeat', 'wait 10', 'glitchlook=own', 200))    # ... and the layer's own back
    else:
        steps += [('do', 'wait 1200'), ('expect', 'glitchlook=own')]  # tears and slips alone: no look but its own
    return dict(START=f'{LAYER},glitchstage={n + 1}', STEPS=steps,
                GUARDS=f"A Glitch layer at stage {n}: its battles {BATTLES[n]}, its words said, " +
                       ('the floor flipped into another look and back.' if n >= 2 else 'no flip of the floor yet.'))


MANY = {
    'warp-taken': dict(
        GUARDS="The Glitch warp's Yes: a Glitch layer of the same depth and area at stage 0, its copy waiting; a quit and "
               'CONTINUE keeps it; its exit leads to the act\'s next layer, the run\'s own.',
        START=WARP + ' --talk glitch:800',
        STEPS=[
            *ARRIVE,
            ('expect', 'glitchwarp=here'),
            ('expect', 'sidelayer=none'),
            ('keep', 'layer area'),
            ('until', 'chat=open', 900),
            ('repeat', YES, 'sidelayer=glitch', 30),
            ('until', 'chat=open', 1200),             # the Glitch layer's arrival
            ('repeat', 'mash A 60; wait 30', 'chat=closed'),
            ('until', 'screen=map', 600),
            ('same', 'layer area'),
            ('expect', 'glitchwarp=none'),
            ('expect', 'glitchstage=0'),
            ('expect', 'copy=waiting'),
            *CONTINUE,
            ('same', 'layer area'),
            ('expect', 'sidelayer=glitch'),
            ('expect', 'copy=waiting'),
            ('until', 'layer=5', 20000),              # the autopilot to its exit
            ('expect', 'sidelayer=none'),
            ('expect', 'glitchwarp=none'),
        ]),
    'warp-refused': dict(
        GUARDS="The Glitch warp's No, and a later talk's No: MegaMan stays on the layer, its warp still there; the layer's own "
               'exit leads on.',
        START=WARP + ' --talk glitch:800,glitch:1400',
        STEPS=[
            *ARRIVE,
            ('until', 'chat=open', 900),
            ('repeat', NO, 'chat=closed', 30),
            ('expect', 'sidelayer=none'),
            ('until', 'chat=open', 900),              # talked to again
            ('repeat', NO, 'chat=closed', 30),
            ('do', 'wait 120'),
            ('expect', 'sidelayer=none'),
            ('expect', 'glitchwarp=here'),
            ('expect', 'screen=map'),
            *CONTINUE,
            ('until', 'layer=5', 20000),
            ('expect', 'sidelayer=none'),
        ]),
    'warp-off': dict(
        GUARDS="The setup's Glitch switched off: no Glitch warp, where the dev switch would put one on every act's first layer.",
        START=WARP + ' --setup endless,0,0,0,0,4',
        STEPS=[
            *ARRIVE,
            ('expect', 'layer=4'),
            ('expect', 'glitchwarp=none'),
            ('expect', 'sidelayer=none'),
        ]),
    **{f'stage-{n}': stage(n) for n in range(5)},
    'stage-by-data': dict(
        GUARDS="A Glitch layer comes apart by its Mystery Data taken (BN6's own flags): stage 1 at the first, 4 with all, "
               'and never back.',
        START=LAYER,
        STEPS=[
            *ARRIVE,
            ('expect', 'glitchstage=0'),
            ('do', f'flags {MD} {MD} 1; wait 30'),
            ('expect', 'glitchstage=1'),
            ('expect', 'glitchbattles=own'),
            ('do', f'flags {MD} {MD + 15} 1; wait 30'),
            ('expect', 'glitchshare=100'),
            ('expect', 'glitchstage=4'),
            ('expect', 'glitchbattles=deeper'),
            ('do', f'flags {MD} {MD + 15} 0; wait 30'),     # put back as they were: the first alone taken
            ('expect', 'glitchshare<100'),
            ('expect', 'glitchstage=4'),
        ]),
    'copy-deleted': dict(
        GUARDS="The copy's Yes: its forms fought in one battle, each swapped in as the last falls; deleted, the glitch settles "
               'and its pick of Navi chips is taken, the copy gone; then the exit to the next layer.',
        START=LAYER + ' --talk copy:800',
        STEPS=[
            *ARRIVE,
            ('expect', 'copy=waiting'),
            ('until', 'chat=open', 900),
            ('repeat', YES, 'copy=fighting', 30),
            ('until', 'turn>=1', 1200),
            *OK,
            ('do', WIN),
            ('repeat', WIN_AGAIN, 'doing=map', 12),
            ('expect', 'copy=deleted'),
            ('expect', 'glitched=settled'),
            ('expect', 'zenny>0'),
            ('until', 'chat=open', 900),                       # its data: the pick's opening
            ('repeat', 'press A; wait 40', 'screen=reward-pick', 20),
            ('repeat', 'press A; wait 40', 'copy=taken', 20),
            ('repeat', 'press A; wait 40', 'chat=closed', 20),
            ('expect', 'glitchlook=own'),
            *CONTINUE,
            ('expect', 'copy=taken'),
            ('expect', 'glitched=settled'),
            ('until', 'layer=5', 20000),
            ('expect', 'sidelayer=none'),
        ]),
    'copy-refused': dict(
        GUARDS="The copy's No, and a later talk's: no fight, the glitch goes on, the copy waits.",
        START=LAYER + ' --talk copy:800,copy:1400',
        STEPS=[
            *ARRIVE,
            ('until', 'chat=open', 900),
            ('repeat', NO, 'chat=closed', 30),
            ('expect', 'copy=waiting'),
            ('until', 'chat=open', 900),
            ('repeat', NO, 'chat=closed', 30),
            ('do', 'wait 120'),
            ('expect', 'copy=waiting'),
            ('expect', 'glitched=on'),
            ('expect', 'screen=map'),
        ]),
    'copy-prize-left': dict(
        GUARDS="The copy's pick left with B: its chips stay with the copy, which waits deleted; a later talk offers them again, "
               'and one taken, it is gone.',
        START=LAYER + ' --talk copy:800,copy:2600',
        STEPS=[
            *ARRIVE,
            ('until', 'chat=open', 900),
            ('repeat', YES, 'copy=fighting', 30),
            ('until', 'turn>=1', 1200),
            *OK,
            ('do', WIN),
            ('repeat', WIN_AGAIN, 'doing=map', 12),
            ('until', 'chat=open', 900),
            ('repeat', 'press A; wait 40', 'screen=reward-pick', 20),
            ('repeat', 'press B; wait 40', 'screen=map', 20),
            ('repeat', 'press A; wait 40', 'chat=closed', 20),
            ('expect', 'copy=deleted'),
            ('expect', 'glitched=settled'),
            ('until', 'screen=reward-pick', 2400),             # talked to again: its data still there, the pick again
            ('repeat', 'press A; wait 40', 'copy=taken', 20),
        ]),
    'copy-lost': dict(
        GUARDS="MegaMan deleted in the copy's fight: the run is over, as at a guardian.",
        START=LAYER + ',fragile --talk copy:800',
        STEPS=[
            *ARRIVE,
            ('until', 'chat=open', 900),
            ('repeat', YES, 'copy=fighting', 30),
            ('until', 'turn>=1', 1200),
            *OK,
            ('until', 'scene=title', 6000),
            ('expect', 'title=summary'),
        ]),
}
