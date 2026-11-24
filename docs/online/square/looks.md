---
type: Feature
title: "Peers' looks"
description: "Each player's MegaMan look in the Net Square (issue #854, protocol v30): sent as LOOK after each WELCOME and as it changes, carried in SNAPSHOT, the peers drawn in it through one copy of BN6's field sprite with a palette a peer slot, as many looks as the part has free OBJ palette slots (the plaza three besides one's own), the nearest first, the rest in BN6's own colours; `peer_looks_of` for the Arena."
tags: [online, megaman, looks]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-24T21:00:00Z }
sources:
  - id: issue
    resource: "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/854"
    title: "Issue #854: MegaMan's look, and step 1's answer (points 4 and 5)"
    author: human:saschb2b
  - id: colours
    resource: "docs/rom/megaman-colours.md"
    title: "MegaMan's colours: the field sprite, the OBJ palette slots, peers' looks"
---

# Peers' looks

A player's [look](../../game/looks.md) (his MegaMan's colour channels)
travels with his name, and the other players' MegaMen in the Square are
drawn in theirs, under one limit BN6 sets: how many sets of colours a
frame can hold.

## On the wire

Protocol v30 ([the wire](../../../shared/online/README.md), Protocol v30):
the game sends LOOK, the look's six bytes, after each WELCOME (a visit
begun, a resume) and again as the look worn changes (`square_looks_send`,
from `online_scene.c` through `square_looks_begin` and `square_looks_update`); the server bounds it by its own numbers, keeps the
latest and puts it last in each SNAPSHOT's peer (56 bytes). A player
whose game never sent one is all `look_none`: BN6's own colours. The
server never knows which palettes a player owns, and an older game or
server is told which side is behind as for any version
([The server and the wire](../server-and-wire.md)).

## Drawn

Every peer's puppet (`online_square.c`) draws list 6's spare number
`0x59` (`BN6_LOOK_FIELD_SPARE`), which the Square points at a copy of
BN6's field MegaMan made as it is installed (`square_looks.c`): his
header and the frames of his 73 animations copied into the map's room
(5.6 KB), each frame's offsets turned to reach BN6's own tiles and
objects from the copy, its palettes a block of the copy's own: BN6's
colours first, then one palette a peer slot. Each frame the slot's
puppet is given its palette (the NPC object's PaletteIndex, which BN6
sets into its sprite every frame) and the palette the colours of his
look (`look_palette_of`) or BN6's own. Only colours change: the frames,
the walking and the poses stay BN6's, so emotes draw as before. The
Square shows forms on nobody, so a Cross or Soul a peer wears there has
nothing to keep; his look returns with him wherever his plain body
shows.

## As many as the part has room for

BN6 gives a frame's sprites their OBJ palette slots by content and
leaves a sprite past the free ones in a stale slot's colours
([MegaMan's colours](../../rom/megaman-colours.md), The OBJ palette
slots). So the engine counts the looks itself (`square_looks_pick.c`):
the part MegaMan stands in has room for 3 besides his own in the plaza,
9 in Trade Central, 10 on the stage (measured in step 1; the Arena's ring
taken as the plaza's until measured). The nearest peers get theirs; a
peer in MegaMan's own look or in one drawn already costs nothing; the
rest are drawn in BN6's own colours, which take one of the places where
MegaMan wears a look of his own. The pick is made again when a peer
comes or goes, MegaMan's look or part changes, and otherwise every 90
frames, a peer keeping his look against one up to 24 world units nearer,
so two peers at one distance do not trade it back and forth. Should BN6
still mark a peer's sprite stale (an effect more than measured), the
part's room is one look smaller for ten seconds.

## For the Arena

`peer_looks.c` keeps the look each player of the Square was last seen in,
by session: `peer_looks_of(session, &look)` gives it, BN6's own where
he was never seen (the state's `peerlooks` line reads it the same way).
A game's own side is `square_looks_sent`, the look the others know it by. A NetBattle's pairing reads its opponent's there; the
two sides are drawn from it ([Looks in a NetBattle](../arena/looks.md)).

## Checked

`tests/scenarios/online_looks.py`: three games, two peers in looks of
their own seen by the third and a look changed there seen by the others;
five games, the plaza past its room, the farthest in BN6's own colours
and his look back as he comes nearest, no peer stale. The state's
`peerlooks NAME P.P.P/C.C.C drawn|classic, ...`, `looksent`, `lookroom` and
`lookstale` lines say it. `tests/test_online.c` holds the pick and the
wire's bounds, `server/tests/test_looks.py` the server's.
