"""A party's guardian (issue #754, docs/online/shared-dive.md, A party's
guardian): at layer 3's guardian of --seed 3, fought on the party's core
by the dev's stood-in party (--dev partycore=N, partyscale=1: scaled as a
party's door scales him), a party of two and of four meet the version a
diver alone meets, his HP times the party."""

FIELD = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --dev quiet,partybot=1,partyscale=1,partycore='

MANY = {
    'solo': dict(START=FIELD + '1', STEPS=[
        ('until', 'guardianfought~party 1', 3000),
        ('expect', 'guardianfought~version 0'),
        ('expect', 'guardianhp=400'),                # (DiveMan V1, his own)
    ]),
    'two': dict(START=FIELD + '2', STEPS=[
        ('until', 'guardianfought~party 2', 3000),
        ('expect', 'guardianfought~version 0'),      # the solo's version, not his next
        ('expect', 'guardianhp=800'),                # his HP times the party
    ]),
    'four': dict(START=FIELD + '4', STEPS=[
        ('until', 'guardianfought~party 4', 3000),
        ('expect', 'guardianfought~version 0'),      # the solo's version, not two up
        ('expect', 'guardianhp=1600'),
    ]),
}
