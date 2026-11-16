"""A run's first layer: the arrival's words, Mr.Prog's gift picked once
(HPMemory, a chip or a program, each given as its card says, none taken by
an A pressed through his lines), the bystander who names the act's
guardian, and L."""

LAYER = 'CYBERWORLD_STATE_POS=1 --scene emu --seed 3 --dev quiet'

# (MegaMan's and Lan's words on arriving, then Mr.Prog's, an A a box: his
# gift's screen opens by itself after them)
TO_GIFT = [
    ('until', 'talk=director', 600),
    ('repeat', 'press A; wait 20', 'pick=1/3', 40),
    ('expect', 'screen=reward-pick'),
    ('keep', 'hp chips programs'),
]
PAGED = ('repeat', 'press A; wait 30', 'held=none')             # a chat's boxes to its end, MegaMan free again


def gift(kind, got):
    """The gift's row of `kind` taken once A takes: its "MegaMan got:" paged,
    `got` held; then Mr.Prog gone, an A where he stood giving nothing."""
    return TO_GIFT + [
        ('repeat', 'press DOWN; wait 10', f'pickof={kind}', 4),
        ('until', 'picktakes=yes', 300),
        ('do', 'press A; wait 30'),
        ('until', 'chat=open', 300),
        PAGED,
        got,
        ('keep', 'hp chips programs'),
        ('do', 'by gift; wait 10; press A; wait 60'),
        PAGED,
        ('same', 'hp chips programs'),
    ]


MANY = {
    'gift-hpmemory': dict(START=LAYER, STEPS=gift('hp', ('expect', 'hp>100'))),
    'gift-chip': dict(START=LAYER, STEPS=gift('chip', ('moved', 'chips', 1))),
    'gift-program': dict(START=LAYER, STEPS=gift('program', ('moved', 'programs', 1))),
    # 0.22.0: A pressed at a steady pace through Mr.Prog's lines landed on his
    # gift's screen just after it opened and took its first choice unseen. A
    # takes nothing until the screen has been up a second and a half.
    'gift-waits-to-be-seen': dict(START=LAYER, STEPS=TO_GIFT + [
        ('expect', 'picktakes=no'),
        ('do', 'press A; wait 20; press A; wait 20'),           # the same pace, two more: 64 frames
        ('expect', 'screen=reward-pick'),
        ('same', 'hp chips programs'),
        ('until', 'picktakes=yes', 300),
        ('do', 'press A; wait 30'),
        ('until', 'chat=open', 300),
        PAGED,
        ('expect', 'hp>100'),
    ]),
    # the act's first bystander has heard who guards it: the card and the PET name him from then on
    'bystander-names-the-guardian': dict(START=LAYER, STEPS=gift('hp', ('expect', 'hp>100')) + [
        ('expect', 'heard=no'),
        ('do', 'by npc; wait 10; press A; wait 40'),
        ('expect', 'chat=open'),
        PAGED,
        ('expect', 'heard=yes'),
    ]),
    # L: MegaMan says the way, and is free again
    'l-says-the-way': dict(START=LAYER, STEPS=gift('hp', ('expect', 'hp>100')) + [
        ('do', 'press L; wait 40'),
        ('expect', 'talk=director'),
        PAGED,
        ('expect', 'talk=none'),
    ]),
}
