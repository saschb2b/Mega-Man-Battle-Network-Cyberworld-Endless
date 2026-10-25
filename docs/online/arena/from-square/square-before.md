---
type: Feature
title: "The Square before a NetBattle"
description: "What the Square shows of the Arena before anyone commits (issue #167, protocol v13): busy marks, the bout called in the world, an open challenge's strip and Call off, the PET ringing, link bars."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T21:51:43Z }
---

# The Square before a NetBattle

Issue #167's fourth step: what a player sees of the Arena while he walks
the Square, before anyone commits. The lens: the session loop (arrive,
see who is here, do one thing together), the Square's pull between
gathering and going, and telegraphs that tell the truth: who is free,
what a challenge is doing, how good the link is, with nothing in a
character's mouth about delay ([Voice](../../../voice/rules.md)).

- **Busy marks** (P1). A small mark of the project's own drawing over a
  player's name: crossed blades in a NetBattle (his own, or a bout of his
  team match), an eye watching one, a chip at a trade; nothing over a
  player who is free or whose challenge is only asked. The server says it
  in SNAPSHOT (protocol v13), so a challenge to a busy player (NOTICE 16,
  "Huh? NAME's busy right now...") is mostly not made.
- **The bout in the world** (P20). The two fighters' blades pulse
  between red and gold while the bout runs, and the referee calls it over
  his head ("ALPHA VS BETA!"), again every ten seconds while it lasts
  (`bout_on`, `square_life.c`), as he calls a log-in's greeting and a
  bout's end.
- **A challenge open, seen and called off** (P3). The challenger's game
  shows a strip at the picture's top while his challenge is open: "Ringing
  NAME's PET..." with a mark going to and fro until the other reads the
  question, then "NAME is reading it..." over a bar running down his 30
  seconds, the link's bars at its end and "A on NAME: call it off". On
  that player's card Battle reads `Call off`: the challenge ends for both
  (ARENA's leave) and MegaMan says "OK. We'll leave NAME be for now.";
  Battle on anyone else meanwhile is MegaMan's "Lan,we already asked
  NAME!", not silence.
- **The challenged's PET rings, and his clock starts when he sees the
  question** (P4). As CHALLENGED comes his PET rings (BN6's effect `0x7E`,
  `ARENA_CUE_RING`; which effect BN6 rings with for a call is not yet
  checked) and a blinking "!" stands over the challenger's name on his
  screen alone. The question still waits for any chat box to close; as it
  opens his game tells the server (ARENA 5, `arena_seen`), which counts the
  30 seconds from then and tells the challenger, whose strip starts
  running down. A challenge never seen ends after 60 seconds.
- **The challenger's card in the question** (P5). Before "Take it,Lan?"
  MegaMan reads the challenger's card as A on his MegaMan reads it (his HP,
  act and layer, Crosses, Souls; never the folder), from the card the game
  already asked for (`square_card_facts`).
- **The link before committing** (P6). Bars, never a number (the owner's
  word): each game says its round trip in its PING (protocol v13), the
  server puts each player's bars in SNAPSHOT (three under 100 ms, two under
  200, else one), and a game shows the fewer of his and its own, the longer
  round trip deciding a battle's delay: at the top right while his card is
  open in the chat box, while his challenge's question is, and on the
  challenger's strip (`square_ask.c`, drawn in the battle badge's look,
  `arena_view_bars`).

Seen (21 October 2026, the harness of [A rematch in a
second](rematch-in-a-second.md), headless captures): Alpha's A on Beta
(the card, its bars at the top right), Battle, and the strip: "Ringing
Beta's PET..." with its mark sweeping, then, as Beta's question opened
(the server's SEEN), "Beta is reading it..." over the bar running down;
on Beta's screen the "!" blinking over Alpha's name, "Alpha's MegaMan
wants a Practice NetBattle!", "HP 100 of 100. Act 3,layer 7!" and "Take
it,Lan?". Alpha's A on Beta again: `Call off` in Battle's place, "OK.
We'll leave Beta be for now.", the strip gone on Alpha's screen and the
"!" on Beta's. In the rematch's run, Gamma by the referee read "ALPHA VS
BETA!" over his head as match 1 began and, beside the two, the crossed
blades over both their names, pulsing.
