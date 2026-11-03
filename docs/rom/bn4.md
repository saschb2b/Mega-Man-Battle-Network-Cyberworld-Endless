---
type: ROM Map
title: "BN4 Red Sun's tables"
description: "Addresses in BN4 Red Sun's ROM, read for research alone: its map, coordinate, song and sprite tables, battle records and the encounter check that starts one, enemy and chip records and names, how much of bn6f's code it shares, and the romlab tool that checks them."
tags: [rom, bn4, multirom]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-03T23:50:00Z }
sources:
  - id: bn4-rom
    resource: "Mega Man Battle Network 4: Red Sun (USA), game code B4WE, SHA-1 a97e96a7da03abd70f7953328e511b9fa29179f1, 8 MB"
    title: "The owner's BN4 Red Sun ROM"
  - id: bn5-rom
    resource: "Mega Man Battle Network 5: Team Colonel (USA), SHA-1 5f472f78d8de2df01d5039e045c043cb40969a39"
    title: "The player's BN5 ROM, the control"
  - id: bn6f
    resource: "https://github.com/dism-exe/bn6f"
    title: "bn6f, the Cybeast Falzar disassembly"
---

# BN4 Red Sun's tables

Battle Network 4: Red Sun (USA), mapped for [Battle Network 4 in a run](../multirom/bn4.md):
its tables found by the shapes BN5's were found by ([BN5's maps, songs, text and objects](bn5-world.md)),
each checked on BN5's ROM as the control where BN5's address is known.
The names are `src/emu/bn4.h`'s and `XRomLayout`'s (`src/core/rom.c`,
`XROM_BN4_RED_SUN_US`); BN4's symbol files are written from them
([Symbols](symbols.md)). The engine reads BN4's ROM only when a dev tool
asks (`xrom_research`: the atlas's `x1`), as it lends a run nothing yet.

`python3 tools/bn4_lab.py` checks the tables below against the ROM
(report), plays it from power-on to Lan's room in romlab (boot) and
starts a battle there (battle): the encounter check patched to return a
record, the ROM's own (`--record 90:0:0`) or one written in the ROM's
free space with the enemies given (`--enemies 7@4,1 13@5,2`).

| Data | Offset | How it was found / verified |
| --- | --- | --- |
| The ROM | BN4 `0x0000AC` its game code | The cartridge header: title `MEGAMANBN4RS`, game code `B4WE`, SHA-1 `a97e96a7da03abd70f7953328e511b9fa29179f1`, 8 MB. Blue Moon is `B4BE`, not looked at. Its data ends at `0x7F739C`, the rest 0xFF. |
| Map tables (`XRomLayout.map_table_rw`, `map_table`) | BN4 `0x028304` the real world's MapBGDescriptor lists (6 groups, 0x00-0x05), `0x02831C` the net's (21, 0x80-0x94) | The map loader (`0x08026144`) takes `0x08028304` below group 0x80 and `0x0802831C` from it, by group - 0x80, as BN5's does (`tools/rom_tables.py` finds the run of lists; the split is the loader's). The lists lie one after another, so a group's maps are its pointer's distance to the next: the net's 0x80 2, 0x81 4, 0x82 5, 0x88 8, 0x8C 16, 0x8D 12, 0x8E 16 (one small comp map repeated), 0x90 3, 0x91 4, 0x92 3, 0x93 4, 0x94 8. The last five are the large net maps (136 x 88 to 168 x 118 tiles): by the community's count of maps per area (Interordi: ACDC 3, Town 4, Park 3, Netfrica, Netopia, Sharo and YumLand 1 each, Undernet 6, Black Earth 2) and the battles they field (below), 0x90 ACDC Area, 0x91 Town Area, 0x92 Park Area, 0x93 the four countries' areas, 0x94 the Undernet and Black Earth. 0x80-0x82's 2, 4 and 5 maps match the scenario dungeons' counts (Elec Tower 2, the toy robot 4, the meteor's computer 4): assumed. Verify: `build/host/cyberworld --headless --rom-dir DIR --atlas OUT:x1`, BN4 beside BN6 in DIR, draws every map with its walls, heights and triggers. |
| Coordinate data (`XRomLayout.coord_table_rw`, `coord_table`) | BN4 `0x028F04` the real world's, `0x028F1C` the net's | The coordinate loader (`0x08026790`) takes them as the map loader takes its tables. Each list's length matches its group's maps (0x00 7 words). |
| Songs (`XRomLayout.song_table`) | BN4 `0x155FF8`: 457 entries of 8 bytes (a song header, two u16 player numbers), as BN6's | A table of song headers (tracks 0-16, a voice group pointer, a pointer per track) found by its shape; the same search finds BN5's at its known `0x155BF4`. 33 songs of four tracks or more, 0x00-0x21 but 0x0E: its music. That its sequences and voices are the MP2K data `src/audio/xsong.c` copies is assumed, as only 109 of the 407 functions of bn6f's MP2K (`asm/libs.s`) are found in BN4 by their bytes (373 in BN5). |
| Sprite lists (`XRomLayout.sprite_lists`) | BN4 `0x02793C` | Ten pointers, the first at the table + 0x28 as BN5's (`0x03272C`), each to a list of sprite pointers; the first sprite's header `28 00 01 13 4C 00 00 00` is laid out as BN5's (`24 00 01 11 44 00 00 00`). Its categories not read yet. |
| eToolkit and the game state (`BN4_TOOLKIT`, `BN4_TOOLKIT_GAMESTATE`) | BN4 `0x02009E70`, its +0x40 | `ldr r0,=eToolkit; mov r10,r0` at `0x080060D8`, as BN5's at `0x08006010`. The game state at +0x40 (BN5's and BN6's at +0x3C), its +4 and +5 the map group and number, as the encounter check and the roll read them. |
| The encounter check and the roll (`BN4_ENCOUNTER_CHECK`, `BN4_ROLL`, `BN4_STORY_OFFSET`, `BN4_ENCOUNTER_RATES`) | BN4 `0x080F5370` the check, `0x080F5448` the roll, `0x080F54B8` the story offset, `0x080189E8` the rates | The check, called by the map loop at `0x080053E6`, runs on the net (group 0x80 on), counts steps, skips a map whose rate class is 7 (16 bytes per group at `0x080189E8`), rolls, stores the record at the game state's +0x2C (`BN4_GAMESTATE_RECORD`, which the battle's setup reads at `0x08006680`) and returns it; the map loop then begins that battle (`0x080053F4`). The roll reads `[[[BN4_BATTLE_TABLES + 4] + (group - 0x80) * 4] + (number + offset) * 4]`, the offset a routine per group (ACDC Area's `0x080F5568`: 0, 0x10, 0x20 or 0x30 by story flags), keeps the records whose +6 meets a progress byte (eToolkit +0x78's 0x17) and draws one by their weights (+0) with GetPositiveSignedRNG. Verified in romlab: the check patched to `ldr r0,=record; bx lr` starts that battle from Lan's room (`tools/bn4_lab.py battle`). |
| Battle records (`BN4_BATTLE_TABLES`, `BN4_RECORD_*`, `BN4_ENTITY_*`) | BN4 `0x08017FB8` | The roll's literal (`0x080F54B4`). Two pointers, the real world's groups and the net's 21; per group a pointer per map index, 16 a story stage (ACDC Area 64); per index a list of 12-byte records, 0xFF ending it (BN5's and BN6's are 16 bytes): +0 the weight, +4 the background (`0x08007ADE`), +6 the progress mask, +8 the entity list. An entity is 4 bytes: the enemy id, the panel's column and row, its kind (0 MegaMan, 1 an enemy, 5 an obstacle), 0xFF ending the list (BN5's and BN6's pack a panel in a nibble each and an id in a u16). Verified in romlab: ACDC Area's first record (`0x080FD65C`) fields two Mettaurs at its panels (5, 2) and (6, 3); a record written with enemy 7 at (4, 1) and 13 at (5, 2) fields VolGear (80 HP) and Billy (50) there; enemy 1 written with kind 5 is a rock. The net areas' lists (`tools/bn4_lab.py`'s report): 0x90 six families with AquaMan, GutsMan and SparkMan at SP, 0x91 eight with FireMan, NumberMan, Roll and TopMan, 0x92 ten with BurnerMan, VideoMan and WoodMan, 0x93 ten with ColdMan, KendoMan, ThunderMan and WindMan, 0x94 21 with Bass, JunkMan, LaserMan, MetalMan, ProtoMan, ShadeMan, SearchMan and the NavShado. |
| Enemy records (`BN4_ENEMY_STATS`, `BN4_ENEMY_*`) | BN4 `0x08011888`: 12 bytes per enemy id, ids 0-252 | Found by the HP read in romlab: battles forced with enemy 1, 3, 5 and 13 alone held 40, 100, 180 and 50 in one RAM word of the enemy's (`0x0203B27C` that time); the one table of the ROM with those at a fixed stride is this one. +0 element << 12 \| HP (BN6's first word's form: VolGear's `0x1050`, Fire, 80), +3 the kind (4 a virus, 8 a Navi), +4 the family or the Navi's number, +5 the version (0-5), +10 a u16 attack (Mettaur 10, Mettaur3 160). 120 viruses and 95 Navi records. GutsMan's first 500 HP and LaserMan's 1000 match the community's figures. BN6's GetVerActorTyAndAIIdx, found byte for byte in BN5, is not in BN4: its ids lead to these records instead. |
| Chip records (`BN4_CHIP_RECORDS`, `BN4_GET_CHIP`) | BN4 `0x0197EC`: 0x2C bytes each, ids 0-388; getChip `0x080190D0` | Cannon's codes `00 01 02 1A` (A, B, C, \*) looked for at every word with each record size from 0x1C to 0x30: only 0x2C runs on, from record 1 at `0x019818`; BN5's search lands on its record 1 (`0x01E23C`). BN6's getChip (bn6f) found in BN4 by its bytes, its literal this table. Cannon's power, 40, at +0x1A as BN6's; +7, BN6's library type, holds 0 to 12 in BN4. 385 of the 389 have codes, a picture and a name. |
| Chip names (`BN4_CHIP_NAMES`, `BN4_CHIP_NAMES_HI`) | BN4 `0x7481D8` (ids 0-255), `0x748C14` (256-388) | BN4's text spells letters and digits as BN6 and BN5 do (A `0x0B`, a `0x26`, digits from `0x01`); "annon" found by its letters' spacing, then Cannon. An archive is a u16 offset per entry, then the strings, each ended by `0xE5`. |
| Enemy names (`BN4_ENEMY_NAMES`) | BN4 `0x74A780`, 253 entries | Mettaur found as Cannon was: viruses 1-126 (Mettaur, MettEX, Mettaur2 and on), the Navis from 128, four entries each, Duo at 224, Bass and BassXX at 228-230, the DS forms 240-252; each name the enemy record of its id. |
| Code shared with BN6 | none to name | `tools/bn6f_match.py`'s search with BN4 in BN5's place (its twins left out): 1,650 of bn6f's 13,646 functions located (12%; BN5 4,329, 32%), none of bn6f's RAM labels where Falzar keeps them. The map scripts' and NPCs' code is found about half to three quarters as often as in BN5, the battle engine's a quarter to a third as often (bn6f's `asm03_0.s` 191 of 735, BN5 421). Of the 69 routines of BN6's that the moved BN5 families and Navis call, 46 are found in BN4 by bytes (59 in BN5). The full table stays in `.build/` ([Symbols](symbols.md)). |
