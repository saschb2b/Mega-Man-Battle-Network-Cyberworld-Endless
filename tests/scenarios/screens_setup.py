"""The JACK-IN SETUP after NEW GAME (src/scenes/title_setup.c): skipped on a
profile's first run; what is shut before it is earned (the endless net,
folders, Crosses, threat rungs) and open after (a short net won opens the
endless net, a threat rung and folders; a Cross's guardian deleted, his
Cross); and each choice reaching the run it starts: the net's length, the
folder, the Cross brought, the threat, each helper, each extra switched off.
The choice is the next setup's to start from."""

SETUP = '--scene setup'
WHAT = 'net startfolder startcross threat helpers extrasoff'
JACK_IN = [   # START: the run begun with the choice as it stood
    ('keep', WHAT),
    ('repeat', 'press START; wait 60', 'scene=emu'),
    ('until', 'where=town', 600),
    ('same', WHAT),
]
# (the short net's last layer by the autopilot, its foes at 1 HP: a win, and its summary)
WIN = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --setup short --run-depth 10 --dev onehit,god'
WON = [
    ('until', 'title=summary', 20000),
    ('repeat', 'press A; wait 30', 'title=start', 8),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('repeat', 'press A; wait 40', 'title=setup', 3),
]
HELPERS = ('head-start', 'heals', 'gentle', 'all-star')
EXTRAS = ('liberation', 'tournament', 'glitch', 'virus-battler', 'darkchips')


def row(tab, name):
    """The cursor on a row of a page: R to the page, Down to the row."""
    return [
        ('repeat', 'press R; wait 10', f'tab={tab}', 3),
        ('repeat', 'press DOWN; wait 6', f'row={name}', 8),
    ]


def switched(tab, k, line, was, now):
    """Row k of a page switched with A, and back with Left: the run has what it was left on."""
    return [
        *row(tab, k),
        ('expect', f'{line}={was}'),
        ('do', 'press A; wait 6'),
        ('expect', f'{line}={now}'),
        ('do', 'press LEFT; wait 6'),
        ('expect', f'{line}={was}'),
        ('do', 'press RIGHT; wait 6'),
        ('expect', f'{line}={now}'),
        *JACK_IN,
        ('expect', f'{line}={now}'),
    ]


MANY = {
    # a profile's first run: NEW GAME jacks in at once, the short net with the Standard folder
    'a-first-run-skips-it': dict(START='--scene title', STEPS=[
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('do', 'press A; wait 6'),
        ('expect', 'title=leaving'),
        ('until', 'where=town', 600),
        ('expect', 'net=short'),
        ('expect', 'startfolder=Standard'),
        ('expect', 'startcross=none'),
        ('expect', 'threat=0'),
        ('expect', 'helpers=0'),
        ('expect', 'extrasoff=0'),
    ]),
    # nothing earned yet: each row of RUN keeps its one value whichever way it is turned
    'shut-before-it-is-earned': dict(START=SETUP, STEPS=[
        ('expect', 'title=setup'),
        ('expect', 'endless=shut'),
        ('expect', 'folders=1'),
        ('expect', 'crosses=0'),
        ('expect', 'threatmax=0'),
        ('expect', 'helpersshown=4'),             # (BeastOut, the fifth, shows once earned)
        ('keep', WHAT),
        *[step for name in ('net', 'folder', 'cross', 'threat') for step in (
            ('repeat', 'press DOWN; wait 6', f'row={name}', 8),
            ('do', 'press RIGHT; wait 6; press A; wait 6; press LEFT; wait 6'),
            ('same', WHAT),
        )],
        *JACK_IN,
        ('expect', 'net=short'),
        ('expect', 'startfolder=Standard'),
    ]),
    # B: back to the title's menu, no run begun, and NEW GAME opens it again
    'b-closes-it': dict(START=WIN, STEPS=[
        *WON,
        ('do', 'press B; wait 20'),
        ('expect', 'title=menu'),
        ('expect', 'cursor=new'),
        ('do', 'press A; wait 20'),
        ('expect', 'title=setup'),
    ]),
    # A on a page's foot: NEXT to the next page, JACK IN! on the last
    'a-goes-page-by-page-to-jack-in': dict(START=SETUP, STEPS=[
        ('expect', 'tab=run'),
        ('expect', 'row=foot'),
        ('do', 'press A; wait 10'),
        ('expect', 'tab=help'),
        ('expect', 'row=foot'),
        ('do', 'press A; wait 10'),
        ('expect', 'tab=extras'),
        ('do', 'press L; wait 10'),               # L and R turn the pages too
        ('expect', 'tab=help'),
        ('do', 'press R; wait 10; press A; wait 10'),
        ('expect', 'title=leaving'),
        ('until', 'where=town', 600),
    ]),
    **{f'helper-{name}': dict(START=SETUP, STEPS=switched('help', k, 'helpers', 0, 1 << k)) for k, name in enumerate(HELPERS)},
    **{f'extra-{name}-off': dict(START=SETUP, STEPS=switched('extras', k, 'extrasoff', 0, 1 << k)) for k, name in enumerate(EXTRAS)},
    # several at once, and the next setup starts from them (the profile's, kept by a restart)
    'the-choice-is-the-next-setups': dict(START=SETUP, STEPS=[
        *row('help', 1), ('do', 'press A; wait 6'),
        *row('help', 2), ('do', 'press A; wait 6'),
        *row('extras', 3), ('do', 'press A; wait 6'),
        ('expect', 'helpers=6'),
        ('expect', 'extrasoff=8'),
        *JACK_IN,
        ('restart', SETUP),
        ('same', WHAT),
    ]),
    # a short net won: the endless net, threat 1 and folders open, each reaching the run
    # (the profile's first run begun as a player begins it, NEW GAME with no
    # setup, then the win: the setup it opens starts on that run's short
    # net, which was never written and read as the endless one)
    'a-win-opens-the-endless-net': dict(START='--scene title', STEPS=[
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('do', 'press A; wait 6'),
        ('until', 'where=town', 600),
        ('restart', WIN),
        *WON,
        ('expect', 'endless=open'),
        ('expect', 'row=net'),                    # the cursor on what the run opened
        ('expect', 'net=short'),                  # the last run's net until it is turned
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'net=endless'),
        *JACK_IN,
        ('expect', 'net=endless'),
        ('restart', SETUP),                       # open for good, and the next setup's start
        ('expect', 'endless=open'),
        ('expect', 'net=endless'),
    ]),
    'a-win-opens-a-threat-rung': dict(START=WIN, STEPS=[
        *WON,
        ('expect', 'threatmax=1'),
        *row('run', 'threat'),
        ('expect', 'threat=0'),
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'threat=1'),
        ('do', 'press RIGHT; wait 6'),            # round: no rung above the open one
        ('expect', 'threat=0'),
        ('do', 'press LEFT; wait 6'),
        ('expect', 'threat=1'),
        *JACK_IN,
        ('expect', 'threat=1'),
    ]),
    'a-win-opens-folders': dict(START=WIN, STEPS=[
        *WON,
        ('expect', 'folders>1'),
        *row('run', 'folder'),
        ('expect', 'startfolder=Standard'),
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'startfolder=Blade'),
        *JACK_IN,
        ('expect', 'startfolder=Blade'),
    ]),
    # a Cross's guardian deleted (HeatMan, by the autopilot, his HP 1), the run left: his Cross
    # and a folder are open, the cursor on the first row with news, and the Cross reaches the run
    'a-guardian-deleted-opens-his-cross': dict(
        START='CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --guardian 1 --dev onehit,god', STEPS=[
            ('until', 'guardian~done', 12000),
            ('restart', SETUP),
            ('expect', 'crosses=1'),
            ('expect', 'folders>1'),
            ('expect', 'row=folder'),
            ('do', 'press DOWN; wait 6'),
            ('expect', 'row=cross'),
            ('expect', 'startcross=none'),
            ('do', 'press RIGHT; wait 6'),
            ('expect', 'startcross=HeatCross'),
            ('do', 'press RIGHT; wait 6'),        # round: none, or the one open
            ('expect', 'startcross=none'),
            ('do', 'press A; wait 6'),            # A turns a row as Right does
            ('expect', 'startcross=HeatCross'),
            *JACK_IN,
            ('expect', 'startcross=HeatCross'),
        ]),
    # the run's setup a capture names: --setup NET,FOLDER,THREAT,HELPERS,CROSS,OFF
    'the-setup-option': dict(START='--scene emu --setup endless,1,2,3,2,5 --dev quiet', STEPS=[
        ('expect', 'net=endless'),
        ('expect', 'startfolder=Blade'),
        ('expect', 'threat=2'),
        ('expect', 'helpers=3'),
        ('expect', 'startcross!=none'),
        ('expect', 'extrasoff=5'),
    ]),
    'the-setup-option-short': dict(START='--scene emu --setup short,0,0,0 --dev quiet', STEPS=[
        ('expect', 'net=short'),
        ('expect', 'startfolder=Standard'),
        ('expect', 'startcross=none'),
        ('expect', 'extrasoff=0'),
    ]),
}
