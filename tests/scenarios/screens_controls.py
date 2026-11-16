"""The controls screen (Options' Game page, src/core/controls.c): a key given
to a button and kept once the new A is pressed, still set after a restart
and working in the game; a change left without the new A not kept; the
last key of a button never cleared; DEFAULTS; a clash swapped and said; and
a controller's preset turned and kept (a virtual one, --pad)."""

# (settings: a headless start reads keys.ini and pad.ini, as a player's start does)
TITLE = '--scene title --dev settings'
PAD = '--scene title --pad xbox --dev settings'
OPEN = [   # Options' Game page, its Controls line, A
    ('repeat', 'press SELECT; wait 40', 'screen~options', 3),
    ('repeat', 'press L; wait 20', 'screen=options-game', 3),
    ('repeat', 'press DOWN; wait 6', 'line=controls', 8),
    ('do', 'press A; wait 40'),
    ('expect', 'screen=controls'),
]
KEYBOARD = [*OPEN, ('repeat', 'press L; wait 6', 'device=keyboard', 2)]


def on(button, slot='main'):
    """The cursor on a button's row and slot."""
    return [
        ('repeat', 'press DOWN; wait 6', f'button={button}', 12),
        ('repeat', 'press RIGHT; wait 6', f'slot={slot}', 2),
    ]


MANY = {
    # B's ALSO key set to M: asked for in a box, kept at DONE once the new A (J) is pressed,
    # there after a restart, and M is B in the game (it leaves Options)
    'a-key-set-and-kept': dict(START=TITLE, STEPS=[
        *KEYBOARD, *on('b', 'also'),
        ('expect', 'bound=Z'),
        ('do', 'press A; wait 20'),
        ('expect', 'asking=slot'),
        ('do', 'press KEY.M; wait 20'),
        ('expect', 'asking=none'),
        ('expect', 'bound=M'),
        ('do', 'press B; wait 20'),               # back: the keep box, as something changed
        ('expect', 'asking=keep'),
        ('do', 'press KEY.J; wait 20'),           # the new map's A
        ('expect', 'screen=options-game'),
        ('do', 'press KEY.M; wait 20'),           # M is B now: Options closes
        ('expect', 'screen=title'),
        ('restart', TITLE),
        *KEYBOARD, *on('b', 'also'),
        ('expect', 'bound=M'),
        ('do', 'press B; wait 20'),               # nothing changed: away at once
        ('expect', 'screen=options-game'),
        ('do', 'press KEY.M; wait 20'),
        ('expect', 'screen=title'),
    ]),
    # the keep box left with B, or waited out: the keys as they were
    'a-change-not-kept-without-the-new-a': dict(START=TITLE, STEPS=[
        *KEYBOARD, *on('b', 'also'),
        ('do', 'press A; wait 20; press KEY.M; wait 20; press B; wait 20'),
        ('expect', 'asking=keep'),
        ('do', 'press B; wait 20'),
        ('expect', 'screen=options-game'),
        ('do', 'press A; wait 40'),
        ('repeat', 'press L; wait 6', 'device=keyboard', 2),
        *on('b', 'also'),
        ('expect', 'bound=Z'),
        ('do', 'press A; wait 20; press KEY.M; wait 20; press B; wait 20'),
        ('expect', 'asking=keep'),
        ('until', 'asking=none', 700),            # ten seconds without the new A
        ('expect', 'said=Not kept: as they were'),
        ('restart', TITLE),
        *KEYBOARD, *on('b', 'also'),
        ('expect', 'bound=Z'),
    ]),
    # a box that gets no key gives up by itself: the slot as it was
    'a-slot-asked-and-not-answered': dict(START=TITLE, STEPS=[
        *KEYBOARD, *on('a', 'main'),
        ('expect', 'bound=J'),
        ('do', 'press A; wait 20'),
        ('expect', 'asking=slot'),
        ('until', 'asking=none', 400),
        ('expect', 'said=Not set: as it was'),
        ('expect', 'bound=J'),
    ]),
    # SELECT clears a slot, ALSO moving up to MAIN; a button's last key stays
    'the-last-key-stays': dict(START=TITLE, STEPS=[
        *KEYBOARD, *on('b', 'main'),
        ('expect', 'bound=K'),
        ('do', 'press SELECT; wait 6'),
        ('expect', 'bound=Z'),                    # ALSO's key took MAIN's place
        ('do', 'press SELECT; wait 6'),
        ('expect', 'bound=Z'),
        ('expect', 'said=B keeps one key'),
    ]),
    # a key another button has: taken from it where it keeps another, swapped where it was its only one, and said
    'a-clash-is-taken-or-swapped-and-said': dict(START=TITLE, STEPS=[
        *KEYBOARD, *on('a', 'main'),
        ('do', 'press A; wait 20; press KEY.K; wait 20'),     # B's main key, K: B keeps Z
        ('expect', 'bound=K'),
        ('expect', 'said=Taken from B, which keeps Z'),
        ('do', 'press A; wait 20; press KEY.Q; wait 20'),     # L's only key, Q: L gets A's old one
        ('expect', 'bound=Q'),
        ('expect', 'said=Swapped: L now on K'),
        *on('l', 'main'),
        ('expect', 'bound=K'),
    ]),
    # DEFAULTS puts the tab's keys back as they came
    'defaults': dict(START=TITLE, STEPS=[
        *KEYBOARD, *on('b', 'also'),
        ('do', 'press A; wait 20; press KEY.M; wait 20'),
        ('expect', 'bound=M'),
        ('repeat', 'press DOWN; wait 6', 'button=setall|button=defaults|button=done', 12),
        ('repeat', 'press RIGHT; wait 6', 'button=defaults', 3),
        ('do', 'press A; wait 6'),
        ('expect', 'said=As it came: DONE keeps it'),
        *on('b', 'also'),
        ('expect', 'bound=Z'),
    ]),
    # L and R switch between the controller's tab and the keyboard's
    'tabs': dict(START=TITLE, STEPS=[
        *KEYBOARD,
        ('do', 'press R; wait 6'),
        ('expect', 'device=controller'),
        ('do', 'press L; wait 6'),
        ('expect', 'device=keyboard'),
    ]),
    # a controller (a virtual Xbox pad, used last): its preset turned gives B another button,
    # kept with the pad's new A, and there after a restart (pad.ini)
    'a-controller-preset-kept': dict(START=PAD, STEPS=[
        ('do', 'wait 60; press PAD.dpup; wait 6'),            # the pad used last: Controls opens on its tab
        *OPEN,
        ('expect', 'device=controller'),
        *on('b', 'main'),
        ('expect', 'bound=B'),
        ('repeat', 'press UP; wait 6', 'button=preset', 12),
        ('do', 'press RIGHT; wait 6'),
        *on('b', 'main'),
        ('expect', 'bound=X'),
        ('do', 'press B; wait 20'),
        ('expect', 'asking=keep'),
        ('do', 'press PAD.a; wait 20'),
        ('expect', 'screen=options-game'),
        ('restart', PAD),
        ('do', 'wait 60; press PAD.dpup; wait 6'),
        *OPEN,
        *on('b', 'main'),
        ('expect', 'bound=X'),
    ]),
}
