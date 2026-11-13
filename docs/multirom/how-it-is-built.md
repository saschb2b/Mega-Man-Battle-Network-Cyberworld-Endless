---
type: Engine Mechanism
title: "How it is built"
description: "Extra ROMs found by game code and SHA-1 (`src/core/rom.c`), their maps learned and tile sets copied into BN6's free space (BN4's 256-colour ones shown so), areas dressed by `run_dress` or held for review, songs copied by `xsong.c`, and the extra ROMs offered on each platform (BN4's whole since 5 November 2026)."
tags: [multirom, bn5]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-13T22:09:57Z }
---

# How it is built

1. **Extra ROMs** (`src/core/rom.c`, `XR[]`): found in the ROM's folder by
   their header's game code and SHA-1, read and never run
   (`xrom_find`). Each has its layout (`XRomLayout`): its tables, found by
   their structure (`tools/rom_tables.py`) and noted in [ROM data](../rom/overview.md).
   A layout marked `research` is read only when a dev tool sets
   `xrom_research` (the atlas, the differential's guest, `--dev
   showbn4`), so a run, the ROMs screen and the 3DS's heap never meet a
   ROM that lends nothing yet, and its areas are shown only to a dev tool
   reviewing them (`xarea_review`: `--net-biome xN`, the atlas). No game
   is so marked today: BN4 Red Sun's mark went on 5 November 2026, when
   the owner turned BN4 on whole ([Battle Network 4 in a run](bn4.md)),
   its ROM offered, its families installed and its areas dressing BN6's
   with their themes and battle backgrounds. One area can still be held
   alone (`NetAreaDef.held`, the walk's gate), which `run_dress` passes
   over.
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
   BN4's are 256-colour, to `0xCE00`, which BN6's net leaves room for
   below its chat box's tiles: while a layer in them shows, BG1 and BG2
   are set 256-colour (`netmap_colours`, [BN4 Red Sun's tables](../rom/bn4.md), Net maps in 256 colours).
   The map's backdrop and animations come with them (`xbackdrop_map`):
   the learned map's backdrop (its record, tiles, map and palette) and
   its GFXAnim scripts (colour cycles, the backdrop's turning tiles) are
   copied into `FREE_MAP_BACKDROPS`, and the taken-over map's entries in its
   group's three tables (animation list, backdrop record, scroll
   callbacks) point at them; a layer of BN6's own puts BN6's back. BN4's
   records are 9 words to BN6's 7 (`XRomLayout.bg_words`) and its
   LoadBGAnim is its EnterMapGroup's fifth call, both read so.
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
   with the ROMs present, so nothing is saved (`run_dress`: a coin per
   area, BN5's by the bits 16-30 of one word as ever, BN4's by a second
   word's, BN5's first where both dress one BN6 area, so with all three
   BN4's dresses such an area in a quarter of the runs, and one BN5 has
   none for in half): a run continued without
   the other ROM goes on in the BN6 area's tiles.
6. **Platforms**: desktop first. Every place a player brings ROMs takes
   the extra ROMs as data, the list `XRomId` names, each offered or not
   (`xrom_offered`): one whose layout is marked `research` is neither
   read, shown, copied nor named for a player until the mark goes, which
   is then the one line that offers it everywhere; `--dev showbn4` (the
   browser's `?dev=showbn4`) shows such a one for review. BN4 Red Sun's
   mark went on 5 November 2026, so its cartridge, pickers, file and
   found lines are a player's on every platform. The
   ROMs screen (`src/launcher/`) has a cartridge for BN6's and one each
   extra ROM offered, its label in its game's colours and its face from
   its own ROM (`XRomLayout.face`: BN5's Colonel, BN4's Duo), three side
   by side in a compact size where a small screen has no room for more;
   the desktops' file dialogs name the game a cartridge wants. Phones
   keep copies: Android's `RomLook` (told the codes offered by
   `pick_android.c`) and iOS's `src/core/ios.m` (its table read from the
   layouts: game code, SHA-1, the copy's name), behind the ROMs screen's
   pickers, copy the extra ROMs offered beside BN6's from the folder the
   player chose, and look in it again at each start, so BN5 put there
   later comes in (issue #66). On the desktops, Android and iOS, R on the
   title opens the ROMs screen again to add one (issue #97). The
   browser's play page (`web/play/app.js`) takes BN5's and BN4's files
   beside BN6's, each told by its header's game code and checked by its
   SHA-1, and keeps them in IndexedDB beside BN6's (`rom/bn5c.gba`,
   `rom/bn4rs.gba`), where
   `xrom_find_beside` reads it (issue #68); it asks the engine which extra
   ROMs it offers (`cw_rom_offered`) and hides the others' cartridges,
   refusing their files as another game's. The New 3DS reads one extra
   ROM offered whole, BN5's before BN4's, beside BN6's or in the 3DS's
   other ROM folders (`src/core/start_3ds.c`), where its 96 MB heap has
   room for it: mGBA's 3DS setup had taken a 16 MB ROM buffer it never
   used, which the game now gives back, so with BN5's 8 MB read the
   heap's fixed part is 8 MB smaller than the 3DS's with BN6 alone was
   ([The 3DS's memory](../engine/emulation/3ds-memory.md)). Two extra ROMs do not fit: modelled
   with BN5's and BN4's both read, 9 of 12 long runs had allocations
   refused (73, among them maps' mirror images and raised views not
   learned), where BN4's alone had none and BN5's alone 18 in 6.
   A second extra ROM, or one a 3DS short of room cannot hold, is left
   unread, the run going on without it, and its log and PET say so.
