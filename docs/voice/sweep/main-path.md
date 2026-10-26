---
type: Review Log
title: "Voice sweep: the main path's words cut"
description: "Log of the 26 October 2026 cut of the words a run's main path pages through (Phase 1 of the core loop's review): the Guardian Data chain, arrival talk and the channels that name the next guardian, with the boxes counted before and after."
tags: [voice]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T17:30:00Z }
---
# Voice sweep: the main path's words cut

Phase 1 of [the core loop's review](../../game/core-loop.md), Words on
the main path cut: time between battles, and the loudest public
criticism of the project, dialogue that reads machine-written. The rule
is [Voice](../rules.md#what-nobody-says)'s: a line that does not need to
be said is cut, not rewritten, and each fact keeps one channel at each
decision point. The review dashboard's db held no verdicts, directions or
sweep marks when this began. Ids are the catalog's before the cut.

## Counts

Boxes a player pages, counted from the debug log's talks (`talk:` lines,
the same seed and a veteran profile, L pressed as the briefing allows)
and from the reward's script, captured with `--talk reward`:

| Where | Before | After |
| --- | --- | --- |
| Guardian Data chain, act 2 (his last word, a new Cross, ExpMemry, the draft, a program that needs room, Dad's mail) | 12 | 9 |
| The same with no Cross and a program that fits | 9 | 8 |
| An act's first layer: its arrival words and L's first briefing | 5 to 7 | 2 |
| A known guardian's layer, L's first briefing | 9 to 10 | 3 (4 with EraseMan's or DiveMan's warning) |
| A busy layer's second L (ProtoMan, a flame, services) | 11 | 9 |
| The catalog | 1890 lines, 2768 boxes | 1878 lines, 2746 boxes |

The Guardian Data chain is not down to one screen: what remains is
BN6's own "MegaMan got:" boxes, the draft's ask and its menu, the call
to install, and the pointer to Dad's mail. Each says something a later
decision needs.

## The next guardian, one channel a decision

| Decision | Kept | Cut |
| --- | --- | --- |
| Which way at home | MegaMan at the portal | (the second screen's home panel and Dad's dive report stay as screens read on demand: no box to page) |
| The folder, as the act begins | The act card | The act's arrival words naming him |
| A purchase | The Net Dealer's word, at his list | L's "his copy guards this area" on the act's other layers |
| His layer | L's "he waits at the end", the last stop's at the arena's door | His whole tip in L, which Dad's mail holds (BN5's guardians and the super bosses, whom no mail covers, keep theirs) |

The warnings that saved runs stay in L on his layer, a box each, the
tips' own words: EraseMan's "Low on HP? He erases us in one blow!" and
DiveMan's "His wave lights our panels? Get to our back column!"
(`guardian_warning`, `src/layer/guardian_lines.c`).

## CUT

| Id | The line as it was | Rule |
| --- | --- | --- |
| `reward_words:reward_items:0` | **MegaMan's HP was fully restored!** | The heal stays, unsaid: the act's end and the way home ask nothing of it |
| `powers_words:cross_feel.feel:1` | **HeatMan's Cross data! I'm all fired up,Lan!** | BN6's "MegaMan got:" box says the Cross; the feel changes no decision |
| `powers_words:cross_feel.feel:2` | **ElecMan's Cross data! It's crackling through me!** | as above |
| `powers_words:cross_feel.feel:3` | **SlashMan's Cross data! I feel faster already!** | as above |
| `powers_words:cross_feel.feel:4` | **EraseMan's Cross data... It's cold,Lan. But strong.** | as above |
| `powers_words:cross_feel.feel:5` | **ChargeMan's Cross data! Choo,choo! Full steam ahead!** | as above |
| `powers_words:powers_reward_words:2` | {name}'s Cross data... **We carry just one,Lan. / So** we keep our {brought}. | One box: keeping ours says the rest |
| `reward_words:draft_take:0` | **Let's install it!**, said after "We'll move one to make room,Lan!" (kept where the program fits) | Making room says the install |
| `story_words:arrival_words:8` | **We're through to {area},Lan! / {name}'s copy guards this one. Let's go!** | The act card, just before, says both |
| `story_words:arrival_words:9` | **We're through to {area},Lan! / A strong Navi's signal waits deeper in. / ...I don't recognize it. / Then let's go find out who!** | The act card says the area and "Guardian: ???" |
| `briefing_words:first_words:1` | **Layer {depth},Lan. We're in {area}!** | The screen's corner says the layer, the act card the area |
| `briefing_words:first_words:4` | **{name}'s copy guards this area!** | The act card and the Net Dealer name him before his layer |
| `briefing_words:first_words:5` | **Word is,{name} guards this area!** | as above |
| `briefing_words:first_words:6` | **A strong Navi's signal waits deeper in...** | as above |
| `briefing_words:guardian_words:1` | **We've got battle data on him! / {tip}** | Dad's mail holds the tip; kept for BN5's guardians and the super bosses, and EraseMan's and DiveMan's one warning |
| `briefing_words:guardian_words:6` | The Net kept copying **while we went back... / {guardian_known}** tougher **for it**,Lan! | One box |
| `briefing_words:spot_where:0` | **It's a winding walk** | Said of ProtoMan and the locks, where no arrow leads; kept for the heal and the last stop's services |
| `briefing_words:flame_words:1` | **It's a winding walk** | as above, for the flame |

## Kept, and why some stay

- The guardian's last word, one box: his voice, the fight's end.
- "MegaMan got:" for his HPMemory and chip, a Cross, ExpMemry and the
  program: BN6's own boxes, and what the folder and the board are set
  from next.
- The draft's ask ("Pick one! Or B for N BugFrags"): the decision.
- "Let's install it!": a program waits off the board until placed (a
  returning playtester left one off for two acts).
- "Lan,mail from Dad! Our battle data on {name}!": the one pointer to the
  channel that now holds the tips.
- Chaud's call before ProtoMan's netbattle (four boxes): the rival's own
  beat, changed this week ([the rival](../../game/rival.md)), left to its
  owner's next pass.
