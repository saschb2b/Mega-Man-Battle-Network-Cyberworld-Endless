"""Issue #733: a form of the Glitch layer's copy taken off takes its own
objects with it. DiveMan's torpedo launcher, bound to him from the
battle's first frame, fired on under JudgeMan; now it ends as DiveMan
glitches out, and nothing of his is left at work under the next form
(docs/game/glitch-layer.md, A form's leftovers; the swap step of
docs/devtools/battle-on-demand.md)."""

# (the autopilot fights; god keeps MegaMan up, DiveMan's share small)
START = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 15 --net-biome 0 --dev god,quiet'

STEPS = [
    ('do', 'wait 300; swap 149:200 155:3000'),        # DiveMan V1, then JudgeMan V1
    ('until', 'swapform=1', 3000),
    ('expect', 'leftovers=0'),
    ('until', 'swapform=2', 12000),                   # DiveMan deleted, JudgeMan on his panel
    ('expect', 'leftovers=0'),                        # his torpedo launcher ended with him
    ('do', 'wait 300'),
    ('expect', 'swapform=2'),
    ('expect', 'leftovers=0'),                        # and nothing of his at work since
]
