---
type: ROM Map
title: "MegaMan's colours"
description: "Where BN6 Gregar keeps MegaMan's colours (his battle, field, mugshot and emotion-window palettes), which indexes form his shade ramps, where a form takes them over, and how the sprite renderer shares its OBJ palette slots: the measuring behind each profile's look (issue #854)."
tags: [rom, graphics, megaman, look]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-24T21:00:00Z }
sources:
  - id: rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6, read in tools/romlab and the engine"
    title: "The player's BN6 ROM"
  - id: bn6f
    resource: "https://github.com/dism-exe/bn6f"
    title: "The Cybeast Falzar disassembly (the sprite renderer, sprite.s and asm38.s)"
  - id: mmkb
    resource: "https://megaman.miraheze.org/wiki/Style_Change"
    title: "MMKB: Style Change (BN2, BN3)"
---

# MegaMan's colours

The measuring behind each profile's look, epic #854 (its step 1): where
BN6 keeps the colours of MegaMan's plain body, which of them a recolour
can take, where a form takes them over, and how many differently
coloured MegaMen one screen can show. The addresses are named in
`src/emu/bn6_look.h`; the forms' tables they meet are in
[MegaMan's forms](souls.md), the sprite lists and their layout in
[Sprites, fonts and pictures](graphics.md). No colour of BN6's is kept in
the tree: a recolour is computed from the palette in the core's ROM copy.

## Where his colours are

| Data | Offset | How it was found / verified |
| --- | --- | --- |
| In battle (`BN6_LOOK_BATTLE_WORD`, `BN6_LOOK_BATTLE_SPRITE`, `BN6_LOOK_BATTLE_PALS`, `BN6_LOOK_BATTLE_PAL_COUNT`) | sprite list 0's entry 0: its word `0x08031CEC`, the sprite at `0x081D8000` (raw), palette 0 at `0x081DE160`, 43 palettes of `0x20` (`BN6_LOOK_PAL_SIZE`) | The sprite list read as `gfx.c` reads it, each frame's palette offset followed: every frame of the 24 animations names the one block. Palette 0 is OBJ palette slot 0's 32 bytes in a battle (`tools/romlab` on a battle's state and the engine's `dump`), and the PET's MegaMan status screen draws the same sprite and palette. The block holds his own palette (0), its pale twin (1, `BN6_LOOK_PAL_PALE`), Full Synchro (4, `BN6_LOOK_PAL_SYNCHRO`), each Cross's pair (the form's palette by `BN6_FORM_PALETTES`, then its pale twin) and four tints by NaviStats `+0x10` (`BN6_LOOK_TINT`; palettes `0x12` + 5n, `BN6_LOOK_PAL_TINT_BASE`, `BN6_LOOK_PAL_TINT_STEP`: 0x17, 0x1C, 0x21, 0x26 with their pale and Full Synchro ones). Palettes 0x19-0x1A, 0x1E-0x1F, 0x23-0x24 and 0x28-0x29 are zero and never picked (the chooser below only adds 0, 1 or 4 to a tint's base). No writer of NaviStats `+0x10` was found (every `bl` to the NaviStats setters `0x08013690`-`0x08013800` with r1 set to `0x10`: none, only reads): assumed unused in play. |
| The palette his body takes (`BN6_LOOK_PALETTE_PICK`) | `0x0801002C` | Read in full (capstone): in no form or Beast Out (forms 0, 0xB, 0xC) palette 4 at Full Synchro (NaviStats `+0x0E` 0xFF) else 0, then in no form the tint added; in a Cross the form's byte of `BN6_FORM_PALETTES` (`0x080203EA`: 2 HeatCross, 7 ElecCross, 9 SlashCross, 0xD EraseCross, 0x13 ChargeCross; Falzar's five 5, 0x11, 0xB, 0xF, 0x15); palette 1 where his AI data `+0x48` has `0x200` (its setter not traced). It has 14 callers, every object drawn as MegaMan by his form. |
| On the field (`BN6_LOOK_FIELD_WORD`, `BN6_LOOK_FIELD_SPRITE`, `BN6_LOOK_FIELD_PALS`, `BN6_LOOK_FIELD_PAL_COUNT`) | sprite list 6's 0x37 (`PUPPET_MEGAMAN`): its word `0x08032270`, the sprite at `0x0842FD60` (raw), palette 0 at `0x08437C34`, 5 palettes | As above; palette 0 is OBJ slot 0 on a layer and in the Net Square, where every peer's puppet draws the same sprite and shares the slot (the engine's `dump`). The palette is the byte `0x02000AA5` (`BN6_LOOK_FIELD_INDEX`, bn6f eStruct2000aa0 `+5`), set into his sprite at `0x0809E770` every frame (`BN6_LOOK_FIELD_INDEX_SET`); 0 in play, 1-4 set by cutscene scripts (bn6f `SetOWPlayerNaviPaletteIndex`'s callers; what each shows not looked at). |
| In a chat box (`BN6_LOOK_MUGSHOT_WORD`, `BN6_LOOK_MUGSHOT_SPRITE`, `BN6_LOOK_MUGSHOT_PAL`) | sprite list 8's 0x37: its word `0x08032674`, the sprite at `0x084ED738` (raw), its one palette at `0x084EDD1C` | The mugshot's id 0x37 is not in `BN6_MUGSHOT_BANKS`, so it draws in bank 0: OBJ slot 13 (`BN6_OBJ_SLOT_MUGSHOT`), its 32 bytes this palette's (the engine's `dump` with MegaMan's line on a layer and on the PET's status screen). Beast MegaMan's face (0x58) is a sprite of its own. |
| The emotion window (`BN6_LOOK_FACE_WINDOW_FIRST`, `BN6_LOOK_FACE_BLUE_FIRST`) | face 0's palette in `BN6_FACE_PALS` (`0x0872D050`), read through `BN6_FACE_PALS_REF` (`0x0801CD74`) | OBJ slot 12 in a battle and on the Custom screen (`BN6_OBJ_SLOT_EMOTION`), its 32 bytes face 0's. In the engine the faces' palettes are `soullook.c`'s copy, the word repointed (a recolour of BN6's own block did nothing until it followed the word). The window's border and fill are indexes 1 and 2 of the same palette: its right end, a 16x16 OBJ in slot 12, has no other index (VRAM's tiles counted); the face's helmet is 11-13. |

## The ramps, and how many channels the art has

Each art lays its 16 colours out its own way. Found by drawing every frame
of each sprite with one index group lit at a time and by counting which
indexes touch (a scratch decoder of `gfx.c`'s layout), then by hue, and
verified by the spike below:

- **Battle** (palette 0): 1 the outline; **2-5 light blue**, helmet, arms
  and legs (`BN6_LOOK_ARMOUR_FIRST`-`_LAST`); **6-9 dark blue**, the
  bodysuit (`BN6_LOOK_SUIT_FIRST`-`_LAST`; 6 repeats 2, the highlight);
  10 white glints, 11 the gold of the helmet's fins and the emblem's ring,
  12 the emblem's red, 13-15 skin. Every Cross's palette changes 2-9 alone.
- **Field** (palette 0): 1 the outline; **2-8 one seven-shade blue ramp**,
  2-5 on the armour, 6-8 on the suit (the two meet in the shading, and
  recoloured apart they stay clean); 9 white, 10 gold, 11 red, 12-15 skin.
- **Mugshot**: 1-3 the helmet's blue (two shades and a dark edge), 4-7
  the green behind him, 8 red, 11 gold, 9 and 13-15 skin, 12 black.
- **Emotion face 0**: 1-2 the window's, 11-13 the helmet's blue.

So the art separates **two blue channels** in battle and on the field
(armour and suit), and one in the mugshot and the emotion face (the
helmet: the suit is not in the picture). Gold and the emblem's red are a
handful of pixels each, shared by the fins, the emblem's ring and its
centre: a third and fourth channel would recolour specks. Skin and outline
are apart in every art. The index lists differ by art, so a look is one
list per art, not one per palette.

Hue alone finds the blue ramps (the indexes of hue 180-250 degrees and
saturation over 0.35, and the field's teal highlight 2): battle 2-9, field
3-8, mugshot 1-3, face 1, 2 and 11-13, where the face's 1-2 are the
window's. A recolour can therefore rotate a ramp's hue and keep BN6's own
saturation and value, which keeps the shading whatever the colour.

## Forms take their own colours

A Cross is MegaMan's own battle sprite in another palette of the same block
(`BN6_FORM_SPRITES` gives every Cross 0), so a recolour of palette 0 (and
1 and 4) leaves every Cross as BN6 draws it; Beast Out is sprite list 0's
0xB or 0xC with palettes of its own; a BN5 Soul draws BN5's sprite and a
BN4 Soul the united Soul's room (`FREE_BN4_SOUL_LOOK`), each with its own
palettes ([MegaMan's forms](souls.md), A Soul's look). The plain palette
comes back the frame the form is 0 again: `BN6_LOOK_PALETTE_PICK` runs
each frame. Verified by the spike: HeatCross worn in its own colours, the
next battle in the recoloured plain body.

## The OBJ palette slots

BN6's sprite renderer gives OBJ palettes by content each frame
(`BN6_OBJ_PAL_ALLOC`, IWRAM `0x03006108`, bn6f `sub_3006108`; its ROM copy
`0x081D6608`, the IWRAM code's at `BN6_IWRAM_CODE_ROM` `0x081D0500`, found
by the routine's bytes in an IWRAM dump): a sprite's palette, after its
colour shader, is compared with the slots already given this frame
(`BN6_OBJ_PAL_COUNT`, `0x0200F340`) and takes the first that holds the same
32 bytes, else the next. The work palette is `BN6_OBJ_PAL_WORK`
(`0x03001550`), copied through the fade to `BN6_OBJ_PAL_FADED`
(`0x03001750`) and palette RAM; a byte poked into it was in palette RAM
that frame and gone the next. The slots it may give are `BN6_OBJ_PAL_LIMIT`
(`0x020098A8`), set by the mode: 12 on the maps and the PET's screens
(`BN6_OBJ_PAL_MODE_FIELD`, `0x080027C4`), 10 in a battle
(`BN6_OBJ_PAL_MODE_BATTLE`, `0x080027D4`), 8 (`0x080027E4`,
`BN6_OBJ_PAL_MODE_EIGHT`) or 16 (`0x080027F4`, `BN6_OBJ_PAL_MODE_ALL`);
read 12 on a layer and in the Square, 10 in a battle. A sprite past the
limit is marked (its `+3` bit `0x10`) and keeps the slot it had, whatever
that slot holds now. The slots above the limit are fixed: a sprite's own,
`+0x15`'s high nibble, filled by `BN6_OBJ_PAL_FIXED` (`0x08002818`): on the
field 12 the HP box, 13 a chat box's mugshot, 14 the chat box.

Every MegaMan in the same colours costs one slot, however many there are.
Each look of its own costs one more. Measured in the Net Square (three
games on a local server, the engine's `dump`): the welcome plaza gives 9 of
its 12 slots (MegaMan and both peers in slot 0), Trade Central 3, the
stage 2. So the plaza can show **three looks besides the player's own**
before BN6 runs out; Trade Central and the stage nine and ten. A peer in
Classic beside a player in a look of his own is a look too.

## Peers' looks

The Square draws each peer's look through one copy of the field sprite
(`src/online/square_looks.c`; [Peers' looks](../online/square/looks.md)):

- **The frames** (`BN6_LOOK_FRAME_SIZE`, `_TILES`, `_PALS`, `_SUB`,
  `_OAM`, `_FLAGS`, `_LAST`): past the sprite's 4-byte header a u32 per
  animation, its first frame's offset; a frame is 20 bytes, +0 its tiles,
  +4 its palettes (a size word, then `BN6_LOOK_PAL_SIZE` each), +8 its
  sub-animation, +12 its objects, each an offset from the data past the
  header, and +18 bit `0x80` its animation's last (gfx.c's sprite layout,
  `xnavi_sprite_len`'s walk). List 6's 0x37 has 73 animations and 231
  frames, every frame's palettes at one offset (`0x7ECC`, whose size word
  reads 0x20), counted from the ROM. A copy of the header and frames
  elsewhere, its offsets turned by the distance (as u32s: the copy lies
  past the sprite, so they wrap), draws BN6's own tiles and objects, the
  renderer adding each to the sprite's address: seen in the Square's
  scenario, each peer walking and posing as BN6 draws him.
- **The NPC's palette** (`BN6_NPC_PALETTE`, the NPC object's `+0x16`,
  bn6f PaletteIndex): `npc.s` calls `sprite_setPalette` with it after
  `sprite_update` on every NPC every frame (`loc_809E658`), NPC command
  0x0F sets it and `set_sprite` sets it to 0 or its variation's; the
  renderer draws that palette of the frame's block. The engine writes it
  for each peer's puppet, its object found by `BN6_NPC_SCRIPT_AT`
  (`+0x4C`, bn6f AnimationScriptPtr: where its script stands, within the
  puppet's `PUPPET_BYTES`).
- **A free list-6 number** (`BN6_LOOK_FIELD_SPARE`, `0x59`): list 6
  (`0x08032194`, 96 words) points at the placeholder sprite `0x084DC040` at
  0x07, 0x28, 0x29, 0x30, 0x31, 0x35, 0x48, 0x4A, 0x4C-0x4E and 0x59
  (read from the ROM); `xnavi.c` takes all but 0x59 (0x30 its yardstick
  of an empty entry), so the copy takes 0x59. Its mugshot (list 8's 0x59)
  is the black one, never shown: a peer has no chat box.
- **The stale mark** (`BN6_SPRITE_PAL_STALE_AT`, `BN6_SPRITE_PAL_STALE`):
  the sprite state's `+3` bit `0x10` above, read on each peer's object
  (its sprite state at `BN6_OBJ_SPRITE_AT`'s high nibble x 16) as the
  engine's check that its count of the slots held.

Verify: `python3 build.py scenarios online_looks` (the state's
`peerlooks` line and `lookstale 0`, and its pictures).

## NetBattles

The Arena hashes each core's whole state (`arena_pair.c`, `pair_hash`:
`saveState` less the sound, and the installs' log). The ROM copy is not in
it, but the palettes copied out of it are (the work palette in IWRAM,
palette RAM, the slots given and each sprite's slot in EWRAM). A recolour
is therefore safe for the lockstep and the rollback only when both
players' games give each core the same palettes: written into the ROM copy
by the installs before the cores start (where `xlog_hash` already holds
them equal), from both players' looks. Both MegaMen of a link battle draw
the same sprite and `BN6_LOOK_PALETTE_PICK` reads no side, so one palette
recolours both; a look per side needs the pick to answer by the object's
side, into the block's spare palettes (0x19-0x1A, 0x1E-0x1F, 0x23-0x24,
0x28-0x29, never read by BN6) or a grown copy: an install both games make
alike, keyed by the core's side and not by which player is local. That is
how [Looks in a NetBattle](../online/arena/looks.md) does it, on these. A shared dive's field (one core, up to four MegaMen on side 0)
takes the same spares and the same two picks answered by seat instead of side
([The shared dive](../online/shared-dive.md), Looks on the field; issue #879):

| Data | Offset | How it was found / verified |
| --- | --- | --- |
| The spare palettes (`BN6_LOOK_PAL_SPARE`, `BN6_LOOK_PAL_SPARES`) | battle palettes 0x19, 0x1A, 0x1E, 0x1F, 0x23, 0x24, 0x28, 0x29 of `BN6_LOOK_BATTLE_PALS` | Zero in the ROM. Verified unread: the pick adds 0, 1 or 4 to a base, the form table's bytes and the tints' bases miss them (above), and all eight marked in both cores of a NetBattle (`CYBERWORLD_ARENA_LOOKS_PROBE=1`, way in and battle, 3,653 frames a core) were never in OBJ palette RAM. |
| The pick's end (`BN6_LOOK_PICK_END`) | `0x0801009E` | `adds r0,r4,#0; pop {r4,r7,pc}` at the end of `BN6_LOOK_PALETTE_PICK`, where every path but the other Navi's (`0x08010096`) meets with the palette in r4 and the object in r5 (capstone). A hook there met both sides' MegaMen on each core every frame of a NetBattle (5,879 and 5,674 calls). |
| The form's second reader (`BN6_LOOK_FORM_READ2`, `BN6_LOOK_FORM_READ2_SET`) | `0x080BF516`, its palette set `0x080BF51A` | `ldr r1,=BN6_FORM_PALETTES` after `bl 0x080136CC` read the side's form (r0) for an object drawn as MegaMan (`0x080BF4FC`), then `ldrb r0,[r1,r0]` (`BN6_FORM_PALETTE_READ2`, which `soullook.c` hooks) and `bl` sprite_setPalette (capstone; one of `BN6_FORM_PALETTES`'s four readers). Not met in a NetBattle of buster shots. |
| The face's copy (`BN6_LOOK_FACE_COPY`) | `0x0801CC2A` | `ldr r1,=` the slot after r0 = the faces' palettes (`BN6_FACE_PALS_REF`) + 0x20 x r4 (the face), then the 0x20-byte copy (capstone). r5 the emotion window, its `BN6_FACE_OWNER` the object whose face it shows: alliance 0 on the first GBA and 1 on the second, every frame (5,677 calls each), so an object's alliance is the cable's side on both GBAs. |


## The spike

A throwaway patch (`CYBERWORLD_LOOK_SPIKE=HUE`, never merged) rotated one
channel, the armour's ramp, to another hue in the ROM copy as the core
started: battle palette 0's and 4's 2-5, the field's 2-5, the mugshot's
2-3 and the emotion face's 11-13 (through `BN6_FACE_PALS_REF`). Captures
of the field, a chat box, the Custom screen, a battle, HeatCross worn and
the next battle showed the armour recoloured with BN6's shading and the
suit, skin and window untouched, HeatCross in its own colours and the
plain look back after it. Its pictures are on the pull request.

## What "shown everywhere" meets

- **The Square's plaza:** three looks besides the player's own (above).
- **The mugshot and the emotion face** are separate arts with their own
  index lists, and the face shares its palette with its window's frame.
- **BN6's own screens:** the PET's status screen draws the battle sprite
  (it follows); the start menu's MegaMan entry is an icon of its own, and
  the NaviCust and BN6's jack-in tunnel draw no MegaMan (captured).
- **A NetBattle** needs the per-side pick above; a palette on one core
  alone parts the hashes.

## The series' precedent

MMKB's Style Change page (BN2, BN3) says an element "changes MegaMan to
that color" and a style changes his shape (Guts's fist, Shield's arm,
Team's crest and boots); its BN2 and BN3 pages do not say whether the
sprite is reused. Verified there that colour and shape were separate axes;
that BN2 and BN3 swap palettes of one sprite is assumed. BN4's and BN5's
pages name no body recolour but Souls, Dark and Chaos.
