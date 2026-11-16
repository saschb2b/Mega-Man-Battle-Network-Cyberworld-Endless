"""The first start's question on the anonymous statistics (issue #104): asked
once on a fresh data folder, the answer written, held by the next start and
not asked again, shown on Options' Statistics line and changed there. Nothing
leaves the machine: a session that asks sends nothing, and the next start's
only address is this machine's own discard port, where nothing listens
(docs/devtools/statistics.md)."""

ASK = '--scene title --dev statistics=ask'
# (the next start holds the answer the first wrote: statsurl makes it a
# session that may, and what it would send goes to 127.0.0.1:9, never Umami)
AGAIN = '--scene title --dev statsurl=http://127.0.0.1:9/api/send'

LINE = [   # Options' Game page, its Statistics line
    ('repeat', 'press SELECT; wait 40', 'screen~options'),
    ('repeat', 'press L; wait 20', 'screen=options-game'),
    ('repeat', 'press DOWN; wait 6', 'line=statistics'),
]


def asked(press, answer, value):
    """The question answered with `press`: the answer kept, and not asked again."""
    return [
        ('expect', 'screen=statistics-question'),
        ('expect', 'statistics=unasked'),
        ('until', 'answer=no', 120),              # no press taken for half a second; the cursor on No
        ('do', press),
        ('until', 'screen=title', 120),
        ('expect', f'statistics={answer}'),
        ('restart', AGAIN),
        ('do', 'wait 120'),
        ('expect', 'screen=title'),               # not asked again
        ('expect', f'statistics={answer}'),
        *LINE,
        ('expect', f'value={value}'),
    ]


MANY = {
    'yes': dict(START=ASK, STEPS=asked('press LEFT; wait 6; press A', 'on', 'On')),
    'no': dict(START=ASK, STEPS=asked('press A', 'off', 'Off')),
    'b-is-no': dict(START=ASK, STEPS=asked('press B', 'off', 'Off')),
    'start-takes-the-answer-under-the-cursor': dict(START=ASK, STEPS=asked('press START', 'off', 'Off')),
    # Options' line opens the question again, the cursor on the answer held:
    # B leaves it as it was, the other answer is kept
    'changed-in-options': dict(START=ASK, STEPS=[
        *asked('press A', 'off', 'Off'),
        ('do', 'press A; wait 10'),
        ('until', 'answer=no', 120),
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'answer=yes'),
        ('do', 'press B; wait 20'),               # B: nothing changed
        ('expect', 'screen=options-game'),
        ('expect', 'statistics=off'),
        ('do', 'press A; wait 10'),
        ('until', 'answer=no', 120),
        ('do', 'press LEFT; wait 6; press A; wait 20'),
        ('expect', 'statistics=on'),
        ('expect', 'value=On'),
        ('restart', AGAIN),
        ('do', 'wait 120'),
        ('expect', 'screen=title'),
        ('expect', 'statistics=on'),
    ]),
    # a start that is no player's (a scene, a seed, headless) neither asks nor holds an answer
    'a-test-start-asks-nothing': dict(START='--scene title', STEPS=[
        ('do', 'wait 120'),
        ('expect', 'screen=title'),
        ('repeat', 'press SELECT; wait 40', 'screen~options'),
        ('repeat', 'press L; wait 20', 'screen=options-game'),
        ('expect', 'line=controls'),              # the page's lines: Controls and Fast forward, no Statistics
        ('do', 'press DOWN; wait 6'),
        ('expect', 'line=fast'),
        ('do', 'press DOWN; wait 6'),
        ('expect', 'line=controls'),
    ]),
}
