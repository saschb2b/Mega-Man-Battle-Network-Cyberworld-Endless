---
type: Design
title: "The Glitch layer"
description: "A rare side layer behind a glitching warp: a normal layer of its area that comes apart the further MegaMan explores it, other areas' looks breaking through on the same floor, its data pulled from deeper in the Net, and a copy at its heart that glitches between guardians; designed with the game-design skill and the owner's calls of 4 November 2026."
tags: [game, design, glitch]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-21T15:30:00Z }
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

A pad on the floor, one of BN6's own overworld objects (sprite list 7:
the big framed warp pad 0x06, the green oval pad 0x22, the
orange-framed pad 0x36), never in the Undernet's purple: its colours
turned one of three ways. Every three to seven seconds it glitches for a
handful of frames, its rows torn sideways and, every other frame, split
by colour (its red drifting left, its blue right), and comes back as
another of the three pads under BN6's own sprite mosaic, clearing; between
its glitches, now and then a frame or two of a few rows slipped a pixel,
so it never quite holds still. The pad's switch and its mosaic are BN6's
NPC commands (`0x25` ns_set_sprite, `0x2D` ns_set_mosaic, verified in
bn6f, `asm/npc.s`): the director points its talker's idle loop at a
snippet that runs them, for a frame or two (`npc_talker_looks`); the tears
and the split are drawn over the frame (`glitch.c`). The dark warp's flame
(0x44) was its first look: BN6 draws it three frames in five (its one
animation's frames, seen in the gallery), a see-through flicker that made
the warp seem to drop frames.

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
| 2 | **flips**: the whole layer shown in another area's look for a moment, under BN6's mosaic in and out, as the title changes its backdrops; colours turned in bands | each battle a group of any area's or net's families ([Encounter groups](progression/encounter-groups.md); a visitor in each, before them) | "The floor turned into another area!" |
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
the layer's own colours over the look, a glitch of their own. Another
game's look comes in the colours its own layer shows from its map's
entry: the first frame of each palette script its learned map runs, read
from that game's ROM (`xbackdrop_palette_first`, `glitch_looks_make`),
laid over its stored palette. A bank its map stores blank is coloured
so: BN5's Undernet's skull totems ([Side pieces](../world/level-design/side-pieces.md))
stood black in its flips before (captured since: a flip into it, a totem
in its colours). The scripts do not run on a flip, so those banks stay at
that frame.

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
the exit and never moves. A later talk on the layer, after a No or a run
from its fight, is its data in one box and the question, and it fights
again ([Risk the player chooses](progression/structure.md)).

**How it fights: the true swap** (the owner's call 2). One BN6 battle,
its forms one after another on the same panel, one pool of HP (one and a
half of the act's guardians' band, a share a form): when a form's share
is gone it glitches out in BN6's own sprite mosaic, uncounted, and the
next arrives as BN6 brings a Navi into a fight under way, its HP plate
listed; deleting the last wins the battle as BN6 wins any, ENEMY
DELETED! and the results, each form in its own sprite, put first in the
battle's sprite cache by BN6's `sprite_decompress` (`src/emu/battle_swap.c`, `glitch_copy.c`;
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

**A form's leftovers** (21 November 2026, issue #733: a player saw
DiveMan's torpedoes cross MegaMan's rows while the copy stood as
JudgeMan). Measured over the pool, each of BN6's Navis taken off by the
swap at 200 HP under the autopilot (`swap ID:200/155:3000`, seed 15,
`CYBERWORLD_EMU_DEBUG`'s trace of the enemies' objects of the third
kind): five leave objects bound to them behind. DiveMan's torpedo
launcher (from the battle's first frame) went on firing two torpedoes a
volley about every 96 frames under JudgeMan until the battle's end;
ElecMan's 0x1A and TomahawkMan's 0x40 lived on the same way (not a flight:
each ends only at the battle's end); TenguMan's three
0x47 ended after 61 frames, ElementMan's 0x90 after 38. Shots already
loose and bound to no one (the torpedoes in the air, CircusMan's,
JudgeMan's own) finish within about 70 frames; ElecMan's two rods and
TomahawkMan's 0x41 are still objects of the field, with HP or none, and
spawn nothing. HeatMan, SlashMan, ChargeMan, SpoutMan, GroundMan,
DustMan, ProtoMan, BlastMan, CircusMan, JudgeMan, Colonel and AI 17 left
nothing bound (EraseMan was not taken off: his deleting beam ended the
run first, god mode or not).

The call, with the game-design skill: the copy is a boss in phases,
each form a new read, so the player reads the form on the field, and an
attack whose source has gone from the field is a telegraph that lies.
**What a form owns ends with it; what it already let fly finishes.** As
a form's dying begins, the third kind's objects bound to it (their
RelatedObject1Ptr) are ended as the battle's end ends them, each by its
own destroy (`src/emu/battle_swap.c`; [BN6's Navis](../rom/navis.md), A
form's own objects); shots in flight cross the field once and are gone
before or while the next form arrives, which MegaMan sees and can dodge
(the arrival never stops him), the glitch's seam; rods and stones stay
as BN6 leaves any obstacle. Chosen over leaving everything (the bug: a
form fighting on under the next) and over clearing every enemy object
at the swap (a cleaner slate, but it ends objects BN6 never ties to the
form and takes the seam away). Held by `tests/scenarios/battle_swap_leftovers.py`.

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
- The warp: a floor pad of BN6's switched between three looks at each
  glitch under BN6's mosaic, through its talker's idle loop; its colours
  turned, torn and split by colour over the frame.
- Dev: `--dev glitch` (the warp on every act's first layer from the
  second), `glitchstage=N`, `side=4`, `glitchlook=AREA` and `glitchhold`
  (below), the `glitch` step and `--talk glitch`/`copy` ([the dev
  menu](../devtools/dev-menu.md)).

## Checking an area's look

A flip is the layer's floor picked again in another area's tiles by the
same pass a layer of that area is picked by (`pick_map`: its stairs,
pads, ornaments, emblems, arrows, bushes, [side
pieces](../world/level-design/side-pieces.md), the safety net for holes
and the scenery), its props left out. So what an area learns to set
around its floors comes along in its flips wherever it is set inside
that pass. Two ways to look at it:

- **Without the game**, `python3 build.py atlas intoN` (`xN` another
  game's area, as `--net-biome` names them): a layer of every other area
  (a plain one at depth 2, and one up a stair where the area climbs
  them), each as built on the left and as a flip into N shows it on the
  right, `.build/atlas/look_intoNN_fromFF_dD_sS.png`, and a line each in
  its `report.txt`: the layer's stairs and rise, N's own stairs, ladders
  and rise, and what each pick set (scenery, side pieces) and the holes
  its safety net left, the layer's `->` the look's. `fromN` draws one of
  N's layers in every other area's look. The [atlas](../devtools/atlas.md)'s
  third field is the seeds per area (1).
- **In the game**, `--dev glitch,side=4,glitchstage=5,glitchlook=N`: a run
  that starts on a Glitch layer at stage 4, its looks from N first (more
  areas joined by `+`, `glitchlook=3+x0`); with `glitchhold` the first
  look stays on the floor, no other glitch over it, for a capture
  (`build.py shot --scene emu --run-depth 4 --dev
  glitch,side=4,glitchstage=5,glitchlook=N,glitchhold`).

## Heights in a flip

Twelve areas raise rooms and terraces on their own stairs
([Heights](../world/level-design/heights.md)), and a Glitch layer of one
keeps them. A look draws a stair from its own area's template of the
same way and width (`nm_paste_stairs`), and its raised floor where a
flat panel `rise` units up the screen would be. Into an area with no
such stair, or one of another rise or length, the ramp was left void
under MegaMan's feet and the raised rooms floated as islands on the
area's short faces (Green's stairs in Central's look, `build.py atlas
into0`, 15 November 2026): the picture lied about where he can walk.

So a look whose area cannot draw every stair of the layer as it stands
(`heights_drawn`, netmap.c: its way and width learned, at the layer's
rise and the stair's length) draws the raised floor and the stairs
level with the ground (`NetmapLook.level`): the rooms where their panels
lie, the ramps as walkways between them. Every panel MegaMan can walk is
drawn; he walks on at his own height over it, a rise above the picture
on a raised room, as the flip already lies about where he is and never
about where he can walk. A look into an area that draws the stairs keeps
them as they stand. The atlas's report says which (`drawn level`, `drawn
as they stand`).

## Who is in front in a flip

On a layer of their own area, MegaMan passes behind a side piece's claw
or horn: its layer priorities, cut with it, are written into the
coordinate data with the layer's own ([side
pieces](../world/level-design/side-pieces.md), Who is in front). A flip
changes the picture alone, so a look's pieces carry none, and the
layer's own pieces' priorities stay under the look. Measured on 15
November 2026 with a look held (`glitchhold`) and MegaMan put on the
panel a claw grips: on a Central layer in CopyBot Comp's look he stands
in front of the look's claw where its own layers would hide his legs;
on a CopyBot layer in Central's look the claw's own priorities hide him
behind nothing, the look's second tile layer being empty there, and he
is drawn whole.

**Decided (the game-design skill): a look's pieces stay without
priorities.** Writing them in step with the tiles means rewriting
section 2 of the coordinate data BN6 reads in place at `0x02027A00`, its
sections' offsets fixed in ROM, at every flip and back, for a draw order
seen for 10 to 130 frames on the few panels a piece grips. Draw order
carries no decision: walls and heights stay the layer's, so where
MegaMan can walk is never misdrawn, and a sprite drawn over the
scenery it should pass behind is a priority slip of the kind a GBA's
corruption shows, the glitch the layer is about. Revisit if a look's
piece ever stands on the way itself.

**Checked:** `build.py test` (`test_glitch`: one run in three over 3,000
seeds, never two, on an act's first layer from the second; the warp and
no dark warp on its layer, away from the arrival; the Glitch layer's
copy, its exit back, its farthest data two acts deeper; the forms), `build.py
lint`; headless: the warp glitching on its layer, the Glitch layer at
each stage (its flips, tears and words), the copy's talk, its fight by
the true swap over three forms to the results, and its pick of three;
the autopilot through Glitch layers on ten seeds to the next layer and
on (stage 0 at their exits, no data taken).

**Left for later:** its record
and first meeting as "???", a title's mark, Dad's mail; the theme of the
look's area played during a flip (a song begun on every flip started the
layer's own from its top again: the music drags instead, Glitchier); the statistics' choices (Glitch layers
entered, stage reached, the copy fought and beaten); another net's Navis
among the forms once the swap is proven on their deletion; a Custom
screen between forms, if play asks for it; the bands in `build.py
pacing` and `build.py economy` for the copy and the deeper data; a run
on a New 3DS.

## Glitchier (15 November 2026)

The owner asked for the layer "even glitchier", and took the three
recommended of the options weighed with the game-design skill: the
channels the glitch had not touched, each telling the player something
or costing nothing at the floor. Sprites flickering, words corrupted,
frames stuttering, a Mystery Data's colour as a timing gamble (it moves
the economy's bands) and two looks on the floor at once (VRAM's room
unmeasured) were left.

**Towards the heart** (`glitch.c`, `heart_near`): within 14 panels of
the copy (448 world units, walking across) the glitches come up to twice
as often, and the glitch shows a stage more from a third of the way in
and two from two thirds: a warmer and colder that leads to the heart.
The picture alone: its battles and MegaMan's words keep the layer's own
stage.

**Its sound** (`glitch_music.c`): while a look is on the floor the
music drags, BN6's own music player's tempo factor at 0x90 to 0xD0 of
0x100 (`BN6_MPLAY_TEMPO_U` and `_I`, as m4aMPlayTempoControl writes
them), and from stage 3 a flip opens on it stalled for 4 to 12 frames
(the factor 0: the sequence stops, its notes sound on). The song is
never restarted, a song begun since (a battle's) is left alone, and a
Glitch layer begun sets the factor back, should a state have been saved
while it dragged. Measured: the player's clock stands still through a
stall and runs on slower through the drag.

**Its sky** (`glitch_look.c`): a flip loads the look's area's backdrop
through BN6's own LoadBGAnimData (`BN6_LOAD_BG_ANIM_DATA`), as a map's
entry loads it: its tiles, BG3's map and palette bank 0; the layer's own
again as the floor comes back. Its record is BN6's own map's, or another
game's copied in (`xbackdrop_area_record`). The layer's own backdrop
animations keep running over it, a glitch of their own.

**Its copy** (`layer_objs.c`, `copy_looks`; `glitch.c`, `warp_look_next`):
the copy's body glitches between its forms' bodies, as the warp's pads
switch, through the same look snippet (`npc_talker_looks`, now carrying
the sprite's list and standing animation), at a block slipped or a
flip, never while a box is open: the fight it holds is shown before it
is chosen. Its forms' bodies are loaded after the layer's props; all
but JudgeMan's are uncompressed and take no room in the map's sprite
list, and a body that did not load is never switched to. The copy
never turns to MegaMan, as another form's sprite may draw none of the
facing BN6 would turn it to.

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

Taken on 21 November 2026 (issue #733), the owner's to turn over:

5. **A form's leftovers:** what a form owns (a launcher, a striker bound
   to it) ends as it glitches out; shots it already let fly finish their
   flight. Chosen over letting everything run on and over clearing every
   enemy object at the swap ([A form's leftovers](#the-copy-at-its-heart)).
