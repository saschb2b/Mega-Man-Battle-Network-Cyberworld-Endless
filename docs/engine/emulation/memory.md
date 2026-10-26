---
type: Engine Mechanism
title: "Memory the engine writes"
description: "The free space plan for the core's ROM copy past BN6's 8 MB, region by region from `EMU_FREE` (`src/emu/free_space.h`, `claims.c`), BN6's tables patched in place, and the EWRAM BN6 leaves alone (`RAM_*`)."
tags: [engine, emulation]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T20:30:00Z }
---

# Memory the engine writes

The core runs a copy of the ROM padded to 16 MB. Everything the engine adds
lives past the original data, from `EMU_FREE` (`0x08800000`), by one plan
that every part and every track of issue #82 follows, in offset order (a
span no row names is free). Issue #82's rows, from `+0x330000`, are fixed
places, the same at every boot with the same ROMs (a state's pointers into
them stay good): the virus families' room and the second-level dispatch,
the families, the chips, the Souls, the lava and sea, then room kept
for the Navis and the chips' moved code, each its own and none written
by another. The plan's numbers are `src/emu/free_space.h`
(`FREE_*`), which every writer below takes its place from;
`tests/test_core.c` checks its regions in order and apart and each
installer's own layout inside its region, and as the core starts each
installer's writes are claimed byte by byte (`src/emu/claims.c`: the free
space it fills, BN6's own words and instructions it changes, RAM it keeps
patched, the instructions its hooks stand on, and every other hook's), a
write or hook over another's bytes told on stderr (`CYBERWORLD_EMU_DEBUG`
prints the count: "claims: 105 ranges of 5 writers ..., 0 over
another's"). BN5's tracks' installs (`xinstall_core`) and the chip
registry's (`xchips_install`) and the Souls' are written down as they are made too
(`src/emu/xinstall_log.c`): every byte they write into the ROM copy and
every hook they set, in their order, which the Arena's cores take under
full rules ([Full rules](../../online/arena/from-square/full-rules.md)), on every system that
offers NetBattles; not in a New 3DS's runs, which offer none (there for
`--dev rollbench` and `--dev arena` alone, about 2 MB):

| Offset | Contents | Code |
| --- | --- | --- |
| `+0x0000` | Warp record and warp list for direct warps | `gamecall.c` |
| `+0x0100` | Where a game call's routine returns: a hook, which takes its r0 and r1 and goes back to the main loop (until issue #33 a stub of hand-written Thumb, with `0x0203FFF0`-`0x0203FFFB` past the game's EWRAM for its mark and results: free since, `RAM_TEAM_TICK` and spare) | `gamecall.c` |
| `+0x0180` | An empty Mystery Data list (12 zero bytes): every map but the layer's points at it, BN6's own too, as BN6 rolls every map's Mystery Data at each jack-in and clears their flags, some of them the run's (issue #122; [The net's maps and their pieces](../../rom/net-maps.md), Map takeover); the rest to `+0x0200` free (the encounter roll's wrapper and trampoline until hooks took their place, issue #29) | `mapslot.c` |
| `+0x0200`, `+0x0280` | Two battle records in turn: BattleSettings, `+0x20` its entity list (MegaMan's panel first, the foes, the field's objects) | `encounter.c` |
| `+0x0300` | Free (the PET's input step until a hook on the game's own handler took its place, issue #33) | |
| `+0x0400`-`+0x0640` | Rush's gaps (issue #14): the gap table from group 0x80 that the check's literals point at, a group's empty map list, the layer's group's maps, the layer's gap records, and handler 0x25's records (BN6's 13, then the layer's two) | `rush.c` |
| `+0x0700`-`+0x12E0` | Map-object handler 3's records (issue #42): BN6's 86, then the engine's Link Navi obstacles and cubes, 13 variants of 8 slots each (the last four another game's looks: BN5's Security Cube, its wall of dark flames two ways and its dark hole, a prop that never opens, [Other games' ROMs](../../multirom/overview.md)); its literal points here, written once a core | `blockers.c` |
| `+0x2F00` | The layer map's warp list (entry 1: the exit pad) | `mapslot.c` |
| `+0x3000`-`+0x10000` | Layer data in two halves, one per layer in turn: NPC lists and scripts, text archive, Mystery Data, sprite list | `mapslot.c` |
| `+0x10000` | Generated tile map (LZ77, literal blocks) | `netmap.c` |
| `+0x60000` | Generated coordinate data (walls, the exit pad's trigger) | `coords.c` |
| `+0x100000` | The town's tile map (LZ77, literal blocks) | `town.c` |
| `+0x130000` | The town's coordinate data (walls, section 2, the jack-in cells, the checks) | `coords.c` |
| `+0x140000`-`+0x148000` | The town's NPC scripts, text, lists, warps, objects, check table and map text archive, apart from the layers' | `mapslot.c` |
| `+0x148000`-`+0x14C000` | Lan's HP's ([Home](../../world/home/overview.md)): the courier's text archive (768 bytes, written again as what waits changes), the Net Square gate keeper's (1536 bytes, written again as the player's Square settings change), the gate's NPC script (its animation byte the director writes) and its keeper's, his and his mark's NPC scripts and the ways back's marks', its NPC list and empty map scripts, its own warp list (BN6's jack-out on its blue pad, the run's portals pointed by the director) and the map music list it adds to | `mapslot.c`, `lanhp.c` |
| `+0x14C000`-`+0x150000` | Lan's house's and room's: their empty NPC lists and map scripts, their own warp lists (BN6's doors copied in), the room's jack-in table and the music list | `mapslot.c`, `lan_house.c` |
| `+0x150000`-`+0x152000` | The director's conversations: one text archive, rewritten for each | `talk.c` |
| `+0x152000`-`+0x153000` | The map-name label's archive: a copy of the game's with its 244 names pointed at where the run is ("Layer 12", "ACDC Town"); its other scripts (the PET's HP, zenny and BugFrags, 0xF0-0xF2) kept | `director_layer.c` |
| `+0x153000`-`+0x154000` | The PET's PLACE: the same copy, its names pointed at the area and the layer ("JudgeTree 14") | `director_layer.c` |
| `+0x154000`-`+0x166000` | The PET's words ([The PET](../../game/pet.md)): the key items' names (+0x154000) and descriptions (+0x155000), the mails' senders and subjects (+0x158000) and bodies (+0x15A000), each BN6's archive rebuilt with the run's scripts in place of some | `pet_text.c` |
| `+0x166000`-`+0x167900` | Portraits of Falzar's Navis ([Guardians and super bosses](../../game/bosses/overview.md)): one mugshot sprite each, 0x500 apart (SpoutMan first), made from the Navi's battle sprite on the layer he guards; the mugshot table points at them from numbers empty in Gregar | `portrait.c` |
| `+0x168000`-`+0x169000` | A layer's Chip Trader's prizes of both nets (issue #82, [One net: the design](../../multirom/one-net.md)): on the older net's areas and in the deep, where BN5's ROM is read, the list its pool's entry points at, BN6's pool's records fitted to the folder's codes as a layer's are, then as many of BN5's catalogue's of each rarity (6-byte records, a 0 chip ending it); written as the layer's trader is installed | `trader.c` |
| `+0x169000`-`+0x16A000` | The PET menu's labels ([The PET](../../game/pet.md#options), Options): BN6's 90 tiles copied, Comm's eight set to Options from the labels' own letters and two more after them for its fifth column (`0xB80` bytes), which the PET's load list points at; written once a core | `pet_options.c` |
| `+0x16A000`-`+0x16B000` | The visit's requests on BN6's Request BBS ([5. Jobs (the Request BBS)](../../world/home/jobs.md)): the titles' archive, then the posts', each LZ77 (its literals alone, every flag byte 0) with the 4-byte header BN6's unpack of either expects, which the BBS's two words (`BN6_BBS_TITLES_PTR`, `BN6_BBS_POSTS_PTR`) point at; written at each visit and as a request is taken or settled | `request_bbs.c` |
| `+0x170000`-`+0x1A0000` | Another game's net area ([Other games' ROMs](../../multirom/overview.md)): its tile set, a header of its own before its two blocks encoded again, and its palette as it is; the map its layers take over points at them | `netmap.c` |
| `+0x1A0000`-`+0x260000` | Other games' songs ([Other games' ROMs](../../multirom/overview.md)): each song's sequence with its pointers moved, the voices it selects, their key splits, drum kits, samples and waves (a piece two songs share once); BN6's empty song slots from 0x26 to 0x3F point at them (issue #82's shared allocations: the slots above, 0x40-0x62, are the sounds the other games' families, Navis, Souls and chips copy, each BN5 sound at a fixed one). Every song the other games' areas play is copied the first time one is needed, in the areas' order, then their games' battle themes, so each has the same place in every session (a checkpoint's state holds the music player's pointers into it) | `xsong.c` |
| `+0x260000`-`+0x2A0000` | Other games' Navis ([Other games' ROMs](../../multirom/overview.md)): an overworld sprite and its mugshot each, copied whole (their offsets are their own); Gregar's list 6 and mugshots point at them from a number Falzar's Navis have there (72, 74, 76-78). Copied, as the songs are, all at once in the areas' order. After them the other games' map objects (sprites of their list 7), all at once the first time one is needed, each at a list-7 number of its own that points at the placeholder `0x084DC040`: BN5's flame of darkness (a DarkChip's, [The meta layer](../../game/meta/overview.md)) at 160, decompressed and its palette turned purple, its Security Cube at 146, its wall of dark flames at 138, decompressed, a dark hole at 135. Its last `0x12000` bytes (from `+0x28E000`) hold a guardian of another game's ([BN5's Navis](../../game/bosses/bn5-navis.md)): his sprite and face, copied anew on each layer he guards and listed at Gregar's 78, which no area's Navi takes | `xnavi.c` |
| `+0x2A0000`-`+0x2E0000` | Other games' battle backgrounds ([Other games' ROMs](../../multirom/overview.md)): each one's BGAnimData record, its tiles and map (LZ77 encoded again), palette, animation scripts and the tiles and lists their frames name; then copies of BN6's three background tables (records, animations, scroll entries) with them after BN6's 22, which the loader's literals point at. Copied all at once in the areas' order | `xbackdrop.c` |
| `+0x2E0000`-`+0x2E1000` | BN6's own flame of darkness ([BN6's own DarkChips](../../game/meta/bn6-darkchips.md)): its blue flame of sprite list 7 (`0x3C`, 1612 bytes) copied whole, its palette turned purple as BN5's flame is, and listed at Gregar's list-7 `0x54`, which points at the placeholder in the player's ROM; a place of its own, the same with or without another game's ROM, copied once a core | `xnavi.c` |
| `+0x310000`-`+0x320000` | Other games' net maps' backdrops and animations ([Other games' ROMs](../../multirom/overview.md)): for each of their areas, its learned map's BGAnimData record with the backdrop's tiles and map (LZ77 encoded again) and palette, and the map's GFXAnim scripts with the colours, tile lists and tiles their frames name (a palette script's RAM moved to BN6's). The map its layers take over has three entries of its group's tables pointed at them while one of its layers stands: its GFXAnim list, its BGAnimData record, and its 16-byte scroll entry rewritten with BN6's callbacks that do the same; every other layer writes BN6's own back. Copied all at once in the areas' order, as the first layer is built | `xbackdrop.c` |
| `+0x320000`-`+0x330000` | Home's other places ([10. AsterLand and the Cyber Academy](../../world/home/asterland-and-academy.md)), a part each: AsterLand's (`+0x320000`) and the Cyber Academy's four maps' (`+0x328000`): their NPC lists and their people's scripts and words, empty map scripts, their own warp lists (BN6's doors copied in, the doors out pointed where the planned town has them) and the music lists | `mapslot.c`, `indoors.c`, `aster_land.c`, `academy.c` |
| `+0x330000`-`+0x348000` | BN6's battle engine given the room other games' virus families take (issue #82, the engine's capacity; [Transplanted families](../../multirom/families.md); [Virus families moved into BN6's battle engine](../../rom/virus-families.md), The engine's room for moved families), written once a core as it starts, before any family's install and wherever one is installed, at fixed addresses (a state's pointers into them stay good; the ROM copy is not in a state, so every boot writes them the same): each of BN6's tables below copied with its entries as the player's ROM has them and the rest its fill, the one word of the ROM that names it pointed at the copy, each word checked first. `+0x330000` the eleven per-family tables, 256 AI indexes each, `0x400` apart (AI 0's entry in the rest); `+0x332C00` the ice blocks' sizes (`BN6_ICE_SIZES`), 256 bytes; `+0x332D00` the three tables of 35 words by AI index (`BN6_AI_HOOKS`, `BN6_AI_HOOKS2`, `BN6_HELD_CHIPS`), 256 words each, `0x400` apart, their triples' virus words pointed at them; `+0x333900` sprite list 3 (the attacks' sprites: MegaMan's sword, the chips' objects), 256 numbers (BN6's 105 as the ROM has them, the placeholder in the rest; grown once for every track, the chips' taking its numbers from `0xFF` down, `XV_LIST3`), `+0x333D00`-`+0x334000` free; `+0x334000` sprite lists 1, 2 (the Navis' battle sprites), 4 and 5, 256 numbers each, `0x400` apart (the placeholder sprite in the rest); `+0x335000` the effects (`BN6_EFFECTS`), 256 ids (effect 0's row in the rest); `+0x335400` the first, third and fourth kinds' types (`BN6_T1_TYPES`, `BN6_T3_TYPES`, `BN6_T4_TYPES`), 256 each, `0x400` apart (0 in the rest; T1 `0x5C`, T3 `0xD2` and T4 `0x91` the second-level dispatch's routines, below, each checked first to hold BN6's placeholder, never a family's); `+0x336000` the enemy ids' triples, `0x400` ids (each past BN6's `0x1C4` its filler `0xBB`'s; `+0x336C00`-`+0x337000` free); `+0x337000` the reward rows, `0x400` ids (each past BN6's `0x191` the filler's row); `+0x341000` the names of the ids from `0x200`, a text archive of 512 entries (16 KB), each an empty name until a family names it, which a hook on BN6's name lookup hands out ([Hooks](hooks.md)); `+0x345000` the second-level dispatch ([Transplanted families](../../multirom/families.md); [Virus families moved into BN6's battle engine](../../rom/virus-families.md), the second-level dispatch): MOVED, the moved routine (Thumb) for each of a game's types, 256 words for each of the first, third and fourth kinds, `0x400` apart, every entry the stray's routine until a family names its type; `+0x345C00` TWIN, BN6's own number for each type or `0xFF` where it is moved, 256 bytes a kind, `0x100` apart, `0xFF` until a family names it; `+0x345F00`-`+0x346100` its code, the project's own Thumb made field by field (`xvirus_dispatch.c`): the stray's routine, the three dispatch types' routines (`+0x345F10`, `0x20` apart) and the six spawners a moved family's calls go to (`+0x345F80`, `0x40` apart), `+0x346100` a virus's support chip run with BN5's support family's timing (`XV_SUPPORT`, the attack family 0x65: BN6's support routine twice in its first frame, [The chip registry and BN5's chips' code](../../rom/chips-code.md), A virus's support chip) and `+0x346120` the chip writer a moved family's code calls (`XV_CHIP_WRITER`: BN6's, its support family answered as 0x65), its side tables in RAM (BN6's RAM, below); `+0x346200` the families' pre-steps (`XV_PRESTEP`, issue #82's A2b: 256 words by AI index, 0 none, a word a family whose game's dispatcher acts for its ids before its state, Dominerd's and Appley's, its pre-step's Thumb address in its home, which a hook on BN6's dispatcher runs: [Virus families moved into BN6's battle engine](../../rom/virus-families.md), their shims and pre-steps); `+0x346100`-`+0x346200` and `+0x346600`-`+0x348000` free. The battle's sprite preload is made to read an id's 12 bits, as its spawn does (`0x080029CE`, `0x080029D0`; [ROM data](../../rom/overview.md)) | `xvirus_grow.c` |
| `+0x348000`-`+0x400000` | The families themselves (issue #82; [Transplanted families](../../multirom/families.md)): `+0x348000` their homes, 64 of `0x800`, one a family: the state table (BN6's layout, 16 entries at most), struct1 (`+0x40`), struct2 (`+0x48`), six empty held-chip lists (`+0x70`, where it holds none), its shims (`+0x130`, `XV_HOME_SHIMS`, the project's own Thumb from its recipe's numbers: its pre-step, its NameID ranges' and its chip compares'; [Virus families moved into BN6's battle engine](../../rom/virus-families.md), their shims and pre-steps) and its veneers (`+0x200`, 12 bytes each: `push {r0,r1}; ldr r0,[pc,#4]; str r0,[sp,#4]; pop {r0,pc}` and the target); `+0x368000`-`+0x380000` their code, block by block, each the same distance from a word boundary as in its game; `+0x380000`-`+0x3FBC00` what they draw, each sprite in a room of its own (an LZ77 one decompressed), and the sounds BN6 has no song of (a song packed whole, its sequence, voice group and samples, in a room of its own there, listed at one of BN6's empty song slots, a fixed one per BN5 sound whatever track copies it, the families' `0x60`-`0x62`: [Virus families moved into BN6's battle engine](../../rom/virus-families.md), their sounds). `+0x3FBC00`-`+0x400000` the viruses' arm points' records grown to every number of sprite list 1 (`XV_ARMS`, 256 of 0x44 bytes, the word of BN6's that names them pointed here: [Virus families moved into BN6's battle engine](../../rom/virus-families.md), their arm points), a family's at its battle sprite's number. Since issue #82's A2b every family BN5 has and BN6 lacks is installed, 25 of them: AI 32-56 with ids `0x200`-`0x295`, homes, code and art one after another in that order ([Transplanted families](../../multirom/families.md), BN5's families, all of them). A family takes an AI index from 32 and six enemy ids from `0x200`, at fixed numbers: its entries go into the grown tables above, each slot checked to hold the fill first. BN5's CanGard, installed wherever BN5's ROM is read, is AI 32 with ids `0x200`-`0x205`: the first home, its code at `+0x368000` and its reticle's at `+0x368400`, its sprite at `+0x380000` (listed at list 1's `0x1E`, a placeholder in the player's ROM), the reticle's at `+0x383000` (list 4's `0x5C`) and its muzzle's blast at `+0x383800` (list 4's `0x5D`, effect `0x6C`); its attack is BN5's third kind's type `0x25`, spawned through the second-level dispatch (its MOVED entry the reticle's code, its TWIN entry `0xFF`). BN5's Handi is AI 33 with ids `0x206`-`0x20B`: the second home (`+0x348800`), its code at `+0x368800`, its sprite at `+0x384800` (listed at list 1's `0x00`, a placeholder in the player's ROM); its bomb is BN6's own attack (type `0x4B`). The control (`--dev xcontrol`: BN6's own Mettaur moved as AI 32, ids `0x200`-`0x205`) takes the first home and the first block in CanGard's place; BN6-side only, the guest core's BN5 copy untouched | `xvirus.c`, `xvirus_families.c` |
| `+0x400000`-`+0x480000` | BN6's chip tables, grown and moved (issue #82; [The chip registry](../../multirom/chip-registry.md)), written as the core starts, the same at every boot with the same ROMs (a state holds none of it; a saved folder, pack or Library names the same chip with BN5's ROM or without it): `+0x400000` the 512 chip records (0x2C each: BN6's 411, the registry's chips of BN5's at their ids, a blank record at each id past them), `getChip`'s one literal (`0x08021AB0`) pointed at them; `+0x406000` and `+0x407000` the names' text archives, ids 0-255 and 256-511 (0x1000 each), `+0x408000` and `+0x40C000` the descriptions' (0x4000 each), their 23 words pointed at them; `+0x410000` the engine's own Thumb routines (0x88 bytes: the pack's split, its totals' count, its clear, the pseudo-chips' test; below); from `+0x411000` BN5's icons (0x80 each, in BN6's icons' palette), pictures (0x540) and their palettes (0x20), a picture or palette two chips share once. Without BN5's ROM each of its chips is a placeholder (its codes, type and MB the registry's, BN6's blank art and an inert behaviour) and nothing past `+0x411000` is written | `chips.c`, `xchips_install.c` |
| `+0x480000`-`+0x4C0000` | BN5's Souls in BN6's engine (issue #82; [Transplanted Souls](../../multirom/souls.md)), written once a core as it starts where the Souls are installed (`--dev soulproxy` or `souls=MASK`, BN5's ROM read beside BN6's; the Souls track's S2): `+0x000` and `+0x004` the two stubs of a Soul's life (a `bx lr` each, hooked: the proxy's entries 8 of the overlay tables point at them, as MegaMan enters and leaves form 8); `+0x012` BN5's A on UNITE (`BN5_UNITE_A`, 0x24 bytes, the same distance from a word boundary as in BN5) and `+0x040` its one veneer, on to BN6's PlaySoundEffect; `+0x100` UNITE's art for the chip's window (`BN5_UNITE_ART_SIZE`, 0x540 bytes) and `+0x700` its two palettes, a Soul's and Chaos Unison's; `+0x800` the Souls' pictures (14 of 0x80 bytes by BN5's number: none, Souls 1-12, Chaos Unison) and `+0xF00` their palette, each copied from BN5's ROM. Mode 0x24's three words point at the pictures and their palette (`BN6_UNITE_PICTURES`, `BN6_UNITE_PICTURES_2`, `BN6_UNITE_PICTURE_PAL`: Gregar reads them in that mode alone, which it never enters), and the emblem art's two words (`BN6_EMBLEM_ART_TILES`, `BN6_EMBLEM_ART_PALS`) at UNITE's art while element 0xB is UNITE, Gregar's own back while it is the emblem. From `+0x1000` the look, abilities and passives (the track's S3-S5, `src/emu/soul_room.h`, the same at every boot with the same ROMs; each number they take from issue #82's tracks' allocations): `+0x1000` eight stubs (a `bx lr` each, hooked: BN5's pending Soul, a spawn left out, the form read by object and by side, which the moved code calls; the proxy's two passive entries; ColonelSoul's side's strike set and BN5's test of an obstacle, `xstrike.c`), `+0x1080` the crossing's phases with BN5's phase 1 (four words), `+0x1100` the moved code's veneers (12 bytes each), `+0x1A00` the Souls' own Thumb (`SOULS_THUMB`, `soul_room.h`): TomahawkSoul's plain panel after his doubled Wood chip and the chip charge's two answers (`movs r0` and `bx lr`, as BN6's callers test the flags; `soulchips.c`), at `+0x1A40` BN5's counter's start (`SOUL_STUB_COUNTER`: BN6's twin called, then the counter family's first frame at once, as BN5's `0x0800E338` runs it; `soulcode.c`), and from `+0x1A80` ColonelSoul's (`soulcolonel.c`): his button's two phases (BN5's, moved: a table of two words), `+0x1A90` and `+0x1AC0` the loaded chip's draw (BN5's `0x080254F8`, moved) before BN6's last draw of the chips' mode and at the end of UNITE's, `+0x1B00` BN5's obstacle states' block (`0x08018002`-`0x0801802E`, the strike, `xstrike.c`: either Colonel's side, written with the core where BN5's Navis are) and `+0x1B80` B giving the button's chip back (BN5's `0x08024CEA`-`0x08024D28`), `+0x1C80` list 0's arm row by sprite number grown and `+0x1D80` its arm rows grown (BN6's three, then the Souls' with BN5's points), `+0x2100` the power attacks grown (BN6's 148, then the Souls' 11 and Chaos Unison's 6) and `+0x2500` their charge times (10 bytes each), `+0x2C00` the Souls' loadouts (6 bytes each), `+0x2C80` the emotion window's faces' table grown and `+0x2D00` their palettes (BN6's 23, then the Souls'), `+0x3100` Chaos Unison's Thumb (`SOULS_CHAOS`, the Souls track's S6, `soulchaos.c`: its six power attacks, the chip writer with each one's DarkChip, the requests' ends, the failure's family 0xC7 with BN5's phases and BN6's steps of a form's end, two stubs), `+0x3800` kept free (BN5's songs a Soul plays go where every track's sounds go, `xsong_install_at`: each at the slot the allocations give that sound, 0x70 at 0x5F, 0x117 at 0x5E, 0x115 at 0x5A, copied into xsong's room or found there where another track's install copied it first), `+0x4000` BN5's code the Souls run (`soulcode_bn5.c`'s 21 blocks, each its CRC32 checked as it is copied, ColonelSoul's strike's soldier among them; the types it shares the other tracks' copies, its calls into them its routes; written with the core where BN5's Navis are, for the obstacle strike, `xinstall.c`), `+0x7000` what they draw that BN6 and no other track has by its bytes (the soldier's, an effect's, ToadSoul's flash's sprites), `+0x15000` the Souls' faces and BN5's emotion window digits, `+0x15C00` BN6's sprite list 0 grown (0x60 words: BN6's as its code reads them through list 0 to 0x44, the Souls' 0x45-0x4A, what their code draws from 0x4B; list 0 the Souls' alone; grown by the Souls' code's install, its art listed, the look listing the Souls' own), `+0x16000` MegaMan in each Soul (BN5's sprite list 0, 7-12, 0x28203 bytes). Their effects take BN6's grown effect ids 0xC0-0xCF where no other track copied the same BN5 effect first, their objects the families' second-level dispatch (`xvirus_dispatch.c`; every BN5 type moved once, its decision and whose copy in `tools/bn5_twins.py`'s TYPES), their families BN6's attack families grown at `+0x600000` (0xC0-0xC7, Chaos Unison's failure 0xC7). The proxy form itself writes BN6's own tables in place (entry 8 of each table by form and its hooks' BKPTs: [MegaMan's forms and BN5's Souls in BN6's engine](../../rom/souls.md), MegaMan's forms) and form 8's object id in the enemy ids' table the engine reads (where the families' room has grown it, its copy at `+0x336000`, through the room's own writer, `xg_bn6_entry`) | `soulproxy.c`, `soulunite.c`, `soullife.c`, `soulcode.c`, `soullook.c`, `soulpower.c`, `soulpassive.c`, `soulchips.c`, `soulcolonel.c`, `xstrike.c`, `soulchaos.c` |
| `+0x4C0000`-`+0x4D0000` | BN5's lava and sea in BN6's battle engine (issue #82; [One net: the design](../../multirom/one-net.md); [BN5's maps, songs, text and objects](../../rom/bn5-world.md), BN5's lava and sea in BN6's engine), written once a core as it starts where BN5's ROM is read, at fixed addresses (IWRAM's copy of the flags' literal names the first, and a state holds IWRAM): `+0x4C0000` BN6's panel flags table grown to 16 words (BN6's 13, BN5's lava's at 13, its sea's at 14, type 0's at 15), the literal of its IWRAM routine pointed at it; `+0x4C0040` the lava's cooling, a few instructions of the project's own Thumb (the panel set plain, the spark's place moved by a random number, the spark), and its four veneers on to BN6's routines after it; `+0x4C0100` BN6's panel tile table grown to 15 types, lava's and the sea's entries BN5's with their tiles at 0x295 on, its literal pointed at it; the rest free (the tiles themselves go to VRAM from C as a battle first shows them, the panels' draw hook below: a tile set grown with them, which the game decompresses as every battle begins, made BN6's own battles start later and their runs differ) | `xpanels.c`, `xpanels_look.c` |
| `+0x4D0000`-`+0x500000` | Free | |
| `+0x500000`-`+0x600000` | BN5's Navis in BN6's battle engine (issue #82; [Transplanted Navis](../../multirom/navis.md); [Navis moved into BN6's battle engine](../../rom/navis.md), Navis moved into the battle engine), written once a core as it starts where BN5's ROM is read (`--dev xnavicontrol` the control instead, `nonavis` none), at fixed addresses, the same at every boot with the same ROMs: `+0x500000` their battle sprites, each at a place of its own (`xnavi_sprites.c`: Colonel's first, the Dark Soul's at `+0x544000`), listed at list 2's 0x20 + n - 7, and from `+0x57B000` the Navis' arm points' records grown to every number of list 2; `+0x580000` BN6's thirteen Navi tables grown to 256 AI indexes (`0x400` apart), the ice block's sizes (`+0x583400`), the held chips' words (`+0x583500`), the companion's start and end (`+0x583900`, `+0x583D00`), the overlay's masks and sprites grown to 64 kinds (`+0x584100`, `+0x584200`), the moved Navis' names (`+0x584400`); `+0x588000` their homes, 16 of `0x800` (the families' layout), the last the moved objects'; `+0x590000` the Navis' stubs (`0x60`: r0 0, the DS chip AI logged, the Navis' buster's setup, Colonel's sides' bits kept in his field, the Dark Soul's chip records by id), `+0x590060` BN5's sea's submerge's code (`0xA0`, `xsea.c`: the marker's spawn, the hit gate's two tests, the splash of an object the sea holds, their veneers), `+0x590100` BN5's engine objects moved once for every track (`xnavi_objects.c`: the image object's module and the sea's marker's, a block each), `+0x592000` the Navis' code, a room of `0x3C00` each (Colonel, ShadowMan, NumberMan, TomahawkMan, KnightMan, ToadMan, the Dark Soul, the control); `+0x5B0000` a room of `0x400` each for its overlay's masks, then from `+0x5B2000` the shared art: every sprite their attacks draw once, at a place and number of its own whichever Navi draws it (lists 4 and 5 grown, `+0x334000`'s), copied by the first install that needs it, to `+0x5FA000`; `+0x5FA000` the Dark Soul's chip records (`XN_DARK_RECORDS`: BN6's 512 records with the four bytes its chip AI reads BN5's, `0x5800` bytes) | `xnavi_sprites.c`, `xnavi_room.c`, `xnavi_install.c`, `xnavi_objects.c`, `xsea.c`, `xnavi_dark.c` |
| `+0x600000`-`+0x700000` | The code of BN5's chips moved into BN6's engine (issue #82's C3; [The chips' code](../../multirom/chips-code.md); [The chip registry and BN5's chips' code](../../rom/chips-code.md), BN5's chips' code in BN6's engine), written once a core as it starts where BN5's ROM is read, all or nothing, after the moved objects and the Navis and before the families (`xinstall.c`: a type the moved objects or a Navi moved its units spawn as that copy, and a family one of theirs as theirs), at fixed addresses (their records, names and art are at `+0x400000`): `+0x600000` BN6's attack families grown (`FREE_ATTACK_FAMILIES`, `xattack_grow.c`), one table for every track that adds an attack: 240 entries for CurAction 0x10-0xFF, BN6's 79 (0x10-0x5E) as the player's ROM has them, 0x60-0xBF the chips' (BN5's family F at 0x50 + F), 0xC0-0xCF the Souls', 0xD0-0xFF spare, every entry not given its stub at `+0x6003C0` (`ldr r0,[pc,#0]; bx r0` and `0x08011715`, BN6's object_exitAttackState: the attack ends), its two words (`0x0801BB10`, `0x080EC384`) pointed at it, each checked first; `+0x600400` BN6's support chips' routines grown to 256 (BN6's 0x2A as the ROM has them, the units' from 0x2A + BN5's subfamily), `+0x600800` its Navi chips' routines to 256 (BN6's 29, the units' from 0x1D + BN5's), their four words (`0x080ED138`, `0x08017BB8`; `0x080E2CF0`, `0x08017BBC`) pointed at them; `+0x600C00` their stubs (a support routine that spawns nothing, `movs r0,#0; mov pc,lr`, a panel claim's spawners' too, an unused computed power's and an unplayed subfamily's; a Navi routine that does nothing, `mov pc,lr`; `+0x600C08` r0 1, `movs r0,#1; mov pc,lr`, the SP powers' link bit; `+0x600C10` MegaMan's point (-1, 26) in r0 and r1, ElemRage's; `+0x600C18` his point 0x1D, (40, 20) by his facing, HolyDrem's; each returning by `mov pc,lr` as BN6's own routines do, since the tables' readers call through `mov lr, pc`, a return address without the Thumb bit that a `bx lr` would go on from in ARM state); `+0x600C30` the SP powers' best deletion times (`XC_SP_TIMES`, BN5's `0x0203E560` as they read it: 0xFFFF each, none, 0x2C a side); `+0x600C90`-`+0x602000` the helpers' code (`XC_HELPERS`: BN5's code several units call, installed once before them: the buster arm, the shake, the computed powers' routines; the muzzle flash BN5's image object, moved once for every track at `+0x590100` with its kinds' sprites, `tools/bn5_objects.py`), the only code besides BN6's and the moved objects' that a unit's calls may reach; `+0x602000` BN6's computed powers' routines grown to 256 (`XC_POWERS`: BN6's 45 as the player's ROM has them, BN5's entry i at 0x80 + i where a helper moved its routine, the rest the stub), its one word (`0x080109D4`) pointed at it; `+0x602400` each unit's veneers (the helpers' first), 64 homes of `0x400` in the install's order; `+0x612400` the units' code, block by block in `UNITS`' order, each the same distance from a word boundary as in BN5; `+0x6A0000`-`+0x700000` what they draw, each sprite in a room of its own (BN5's sprite of a list and number BN6 keeps another of, listed at a number of the chips' in the grown list: lists 1-5, list 3 grown at `+0x333900` for every track by this one), the chips taking the grown lists' numbers from `0xFF` down, the families from the bottom up, each slot checked to hold the fill first; the grown effects' ids 0xD0-0xFF from the top down; BN5's sounds BN6 has another song of at their slots fixed for every track (0x40-0x62, the chips' own from 0x52 down, `xsong_install_at`, [The chips' code](../../multirom/chips-code.md)). BN6's chip records are named moved as the registry writes them (BN6's at `0x08021DA8` to `+0x400000`), so a trace or the differential's poke at BN6's own record reads the copy the game reads | `xattack_grow.c`, `xchips_code.c`, `xchips_bn5.c` |
| `+0x700000`-`+0x710000` | The Souls' art past their room (`FREE_SOULS_ART`, issue #82's grant to the Souls track): from `+0x700000` the sprites `tools/xsoul.py` places past `SOULS_ART` (none since the tracks' merge: the flash kinds' 3:0x03 and 3:0x00 are the moved objects' copies at the Navis' numbers 5:0xA6 and 4:0xB5, tools/bn5_objects.py, each sprite's bytes in one place), to `+0x70F000`; `+0x70F000`-`+0x70FA00` ColonelSoul's button's art, BN5's copied (`SOULS_COLONEL_ART`: its tiles on the chips' screen lit, grey and chosen, 0x180 bytes each, BN5's lit for chosen; its picture in the chip's window, 0x540 bytes; that picture's palette and the window's name line's), written once a core with the Souls' code | `soulcode.c`, `soulcolonel.c` |
| `+0x710000`-`+0x710400` | BN6's Program Advances' list grown with BN5's (`FREE_CHIP_PAS`, issue #82; [The chip registry and BN5's chips' code](../../rom/chips-code.md), BN5's Program Advances in BN6's lists), written as the chips' code is installed, where BN5's ROM is read: BN5's recipes of the registry's in BN6's ids (`src/core/xchips_pas.c`, 6 bytes each), then at the end BN6's 43 recipes' words, BN5's after them and 0, the Custom screen's word (`0x080295B4`) pointed at it | `xchips_code.c` |
| `+0x710400`-`+0x710440` | The player's chip AI's stub (`FREE_CHIP_AI`, issue #82): from BN6's idle state's hook, a call into BN5's chip AI for the player (moved with the Dark Soul's code) and the state left as BN5's leaves it (`movs r0,#1; pop {r4,r6,r7,pc}`), where BN5's ROM is read | `darkinvs.c` |
| `+0x710440`-`+0x800000` | Free | |

The guest core's ROM copy (BN5's: the differential's reference since the
guest left runs, [Guest battles](../../multirom/guest-battles.md)) is padded the same way;
past BN5's 8 MB, from `0x08800000` (`BN5_FREE`), the differential's record
(`+0x200`, its entities from `+0x210`: `guest_dev_battle`, docs/
[The differential: a moved family against its own game](../../devtools/differential.md)). In BN5's own data it writes the encounter
roll (its answer) alone. Until issue #82's last round a run's guest
battles wrote there too: a battle's record copy (`+0x00`) and entity list
(`+0x20`), the DarkChips' deck shelf (`+0x100`), the deck's compaction,
its chips' records, reward rows and finds (All *, the rewards' codes and
zenny), a guardian's stats row, and its event flags for the run's Souls.

A battle with a green Mystery Data on the field also rewrites the reward
row its record names (rows 0 and 1, one per record, of the table at
`0x080211A0`, [ROM data](../../rom/overview.md)) with the run's own rewards before the
battle; one of BN5's records fought in BN6's engine (issue #82, the
xrecord step, `src/core/xbattles.c`) with BN5's own row of finds, as
BN6's ([BN5's battles, records and rewards](../../rom/bn5-battles.md), BN5's records in BN6's engine).

Where BN5's ROM is read, as the core starts (`xvirus_install`), one more
of BN6's tables is written in place (issue #82, [Other games' ROMs](../../multirom/overview.md)): 26
battlefields that no BattleSettings of BN6's names (12 bytes each at
`0x0800D730` + n x 12: 0x09, 0x0D-0x0F, 0x11, 0x1D, 0x22-0x25, 0x28,
0x2F-0x31, 0x46, 0x55, 0x63 and 0x64, and for BN5's lava and sea 0x56,
0x5F, 0x61, 0x65-0x67, 0x69 and 0x6A) with fields of BN5's that its
areas' records name, their panels' types as BN6's (`src/emu/xfields.c`;
[BN6's battles](../../rom/battle.md), Battlefields and their panels' types), each first
checked to hold BN6's own. It is not in a state: every boot with the
same ROMs writes them the same, and BN6's own battles name none of those
fields. The reward rows of the ids a virus family moved into the engine
takes (from 0x200; CanGard's 0x200-0x205) are its game's rows for its
versions, each entry as BN6's (a chip as BN6's of its name, else 200
zenny), in the grown copy of the rows (`+0x337000` above), each first
checked to hold the filler the copy gives the ids past BN6's. For BN5's
lava and sea, BN6's panel types 13 and 14 (`src/emu/xpanels.c`;
[BN5's maps, songs, text and objects](../../rom/bn5-world.md), BN5's lava and sea in BN6's engine), two of BN6's
IWRAM routines are patched too: the literal that
names the panels' flags table (`0x0300791C`, pointed at the grown table
in the free space), the element hit's volcano and conveyor tests
(`0x03007742`-`0x03007771`, made again with lava cooled under Aqua as
the volcano is) and the panel bonus's tests (`0x03007678`-`0x0300768D`,
made again with an Elec hit on sea counted twice as BN5's are, after
Fire on grass), and for BN5's sea's submerge (`src/emu/xsea.c`;
[BN5's maps, songs, text and objects](../../rom/bn5-world.md), BN5's sea's submerge and hold in BN6's engine) the hit
gate's two tests of an invisible target (`0x0300724A`-`0x03007257` and
`0x0300728C`-`0x0300729B`, each made a call to the project's own test in
ROM, which stops a hit on the submerged as BN6's stops one on the
invisible but for Elec), each in the routines' image in ROM (`0x081D6000` on),
which the game copies to IWRAM as it powers on, and kept so in RAM
(`emu_keep`): a state is saved with BN6's own bytes there, so it loads
in any build, and they are made the engine's again as one is loaded.
BN6's own battles run as they did: its panels take the element hit's
steps in the same cycles (but a Wood hit on a conveyor, three more),
their flags are read from the ROM's table (a few wait states a read),
and lava's and the sea's tiles go to VRAM from C, at no cost in the
game's cycles, only where a battle shows them; the autopilot's runs over
seeds 1-12 log the same battles as the build before, and draw the same
picture every 250 frames.

As a battle ends, the reward rows of its enemies (`0x080AC718` + id x
0x28, or their grown copy at `+0x337000` where a family is installed:
the rows the results read) are rewritten in the folder's codes, half the
time, by a hook on the reward pick just before it reads them ([The meta layer](../../game/meta/overview.md),
[ROM data](../../rom/overview.md)); with the All * helper every chip entry in *.

While a run has the All * helper ([All *, a fourth helper](../../game/meta/all-star-helper.md), issue #18), every chip
record of ids 1-511 holds * alone (its four codes at the chip records'
place, `EMU_FREE` + `0x400000` + id x 0x2C, written `1A FF FF FF`), and the Program Advance check's `cmp r2, #1`
at `0x08029606` is `cmp r2, #3`; each new layer writes them as the run's
helpers have them, and the ROM's own back before the game's own NEW GAME
boots (the folder it gives is counted in the pack by the records). As
such a run begins, the folder's 30 entries go to * and each chip's four
pack counts to its first.

BN6's own DarkChips ([The meta layer](../../game/meta/overview.md), [ROM data](../../rom/overview.md); issue #70): as each
layer is made, the records of DrkSword, DarkThnd, DrkRecov and DarkInvs
(ids 286-289) are made whole in the core's copy: effect flag 0x20 set
(BN6's DarkChip class: three in a folder, MegaMan's NAVIGATOR line, the
purple card), library flag 0x20 cleared (the pack lists them), their
icon, picture and palette pointers their base chips' (Sword, Thunder,
Recov10, Invisibl), and their sort keys (IDSortPos their id,
AlphabetSortPos among the names before theirs), which the US version left
0 and by which the folder editor took any two DarkChips for one chip.
Their attack power is written as the BugFrags held say, on the map and as
a battle begins: their own with one or more, their base chip's with none,
and the base chip's for the rest of a battle whose last BugFrag a
DarkChip spent. DarkPlus's record is left as it is, out of the pack.

BN6's chip tables (issue #82; [The chip registry](../../multirom/chip-registry.md);
`src/emu/xchips_install.c`), as the core starts: besides the records,
archives and art past `EMU_FREE` (above), BN6's pack routines are split
so ids 320-511 count in an extension of the pack, 12 bytes an id from
`0x02010CE0` to `0x020115E0` (EWRAM BN6 leaves alone: only its power-on
clear writes it, as it writes the pack; a state saves it with the rest):
getOffsetToQuantityOfChipCode's tail (`0x08021C9A`) and GetTotalChipCount's
(`0x08021BE4`) jump to the engine's routines, the pack totals' count
(`0x08021CD6`) calls one and its loop runs to 511 (`0x08021D04`), the
pack's list runs to 511 (`0x08121D2C`, 0x13A before), and its clear
(`0x08021C70`, a NEW GAME's and the folder editor's as it writes the pack
again) clears the extension too. The support and Navi chips' routines
test a pseudo-chip by its id through the engine's routine, 0x170-0x19A
(`0x080ED11C`, `0x080ED6C4`, their literals `0x080ED13C`, `0x080ED6D8`):
BN6's id test was every id from 0x170, which would take a chip of the
registry's from 411 for one. Each halfword and word is read first and
must be BN6's own, the free space `0xFF`; then all are written.

In the rival's netbattle ([The rival: Chaud and ProtoMan](../../game/rival.md)), ProtoMan's HP and MaxHP are
given the act's guardian band at most as he spawns, where his own 1800 is
above it: a hook lowers the value BN6's spawn sets both from.

In any of BN6's battles, ProtoMan landing from his cross gets his
collision region (his collision data's `+0x01`) back on his own panel
where the cross left it cleared, as BN6 does for a cross cut short but
not for one that ends, which left him untouchable ([Fidelity](../../game/fidelity.md),
issue #55): a hook at `0x080FDFA0`.

The engine also takes over Central Town (`0x01:0`) or ACDC Town (`0x00:0`)
for the town (its tile map, coordinate data, NPC list, map scripts,
objects, sprite list, warp list, jack-in table, check table
`0x0803461C`, text archive `0x08040794` and music), clears the check
flags `0x16C0`-`0x16CF`, rewrites jack-in destination 42 (`0x08099A00` +
42 x 20) to the first layer, and
widens MegaMan's facing probes for talking (`0x0809F164`; in the Net
only, BN6's own in the real world, where A's checks look along them), redraws the
chat font's version marks as two letters side by side (`0x086AACAC`,
widths `0x08043C74`), points the
map-name label and the PET's at the run's own names, rewrites the stock of
the game's shops 0 and 3 (in RAM and
in the initial table in ROM, `shop.c`), clears the Mystery Data flags
(`0x1400`+) and choice flags (`0x1440`+) it uses, sets the flags that stop
jacking out and the PET's Save (`0x1727`, `0x1706`) and the one that puts
the NaviCust under MegaMan in the PET (`0x00F2`; without it MegaMan's entry
goes straight to his status and a program cannot be installed. Found by
setting blocks of flags with the PET open, `tools/play.py`'s dev `flags`
step, and halving: `0x00AC` and `0x00F7` add Records there instead), and
calls the game's routines (a warp among them) through a hook at
cbGameState (`0x080050EC`), a frame of the game's state update each.


At a new run's start the engine writes the chosen starting folder ([The meta layer](../../game/meta/overview.md))
over the game's first folder: 30 u16 at the chip data eToolkit `+0x48` points
at ([Chips, folders and the Library](../../rom/chips.md), chip folders), as BN6's own GiveFolder copies one in.
The Standard folder writes nothing.

## BN6's RAM

The engine keeps its own state out of BN6's RAM but where issue #82's
RAM plan gives the moved content room that BN6 leaves alone, by one plan
every track follows (its numbers `RAM_*` in `src/emu/free_space.h`, which
`tests/test_core.c` checks in order and apart, `BN6_PACK_EXT` and
`BN6_DISPATCH_SIDE` at its places):

| Range | Contents | Code |
| --- | --- | --- |
| `0x02010CE0`-`0x020115DF` | Persistent: the chip pack's extension, ids 320-511, 12 bytes an id (`BN6_PACK_EXT`; [The chip registry](../../multirom/chip-registry.md)) | `xchips_install.c` |
| `0x020115E0`-`0x0201172F` | Persistent: MegaMan's AI data, BN5's 0x140 bytes, where BN5's ROM is read (`RAM_PLAYER_AI`: his object's +0x58 pointed here as he is set up, `0x080172F6`, his pool's 0x100 bytes copied first, the rest zero; BN5's chip AI keeps its state from +0xF0 to +0x13F, past BN6's 0x100), then at `0x02011720` his place in BN6's pool; DarkInvs's machine, [The chips' code](../../multirom/chips-code.md) | `darkinvs.c` |
| `0x02011730`-`0x0201174F` | Persistent: `--dev arena2v2`'s list of what a NetBattle spawns, two MegaMen a side (`RAM_TEAM`; epic #167's spike, [Online: the Square, the Arena and the link battle](../../rom/online.md), Two MegaMen a side) | `arena_team.c` |
| `0x02011750`-`0x0201178F` | Persistent: two MegaMen a side's partners' words (`RAM_TEAM_LINK`, `0x20` bytes a side, each core's own): a side's GBA's outbox of its partner's messages (12 u16 at +0, its head +0x18 and count +0x19: `team_outbox.h`), his keys for its next packet (+0x1A), and his keys and word as his side's last packet brought them (+0x1C, +0x1E), which `BN6_KEY_COPY` gives him; written by the pair before the GBA's frame and where the battle reads the packets and stores its pad (`BN6_LINK_PAD_STORE`; in sub_801FFD6 `BN6_LINK_READ_B` and `BN6_LINK_PAD_STORE_B`), emptied by the battle's spawn ([The partners' words in BN6's packet](../../online/arena/two-a-side/partners-words.md)) | `arena_team_link.c` |
| `0x02011790`-`0x020117FF` | Persistent: `--dev arena2v2`'s partners' decks and turns (`RAM_TEAM_CUSTOM`: `0x38` bytes a side, the same in both of the pair's cores: his deck as folder indices in the order drawn, his turn's state, the picks his stream has said, the iteration they go into his hand and their folder indices), written by the pair as it samples his side's GBA's frames and read by `BN6_KEY_COPY`'s hook; emptied by the battle's spawn ([A partner's own Custom screen](../../online/arena/two-a-side/custom-screen.md)) | `arena_team_custom.c` |
| `0x0203FB50`-`0x0203FBDF` | Per battle: BN5's sea's submerge (`RAM_SEA`; `XSEA_MASK` the submerged, a bit each by its collision data's own, `BN6_COLL_INDEX_BIT`; `XSEA_MARKERS` each one's marker by that bit's index, 32 words; 0x84 bytes used), which BN5 keeps in the collision data where BN6 keeps its bubble | `xsea.c` |
| `0x0203FBE0`-`0x0203FD9F` | Per battle: the Dark Soul's chip AI's lists (`RAM_DARK_LISTS`, `BN6_DARK_LISTS`: BN5's `0x02034C20` laid out as BN5's, 0xE0 a side, BN6's chip ids), empty (0) as each battle is set up | `xnavi_dark.c`, `xnavi_bn5.c` |
| `0x0203FDA0`-`0x0203FDAF` | Per battle: the moved Navis' own state (`RAM_NAVIS`): Colonel's sides' bits (`BN6_COLONEL_SIDES`, which BN5 keeps in BattleState +0x5C), the object the Dark Soul's AI data is now and its own in the pool (`BN6_DARK_AI_OWNER`, `BN6_DARK_AI_POOL`) | `xnavi_room.c`, `xnavi_dark.c` |
| `0x0203FDB0`-`0x0203FEEF` | Per battle: the Dark Soul's AI data (`RAM_DARK_AI`, `BN6_DARK_AI`: BN5's 0x140 bytes, its chip AI's state past BN6's 0x100), one Dark Soul's at a time | `xnavi_dark.c` |
| `0x0203FEF0`-`0x0203FEFF` | Per battle: the moved chips' own state (`RAM_BATTLE`): BN5's green flash's frames, a u16 a side (`XC_FLASH`, which BN5 keeps in his AIData +0x3C, BN6's resting height; [The chip registry and BN5's chips' code](../../rom/chips-code.md), BN5's DarkPlus and its green flash), zeroed with the battle's RAM; `+4` on kept for the moved families' | `xchips_flash.c` |
| `0x0203FF00`-`0x0203FF5F` | Per battle: the second-level dispatch's side tables (`BN6_DISPATCH_SIDE`: a byte an object slot, its game's type, 32 for each of the first, third and fourth kinds), written by the dispatch's spawners and read by its routines | `xvirus_dispatch.c` |
| `0x0203FF60`-`0x0203FF6F` | Per battle: BN5's Colonel's table moved (`BN6_COLONEL_TABLE`, `RAM_COLONEL`: two 8-byte entries by side, which BN5 keeps at `0x02034000`), written by his moved code and read by his attacks' objects, zeroed with the battle's RAM as each battle is set up | `xnavi_bn5.c` |
| `0x0203FF70`-`0x0203FFEF` | Per battle: the Souls' state in BN6's battles (`BN6_SOULS`: 0x20 bytes a side, and room for the track's later milestones; each side's +0x0E and +0x0F BN5's mood as BN6's battles keep it and whether a BN5 DarkChip made him dark, +0x06 the chaos level, +0x16 the Chaos Unison's charge's frame in its phases' cycle and +0x17 its release's request standing (`BN6_SOUL_LEVEL`, `BN6_SOUL_CYCLE`, `BN6_SOUL_REQUEST`: BN5's AIData +0x6C and +0x1C; [MegaMan's forms and BN5's Souls in BN6's engine](../../rom/souls.md), Chaos Unison), +0x14 the chip ColonelSoul's button took, committed at OK (his obstacle strike's side is `BN6_COLONEL_SIDES`, the Navis' field a Navi Colonel's spawn sets too, never BattleState +0x5C's bits, which BN6 reads as its own); [MegaMan's forms and BN5's Souls in BN6's engine](../../rom/souls.md), BN5's mood in BN6's battles; [MegaMan's forms and BN5's Souls in BN6's engine](../../rom/souls.md), UNITE on BN6's Custom screen, and a Soul's life on the proxy form): so every battle begins with no Soul, read and written by the Souls' hooks alone; from +0x40 each side's partner's own block under `--dev arena2v2` (`BN6_SOULS_PARTNERS`, laid out as a side's: his mood and his dark, never a Soul; `soul_field_of`) | `soulunite.c`, `soullife.c`, `soulmood.c`, `soulcolonel.c`, `soulchaos.c` |
| `0x0203FFF0`-`0x0203FFF3` | Per battle: two MegaMen a side's count of the battle loop's iterations (`RAM_TEAM_TICK`, a u32 the battle's setup zeroes), which both GBAs keep alike: a partner's picks go into his hand at the iteration after his side's screen closed, and the GBAs are compared by it | `arena_team.c` |
| `0x0203FFF4`-`0x0203FFF7` | Per battle: the entries of BN6's AI data's pool BN5's chip AI took for its rooms (`RAM_CHIP_AI_ROOMS`, a u32 of `BN6_AI_POOL_USED`'s bits, zero as each battle is set up; issue #182, below) | `darkinvs.c` |
| `0x0203FFF8`-`0x0203FFFB` | Per battle: two MegaMen a side's turn's clock (`RAM_TEAM_CLOCK`: a u16 of frames since the first OK of the turn the core has seen, then a byte 1 once seen; zero outside the Custom screen and with the battle's RAM), each core's own ([The leads wait for their partners](../../online/arena/two-a-side/leads-wait.md)) | `arena_team_custom.c` |
| `0x0203FFFC`-`0x0203FFFF` | Per battle: spare (the game call's old mark, not used since issue #33) | |

BN5's chip AI's rooms (issue #182; `src/emu/darkinvs_room.h`): in a
NetBattle a MegaMan other than side 0's player whom BN5's chip AI takes
(DarkInvs, a Chaos Unison's) needs BN5's 0x140 bytes of AI data, so he
takes a room of his own in the last entries of BN6's AI data's pool
(`BN6_AI_POOL`, eight entries of 0x100 bytes; side 1's player
`0x02034740`, on entries 6 and 7; side 0's partner `0x02034600`; side
1's partner `0x020344C0`), their bits set in the pool's own word
(`BN6_AI_POOL_USED`) so that no spawn takes or lays them out until the
next battle's set-up clears it, and noted in `RAM_CHIP_AI_ROOMS`. Where
an actor holds an entry of his room he is left out of the chip AI. A
room taken in BN6's own allocator, as the partners' NaviStats below, and
only as the chip AI first takes him: a NetBattle without it keeps all
eight entries.

Under `--dev arena2v2` alone, two object slots of BN6's own are taken
too: the fourth kind's (the effects') slots 30 and 31, `0xC8` bytes each
from their nodes, marked in play in its pool (`BN6_T4_ACTIVE`) as the
battle spawns its field and never linked into the run order, so BN6
never runs, frees or zeroes them until the next battle sets its pool up:
each partner's own battle NaviStats and hand (`arena_team_navi.c`;
[The partners' own NaviStats](../../online/arena/two-a-side/navistats.md)). A room taken in
BN6's own allocator, not a region of this plan: the battle keeps 30
effects.

Persistent, `0x02010CDC`-`0x020117FF`: the engine RAM survey of issue #82
(static scans, 592 runs, every RAM byte logged) found it written by the
power-on clear (`0x080001CC`) alone and read by nothing, the RFU library's
globals below it and the battle sprite pool from `0x02011800` above; the
pack's extension starts on a 16-byte boundary, the four bytes before it
left. Per battle, `0x0203FB50`-`0x0203FFFF` (1200 bytes): the battle's
setup clears it every battle (`0x080071F2`, a fill of
`0x0203CDA8`-`0x0203FFFF`), and nothing else of BN6's reads or writes it
(the engine research's watch over its runs; no code literal points past
`0x0203F7E0`, and the fourth kind's list at `0x0203F750`, its count a
byte, reaches `0x0203FB4F` at most). Both are in a state as all of EWRAM
is.
