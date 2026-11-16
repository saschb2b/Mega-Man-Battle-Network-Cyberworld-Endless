"""A run's end on the title (src/scenes/title_summary.c): the RESULT and the
RECORDS of a run MegaMan was deleted in and of a short net won, what the
profile holds after them (its runs, best layer and marks, kept by the next
start), no CONTINUE left, and the way on: NEW GAME, the JACK-IN SETUP with
what the run opened, the next run."""

# (a layer walked by the autopilot; fragile: the first hit deletes MegaMan)
LOST = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 4 --dev fragile'
# (the short net's last layer, the Cybeast's Nest, its foes at 1 HP: its fall wins the run)
WON = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --setup short --run-depth 10 --dev onehit,god'
MENU = ('repeat', 'press START; wait 40', 'title=menu')
NEXT = [   # the way on from the title: NEW GAME, the setup (a profile with a run behind it), the next run
    MENU,
    ('expect', 'saved=0'),                        # the run is over: no CONTINUE
    ('expect', 'cursor=new'),
    ('repeat', 'press A; wait 40', 'title=setup', 3),
]

MANY = {
    'deleted': dict(START=LOST, STEPS=[
        ('until', 'title=summary', 12000),
        ('expect', 'summary=result'),
        ('expect', 'ending=deleted'),
        ('expect', 'reached>=4'),                 # (on the layer it began on, or the next)
        ('expect', 'newbest=no'),                 # a first run has no record to beat
        ('expect', 'opened=none'),
        ('repeat', 'press A; wait 30', 'summary=records', 6),     # (no press taken for its first second)
        ('expect', 'title=summary'),
        ('repeat', 'press A; wait 30', 'title=start', 6),
        ('expect', 'runs=1'),
        ('expect', 'best>=4'),
        ('expect', 'marks=000'),
        ('keep', 'runs best'),
        *NEXT,
        ('expect', 'endless=shut'),
        ('expect', 'net=short'),
        ('repeat', 'press START; wait 60', 'scene=emu'),
        ('until', 'where=town', 600),
        ('expect', 'net=short'),
        ('restart', '--scene title'),             # the record is the profile's: the next start has it
        ('do', 'wait 30'),
        ('same', 'runs best'),
    ]),
    # START turns its screens as A does
    'deleted-and-start-turns-the-screens': dict(START=LOST, STEPS=[
        ('until', 'title=summary', 12000),
        ('repeat', 'press START; wait 30', 'summary=records', 6),
        ('repeat', 'press START; wait 30', 'title=start', 6),
        ('expect', 'runs=1'),
    ]),
    'won': dict(START=WON, STEPS=[
        ('until', 'title=summary', 20000),
        ('expect', 'summary=result'),
        ('expect', 'ending=won'),
        ('expect', 'reached=10'),
        ('expect', 'opened~the endless net'),
        ('expect', 'opened~threat 1'),
        ('repeat', 'press A; wait 30', 'summary=records', 6),
        ('repeat', 'press A; wait 30', 'title=start', 6),
        ('expect', 'runs=1'),
        ('expect', 'best=10'),
        ('expect', 'marks=080'),                  # the title's mark of a short net won
        *NEXT,
        ('expect', 'endless=open'),               # what the win opened, the cursor on its row
        ('expect', 'row=net'),
        ('expect', 'threatmax=1'),
        ('restart', '--scene title'),
        ('do', 'wait 30'),
        ('expect', 'runs=1'),
        ('expect', 'best=10'),
        ('expect', 'marks=080'),
    ]),
    # the made-up summaries a capture starts on, and its marks
    'the-summary-scene-of-a-run-lost': dict(START='--scene summary --run-depth 4', STEPS=[
        ('expect', 'title=summary'),
        ('expect', 'ending=deleted'),
        ('expect', 'reached=4'),
    ]),
    'the-summary-scene-of-a-run-won': dict(START='--scene summary --setup short --run-depth 10', STEPS=[
        ('expect', 'title=summary'),
        ('expect', 'ending=won'),
        ('expect', 'reached=10'),
        ('expect', 'opened~the endless net'),
    ]),
    'the-marks-option': dict(START='--scene title --marks 1FE', STEPS=[
        ('expect', 'marks=1fe'),
    ]),
}
