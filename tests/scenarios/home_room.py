"""A new run begins in Lan's room: Dad's call paged, Lan free to walk, the
run's request board empty, one portal ahead and the first layer behind it
(docs/world/home/central-town.md)."""

# (CYBERWORLD_STATE_POS: the state says what of BN6 holds Lan, and where he stands)
START = 'CYBERWORLD_STATE_POS=1 --scene town --seed 2'

STEPS = [
    ('until', 'chat=open', 600),                    # Dad's call
    ('expect', 'place=Lan\'s Room'),
    ('repeat', 'mash A 60', 'chat=closed'),
    ('until', 'held=none', 600),                    # nothing of BN6 holds him
    ('expect', 'talk=none'),
    ('expect', 'request~none held 0 settled 0'),
    ('expect', 'portals=on 1 back 0'),
    ('expect', 'ahead=layer 1'),
    ('expect', 'hour=morning'),
    ('keep', 'pos'),
    ('do', 'hold LEFT 30; wait 10'),                # he walks
    ('expect', 'pos<-62'),
    ('expect', 'place=Lan\'s Room'),
]
