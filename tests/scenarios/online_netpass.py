"""The NetPass (docs/online/netpass.md): what a game's ROMs open online, as
the pass says it and as a run's start reads it: BN6's alone the Square,
BN5's beside it still the Square (BN4's slot open), the set of three the
full pass and its four stamps; and the pass's screen opened over the game."""

HOME = '--scene home --run-depth 7 --seed 3 --dev quiet'


def level(needs, word, stamps):
    return dict(START=HOME, NEEDS=needs, STEPS=[
        ('do', 'wait 300'),
        ('repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('expect', f'netpass={word}'),
        ('expect', f'stamps={stamps}'),
        ('do', 'netpass; wait 30'),                    # the pass opened (the dev step, as the PET's entry opens it)
        ('expect', 'screen=netpass'),
        ('repeat', 'press B; wait 20', 'screen=town', 6),
    ])


MANY = {
    'bn6-alone': level((), 'square', 1),
    'bn6-and-bn5': level(('bn5',), 'square', 1),
    'the-set': level(('bn5', 'bn4'), 'full', 4),
}
