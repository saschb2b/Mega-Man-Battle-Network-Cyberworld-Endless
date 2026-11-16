"""The Arena from the Net Square (docs/online/arena/): a Practice NetBattle
asked of another player's MegaMan, taken, fought to its end by both games,
its result on both cards and in each game's record; a challenge turned
down and one called off; and a player whose game closes mid-battle, the
other's card saying so and his own run going on."""
import scenarios_online as online
from scenarios_online import soon

NEEDS = ('bn5', 'bn4')   # (a NetBattle takes the NetPass's full set on both games)
# (arenaboth: a game takes a challenge at once and plays its battle by script, charged shots; arenahp: its own MegaMan's
# HP. The scripts are the same on both sides and the link's timing decides between equals (9 to 0 one run, 0 to 4 the
# next), so Alpha comes with HP no script's shots take and Beta with one shot's)
FIGHT = online.HOME + ',arenaboth,arenadelay=6,arenahp=200'
FIGHT_WEAK = online.HOME + ',arenaboth,arenadelay=6,arenahp=5'
CAST = dict(NEEDS=NEEDS, SERVER=True, FILES={'netpass.ini': online.SEEN_FULL})
# (both in the plaza, Alpha within A's reach of Beta and facing him: BN6's talk probe reaches 8 ahead, a peer within 24 of it)
MEET = online.course(
    online.into_square(),
    online.into_square('beta'),
    soon('peers~Beta'),
    ('@beta', 'do', 'place 116 -52 5; wait 10'),
    ('do', 'place 92 -52 1; wait 10'),
    soon('peer~Beta at 116 -52'),
)
# (his card paged to its options, Battle the first: the challenge)
CHALLENGE = [('repeat', 'press A; wait 10', 'choice=1/3', 20), *online.pick('challenge~to Beta|visit=battle')]

MANY = {
    'netbattle': dict(CAST, START=FIGHT, GAMES={'beta': FIGHT_WEAK}, STEPS=online.course(
        MEET, CHALLENGE,
        soon('visit=battle', '', 600),
        soon('visit=battle', 'beta', 600),
        soon('arena=battle', '', 900),             # the way in: both PETs linked, the Custom screen
        ('expect', 'peer~Beta at 116 -52 in plaza battle'),
        soon('arena=over', '', 6000),              # fought to its end by both games' scripts
        ('expect', 'arenacard=won'),
        ('expect', 'arenahp>=1'),                   # his own HP left, then the other's: "196 0"
        ('expect', 'arenahp~ 0'),
        ('expect', 'rematch=open no'),             # asked on the card, the cursor on No
        soon('arena=over', 'beta', 600),
        ('@beta', 'expect', 'arenacard=lost'),
        ('@beta', 'expect', 'arenahp<=0'),            # "0 196": his own, then the other's
        ('repeat', 'press B; wait 10', 'visit=square', 20),
        ('expect', 'bout=won against Beta won 1 lost 0'),
        ('expect', 'bouts=1'),
        ('expect', 'squarex=92'),                  # back where he stood
        ('@beta', 'repeat', 'press B; wait 10', 'visit=square', 20),
        ('@beta', 'expect', 'bout=lost against Alpha won 0 lost 1'),
    )),
    'challenge-turned-down': dict(CAST, START=online.HOME, GAMES={'beta': online.HOME}, STEPS=online.course(
        MEET, CHALLENGE,
        soon('challenge~to Beta'),
        soon('challenge~from Alpha', 'beta'),
        ('@beta', 'repeat', 'press A; wait 10', 'choice~/2', 20),   # "Alpha's MegaMan wants a Practice NetBattle!", paged
        ('@beta', 'repeat', 'wait 4', 'choice=2/2', 90),            # "Take it?", the cursor on No
        ('@beta', 'do', 'press A; wait 10'),
        soon('challenge=none', 'beta'),
        soon('challenge=none'),
        ('repeat', 'press A; wait 10', 'chat=closed', 20),   # MegaMan's word on it
        ('expect', 'arena=idle'),
        ('expect', 'bouts=0'),
        ('expect', 'visit=square'),
        ('@beta', 'expect', 'visit=square'),
        ('@beta', 'expect', 'bouts=0'),
    )),
    'challenge-called-off': dict(CAST, START=online.HOME, GAMES={'beta': online.HOME}, STEPS=online.course(
        MEET, CHALLENGE,
        soon('challenge~to Beta'),
        soon('challenge~from Alpha', 'beta'),
        ('do', 'wait 30'),
        ('repeat', 'press A; wait 10', 'choice=1/3', 20),    # his card again: Call off in Battle's place
        online.pick('challenge=none'),
        soon('challenge=none'),
        soon('challenge=none', 'beta'),
        ('@beta', 'repeat', 'press A; wait 10', 'chat=closed', 20),
        ('@beta', 'expect', 'arena=idle'),
        ('expect', 'bouts=0'),
    )),
    'leaves-mid-battle': dict(CAST, START=FIGHT, GAMES={'beta': FIGHT}, STEPS=online.course(
        online.paged('beta'),
        ('@beta', 'keep', 'hp zenny spares place'),
        MEET, CHALLENGE,
        soon('arena=battle', '', 900),
        soon('arena=battle', 'beta', 900),
        ('do', 'wait 120'),
        # (Beta's game closed in the fight, as his system closes it; then started again: CONTINUE at the gate's checkpoint)
        ('@beta', 'restart', '--scene title'),
        soon('arena=over', '', 1200),
        ('expect', 'arenacard=stopped'),
        ('repeat', 'press B; wait 10', 'visit=square', 20),
        ('expect', 'bout~stopped'),
        soon('peers=0', '', 600),
        ('@beta', 'repeat', 'press START; wait 40', 'title=menu'),
        ('@beta', 'expect', 'cursor=continue'),
        ('@beta', 'repeat', 'press A; wait 40', 'scene=emu'),
        ('@beta', 'until', 'where=town', 1200),
        ('@beta', 'repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('@beta', 'same', 'hp zenny spares place'),
        ('@beta', 'expect', 'visit=home'),
    )),
    'server-gone-mid-battle': dict(CAST, START=FIGHT, GAMES={'beta': FIGHT}, STEPS=online.course(
        online.paged(),
        ('keep', 'hp zenny spares place'),
        MEET, CHALLENGE,
        soon('arena=battle', '', 900),
        soon('arena=battle', 'beta', 900),
        ('do', 'wait 60'),
        ('server', 'stop'),
        soon('arena=over', '', 1800),              # the battle stopped, its card saying so
        ('expect', 'arenacard=stopped'),
        ('repeat', 'press B; wait 10', 'visit=waiting|visit=home', 20),
        ('repeat', 'press B; wait 10', 'visit=home', 6),   # the Square's link gone too: B, home
        ('expect', 'visitended=lost'),
        ('repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('same', 'hp zenny spares place'),
    )),
}
