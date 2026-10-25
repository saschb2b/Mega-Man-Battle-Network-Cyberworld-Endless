---
type: Tool
title: "Economy report: what a run hands out"
description: "`python3 build.py economy [RUNS]` plays 300 runs without running the game and prints per act the chips, Mystery Data, zenny and codes handed out under today's plan and the proposal's, flagging any past its band."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-25T13:09:14Z }
---

# Economy report: what a run hands out

```bash
python3 build.py economy [RUNS]
```

Plays many runs (300 by default; `--economy PATH[:RUNS]` on the binary)
of the endless net's first cycle and of the short net without running the
game (`src/dev/economy_report.c`, [Economy](../game/economy/overview.md), issue #154), under
today's plan and the proposal's (`src/core/economy.c`). The layers,
their Mystery Data and what each holds, the dealers' and vendor's stock,
the Servers' prizes, the guardians' Guardian Data and the requests' pay
are the engine's own code's; each random battle's reward is BN6's, from
its reward rows by its busting table read from the ROM, at an assumed
mix of busting levels and one to three battles a layer (the header says
every assumption). It prints, per act:

- today's and the proposal's chips gained, good chips' share, Mystery
  Data laid, zenny earned and codes held (the most chips of one letter
  with the \*s, median run), each against its plan's band, `!` past it,
  and the zenny earned against what the act's answer and HPMemory cost;
- the same for the short net, unflagged;
- where each act's chips and zenny come from (battles, the field's
  Mystery Data, the layers', the guardian) and what is offered (dealers,
  Servers, requests, traders, gates), for each plan;
- the chips gained by tier, the strong ones, those that fit the folder's
  codes, Megas and Gigas; and by layer the Mystery Data laid (green, blue,
  purple), chips gained, codes held and zenny earned so far;
- the proposal's curves by act on either net, and BN6's busting table's
  pay: a battle's chip share and zenny with every battle at one level;
- a dark layer's par prize by act;
- a held Soul's kind (where BN5's ROM is read): each Soul held alone from
  the first layer, the chips of its kind gained by act with none held and
  with it (`src/core/loot_souls.c`), on the dealers' lists, every chip
  gained over the net with and without, and the chips of each kind a run
  can hold, by tier.

Output: `.build/economy.txt`, in a few seconds. It needs the ROM, and
prints how many numbers of the plan in play are past their band: 0 now;
a change to drops, prices, Mystery Data or the dealers keeps it 0, or
sets the band's centre in `economy.c` with the before and after in its
commit. The run log's `bust` and `reward` (below) check its battles
against the game.

`--economy-plan flat` on the game's binary plays a game under the flat
plan, the rolls as they were before issue #154 (`build.py shot`,
`tools/play.py start NAME -- --economy-plan flat`), to set beside the
opening plan in play: its layers lay and hold what the report's before
column measures. A dev's look only, in a data folder of its own: a run
saved under one plan and continued under the other makes its layer again
otherwise.
