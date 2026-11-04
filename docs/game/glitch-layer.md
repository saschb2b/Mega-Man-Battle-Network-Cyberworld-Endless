---
type: Design
title: "The Glitch layer"
description: "A rare side layer behind a glitching warp: a normal layer of its area that comes apart the further MegaMan explores it, other areas' looks breaking through on the same floor, its data pulled from deeper in the Net, and a copy at its heart that glitches between guardians; designed with the game-design skill and the owner's calls of 4 November 2026."
tags: [game, design, glitch]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-04T17:00:00Z }
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

A warp like the dark warp (its sprite, BN6's overworld object), never in
the Undernet's purple: its colours turned one of three ways, and every
three to seven seconds it glitches for a handful of frames, its rows torn
sideways sprite and all, and comes back in other colours (drawn over the
frame, `glitch.c`). BN6's own NPC commands could carry more of it
(verified in bn6f, `asm/npc.s`): `0x2D` mosaic, "the 'blur' effect when
viruses spawn in a battle", `0x2B` a colour shader mixed with its
palette, `0x2F` a squash and rotation, and `ns_set_sprite` another warp's
look (the Secret Area's gate): left for later.

Its read ([a fork's read](../world/level-design/fork-read.md)): at A,
MegaMan senses, never names: "W-Whoa... Lan,this warp's wrong. It keeps
changing!", where its exit leads, and the question. At the exit pad,
nothing: the warp is its own fork.

## The layer, and how it comes apart

The Glitch layer is a normal layer of its area and depth: a fresh layout
from its own seed, its services, locks and secrets rolled as any layer's
(the Net Dealer and a heal at their usual odds for a side layer, its
purple data, gaps and hidden paths), its exit pad open from the first
step. What is new is its soul:

**Its stage** is the share of its Mystery Data taken, read from BN6's own
flags (so a CONTINUE keeps it), in five stages: 0 on arrival, 1 at the
first data, 2 at 30%, 3 at 55%, 4 at 80%. Its data lie at the ends of its
detours, so the layer comes apart the further MegaMan explores it, and
each data taken is the push-your-luck's one decision: a better find, a
worse glitch. The stage never falls. (The first rule, the share of the
floor come near, was measured and dropped: a straight walk to the exit
came near 57 to 97% of the floor on ten seeds, the autopilot's walk
reaching stage 2 to 4 without a detour; by the data taken it stays at 0.)

| Stage | Its look | Its battles | Said |
| --- | --- | --- | --- |
| 0 | the layer's own; now and then a band of rows torn sideways for a few frames | the layer's depth, as any layer's | on arrival, three boxes |
| 1 | tears more often; blocks of the picture slipped by a tile | as 0 | |
| 2 | **flips**: the whole layer shown in another area's look for a moment, under BN6's mosaic in and out, as the title changes its backdrops; colours turned in bands | a visitor of another area or net in each battle | "The floor turned into another area!" |
| 3 | flips longer and more often | an act deeper | "The viruses here... They're getting stronger!" |
| 4 | flips every second or two | as 3 | "Something's at the middle of all this!" |

**The flips are BN6's own renderer.** A layer is a BN6 map whose tile
set, palette and tile map the engine writes (`netmap.c`). As the map is
entered BN6 copies its palette's 13 banks into its palette buffer
(`palette_3001960`), decompresses its two tile sets into VRAM at their
headers' offsets and its tile map into EWRAM (`BN6_MAP_TILEMAP`,
`0x02013A00`), where it stays and is drawn from as the camera moves
(verified in bn6f, `decompAndCopyMapTiles_8030472`, and by the flips
themselves: [the net maps' rows](../rom/net-maps.md)). As the Glitch
layer begins, the engine picks up to three other areas' tiles for its
floor as it stands (`netmap_look`: the areas that draw the run's this
run, BN5's among them where its ROM is there, the layer's own props left
out; two on the 3DS), and a flip writes a look's tile sets and colours
into VRAM and the palette buffer in one director step and its tile map
into BN6's buffer and the screen maps in the next (`glitch_look.c`): the
frame between, one area's tile map read with the other's tiles, is the
corruption a GBA really shows, kept as the flip's edge. The way back
writes what the look covered, read before it came. Sprites, MegaMan, the
NPCs and the walls never change; the palette's banks reach the backdrop
too, whose sky turns with the floor; BN6's map animations keep cycling
the layer's own colours over the look, a glitch of their own.

**The tears, slips and turned colours are the frame's**, drawn after BN6
has drawn it as the dark way's portal is recoloured (`liberation_way.c`):
the frame alone changes, the backdrop, the sprites (told by the OBJ
palette's colours) and a chat box's rows left whole, decided in the
update and read in the draw (`glitch.c`).

**Its data** stand where any layer's do, their content as data of a
deeper layer by how far off the way they lie, decided as the layer is
made (`NetObj.deep`): on the way the layer's own, on a detour an act
deeper, at a detour's far end or past a set piece's lock two acts
deeper, and the farthest of them two acts deeper on any layout; the
[code lean](economy/opening-plan.md) as everywhere.

## The copy at its heart

The run's guardians are the Nest's copies ([the rival](rival.md)). At the
Glitch layer's heart, the farthest end of its detours, stands one that
went wrong: copies written over each other. On the layer it is its first
form's body, its colours turned and torn as the warp's are. Talked to, it
names what its data holds (the three chips) and asks; it never blocks
the exit and never moves.

**How it fights: the true swap** (the owner's call 2). One BN6 battle,
its forms one after another on the same panel, one pool of HP (one and a
half of the act's guardians' band, a share a form): when a form's share
is gone it glitches out in BN6's own sprite mosaic, uncounted, and the
next arrives as BN6 brings a Navi into a fight under way, its HP plate
listed; deleting the last wins the battle as BN6 wins any, ENEMY
DELETED! and the results (`src/emu/battle_swap.c`, `glitch_copy.c`;
how, address by address, in [BN6's Navis](../rom/navis.md), A Navi
swapped mid-battle). No Custom screen between forms. The forms: the
guardians the run has beaten, the last first (the copy is made of what
MegaMan has fought), then the one that last deleted MegaMan in any run
(`rivals.sav`), then guardians the run set in areas off its way; BN6's
own Navis alone, never a super boss nor the act's guardian ahead
(`run_glitch.c`). Where the swap refuses a plan, its first form alone at
the pool's HP.

The three ways the brief allowed, weighed before the call:

| | Forms as battles cut together | A true swap inside one battle (built) | Sabotage along the layer |
| --- | --- | --- | --- |
| What it is | three BN6 battles cut into one fight by the glitch | the Navi's object replaced mid-battle by another's on the same panel | the copy crosses the layer, forcing battles, trapping data, cutting ways |
| In BN6's engine | the established hooks: the draw, the spawn's HP, a forced battle | BN6's mid-fight spawn and its enemy counts, four hooks idle until armed | NPC scripts and forced battles; no battle change |
| Fun | a boss in phases, each form a new read | the same, without a seam | tension, but paid in walking, the core loop's first lack, and in danger the player did not choose |

**What it pays** (the owner's call 3): a pick of one of its forms' Navi
chips, a version above what the act's guardians are fought at (SP at
most), in the folder's codes (the glitch rewrote its code), on the pick
screen the Server's prize uses, B leaving them with the copy, which logs
out once one is taken. A BN prize (a Navi beaten gives his chip) and a
decision about the folder, not a roll. With the copy deleted the glitch
settles: the layer back in its own look, no more glitches.

## The words

Every line through [the voice rules](../voice/rules.md), in the feature's
words files (`glitch_words.c`, the arrival and the act card in
`story_words.c`, the pick in `reward_words.c`): the warp's (three boxes
and the question, its exit's way said as the Undernet's warp says its
own), the arrival (three), one a stage from 2, the copy's (MegaMan's,
and its own cut words in a box with no face), the pick's opening. MegaMan
senses, Lan reacts; neither names what they have not seen.

## As built (4 November 2026)

- `LAYER_GLITCH`, a side kind (`net.h`), its seed `run_layer_seed(depth,
  LAYER_GLITCH)`; which layer holds the warp from the run's seed
  (`run_glitch_at`, `run_glitch.c`); the warp in a dark warp's place
  (`net_services.c`, from the seed, not the rolls, so no other layer
  changes), on a spur's first panels or a room the way crosses, six panels
  at least from the arrival (`net_gen.c`). `LAYER_MAKE` 110. No `Run`
  change: the side kind is `run.side_kind`, the stage comes back from
  BN6's data flags, the copy's outcome is event flags `0x147A` and
  `0x147B` ([a layer](../engine/emulation/layer.md)).
- The warp: the dark warp's sprite, its colours turned one of three ways
  that change with each glitch, torn every three to seven seconds, drawn
  over the frame.
- Dev: `--dev glitch` (the warp on every act's first layer from the
  second), `glitchstage=N`, `side=4`, the `glitch` step and `--talk
  glitch`/`copy` ([the dev menu](../devtools/dev-menu.md)).

**Checked:** `build.py test` (`test_glitch`: one run in three over 3,000
seeds, never two, on an act's first layer from the second; the warp and
no dark warp on its layer, away from the arrival; the Glitch layer's
copy, its exit back, its farthest data two acts deeper; the forms), `build.py
lint`; headless: the warp glitching on its layer, the Glitch layer at
each stage (its flips, tears and words), the copy's talk, its fight by
the true swap over three forms to the results, and its pick of three;
the autopilot through Glitch layers on ten seeds to the next layer and
on (stage 0 at their exits, no data taken).

**Left for later:** the copy's body switching between its forms on the
layer (BN6's `ns_set_sprite` and mosaic, `0x25` and `0x2D`); its record
and first meeting as "???", a title's mark, Dad's mail; the backdrop and
the theme flipping with the floor; the statistics' choices (Glitch layers
entered, stage reached, the copy fought and beaten); another net's Navis
among the forms once the swap is proven on their deletion; a Custom
screen between forms, if play asks for it; the bands in `build.py
pacing` and `build.py economy` for the copy and the deeper data; a run
on a New 3DS.

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
