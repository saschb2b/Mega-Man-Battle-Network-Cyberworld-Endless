---
type: Feature
title: "One rule: full rules"
description: "Every NetBattle by full rules since protocol v9, the BN6 rules and stand-ins of v4 to v8 retired: the folder Lan picks (`arena_folder.c`), ARENA_LOADOUT, the ready hash, the 60-frame hashes, the battle's end."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T16:53:46Z }
---

# One rule: full rules

Every NetBattle is by full rules since protocol v9 (issue #131): both
games hold the NetPass's Arena stamp, which takes the full set of ROMs
([The NetPass](../../netpass.md)), so BN5's chips go, in the folder
and in the pack's extension (every id of the chip registry, BN6's blanks
among them), and so do MegaMan's Souls: UNITE on the Custom screen as in
a run. Both pair cores of each game hold BN5's installs ([Full
rules](full-rules.md), below). Nothing on the wire names the rules: v9
took the rules byte out of ARENA, ARENA_STATE, STANDS and WATCH_STATE,
and the server holds the pair to the full set instead (NOTICE 28).

From protocol v4 to v8 a challenge named one of two formats, chosen from
both players' ROMs: **full rules** between two games with BN5's ROM, and
**BN6 rules** for any other pair, plain BN6 cores in which the Souls
stayed home and each BN5 chip of the folder was stood in for by the
Pack's nearest BN6 chip for that battle (`arena_standin.c`, the owner's
interim decision of 18 October 2026). The server refused full rules
otherwise (NOTICE 19). v9 has no mixed pairs, so BN6 rules, the
stand-ins and NOTICE 19 went with them.

**The folder.** A run's MegaMan holds Folder1, Folder2 and the Extra
Folder (BN6's names; the second's Regular and TagChips at NaviStats
+0x2F and +0x58, bn6f's NaviStats; the Extra Folder has none of its own
here). `arena_folders_fitting` says which can go: each that holds a chip.
Where more than one can, MegaMan asks in BN6's
chat box, "Which folder goes,Lan?" (`arena_folder.c`, flags
`0x14A1`-`0x14A4`); B sends nothing. Where one can, it goes unasked. A
run holds Folder1 alone (no run hands out Folder2 or the Extra Folder),
so the question shows only with the `folder` dev step ([Dev tools](../../../devtools/overview.md)). The
carried MegaMan (`arena_format_carry`: the folder picked, its Regular and
TagChips, as the one the core's Folder1 holds) then passes
`arena_loadout_check`, the same last guard on both games.

START gives each game its side of the cable (the challenger's 0) and the
input delay. Each sends its MegaMan (ARENA_LOADOUT: the folder picked, the
NaviCust's placed programs and grid, max and base HP, the buster,
Regular and TagChips, Crosses, Souls and Beast Out; the pack stays home)
and builds the pair from both, its own as the other reads it, so that the
two pairs are the same byte for byte; the other's is checked with the
local refusals. The Square's core holds still meanwhile (the scene runs
the battle's frames in its place: `online_visit_held`), its connection
kept, MegaMan standing where he stood on the server's view.

After the way in each game says it is ready with the pair's hash at the
Custom screen (ARENA_HASH at frame 0xFFFFFFFF), and starts its ticks when
the other's has come, so both start within the link's one way; a
different hash there stops both before the first frame. My keys go a
frame a message as each tick samples them (two frames a message, tried
first, left the earlier side waiting half its ticks at a delay of 6:
`ARENA_NET_FLUSH`); where the transport's outbox is full they wait and go
together, eight a message at most, and a ninth breaks the link. The
pair's hash every 60 frames goes too, and a difference stops the battle
on both games, the log saying at which frame ("the two games went apart
by frame N"), the result card in the PET's words ("Communication error!
The two PETs saw the battle apart.") and MegaMan's after it ("Whoa! The
NetBattle broke off!").

The battle's end is the cable's first's to tell the server (ARENA over;
the server takes it from that seat alone, another's ignored, issue
#181's review), after its last keys; as is a stop both games see (the hashes apart),
lest the server answer the second with a notice that the battle is over.
A stop of one game alone (a link broken, the cable stuck, the reset
chord: back to the gate) leaves it. The server's ENDED to the other
game: its link spends the keys already come, the battle over on the same
frame there as here, and breaks after `ARENA_NET_GRACE` ticks
("the other player left the NetBattle"; MegaMan: "Huh? They're gone...").
The connection lost breaks it at once. The result shows on the picture
(A or B after half a second, or five seconds, goes back), then MegaMan
says it in the Square where he stood ("Alright! We won,Lan!!", "Ugh...
They got us." and "Next time,Lan!").
