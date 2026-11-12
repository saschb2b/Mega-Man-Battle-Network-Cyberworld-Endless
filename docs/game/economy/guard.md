---
type: Design
title: "The guard"
description: "The guard against economy drift: `build.py economy` holding the plan in play to bands a fifth either side (`econ_band`), `test_economy`'s curve tests, the run log's counts, the run-end statistics event and AGENTS.md's checks row."
tags: [game, economy]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-05T17:00:00Z }
---

# The guard

1. **The report with bands.** `python3 build.py economy` prints both
   plans on both nets and flags (`!`) any act's chips, good share,
   Mystery Data, zenny or codes held a fifth past its plan's measured
   number (`econ_band`, `src/core/economy.c`: the endless net's six acts
   and Nest, the short net's three and Nest); it prints how many of the
   plan in play's (the opening one's) are outside: 0. Run it wherever
   drops, prices, Mystery Data, the dealers' stock or the levers change.
   A change that moves a number on purpose sets its band's centre in the
   same commit, with the report's before and after in its message:
   `python3 tools/economy_centres.py` shows the centres the report gives
   beside those in `economy.c`, `--write` sets them, and the report run
   again says 0 outside (Re-centring, below). A
   run measured is a profile's past the Nest: the tournament's two rounds
   pay their results screens and Navi chips where its coin puts a bracket,
   and acts 3 to 5's chips and zenny were set with them on 5 November 2026
   ([the tournament](../tournament.md), As built).
   `--economy-plan flat` plays a game under the flat plan, to set beside
   the one in play ([Dev tools](../../devtools/overview.md)).
2. **The curves' tests.** `tests/test_core.c`'s `test_economy`: the
   opening plan is the game's; every curve of it starts at its floor, ends
   at 1, rises ever more slowly and never past the flat plan's; the flat
   plan is 1 everywhere and draws no random number; the way through the
   endless and the short net; the shares kept by draws and by the hook's
   hash (none leaned on act 1, the same for the same battle, no draw);
   every band around its number.
3. **The run log.** Each battle's line keeps the zenny and BugFrags on
   hand, and a win's busting level and reward; the run's last line its
   economy's counts ([Run log](../../devtools/run-log.md)). Playtests and autopilot
   runs check the report against the game.
4. **The statistics.** The run-end event carries the run's economy, where
   the player said yes, numbers only: `chips` found (a battle's reward, a
   Mystery Data's, a Guardian Data's Navi chip), `good` among them (tier 2
   or more), `md` opened, `zenny` earned (battles and Mystery Data), and
   the same for its first act (`chips1`, `good1`, `md1`, `zenny1`). They
   are counted whatever the answer by the run log (`runlog_tally_*`,
   `runlog_md_opened`, src/director/runlog.h; `analytics_count_*`), kept
   beside the run's checkpoint (`run.tally`, carried in a `.cwsave`), and
   written in the run log's last line; [The player's guide](../../GUIDE.md)'s table says so. Real runs' first acts can then be set beside the
   report's act 1 (9.5 chips, 2.2 good, 11.6 Mystery Data, 2,630 zenny,
   every one of them opened and won).
5. **AGENTS.md's checks table** has a row for drops, prices, Mystery Data
   and the economy's curves: `build.py economy` with 0 outside the bands,
   `build.py test`, `build.py pacing` where battles' rewards change, and
   the game-design skill first.

## Re-centring

For a change that moves what a run hands out on purpose: a chip moved
into a family's reward row (each of BN4's still to come does this, since
the acts' arcs pay from the signature family's row), a price, a lever.

1. `python3 build.py economy`: the report as the change left it. Read
   which numbers stand outside (`!`) and that they are the ones the
   change means to move. One that moved without a reason is a bug in the
   change, not a centre to set.
2. `python3 tools/economy_centres.py`: each centre the report measures
   beside the one in `src/core/economy.c`. `--write` sets them all, both
   plans and both nets, to the measured numbers.
3. `python3 build.py economy` again: 0 of the plan in play outside its
   bands.
4. The commit names the numbers that moved, before and after, by act.
   `python3 build.py economy noarc` writes `.build/economy-noarc.txt`,
   the same report with the acts' arcs off, where the arcs' share of a
   change is the question.

When the encounter groups were turned on (12 November 2026) the arcs
moved an act's battle zenny by -8% to +16% and its whole zenny by 5% at
most, inside every band; the centres were set to the report with the
arcs on, which also took in what the chips moved since the last setting.
