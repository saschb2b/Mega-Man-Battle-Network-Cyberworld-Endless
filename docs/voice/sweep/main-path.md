---
type: Review Log
title: "Voice sweep: the main path's words cut"
description: "Log of the 26 October 2026 cuts of the words a run's main path pages through (Phase 1 of the core loop's review): the Guardian Data chain in two passes, arrival talk and the channels that name the next guardian, with the boxes counted before and after."
tags: [voice]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T21:00:00Z }
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

The Guardian Data chain was not down to one screen after this pass: what
remained was BN6's own "MegaMan got:" boxes, the draft's ask and its
menu, the call to install, and the pointer to Dad's mail. The second
pass, below, took it further.

## The Guardian Data chain, a second pass

Round three of Phase 1, the same evening. BN6's own "MegaMan got:" boxes
carry the facts, two things a box (the form the HPMemory and the chip
already shared): his Cross now beside the HPMemory, ExpMemry beside the
chip, each pair given just before its box and the heal with the
HPMemory. The call to install moved into the draft's ask, said before
every pick ("Program data! Let's install one! Or B for 10 BugFrags!"),
and after the pick only BN6's own box says what was got; L's briefing
still names a program left off the board ("Lan,{off} is still off our
board!"). "We'll move one to make room" went with it: BN6's NaviCust
shows what has to move, and the draft offers only programs that fit
once one does. The draft's ask and menu stay whole: the B price is the
decision's.

Counted from the reward's script with `tools/cw.py shot` and `ref dev`
(`--scene emu --seed 2 --talk reward:300`, A every 75 frames), then the
boxes around it as the first pass counted them:

| Where | Before | After |
| --- | --- | --- |
| The Guardian Data's own boxes, act 2 (a new Cross, HPMemory, chip, ExpMemry, the draft) | 7 | 5 |
| The same, act 1's SpoutMan (no Cross, no ExpMemry) | 5 | 4 |
| The chain, act 2, with his last word and Dad's mail (the first pass's 9) | 9 | 7 |
| The chain, any act, a veteran (no mail; a brought Cross's word one more) | 6 to 8 | 5 to 6 |
| The catalog | 1894 lines, 2240 boxes | 1893 lines, 2239 boxes |

Still not one screen: his last word (his voice, the fight's end), two
"MegaMan got:" boxes, the ask, the menu, the program's own box, and Dad's
mail the first time. Every one is a fact BN6's own box says or the
draft's choice; going further means BN6's boxes holding more than two
things, which its three lines of twenty letters do not.

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
| `powers_words:powers_reward_words:0` | MegaMan got: / "{powers_cross_name}"!! **in a box of its own** (the second pass) | Folded, not cut: the Cross shares the items' "MegaMan got:" box, two things a box |
| `reward_words:draft_take:0` | **Let's install it!** (a program that fits; the second pass) | The draft's ask says it before every pick; L names one left off |
| `reward_words:draft_take:2` | **We'll move one / to make room,Lan!** (the second pass) | BN6's NaviCust shows what moves; the draft offers only programs that fit once one does |
| `reward_words:draft_menu:0` | Program data**,too**! / **Pick one!** Or B for / {skip_frags} BugFrags**,Lan**! (the second pass) | Reworded, not cut: "Let's install one!" carries the install, and the line's three fit one box at three-digit BugFrags |

## Kept, and why some stay

- The guardian's last word, one box: his voice, the fight's end.
- "MegaMan got:" for his HPMemory and chip, a Cross, ExpMemry and the
  program: BN6's own boxes, and what the folder and the board are set
  from next.
- The draft's ask ("Pick one! Or B for N BugFrags"): the decision.
- "Let's install it!": a program waits off the board until placed (a
  returning playtester left one off for two acts). The second pass moved
  it into the ask, "Let's install one!", said before every pick.
- "Lan,mail from Dad! Our battle data on {name}!": the one pointer to the
  channel that now holds the tips.
- Chaud's call before ProtoMan's netbattle (four boxes): the rival's own
  beat, changed this week ([the rival](../../game/rival.md)), left to its
  owner's next pass.
