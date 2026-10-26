---
type: Tool
title: "Run log"
description: "`runlog.txt` in the data folder (`src/director/runlog.c`) records every battle (seed, depth, area, kind, HP before and after, foes, busting level, reward), each layer left with its random battles, and each run's end with its economy's counts, on the handheld too."
tags: [devtools]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T17:30:00Z }
---

# Run log

Every battle and every finished run is appended to `runlog.txt` in the data
folder (`src/director/runlog.c`), on the handheld too: the seed, depth,
area, what kind of battle (`battle`, `floor` for the one a layer owed at
its exit pad ([the battle floor](../game/progression/structure.md)), `challenge`, `duel` for ProtoMan's,
`guardian`), MegaMan's HP and max HP before, the zenny and BugFrags on
hand (`z 1850 bf 3`), each foe as family.version, the foes' HP together,
and MegaMan's HP after (or "deleted"); a won battle's busting level, 1
to 10 and 11 for S, and the reward BN6 picked by it, in the game's
encoding (`-> won hp 200 bust 9 reward 43e8`: zenny, 1000; bits 14-15 0
a chip, its code in bits 9-13; [BN6's battles](../rom/battle.md), Battle rewards), which
the economy report's battle drops are checked against. One of BN5's records fought in BN6's engine (a territory's,
the deep's: [One net: the design](../multirom/one-net.md)) has `x` and its address in BN5's
ROM after MegaMan's HP (`... battle hp 300/300 x811DDD0 v35.0 v35.0 foehp
160 -> won hp 220`: BN6's AI indexes, BN5's families moved into the
engine from 32), and a visitor of the other net has `+` before it (`v8.0
+v34.1 v11.0`). A guardian's line names him and his foe as the engine
has him: `g` and his navi number, `n` and his family there with its
version, and the HP he spawns with (`... guardian hp 100/100 g28 n36.0
foehp 600 -> won hp 100`: BN5's KnightMan, moved in as Navi AI 36, V1).
A battle MegaMan united with a Soul in has `unite` and the Soul by BN5's
number before its end (`unite 9`, NumberMan's; `unite 11c`, KnightMan's
Chaos Unison). Until issue #82's last round a battle in BN5's engine had
`guest`, its record and BN5's enemy ids instead ([Guest battles](../multirom/guest-battles.md), Guest
battles). Each layer MegaMan leaves by its exit has a line
with its act and kind (`layer`, `undernet`, `secret`, `back`, and `dark`
for a dark layer's board) and the random battles fought on it, `guardian`
and his number where one stood (`seed 00000004 depth 6 area 1 act 2 layer
left battles 0 guardian g12`): a layer without a battle shows, which
`build.py pacing` counts ([Pacing report](pacing-report.md)). A last
line gives where the run ended (`run over`, or `run won`) and its
economy's counts: the chips found and the good among them, the Mystery
Data opened and the zenny earned, then the same for its first act
(`chips 11 good 3 md 9 zenny 4210 (act 1: 6 2 7 1830)`; the anonymous
statistics' run-end event carries them, src/analytics/). Past 512 KB
the log moves to `runlog.old`. Collected from real runs, it shows where
runs are lost. The same moments give the anonymous statistics their
guardian and run-end events (src/analytics/, [The player's guide](../GUIDE.md#anonymous-statistics)'s Anonymous
statistics), where the player said yes.
