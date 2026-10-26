---
type: Engine Mechanism
title: "A layer"
description: "How a layer is built into the game (`net_gen.c`, `netmap.c`, `layer_objs.c`, `mapslot.c`), its exit pad's warp, what `director.c` watches, checkpoints and CONTINUE, the engine's event flags and `talk.c`'s conversations."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-22T13:42:36Z }
---

# A layer

1. `net_gen.c` generates rooms, walkways and objects from the run seed.
2. `netmap.c` builds the area's tile map from tiles learned per class from
   one original map (`area_src.c`), and its wall list, and points the map's
   descriptor and coordinate data at them.
3. `layer_objs.c` turns the objects into NPC scripts (`npc.c`), text
   (`text.c`, `scripts.c`), Mystery Data and shop stock, and `mapslot.c`
   points the map's NPC list, scripts, objects, Mystery Data and sprite list
   at them.
4. `gamecall.c` warps MegaMan in with the game's own warp routine.

The exit pad is the game's own warp pad: trigger cells that take warp 1 of
the map's warp list. When MegaMan steps on it, the game starts its jack-out;
the director builds the next layer meanwhile and points warp 1 at its start,
so the game jacks him in there. A guardian keeps the pad hidden and shut
(flag `0x16F1`) until its Guardian Data is taken ([Guardians and super bosses](../../game/bosses/overview.md)). A Yes to the Undernet or the Secret Area
starts the same departure (`0x080059B5`) to a side layer built at once.
On an act's last layer the pad leads home instead ([Home](../../world/home/overview.md),
`director_home.c`): the next layer is built as ever, Lan's HP is
installed (`lanhp.c`) with that layer behind its first portal, and warp 1
points at its arrival, BN6's blue pad, where Lan's PC's jack-in lands. In
Lan's HP the director holds the open links' story flags every frame (BN6's
link markers lock the others), and a portal's warp, seen pending as it
starts, builds its way's layer where another's was built and points the
portal's own warp entry at it before the game reads it. The run's town
stays installed, its jack-in to Lan's HP's arrival, and GameState's saved
real-world place at its port, so BN6's own jack-out from Lan's HP (R, or
the blue pad's warp) sets Lan down there.

`director.c` then watches the game: the exit pad's warp, choices made in text (event flags set by
Yes), battles won (viruses counted, bosses beaten), the game's GAME OVER
(the run ends), MegaMan on any other map for 90 frames (warped back to
the layer's start), and a checkpoint shortly after each arrival (`run.sav` plus
the core's `run.state`), once MegaMan is free to move: free as BN6 tests it
before the pad and START (`src/director/director_hold.c`: no fade, cutscene,
chat, lock, scripted walk, conveyor, NPC chat, warp, PET or pause), as every
save waits (the arena door's, home's, the PET's Save's, the quit's; issue
#23: a state written while BN6 held him came back held). A CONTINUE is
watched until MegaMan first walks: held from its load on with nothing of the
director's under way for two seconds, or the pad pushed three seconds with
no step taken (the player object's `BN6_PLAYER_PAD`), BN6's holds are let go
as its own routines let go of them and the map is entered again (then the
layer's start); a map's entry waiting on a fade's mark no fade runs is let
go too. The dev step `stuck NAME` (play.py, `--input "0:stuck conveyor"`) saves
the run held so, to test it. Quitting (Escape
twice, SIGTERM or SIGINT from a launcher) saves the run once more where he
stands if he is free on the layer's map with nothing of the guardian under
way; loading it restores the guardian's state from its flags. The shops'
stock is written again after the load (the ROM copy the shop screen checks
the list against is not in a state), each entry the saved list holds
keeping its stock there, and `run.make` records the build's `LAYER_MAKE`: a run saved by a
build that makes layers otherwise continues its layer from the start, with
the layer's flags and Mystery Data picks cleared (its RAM would not match
this build's objects). `battle.sav` keeps what the random battles
remember of the last two (their virus families, loot.h), so the first after
a CONTINUE brings none of them either. `run.seen` beside it keeps the map's panels seen so
far, and event flag `0x144E` in the state that L has told where they are,
which CONTINUE clears so that L starts over (`0x144F`: that the Net
Dealer has said his words, so a later talk is a line and the list;
`0x1450` the same for the NaviCust vendor, `0x1451` for a
Recovery Mr. Prog, whose heal is then one box, and `0x1464` that
MegaMan's HP is full, the director's each frame on the map, so he keeps
his patch; `0x1452` that a super
boss's staging has Bass's stone shake and crack (it was the Guardian
Data's second way on, until the act's ways became ports at home, issue
#86; [Super bosses](../../game/bosses/super-bosses.md)); `0x1453`
that the layer's collector's vault gave its chip; `0x1454` that the super
boss strikes his pose out of the white (it was the Guardian Data's dark
way into the Undernet); `0x1455` that the
layer's official gate gave its chip, `0x1456` that Chaud's clearance
opens it, `0x1457` that Chaud's call on a duel layer was made,
[The rival: Chaud and ProtoMan](../../game/rival.md) (`0x146E`, which named the chips that sat out of the
guest core's battles, is free since they left runs); at home `0x1460`-`0x1462` that request 0-2 was taken, `0x1463`
that one is held and `0x146A` that the held one was settled, which the
askers' scripts set and the director keeps as each visit's places are
installed, [5. Jobs (the Request BBS)](../../world/home/jobs.md); `0x146C` that the town holds nothing
more for this visit, which the director keeps every frame at home and
which sends Lan's HP's courier and his mark off, piece 2; `0x146F` the
Yes of a link's question in Lan's HP (a trip back's or a way on's), which the director clears
before it asks and acts on as the talk closes, and `0x1470`-`0x1471`
set while Lan's HP's link on the left and down front is no way back,
which sends their way-back marks off, piece 2; `0x1472` that the
courier has been heard, which his words set and which sends his burst
off, piece 2; `0x146B` that this visit's order was
made at AsterLand's counter, piece 6; `0x1473` the Yes of the Net
Square's gate's questions and `0x1474`-`0x1476` its keeper's options,
the official Square, another server and the name, piece 11, which the
director clears as each visit home begins). In the Net Square itself,
whose RAM is thrown away on the way home, `0x1480`-`0x14D0` are its own:
the phrases and emotes asked for (`0x1480`-`0x148D`, and emotes 6-11's
`0x14CA`-`0x14CF`, issue #176), the channel
keeper's next and last (`0x148E`, `0x148F`), a peer's wave, mute and
block (`0x1490`-`0x1492`), the way home (`0x1493`), Trade Central's
options and the Navi picked (`0x1494`-`0x149F`), a NetBattle asked for
and the Yes to one (`0x14A0`, `0x14A1`), the folder a NetBattle carries
(`0x14A2`-`0x14A4`), the greeter's dive into the week's net (`0x14AF`;
with the way home's `0x1493` set beside it, the unfinished dive begun
again; issue #127), the spare picked from the clerk's chat-box list
(`0x14B0`-`0x14C3`), and the referee's Yes to watching a NetBattle from
the stands (`0x14C4`, issue #128), L's Look around (`0x14C5`), a team
asked for, its Yes and its order or a team challenge's format (`0x14C6`-`0x14C9`), and L's quiet
(`0x14D0`, issue #176);
[Online: the Net Square](../../online/overview.md). Its life
(`square_life.c`: the walkers, the log-in beam, the cheer, the greeter's
and the referee's words) takes none.
CONTINUE loads the state and enters the map again, so the game reloads it
from the current build's tables.

It also starts conversations of its own (`talk.c`): Lan and MegaMan on
arriving somewhere new (the first layer, the Undernet and the Graveyard,
the Nest, a side layer, a net rebuilt after the Nest), Chaud's call after
the Secret Area's guardian, and MegaMan's word on where they are when L is
pressed on the map (in battle L still opens the Custom screen). Each is
built into the talk slot and run by the game's chat box
(`BN6_CHAT_RUN_SCRIPT`), once the area card has gone and no guardian scene
is playing; the player's keys only page it until the box closes. In the
town the first conversation is the town's own (its text archive, Dad's
call on the very first run).
