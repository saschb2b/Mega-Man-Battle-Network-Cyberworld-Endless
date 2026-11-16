"""Deleted by a guardian: the title's summary names him and where, and
that first fight's battle data is learned; the next run knows him from
its first frame (named in the state, where a first profile reads a
strong signal, "???"), and a second deletion by him teaches nothing
new."""

# (fragile: his first hit deletes MegaMan; quiet: no battle on the way)
START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --guardian 1 --dev fragile,quiet'

# from a guardian's fight to the title, the summary's two pages read
LOST = [
    ('until', 'battle=guardian', 4000),
    ('until', 'scene=title', 8000),
    ('expect', 'title=summary'),
    ('expect', 'ended=deleted'),
    ('expect', 'cause=by HeatMan in Seaside Area'),
    ('expect', 'navis=0'),
    ('expect', 'reached=3'),
]

STEPS = [
    ('expect', 'guardian=??? waiting'),
    ('expect', 'known=no'),
    *LOST,
    ('expect', 'learned=HeatMan'),            # the first fight: his battle data
    ('repeat', 'press A; wait 40', 'page=records'),
    ('repeat', 'press A; wait 40', 'title=start'),
    # the next run, dropped in before his arena again
    ('restart', 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --guardian 1 --dev fragile,quiet'),
    ('expect', 'guardian=HeatMan waiting'),   # known: named before he is met
    ('expect', 'known=yes'),
    *LOST,
    ('expect', 'learned=none'),               # nothing new the second time
]
