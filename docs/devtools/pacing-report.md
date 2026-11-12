---
type: Tool
title: "Pacing report: every act's battles and guardians"
description: "`python3 build.py pacing` rolls every area's random battles in every act and draws 500 runs' guardians into `.build/pacing.txt`, marking OVER or OUTSIDE past the act's band, BN5's territories and the encounter groups included, then plays seeds 1-12 for the battles per layer and act, NONE marking a layer without one."
tags: [devtools]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-12T21:00:00Z }
---

# Pacing report: every act's battles and guardians

```bash
python3 build.py pacing
```

Rolls the random battles of every area in every act it can visit, on the
first two cycles, without running the game (`src/dev/pacing_report.c`):
300 rolls per layer, the run's opening battles and a Server challenge. For
each it prints the act's band ([Progression](../game/progression/overview.md)), the viruses' HP
together (lowest, median, highest), the strongest hit and how many battles
reached each version, and marks OVER where a battle lies past the band or
its damage cap. A challenge that meets an SP Navi is counted apart. Then it
draws 500 runs and lists the guardian each act met, with version and HP,
marked OUTSIDE when one lies past the act's band, and the Net Dealers'
answers: per act and element, the chips 300 layers list first, with a `+`
on those over the act's cap (stocked one, not two). Last, the Net's clock
([3. Going back, and the Net's clock](../world/home/going-back.md)): each act's median guardian HP at 0 to 6
notches, beside the next act's band. The whole report takes about a
minute.

Where BN5's ROM is beside BN6's, its families and Navis are installed as
a core starts, and each area one of its areas dresses ([Other games' ROMs](../multirom/overview.md))
is followed by that area's battles: its own records in BN6's engine where
it is native (issue #82's D1: rows `native`, `opening` and `challenge`,
as the director rolls them, `xterritory_encounter`, with how many were
the older net's); where it is not (an install that failed) a line says
so, its layers fighting the BN6 area's own battles above. The deep's acts
(the Undernet's, the Graveyard's, the Nest) say "the deep" and count the
older net's battles among their rolls (about half: One net, the deep).
After the families, the encounter groups
([Encounter groups](../game/progression/encounter-groups.md)): for each ROM set the ROMs read make up (BN6 alone, with
BN5, with BN4, with both), the pool's families by act and how many ways
each of the eleven templates fills under the act's band, its cap on one
hit and its pressure cap; then 300 group battles rolled for every layer
of every act's areas, and one layer of three acts on the second cycle,
each one's HP, hit and predicted pressure read again from the ROM's
records: lowest, median and highest HP, the hardest hit, the most
pressure, the share standing on a field of the area's own, each
template's count, OVER past a limit or on a shared or special panel.
Then the acts' arcs: 200 runs' arcs for every act's
area by their seeds, how many acts have one, the share whose signature
is one of the area's own, how many families were a signature, the most
pressure of each beat and of the Server's challenge against its cap,
OVER past a limit; and under each line what the beats' paying rows hold
against the rows of the area's own battles (chip entries of ten, zenny
a row, kinds of chip). Act 1's lines also list each distinct beat once
as the `foes` step takes it (`beat 2: 1@52/200@51`), few enough to fight
each one ([Encounter tags and danger](encounter-tags.md)).

Before them, each net's families as their own records set them (their
role, stage and placements; `*` its net's own, `!` those set on special
panels alone): what the groups' pool takes their places from. The
visitors' rolls that stood here went with the visitors. The guardians of
the older net follow
the first cycle's own: each of BN5's six in every act its area can come
in, and in the deep's acts where his HP reaches their band (those short
of it are not in the pool: said so), at the version he is fought at, his
own HP and as he spawns, on threat 4 too, against BN6's guardians' band;
OUTSIDE past it.

Then `tools/battle_floor.py` plays seeds 1-12 side by side (the test
autopilot, each in a fresh data folder under `.build/battle_floor/`, with
`--dev god,onehit` so the walk is measured act after act, 60000 frames;
`--seeds` and `--frames` after the action) and counts the random battles
on every layer MegaMan left, from the run log's layer lines
([Run log](run-log.md)): a row a seed (`4:1`, depth 4 and one battle; `g` a
guardian's layer, `l` a dark one), then each act's layers, battles, the
share a layer, how many had none and its whole acts' range. A layer
without a guardian (not a dark layer's board, not a trip back) holds one
to three ([the battle floor](../game/progression/structure.md)), NONE marking one without and
OVER past three, and an act six at most, its guardian's layer's battles
among them (BN5's End Area's eight in two would show). About three
minutes on all cores.

Output: `.build/pacing.txt`. It needs the ROM, and prints how many
battles and guardians were past their band, the territories' included,
and how many layers and acts were past theirs; after a change to the
bands, the areas, the encounter code or the generator's walk, both
numbers should stay 0.
