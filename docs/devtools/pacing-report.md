---
type: Tool
title: "Pacing report: every act's battles and guardians"
description: "`python3 build.py pacing` rolls every area's random battles in every act and draws 500 runs' guardians into `.build/pacing.txt`, marking OVER or OUTSIDE past the act's band, BN5's territories and visitors included, then plays seeds 1-12 for the battles per layer and act, NONE marking a layer without one."
tags: [devtools]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T17:30:00Z }
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
Last, the visitors: each net's families as their own records set them
(their role, stage and placements; `*` those that may visit, `!` those
set on special panels alone), then acts 2 to 4's areas and territories,
300 battles each given a visitor where one fits: how many took one, the
battles' HP with it, the pairs (the replaced family's AI index > the
visitor's), OVER past the band. The guardians of the older net follow
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
