"""The keepers on a layer, from the first A to what MegaMan is left with:
the Net Dealer and the NaviCust vendor sell for the price shown and refuse
an empty purse, the Recovery Mr.Prog's one patch heals once, kept where
MegaMan says No or is not hurt, and a real patch after a CONTINUE too."""

# (an act's middle layer: the Net Dealer, the vendor and the Prog all stand
# on it; MegaMan put beside each, as a player walks up, and A pressed)
LAYER = '--scene emu --seed 3 --run-depth 2 --dev quiet'

ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),                    # the arrival's words paged
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('expect', 'held=none'),
]


# (the row under the cursor, "Are you sure? > Yes", and "I bought ..." closed)
BUY = 'press A; wait 40; press A; wait 60; press A; wait 60'


def counter(keeper, purse):
    """Up to `keeper`'s list: A at him, his words paged, BN6's shop screen
    up and past the second and a half it takes no A for; MegaMan holding
    `purse` (a state line's word) to pay with."""
    return ARRIVE + [
        ('keep', f'{purse} chips programs'),
        ('do', f'by {keeper}; wait 10'),
        ('repeat', 'press A; wait 30', 'screen=shop'),
        ('do', 'wait 100'),
        ('keep', 'price stock shoprows'),
    ]


# (hp=100/40 holds MegaMan at 40 HP through the run's first 600 frames: hurt
# as he reaches the Prog)
HURT = f'CYBERWORLD_STATE_POS=1 {LAYER.replace("quiet", "quiet,hp=100/40")}'
TO_PROG = ARRIVE + [
    ('until', 'frame>=620', 700),
    ('expect', 'hp=40/100'),
    ('do', 'by heal; wait 10'),
]
PAGED = ('repeat', 'press A; wait 30', 'held=none')             # a chat's boxes to its end, MegaMan free again


def leave():
    return [('repeat', 'press B; wait 40', 'screen=map'), ('until', 'held=none', 600)]


MANY = {
    # the first row bought: one fewer in stock, the chip in the Pack, the zenny down by its price
    'dealer-sells': dict(START=f'CYBERWORLD_STATE_POS=1 {LAYER} --talk zenny:10', STEPS=counter('shop', 'zenny') + [
        ('expect', 'stock>=1'),
        ('do', 'press A; wait 40; press A; wait 60'),           # the row, then "Are you sure? > Yes"
        ('moved', 'stock', -1),
        ('moved', 'chips', 1),
    ] + leave() + [
        ('moved', 'zenny', '-price'),
        ('moved', 'chips', 1),
    ]),
    # ... and bought out: the dealer keeps two of his first chip, then its row is gone
    'dealer-sells-out': dict(START=f'CYBERWORLD_STATE_POS=1 {LAYER} --talk zenny:10', STEPS=counter('shop', 'zenny') + [
        ('expect', 'stock=2'),
        ('do', BUY),
        ('moved', 'stock', -1),
        ('do', BUY),
        ('moved', 'shoprows', -1),
        ('moved', 'chips', 2),
    ] + leave()),
    # no zenny: the dealer says so, and nothing changes hands
    'dealer-refuses': dict(START=f'CYBERWORLD_STATE_POS=1 {LAYER}', STEPS=counter('shop', 'zenny') + [
        ('expect', 'zenny=0'),
        ('do', 'press A; wait 40; press A; wait 60; press A; wait 60'),
        ('same', 'stock chips'),
    ] + leave() + [
        ('same', 'zenny chips'),
    ]),
    # the NaviCust vendor's first program, for BugFrags; MegaMan says it is not on the board yet
    'vendor-sells': dict(START=f'CYBERWORLD_STATE_POS=1 {LAYER} --talk bugfrags:10', STEPS=counter('programs', 'bugfrags') + [
        ('do', 'press A; wait 40; press A; wait 60'),
        ('moved', 'programs', 1),
        ('repeat', 'press B; wait 40', 'screen=map'),
        ('until', 'talk=director', 300),
        ('repeat', 'press A; wait 30', 'held=none'),
        ('moved', 'bugfrags', '-price'),
        ('moved', 'programs', 1),
    ]),
    'vendor-refuses': dict(START=f'CYBERWORLD_STATE_POS=1 {LAYER}', STEPS=counter('programs', 'bugfrags') + [
        ('expect', 'bugfrags=0'),
        ('do', 'press A; wait 40; press A; wait 60; press A; wait 60'),
        ('same', 'programs'),
    ] + leave() + [
        ('same', 'bugfrags programs'),
    ]),
    # the Recovery Mr.Prog: his question's Yes heals, and the layer's one patch is spent
    'prog-heals-once': dict(START=HURT, STEPS=TO_PROG + [
        ('repeat', 'press A; wait 30', 'hp>40'),
        PAGED,
        ('expect', 'hp>60'),                                    # a patch, half his max HP as built: not a token
        ('keep', 'hp'),
        ('do', 'press A; wait 40'),
        ('expect', 'chat=open'),                                # he says the patch is spent
        PAGED,
        ('same', 'hp'),
    ]),
    # ... No keeps it: no HP now, the patch still there for a Yes
    'prog-declined': dict(START=HURT, STEPS=TO_PROG + [
        ('repeat', 'press A; wait 30', 'choice=1/2'),           # his greeting, then "USE IT NOW? > Yes  No"
        ('do', 'press RIGHT; wait 10'),
        ('expect', 'choice=2/2'),
        ('do', 'press A; wait 30'),
        PAGED,
        ('expect', 'hp=40/100'),
        ('repeat', 'press A; wait 30', 'hp>40'),
        PAGED,
    ]),
    # ... and at full HP he sends MegaMan off: nothing asked, nothing spent
    'prog-not-hurt': dict(START=f'CYBERWORLD_STATE_POS=1 {LAYER}', STEPS=ARRIVE + [
        ('expect', 'hp=100/100'),
        ('do', 'by heal; wait 10; press A; wait 40'),
        ('expect', 'chat=open'),
        PAGED,
        ('expect', 'hp=100/100'),
    ]),
    # 0.22.0: on a layer continued from the title the Prog asked for the layer's one patch, healed 1 HP and called
    # it spent (the layer was built before the state held MegaMan's max HP). It heals as on any layer.
    'prog-heals-after-a-continue': dict(START=HURT, STEPS=ARRIVE + [
        ('until', 'frame>=620', 700),
        ('expect', 'hp=40/100'),
        ('restart', 'CYBERWORLD_STATE_POS=1 --scene title'),
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('expect', 'cursor=continue'),
        ('repeat', 'press A; wait 40', 'scene=emu'),
        ('until', 'screen=map', 1200),
        ('until', 'held=none', 600),
        ('expect', 'hp=40/100'),
        ('do', 'by heal; wait 10'),
        ('repeat', 'press A; wait 30', 'hp>40'),
        PAGED,
        ('expect', 'hp>60'),
    ]),
}
