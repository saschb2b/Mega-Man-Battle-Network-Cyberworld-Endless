"""Team NetBattles from the Net Square (docs/online/arena/teams.md,
docs/online/arena/two-a-side/over-the-server.md): two teams of two formed
on each other's cards, a team challenge fought side by side, all four
games on one field, and one player's link dropped in the middle of it
(issue #180, docs/online/arena/dropped-link.md): the server holds the
team match and its battle with his place, every game waits on his keys,
and the battle is fought to its end once he is back, the match's result
on all four games."""
import scenarios_online as online
from scenarios_online import soon

NEEDS = ('bn5', 'bn4')   # (a NetBattle takes the NetPass's full set on every game)
# (arenaboth: a game takes a challenge at once and plays its battle by script: the leads' keys the bot's, the
# partners' their stood-in script's; Alpha's team with HP no script's shots take, Gamma's with one shot's)
STRONG = online.HOME + ',arenaboth,arenadelay=6,arenahp=200'
WEAK = online.HOME + ',arenaboth,arenadelay=6,arenahp=5'
CAST = dict(NEEDS=NEEDS, SERVER=True, FILES={'netpass.ini': online.SEEN_FULL})


def team_up(asker, asked, name):
    """`asker` (facing `asked`) asks him to team up on his card (its second option); `asked` says Yes."""
    return online.course(
        online.at(asker, ('repeat', 'press A; wait 10', 'choice=1/3', 20), ('do', 'press DOWN; wait 6')),
        online.at(asker, ('repeat', 'wait 4', 'choice=2/3', 30)),
        online.at(asker, ('do', 'press A; wait 30')),
        online.at(asked, ('repeat', 'press A; wait 10', 'choice~/2', 30)),
        online.yes(asked),
        soon(f'team={name}', asker, 600),
    )


# Alpha and Beta at the plaza's first spot, Gamma and Delta a little below: each pair within A's reach, facing
# (BN6's talk probe reaches 8 ahead, a peer within 24 of it)
MEET = online.course(
    online.into_square(),
    online.into_square('beta'),
    online.into_square('gamma'),
    online.into_square('delta'),
    soon('peers~3:', '', 900),
    ('@beta', 'do', 'place 116 -52 5; wait 10'),
    ('do', 'place 92 -52 1; wait 10'),
    ('@delta', 'do', 'place 116 0 5; wait 10'),
    ('@gamma', 'do', 'place 92 0 1; wait 10'),
    soon('peer~Beta at 116 -52', '', 300),
    ('@gamma', 'do', 'wait 20'),                 # (a state's first peer line alone is read: the others' places are the server's)
)
TEAMS = online.course(
    team_up('', 'beta', 'Beta'),
    soon('team=Alpha', 'beta', 300),
    team_up('gamma', 'delta', 'Delta'),
    soon('team=Gamma', 'delta', 300),
)
# (Alpha before Gamma, his Battle: its format asked, Side by side the second; Gamma's game takes it by script)
SIDE_BY_SIDE = online.course(
    # (MegaMan's word on the team paged on every game first: a place is not taken while a chat is open)
    *[online.at(g, ('repeat', 'press B; wait 10', 'chat=closed', 20)) for g in ('', 'beta', 'gamma', 'delta')],
    ('do', 'place 68 0 1; wait 10'),
    ('expect', 'pos<=72'),
    ('do', 'wait 20'),
    ('repeat', 'press A; wait 10', 'choice=1/3', 20),
    online.pick('choice~/2'),
    ('repeat', 'wait 4', 'choice~/2', 30),
    ('repeat', 'press DOWN; wait 6', 'choice=2/2', 6),
    online.pick('challenge~to Gamma|teammatch!=none'),
)

MANY = {
    'side-by-side-through-a-drop': dict(CAST, START=STRONG, GAMES={'beta': STRONG, 'gamma': WEAK, 'delta': WEAK}, STEPS=online.course(
        MEET, TEAMS, SIDE_BY_SIDE,
        # (MegaMan's words on the match paged on every game: a word open holds his game's bout back)
        *[online.at(g, ('repeat', 'press B; wait 10', 'arena!=idle', 90)) for g in ('', 'beta', 'gamma', 'delta')],
        soon('arena=battle', '', 1200),
        soon('arena=battle', 'beta', 900),
        soon('arena=battle', 'gamma', 900),
        soon('arena=battle', 'delta', 900),
        ('do', 'wait 120'),
        # (Delta's link cut for three seconds, as a lost network leaves it: his game tries again every half second
        # while the battle waits, takes his place, his team and the battle back)
        ('@delta', 'do', 'reconnect cut 3'),
        soon('arenalink~down', 'delta', 120),
        ('@delta', 'expect', 'arena=battle'),
        soon('link=up', 'delta', 900),
        ('@delta', 'expect', 'arenalink~up'),
        ('@delta', 'expect', 'team=Gamma'),
        ('expect', 'arena=battle'),                 # the others waited on his keys, never stopped
        soon('arena=over', '', 6000),
        soon('teammatch=over won', '', 600),        # fought to its end: Alpha's team won, not broken off
        soon('teammatch=over won', 'beta', 600),
        soon('teammatch=over lost', 'gamma', 600),
        soon('teammatch=over lost', 'delta', 600),
    )),
}
