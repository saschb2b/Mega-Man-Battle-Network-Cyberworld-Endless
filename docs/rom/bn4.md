---
type: ROM Map
title: "BN4 Red Sun's tables"
description: "Addresses in BN4 Red Sun's ROM found by the shapes BN5 and BN6 share (its map, coordinate and song tables, chip records, chip and enemy names), how much of bn6f's code it shares, and what is not located yet."
tags: [rom, bn4, multirom]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-03T22:20:00Z }
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

The first look into Battle Network 4: Red Sun (USA) for
[Battle Network 4 in a run](../multirom/bn4.md): its tables found by the
shapes BN5's were found by ([BN5's maps, songs, text and objects](bn5-world.md)),
each checked on BN5's ROM as the control where BN5's address is known.
Read only, as research; no code reads any of it yet, and no `bn4.h`
names them until the first step of that plan does. Until then its
column is not an Offset column, so `tools/symbols.py` does not take
these for BN6's addresses ([Symbols](symbols.md)).

| Data | In BN4's ROM | How it was found / verified |
| --- | --- | --- |
| The ROM | game code `B4WE`, title `MEGAMANBN4RS` | The cartridge header (0xA0 title, 0xAC code), SHA-1 `a97e96a7da03abd70f7953328e511b9fa29179f1`, 8 MB. Blue Moon is `B4BE`, not looked at. |
| Map tables | `0x028304` the real world's MapBGDescriptor lists (18 groups, 0x00-0x11), `0x02834C` the internet's (9 groups, 0x80-0x88) | `python3 tools/rom_tables.py ROM` finds them as it finds BN5's and BN6's. Filled: real world 0x00-0x08 and 0x0E, internet 0x80-0x82 and 0x84-0x88. The lists lie one after another, so a group's maps are its pointer's distance to the next: real world 7, 3, 4, 6, 10, 4, 2, 4, 5 and 8 (0x0E); internet 0x80 16, 0x81 12, 0x82 16, 0x84 3, 0x85 4, 0x86 3, 0x87 4, 0x88 8. Every map of 0x80-0x88 drew whole in a Python copy of the engine's reader (`src/map/area_src.c`'s `map_desc`, `map_gfx`, `draw_layers`: the tile set's two LZ77 blocks, the palette past its 4-byte header, 4bpp map entries), kept out of git: 0x80-0x82 are one small comp map repeated (100 x 60 tiles; 0x80:7-9 a ring of walkways), 0x84-0x88 the 22 large net maps (136 x 88 to 168 x 118 tiles): 0x84 lime green, 0x85 teal, 0x86 and 0x87 lavender, 0x88 dark green circuits. Which BN4 area each is, assumed from the community's count of maps per area (Interordi's BN4 page: ACDC 3, Town 4, Park 3, Netfrica, Netopia, Sharo and YumLand 1 each, Undernet 6, Black Earth 2), which matches the groups' counts in order: 0x84 ACDC Area, 0x85 Town Area, 0x86 Park Area, 0x87 the four countries' areas, 0x88 the Undernet and Black Earth; to verify by warping into each in romlab. The scenario dungeons (Elec Tower, the toy robot, the meteor's computer) are not among them: not looked for. |
| Coordinate data | `0x028F04` the real world's, `0x028F4C` the internet's | `tools/rom_tables.py` (the empty groups as the map table's). Each list's length matches its group's maps: 0x00 7 words (`0x08028F70` to `0x08028F8C`), 0x80 16 (`0x08029044` to `0x08029084`). |
| Songs | `0x155FF8`: 457 entries of 8 bytes (a song header, two u16 player numbers), as BN6's | A table of song headers (tracks 0-16, a voice group pointer, a pointer per track) found by its shape; the same search finds BN5's at its known `0x155BF4` (372 entries). 33 songs of four tracks or more, songs 0x00-0x21 but 0x0E: its music; song 0 holds 7 tracks. That its sequences and voices are the MP2K data `src/audio/xsong.c` copies: assumed, as only 109 of the 407 functions of bn6f's MP2K (`asm/libs.s`) are found in BN4 by their bytes (373 in BN5), so its sound engine is another build and `tools/mp2k_verify.py` has to check a copied song against BN4's own playing. |
| Chip records | `0x0197EC`: 0x2C bytes each, ids 0-388 | Cannon's codes `00 01 02 1A` (A, B, C, \*) looked for at every word with each record size from 0x1C to 0x30: only 0x2C runs on, from `0x019818`, record 1 (BN5's search lands on its record 1, `0x01E23C`, `BN5_CHIP_RECORDS` + 0x2C). Cannon's power, 40, at +0x1A as BN6's (`BN6_CHIP_ATTACK_POWER`), its picture pointers at +0x20 on. +7, BN6's library type, holds 0 to 12 in BN4: another field. 385 of the 389 records have codes in range, a picture pointer and a name. |
| Chip names | text archives `0x7481D8` (ids 0-255) and `0x748C14` (256-388) | BN4's text spells letters and digits as BN6 and BN5 do (A `0x0B`, a `0x26`, digits from `0x01`; its dash `0x40`): "annon" found by its letters' spacing, then Cannon whole. An archive is a u16 offset per entry, then the strings, each ended by `0xE5`; the first's first offset 0x200 makes 256 entries, entry 1 Cannon. |
| Enemy names | text archive `0x74A780` (253 entries) | Mettaur found as Cannon was. Viruses 1-126 (their versions in a row: Mettaur, MettEX, Mettaur2 and on), the Navis from 128, four entries each (no mark, then the marks `0x59`, `0x5A`, `0x5B`), Duo at 224, Bass and BassXX at 228-230, the DS forms 240-252. The next archive (`0x74B0B0`) holds 24 blank entries. |
| Code shared with BN6 | none to name yet | `tools/bn6f_match.py`'s search with BN4 Red Sun in place of BN5 (its twins left out): 1,650 of bn6f's 13,646 functions located (12%; BN5 4,329, 32%), 1,023 by their bytes, and none of bn6f's RAM labels where Falzar keeps them (111 checked). By part: the map scripts' and NPCs' code about half to three quarters as in BN5, the battle engine's files a quarter to a third of BN5's share (bn6f's `asm03_0.s` 191 of 735 functions, BN5 421; `asm00_2.s` 133 of 980, BN5 459). Of the 69 of BN6's routines the moved BN5 families and Navis call (`src/emu/xvirus_families.c`, `src/emu/xnavi_recipe.c`), 46 are found in BN4 by bytes (59 in BN5). The full table stays in `.build/`, as bn6f's names do ([Symbols](symbols.md)). |
| Enemy ids table | not found | BN6's GetVerActorTyAndAIIdx (`movs r1,#3; muls r1,r0; ldr r0,[pc,#4]; adds r0,r1; mov pc,lr`), found byte for byte in BN5 at `0x08014C7C`, is not in BN4: its enemy ids, stats and battle records wait for a search of their own, as BN5's guest battles' first did ([BN5's battles, records and rewards](bn5-battles.md)). |
