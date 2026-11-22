"""MegaMan's forms (issue #774; docs/game/meta/forms.md): a run of each form
pays a guardian of another game in its own form. CROSS: BN5's ToadMan
gives SpoutCross by his kind, Aqua, never his Soul. SOUL BN5: BN6's
HeatMan, Fire, whose NapalmSoul is not moved yet, gives a Soul the run
lacks, the layer's seed's. SOUL BN4: HeatMan gives BN4's one moved Soul,
ThunSoul, the one the run lacks: the powers line before his data and
after it is taken."""

# (onehit: a short fight; quiet: no random battle on the way to his arena)
BASE = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --dev onehit,quiet,'


def course(form, power):
    return [
        ('expect', f'form={form}'),
        ('expect', 'powers=none'),
        ('until', 'battle=guardian', 4200),
        ('until', 'data=fresh|data=waiting', 4600),
        ('expect', f'powers={power}'),
        ('until', 'data=taken', 2000),
        ('expect', f'powers={power}'),
    ]


MANY = {
    'cross-bn5-guardian': dict(START=BASE + 'form=1 --guardian 29', NEEDS=('bn5',), STEPS=course('CROSS', 'SpoutCross')),
    'soul-bn5-bn6-guardian': dict(START=BASE + 'form=2 --guardian 1', NEEDS=('bn5',), STEPS=course('SOUL BN5', 'ToadSoul')),
    'soul-bn4-bn6-guardian': dict(START=BASE + 'form=3 --guardian 1', NEEDS=('bn4',), STEPS=course('SOUL BN4', 'ThunSoul')),
}
