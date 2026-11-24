"""Who is who on a party's field (issue #713, docs/online/shared-dive.md,
Who is who): each MegaMan standing marked by his feet, the player's own
told apart from the crew's, no two marks meeting, none over the Custom
screen, and a partner's HP box his own (BN6's shows the lead's).

On one game, three MegaMen against layer 3's guardian on the party's core
(--dev partycore=3, the partners the bot's), drawn as seat 1 sees it
(partyview=1): the dev's partners have no names, so the marks are the
Arena's P1 to P3. Over a server, two divers crewed on the week's net and
a crew's join-in battle on one field: each game his own name marked as
his and the other's as the crew's, Beta's name long enough to be cut.

A crew of three and of four in one column (issue #886): each plate, a
name and a partner's HP under it, stands where it covers no MegaMan's
body and no other plate, on every game of the field (`plates=N clear`),
as on the dev's field of four."""
import scenarios_online as online
from scenarios_online import soon

NEEDS = ('bn5', 'bn4')   # (the dive takes the NetPass's DeepNet stamp: the full set)
PORTAL = 'place -44 150 3; wait 10'    # before the week's portal, facing it (seed 120's Square)
LONG = 'wwwwwwwwwwwwwwww'              # a name of sixteen wide letters, the most a name has: cut to a panel
# (the field's turns go on, each game's keys there a bot's; not quiet: the battle step goes through BN6's roll)
BOT = online.HOME.replace('quiet,', '') + ',partybot=1'


def dive(game=''):
    """The week's portal by the Board spoken to, its Yes, its word paged: the dive's first layer, its arrival words paged."""
    return online.course(
        online.at(game, ('do', PORTAL), ('repeat', 'press A; wait 10', 'choice~/2', 20)),
        online.yes(game),
        online.at(game, ('repeat', 'press A; wait 10', 'dive=on', 30), ('until', 'where=layer', 1200), ('until', 'chat=open', 600),
                  ('repeat', 'press A; wait 10', 'chat=closed', 20)))


def crew(other):
    """A crew of two: the other arrived where Alpha stands, and the server makes them one (issue #714)."""
    # (a dive's layer runs faster than the clock headless: the server's second before a crew, waited out as such)
    return online.course(('sleep', 1.5), soon('crew~2', '', 600), soon('crew~2', other, 600))


MANY = {
    'dev-field': dict(
        START='CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --dev quiet,partycore=3,partybot=1,partyview=1', STEPS=[
            ('until', 'partyfield=3 seat 1', 3000),   # the guardian's fight on the party's core, at its first Custom screen
            ('expect', 'marks=0 apart'),              # no marks over the Custom screen
            ('expect', 'hpbox=own'),                  # a partner's HP box his own, not the lead's
            ('until', 'marks=3 apart', 1500),         # the screen closed: three marks, none meeting
            ('expect', 'mark1~own'),
            ('expect', 'mark1~tag P2'),
            ('expect', 'mark0~crew'),
            ('expect', 'mark2~crew'),
        ]),
    'crew-battle': dict(
        START=BOT, NEEDS=NEEDS, SERVER=True, FILES={'netpass.ini': online.SEEN_FULL}, GAMES={LONG: BOT}, STEPS=online.course(
            online.into_square(), dive(),
            online.into_square(LONG), dive(LONG),
            soon('divers~Www', '', 600),
            crew(LONG),
            ('do', 'battle; wait 10'),                # Alpha's next battle: the crewmate near joins it unasked
            soon('partyfield=2 seat 0', '', 1200),
            soon('partyfield=2 seat 1', LONG, 1200),
            # (the Custom screen closed on Alpha's game, both marked: over the server the lead's OK waits up to 30 seconds
            # of the clock for his partner's)
            soon('marks=2 apart', '', 3600),
            ('expect', 'mark0~own'),
            ('expect', 'mark0~tag Alpha'),
            ('expect', 'mark1~crew'),
            ('expect', 'hpbox=bn6'),                  # the lead's HP box BN6's own
            soon('marks=2 apart', LONG, 3600),
            (f'@{LONG}', 'expect', 'mark1~own'),
            (f'@{LONG}', 'expect', 'mark0~tag Alpha'),
            (f'@{LONG}', 'expect', 'hpbox=own'),      # Beta's HP box his own
        )),
    # (issue #886: four on the dev's field, the lead's column full and one behind him; no plate over a MegaMan)
    'dev-field-four': dict(
        START='CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --dev quiet,partycore=4,partybot=1,partyview=1', STEPS=[
            ('until', 'partyfield=4 seat 1', 3000),
            ('until', 'marks=4 apart', 1500),
            ('expect', 'plates=4 clear'),
            ('do', 'wait 200'),
            ('expect', 'plates=4 clear'),
        ]),
}


def crew_battle(others):
    """Alpha and the others dived in turn, each arriving where Alpha stands, one crew; Alpha's battle with every
    crewmate in it unasked, the bots' keys on each game: on each game's field every plate clear of every MegaMan."""
    games = [''] + list(others)
    n = len(games)
    names = ' '.join(g.capitalize() for g in ['alpha'] + list(others))
    return dict(START=BOT, NEEDS=NEEDS, SERVER=True, FILES={'netpass.ini': online.SEEN_FULL}, GAMES={g: BOT for g in others},
                STEPS=online.course(
        *[part for g in games for part in online.course(online.into_square(g), dive(g))],
        ('sleep', 1.5),
        *[soon(f'crew={n}: {names}', g, 900) for g in games],
        ('do', 'battle; wait 10'),
        *[soon(f'partyfield~{n} seat', g, 1200) for g in games],
        # (each game's field drawn, not held under his own Custom screen or BATTLE START!: every plate clear, its picture)
        *[step for g in games for step in (soon(f'marks={n} apart', g, 3600), soon(f'plates={n} clear', g, 3600),
                                           online.at(g, ('do', 'shot')))],
    ))


MANY['crew-of-two'] = crew_battle(('beta',))
MANY['crew-of-three'] = crew_battle(('beta', 'gamma'))
MANY['crew-of-four'] = crew_battle(('beta', 'gamma', 'delta'))
