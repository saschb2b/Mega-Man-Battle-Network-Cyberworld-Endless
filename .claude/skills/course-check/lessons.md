# The course check's log

Newest last. One entry per check: when, the team and its cost, what the
verdict was, what the devil's advocate and the numbers changed, what the
owner decided, and what the next check should measure again. The
reports themselves stay in `.build/research/` (not committed).

## Check 1: 26 October 2026, after 0.15.1

**Asked:** "take a step back and reevaluate our current core gameplay
loop: is it still the direction, does it lack something, or has it
bloated functionality that should be reevaluated?" Fifteen releases in
a month (0.1.0 on 27 September to 0.15.1 on 25 October).

**Team:** Hades and Hades II; Cult of the Lamb; the build against One
Step From Eden and the deckbuilders; the audience; the systems audit;
the playtest record; then the devil's advocate. 1.7 million tokens of
subagents, each agent 7 to 12 minutes. The numbers were read last, by
the manager, after the owner signed in to Umami. Reports in
`.build/research/loop-review/`.

**Verdict** (docs/game/core-loop.md): keep the direction (BN6's battle and a
folder the player builds, the act's chain to a known guardian, memory as
the pull, variety not power); fix the pace and put the danger where the
run's peaks are; count the choices before cutting; freeze side systems
until the main path is played again.

**What the devil's advocate changed:** the persona's falling scores
(8.0 to 3.0) were two main-path failures (a pad bug, BN5's End Area
junctions), not bloat; the build analyst's numbers were the flat plan's,
superseded by the opening plan; the code lean already follows the folder
the player edits; a 12-to-15-minute act target contradicted acts of 9
to 11 minutes; bloat counted by systems (61) mattered less than the main
path's words and walks. Dropped: one Cross slot a run, a code named at
Lan's HP, threat 2's heals as default, folding home's shops. Pace moved
ahead of the economy.

**What the numbers changed** (119 players, 251 runs, 564 guardian
battles, 6 to 26 October): guardians won 93% of the time with a median
81 to 85% of MegaMan's HP left after about 30 seconds, so the act's
climax does not test the plan; people open a median 3.25 Mystery Data a
layer of 5 to 8 (the personas open all of them), so the layer needs no
new price; ProtoMan's netbattle ended 30 of 88 lost runs; 63 of 119
players started one run only; 89% of release runs carry BN5; the Square
is visited a median minute.

**Decided by the owner:** a lost netbattle costs the HP and the
clearance, never the run (built the same day: `battle_spare.c`). The
freeze, deferring Virus Battler and a playtest gate per minor release
were proposed and left to the owner.

**Lessons for the check itself:**

- Read the numbers first and give them to the team. They came last and
  overturned two findings every analyst had built on.
- The persona playtesters play unlike people: full clears, threat 1,
  generous scores. Never let their trend stand as a design finding.
- Make every role say ON or OFF the main path for what it calls bloat.
- Every number in a finding is checked against the plan in play; the
  devil's advocate is told to.
- Reddit refuses the tools; the Pullpush archive reads it.
- A finding from one role alone is a lead; two or more roles reaching it
  apart (walking, zenny after act 1, BugFrags with no sink, the home
  walk, the run log counting no choice) is the verdict's spine.
- The first item of the verdict was a fix found in the numbers, done in
  the same session; offer that every time.

**Measure at the next check:**

- ProtoMan: runs ended by him (should fall to the races' few), the
  netbattle's take rate and win rate now a loss is cheap.
- Guardians' HP left and seconds, once Phase 2's band is in.
- Mystery Data a layer, minutes a won short net, the share of players
  with one run only.
- Threat climbed past 1; folders and Crosses brought.
- The Square's minutes and NetBattles, under the freeze.
- Whether the choice counts (Phase 0) are in the statistics yet; if
  they are, read them first.
