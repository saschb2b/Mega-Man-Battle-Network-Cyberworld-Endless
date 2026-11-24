---
type: Feature
title: "Looks in a NetBattle"
description: "Each player's MegaMan in his own look in a NetBattle (issue #854): both looks written into every Arena core's spare battle palettes before it starts, BN6's three picks of his palette answered by the object's side, the looks in the pair's hash, measured through rollbacks and a mutation that parts the games."
tags: [online, arena, look, megaman]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-24T21:00:00Z }
sources:
  - id: rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), read in capstone and the Arena's cores"
    title: "The player's BN6 ROM"
---

# Looks in a NetBattle

Each profile's MegaMan wears a look of his own ([MegaMan's look](../../game/looks.md);
[MegaMan's colours](../../rom/megaman-colours.md) measured where his colours are). In a NetBattle each player's MegaMan
wears **that player's look on both GBAs of both games**: mine on my
screen and on the other's, his on both. The Arena's lockstep holds every
core of both games to the same state ([What runs](what-runs.md), the pair
hash), so a look is safe only where both games give each core the same
palette bytes and the same code. This page is how that is kept.

## How

- **Where the colours go.** MegaMan's battle sprite has one block of 43
  palettes; BN6 picks his body's by form. Eight of them are zero and never
  picked (0x19-0x1A, 0x1E-0x1F, 0x23-0x24, 0x28-0x29,
  `BN6_LOOK_PAL_SPARE`). Every Arena core's ROM copy takes, after BN5's
  installs (`arena_pair.c`'s `rom_copy`, `arena_looks_rom`), side 0's
  player's plain body, its pale twin and its Full Synchro in 0x19, 0x1A
  and 0x1E, side 1's in 0x1F, 0x23 and 0x24, and each one's emotion face in
  0x28 and 0x29, each computed from BN6's own bytes in the player's ROM
  (`look_palette_of`), never from a core a look was written into. Both
  cores of a pair, both games and the New 3DS's one shared copy get the
  same bytes.
- **Who is who.** An object's alliance (`+0x16`) is its player's side of
  the cable on both GBAs; BN6 draws each GBA's own player on the left
  whichever side he is (measured below: on the second GBA the emotion
  window's owner is alliance 1). So an object's look is its alliance's.
- **The picks, answered by side.** Three hooks on each core
  (`arena_looks_hooks`; the addresses in
  [MegaMan's colours](../../rom/megaman-colours.md), NetBattles):
  - the pick's end (`BN6_LOOK_PICK_END`, `0x0801009E`): where it chose
    palette 0, 1 or 4 for MegaMan in no form, the side's 0x19 + 6n, 0x1A
    + 6n or 0x1E + 6n (n the side) instead. A Cross's palette comes from
    BN6's form table, Beast Out and a Soul draw sprites of their own, and a
    tint never comes in a run: those pass untouched;
  - the form's second reader (`BN6_LOOK_FORM_READ2`, `0x080BF516`): an
    object drawn as MegaMan by his side's form, in no form the side's plain
    palette (a Soul there is still `soullook.c`'s);
  - the face's copy (`BN6_LOOK_FACE_COPY`, `0x0801CC2A`): face 0, his own,
    copied from the window's owner's side's face palette.
- **Held equal.** The two looks are folded into the pair's hash
  (`arena_looks_hash`) beside the installs' record, so two games given
  different looks part at the ready's hash, before the battle's first frame
  ("the two games went apart before the battle"), not at a later frame
  where the palettes are first drawn. A kept pair is gone into again only
  for the same two looks (`arena.c`'s key). The hooks keep nothing between
  calls: a rollback puts back the cores and the palettes they copied, and
  the looks come back with them.

## What a NetBattle shows of him

| What | Whose look |
| --- | --- |
| Both MegaMen on the field, plain, pale (a hit's flash) and Full Synchro | Each his own player's, on both GBAs |
| The emotion window (the battle and the Custom screen) | Each GBA's own player's face: mine on my screen, his on his |
| A Cross, a Soul, Beast Out | BN6's (or BN5's, BN4's) own colours, as in a run |
| The Custom screen's chips, the HP box, the result | No MegaMan drawn |
| A chat box's mugshot | None in a NetBattle |

## Where the looks come from

Neither game knows the other's look until the Square's wire carries it
(looks-square's protocol version). Both games must install the same two
looks or none, so until then a NetBattle is fought in BN6's colours on both
sides (`arena_looks_bout` with nothing given), and the dev switch gives
them: `--dev arenalook=MINE/THEIRS`, twelve hex digits each (a `Look`'s
six bytes in its order: the three channels' palettes, then their colours;
`FF` a channel as BN6 draws it). `--dev arena` takes MINE for the first GBA
and THEIRS for the scripted second; in the Square each game names its own
first. Once the wire carries looks, MINE is the profile's worn look and
THEIRS the one the Square knows the peer by.

- **The stands** rebuild the pair from both players' keys: until the
  wire carries both players' looks to a spectator, a watched battle with
  looks would part from the players' hashes, so the stands set none, and
  the players set none either without the dev switch.
- **Team NetBattles** fight one bout at a time between two games: each
  bout takes its two fighters' looks the same way.
- **Two MegaMen a side** (`--dev arena2v2` and the Square's 2v2) keep
  BN6's colours: four players need four plain palettes and four faces
  more, and the block has eight spare palettes in all.

## Measured

On the Linux host, `--dev arena,arenaboth` with seed 7, 24 November 2026
(`CYBERWORLD_ARENA_LOOKS_PROBE`, `arena_looks.c`'s probe: each frame every
OBJ palette slot of palette RAM compared with the eight spare palettes,
and the hooks' calls counted by core, alliance and kind):

- **The spares are never drawn by BN6.** Probe 1 marks all eight and sets
  no hook: through the way in and a battle fought to its end (3,653 frames
  on each core) none of them was ever in OBJ palette RAM on either core.
  BN6's own code agrees: the pick adds only 0, 1 or 4 to a form's or a
  tint's base, the form table's bytes are 2, 5, 7, 9, 0xB, 0xD, 0xF, 0x11,
  0x13 and 0x15, and the tints' bases 0x17, 0x1C, 0x21 and 0x26.
- **Who is who.** With two looks and the hooks (probe 2), the face's copy
  met the window's owner on alliance 0 on the first GBA and on alliance 1
  on the second, 5,677 times each; the pick's end met alliance 0 5,879 and
  alliance 1 5,674 times on each core alike.
- **Rollback.** With `arenabusy,arenadelay=1,arenalag=5` the battle rolled back
  489 times (2,445 frames run again), every one of its 54 hashes agreed
  and it ended as it does straight. `--dev rollbench` with both looks: 11
  rollbacks of 8 frames checked, the game's state the same in all 11.
- **Two games.** `online-arena/netbattle-looks` (two games over a local
  server, each naming its own look first) fights to its end with every
  hash agreed; its mutation `netbattle-looks-apart`, Beta given another
  look for Alpha, stops both games before the first frame ("the two games
  went apart before the battle", the state's `arenastop apart`), so the
  proof can fail. The rest of `online-arena` passes unchanged.
