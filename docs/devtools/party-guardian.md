---
type: Tool
title: "A party's guardian against going alone"
description: "`python3 tools/party_guardian.py` fights each act's guardian on the party's core for one to four bot fighters, scaled as a party's door scales him (or by another rule), measures his HP and the damage he deals the party a second, and works out what a fighter pays against the solo's fight: the time to the win and the HP he loses."
tags: [devtools, online, balance]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-22T09:30:00Z }
---

# A party's guardian against going alone

```bash
python3 tools/party_guardian.py                       # acts 1-3, one to four fighters, seeds 1-6, the party rule as built
python3 tools/party_guardian.py --hp-per 50           # his HP half his own more a fighter past the first
python3 tools/party_guardian.py --steps 0 --acts 1    # no version raised for a party, act 1 alone
```

`build.py pacing` cannot fight a party ([Pacing report](pacing-report.md)), and
[issue #754](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/754)
asks whether a party's guardian is harder per head than going alone
([The shared dive](../online/shared-dive.md), the party's scaling). For
each act's guardian (layers 3, 6 and 9) and each count of fighters, the
host build runs a seed headless: the autopilot walks to his door, and his
fight goes to the party's core (`--dev partycore=N`, one the solo on the
same field), every MegaMan the same bot (`partybot=1`), the guardian
scaled for the party as a party's door scales him (`partyscale=1`: since
22 November 2026 his version a diver alone meets and his HP times the
party, [A party's guardian](../online/shared-dive.md#a-partys-guardian-22-november-2026); dev
knobs only, `boss_party_dev`: `partyhpper=P` his HP P percent of his own
more a fighter past the first, `partysteps=S` his version S on).

What it measures and what it works out:

- **Measured**: the guardian's version and HP ("his HP N (his own M)"),
  and the damage he deals the party a second (every fighter's HP lost
  over the fight's frames, the aim passed round the party as built; a
  fight not over in the window measured to its last hit, where that came
  after 1800 frames). The
  bot dodges little and deals little (a guardian takes a hit of 1 now and
  then), so it never wins: only this side of the fight is measured.
- **Worked out**, each fighter assumed to deal the same damage a second
  whatever the party: the guardian's HP a head against the solo's (the
  time to the win, 1.0 solo), and the HP a fighter loses on the way (his
  share of the guardian's damage, over that time) against the solo's.
  A party is the better fight to be in where both stand at 1.0 or below.

It reads the field's own log lines (`party_field.c`'s and
`party_play.c`'s), so a run takes the clock's pace of a party's field:
about a minute and a half each, four side by side.
