---
type: Engine Mechanism
title: "How it is built"
description: "Extra ROMs found by game code and SHA-1 (`src/core/rom.c`), their maps learned and tile sets copied into BN6's free space, areas dressed by `run_dress`, songs copied by `xsong.c`, and BN5's ROM on each platform."
tags: [multirom, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-13T00:52:19Z }
---

# How it is built

1. **Extra ROMs** (`src/core/rom.c`, `XR[]`): found in the ROM's folder by
   their header's game code and SHA-1, read and never run
   (`xrom_find`). Each has its layout (`XRomLayout`): its tables, found by
   their structure (`tools/rom_tables.py`) and noted in [ROM data](../rom/overview.md).
2. **A map from any of them** (`area_src_load_x`): the map reader takes
   its maps from the game's ROM; what learns from a map (`tiles_learn.c`,
   `props.c`, `decor.c`, `stairs.c`) works on the decoded map, whichever
   game it came from, and each map keeps where it was read (`AreaSrc.rom`).
3. **Into BN6**: another game's areas (`XRomLayout.areas`, the same
   `NetAreaDef` as BN6's own, numbered after BN6's 19) name the BN6 map
   their layers take over (`over`) and the BN6 area whose layouts and
   furnishings they take (`like`). A layer in another game's tiles copies
   that game's tile set (its two blocks decoded and encoded again, after a
   header of its own) and palette into BN6's in-memory free space
   (`+0x170000`, [Running the game's own logic](../engine/emulation/overview.md)), and the taken-over map's descriptor
   points at them. BN5's tile sets fill the same VRAM as BN6's (their
   first block at 0, the second after it: 31200 bytes against 30496).
   The map's backdrop and animations come with them (`xbackdrop_map`):
   the learned map's backdrop (its record, tiles, map and palette) and
   its GFXAnim scripts (colour cycles, the backdrop's turning tiles) are
   copied into `+0x310000`, and the taken-over map's entries in its
   group's three tables (animation list, backdrop record, scroll
   callbacks) point at them; a layer of BN6's own puts BN6's back.
4. **Pools by source**: each area, town, song and bystander is an entry
   with the game it came from; BN6 alone is today's game.
5. **Runs follow the games there**: an area of another game dresses the BN6
   area it is like (`run_dress`): its layouts, guardian and battles are
   that area's, its tiles, name and theme its own. A theme (`xsong`, a song
   in its game's song table) is copied into BN6's free space with what it
   plays on (`src/audio/xsong.c`): its sequence with the pointers moved,
   the voices it selects, their key splits, drum kits and samples, and
   entered in one of BN6's 61 empty song slots, where BN6's own sound
   engine plays it as one of its own; its battles name its game's battle
   themes, copied the same way (`encounter_song`), and its areas' battle
   backgrounds (`encounter_backdrop`). The run's seed picks it,
   with the ROMs present, so nothing is saved: a run continued without
   the other ROM goes on in the BN6 area's tiles.
6. **Platforms**: desktop first. Phones keep copies: Android's
   `RomLook` and iOS's `src/core/ios.m`, behind the ROMs screen's
   pickers (`src/launcher/`), copy BN5's ROM beside BN6's from the folder
   the player chose, and look in it again at each start, so BN5 put there
   later comes in (issue #66). On the desktops, Android and iOS, R on the
   title opens the ROMs screen again to add BN5 (issue #97). The browser's
   play page (`web/play/app.js`) takes BN5's file beside BN6's, either
   told by its header's game code and checked by its SHA-1, and keeps it
   in IndexedDB beside BN6's (`rom/bn5c.gba`), where `xrom_find_beside`
   reads it (issue #68). The New 3DS reads BN5's file whole as well,
   beside BN6's or in the 3DS's other ROM folders (`src/core/start_3ds.c`),
   where its 96 MB heap has room for it: mGBA's 3DS setup had taken a
   16 MB ROM buffer it never used, which the game now gives back, so with
   BN5's 8 MB read the heap's fixed part is 8 MB smaller than the 3DS's
   with BN6 alone was ([The 3DS's memory](../engine/emulation/3ds-memory.md)). A 3DS short
   of room leaves BN5's ROM unread, a run of BN6's, and its log and PET
   say so.
