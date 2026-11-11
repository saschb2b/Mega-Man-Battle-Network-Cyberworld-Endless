---
type: Feature
title: "The PET in the Square"
description: "BN6's PET in the Net Square (`square_pet.c`): START opens it there as everywhere, what each of its screens writes, the record that carries those bytes home onto the gate's checkpoint (`pet_carry.c`, `run.pet`), Save's note, R:NETPASS on its menu, and what a folder made there means for a NetBattle and a trade."
tags: [online, pet]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-11T17:01:24Z }
---

# The PET in the Square

The owner, 11 November 2026: in the Net Square START opened the NetPass,
and nothing opened BN6's PET, so a folder could not be tuned before a
NetBattle. "Online pass should not block our normal mmbn menu." START
now opens BN6's own PET there, every screen of it BN6's own on the
game's own core, and what it changes comes home.

## Why it was shut

A visit runs on the run's own core ([A visit](overview.md)): the gate
saves a checkpoint, the Square is built into the running game, and the
way home loads that checkpoint again (`come_home`, `director_resume`).
Everything the Square's game wrote goes with it. A NetBattle and the
card other players read were made from MegaMan as read once on arrival
(`online_snapshot_read`). So a folder edited in the Square's PET was lost
at home and was not the folder the next NetBattle fought with. The PET
was shut for that, START given to the NetPass instead.

Measured (the game on a local server, `--dev nocarry` for the old way
home; the dev step `dump` writes the work RAM and the save RAM now, two
dumps compared): the same folder edit made in the Square stands at home
with the carry and is gone without it.

## What each screen writes

Read from the game's RAM before and after each PET session in the
Square, against BN6's kept data (its save block, `0x02000000` to
`0x02006840`, bn6f `ewram.s`):

| Screen | Writes | Carried |
| --- | --- | --- |
| ChipFolder, EDIT | the folder's entries (`0x02002178` on), the Pack's order stamps | yes |
| ChipFolder, Regular and TagChips, EQUIP | MegaMan's NaviStats | yes |
| MegaMan, NaviCust RUN | the board's grid and placed list, a key item's count and its check (`0x02004A8C` on), NaviStats, the command line's list (`BN6_NAVICUST_LINE`) | yes |
| SubChip | a key item's count and its check, HP | yes |
| E-Mail, KeyItem | nothing (Dad's mails are never marked) | - |
| Library, Options, the Jukebox, the NetPass | nothing of the game's (the engine's own screens) | - |
| Save | nothing: the run was saved at the gate | a note says so |

As the PET closes BN6 enters the map again, which clears its
no-encounters flag (`0x1700`) and moves the map's own state (the NPC
objects from `0x020057B0`): neither is the PET's, neither is carried.

## The carry

`pet_carry.c` (no game in it, tested in `tests/test_online.c`) keeps a
record over four regions: the event flags to the request BBS's state
(`BN6_EVENT_FLAGS` to `BN6_BBS_STATE`: the folders, the Pack, the key
items and their checks, the NaviCust, NaviStats, the chips' marks), the
NaviCust's command line, and the Pack past BN6's own ids (its extension
and the save RAM's). `square_pet.c` reads them as the PET opens and
again once MegaMan is back on the Square's map; each byte that differs
goes into the record with what stood there before. A flag's byte is kept
a bit at a time, since the Square's own flags share bytes with it; BN6's
state flags (`0x1700`-`0x17FF`) are left out. A PET still open when the
visit ends is not noted: a folder of 29 is no folder.

The record is written beside the checkpoint each time (`run.pet`, the
slot's own file). At home, on the checkpoint just loaded, every changed
byte must still stand as the PET found it, or nothing is written: then
the bytes go in, and the run is saved there (`settle` in
`online_visit.c`). A game closed in the Square has them at its next
CONTINUE, from the file; the week's dive leaves them for the run's own
CONTINUE. The record names its run's seed and layer, so another run
never takes it.

What it comes to: a change made in the Square's PET is the same as one
made in Lan's HP before the visit. The run reads its folder for the next
layer's stock as MegaMan moves on (`note_folder_codes`), as after any
edit at home.

## Save

A on Save closes the PET as in a run, and the game's note shows in the
corner as a run's "Run saved" does: "Run saved at the gate". No
character says it ([Voice](../../voice/rules.md), What nobody says). A
checkpoint taken in the Square would save the Square's map as the run's.

## The NetPass

Outside the Square the pass opens from KeyItem, and it does in the
Square too. START was its one-press key there; R on the PET's menu is
now, two presses from the map. BN6 leaves L, R and SELECT unused on that
menu (pressed on it, nothing happens; START and B leave), and its band's
right end is empty. The hint stands there as E-Mail names its one
unguessed key, `R:NETPASS`, the key white and the action orange
(`look_hint`, [The PET's screens](../../engine/ui.md), Keys).

The lens (the game-design skill, interface): the PET is where BN6's
players look for their folder, so START does there what it does
everywhere (consistency, Hodent and Nielsen); the pass is a status card
read now and then, so it costs a press more and is named where the
player already is (a contextual prompt on the one screen it works from,
recognition over recall), not a chord on the map nobody would find.
SELECT would read against the menu's own SELECT label; L and R are the
PET's page keys. Whether the hint shows outside the Square too is the
owner's call (asked 11 November 2026).

## A NetBattle after an edit

Each time the PET closes with something changed, MegaMan as carried is
read again from the Square's game and his card sent again
(`online_visit_carried_again`). The next challenge checks him as before
(`mine_goes`, `arena_loadout_check`), the same checks the other game
makes: BN6's own editor keeps a folder at thirty chips and within its
Mega and Giga limits, and the Arena takes only games with the full pass,
so every chip of his folder is one the other game holds
([The NetPass](../netpass.md)). A rematch whose MegaMan changed goes the
whole way in again, not from the kept pair.

Shown, two games on a local server: Kei put FireBrn1 F first in his
folder in the Square's PET, challenged Mira, and both games dealt his
side that folder (`0x1414` its first chip); 20 of 20 hashes agreed to
the result.

## A trade and the PET

A done trade used to wait for home in `trade.pending`. With the PET
open in the Square that left the chip given in the Pack there, free to
go into a folder, and home would then have kept it: a chip made twice.
So ([Trade Central](../trade-central.md)):

- The swap is made in the Square's game the moment the server says
  done (`online_trade_here`), and noted in the same record as the PET's
  changes, so it comes home with them and is not made again
  (`online_trade_settle`'s `made`). The Pack in the Square's PET is the
  Pack home will have: the chip got can go into a folder at once.
- A deal is accepted only while its chip is still a spare
  (`online_trade_accept`); the server commits a trade only on both
  accepts, and the chip trade's screen holds the game from the accept to
  the swap.
- A chip up on the board that goes into a folder takes its offer down,
  and MegaMan says so; an offer or answer under way whose chip is no
  spare any more is called off.

Shown, two games: Kei traded Cannon A for Mira's FireBrn2 S, put it
first in his folder in the Square, and came home with it there, the Pack
one Cannon A less and no FireBrn2 S spare.

## Not yet

Seen on the Linux build headless. The second screen follows the PET's
screens in the Square as at home (its folder panel seen); the consoles,
phones and the browser run the same code, untried there.
