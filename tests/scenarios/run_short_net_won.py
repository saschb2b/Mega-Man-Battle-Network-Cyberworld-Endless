"""A short net won: the Nest's guardian deleted on layer 10, his exit pad
the run's end, the title's summary of a win and what it opened (the
endless net, the first threat rung, folders), no run left to continue,
and NEW GAME opening the setup, which a first run never sees."""

# (the weak autopilot ends the Nest's guardian's fight)
START = 'CYBERWORLD_AUTOPILOT=weak --scene emu --seed 3 --run-depth 10 --setup short --dev quiet'

STEPS = [
    ('expect', 'area=the Cybeast Nest'),
    ('until', 'battle=guardian', 6000),
    ('until', 'exitpad=open', 6000),
    ('expect', 'navis=1'),
    ('until', 'scene=title', 2400),           # his pad: the run is won
    ('expect', 'title=summary'),
    ('expect', 'page=result'),
    ('expect', 'ended=won'),
    ('expect', 'cause=on layer 10'),
    ('expect', 'reached=10'),
    ('expect', 'navis=1'),
    ('repeat', 'press A; wait 40', 'page=records'),
    ('expect', 'opened~the endless net'),
    ('expect', 'opened~threat 1'),
    ('repeat', 'press A; wait 40', 'title=start'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=new'),                 # won: nothing to continue
    ('repeat', 'press A; wait 40', 'title=setup'),
]
