"""Options (src/core/options.c): each line of its Display, Audio and Game
pages turned, shown changed, and still so after the game was quit and
started again (settings.ini); START's question putting a page back as it
came, No keeping it; the page and the lines it was left on kept; and the
same Options from the PET in a run, a setting made on either side holding
on the other."""

# (settings: a headless start reads the data folder's settings.ini, as a player's start does)
TITLE = '--scene title --dev settings'
PAGES = ('display', 'audio', 'game')


def line(page, name):
    """Options open on a page, the cursor on a line (SELECT on the title; R to the page; Down to the line)."""
    return [
        ('repeat', 'press SELECT; wait 40', 'screen~options', 3),
        ('repeat', 'press R; wait 20', f'screen=options-{page}', 3),
        ('repeat', 'press DOWN; wait 6', f'line={name}', 8),
    ]


def kept(page, name, came, press, value, start=TITLE):
    """A line as it came, turned by `press`, and the same after a restart."""
    return [
        *line(page, name),
        ('expect', f'value={came}'),
        ('do', press + '; wait 6'),
        ('expect', f'value={value}'),
        ('do', 'press B; wait 20'),
        ('expect', 'screen=title'),
        ('restart', start),
        *line(page, name),
        ('expect', f'value={value}'),
    ]


# page, line, as it comes, the presses, the value then (Left and Right step,
# A goes round; the levels stop at their ends)
LINES = [
    ('display', 'scale', 'Auto', 'press RIGHT', 'Integer'),
    ('display', 'smooth', 'Off', 'press A', 'On'),
    ('display', 'colour', 'Off', 'press RIGHT; wait 6; press RIGHT', 'VBA'),
    ('display', 'blend', 'Off', 'press RIGHT', 'Simple'),
    ('display', 'lcd3x', 'Off', 'press A', 'On'),
    ('display', 'depth', 'Off', 'press RIGHT', 'On'),
    ('audio', 'music', '10', 'press LEFT; wait 6; press LEFT; wait 6; press LEFT', '7'),
    ('audio', 'effects', '10', 'press A', 'Off'),               # (A from the loudest: round to Off)
    ('audio', 'output', 'Speakers', 'press RIGHT', 'Headphones'),
    ('audio', 'sound', 'GBA', 'press LEFT', 'mGBA'),            # (Left from the first: round to the last)
    ('game', 'fast', '3x', 'press RIGHT', '4x'),
]

MANY = {
    **{f'{name}-kept': dict(START=TITLE, STEPS=kept(page, name, came, press, value)) for page, name, came, press, value in LINES},
    # a controller's line (a virtual one, --pad): Vibration
    'vibration-kept': dict(START=TITLE + ' --pad xbox', STEPS=kept('audio', 'vibration', 'Medium', 'press RIGHT', 'Strong', TITLE + ' --pad xbox')),
    # the levels stop at their ends by Left and Right: no leap from silence to the loudest
    'levels-stop-at-their-ends': dict(START=TITLE, STEPS=[
        *line('audio', 'music'),
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'value=10'),
        ('do', 'press A; wait 6'),
        ('expect', 'value=Off'),
        ('do', 'press LEFT; wait 6'),
        ('expect', 'value=Off'),
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'value=1'),
    ]),
    # START asks before it puts a page back: No (the cursor's answer) and B keep it, Yes puts
    # the page's lines back as they came, the other pages' left alone, and that is kept too
    'defaults-ask-first': dict(START=TITLE, STEPS=[
        *line('audio', 'output'),
        ('do', 'press RIGHT; wait 6'),
        *line('audio', 'music'),
        ('do', 'press LEFT; wait 6'),
        ('expect', 'value=9'),
        *line('display', 'colour'),
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'value=GBA'),
        *line('audio', 'music'),
        ('do', 'press START; wait 6'),
        ('expect', 'defaults=no'),
        ('do', 'press A; wait 6'),                # No
        ('expect', 'value=9'),
        ('do', 'press START; wait 6; press LEFT; wait 6'),
        ('expect', 'defaults=yes'),
        ('do', 'press B; wait 6'),                # B: no, whatever the cursor is on
        ('expect', 'value=9'),
        ('expect', 'screen=options-audio'),
        ('do', 'press START; wait 6; press LEFT; wait 6; press A; wait 6'),
        ('expect', 'value=10'),
        *line('audio', 'output'),
        ('expect', 'value=Speakers'),
        *line('display', 'colour'),
        ('expect', 'value=GBA'),                  # another page's line: not this START's
        ('do', 'press B; wait 20'),
        ('restart', TITLE),
        *line('audio', 'music'),
        ('expect', 'value=10'),
        *line('audio', 'output'),
        ('expect', 'value=Speakers'),
        *line('display', 'colour'),
        ('expect', 'value=GBA'),
    ]),
    # L and R turn the pages round; opened again, Options is on the page and the line it was left on
    'pages-and-the-place-left': dict(START=TITLE, STEPS=[
        ('repeat', 'press SELECT; wait 40', 'screen~options', 3),
        ('expect', 'screen=options-display'),
        ('do', 'press R; wait 20'),
        ('expect', 'screen=options-audio'),
        ('do', 'press R; wait 20'),
        ('expect', 'screen=options-game'),
        ('do', 'press R; wait 20'),
        ('expect', 'screen=options-display'),
        ('do', 'press L; wait 20'),
        ('expect', 'screen=options-game'),
        ('do', 'press L; wait 20'),
        ('expect', 'screen=options-audio'),
        ('repeat', 'press DOWN; wait 6', 'line=output', 8),
        ('do', 'press B; wait 20'),
        ('expect', 'screen=title'),
        ('do', 'press SELECT; wait 40'),
        ('expect', 'screen=options-audio'),
        ('expect', 'line=output'),
    ]),
    # the Game page's Controls line opens the controls screen, B back to the page
    'controls-line': dict(START=TITLE, STEPS=[
        *line('game', 'controls'),
        ('expect', 'value=-'),
        ('do', 'press A; wait 40'),
        ('expect', 'screen=controls'),
        ('do', 'press B; wait 20'),
        ('expect', 'screen=options-game'),
    ]),
    # in a run the PET's last row is the same Options: a line turned there holds on the title after a quit
    'from-the-pet-in-a-run': dict(START='pet/options --dev settings', STEPS=[
        ('expect', 'screen=options-display'),
        ('expect', 'scene=emu'),
        ('repeat', 'press DOWN; wait 6', 'line=colour', 8),
        ('expect', 'value=Off'),
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'value=GBA'),
        ('do', 'press B; wait 40'),
        ('expect', 'screen=pet'),                 # back in the PET, the run going on
        ('restart', TITLE),
        *line('display', 'colour'),
        ('expect', 'value=GBA'),
    ]),
    # ... and one turned on the title is the PET's in the next run
    'the-titles-setting-in-the-pet': dict(START=TITLE, STEPS=[
        *line('audio', 'sound'),
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'value=Soft'),
        ('do', 'press B; wait 20'),
        ('restart', 'pet/options --dev settings'),
        ('repeat', 'press R; wait 20', 'screen=options-audio', 3),
        ('repeat', 'press DOWN; wait 6', 'line=sound', 8),
        ('expect', 'value=Soft'),
    ]),
}
