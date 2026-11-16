"""What a layer keeps behind a question or a record: a Navi gate sealed
until his code is earned, a collector's vault shut until the Library
opens it and paying one chip of three, an official gate, a Server's
offer and ProtoMan's terms (each asked on No: No keeps the map, Yes is
the battle), the Chip Trader's three for one and the BugFrag Trader's
ten for one, a ScrtData, and the golden gate that three of them open."""

# (--dev last: a scenario adds its own switch to it, ",veteran")
RUN = 'CYBERWORLD_STATE_POS=1 --scene emu'
GATE = f'{RUN} --seed 4 --run-depth 7 --dev quiet'        # HeatMan's gate; a veteran's record has deleted him twice
VAULT = f'{RUN} --seed 8 --run-depth 4 --dev quiet'       # a collector's vault; library=1 fills the Library
OFFICIAL = f'{RUN} --seed 3 --run-depth 2 --dev quiet'    # ProtoMan and his official gate
SERVER = f'{RUN} --seed 1 --run-depth 3 --dev quiet'      # a Server's strong virus signal
TRADER = f'{RUN} --seed 5 --run-depth 4 --dev quiet'      # a Chip Trader; pack=6 puts six spare chips in the Pack
SCRTDATA = f'{RUN} --seed 1 --run-depth 4 --dev quiet'    # a blue Mystery Data that holds a ScrtData
UNDERNET = f'{RUN} --seed 3 --run-depth 5 --dev quiet,side=1'   # an Undernet layer: the BugFrag Trader, the golden gate
HOLDS = 'hp zenny bugfrags chips programs'
# (found by this scenario, 16 November 2026: 0.20.0's build takes the three, 0.21.0's and this one none or one)
TRADE_BUG = "the Chip Trader gives its chip and keeps the three put in: a trade leaves one chip fewer or none, not two (since 0.21.0)"

ARRIVE = [
    ('do', 'wait 150; mash A 300; wait 20'),                    # the arrival's words paged
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
    ('expect', 'held=none'),
    ('keep', HOLDS),
]
PAGED = ('repeat', 'press A; wait 30', 'held=none', 30)         # a chat's boxes to its end, MegaMan free again


def words_only(start, what, *first):
    """A at `what`: its words, and nothing changed."""
    return dict(START=start, STEPS=ARRIVE + list(first) + [
        ('do', f'by {what}; wait 10; press A; wait 40'),
        ('expect', 'chat=open'),
        PAGED,
        ('expect', 'doing=map'),
        ('same', HOLDS),
    ])


def asked(start, what, answer):
    """`what`'s question, its cursor on No as it comes up; No: the map, and
    nothing changed; Yes: its battle."""
    steps = ARRIVE + [
        ('do', f'by {what}; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=2/2', 30),       # on No: an A pressed through the words takes nothing
    ]
    if answer == 'no':
        return dict(START=start, STEPS=steps + [('do', 'press A; wait 30'), PAGED, ('expect', 'doing=map'), ('same', HOLDS)])
    return dict(START=start, STEPS=steps + [
        ('do', 'press LEFT; wait 10'),
        ('expect', 'choice=1/2'),
        ('do', 'press A; wait 30'),
        ('until', 'screen=battle', 1800),
    ])


MANY = {
    # a Navi gate before his code is earned: MegaMan says whose it is and how it opens
    'navi-gate-sealed': words_only(GATE, 'navigate', ('expect', 'gate~sealed')),
    # ... earned (he was deleted twice): it asks for his SP
    'navi-gate-declined': asked(f'{GATE},veteran', 'navigate', 'no'),
    'navi-gate-opens': asked(f'{GATE},veteran', 'navigate', 'yes'),
    # a collector's vault and a small Library: shut
    'vault-shut': words_only(VAULT, 'vault'),
    # ... a Library that opens it: one of its three chips taken, and the vault is empty after
    'vault-pays-one-chip': dict(START=f'{VAULT},library=1', STEPS=ARRIVE + [
        ('do', 'by vault; wait 10'),
        ('repeat', 'press A; wait 30', 'pick=1/3', 30),
        ('expect', 'pickof=chip'),
        ('until', 'picktakes=yes', 300),
        ('do', 'press A; wait 30'),
        ('until', 'chat=open', 300),
        PAGED,
        ('moved', 'chips', 1),
        ('keep', 'chips'),
        ('do', 'by vault; wait 10; press A; wait 60'),
        PAGED,
        ('expect', 'screen=map'),
        ('same', 'chips'),
    ]),
    # ... B leaves it shut and whole: nothing taken, the pick there again at the next A
    'vault-left-keeps': dict(START=f'{VAULT},library=1', STEPS=ARRIVE + [
        ('do', 'by vault; wait 10'),
        ('repeat', 'press A; wait 30', 'pick=1/3', 30),
        ('until', 'picktakes=yes', 300),
        ('do', 'press B; wait 60'),
        PAGED,                                                  # MegaMan's word on leaving it
        ('expect', 'screen=map'),
        ('same', 'chips'),
        ('do', 'by vault; wait 10'),
        ('repeat', 'press A; wait 30', 'pick=1/3', 30),
    ]),
    # an official gate while ProtoMan is unbeaten here: its words, nothing given
    'official-gate-shut': words_only(OFFICIAL, 'official'),
    # a Server's strong virus signal: its offer on No
    'server-declined': asked(SERVER, 'challenge', 'no'),
    'server-taken': asked(SERVER, 'challenge', 'yes'),
    # ProtoMan's terms
    'protoman-declined': asked(OFFICIAL, 'duel', 'no'),
    'protoman-accepted': asked(OFFICIAL, 'duel', 'yes'),
    # the Chip Trader and an empty Pack: it takes three, MegaMan has none to spare
    'trader-needs-three': dict(START=TRADER, STEPS=ARRIVE + [
        ('do', 'by trader; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=1/2'),
        ('do', 'press A; wait 30'),
        PAGED,
        ('expect', 'screen=map'),
        ('same', 'chips'),
    ]),
    # ... six spare: three in, one out
    'trader-trades-three-for-one': dict(KNOWN=TRADE_BUG, START=f'{TRADER},pack=6', STEPS=ARRIVE + [
        ('do', 'by trader; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=1/2'),
        ('do', 'press A; wait 30'),
        ('until', 'doing=menu', 300),                           # BN6's trader screen, the Pack's chips listed
        ('do', 'wait 60'),
        ('repeat', 'press A; wait 40', 'choice=1/2'),           # three chips marked, then "Is this OK? > Yes"
        ('do', 'press A; wait 30'),
        ('until', 'doing=map', 900),
        ('repeat', 'press A; wait 30', 'choice=1/2'),           # the chip it gives, named; then it asks again
        ('repeat', 'press RIGHT; wait 10', 'choice=2/2'),       # "Try again? Yes > No"
        ('do', 'press A; wait 30'),
        PAGED,
        ('moved', 'chips', -2),
    ]),
    # a ScrtData in a blue Mystery Data: counted, and MegaMan says what it is for
    'scrtdata-found': dict(START=SCRTDATA, STEPS=ARRIVE + [
        ('expect', 'scrtdata=0'),
        ('do', 'by scrtdata; wait 10; press A; wait 40'),
        ('expect', 'chat=open'),
        PAGED,
        ('expect', 'scrtdata=1'),
    ]),
    # the BugFrag Trader with none: its Yes trades nothing
    'bugfrag-trader-needs-ten': dict(START=UNDERNET, STEPS=ARRIVE + [
        ('expect', 'bugfrags=0'),
        ('do', 'by bugtrader; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=1/2'),
        ('do', 'press A; wait 30'),
        PAGED,
        ('expect', 'screen=map'),
        ('same', 'bugfrags chips'),
    ]),
    # ... with fifty: ten in, a chip out; it asks again, and No leaves
    'bugfrag-trader-trades-ten-for-one': dict(START=f'{UNDERNET} --talk bugfrags:10', STEPS=[('do', 'wait 60')] + ARRIVE + [
        ('expect', 'bugfrags=50'),
        ('do', 'by bugtrader; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=1/2'),
        ('do', 'press A; wait 60'),
        ('moved', 'bugfrags', -10),
        ('moved', 'chips', 1),
        ('repeat', 'press A; wait 30', 'choice=1/2'),           # the chip it gives, named; then it asks again
        ('repeat', 'press RIGHT; wait 10', 'choice=2/2'),
        ('do', 'press A; wait 30'),
        PAGED,
        ('moved', 'bugfrags', -10),
        ('moved', 'chips', 1),
    ]),
    # the golden gate without three ScrtData: MegaMan's words, and it stays
    'golden-gate-wants-three': words_only(UNDERNET, 'gate', ('expect', 'scrtdata=0')),
    # ... with three: it asks, and Yes is the Secret Area, its guardian waiting
    'golden-gate-opens': dict(START=f'{UNDERNET} --talk scrtdata:10', STEPS=[('do', 'wait 60')] + ARRIVE + [
        ('expect', 'scrtdata=3'),
        ('expect', 'area=the Undernet'),
        ('do', 'by gate; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=1/2'),
        ('do', 'press A; wait 30'),
        ('until', 'area=the Secret Area', 900),
        ('until', 'held=none', 900),
        ('expect', 'guardian~waiting'),
    ]),
}
