"""The threat rungs, where the state tells them apart from a plain run
(docs/game/meta/): EX guardians from the fourth, a draft of two programs
from the eighth, four HPMemory a guardian from the ninth, and on the
tenth a second guardian below the short net's Nest, the first one's pad
leading down, not to the win."""


def start(threat, depth, auto='1', dev='onehit,quiet'):
    return f'CYBERWORLD_AUTOPILOT={auto} --scene emu --seed 3 --run-depth {depth} --guardian 1 --setup short,0,{threat},0 --dev {dev}'


def data(hp, rows):
    """An act 1 guardian's Guardian Data: MegaMan's HP after it, the draft's rows (its programs and the BugFrags')."""
    return [
        ('until', 'battle=guardian', 4000),
        ('until', 'screen=reward-pick', 6000),
        ('expect', 'pick=draft'),
        ('expect', f'hp={hp}/{hp}'),
        ('expect', f'pickrows={rows}'),
        ('expect', 'offers~bugfrags'),
    ]


MANY = {
    'plain/data': dict(START=start(0, 3), STEPS=data(200, 4)),
    'rung8/draft-of-two': dict(START=start(8, 3), STEPS=data(200, 3)),
    'rung9/four-hpmemory': dict(START=start(9, 3), STEPS=data(180, 3)),
    # (act 2's guardian: V1 in a plain run, EX from the fourth rung)
    'plain/version': dict(START=start(0, 6), STEPS=[('expect', 'version=0')]),
    'rung3/version': dict(START=start(3, 6), STEPS=[('expect', 'version=0')]),
    'rung4/ex-guardians': dict(START=start(4, 6), STEPS=[('expect', 'version=1')]),
    'rung10/second-guardian': dict(START=start(10, 10, 'weak', 'quiet'), STEPS=[
        ('expect', 'area=the Cybeast Nest'),
        ('until', 'battle=guardian', 6000),
        ('until', 'exitpad=open', 6000),
        ('until', 'layer=11', 2400),          # his pad leads down
        ('expect', 'scene=emu'),
        ('expect', 'area=the Cybeast Nest'),
        ('expect', 'exitpad=shut'),           # the second stands
        ('expect', 'navis=1'),
    ]),
}
