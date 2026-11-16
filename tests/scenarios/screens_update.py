"""The update box (docs/engine/updates.md; src/core/options_news.c): a newer
version found at the start opens its news over the title's menu once; B
says Later and it does not open again, in this session or the next;
Options shows the version on a line of its own, whose A opens the box
again; and with Updates off the game looks for nothing. The newer version
is made up and served on this machine alone ({update}); the box's Yes,
which would open the download page in a browser, is never pressed."""

# (the game told where to look, and which version it counts as; settings:
# a headless start reads settings.ini, where a Later and the switch are kept)
START = 'CYBERWORLD_UPDATE_URL={update} CYBERWORLD_UPDATE_FROM=0.1.0 --scene title --dev settings'
MENU = ('repeat', 'press START; wait 40', 'title=menu')
GAME_PAGE = [
    ('repeat', 'press SELECT; wait 40', 'screen~options', 3),
    ('repeat', 'press L; wait 20', 'screen=options-game', 3),
]

MANY = {
    'the-box-opens-once-and-b-is-later': dict(START=START, STEPS=[
        MENU,
        ('until', 'screen=update-box', 20000),
        ('expect', 'update=found'),
        ('expect', 'newversion=99.0.0'),
        ('do', 'wait 30; press RIGHT; wait 6; press B; wait 20'),   # a page on, then Later
        ('expect', 'screen=title'),
        ('expect', 'title=menu'),
        ('do', 'wait 200'),
        ('expect', 'screen=title'),               # once: not over the menu again
        ('do', 'press SELECT; wait 40'),          # Options opens on the version's line
        ('expect', 'screen=options-game'),
        ('expect', 'line=update'),
        ('do', 'press A; wait 40'),               # its A: the box again
        ('expect', 'screen=update-box'),
        ('do', 'press B; wait 20'),
        ('expect', 'screen=options-game'),
        ('restart', START),                       # the next start finds it again, and asks nothing: Later is kept
        MENU,
        ('until', 'update=found', 20000),
        ('do', 'wait 200'),
        ('expect', 'screen=title'),
        ('expect', 'title=menu'),
    ]),
    'updates-off-looks-for-nothing': dict(START=START, STEPS=[
        MENU,
        ('until', 'screen=update-box', 20000),
        ('do', 'wait 30; press B; wait 20'),
        *GAME_PAGE,
        ('repeat', 'press DOWN; wait 6', 'line=updates', 8),
        ('expect', 'value=On'),
        ('do', 'press A; wait 6'),
        ('expect', 'value=Off'),
        ('expect', 'update=none'),                # off at once: its line gone
        ('do', 'press B; wait 20'),
        ('restart', START),
        MENU,
        ('do', 'wait 600'),                       # (a look answers in a few frames here)
        ('expect', 'screen=title'),
        *GAME_PAGE,
        ('repeat', 'press DOWN; wait 6', 'line=updates', 8),
        ('expect', 'value=Off'),
        ('expect', 'update=none'),                # nothing was looked for, and nothing found
    ]),
}
