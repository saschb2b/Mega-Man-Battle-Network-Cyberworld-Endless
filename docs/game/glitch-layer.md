---
type: Design
title: "The Glitch layer"
description: "A rare side layer behind a glitching warp: a normal layer of its area that comes apart the further MegaMan explores it, other areas' looks breaking through on the same floor, its data pulled from deeper in the Net, and a copy at its heart that glitches between guardians; designed with the game-design skill and the owner's calls of 4 November 2026."
tags: [game, design, glitch]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-04T15:00:00Z }
sources:
  - id: owner
    resource: "The owner's brief, given in the project's sessions on 4 November 2026"
    title: "The owner's brief for a Glitch layer"
    author: human:saschb2b
  - id: bn6f
    resource: "~/.cache/mmbn-ref/bn6f, asm/npc.s: NPCCommand_set_color_shader (0x2b), NPCCommand_set_mosaic (0x2d), NPCCommand_set_transform (0x2f)"
    title: "bn6f, BN6's NPC script commands"
  - id: code
    resource: "src/scenes/scene_title.c (the backdrops' mosaic), src/director/liberation_way.c (a portal recoloured on the frame), src/director/battle_spare.c (a battle ended as BN6's draw), src/net/net_services.c (the dark warp's roll), src/director/director_state.h (D.seen)"
    title: "What the engine already has"
---

# The Glitch layer

The owner's brief (4 November 2026): a new, rare fork variant, as
Liberation Missions are one. Its portal glitches now and then and changes
its look; the layer glitches more the further MegaMan explores it, layers
glitched into each other showing through; hacky, BN-like animations, the
feeling "what is happening here". It keeps a normal layer's workings and
secrets with a new soul. Its guardian glitches between a few guardians
mid-fight or sabotages MegaMan along the layer. It needs a reason to go in
and a reward worth it.

This page is the design, run through the game-design skill and checked
against [the core loop's review](core-loop.md), with the owner's calls
at the end.

## The experience

> "Something is wrong with this layer. The further in I go, the more it
> comes apart, and every step shows me more and pays more, until I find
> what is doing it."

The aesthetics, in MDA's terms: discovery first (a place that is not what
it should be), chosen challenge second (the deeper, the harder), sensation
third (the picture itself breaking). The dynamics that make them: a
glitch that grows with the floor explored, finds and battles that grow
with it, and an exit that never closes, so every step in is a choice.

## The dialectic and the loop

- **The dialectic** is the run's own, *press on or hold back*
  ([the meta layer](meta/overview.md)), here as greed against stability,
  restated at three scales: the warp (go in, knowing only that something
  is off), each detour on the layer (the farther from the way, the deeper
  the glitch, the better its data and the harder its battles), and the
  heart (fight the copy at the glitch's peak, or leave with what was
  found).
- **The loop layer** is the layer's (the walk, the data, the battles on
  it), feeding the run's (the act's folder: the finds and the copy's
  prize in the folder's codes); the meta only by memory (the copy
  remembers MegaMan), never by power.
- **Against the core loop**: it is Phase 3's "risk the player chooses on
  the layer" in one piece (a layer modifier and an optional second
  guardian, the long design post's asks), "variety a veteran feels" for
  the hour-twenty player, rare enough that the main path's pace is
  untouched: the floor's shape and MegaMan's walk never change, and only
  a player who goes in pays its time. No currency, no gathering, nothing
  carried to the next run but the record.

## The patterns

- **An honest telegraph that still unsettles.** Every look the glitch
  shows is the same floor: other areas' tiles drawn on the layer's own
  shape, so the picture lies about where MegaMan is, never about where he
  can walk. Its strength is on screen (the glitches' frequency and reach)
  and said once a stage; the battles' change is said before it bites.
- **Branching map, one chamber ahead** (Hades' doors): the warp is unlike
  any other on the Net, its kind read, its contents not.
- **Push your luck, fractally**: the run's dialectic restated by the
  floor's depth off the way, the [Server](progression/structure.md)'s
  "press on" spread over a whole layer.
- **Handcrafted units, generated layout**: the layer is the generator's,
  the areas' tiles Capcom's, read from the ROMs; what is new is which
  area's tiles a panel shows, and when.
- **Meta as variety, not power**: the copy's record and the title's
  marks; nothing that makes the next run easier.

Smells checked:

- *Dominant strategy:* "always go in" is held off by its price (an extra
  layer's time, attrition before the act's guardian, battles a band up
  deep in), "never go in" by its pay (data up to two acts deeper, a pick
  of three); "go deep, skip the copy" stays a real line, since the exit
  is always open.
- *Complexity over depth:* one rule to learn ("the deeper in, the worse
  the glitch: better finds, harder battles") and one new fight. No new
  lock kind, no meter on screen beyond the picture itself.
- *Words:* the main path's budget holds: a box or two at the warp, on
  arrival, at each stage and at the heart; the copy's lines are few and
  cut.

## Where it stands in a run

**Recommended: a warp on a layer** (the owner's call 1). The run's seed
decides, as it decides an act's [dark layer](liberation/overview.md):

- One run in three holds one Glitch layer, never two (the owner's call 4).
- From act 2 (a new system meets a player opt-in: this one is behind a
  Yes), on the act's first layer: the short net's acts 2 and 3, the
  endless net's 2 to 5 (act 6 is the deep's own).
- Never on a layer that holds the Undernet's dark warp, an act's dark
  layer or a guardian.
- Its warp stands on a short spur off the way, in view of it, never in
  the arrival's room: half of all players skip half the data
  ([the statistics](core-loop.md)), and a rare thing they walk past is
  wasted.
- Its Yes starts the same departure the Undernet's dark warp starts, to a
  side layer (`LAYER_GLITCH`) of the same depth, in the same area, its
  exit leading on to the act's next layer: the fork is the rest of this
  layer and its exit, or the Glitch layer and its exit.

The alternative, a glitching portal among Lan's HP's ways with the way's
second layer glitched, makes it an act-level choice beside Liberation's
dark way, the act's areas and guardians changed with it. The warp keeps
the act's areas and their guardians as they are, meets a player on the
layer as a find (the discovery the experience starts from), and reuses
the side layer's plumbing whole (`run_layer_seed`'s side, the departure,
the exit to the next layer).

**Beside the other forks:**

| | The Undernet's dark warp | A dark layer (Liberation) | The Glitch layer |
| --- | --- | --- | --- |
| Where | a warp on 12% of layers from layer 4 | an act's second layer, behind one way at home | a warp on one layer, one run in three, from act 2 |
| Needs | BN6 | BN5's ROM | BN6 (more looks with BN5's) |
| Its layer | the Undernet's | BN5's board on the area's floor | the area's own, a new layout |
| Its pay | tougher viruses, better data | BN5's par tiers, a route | data from deeper, the copy's pick |

## The warp

A floor object like the dark warp, in the area's colours, that every few
seconds (the seed's, three to seven) glitches for a handful of frames and
sometimes comes back as something else: another warp's look of BN6's own
(the dark warp, the Secret Area's gate: overworld objects of sprite list
7, `layer_objs.c`), changed by the NPC's own `ns_set_sprite`.
BN6's own NPC commands carry the glitch itself (verified in bn6f,
`asm/npc.s`): `0x2D` mosaic, "the 'blur' effect when viruses spawn in a
battle", `0x2B` a colour shader mixed with its palette, `0x2F` a squash
and rotation. Around it, on the frame only, a few rows torn sideways. Seen
from the way, the screen hitches as it comes into view.

Its read ([a fork's read](../world/level-design/fork-read.md)): at A,
MegaMan senses, never names: something like "Lan... This warp's wrong. It
keeps changing!", and the question. At the exit pad, nothing: the warp is
its own fork.

## The layer, and how it comes apart

The Glitch layer is a normal layer of its area and depth: a fresh layout
from its own seed, its services, locks and secrets rolled as any layer's
(the Net Dealer and a heal at their usual odds for a side layer, its
purple data, gaps and hidden paths), its exit pad open from the first
step. What is new is its soul:

**Its stage** is the share of its floor MegaMan has come near (`D.seen`,
which `run.seen` keeps for a CONTINUE), in five stages: 0 on arrival, 4
at about the far end of every detour. A straight walk to the exit should
reach stage 1 or 2 (to be measured on the generator); the heart only
opens to the full explorer. The stage never falls.

| Stage | Its look | Its battles | Said |
| --- | --- | --- | --- |
| 0 | the layer's own; now and then a band of rows torn sideways for a few frames | the layer's depth, as any layer's | on arrival, two boxes |
| 1 | tears more often; blocks of the picture slipped by a tile | as 0 | |
| 2 | **flips**: the whole layer shown in another area's look for a moment, mosaic in and out, as the title changes its backdrops | a visitor of another area or net in each battle; battle backgrounds of other areas | "The layer... It's turning into other layers!?" |
| 3 | flips longer and more often, BN5's areas among them where its ROM is there; the backdrop flips with the floor; the area's theme stutters into another's | the next act's band | "The viruses... They're getting stronger!" |
| 4 | the heart: flips every few seconds, layers half into each other; the copy seen | the next act's band, a version up (the Server's) | at the heart |

**The flips are BN6's own renderer.** A layer is a BN6 map whose tile
set, palette and tile map the engine writes (`netmap.c`). As the map is
entered BN6 copies its palette's 13 banks into its palette buffer
(`palette_3001960`), decompresses its two tile sets into VRAM at their
headers' offsets and its tile map into EWRAM (`BN6_MAP_TILEMAP`,
`0x02013A00`), where it stays (verified in bn6f,
`decompAndCopyMapTiles_8030472`; [the net maps' rows](../rom/net-maps.md)).
For each other look the layer shows, the engine picks that area's tiles
for the same layout (its classes, as any layer of that area), and a flip
writes its tile sets, palette and tile map to those three places between
frames, in the director's step; the way back writes the layer's own.
That BN6 draws the screen's tiles from that buffer as the camera moves,
so a flip shows at once and scrolls whole, is the first thing to prove
(a write in romlab; assumed until then). The frame or two where one area's tile map meets the other's
tiles is the corruption a GBA really shows, and is kept as the flip's
edge. Sprites, MegaMan, the NPCs and the floor's walls never change.
BN6's map animations (colour cycles) keep running on whichever palette is
in, a glitch of their own.

**The tears and slips are the frame's**, drawn as the dark way's portal
is recoloured (`liberation_way.c`): the frame alone changes, read-only to
the game, after BN6 has drawn it. The flips' mosaic is the canvas's
(`P.fx_mosaic`, the GBA's MOSAIC register as the title uses it).

**Its data** stand where any layer's do, their tier by how far off the
way they lie, decided as the layer is made: on the way the act's, at the
middle detours an act deeper, at the farthest two acts deeper (the glitch
pulls data from deeper in the Net), the [code lean](economy/opening-plan.md)
as everywhere. `build.py economy` gives the glitch layer a band of its own.

## The copy at its heart

The run's guardians are the Nest's copies ([the rival](rival.md)). At the
Glitch layer's heart (its farthest room, an arena as a guardian's) stands
one that went wrong: copies written over each other. On the layer it is a
Navi whose body switches between guardians' bodies (`guardian_body`), by
`ns_set_sprite`, mosaic and shader, seen flickering at the heart from
stage 3. Talked to, it fights. It never blocks the exit and never moves.

**How it fights** (the owner's call 2: the true swap, below; the forms as
battles were the recommendation, kept here as the fallback). One fight in
three forms, each a guardian, with one pool of HP:

- Each form is a whole BN6 battle against one guardian. When a form's
  share of the pool is gone, its battle ends as BN6's draw ends one, its
  banner passed by (the hooks the rival's netbattle already uses,
  `battle_spare.c`), and the next form's battle is forced at once, its
  HP set at the enemy's spawn (the netbattle's hook, `0x08007740`), the
  cut masked by the glitch (tears, mosaic, a torn white). MegaMan's HP
  goes on as it stands; each form opens with BN6's Custom screen, a fresh
  hand: a boss in phases. No battle of BN6's changes; the cut is between
  them.
- The forms: the guardians the run has beaten, the last first (the copy
  is made of what MegaMan has fought), then the one that last deleted
  MegaMan in any run (`rivals.sav`), then the act's pool; another net's
  Navi where his game is there. The pool is about one and a half act
  guardians' HP at the act's band (tuned in `build.py pacing`, a band of
  its own).

Weighed against the brief's other two:

| | One fight in forms (recommended) | A true swap inside one battle | Sabotage along the layer |
| --- | --- | --- | --- |
| What it is | three BN6 battles cut into one fight by the glitch | the Navi's object replaced mid-battle by another's on the same panel | the copy crosses the layer, forcing battles, trapping data, cutting ways |
| In BN6's engine | the established hooks: the draw, the spawn's HP, a forced battle | new RE: BN6's spawner called mid-battle, the old object freed, the new Navi's sprite loaded, the HP window; nothing to diff it against | NPC scripts and forced battles; no battle change |
| Fun | a boss in phases, each form a new read; the climax that bites | the most striking; the same, without the Custom screen between | tension, but paid in walking, the core loop's first lack, and in danger the player did not choose |
| Cost | days | weeks, and risk to every battle | days, and the "honest floor" lost if ways are cut |

The true swap stays the deepening once the forms have been played: the
same fight without its seams.

**What it pays** (the owner's call 3). Recommended: a pick of one of its
forms' Navi chips, a version above the act's guardians' (tuned in
`build.py economy`), in the folder's codes (the glitch rewrote its code),
on the vault's pick screen the Server's prize uses, B leaving them with
the copy. A BN prize (a Navi beaten gives his chip) and a decision about
the folder, not a roll. With the copy deleted the glitch settles: the
layer back in its own look and theme, the exit pad on.

**What it remembers:** its first meeting is "???", as a guardian's; after,
the record (met, beaten) in `rivals.sav`, and its opening line says which
forms it took last time. A title's mark for the first copy deleted.

## The words

Every line through [the voice rules](../voice/rules.md) and the review
dashboard, in a words file (`glitch_words.c`): the warp's (one box and the
question), the arrival (two), one a stage from 2 (three), the heart's (one
or two), the copy's (a few, cut and stuttering, made of guardians' kinds
of lines, never their quotes), the settling (one), and Dad's one mail
after the first copy deleted. MegaMan senses, Lan reacts; neither names
what they have not seen.

## What it asks of the engine

- `LAYER_GLITCH`, a side kind (`net.h`), its seed `run_layer_seed(depth,
  LAYER_GLITCH)`; the run's Glitch act from the seed (`run.c`, beside
  `run_dark_area`); the warp's roll on numbers of its own (`net_services.c`,
  from the seed, not the rolls, so no other layer changes). `LAYER_MAKE`
  bumped for the warp's place. No `Run` change: the side kind is
  `run.side_kind`, the stage comes back from `run.seen`, the copy's
  outcome is an event flag in the state.
- The flips: a second tile pick for the same layout per look (no ROM
  write), the looks' sets in free space (`free_space.h`), the director's
  writes between frames, the layer's own written back on a load.
- The frame's glitches in the post-draw step beside the portal's tint;
  nothing in a draw call changes the game.
- The copy's fight on the netbattle's hooks; its prize on the vault's
  screen; the statistics' choices counted (layers entered, stage reached,
  the copy fought and beaten).

## The owner's calls

Decided on 4 November 2026:

1. **Where it stands:** a warp on a layer, a detour to the next layer, as
   recommended (not chosen: a glitching portal at Lan's HP, the way's
   second layer glitched).
2. **The copy's fight: the true swap first**, inside one battle: the
   copy's Navi replaced on its panel by the next form's, one BN6 battle
   from the first form to the last (chosen over the recommended forms as
   battles cut together, and over sabotage along the layer). The forms
   chained as battles stay the fallback only if the swap cannot be made
   safe for every battle.
3. **The prize:** a pick of one of its forms' Navi chips in the folder's
   codes, as recommended.
4. **How rare, and BN4's looks:** one run in three, never two; BN4 Red
   Sun's areas among the flips only once they are admitted to runs
   ([Battle Network 4 in a run](../multirom/bn4.md)), as recommended.

## The true swap: where it starts

BN6 already spawns an enemy mid-battle: `BN6_SPAWN_ENEMY_1` and
`BN6_SPAWN_ENEMY_0` (`0x08007696`, `0x080076A0`), BN5's Chaos Unison
failure's twin, which the Souls track uses to bring the Dark Soul in
([BN6's Navis](../rom/navis.md)). The spawn's hook sets an enemy's HP
(`0x08007740`); a Navi's init loads its sprite (`0x08016F57`). What a swap
asks beyond them, to locate and prove: the old form's object taken off
without its deletion (no explosion, no win, no reward pick), the next
form spawned on its panel first so the battle never stands without an
enemy, the enemy HP window and the targeting following the new object,
and the glitch over the cut (BN6's own sprite mosaic, `ObjectSprite`'s
mosaic bit; a Navi's log-in frames, [BN6's battles](../rom/battle.md)).
