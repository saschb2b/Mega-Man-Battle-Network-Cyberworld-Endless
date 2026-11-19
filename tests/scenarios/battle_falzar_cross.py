"""Issue #706: a Falzar Navi fallen as an act's guardian gives his Cross,
as Gregar's five Cross Navis do. SpoutMan guards act 1's arena (the
autopilot's run to his door and his fight); once his Guardian Data is
taken, CROSSSELECT offers SpoutCross (its flag, 0xE7, on), where no Cross
was before."""

# (onehit: a short fight; quiet: no random battle on the way to his arena)
START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --guardian 6 --dev onehit,quiet'

STEPS = [
    ('expect', 'crossesheld=none'),
    ('until', 'battle=guardian', 4000),
    ('expect', 'guardian=SpoutMan fighting'),
    ('until', 'guardian=SpoutMan beaten', 4000),
    ('until', 'crossesheld=SpoutCross', 3000),    # his Cross, by the time his data is taken
    ('until', 'data=taken', 3000),
    ('expect', 'crossesheld=SpoutCross'),
]
