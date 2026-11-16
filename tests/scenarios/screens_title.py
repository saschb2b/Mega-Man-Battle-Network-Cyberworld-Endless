"""The title (src/scenes/scene_title.c): PRESS START, the rows of its menu
with and without a saved run (CONTINUE only with one, the cursor on it),
OPTIONS by its row and by SELECT, and NEW GAME over a saved run, which asks
first: No (the cursor's answer) and B keep the run, Yes ends it for a new
one."""

FRESH = '--scene title'
# (a run saved on its second layer: its arrival's words paged, then the quit's save)
SAVED = '--scene emu --seed 3 --run-depth 2 --dev quiet'
SAVE = [
    ('do', 'wait 150; mash A 300; wait 20'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('keep', 'layer area zenny'),
    ('restart', FRESH),
]
MENU = ('repeat', 'press START; wait 40', 'title=menu')
ASK = [   # NEW GAME over the saved run: the question typed out, No under the cursor
    MENU,
    ('expect', 'cursor=continue'),
    ('do', 'press UP; wait 6'),
    ('expect', 'cursor=new'),
    ('do', 'press A; wait 6'),
    ('expect', 'title=confirm'),
    ('until', 'asks=no', 120),
]
KEPT = [   # the run is still CONTINUE's, on its layer
    ('expect', 'title=menu'),
    ('expect', 'saved=2'),
    ('do', 'press DOWN; wait 6'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'screen=map', 1200),
    ('same', 'layer area zenny'),
]

MANY = {
    # no saved run: PRESS START, then NEW GAME and OPTIONS, round; no CONTINUE
    'rows-without-a-save': dict(START=FRESH, STEPS=[
        ('do', 'wait 30'),
        ('expect', 'title=start'),
        ('expect', 'saved=0'),
        ('do', 'press START; wait 6'),
        ('expect', 'title=opening'),              # PRESS START flickers a moment
        ('until', 'title=menu', 120),
        ('expect', 'cursor=new'),
        ('do', 'press DOWN; wait 6'),
        ('expect', 'cursor=options'),
        ('do', 'press DOWN; wait 6'),
        ('expect', 'cursor=new'),
        ('do', 'press UP; wait 6'),
        ('expect', 'cursor=options'),
        ('do', 'press A; wait 40'),               # OPTIONS, the last row
        ('expect', 'screen=options-display'),
        ('do', 'press B; wait 20'),
        ('expect', 'screen=title'),
        ('expect', 'title=menu'),
        ('expect', 'cursor=options'),
    ]),
    # A starts as START does
    'a-is-start': dict(START=FRESH, STEPS=[
        ('repeat', 'press A; wait 40', 'title=menu', 3),
        ('expect', 'cursor=new'),
    ]),
    # SELECT: Options from PRESS START and from the menu, B back to where it was
    'select-is-options': dict(START=FRESH, STEPS=[
        ('do', 'wait 60; press SELECT; wait 40'),
        ('expect', 'screen=options-display'),
        ('do', 'press B; wait 20'),
        ('expect', 'title=start'),
        MENU,
        ('do', 'press SELECT; wait 40'),
        ('expect', 'screen=options-display'),
        ('do', 'press B; wait 20'),
        ('expect', 'title=menu'),
        ('expect', 'cursor=new'),
    ]),
    # a saved run: CONTINUE between NEW GAME and OPTIONS, the cursor on it, its layer known
    'rows-with-a-save': dict(START=SAVED, STEPS=[
        *SAVE,
        ('do', 'wait 30'),
        ('expect', 'saved=2'),
        MENU,
        ('expect', 'cursor=continue'),
        ('do', 'press DOWN; wait 6'),
        ('expect', 'cursor=options'),
        ('do', 'press DOWN; wait 6'),
        ('expect', 'cursor=new'),
        ('do', 'press DOWN; wait 6'),
        ('expect', 'cursor=continue'),
        ('repeat', 'press A; wait 40', 'scene=emu'),
        ('until', 'screen=map', 1200),
        ('same', 'layer area zenny'),
    ]),
    'new-game-asks-and-no-keeps-the-run': dict(START=SAVED, STEPS=[
        *SAVE, *ASK,
        ('do', 'press A; wait 6'),                # No, where the cursor stands
        *KEPT,
    ]),
    'new-game-asks-and-b-keeps-the-run': dict(START=SAVED, STEPS=[
        *SAVE, *ASK,
        ('do', 'press LEFT; wait 6'),
        ('expect', 'asks=yes'),
        ('do', 'press B; wait 6'),                # B, even with Yes under the cursor
        *KEPT,
    ]),
    # a press while the question types shows all of it and answers nothing
    'new-game-asks-and-a-press-while-it-types-answers-nothing': dict(START=SAVED, STEPS=[
        *SAVE,
        MENU,
        ('do', 'press UP; wait 6; press A; wait 2'),
        ('expect', 'asks=typing'),
        ('do', 'press A; wait 6'),
        ('expect', 'title=confirm'),
        ('expect', 'asks=no'),
        ('do', 'press B; wait 6'),
        *KEPT,
    ]),
    # Yes: a new run from Lan's room, and the old one gone: no CONTINUE brings its layer back
    'new-game-asks-and-yes-ends-the-run': dict(START=SAVED, STEPS=[
        *SAVE, *ASK,
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'asks=yes'),
        ('do', 'press A; wait 6'),
        ('repeat', 'press START; wait 60', 'scene=emu'),          # (the setup's START where it shows)
        ('until', 'where=town', 600),
        ('do', 'wait 40; mash A 460; wait 30'),                   # Dad's call paged: Lan free in his room
        ('restart', FRESH),
        MENU,
        ('expect', 'saved=0'),                                    # the old run is no CONTINUE's any more,
        ('expect', 'cursor=new'),                                 # ... and Lan's room, before a checkpoint, none yet
    ]),
}
