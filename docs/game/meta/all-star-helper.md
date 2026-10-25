---
type: Feature
title: "All *, a fourth helper"
description: "All *, the fourth helper (issue #18): every chip of the run in BN6's `*` wildcard through the core's chip records, Program Advances still forming, an opt-in assist that counts toward unlocks."
tags: [game, meta]
generated: { by: claude-code/unrecorded, at: 2026-10-23T18:02:58Z }
---

# All *, a fourth helper (4 October 2026, issue #18)

A player asked for an option that makes every chip a `*`: they like the
series, but grinding codes for a decent folder always put them off. A run
already spares the grind (no pack to farm, rewards in the folder's codes
or `*`: phase two), but every reward is still a pick between a strong
chip in the wrong code and a weaker one that fits, and every hand is still
cut by its codes. The owner's call: a new option of the setup. What
follows was reasoned with the game-design skill and is built as written;
it is a proposal, and each decision is one switch for the owner to turn
the other way.

- **What it is.** A fourth helper on the JACK-IN SETUP's Help row, All *,
  beside HP+, Heals and Gentle: off by default, chosen per run, kept with
  the run's helpers (a bit of `Run.helpers`, no change to the save). With
  it every chip of the run comes in `*`, BN6's wildcard: the starting
  folder, every reward, Mystery Data, stock, trade, gift, vault and prize,
  and the chips in BN5's battles where BN5 dresses an area. A hand takes
  any five chips together.
- **The dialectic** is the meta layer's, what you bring against what you
  find: All * is brought, the accessibility dial every helper is (the
  stance above), not power earned. It takes one constraint, the codes, out
  of the folder's own tension, a planned hand against a drawn one. What
  stays is the folder's 30 slots and copy limits, the Reg memory, the
  chips themselves (power, element, reach) and the Program Advances as
  recipes of chips.
- **The loop layers** are the moment, every hand any five chips, never
  two codes that cannot be played together, and the session, a reward
  judged on the chip alone.
- **The patterns:** an assist in Celeste's sense, granular and opt-in, per
  run, named, no penalty for the player who needs it ("flexibility, not
  dilution": the barrier goes, the challenge stays for whoever keeps the
  codes); and the convenience rule, that every convenience deletes a
  decision, added where that decision was not worth making to its player.
- **What it costs the game.** Power: with every chip `*` every hand can
  hold five chips, a jump beyond comfort (more damage a turn, a Recovery
  chip in any hand, higher busting levels and their better rewards). It
  flattens the folders built on codes: Blade keeps its swords and loses
  only its S, and the planned Lean (codes aligned for big hands) and
  Z-Canon (Cannons in A, B and C) would have nothing left to offer such a
  run. A Chip Trader stops fixing codes (every chip fits) and stays the
  collector's machine. Accepted: it is a dial the player turns, off by
  default, and named.
- **It stands alone,** not paired with Gentle: a player put off by codes
  may still want BN6's battles at full strength, and one who wants gentler
  battles may like the codes. Separate toggles over one easy mode, as
  Celeste's assists are; whoever wants both turns on both.
- **It counts toward unlocks,** as decision 3 has every helper count:
  unlocks are options, never power, and a helper that shut its player out
  of the meta loop would be the judging mode Celeste renamed Assist to
  avoid. The run's summary and Dad's dive report name it, as they name
  every helper: a small line on the band of the summary's RESULT ("HELP:
  HP+ ALL *", "HELP: 5 ON" where the names run long), where nothing else
  stands.
- **Program Advances still form, all of BN6's.** BN6 checks one two ways
  (bn6f `sub_8029520`): a recipe that lists its chips (LifeSrd: Sword,
  WideSwrd, LongSwrd) compares the chips alone, the Custom screen having
  let them be picked together, so `*` chips make it; a recipe of one chip
  in three codes in a row (GigaCan1: Cannon A, B, C) takes one `*` of the
  three at most (`sub_80295C8`). Every chip in `*` would have left 30 of
  BN6's 43 recipes unmade (GigaCan1-3, WideBrn1-3, FlmHook1-3, H-Burst,
  PwrWave1-3 and the rest): a hidden cost for the very player the helper
  is for, and one that reads as a bug to anyone who knows `*` as the
  wildcard. So while a run has All *, that check takes all three in `*`
  (the core's copy only, [ROM data](../../rom/overview.md)), and three Cannons in a row
  make GigaCan1. Seen in play: three Cannon * staged in a first hand made
  GigaCan1, and fired as three plain Cannons with the check left as BN6
  has it; Sword, WideSwrd and LongSwrd * made LifeSrd. The net's rumors
  tell every recipe in `*`.
- **How it is done.** While a run has All *, every chip's record holds `*`
  alone in the core's ROM copy: the pack then keeps a chip in `*` whatever
  code it was given in (BN6 counts a code its record lacks as the
  record's first, and most chips, 176 of the 266 standard, Mega and Giga
  records, have no `*` of their own), and the folder's EDIT moves it back
  in in `*`. The engine reads the records as the game then does
  (chip_info), so every roll it makes comes in `*`; the folder and the
  pack turn to `*` as the run begins; the reward rows are rewritten in `*`
  as a battle ends; BN5's records, folder, DarkChips and reward rows the
  same in its battles. A run without the helper gets the ROM's own records
  back.
- **Not chosen:** a patch to the Custom screen's code check alone (one
  place, but every chip would still show its letter, a telegraph that lies
  about which chips go together); keeping BN6's one `*` in a row (an honest
  cost, but not one this player asked for); pairing it with Gentle;
  keeping it out of unlocks.
- **What could go wrong:** the strongest helper picked every run, by
  players who would have enjoyed the codes (it is named, and the threat
  rungs still climb); and a profile's very first jack-in has no setup, as
  before, so a player who wants All * meets it on their second.
