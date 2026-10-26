---
type: Feature
title: "The week's net"
description: "The week's net (issue #127): each server's weekly seed (`week.py`), a dive in a save slot of its own from a standard profile (`src/core/weekly.c`), the greeter's offer, WEEK_POST and the Board's reported depths."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-23T14:25:12Z }
---

# The week's net

[Issue #127](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/127):
each server gives a net of its own for the week, every player who dives
it dives the same net, and the Board in the welcome plaza lists how deep
they say they got. The owner approved the proposal on 19 October 2026
(each server its own week, the setup locked, as many dives as a player
likes, depths posted as "reported") and decided one thing while it was
built: **a dive keeps a save slot of its own**, so the run in progress
is never touched, and the player goes back to it when the dive ends.
Diving takes the NetPass's DeepNet stamp (above); the Board's ranking
reads on every game. Every diver has BN5's ROM, so the areas a game with
it dresses in BN5's (`run_dress`, by the seed and the ROMs present) are
the same on every dive.

## Its design

Through the game-design skill:

- **Dialectic:** the run's integrity against the community's generosity,
  as the Square's own. The week's net is the pattern "a seedable RNG buys
  community" (Slay the Spire's daily climb): the same net for everyone
  this week, so a depth means something when it is compared.
- **Loop layer:** a session goal beside the normal runs, at the meta
  loop's pace (a week), feeding nothing but the Board. A dive never
  makes a normal run stronger and a normal run never makes a dive
  stronger.
- **Patterns:** a diegetic board (the BBS, in posts); a world that
  offers rather than a menu (the greeter Mr.Prog by the Board asks, BN6's
  own Yes/No with the cursor on No); the same contest for everyone (the
  standard setup and a standard profile); honest feedforward (he says the
  dates and that every Navi dives the same net, the Board says the layers
  are as reported).

What the proposal left open, decided the same way:

1. **What a dive shares with the normal runs.** Nothing that changes
   what a run is or how strong it starts. A dive starts from a *standard
   profile*, made from the player's at its start: his settings and what
   he has been told (MegaMan's tips are not said twice; [Voice](../voice/rules.md), a
   button is named once), and none of what runs earned: no threat rungs,
   folders, Cross starts or BeastOut opened, no Spins, programs found or
   compression codes kept, no rival's clearance, no title marks (the
   Secret Area's master follows them, `run_new`, so marks would make
   another net), no records. Anything the dive earns stays in its own
   profile and is let go at the next dive: the critique's "no opportunity
   cost" and "power creep" both stay out, and the comparison stays fair.
   Its end opens nothing (`meta_run_quiet`) and counts in no record of
   the normal runs. Nor does a dive trade: on one, Trade Central's clerk
   says "DIVERS DON'T TRADE! SAME NET,SAME CHIPS FOR ALL!", MegaMan's
   board offer is taken down and an offer that comes in is called off
   unheard of (`trade_central.c`), so every dive plays the standard
   setup's chips and no swap can land in the other slot.
2. **How an unfinished dive goes on.** In the world, as it began: the
   greeter offers it again (Keep diving, Start over, Not now; B is Not
   now). The title's CONTINUE stays the normal run's, so the title keeps
   its one meaning and needs no new row; the owner's rule, no new menu
   screens where the world can do the job, holds. The price is a walk
   from the gate each session and a connection to the server, which the
   dive needs anyway to post.
3. **When the week turns mid-dive.** The dive keeps its week and its
   seed; its end is posted to that week while the server still takes it
   (this week or the last), and is let go after. An unfinished dive of an
   older week gives way: the greeter offers this week's net, which begins
   over it.
4. **Ranking.** By layer, then a win (the short net's Nest), then fewer
   minutes, then the earlier post: a speed tie-break that rewards the
   run's own dialectic, prepare or press on, among the many who win.

## On the server

`server/cyberworld_server/week.py`. A week is numbered from Monday 5
January 1970 (`week_epoch`, 345600 seconds), so every Monday 00:00 UTC
begins one. Its seed is the first four bytes of HMAC-SHA256(the server's
identity secret, "week" and the week's number): each server has its own
week, the same across restarts, and nobody can guess the next. WELCOME
carries the seed and the week's number (protocol v6). WEEK_POST posts a
dive that ended: its week, seed, layer, act, won, minutes and a summary
hash. The server checks only what it can: the week is this one or the
last, the seed is that week's, the layer is 1-10 and the act is that
layer's (three layers an act, the Nest the fourth), a win only on layer
10, the hash not zero; anything else is a NOTICE (23-26). One entry is
kept per identity and week, the best, and each improvement is appended
to the state folder's `weeks.jsonl`, read back at the start. WEEK_BOARD
sends each player this week's top eight, his own best and its place,
and last week's top three, after WELCOME and whenever the Board
changes. A moderator takes an entry down with `--admin hide IDENTITY
[--week N]`, which also refuses that identity's posts for that week.
A depth is a claim: a modified game can post any layer, and nothing is
given for one, so the Board says "as reported".

## In the game

- **The slot** (`src/core/weekly.c`, [Portable saves](../engine/saves.md#the-weekly-slot), The weekly slot):
  `save_slot(true)` puts every progress file of the run after
  `weekly-` in `savedata/`; `weekly.sav` (the normal slot's) holds the
  dive in the slot and a dive's end to post. Starting a dive clears the
  slot, makes its standard profile and the run with `run_new` on the
  week's seed (not `run_new_varied`, which avoids the last run's first
  guardian), `run_setup(RUN_SHORT, FOLDER_STANDARD, 0, 0, 0)`, and the
  game starts it in Lan's room as NEW GAME does. Continuing it loads the
  slot's checkpoint as CONTINUE loads a run. The title always leaves the
  slot first (`weekly_normal_saved`), so its CONTINUE is the normal
  run's whether the dive ended, was reset or the game was closed.
- **The greeter** (`src/online/square_week.c`, words in
  `square_week_words.c`): his archive is his welcome, then, to a pass
  with the DeepNet stamp, the week's post: the net shifted, this week's deep net open, its dates, every Navi
  dives the same net, and "DIVE IN NOW?" (Yes/No, the cursor on No). With
  an unfinished dive of this week, "YOUR DIVE INTO THE DEEP NET ISN'T
  OVER!" and Keep diving, Start over, Not now. On a dive, he wishes
  MegaMan luck; with a trade waiting to go home, he asks for the trade
  home first (the swap is settled into the run the gate saved, never a
  dive). His choice sets the Square's free flag `0x14AF` (with the way
  home's `0x1493` beside it for Start over); the visit ends
  (`VISIT_END_DIVE`) without the gate's resume, and the dive begins or
  goes on. The normal run's gate checkpoint stays as the gate wrote it.
  A game without the stamp is offered no dive, and the server refuses its
  WEEK_POST (NOTICE 28).
- **The post**: a dive's end (`weekly_ended`, after the run's records
  and saves are done in its slot) keeps its layer, act, won, minutes and
  hash, the better of two of the same week. The next visit sends it as
  soon as WELCOME has named the week; the server's answer, a WEEK_BOARD
  or a NOTICE, lets it go. The greeter then says "YOUR DIVE TO LAYER 5 IS
  UP ON THE BOARD!".
- **The Board**: after its channel's posts, "DEEP NET" with the week's
  dates, "Deepest layers,as Navis report them.", the top eight three to
  a box ("1.Alpha:5", a "!" for a win), "Your best is layer 5,#1!", and
  last week's best three.
- **The summary**: a dive's shows its layer, the viruses and Navis
  deleted, the Library without a count added, and "This week" with the
  deepest layer of this week's dives in place of the best ever; no goal
  line.

`tests/test_backup.c` holds the slot carried and put back byte for byte
in a `.cwsave`, its names (and none else) taken; `tests/test_online.c`
WELCOME's week, WEEK_BOARD read and refused, the post's bytes and the
Board's words and dates; `server/tests/test_week.py` the seed, the best
kept, the refusals, the file read back across a restart and a week's
turn, and a moderator's hide.
