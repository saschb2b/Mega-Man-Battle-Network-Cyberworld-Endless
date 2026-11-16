"""A button that must not fall through one screen into the next, 0.22.0's
fixes each as the play that showed it: the A after the PET's Save closes
its box and takes nothing off the map, an A pressed through a Net
Dealer's words opens no purchase, and one pressed through a flame of
darkness's words takes no DarkChip. (Mr.Prog's gift taken unseen is
layer-first/gift-waits-to-be-seen.)"""

RUN = 'CYBERWORLD_STATE_POS=1 --scene emu --run-depth 2 --dev quiet'
MIDDLE = f'{RUN} --seed 3'                    # the Net Dealer, a green Mystery Data
FLAME = f'{RUN} --seed 1 --talk dark:700'     # a layer with a flame of darkness, its talk opened at frame 700

ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),                    # the arrival's words paged
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('expect', 'held=none'),
]
PAGED = ('repeat', 'press A; wait 30', 'held=none', 40)         # a chat's boxes to its end, MegaMan free again

MANY = {
    # 0.22.0: the PET's Save closed to the map with a note in the corner, and the A meant for a save box picked up
    # what MegaMan faced (a Guardian Data, once). It ends on a box of its own, and A waits for it.
    'pet-save-keeps-the-next-a': dict(START=MIDDLE, STEPS=ARRIVE + [
        ('do', 'by green; wait 10; press START; wait 60'),      # facing a Mystery Data, the PET opened
        ('expect', 'screen=pet'),
        ('do', 'press UP; wait 20; press A; wait 4'),           # its last row, Save
        ('repeat', 'press A; wait 10', 'talk=director', 8),     # A at once and on: "Run saved." comes up, nothing else
        ('expect', 'greentaken=0'),
        PAGED,
        ('expect', 'greentaken=0'),
        ('do', 'press A; wait 40'),                             # the next A is the map's again: the data he faces
        PAGED,
        ('expect', 'greentaken=1'),
    ]),
    # 0.22.0: with the Net Dealer down to one box, an A pressed at it as at the boxes before landed on his list and
    # opened "... Are you sure? > Yes". A shop opened from a talk takes no A for a second and a half.
    'shop-takes-no-stray-a': dict(START=f'{MIDDLE} --talk zenny:10', STEPS=ARRIVE + [
        ('keep', 'chips'),
        ('do', 'by shop; wait 10'),
        ('repeat', 'press A; wait 10', 'screen=shop', 40),      # a steady A through his words
        ('do', 'press A; wait 10; press A; wait 10; press A; wait 10'),   # ... and three more, onto his list
        ('do', 'wait 60'),
        ('expect', 'screen=shop'),
        ('keep', 'stock'),
        ('expect', 'stock=2'),
        ('same', 'chips'),
        ('do', 'press A; wait 40; press A; wait 60'),           # past the guard, A buys as ever
        ('moved', 'chips', 1),
    ]),
    # 0.22.0: "Take the DarkChip?" stood on Yes, its options up with the question: an A pressed through the flame's
    # words took it. It starts on No.
    'darkchip-not-taken-by-a-stray-a': dict(START=FLAME, STEPS=ARRIVE + [
        ('keep', 'chips hp'),
        ('until', 'chat=open', 600),
        ('repeat', 'press A; wait 20', 'choice=2/2', 40),       # on No as it comes up
        ('repeat', 'press A; wait 20', 'held=none', 40),        # ... and the same A on through it
        ('same', 'chips hp'),
    ]),
    # ... and Yes still takes it
    'darkchip-taken-on-yes': dict(START=FLAME, STEPS=ARRIVE + [
        ('keep', 'chips'),
        ('until', 'chat=open', 600),
        ('repeat', 'press A; wait 20', 'choice=2/2', 40),
        ('repeat', 'press LEFT; wait 10', 'choice=1/2'),
        ('do', 'press A; wait 30'),
        PAGED,
        ('moved', 'chips', 1),
    ]),
}
