# Running the game's own logic

Battles, the net, menus, shops and messages run on BN6's own code, executed
from the player's ROM by an embedded [mGBA](https://mgba.io) core (0.10.5,
MPL-2.0, built by `docker/mgba.sh`). Cyberworld Endless is the director
around it: it generates each layer in the game's own formats, patches the
map tables of its in-memory ROM copy to point at them, and follows the game
to move the run along: hooks on its code tell it what happens as it happens
(Hooks, below), and it reads the game's memory between frames for the rest.
Nothing from the ROM is shipped.

| Layer | Owner |
| --- | --- |
| CPU, video, sound, input | mGBA core (`src/emu/emu.c`), presented through SDL |
| Battles, net movement, menus, messages, shops, traders | The game's code |
| Run structure, layer generation, loot, saves, the title | Cyberworld Endless |

The player's A+B+Select+Start returns to Cyberworld Endless's title
before the core sees it (`scene_emu.c`, issue #113): BN6's own reset would
start its title on the map tables the run has taken over. The director
stops without saving, so CONTINUE takes the run up at its last checkpoint,
as after a quit in a battle. The four buttons held together are no
controller's Select+Start quit either.

## Why an emulator

Ship of Harkinian runs Ocarina of Time with no emulator, and Zelda64Recomp
runs Majora's Mask so too. Both stand on work BN6 does not have:

- [Ship of Harkinian](https://github.com/HarbourMasters/Shipwright) compiles
  [zeldaret/oot](https://github.com/zeldaret/oot), a matching decompilation:
  C written by hand that builds back into the original ROM byte for byte.
  [libultraship](https://github.com/Kenix3/libultraship) stands in for the
  N64's hardware, its Fast3D renderer turning the game's display lists into
  OpenGL, Direct3D or Metal; the assets come from the player's ROM at first
  start. [2Ship2Harkinian](https://github.com/2ship2harkinian/2ship2harkinian)
  does the same for Majora's Mask.
- [Zelda64Recomp](https://github.com/Zelda64Recomp/Zelda64Recomp) translates
  the game's MIPS code to C, function by function, with
  [N64Recomp](https://github.com/N64Recomp/N64Recomp), which finds the
  functions by the decompilation's symbols; [RT64](https://github.com/rt64/rt64)
  renders. The translated code ships in the executable.

Neither runs a CPU emulator, and both replace the GPU at the display-list
level, which the N64 allows: its games hand the GPU lists of commands. BN6
keeps mGBA because:

1. **No C decompilation of BN6 exists.** [bn6f](https://github.com/dism-exe/bn6f)
   is a matching disassembly of Falzar, almost all of it assembly.
2. **The GBA has no display list.** BN6 writes tiles, maps, OAM and video
   registers itself, some of it timed to the scanline, so a native build
   would still emulate the GBA's video and sound. Only the ARM interpreter
   would go, the core's cheapest part everywhere but the 3DS.
3. **A decompiled or recompiled BN6 ships Capcom's code.** The project ships
   none of it, which is what keeps it on PortMaster, Flathub, the 3DS and
   GitHub Pages.
4. **Other games' ROMs lend their content as data** (docs/MULTIROM.md); a
   native build would need each game's code ported too.
5. **Save states are free checkpoints** (`run.state`) on every target.

What the native ports do have is worth taking: hooks, where the game's code
calls the port's at named points. The engine's hooks (below) are that, on the
emulator.

The question is worth asking again when a C decompilation of BN6 covers most
of its battle and overworld code **and** a run needs changes deep in battle
logic (new chip behaviour, enemy AI) that hooks make awkward; or when the New
3DS's measurements (issue #35) show that the ARM interpreter, not the
renderer, holds its frame rate back. Even then a decompilation helps first as
a map for hooks, as Majora's Mask's does for Zelda64Recomp.

## Memory the engine writes

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
full rules (docs/ONLINE_ARENA.md, Full rules), on every system that
offers NetBattles; not in a New 3DS's runs, which offer none (there for
`--dev rollbench` and `--dev arena` alone, about 2 MB):

| Offset | Contents | Code |
| --- | --- | --- |
| `+0x0000` | Warp record and warp list for direct warps | `gamecall.c` |
| `+0x0100` | Where a game call's routine returns: a hook, which takes its r0 and r1 and goes back to the main loop (until issue #33 a stub of hand-written Thumb, with `0x0203FFF0`-`0x0203FFFB` past the game's EWRAM for its mark and results: free since, `RAM_TEAM_TICK` and spare) | `gamecall.c` |
| `+0x0180` | An empty Mystery Data list (12 zero bytes): every map but the layer's points at it, BN6's own too, as BN6 rolls every map's Mystery Data at each jack-in and clears their flags, some of them the run's (issue #122; docs/ROM_DATA.md, Map takeover); the rest to `+0x0200` free (the encounter roll's wrapper and trampoline until hooks took their place, issue #29) | `mapslot.c` |
| `+0x0200`, `+0x0280` | Two battle records in turn: BattleSettings, `+0x20` its entity list (MegaMan's panel first, the foes, the field's objects) | `encounter.c` |
| `+0x0300` | Free (the PET's input step until a hook on the game's own handler took its place, issue #33) | |
| `+0x0400`-`+0x0640` | Rush's gaps (issue #14): the gap table from group 0x80 that the check's literals point at, a group's empty map list, the layer's group's maps, the layer's gap records, and handler 0x25's records (BN6's 13, then the layer's two) | `rush.c` |
| `+0x0700`-`+0x12E0` | Map-object handler 3's records (issue #42): BN6's 86, then the engine's Link Navi obstacles and cubes, 13 variants of 8 slots each (the last four another game's looks: BN5's Security Cube, its wall of dark flames two ways and its dark hole, a prop that never opens, docs/MULTIROM.md); its literal points here, written once a core | `blockers.c` |
| `+0x2F00` | The layer map's warp list (entry 1: the exit pad) | `mapslot.c` |
| `+0x3000`-`+0x10000` | Layer data in two halves, one per layer in turn: NPC lists and scripts, text archive, Mystery Data, sprite list | `mapslot.c` |
| `+0x10000` | Generated tile map (LZ77, literal blocks) | `netmap.c` |
| `+0x60000` | Generated coordinate data (walls, the exit pad's trigger) | `coords.c` |
| `+0x100000` | The town's tile map (LZ77, literal blocks) | `town.c` |
| `+0x130000` | The town's coordinate data (walls, section 2, the jack-in cells, the checks) | `coords.c` |
| `+0x140000`-`+0x148000` | The town's NPC scripts, text, lists, warps, objects, check table and map text archive, apart from the layers' | `mapslot.c` |
| `+0x148000`-`+0x14C000` | Lan's HP's (docs/HOME.md): the courier's text archive (768 bytes, written again as what waits changes), the Net Square gate keeper's (1536 bytes, written again as the player's Square settings change), the gate's NPC script (its animation byte the director writes) and its keeper's, his and his mark's NPC scripts and the ways back's marks', its NPC list and empty map scripts, its own warp list (BN6's jack-out on its blue pad, the run's portals pointed by the director) and the map music list it adds to | `mapslot.c`, `lanhp.c` |
| `+0x14C000`-`+0x150000` | Lan's house's and room's: their empty NPC lists and map scripts, their own warp lists (BN6's doors copied in), the room's jack-in table and the music list | `mapslot.c`, `lan_house.c` |
| `+0x150000`-`+0x152000` | The director's conversations: one text archive, rewritten for each | `talk.c` |
| `+0x152000`-`+0x153000` | The map-name label's archive: a copy of the game's with its 244 names pointed at where the run is ("Layer 12", "ACDC Town"); its other scripts (the PET's HP, zenny and BugFrags, 0xF0-0xF2) kept | `director_layer.c` |
| `+0x153000`-`+0x154000` | The PET's PLACE: the same copy, its names pointed at the area and the layer ("JudgeTree 14") | `director_layer.c` |
| `+0x154000`-`+0x166000` | The PET's words (docs/PET.md): the key items' names (+0x154000) and descriptions (+0x155000), the mails' senders and subjects (+0x158000) and bodies (+0x15A000), each BN6's archive rebuilt with the run's scripts in place of some | `pet_text.c` |
| `+0x166000`-`+0x167900` | Portraits of Falzar's Navis (docs/BOSSES.md): one mugshot sprite each, 0x500 apart (SpoutMan first), made from the Navi's battle sprite on the layer he guards; the mugshot table points at them from numbers empty in Gregar | `portrait.c` |
| `+0x168000`-`+0x169000` | A layer's Chip Trader's prizes of both nets (issue #82, docs/MULTIROM.md, One net): on the older net's areas and in the deep, where BN5's ROM is read, the list its pool's entry points at, BN6's pool's records fitted to the folder's codes as a layer's are, then as many of BN5's catalogue's of each rarity (6-byte records, a 0 chip ending it); written as the layer's trader is installed | `trader.c` |
| `+0x170000`-`+0x1A0000` | Another game's net area (docs/MULTIROM.md): its tile set, a header of its own before its two blocks encoded again, and its palette as it is; the map its layers take over points at them | `netmap.c` |
| `+0x1A0000`-`+0x260000` | Other games' songs (docs/MULTIROM.md): each song's sequence with its pointers moved, the voices it selects, their key splits, drum kits, samples and waves (a piece two songs share once); BN6's empty song slots from 0x26 to 0x3F point at them (issue #82's shared allocations: the slots above, 0x40-0x62, are the sounds the other games' families, Navis, Souls and chips copy, each BN5 sound at a fixed one). Every song the other games' areas play is copied the first time one is needed, in the areas' order, then their games' battle themes, so each has the same place in every session (a checkpoint's state holds the music player's pointers into it) | `xsong.c` |
| `+0x260000`-`+0x2A0000` | Other games' Navis (docs/MULTIROM.md): an overworld sprite and its mugshot each, copied whole (their offsets are their own); Gregar's list 6 and mugshots point at them from a number Falzar's Navis have there (72, 74, 76-78). Copied, as the songs are, all at once in the areas' order. After them the other games' map objects (sprites of their list 7), all at once the first time one is needed, each at a list-7 number of its own that points at the placeholder `0x084DC040`: BN5's flame of darkness (a DarkChip's, docs/META.md) at 160, decompressed and its palette turned purple, its Security Cube at 146, its wall of dark flames at 138, decompressed, a dark hole at 135. Its last `0x12000` bytes (from `+0x28E000`) hold a guardian of another game's (docs/BOSSES.md, BN5's Navis): his sprite and face, copied anew on each layer he guards and listed at Gregar's 78, which no area's Navi takes | `xnavi.c` |
| `+0x2A0000`-`+0x2E0000` | Other games' battle backgrounds (docs/MULTIROM.md): each one's BGAnimData record, its tiles and map (LZ77 encoded again), palette, animation scripts and the tiles and lists their frames name; then copies of BN6's three background tables (records, animations, scroll entries) with them after BN6's 22, which the loader's literals point at. Copied all at once in the areas' order | `xbackdrop.c` |
| `+0x2E0000`-`+0x2E1000` | BN6's own flame of darkness (docs/META.md, BN6's own DarkChips): its blue flame of sprite list 7 (`0x3C`, 1612 bytes) copied whole, its palette turned purple as BN5's flame is, and listed at Gregar's list-7 `0x54`, which points at the placeholder in the player's ROM; a place of its own, the same with or without another game's ROM, copied once a core | `xnavi.c` |
| `+0x310000`-`+0x320000` | Other games' net maps' backdrops and animations (docs/MULTIROM.md): for each of their areas, its learned map's BGAnimData record with the backdrop's tiles and map (LZ77 encoded again) and palette, and the map's GFXAnim scripts with the colours, tile lists and tiles their frames name (a palette script's RAM moved to BN6's). The map its layers take over has three entries of its group's tables pointed at them while one of its layers stands: its GFXAnim list, its BGAnimData record, and its 16-byte scroll entry rewritten with BN6's callbacks that do the same; every other layer writes BN6's own back. Copied all at once in the areas' order, as the first layer is built | `xbackdrop.c` |
| `+0x320000`-`+0x330000` | Home's other places (docs/HOME.md, piece 10), a part each: AsterLand's (`+0x320000`) and the Cyber Academy's four maps' (`+0x328000`): their NPC lists and their people's scripts and words, empty map scripts, their own warp lists (BN6's doors copied in, the doors out pointed where the planned town has them) and the music lists | `mapslot.c`, `indoors.c`, `aster_land.c`, `academy.c` |
| `+0x330000`-`+0x348000` | BN6's battle engine given the room other games' virus families take (issue #82, the engine's capacity; docs/MULTIROM.md, Transplanted families; docs/ROM_DATA.md, The engine's room for moved families), written once a core as it starts, before any family's install and wherever one is installed, at fixed addresses (a state's pointers into them stay good; the ROM copy is not in a state, so every boot writes them the same): each of BN6's tables below copied with its entries as the player's ROM has them and the rest its fill, the one word of the ROM that names it pointed at the copy, each word checked first. `+0x330000` the eleven per-family tables, 256 AI indexes each, `0x400` apart (AI 0's entry in the rest); `+0x332C00` the ice blocks' sizes (`BN6_ICE_SIZES`), 256 bytes; `+0x332D00` the three tables of 35 words by AI index (`BN6_AI_HOOKS`, `BN6_AI_HOOKS2`, `BN6_HELD_CHIPS`), 256 words each, `0x400` apart, their triples' virus words pointed at them; `+0x333900` sprite list 3 (the attacks' sprites: MegaMan's sword, the chips' objects), 256 numbers (BN6's 105 as the ROM has them, the placeholder in the rest; grown once for every track, the chips' taking its numbers from `0xFF` down, `XV_LIST3`), `+0x333D00`-`+0x334000` free; `+0x334000` sprite lists 1, 2 (the Navis' battle sprites), 4 and 5, 256 numbers each, `0x400` apart (the placeholder sprite in the rest); `+0x335000` the effects (`BN6_EFFECTS`), 256 ids (effect 0's row in the rest); `+0x335400` the first, third and fourth kinds' types (`BN6_T1_TYPES`, `BN6_T3_TYPES`, `BN6_T4_TYPES`), 256 each, `0x400` apart (0 in the rest; T1 `0x5C`, T3 `0xD2` and T4 `0x91` the second-level dispatch's routines, below, each checked first to hold BN6's placeholder, never a family's); `+0x336000` the enemy ids' triples, `0x400` ids (each past BN6's `0x1C4` its filler `0xBB`'s; `+0x336C00`-`+0x337000` free); `+0x337000` the reward rows, `0x400` ids (each past BN6's `0x191` the filler's row); `+0x341000` the names of the ids from `0x200`, a text archive of 512 entries (16 KB), each an empty name until a family names it, which a hook on BN6's name lookup hands out (Hooks, below); `+0x345000` the second-level dispatch (docs/MULTIROM.md, Transplanted families; docs/ROM_DATA.md, the second-level dispatch): MOVED, the moved routine (Thumb) for each of a game's types, 256 words for each of the first, third and fourth kinds, `0x400` apart, every entry the stray's routine until a family names its type; `+0x345C00` TWIN, BN6's own number for each type or `0xFF` where it is moved, 256 bytes a kind, `0x100` apart, `0xFF` until a family names it; `+0x345F00`-`+0x346100` its code, the project's own Thumb made field by field (`xvirus_dispatch.c`): the stray's routine, the three dispatch types' routines (`+0x345F10`, `0x20` apart) and the six spawners a moved family's calls go to (`+0x345F80`, `0x40` apart), `+0x346100` a virus's support chip run with BN5's support family's timing (`XV_SUPPORT`, the attack family 0x65: BN6's support routine twice in its first frame, docs/ROM_DATA.md, A virus's support chip) and `+0x346120` the chip writer a moved family's code calls (`XV_CHIP_WRITER`: BN6's, its support family answered as 0x65), its side tables in RAM (BN6's RAM, below); `+0x346200` the families' pre-steps (`XV_PRESTEP`, issue #82's A2b: 256 words by AI index, 0 none, a word a family whose game's dispatcher acts for its ids before its state, Dominerd's and Appley's, its pre-step's Thumb address in its home, which a hook on BN6's dispatcher runs: docs/ROM_DATA.md, their shims and pre-steps); `+0x346100`-`+0x346200` and `+0x346600`-`+0x348000` free. The battle's sprite preload is made to read an id's 12 bits, as its spawn does (`0x080029CE`, `0x080029D0`; docs/ROM_DATA.md) | `xvirus_grow.c` |
| `+0x348000`-`+0x400000` | The families themselves (issue #82; docs/MULTIROM.md, Transplanted families): `+0x348000` their homes, 64 of `0x800`, one a family: the state table (BN6's layout, 16 entries at most), struct1 (`+0x40`), struct2 (`+0x48`), six empty held-chip lists (`+0x70`, where it holds none), its shims (`+0x130`, `XV_HOME_SHIMS`, the project's own Thumb from its recipe's numbers: its pre-step, its NameID ranges' and its chip compares'; docs/ROM_DATA.md, their shims and pre-steps) and its veneers (`+0x200`, 12 bytes each: `push {r0,r1}; ldr r0,[pc,#4]; str r0,[sp,#4]; pop {r0,pc}` and the target); `+0x368000`-`+0x380000` their code, block by block, each the same distance from a word boundary as in its game; `+0x380000`-`+0x3FBC00` what they draw, each sprite in a room of its own (an LZ77 one decompressed), and the sounds BN6 has no song of (a song packed whole, its sequence, voice group and samples, in a room of its own there, listed at one of BN6's empty song slots, a fixed one per BN5 sound whatever track copies it, the families' `0x60`-`0x62`: docs/ROM_DATA.md, their sounds). `+0x3FBC00`-`+0x400000` the viruses' arm points' records grown to every number of sprite list 1 (`XV_ARMS`, 256 of 0x44 bytes, the word of BN6's that names them pointed here: docs/ROM_DATA.md, their arm points), a family's at its battle sprite's number. Since issue #82's A2b every family BN5 has and BN6 lacks is installed, 25 of them: AI 32-56 with ids `0x200`-`0x295`, homes, code and art one after another in that order (docs/MULTIROM.md, BN5's families, all of them). A family takes an AI index from 32 and six enemy ids from `0x200`, at fixed numbers: its entries go into the grown tables above, each slot checked to hold the fill first. BN5's CanGard, installed wherever BN5's ROM is read, is AI 32 with ids `0x200`-`0x205`: the first home, its code at `+0x368000` and its reticle's at `+0x368400`, its sprite at `+0x380000` (listed at list 1's `0x1E`, a placeholder in the player's ROM), the reticle's at `+0x383000` (list 4's `0x5C`) and its muzzle's blast at `+0x383800` (list 4's `0x5D`, effect `0x6C`); its attack is BN5's third kind's type `0x25`, spawned through the second-level dispatch (its MOVED entry the reticle's code, its TWIN entry `0xFF`). BN5's Handi is AI 33 with ids `0x206`-`0x20B`: the second home (`+0x348800`), its code at `+0x368800`, its sprite at `+0x384800` (listed at list 1's `0x00`, a placeholder in the player's ROM); its bomb is BN6's own attack (type `0x4B`). The control (`--dev xcontrol`: BN6's own Mettaur moved as AI 32, ids `0x200`-`0x205`) takes the first home and the first block in CanGard's place; BN6-side only, the guest core's BN5 copy untouched | `xvirus.c`, `xvirus_families.c` |
| `+0x400000`-`+0x480000` | BN6's chip tables, grown and moved (issue #82; docs/MULTIROM.md, The chip registry), written as the core starts, the same at every boot with the same ROMs (a state holds none of it; a saved folder, pack or Library names the same chip with BN5's ROM or without it): `+0x400000` the 512 chip records (0x2C each: BN6's 411, the registry's chips of BN5's at their ids, a blank record at each id past them), `getChip`'s one literal (`0x08021AB0`) pointed at them; `+0x406000` and `+0x407000` the names' text archives, ids 0-255 and 256-511 (0x1000 each), `+0x408000` and `+0x40C000` the descriptions' (0x4000 each), their 23 words pointed at them; `+0x410000` the engine's own Thumb routines (0x88 bytes: the pack's split, its totals' count, its clear, the pseudo-chips' test; below); from `+0x411000` BN5's icons (0x80 each, in BN6's icons' palette), pictures (0x540) and their palettes (0x20), a picture or palette two chips share once. Without BN5's ROM each of its chips is a placeholder (its codes, type and MB the registry's, BN6's blank art and an inert behaviour) and nothing past `+0x411000` is written | `chips.c`, `xchips_install.c` |
| `+0x480000`-`+0x4C0000` | BN5's Souls in BN6's engine (issue #82; docs/MULTIROM.md, Transplanted Souls), written once a core as it starts where the Souls are installed (`--dev soulproxy` or `souls=MASK`, BN5's ROM read beside BN6's; the Souls track's S2): `+0x000` and `+0x004` the two stubs of a Soul's life (a `bx lr` each, hooked: the proxy's entries 8 of the overlay tables point at them, as MegaMan enters and leaves form 8); `+0x012` BN5's A on UNITE (`BN5_UNITE_A`, 0x24 bytes, the same distance from a word boundary as in BN5) and `+0x040` its one veneer, on to BN6's PlaySoundEffect; `+0x100` UNITE's art for the chip's window (`BN5_UNITE_ART_SIZE`, 0x540 bytes) and `+0x700` its two palettes, a Soul's and Chaos Unison's; `+0x800` the Souls' pictures (14 of 0x80 bytes by BN5's number: none, Souls 1-12, Chaos Unison) and `+0xF00` their palette, each copied from BN5's ROM. Mode 0x24's three words point at the pictures and their palette (`BN6_UNITE_PICTURES`, `BN6_UNITE_PICTURES_2`, `BN6_UNITE_PICTURE_PAL`: Gregar reads them in that mode alone, which it never enters), and the emblem art's two words (`BN6_EMBLEM_ART_TILES`, `BN6_EMBLEM_ART_PALS`) at UNITE's art while element 0xB is UNITE, Gregar's own back while it is the emblem. From `+0x1000` the look, abilities and passives (the track's S3-S5, `src/emu/soul_room.h`, the same at every boot with the same ROMs; each number they take from issue #82's tracks' allocations): `+0x1000` eight stubs (a `bx lr` each, hooked: BN5's pending Soul, a spawn left out, the form read by object and by side, which the moved code calls; the proxy's two passive entries; ColonelSoul's side's strike set and BN5's test of an obstacle, `xstrike.c`), `+0x1080` the crossing's phases with BN5's phase 1 (four words), `+0x1100` the moved code's veneers (12 bytes each), `+0x1A00` the Souls' own Thumb (`SOULS_THUMB`, `soul_room.h`): TomahawkSoul's plain panel after his doubled Wood chip and the chip charge's two answers (`movs r0` and `bx lr`, as BN6's callers test the flags; `soulchips.c`), at `+0x1A40` BN5's counter's start (`SOUL_STUB_COUNTER`: BN6's twin called, then the counter family's first frame at once, as BN5's `0x0800E338` runs it; `soulcode.c`), and from `+0x1A80` ColonelSoul's (`soulcolonel.c`): his button's two phases (BN5's, moved: a table of two words), `+0x1A90` and `+0x1AC0` the loaded chip's draw (BN5's `0x080254F8`, moved) before BN6's last draw of the chips' mode and at the end of UNITE's, `+0x1B00` BN5's obstacle states' block (`0x08018002`-`0x0801802E`, the strike, `xstrike.c`: either Colonel's side, written with the core where BN5's Navis are) and `+0x1B80` B giving the button's chip back (BN5's `0x08024CEA`-`0x08024D28`), `+0x1C80` list 0's arm row by sprite number grown and `+0x1D80` its arm rows grown (BN6's three, then the Souls' with BN5's points), `+0x2100` the power attacks grown (BN6's 148, then the Souls' 11 and Chaos Unison's 6) and `+0x2500` their charge times (10 bytes each), `+0x2C00` the Souls' loadouts (6 bytes each), `+0x2C80` the emotion window's faces' table grown and `+0x2D00` their palettes (BN6's 23, then the Souls'), `+0x3100` Chaos Unison's Thumb (`SOULS_CHAOS`, the Souls track's S6, `soulchaos.c`: its six power attacks, the chip writer with each one's DarkChip, the requests' ends, the failure's family 0xC7 with BN5's phases and BN6's steps of a form's end, two stubs), `+0x3800` kept free (BN5's songs a Soul plays go where every track's sounds go, `xsong_install_at`: each at the slot the allocations give that sound, 0x70 at 0x5F, 0x117 at 0x5E, 0x115 at 0x5A, copied into xsong's room or found there where another track's install copied it first), `+0x4000` BN5's code the Souls run (`soulcode_bn5.c`'s 21 blocks, each its CRC32 checked as it is copied, ColonelSoul's strike's soldier among them; the types it shares the other tracks' copies, its calls into them its routes; written with the core where BN5's Navis are, for the obstacle strike, `xinstall.c`), `+0x7000` what they draw that BN6 and no other track has by its bytes (the soldier's, an effect's, ToadSoul's flash's sprites), `+0x15000` the Souls' faces and BN5's emotion window digits, `+0x15C00` BN6's sprite list 0 grown (0x60 words: BN6's as its code reads them through list 0 to 0x44, the Souls' 0x45-0x4A, what their code draws from 0x4B; list 0 the Souls' alone; grown by the Souls' code's install, its art listed, the look listing the Souls' own), `+0x16000` MegaMan in each Soul (BN5's sprite list 0, 7-12, 0x28203 bytes). Their effects take BN6's grown effect ids 0xC0-0xCF where no other track copied the same BN5 effect first, their objects the families' second-level dispatch (`xvirus_dispatch.c`; every BN5 type moved once, its decision and whose copy in `tools/bn5_twins.py`'s TYPES), their families BN6's attack families grown at `+0x600000` (0xC0-0xC7, Chaos Unison's failure 0xC7). The proxy form itself writes BN6's own tables in place (entry 8 of each table by form and its hooks' BKPTs: docs/ROM_DATA.md, MegaMan's forms) and form 8's object id in the enemy ids' table the engine reads (where the families' room has grown it, its copy at `+0x336000`, through the room's own writer, `xg_bn6_entry`) | `soulproxy.c`, `soulunite.c`, `soullife.c`, `soulcode.c`, `soullook.c`, `soulpower.c`, `soulpassive.c`, `soulchips.c`, `soulcolonel.c`, `xstrike.c`, `soulchaos.c` |
| `+0x4C0000`-`+0x4D0000` | BN5's lava and sea in BN6's battle engine (issue #82; docs/MULTIROM.md, One net; docs/ROM_DATA.md, BN5's lava and sea in BN6's engine), written once a core as it starts where BN5's ROM is read, at fixed addresses (IWRAM's copy of the flags' literal names the first, and a state holds IWRAM): `+0x4C0000` BN6's panel flags table grown to 16 words (BN6's 13, BN5's lava's at 13, its sea's at 14, type 0's at 15), the literal of its IWRAM routine pointed at it; `+0x4C0040` the lava's cooling, a few instructions of the project's own Thumb (the panel set plain, the spark's place moved by a random number, the spark), and its four veneers on to BN6's routines after it; `+0x4C0100` BN6's panel tile table grown to 15 types, lava's and the sea's entries BN5's with their tiles at 0x295 on, its literal pointed at it; the rest free (the tiles themselves go to VRAM from C as a battle first shows them, the panels' draw hook below: a tile set grown with them, which the game decompresses as every battle begins, made BN6's own battles start later and their runs differ) | `xpanels.c`, `xpanels_look.c` |
| `+0x4D0000`-`+0x500000` | Free | |
| `+0x500000`-`+0x600000` | BN5's Navis in BN6's battle engine (issue #82; docs/MULTIROM.md, Transplanted Navis; docs/ROM_DATA.md, Navis moved into the battle engine), written once a core as it starts where BN5's ROM is read (`--dev xnavicontrol` the control instead, `nonavis` none), at fixed addresses, the same at every boot with the same ROMs: `+0x500000` their battle sprites, each at a place of its own (`xnavi_sprites.c`: Colonel's first, the Dark Soul's at `+0x544000`), listed at list 2's 0x20 + n - 7, and from `+0x57B000` the Navis' arm points' records grown to every number of list 2; `+0x580000` BN6's thirteen Navi tables grown to 256 AI indexes (`0x400` apart), the ice block's sizes (`+0x583400`), the held chips' words (`+0x583500`), the companion's start and end (`+0x583900`, `+0x583D00`), the overlay's masks and sprites grown to 64 kinds (`+0x584100`, `+0x584200`), the moved Navis' names (`+0x584400`); `+0x588000` their homes, 16 of `0x800` (the families' layout), the last the moved objects'; `+0x590000` the Navis' stubs (`0x60`: r0 0, the DS chip AI logged, the Navis' buster's setup, Colonel's sides' bits kept in his field, the Dark Soul's chip records by id), `+0x590060` BN5's sea's submerge's code (`0xA0`, `xsea.c`: the marker's spawn, the hit gate's two tests, the splash of an object the sea holds, their veneers), `+0x590100` BN5's engine objects moved once for every track (`xnavi_objects.c`: the image object's module and the sea's marker's, a block each), `+0x592000` the Navis' code, a room of `0x3C00` each (Colonel, ShadowMan, NumberMan, TomahawkMan, KnightMan, ToadMan, the Dark Soul, the control); `+0x5B0000` a room of `0x400` each for its overlay's masks, then from `+0x5B2000` the shared art: every sprite their attacks draw once, at a place and number of its own whichever Navi draws it (lists 4 and 5 grown, `+0x334000`'s), copied by the first install that needs it, to `+0x5FA000`; `+0x5FA000` the Dark Soul's chip records (`XN_DARK_RECORDS`: BN6's 512 records with the four bytes its chip AI reads BN5's, `0x5800` bytes) | `xnavi_sprites.c`, `xnavi_room.c`, `xnavi_install.c`, `xnavi_objects.c`, `xsea.c`, `xnavi_dark.c` |
| `+0x600000`-`+0x700000` | The code of BN5's chips moved into BN6's engine (issue #82's C3; docs/MULTIROM.md, The chips' code; docs/ROM_DATA.md, BN5's chips' code in BN6's engine), written once a core as it starts where BN5's ROM is read, all or nothing, after the moved objects and the Navis and before the families (`xinstall.c`: a type the moved objects or a Navi moved its units spawn as that copy, and a family one of theirs as theirs), at fixed addresses (their records, names and art are at `+0x400000`): `+0x600000` BN6's attack families grown (`FREE_ATTACK_FAMILIES`, `xattack_grow.c`), one table for every track that adds an attack: 240 entries for CurAction 0x10-0xFF, BN6's 79 (0x10-0x5E) as the player's ROM has them, 0x60-0xBF the chips' (BN5's family F at 0x50 + F), 0xC0-0xCF the Souls', 0xD0-0xFF spare, every entry not given its stub at `+0x6003C0` (`ldr r0,[pc,#0]; bx r0` and `0x08011715`, BN6's object_exitAttackState: the attack ends), its two words (`0x0801BB10`, `0x080EC384`) pointed at it, each checked first; `+0x600400` BN6's support chips' routines grown to 256 (BN6's 0x2A as the ROM has them, the units' from 0x2A + BN5's subfamily), `+0x600800` its Navi chips' routines to 256 (BN6's 29, the units' from 0x1D + BN5's), their four words (`0x080ED138`, `0x08017BB8`; `0x080E2CF0`, `0x08017BBC`) pointed at them; `+0x600C00` their stubs (a support routine that spawns nothing, `movs r0,#0; mov pc,lr`, a panel claim's spawners' too, an unused computed power's and an unplayed subfamily's; a Navi routine that does nothing, `mov pc,lr`; `+0x600C08` r0 1, `movs r0,#1; mov pc,lr`, the SP powers' link bit; `+0x600C10` MegaMan's point (-1, 26) in r0 and r1, ElemRage's; `+0x600C18` his point 0x1D, (40, 20) by his facing, HolyDrem's; each returning by `mov pc,lr` as BN6's own routines do, since the tables' readers call through `mov lr, pc`, a return address without the Thumb bit that a `bx lr` would go on from in ARM state); `+0x600C30` the SP powers' best deletion times (`XC_SP_TIMES`, BN5's `0x0203E560` as they read it: 0xFFFF each, none, 0x2C a side); `+0x600C90`-`+0x602000` the helpers' code (`XC_HELPERS`: BN5's code several units call, installed once before them: the buster arm, the shake, the computed powers' routines; the muzzle flash BN5's image object, moved once for every track at `+0x590100` with its kinds' sprites, `tools/bn5_objects.py`), the only code besides BN6's and the moved objects' that a unit's calls may reach; `+0x602000` BN6's computed powers' routines grown to 256 (`XC_POWERS`: BN6's 45 as the player's ROM has them, BN5's entry i at 0x80 + i where a helper moved its routine, the rest the stub), its one word (`0x080109D4`) pointed at it; `+0x602400` each unit's veneers (the helpers' first), 64 homes of `0x400` in the install's order; `+0x612400` the units' code, block by block in `UNITS`' order, each the same distance from a word boundary as in BN5; `+0x6A0000`-`+0x700000` what they draw, each sprite in a room of its own (BN5's sprite of a list and number BN6 keeps another of, listed at a number of the chips' in the grown list: lists 1-5, list 3 grown at `+0x333900` for every track by this one), the chips taking the grown lists' numbers from `0xFF` down, the families from the bottom up, each slot checked to hold the fill first; the grown effects' ids 0xD0-0xFF from the top down; BN5's sounds BN6 has another song of at their slots fixed for every track (0x40-0x62, the chips' own from 0x52 down, `xsong_install_at`, docs/MULTIROM.md, The chips' code). BN6's chip records are named moved as the registry writes them (BN6's at `0x08021DA8` to `+0x400000`), so a trace or the differential's poke at BN6's own record reads the copy the game reads | `xattack_grow.c`, `xchips_code.c`, `xchips_bn5.c` |
| `+0x700000`-`+0x710000` | The Souls' art past their room (`FREE_SOULS_ART`, issue #82's grant to the Souls track): from `+0x700000` the sprites `tools/xsoul.py` places past `SOULS_ART` (none since the tracks' merge: the flash kinds' 3:0x03 and 3:0x00 are the moved objects' copies at the Navis' numbers 5:0xA6 and 4:0xB5, tools/bn5_objects.py, each sprite's bytes in one place), to `+0x70F000`; `+0x70F000`-`+0x70FA00` ColonelSoul's button's art, BN5's copied (`SOULS_COLONEL_ART`: its tiles on the chips' screen lit, grey and chosen, 0x180 bytes each, BN5's lit for chosen; its picture in the chip's window, 0x540 bytes; that picture's palette and the window's name line's), written once a core with the Souls' code | `soulcode.c`, `soulcolonel.c` |
| `+0x710000`-`+0x710400` | BN6's Program Advances' list grown with BN5's (`FREE_CHIP_PAS`, issue #82; docs/ROM_DATA.md, BN5's Program Advances in BN6's lists), written as the chips' code is installed, where BN5's ROM is read: BN5's recipes of the registry's in BN6's ids (`src/core/xchips_pas.c`, 6 bytes each), then at the end BN6's 43 recipes' words, BN5's after them and 0, the Custom screen's word (`0x080295B4`) pointed at it | `xchips_code.c` |
| `+0x710400`-`+0x710440` | The player's chip AI's stub (`FREE_CHIP_AI`, issue #82): from BN6's idle state's hook, a call into BN5's chip AI for the player (moved with the Dark Soul's code) and the state left as BN5's leaves it (`movs r0,#1; pop {r4,r6,r7,pc}`), where BN5's ROM is read | `darkinvs.c` |
| `+0x710440`-`+0x800000` | Free | |

The guest core's ROM copy (BN5's: the differential's reference since the
guest left runs, docs/MULTIROM.md, Guest battles) is padded the same way;
past BN5's 8 MB, from `0x08800000` (`BN5_FREE`), the differential's record
(`+0x200`, its entities from `+0x210`: `guest_dev_battle`, docs/
DEVTOOLS.md, The differential). In BN5's own data it writes the encounter
roll (its answer) alone. Until issue #82's last round a run's guest
battles wrote there too: a battle's record copy (`+0x00`) and entity list
(`+0x20`), the DarkChips' deck shelf (`+0x100`), the deck's compaction,
its chips' records, reward rows and finds (All *, the rewards' codes and
zenny), a guardian's stats row, and its event flags for the run's Souls.

A battle with a green Mystery Data on the field also rewrites the reward
row its record names (rows 0 and 1, one per record, of the table at
`0x080211A0`, docs/ROM_DATA.md) with the run's own rewards before the
battle; one of BN5's records fought in BN6's engine (issue #82, the
xrecord step, `src/core/xbattles.c`) with BN5's own row of finds, as
BN6's (docs/ROM_DATA.md, BN5's records in BN6's engine).

Where BN5's ROM is read, as the core starts (`xvirus_install`), one more
of BN6's tables is written in place (issue #82, docs/MULTIROM.md): 26
battlefields that no BattleSettings of BN6's names (12 bytes each at
`0x0800D730` + n x 12: 0x09, 0x0D-0x0F, 0x11, 0x1D, 0x22-0x25, 0x28,
0x2F-0x31, 0x46, 0x55, 0x63 and 0x64, and for BN5's lava and sea 0x56,
0x5F, 0x61, 0x65-0x67, 0x69 and 0x6A) with fields of BN5's that its
areas' records name, their panels' types as BN6's (`src/emu/xfields.c`;
docs/ROM_DATA.md, Battlefields and their panels' types), each first
checked to hold BN6's own. It is not in a state: every boot with the
same ROMs writes them the same, and BN6's own battles name none of those
fields. The reward rows of the ids a virus family moved into the engine
takes (from 0x200; CanGard's 0x200-0x205) are its game's rows for its
versions, each entry as BN6's (a chip as BN6's of its name, else 200
zenny), in the grown copy of the rows (`+0x337000` above), each first
checked to hold the filler the copy gives the ids past BN6's. For BN5's
lava and sea, BN6's panel types 13 and 14 (`src/emu/xpanels.c`;
docs/ROM_DATA.md, BN5's lava and sea in BN6's engine), two of BN6's
IWRAM routines are patched too: the literal that
names the panels' flags table (`0x0300791C`, pointed at the grown table
in the free space), the element hit's volcano and conveyor tests
(`0x03007742`-`0x03007771`, made again with lava cooled under Aqua as
the volcano is) and the panel bonus's tests (`0x03007678`-`0x0300768D`,
made again with an Elec hit on sea counted twice as BN5's are, after
Fire on grass), and for BN5's sea's submerge (`src/emu/xsea.c`;
docs/ROM_DATA.md, BN5's sea's submerge and hold in BN6's engine) the hit
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
time, by a hook on the reward pick just before it reads them (docs/META.md,
docs/ROM_DATA.md); with the All * helper every chip entry in *.

While a run has the All * helper (docs/META.md, issue #18), every chip
record of ids 1-511 holds * alone (its four codes at the chip records'
place, `EMU_FREE` + `0x400000` + id x 0x2C, written `1A FF FF FF`), and the Program Advance check's `cmp r2, #1`
at `0x08029606` is `cmp r2, #3`; each new layer writes them as the run's
helpers have them, and the ROM's own back before the game's own NEW GAME
boots (the folder it gives is counted in the pack by the records). As
such a run begins, the folder's 30 entries go to * and each chip's four
pack counts to its first.

BN6's own DarkChips (docs/META.md, docs/ROM_DATA.md; issue #70): as each
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

BN6's chip tables (issue #82; docs/MULTIROM.md, The chip registry;
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

In the rival's netbattle (docs/RIVAL.md), ProtoMan's HP and MaxHP are
given the act's guardian band at most as he spawns, where his own 1800 is
above it: a hook lowers the value BN6's spawn sets both from.

In any of BN6's battles, ProtoMan landing from his cross gets his
collision region (his collision data's `+0x01`) back on his own panel
where the cross left it cleared, as BN6 does for a cross cut short but
not for one that ends, which left him untouchable (docs/FIDELITY.md,
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


At a new run's start the engine writes the chosen starting folder (docs/META.md)
over the game's first folder: 30 u16 at the chip data eToolkit `+0x48` points
at (docs/ROM_DATA.md, chip folders), as BN6's own GiveFolder copies one in.
The Standard folder writes nothing.

### BN6's RAM

The engine keeps its own state out of BN6's RAM but where issue #82's
RAM plan gives the moved content room that BN6 leaves alone, by one plan
every track follows (its numbers `RAM_*` in `src/emu/free_space.h`, which
`tests/test_core.c` checks in order and apart, `BN6_PACK_EXT` and
`BN6_DISPATCH_SIDE` at its places):

| Range | Contents | Code |
| --- | --- | --- |
| `0x02010CE0`-`0x020115DF` | Persistent: the chip pack's extension, ids 320-511, 12 bytes an id (`BN6_PACK_EXT`; docs/MULTIROM.md, The chip registry) | `xchips_install.c` |
| `0x020115E0`-`0x0201172F` | Persistent: MegaMan's AI data, BN5's 0x140 bytes, where BN5's ROM is read (`RAM_PLAYER_AI`: his object's +0x58 pointed here as he is set up, `0x080172F6`, his pool's 0x100 bytes copied first, the rest zero; BN5's chip AI keeps its state from +0xF0 to +0x13F, past BN6's 0x100), then at `0x02011720` his place in BN6's pool; DarkInvs's machine, docs/MULTIROM.md, The chips' code | `darkinvs.c` |
| `0x02011730`-`0x0201174F` | Persistent: `--dev arena2v2`'s list of what a NetBattle spawns, two MegaMen a side (`RAM_TEAM`; epic #167's spike, docs/ROM_DATA.md, Two MegaMen a side) | `arena_team.c` |
| `0x02011750`-`0x0201178F` | Persistent: two MegaMen a side's partners' words (`RAM_TEAM_LINK`, `0x20` bytes a side, each core's own): a side's GBA's outbox of its partner's messages (12 u16 at +0, its head +0x18 and count +0x19: `team_outbox.h`), his keys for its next packet (+0x1A), and his keys and word as his side's last packet brought them (+0x1C, +0x1E), which `BN6_KEY_COPY` gives him; written by the pair before the GBA's frame and where the battle reads the packets and stores its pad (`BN6_LINK_PAD_STORE`; in sub_801FFD6 `BN6_LINK_READ_B` and `BN6_LINK_PAD_STORE_B`), emptied by the battle's spawn (docs/ONLINE_ARENA.md, The partners' words in BN6's packet) | `arena_team_link.c` |
| `0x02011790`-`0x020117FF` | Persistent: `--dev arena2v2`'s partners' decks and turns (`RAM_TEAM_CUSTOM`: `0x38` bytes a side, the same in both of the pair's cores: his deck as folder indices in the order drawn, his turn's state, the picks his stream has said, the iteration they go into his hand and their folder indices), written by the pair as it samples his side's GBA's frames and read by `BN6_KEY_COPY`'s hook; emptied by the battle's spawn (docs/ONLINE_ARENA.md, A partner's own Custom screen) | `arena_team_custom.c` |
| `0x0203FB50`-`0x0203FBDF` | Per battle: BN5's sea's submerge (`RAM_SEA`; `XSEA_MASK` the submerged, a bit each by its collision data's own, `BN6_COLL_INDEX_BIT`; `XSEA_MARKERS` each one's marker by that bit's index, 32 words; 0x84 bytes used), which BN5 keeps in the collision data where BN6 keeps its bubble | `xsea.c` |
| `0x0203FBE0`-`0x0203FD9F` | Per battle: the Dark Soul's chip AI's lists (`RAM_DARK_LISTS`, `BN6_DARK_LISTS`: BN5's `0x02034C20` laid out as BN5's, 0xE0 a side, BN6's chip ids), empty (0) as each battle is set up | `xnavi_dark.c`, `xnavi_bn5.c` |
| `0x0203FDA0`-`0x0203FDAF` | Per battle: the moved Navis' own state (`RAM_NAVIS`): Colonel's sides' bits (`BN6_COLONEL_SIDES`, which BN5 keeps in BattleState +0x5C), the object the Dark Soul's AI data is now and its own in the pool (`BN6_DARK_AI_OWNER`, `BN6_DARK_AI_POOL`) | `xnavi_room.c`, `xnavi_dark.c` |
| `0x0203FDB0`-`0x0203FEEF` | Per battle: the Dark Soul's AI data (`RAM_DARK_AI`, `BN6_DARK_AI`: BN5's 0x140 bytes, its chip AI's state past BN6's 0x100), one Dark Soul's at a time | `xnavi_dark.c` |
| `0x0203FEF0`-`0x0203FEFF` | Per battle: the moved chips' own state (`RAM_BATTLE`): BN5's green flash's frames, a u16 a side (`XC_FLASH`, which BN5 keeps in his AIData +0x3C, BN6's resting height; docs/ROM_DATA.md, BN5's DarkPlus and its green flash), zeroed with the battle's RAM; `+4` on kept for the moved families' | `xchips_flash.c` |
| `0x0203FF00`-`0x0203FF5F` | Per battle: the second-level dispatch's side tables (`BN6_DISPATCH_SIDE`: a byte an object slot, its game's type, 32 for each of the first, third and fourth kinds), written by the dispatch's spawners and read by its routines | `xvirus_dispatch.c` |
| `0x0203FF60`-`0x0203FF6F` | Per battle: BN5's Colonel's table moved (`BN6_COLONEL_TABLE`, `RAM_COLONEL`: two 8-byte entries by side, which BN5 keeps at `0x02034000`), written by his moved code and read by his attacks' objects, zeroed with the battle's RAM as each battle is set up | `xnavi_bn5.c` |
| `0x0203FF70`-`0x0203FFEF` | Per battle: the Souls' state in BN6's battles (`BN6_SOULS`: 0x20 bytes a side, and room for the track's later milestones; each side's +0x0E and +0x0F BN5's mood as BN6's battles keep it and whether a BN5 DarkChip made him dark, +0x06 the chaos level, +0x16 the Chaos Unison's charge's frame in its phases' cycle and +0x17 its release's request standing (`BN6_SOUL_LEVEL`, `BN6_SOUL_CYCLE`, `BN6_SOUL_REQUEST`: BN5's AIData +0x6C and +0x1C; docs/ROM_DATA.md, Chaos Unison), +0x14 the chip ColonelSoul's button took, committed at OK (his obstacle strike's side is `BN6_COLONEL_SIDES`, the Navis' field a Navi Colonel's spawn sets too, never BattleState +0x5C's bits, which BN6 reads as its own); docs/ROM_DATA.md, BN5's mood in BN6's battles; docs/ROM_DATA.md, UNITE on BN6's Custom screen, and a Soul's life on the proxy form): so every battle begins with no Soul, read and written by the Souls' hooks alone; from +0x40 each side's partner's own block under `--dev arena2v2` (`BN6_SOULS_PARTNERS`, laid out as a side's: his mood and his dark, never a Soul; `soul_field_of`) | `soulunite.c`, `soullife.c`, `soulmood.c`, `soulcolonel.c`, `soulchaos.c` |
| `0x0203FFF0`-`0x0203FFF3` | Per battle: two MegaMen a side's count of the battle loop's iterations (`RAM_TEAM_TICK`, a u32 the battle's setup zeroes), which both GBAs keep alike: a partner's picks go into his hand at the iteration after his side's screen closed, and the GBAs are compared by it | `arena_team.c` |
| `0x0203FFF4`-`0x0203FFF7` | Per battle: the entries of BN6's AI data's pool BN5's chip AI took for its rooms (`RAM_CHIP_AI_ROOMS`, a u32 of `BN6_AI_POOL_USED`'s bits, zero as each battle is set up; issue #182, below) | `darkinvs.c` |
| `0x0203FFF8`-`0x0203FFFB` | Per battle: two MegaMen a side's turn's clock (`RAM_TEAM_CLOCK`: a u16 of frames since the first OK of the turn the core has seen, then a byte 1 once seen; zero outside the Custom screen and with the battle's RAM), each core's own (docs/ONLINE_ARENA.md, The leads wait for their partners) | `arena_team_custom.c` |
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
docs/ONLINE_ARENA.md, The partners' own NaviStats). A room taken in
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

## A layer

1. `net_gen.c` generates rooms, walkways and objects from the run seed.
2. `netmap.c` builds the area's tile map from tiles learned per class from
   one original map (`area_src.c`), and its wall list, and points the map's
   descriptor and coordinate data at them.
3. `layer_objs.c` turns the objects into NPC scripts (`npc.c`), text
   (`text.c`, `scripts.c`), Mystery Data and shop stock, and `mapslot.c`
   points the map's NPC list, scripts, objects, Mystery Data and sprite list
   at them.
4. `gamecall.c` warps MegaMan in with the game's own warp routine.

The exit pad is the game's own warp pad: trigger cells that take warp 1 of
the map's warp list. When MegaMan steps on it, the game starts its jack-out;
the director builds the next layer meanwhile and points warp 1 at its start,
so the game jacks him in there. A guardian keeps the pad hidden and shut
(flag `0x16F1`) until its Guardian Data is taken (docs/BOSSES.md). A Yes to the Undernet or the Secret Area
starts the same departure (`0x080059B5`) to a side layer built at once.
On an act's last layer the pad leads home instead (docs/HOME.md,
`director_home.c`): the next layer is built as ever, Lan's HP is
installed (`lanhp.c`) with that layer behind its first portal, and warp 1
points at its arrival, BN6's blue pad, where Lan's PC's jack-in lands. In
Lan's HP the director holds the open links' story flags every frame (BN6's
link markers lock the others), and a portal's warp, seen pending as it
starts, builds its way's layer where another's was built and points the
portal's own warp entry at it before the game reads it. The run's town
stays installed, its jack-in to Lan's HP's arrival, and GameState's saved
real-world place at its port, so BN6's own jack-out from Lan's HP (R, or
the blue pad's warp) sets Lan down there.

`director.c` then watches the game: the exit pad's warp, choices made in text (event flags set by
Yes), battles won (viruses counted, bosses beaten), the game's GAME OVER
(the run ends), MegaMan on any other map for 90 frames (warped back to
the layer's start), and a checkpoint shortly after each arrival (`run.sav` plus
the core's `run.state`), once MegaMan is free to move: free as BN6 tests it
before the pad and START (`src/director/director_hold.c`: no fade, cutscene,
chat, lock, scripted walk, conveyor, NPC chat, warp, PET or pause), as every
save waits (the arena door's, home's, the PET's Save's, the quit's; issue
#23: a state written while BN6 held him came back held). A CONTINUE is
watched until MegaMan first walks: held from its load on with nothing of the
director's under way for two seconds, or the pad pushed three seconds with
no step taken (the player object's `BN6_PLAYER_PAD`), BN6's holds are let go
as its own routines let go of them and the map is entered again (then the
layer's start); a map's entry waiting on a fade's mark no fade runs is let
go too. The dev step `stuck NAME` (play.py, `--input "0:stuck conveyor"`) saves
the run held so, to test it. Quitting (Escape
twice, SIGTERM or SIGINT from a launcher) saves the run once more where he
stands if he is free on the layer's map with nothing of the guardian under
way; loading it restores the guardian's state from its flags. The shops'
stock is written again after the load (the ROM copy the shop screen checks
the list against is not in a state), each entry the saved list holds
keeping its stock there, and `run.make` records the build's `LAYER_MAKE`: a run saved by a
build that makes layers otherwise continues its layer from the start, with
the layer's flags and Mystery Data picks cleared (its RAM would not match
this build's objects). `battle.sav` keeps what the random battles
remember of the last two (their virus families, loot.h), so the first after
a CONTINUE brings none of them either. `run.seen` beside it keeps the map's panels seen so
far, and event flag `0x144E` in the state that L has told where they are,
which CONTINUE clears so that L starts over (`0x144F`: that the Net
Dealer has said his words, so a later talk is a line and the list;
`0x1450` the same for the NaviCust vendor, `0x1451` for a
Recovery Mr. Prog, whose heal is then one box, and `0x1464` that
MegaMan's HP is full, the director's each frame on the map, so he keeps
his patch; `0x1452` that a super
boss's staging has Bass's stone shake and crack (it was the Guardian
Data's second way on, until the act's ways became ports at home, issue
#86; docs/BOSSES.md, Super bosses); `0x1453`
that the layer's collector's vault gave its chip; `0x1454` that the super
boss strikes his pose out of the white (it was the Guardian Data's dark
way into the Undernet); `0x1455` that the
layer's official gate gave its chip, `0x1456` that Chaud's clearance
opens it, `0x1457` that Chaud's call on a duel layer was made,
docs/RIVAL.md (`0x146E`, which named the chips that sat out of the
guest core's battles, is free since they left runs); at home `0x1460`-`0x1462` that request 0-2 was taken, `0x1463`
that one is held and `0x146A` that the held one was settled, which the
askers' scripts set and the director keeps as each visit's places are
installed, docs/HOME.md piece 5; `0x146C` that the town holds nothing
more for this visit, which the director keeps every frame at home and
which sends Lan's HP's courier and his mark off, piece 2; `0x146F` the
Yes of a link's question in Lan's HP (a trip back's or a way on's), which the director clears
before it asks and acts on as the talk closes, and `0x1470`-`0x1471`
set while Lan's HP's link on the left and down front is no way back,
which sends their way-back marks off, piece 2; `0x1472` that the
courier has been heard, which his words set and which sends his burst
off, piece 2; `0x146B` that this visit's order was
made at AsterLand's counter, piece 6; `0x1473` the Yes of the Net
Square's gate's questions and `0x1474`-`0x1476` its keeper's options,
the official Square, another server and the name, piece 11, which the
director clears as each visit home begins). In the Net Square itself,
whose RAM is thrown away on the way home, `0x1480`-`0x14D0` are its own:
the phrases and emotes asked for (`0x1480`-`0x148D`, and emotes 6-11's
`0x14CA`-`0x14CF`, issue #176), the channel
keeper's next and last (`0x148E`, `0x148F`), a peer's wave, mute and
block (`0x1490`-`0x1492`), the way home (`0x1493`), Trade Central's
options and the Navi picked (`0x1494`-`0x149F`), a NetBattle asked for
and the Yes to one (`0x14A0`, `0x14A1`), the folder a NetBattle carries
(`0x14A2`-`0x14A4`), the greeter's dive into the week's net (`0x14AF`;
with the way home's `0x1493` set beside it, the unfinished dive begun
again; issue #127), the spare picked from the clerk's chat-box list
(`0x14B0`-`0x14C3`), and the referee's Yes to watching a NetBattle from
the stands (`0x14C4`, issue #128), L's Look around (`0x14C5`), a team
asked for, its Yes and its order or a team challenge's format (`0x14C6`-`0x14C9`), and L's quiet
(`0x14D0`, issue #176);
docs/ONLINE.md. Its life
(`square_life.c`: the walkers, the log-in beam, the cheer, the greeter's
and the referee's words) takes none.
CONTINUE loads the state and enters the map again, so the game reloads it
from the current build's tables.

It also starts conversations of its own (`talk.c`): Lan and MegaMan on
arriving somewhere new (the first layer, the Undernet and the Graveyard,
the Nest, a side layer, a net rebuilt after the Nest), Chaud's call after
the Secret Area's guardian, and MegaMan's word on where they are when L is
pressed on the map (in battle L still opens the Custom screen). Each is
built into the talk slot and run by the game's chat box
(`BN6_CHAT_RUN_SCRIPT`), once the area card has gone and no guardian scene
is playing; the player's keys only page it until the box closes. In the
town the first conversation is the town's own (its text archive, Dad's
call on the very first run).

## A new run: the town

NEW GAME builds the run's first layer and the town (docs/OVERWORLD.md),
warps Lan to where the town starts him (his front door in Central Town,
the Metroline's stairs in ACDC Town) and clears the flag that stops
jacking in (`0x1727`); the PET's Save stays off. While Lan walks the town
the director waits: the town's jack-in cells and jack-in table send the
game's own jack-in to the first layer, and when MegaMan has arrived there
the run goes on as after any layer's warp (the flags set again, a
checkpoint). A NEW GAME deletes the last run's save at once, so CONTINUE
always resumes a run in the net.

## Boot

`emu_boot` resets the core, presses through the title and NEW GAME, warps to
Seaside Area 1 and ends the intro cutscene, then saves `boot-3.state` in the
data directory. Later runs start from that state. It is made on the device
and never shipped.

The boot's frames are never shown, so the core draws no picture for them;
nor for a frame the main loop plays unshown to catch up with the GBA's pace
(`emu_frame` sets the core's frame-skip counter for that one frame, where
its struct reads back as expected).

## The core on its own thread

On the New 3DS the GBA core takes most of a frame, so it runs on the third
core where the system gives it one (`emu_threaded`). `emu_frame` then starts
the frame and returns, and every other access to the core (`emu_read*`,
`emu_write*`, states, the sound's rate) waits for that frame first: the
director's logic takes in each frame whole, as on the main thread. The emu
scene's update takes the frame done in and then starts the next, which runs
while the frame done is drawn from a copy of its picture; keys, frames and
the logic keep their order, and the autopilot's run log comes out the same.
`CYBERWORLD_EMU_THREAD=1` runs it so on a computer, to test it. With the
core on its own thread, mGBA's threaded video draws its picture on one
more (`threadedVideo`), beside the emulation; the worker waits for it at
each frame's end, so the picture copied is whole. On the 3DS that thread
is on the main core, which waits most of a frame for the emulation.

A run has one core, BN6's, on every build since issue #82's last round:
BN5's guest core, which booted on a thread of its own (the browser's
frames' spare time) from the game's start on, left runs (docs/MULTIROM.md,
Guest battles), and the differential's boots it on the main thread.

## The browser's memory

The browser build's WebAssembly memory starts at 64 MB
(`-sINITIAL_MEMORY=64MB`) and grows as the game asks, never shrinking;
what it reached is the page's high-water mark (`Module.HEAP8.byteLength`
in the page). Measured on 12 October 2026 in headless Chrome
(agent-browser), both ROMs given, the same saves loaded from a backup (a
run at the arena door of End Area's third layer, an older net's act, seed
3), each build served by `build.py serve`:

| | This build (issue #82's last round) | Before it (a9d98987, the guest core in runs) |
| --- | --- | --- |
| The title | 64 MB | 64 MB |
| A new game's first layer (Central Area) | 110.6 MB | (not measured) |
| CONTINUE on the older net's layer | 132.6 MB | 159.2 MB (the guest core booted with it) |
| A battle there (CanGards, BN6's engine) | 132.6 MB | 159.2 MB |

So a run with BN5's ROM keeps some 27 MB less: the guest core's ROM copy
(BN5's 8 MB padded to `EMU_ROM_SIZE`, which mGBA copies at its first
patch) and its own state are gone. The JavaScript heap stayed near 20 MB
in this build. Issue #68's measure, before BN5's battles came into BN6's
engine: 97 MB on a first layer with BN6 alone, 153 MB on BN5's first
layer with the guest booted, 197 MB at most.

## The 3DS's memory

The New 3DS gives the game 124 MB. `src/core/start_3ds.c` splits it before
`main`: the heap takes all its area holds, 96 MB, and the linear heap the
rest, some 13 MB (the screens', the sound's, and the GBA's own RAM, which
mGBA's 3DS setup takes there). The heap is what runs short. What it holds
for a whole session:

| | The 3DS before issue #82's 3DS round | BN6 alone now | With BN5's ROM now |
| --- | --- | --- | --- |
| mGBA's ROM buffer | 16 MB | given back | given back |
| BN6's ROM (`R.data`, the player's bytes) | 8 MB | 8 MB | 8 MB |
| The core's copy (`EMU_ROM_SIZE`, patched) | 16 MB | 16 MB | 16 MB |
| BN5's ROM (`XR`) | | | 8 MB |
| All | 40 MB | 24 MB | 32 MB |

mGBA's 3DS setup (its `ctru-heap.c`, linked in with the core) takes its ROM
buffer before `main`, 16 MB as `docker/mgba.sh` builds it, for loaders of
its own (`mCorePreloadVF`, `mCoreLoadFile`) the game never calls: `emu.c`
hands the core its padded copy as a file in memory, which `GBALoadROM`
maps as it is for a 16 MB ROM and patches in place (mGBA 0.10.5's
`src/gba/gba.c` and `memory.c`'s `_pristineCow`). `start_3ds.c` gives the
buffer back at the start. BN5's ROM is then read whole, as on every
build, where the heap still has 80 MB free for it: its own 8 MB, the
core's copy still to come and 56 MB of play (a New 3DS has some 88 MB
free there); short of that it is left unread, the run is BN6's, and the
log and the PET's home say so.

What play takes was measured on a computer (`tools/heap3ds.py`,
docs/DEVTOOLS.md: the host build's allocations counted as the 3DS's heap
holds them, the core's copy kept, one past 96 MB refused). The installs
keep little of the heap (BN5's code and data, some 2 MB of its file read,
go into the core's copy, past the game's 8 MB). A layer's area learned
from its original maps takes up to 50 MB at once (a map, its mirror image
and its raised views, each a whole map's pixels twice), and what it
learned stays, 2 to 7 MB an area. Kept for every area a run had passed
through, those filled the heap in long runs: with the 3DS as it was, 20
of 24 long runs (seeds 1-24, the test autopilot kept alive, 120000
frames, 5 to 41 layers) had allocations refused, 544 in all (a map's
mirror image not learned, a book of its tiles lost, a cache left out).
So the 3DS keeps the learned maps of the layer's own area alone
(`src/map/netmap.c`), and an area a run comes back to is learned again,
the same (the endless net's acts once round: some 4 times a long run;
`log.txt` says "area N learned again"). Two areas kept refused
allocations in 17 of the 24 runs with BN5's ROM read.

| The same 24 long runs | The 3DS before | BN6 alone now | With BN5's ROM now |
| --- | --- | --- | --- |
| The heap's peak | 96 MB (full) | 87.7 MB | 95.8 MB |
| Runs with an allocation refused | 20 (544 refused) | 0 | 5 (6 refused: a speed cache of the tiles' learning each, its result the same) |
| Crashes | 0 | 0 | 0 |
| Areas learned again | 0 | 95 | 111 |

With BN5's ROM the heap is near full at a long run's worst: modelled 6 MB
short (`--cap 90`, what the 3DS's fragmentation might take), the same runs
with BN5 refused 332 allocations in 20 of the 24, 39 of them a book of an
area's tiles lost (its layers drawn from fewer of its original tiles),
none crashed; BN6 alone refused none. Two crashes the model found on the
way, an allocation refused where its result was used unchecked, are
mended: the pads found in a map's floor
(`tl_find_pads`) and the tile pairs its neighbours show (`seams_add`).
Other places in the map's learning still use an allocation unchecked
(`tests/lint/analyzer.txt`'s `decor.c`, `props.c`, `stairs.c`). The model
counts bytes, not the 3DS's fragmentation, so the device's own numbers are
`log.txt`'s: the heap after the ROMs are read ("3ds: BN5 read and checked
... heap N KB in use, M KB at most of 98304 KB") and after the installs
("3ds: the core started, the installs done"), and with `frame_log = on`
each second's line, whose "at most" is the heap's high-water mark (malloc's
trimming is off on the 3DS, so the space it took never shrinks).

BN5's ROM read in windows from the SD card instead, as a run needs it,
would keep its 8 MB out of the heap: its readers (the installs, the
dressing of a layer, its records and text, some 40 files over
`XR[].data`) read about 2 MB of it at the start (the installs, measured
page by page) and a layer's dressing 70 to 140 KB more. With the buffer
given back and one area's maps kept, the heap holds the file whole, and
every system reads BN5 alike. Where a New 3DS's own log shows BN5's long
runs short (the heap's "at most" at its size, `log.txt`), those windows
are the next room to make: some 6 MB.

## The core's time

Measured on a computer (issue #35), with a build that sampled the GBA's PC
every 1000 cycles and timed each frame with its picture drawn and, every
other frame, not, over autopilot runs: a walked layer, battles, the town.

| Scene | Emulation, ms a frame | With the picture | Busy-waiting for VBlank | With the idle hook |
| --- | --- | --- | --- | --- |
| A layer | 0.52 | 0.64 | 48% of the GBA's cycles | 0.36 (0.48) |
| A battle | 0.53 | 0.67 | 57% | 0.35 (0.49) |
| The town | 0.51 | 0.59 | 44% | 0.37 (0.45) |

- **The picture** is about a fifth of the core's time on a layer and in
  battle. On the New 3DS mGBA's threaded video draws it on the main core,
  beside the emulation, so the core's own thread pays the interpreter.
- **The wait for VBlank** was half of the interpreter's work. BN6's main
  loop waits for each frame by reading DISPSTAT until its VBlank flag is
  set (bn6f `main_awaitFrame`), and mGBA's own idle-loop removal leaves a
  loop that reads DISPSTAT alone, since the flag changes with no
  interrupt to wake a halted CPU. A hook (`src/emu/idle.c`) halts the CPU
  there while the VBlank interrupt is on, which ends the halt where the
  flag would have ended the loop: the autopilot's logs and run logs are
  the same line for line, at about 30% less of the core's time. The
  Arena's two cores take the same hook on their own tables (`idle_on`),
  run a run loop at a time (docs/ONLINE_ARENA.md, Rollback).
- **What is left** is spread out: on a layer the hottest routine is
  bn6f `checkOWObjectInteractions` (`0x080037F4`, a sixth of the busy
  cycles, each object's interaction area against the others'); BN6's
  memory copies through the BIOS's CpuSet and CpuFastSet, which mGBA's
  own BIOS runs as interpreted ARM (a sixth); MP2K's sound mixer,
  copied to IWRAM at `0x03005700` (a tenth); the sprite and OAM code in
  IWRAM; in battle no one routine passes 3.3%.
- **No routine as C.** An answer hook that does a routine's work in C
  takes the routine's GBA cycles away, and with them BN6's own lag
  frames, where a frame's work runs past VBlank and the game takes two
  VBlanks for it. Tried on the object interactions (7-11% less of the
  core's time on a map, on a computer): the game ran one frame more in
  a layer's first second, its RNG a call ahead from there on, and the
  random battles came at other steps. The wait for VBlank is the one
  place the GBA's time goes without the game noticing: no work is
  skipped there, only the wait.

The sound is not the same sample for sample as 0.6.0's: the hooks (and
before them the stubs they replaced, whose own instructions took the
GBA's cycles) move the game's writes to the sound registers by a few
cycles, which shifts the phase of the GBA's tone and noise channels. Its
loudness, 20 ms at a time over 50 s of an autopilot run, follows 0.6.0's
at a correlation of 1.0000, the mean the same.

On a Retroid Nova (ROCKNIX), the same 90 s of an autopilot run (`--seed 7
--dev god,onehit`, `tools/device_run.py` with `frame_log = on`) took the
GBA 3.98 ms a frame with its picture on 0.6.0 and 3.09 ms with the hook
(the median 4.00 and 2.80).

On a New 3DS (`frame_log = on`, a minute or so of the town, a layer and a
battle each), the core's own thread took 15.9 ms a frame with 0.6.0 and
11.0 with the hook (the median), and the main thread's wait for it fell
from 10.4 ms to 1. The frames shown are paced by the refresh now: the
present waits for it where the core has its own thread
(`present_3ds.c`). Before, a timer paced them, and the long core frame
had steadied that timer by accident: with the core faster, 6 frames a
second came early and 8 late (gaps under 12.5 ms or over 20); with the
refresh's pace, 0.1 and 1.3, against 0.6.0's 0.1 and 2. An old 3DS or
2DS has no third core: the core and its picture share the main one,
and there the title stuttered and the net went black after a few
frames, as before (3ds/README.md).

## The core's sound

mGBA's blip buffers resample the GBA's sound to the device's 48 kHz, a
frame at a time, into `emu.c`'s ring, which the audio callback reads. The
GBA runs 59.7275 frames a second and the loop 60, so the core makes 0.45%
more sound than the device plays: the rate the blip buffers resample to is
nudged by how full the ring is (`audio_pace.c`), and the game's sound
plays 0.45% high and fast, as every 60 Hz emulator plays it. The nudge
reads the ring's fill averaged over about two thirds of a second. It read
the fill itself before: the device takes the ring in blocks (512 samples
on a desktop, 1024 on a handheld, more on some sound servers), so the
fill after each frame swung by a block, and the pitch with it, 10 to 18
cents from one frame to the next in a model of those reads (40 with
2048-sample ones), a warble on held notes; averaged, under a cent
(`tests/test_core.c`'s `test_audio_pace` holds it there, and fails on
the old way). A ring past `LAT_MAX` is still cut to its target, the
average with it.

What the device hears then goes through settings.ini's `sound`
(`src/audio/sound_filter.c`, the engine's own sound too): `gba` as it is,
`soft` with the Legacy Collection's balance. That was measured, not
guessed: its net and virus battle themes found in a recording of it by
their notes (chroma matched against our captures, 35 chunks of 6 s), each
chunk's third-octave spectrum against ours on the same bars. Flat from
300 Hz to 5 kHz, it falls away above (-4.5 dB at 8 kHz, -10 at 10, -18 at
13, -29 at 16) and below (-3.5 from 80 to 160 Hz, -12 at 40), the two
songs within a dB of each other. Soft is three biquads fitted to it,
within 0.8 dB from 35 Hz to 17 kHz: a low-pass at 6750 Hz (Q 0.88), a
high-pass at 68 Hz and a low shelf of -2.5 dB at 320 Hz. It takes the
images of BN6's 10512 Hz mix (docs/ROM_DATA.md, The sound's mix) down
with the rest of the highs. Neither setting touches the game: a headless
capture's frames are the same byte for byte with either.

## The engine's own sound

Before a run and beside it (the title and its summary, the setup,
Options, SAVES, the PET's screens over the held game) the core is not
running, and the engine plays BN6's songs and sounds itself. It does so
with BN6's own sound engine's rules: MP2K, Nintendo's "m4a", read from
BN6's code in bn6f's disassembly (asm/libs.s; the routines and tables in
Gregar, docs/ROM_DATA.md, BN6's sound engine) and written again as the
engine's own C (`src/audio/mp2k*.c`), its data the player's ROM's:

- **the sequencer** (`mp2k_seq.c`, `mp2k_note.c`): 32 music players as
  BN6_MPLAYS has them, the music on player 31 and each sound on the
  player its song names; MPlayMain's ticks at the song's tempo against
  150, the commands of BN6's jump table (MEMACC and XCMD as BN6's start
  puts them in), a note given one of BN6's four DirectSound channels (or
  its PSG channel) by priority and track, the LFO, the fade;
- **the mixer** (`mp2k_mix.c`): SoundMainRAM at 10512 Hz, 176 samples a
  frame into a ring of nine frames, the song's reverb from the ring's
  next part, each channel's envelope stepped once a frame, its samples
  (interpolated, or one a step at fixed pitch) added into 8-bit bytes
  that wrap and never clip;
- **the PSG's driver** (`mp2k_cgb.c`): CgbSound's software envelope, its
  extra step every fifteenth frame, and its register writes, which set
  NRx2's start volume and its hardware step and retrigger the channel;
- **the GBA's sound after it** (`gba_sound.c`), as mGBA 0.10.5 plays it:
  the registers' writes as its io.c makes them, mGBA's own PSG (GBAudio,
  run on a clock of its own with the GBA's frame sequencer), the ring's
  samples held from one timer 0 overflow to the next, a frame after they
  were mixed, and the output mixed as GBAAudioSample mixes it into
  blip_buf, read at 48 kHz.

It was proved against the game itself (`tools/mp2k_verify.py`): the plain
ROM in libmgba (romlab, mGBA 0.10.5 as the engine's) with its title's
song-table entry pointed at a song, its MP2K state recorded each frame
(romlab's `sound`), laid beside the engine's `--mp2k-trace` of the same
song from the same phases (`tools/mp2k_diff.py`): the music player and
its eight tracks, the four DirectSound and the four PSG channels, the
reverb and both sides of the mix the same byte for byte in every one of
900 frames, for the title (0x01), the credits (0x1D), the net (0x13) and
seven of the menus' sounds; two bugs planted (the reverb's rounding, a
tie between channels) each failed it. The 48 kHz sound then: the
DirectSound part alone at a correlation of 0.997 with the game's, the
same gain (0.9996), what differs rising with frequency, as the GBA's
timer phase against its 65536 Hz output moves the steps (the phase is
the game's since its power-on; `CYBERWORLD_SOUND_PHASE` sets it, about
584 cycles matching this capture to -30 dB); each PSG channel alone at
the game's level (within 0.4%) and loudness (0.998-0.9999 by 100 ms),
its waveform apart by its square's and its frame sequencer's phases,
which carry over from whatever the game played before. Through the
title's own path the title's song is within 0.4 dB of the game's (the
plain ROM in mGBA at 48 kHz) in every third of an octave, its level
within 0.05 dB. The player it replaced was the engine's own
approximation (float mixing at 48 kHz, its own envelopes, no reverb, 32
channels): 1.6 dB quieter, up to 9 dB short in the deep bass and 1.4 to
2.5 dB short from 7 to 14 kHz, which the owner heard as a duller title
than the cartridge's in mGBA.

`audio_set_volume`'s music volume (8, the default, the game's level) and
settings.ini's `sound` (sound_filter.c) apply after it; on the PET's
screens over a run its sounds add to the core's. There the engine runs
only while it has something to play (`mp2k_busy`), and a song just begun
counts: MPlayStart leaves its player's status 0 till MPlayMain's first
tick, which counted as idle would never come, and the screens' sounds
would never start.

**Another game's songs.** A song of BN5's plays on the same code: BN5's
MP2K library is BN6's byte for byte and its tables are BN6's
(docs/ROM_DATA.md, Another game's songs), so the engine runs BN6's
rules and tables on BN5's song data. Each music player keeps the ROM its
song is in, and each channel its player's at its note, and the part
running reads that ROM (`mp2k_read`): the sequence, the voices, the
samples and the PSG's waves; the library's tables are always BN6's
(`mp2k_lib_u8`). `mp2k_song_start_in` takes the game, `--render-song
x0/ID:SECONDS:PATH` renders BN5's song ID. BN6's songs stay byte for
byte the game's (`tools/mp2k_verify.py`, the title, the net and a menu
sound after the change); BN5's title is the plain BN5 ROM's within the
same bounds as BN6's (docs/SOURCES.md, The Jukebox's songs).

**The Jukebox** (docs/PET.md) holds the core still while it is up (no
frame of the game runs, so its music stops and its sound ring empties
to silence), and plays its songs on player 31 through `audio.c`'s
`audio_song_*`, which take the audio device's lock: the engine runs on
the sound's own thread, and a call into it from the frame's thread
waits for the mix in hand.

## Hooks

A hook lets one instruction of the game's code call the engine's C right
there, mid-frame, with the CPU's registers (`src/emu/hook.c`, issue #27):
the reach a native port's hooks give (Ship of Harkinian's, N64Recomp's
function replacement), while BN6's code stays the ROM's.

- **The BKPT.** A Thumb `BKPT #0xCE` (`0xBECE`), or an ARM one
  (`0xE1200C7E`), is written over the instruction in the core's ROM copy,
  as every patch is, or in RAM. mGBA runs a BKPT's handler at no cost in
  cycles, with the PC already past it (`src/arm/isa-thumb.c`). The board's
  handler, which mGBA installs once in `GBAInit` (a reset keeps ours),
  takes immediate 0 (the debugger) and 1 (the cheat device): ours comes
  first and passes it every other immediate.
- **After the hook.** `ARMRunFake` puts an instruction into the prefetch in
  the BKPT's place, as mGBA's own cheat hooks do: the original, which runs
  as if never replaced (`HOOK_CONTINUE`, with the registers as the hook
  left them); `bx lr` (`HOOK_RETURN`, r0 and r1 the result; at a routine's
  first instruction, before it pushes anything); or `bx r12` (`HOOK_JUMP`,
  on elsewhere).
- **Two kinds.** An *event hook* only queues the registers it met, and the
  director takes the queue up after the frame (`emu_hook_events`,
  `src/director/events.h` its kinds). An *answer hook* returns, or changes,
  what the director prepared before the frame, and may queue an event of
  its own where only some calls are worth one (`hook_post`). Neither
  touches the director's state.
- **Threads.** Where the core has a thread of its own (the New 3DS,
  `CYBERWORLD_EMU_THREAD=1`), a hook runs on it while the main thread
  draws. `emu_read*` and `emu_write*` wait for the frame in progress, so a
  hook reads and writes the game's memory with `hook_read*` and
  `hook_write*`, straight through mGBA's raw access; the queue is written
  only during a frame and read only after one, so it needs no lock. The
  browser build has no threads, and the same rules hold there.
- **States and resets.** A state holds RAM, not the ROM copy, so a hook in
  the ROM stays; one in RAM is written again after a state is loaded or the
  core reset (`hook_reapply`). A patch of RAM the game copies from a ROM
  image patched alike (IWRAM's routines, as the game powers on) is kept
  (`emu_keep`): saved with the game's own bytes, made the engine's again
  as a state is loaded. A hook taken off restores the original; its
  slot stays only while the CPU may hold its BKPT in the prefetch.
- **A table per core.** Each core attached keeps its own hooks and queue,
  found by the CPU that met the BKPT, and a hook reads and writes the core
  it runs on: BN6's (`hook_attach`, which `emu_hook` and the others use)
  and, for the differential alone, the guest's (`hook_core`, through
  `guest_hook`), so its hooks on BN5's code never touch BN6's core
  (`tests/test_emu.c`: one address hooked on both, each core's frames
  running its own hook alone, reading its own memory). Two
  cores' hooks run one core's frame at a time (the differential's
  frames run on the thread BN6's do), but for the Arena's pair under
  full rules (docs/ONLINE_ARENA.md, Full rules), whose two cores take
  BN5's installs' hooks on tables of their own (`xinstall_log.c`) and
  step an instruction at a time in turn: what a hook keeps between two
  of its calls is kept one a core (`hook_slot`: `xsea.c`'s and
  `xpanels.c`'s way back through their stubs, `soulchaos.c`'s charge
  stepped), and so are the Souls UNITE offers (`soulunite_held_core`).

The hooks in use (`src/director/encounter.c`; docs/ROM_DATA.md):

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x08005A98`, `0x08005AE2` | checkThenStartBattle: its first test, the branch after the roll | answer: a forced battle, the engine's record (issue #29) | once a frame on the map |
| `0x08005BC8` | StartBattle | event: the record the battle starts from | once a battle |
| `0x080AC180` | the reward pick (bn6f `sub_80AA910`) | answer: its enemies' reward rows rewritten first | once a battle, as it ends |
| `0x0800A4D2` | the battle's deck dealt from the folder (bn6f `sub_800A3E4` at its end, `src/emu/xchips_install.c`) | answer: the chips that sit out (a chip of BN5's whose code BN6 lacks, a DarkChip of BN5's, every one of BN5's without its ROM: docs/MULTIROM.md) taken out of the deck, the rest in their order, `0xFFFF` after them; the Regular chip's and the TagChips' marks (BattleState `+0x17`, `+0x44`) cleared where theirs sat out | once a battle |
| `0x08010D58` | every chip use's DarkChip check (bn6f `sub_8010D58`, `src/director/darkbn6.c`) | answer, posting an event (`hook_post`) where a DarkChip of MegaMan's runs as its base chip, no BugFrag in the battle's count | once a chip used |
| `0x0800B79A` | a chip's after-effects as it runs (bn6f `sub_800B79A`) | answer, posting an event where a DarkChip's dark power ran for MegaMan (its id 0x11E-0x122 reaches it only then): its price after the battle | once a chip used |
| `0x0800E2D8` | object_subtractHP | answer, posting an event (`hook_post`) where MegaMan's HP falls | in the rival's duel only: once per battle object a frame |
| `0x08007740` | the enemy spawn (bn6f `sub_800768C`), HP and MaxHP set | answer: the netbattle's ProtoMan held to the act's band | in the rival's netbattle only |
| `0x080FDFA0` | ProtoMan's cross landing back on his panel (bn6f `sub_80FCC4C`, `src/emu/protoman_cross.c`) | answer: his collision region given his own panel back where the cross left it cleared (issue #55; `CYBERWORLD_EMU_DEBUG` says each) | once a cross |
| `0x08005152` | EnterMap past its wait for the fade (`src/director/events.c`) | event: a map entered, after a warp or a battle | once a map |
| `0x0802F114` | SetEventFlag | answer, posting an event for a layer's choice flag (`0x1440`-`0x1447`) and a request's at home (`0x1460`-`0x1462`, `0x146A`) | about once a frame on the map |
| `0x0803CD6C` | GiveItem | event: a key item given (a ScrtData, the run's Spin) | once an item |
| `0x080050EC` | cbGameState, the game mode's state update (`src/emu/gamecall.c`) | answer: a queued game call jumps to its routine in the update's place, its return to `EMU_FREE` + `0x100` | once a frame of the game mode |
| `EMU_FREE` + `0x100` | where a game call's routine returns | answer: its r0 and r1 kept, r4-r11 put back, on to the main loop; a call with the update (`game_call_then_update`, issue #187) on into cbGameState instead, lr the main loop's, so the frame's sprites are drawn | once a call |
| `0x08120B90` | the PET's input handler (`src/director/pet.c`) | answer: A on Save taken for the engine (docs/PET.md) | each frame the PET takes input |
| `0x080003A6` | `main_awaitFrame`'s DISPSTAT loop (`src/emu/idle.c`) | answer: the CPU halted till the VBlank interrupt (The core's time) | about four times a frame (each interrupt wakes it) |
| `0x0801A802`, `0x0801AD3C`, `0x0801AD94`, `0x0801ADC8`, `0x0801AE24`, `0x080DA5FA` | each actor's status step (its pre-steps' first call), four obstacles' pre-steps and one object's routine: where BN5 steps an object on its lava first (`src/emu/xpanels.c`) | answer: an object on lava hurt as BN5 hurts it, then the lava cooled under a spark by a few instructions of the project's own (`EMU_FREE` + `0x4C0040`), which come back to the hooked instruction, run then | where BN5's ROM is read: once per actor and obstacle a frame |
| `0x0801A1AA` | what the panel under an actor does to it, its poison test (`xpanels.c`) | answer: a Fire object on sea taken as on poison | the same: once per actor a frame |
| `0x0800C3B8` | the per-panel update, its conveyors' test (`xpanels.c`) | answer: lava and sea counted down to plain as a conveyor, from BN5's 0x3C0 | the same: once per panel a frame |
| `0x0800CC0A` | object_setPanelType (`xpanels.c`) | answer: a chip's lava or sea made with BN5's 0x3C0 frames | the same: once a panel set |
| `0x0800C5E6` | the panels' draw, past its palettes' step (`src/emu/xpanels_look.c`) | answer: where lava or sea is on the field, its tiles written into VRAM at tile 0x295 on if a word of each says they are not there (a battle's first frame with them), and its colours over the volcano's or the conveyors', by BN5's animations (where those are not on the field) | the same: once a battle frame |
| `0x0802D644` | the delete count raised as a virus of version 0-3 is deleted (`src/emu/xvirus_grow.c`) | answer: returns at once for an AI index of 32 or more (a family moved into the engine, docs/MULTIROM.md), whose byte would lie past the 32 BN6 counts (`0x02000070`) | where a family is installed: once a virus deleted |
| `0x0802D652` | the delete count's read (the rare versions' check) | answer: 0 for an AI index of 32 or more | the same: once a rare version is looked for |
| `0x0802D65E` | a rare version's deletion (version 4-5: with event flag 0x1D8 set, the AI index put at `0x0203EAEA` for the chat of new virus data after the results) | answer: returns at once for an AI index of 32 or more | the same: once a rare version deleted |
| `0x0800EC56` | an enemy id's name (`BN6_NAME_OF`: the viruses' archive, the Navis' past 0xFF, and the id's low byte; `src/emu/xvirus_grow.c`) | answer: for an id from 0x200, a family's, the families' archive (`EMU_FREE` + `0x341000`) and its entry there, id - 0x200 | where a family is installed: once a name plate, as a battle begins |

The differential (`--xdiff`, dev only; docs/DEVTOOLS.md) hooks both cores
on each game's own routines: the four RNG draws, the spawns' common
routine at its start and its end, PlaySoundEffect, an effect's spawner,
setAttack, a panel reserved, the blink and sprite_load (docs/ROM_DATA.md,
The differential's hooks), and BN5's alone where a hit landed raises a
side's mood (`BN5_MOOD_GAIN`, which BN6 has not: MegaMan as the
differential's subject); `CYBERWORLD_XDRAWS` puts the draws' four on
BN6's core for the control's runs (`tools/xcontrol.py`).

The Souls' proxy form (`--dev soulproxy`, dev only; docs/DEVTOOLS.md, The
proxy form) hooks the three places Gregar's code reads MegaMan's form
itself, each answering form 8 as HeatCross's 1 and running on: the chips'
bonus by form (`0x0800EF62`, r0), his arm flash's look (`0x080EC8DE`, r0)
and the summon as he crosses (`0x08014BB6`, r4). With a Soul united the
first two answer the Soul's (BN5's bonus by form, NumberSoul's +10, on to
the bonuses' end `0x0800F090`; BN5's flash kind and the Soul as its
frame). The dev step `cross F`
hooks CROSSSELECT's commit (`0x08029394`): the next form it writes (r0)
is F, once.

The Souls on it (the same switches, or `--dev souls=MASK`; the Souls
track's S2, `src/emu/soulunite.c` and `soullife.c`; docs/ROM_DATA.md,
UNITE on BN6's Custom screen, and a Soul's life on the proxy form) add
fourteen, each an answer that leaves BN6's own way alone while the run
holds no Soul:

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x08027EC6` | the Custom screen's set-up, at Beast Out's test for element 0xB (`BN6_UNITE_STAND`) | answer: element 0xB stands for UNITE too (BN5's rule), its role and art chosen; a battle's first screen opens his mood as his HP says; a Soul's screen marked | once a Custom screen |
| `0x0802A57A` | the emblem's lit test (`BN6_EMBLEM_LIT`) | answer: grey for UNITE (the check below lights it), and for the emblem while a Soul is united | as the screen draws element 0xB |
| `0x08028F48` | element 0xB's update after each pick (`BN6_UNITE_LIT`) | answer: UNITE lit for a Soul the last pick lights (BN5's check), else the emblem by BN6's rule or UNITE grey | once a pick |
| `0x08028D6C` | A on element 0xB (`BN6_EMBLEM_A`) | answer: BN5's own code for UNITE, moved to `EMU_FREE` + `0x480012`, into mode 0x24 | once A on it |
| `0x08029072` | B on the last pick (`BN6_EMBLEM_B`) | answer: UNITE's chip given back, its icon redrawn, on to BN6's own end of it (`0x080290F2`) | once B on UNITE |
| `0x08029418` | the picks leaving the deck (`BN6_EMBLEM_GIVEN`) | answer: UNITE's chip leaves it too, as in BN5 | once a screen's OK |
| `0x08029150` | OK's commit of a type 2 pick (`BN6_EMBLEM_COMMIT`) | answer: UNITE's, the proxy form with the Soul's turns and its used bit, on past Beast Out's (`0x08029154`) | once OK on UNITE |
| `0x08008472`, `0x0800847C` | the turn's change, BN6's countdown calls (`BN6_TURN_COUNT`, `BN6_TURN_COUNT_1`) | answer: a Soul's turns one less, as BN5 counts them | once a Custom screen, per side |
| `0x0801488A`, `0x080148B2` | the turn's form check, Beast Out's end asked (`BN6_TURN_END`, `BN6_TURN_END_1`) | answer: a Soul out of turns ends, through BN6's request to leave the form (`0x08014890`, `0x080148B8`) | twice a turn, per side |
| `0x0801B034` | the weakness break (`BN6_FORM_BREAK`) | answer: none for a Soul (on at `0x0801B052`) | once a weakness hit |
| `EMU_FREE` + `0x480000`, + `0x480004` | the proxy's overlay entries 8, entered and left | answer: the Soul committed taken up, or cleared, then on to HeatCross's own; a double of MegaMan set up while a Soul is his (HolyDrem's, Poltrgst's) on to the Soul's own entry, as BN5's `0x0800F01C` | once a form entered or left, a double made |

A Soul's look, abilities and passives (the track's S3-S5, `src/emu/soullook.c`, `soulpower.c`, `soulpassive.c` and `soulcode.c`; docs/ROM_DATA.md, A Soul's look, abilities and passives) add these, each answering only for MegaMan in the proxy form with a Soul (BN6's own way otherwise):

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x0800FCA4` | the sprite selector's read by the form (`BN6_FORM_SPRITE_READ`) | answer: the Soul's sprite in the grown list 0 | as his sprite is set |
| `0x08010066`, `0x080BF518` | his palette's reads by the form (`BN6_FORM_PALETTE_READ`, `BN6_FORM_PALETTE_READ2`) | answer: palette 0, 2 in Chaos Unison | as his palette is set |
| `0x08014A82` | the crossing's phases by the form (`BN6_CROSSING_PHASES`) | answer: BN6's phases with BN5's phase 1, the Soul's image (`EMU_FREE` + `0x481080`) | each frame of a unison |
| `0x08014C8E` | the crossing's phase 2, its sound (`BN6_CROSSING_SOUND`) | answer: none (BN5's phase 1 played it); TomahawkSoul's landing: the camera shaken, the battle's flag 0x20 | once a unison |
| `0x080C1D40`, `0x080C1D88`, `0x080C1E66` | the image's sprite, its decoration and its end (`BN6_SOUL_IMAGE_SPRITE`, `_DECO`, `_UNDECO`) | answer: the Soul's Navi's battle sprite in list 2; no decoration of BN6's | once a unison |
| `0x0801E6C2`, `0x0801E666` | the emotion window's and the Custom screen's face by the form (`BN6_FACE_OF_FORM`, `BN6_FACE_OF_FORM2`) | answer: the Soul's face and its turns | as the face is drawn |
| `0x0801D814`, `0x0801CBFE` | whether the window counts, its digits (`BN6_FACE_COUNTER`, `BN6_FACE_DIGITS`) | answer: always for a Soul, in BN5's digits | as the face is drawn |
| `0x080EC954` | MegaMan's arm flash routine before its call of BN6's flash spawner (`BN6_ARM_FLASH_SPAWN`) | jump: a Soul's kind (BN5's 0x23, 0x33) to BN5's spawner, moved, on as the call returns: BN5's own flash object with BN5's kinds | once a flash |
| `0x080BA5A2` | a Cross's flash raised in BN6's own flash's set-up (`BN6_FLASH_HEIGHT`) | answer: a Soul's at its kind's height, as BN5's | once a flash of BN6's |
| `0x0800FF3E`, `0x08010884`, `0x080108A8`, `0x0801268C` | his loadout's, element's, weakness's and buster bonus's reads by the form (`BN6_FORM_LOADOUT_READ`, `_ELEMENT_READ`, `_WEAKNESS_READ`, `_BONUS_READ`) | answer: the Soul's row in BN6's ids, its element, no weakness, no bonus | as a form is set up, a shot fired |
| `EMU_FREE` + `0x481000`-`0x48100C` | the moved code's stubs: BN5's pending Soul, a spawn left out, the form by object and by side | answer: from `BN6_SOULS` | as the moved code calls them |
| the moved code's reads of BN5's AI data's Soul fields (`soulcode_bn5.c`'s fields) | in BN5's code moved | answer: from `BN6_SOULS`, where BN6's AI data holds its own | as the moved code reads them |
| `EMU_FREE` + `0x481010`, + `0x481014` | the proxy's passive entries 8 (`BN6_FORM_PASSIVES`, `BN6_FORM_PASSIVES_COLL`) | answer: the Soul's, BN5's routine moved, or none; HeatCross's without a Soul | as a form is set up, its state set again |
| `0x0802A494` | the Custom screen's hand size set (`BN6_HAND_SIZE_SET`) | answer: NumberSoul's ten | once a screen |
| `0x08013236` | the chip charge's test (`BN6_CHARGE_TEST`) | jump: a chip of the Soul's kind charges by BN5's rule (ShadowSoul's swords, KnightSoul's Break chips, ToadSoul's Aqua chips), else none; the answer one of the two `movs` and `bx lr` at `SOULS_THUMB` | as A is held with a chip |
| `0x08012C7C` | a charged chip's bonus (`BN6_CHARGE_BONUS`) | answer: KnightSoul's and ToadSoul's double (0xFF), none for the rest | once a chip used |
| `0x0800FBA4` | the release of a charged A, its power attack (`BN6_CHARGED_RELEASE`) | jump: a Soul's empty power attack to the chip used charged (`0x0800FBEC`) | once a release |
| `0x0801285E` | the chip's damage as its modifier leaves it (`BN6_CHIP_MODIFIED`) | jump: TomahawkSoul's Wood attack chip on grass doubled, to the Thumb that makes the panel plain and plays the modifier's sound | once a chip used |
| `0x080128E8` | the chip use's end (`BN6_CHIP_USED_END`) | answer: BN5's DarkChip use (dark for the battle), BN5's dark gate (a chip refused made the chip of nothing), KnightSoul's 50 frames unharmed from his front column | once a chip used |
| `0x080128EE` | the chip's family read (`BN6_CHIP_FAMILY_READ`) | jump: ShadowSoul's charged sword to BN5's sword family, moved | once a chip used |
| `0x08010340` | the shadow's type by side (`BN6_SHADOW_TYPE_SIDE`) | jump: none for ShadowSoul (BN5's `0x0800E0B6`) | as his shadow is set |

ColonelSoul's button, chip shot and obstacle strike (the track's S5, `src/emu/soulcolonel.c`; docs/ROM_DATA.md, ColonelSoul's button, chip shot and obstacle strike) add these, each answering only where MegaMan is united with ColonelSoul (BN6's own way, DustCross's, otherwise):

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x08027F10` | DustCross's elements 8 and 9 as a Custom screen opens (`BN6_DUST_ELEMENTS`) | answer: BN5's ColonelSoul's set-up (`0x08023D70`), the button grey | once a screen |
| `0x08028F84` | DustCross's lit rule (`BN6_DUST_LIT`) | answer: BN5's (`0x08024C04`), its chip, slot and place kept | after each pick |
| `0x08027410` | mode 0x38's phases' table (`BN6_DUST_PHASES`) | jump: BN5's two phases, moved | A on the button |
| `0x08028342`, `0x08028782`, `0x0802878C`, `0x080287DE` | the button's tiles and the chip's window's picture and palettes (`BN6_DUST_ART`, `BN6_DUST_INFO_ART`, `BN6_DUST_INFO_PAL`, `BN6_DUST_NAME_PAL`) | jump: BN5's art, copied (`SOULS_COLONEL_ART`) | as each is drawn |
| `0x08026CFA`, `0x08027058` | mode 4's last draw, mode 0x24's end (`BN6_MODE4_LAST_DRAW`, `BN6_MODE24_END`) | jump: BN5's draw of the loaded chip, moved, then on | each frame the button is chosen |
| `0x08029032` | B on the chips (`BN6_CUSTOM_B`) | jump: the button's chip given back first (BN5's `0x08024CEA`), then BN6's end | B with the button the last thing done |
| `0x08029222`, `0x08029458` | OK's end, its picks spent (`BN6_OK_END`, `BN6_OK_SPENT`) | answer: the button's chip committed (the block's +0x14) and its deck entry spent | once OK |
| `0x0801486C` | the turn's form check (`BN6_TURN_FORMS`) | answer: his chip his (the block's +4) and his charged shot the chip shot, ScreenDivide without | as a turn begins |
| `0x0801B3CA`, `0x0801B506`, `0x0801B646`, `0x0801B786` | the obstacles' states at their collision flags' read (`BN6_OBSTACLE_FLAGS_A`-`_D`) | jump: BN5's block (`SOUL_COLONEL_STRIKE`): an obstacle the strike finds turns into BN5's soldier (`xstrike.c`, with the core where BN5's Navis are: either Colonel's side) | each frame of an obstacle while a side is armed |
| `EMU_FREE` + `0x481018`, + `0x48101C` | the moved code's stubs: his side's strike set, BN5's test of an obstacle (`SOUL_STUB_STRIKE`, `SOUL_STUB_STRIKE_CHECK`; `xstrike.c`) | answer: his side's bit in `BN6_COLONEL_SIDES`, where a Navi Colonel's spawn sets his; BN5's searches on BN6's panels | as he unites; as the strike's block runs |

BN5's mood in BN6's battles (the track's S7, `src/emu/soulmood.c`; docs/ROM_DATA.md, BN5's mood in BN6's battles) adds these where the Souls are installed, each keeping BN5's mood in the Souls' block beside BN6's own:

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x0801A24C` | a hit's mood step, past BN6's own Full Synchro (`BN6_MOOD_HIT`) | answer: the attacker's mood up by the hit's gain (no counter), a counter's Full Synchro, the struck's down by its loss | once a hit |
| `0x08015BEC`, `0x080143A6` | BN6's mood set and calm (`BN6_MOOD_SET`, `BN6_MOOD_CALM`) | answer: BN5's twin writers' callers mirrored (the chip's Full Synchro and Anger, Anger's start and end, a Custom screen's script) | as BN6 writes its own |
| `0x08015C0E`, `0x080143B0` | their writes (`BN6_MOOD_SET_WRITE`, `BN6_MOOD_CALM_WRITE`) | jump: none while a BN5 DarkChip keeps him dark (past to `0x08015C10`, `0x080143B2`) | as BN6 writes its own |
| `0x0802945A` | the deck's compaction as a Custom screen opens (`BN6_COMPACT`) | answer: the battle's opening mood at its first screen; where the deck holds a BN5 DarkChip, BN5's worried rule (the first DarkChip into the deck's place at the hand's count, deck[5] on the first screen) and the run's shelf of BN5's DarkChips, the compaction ours | once a screen |

Chaos Unison (the track's S6, `src/emu/soulchaos.c`; docs/ROM_DATA.md, Chaos Unison) adds these where the Souls' own code is placed, each answering only for a side united in a Chaos Unison (`BN6_SOUL_CHAOS`) or while its release's request stands (`BN6_SOUL_REQUEST`):

| Address | Routine | Kind | Runs |
| --- | --- | --- | --- |
| `0x08012F62` | a charge's time by its button (`BN6_CHAOS_CHARGE`) | answer: a chaos B's, his chaos power's row of the charge times by the level (BN5's `0x08010654`) | as a charge starts |
| `0x08012E8A`, `0x08012E8E` | the charge's state's step and past it (`BN6_CHAOS_DUE`, `BN6_CHAOS_PHASE`) | answer: the phases' cycle of a full chaos B stepped (BN5's `0x080105F0`), `BN6_SOUL_CYCLE` | each frame of a charge |
| `0x080131D4` | the release's request (`BN6_CHAOS_RELEASE`) | answer: 0x8000 in phase 2, else 0x10000 (BN5's), the request the Souls' | once a release |
| `0x080F180E` | the player's requests read (`BN6_CHAOS_CONSUME`) | jump: the chaos attack (the level one up, the power attack by his AIData +0x11) or the failure (family 0xC7), Chaos Unison's Thumb | once a release |
| `0x080F1870` | the player's idle state past the chip request's test (`BN6_PLAYER_IDLE_FREE`, `darkinvs.c`, where BN5's ROM is read) | jump: BN5's chip AI for the player, through `FREE_CHIP_AI`, where his AI data's +0x48 bit 0x200 stands (BN5's rule; nothing of BN6's sets it) | a frame he idles |
| `0x080172F6` | the player's set-up past his AI data's lay-out (`BN6_PLAYER_INIT_AI`, `darkinvs.c`) | answer: side 0's player's AI data moved into `RAM_PLAYER_AI` (BN5's 0x140 bytes); a NetBattle's other MegaMen move into rooms of their own as the chip AI first takes them (issue #182) | once a battle |
| `0x0801AD10` | the player's frame's end (`BN6_PLAYER_FRAME_END`) | answer: BN5's time under the chip AI counted down (AIData +0x36, BN5's `0x0800DBD8`), at 0 his request 0x80000 | a frame he runs |
| `0x080144A2` | BN6's drain while AIData +0x36 is not 0 (`BN6_AI_DRAIN_TEST`) | jump: none while the chip AI has him (to `0x080144B2`) | a frame |
| `0x080ED12A` | the support family past the chip's routine (`BN6_SUPPORT_MARK`) | jump: DarkInvs's (subfamily 0x65) attack ended in this frame, BN5's (to `0x080ED130`) | a support chip used |
| `0x080EBA9C` | the attack's requests' answer to 0x80000 (`BN6_REACTION_START`) | jump: BN5's family 0x49 (BN6's 0x99) where the chip AI has him, on at the bl `0x080EBA9E` | DarkInvs's end |
| the Navis' copy of BN5's `0x080EBBFE` | its buster arm's read of his Chaos Unison (`BN5_ARM_CHAOS_READ`) | jump: the Souls' block's `BN6_SOUL_CHAOS` | a buster shot of the machine's |
| `XC_STUBS` + 0x24, + 0x28, + 0x2C | DarkInvs's calls BN6 has no twin of (`XC_STUB_DARKINVS`) | answer: the chip AI's state cleared, its entry (time, flag, list), his statuses cleared at BN6's places and BN5's calm of his mood | DarkInvs used |
| `0x08011744` | the attack's end, its requests cleared (`BN6_CHAOS_END`) | answer: BN5's 0x18000 cleared too; the failure's kind his chaos ended | once an attack ends |
| `0x080EBA84`, `0x080ED7B0`, `0x080ED90E`, `0x080EEE32` | BN6's tests of the counter requests 0x8600 (`BN6_COUNTER_TEST`, `_1D`, `_1E`, `_37`) | answer: 0x8000 left out while the Souls' request stands (BN5 tests 0x600) | each frame |
| `0x080E21E4`, `0x080E22A8` | the charge glow's and its sounds' side tests (`BN6_GLOW_SHOW`, `BN6_GLOW_SOUND`) | jump: in chaos, BN5's test by the side | each frame of a glow |
| `0x080E2272`, `0x080E2280`, `0x080E2208` | the glow's sprite past A's, its load, its animation (`BN6_GLOW_PICK`, `BN6_GLOW_LOAD`, `BN6_GLOW_ANIM`) | answer: BN5's dark glow (the Souls' copy in list 0), its animation the phase + 2 once full | as the glow is set and each frame |
| `EMU_FREE` + `0x4833EC` | the failure's stub (`CHAOS_OUT_STUB`, a `bx lr` in Chaos Unison's Thumb) | answer: a Dark Soul already out ended, his side's chip lists where the battle gave none | once a failure |

BN5's green flash (the chips' code, `src/emu/xchips_flash.c`; docs/ROM_DATA.md, BN5's DarkPlus and its green flash) adds two hooks on the moved DarkPlus's stores of it (in its copy, under the chips' claims) and two of BN6's: the player's timers past the time stop's test (`0x080107DC`, `BN6_FLASH_COUNT`: the flash's frames one less) and an object's colours' chain before `0x080168C8` (`0x0801B15A`, `BN6_FLASH_TINT`: the colour shader by BN5's table while it lasts), each frame of a battle where BN5's chips' code is installed.

The duel's two are set as the duel begins and taken off as it ends, so
the other battles carry no hook a frame per object. The hooks replaced
reading the game every frame: the battle's objects for MegaMan's HP and
ProtoMan's, the battle state for its record, every 16 frames the
enemies' reward rows, the flags of the layer's choices, the key items
for a ScrtData or the Spin picked up, and the exit pad's warp-off flag,
written every frame because EnterMap clears the map's flags (`0x1640`-
`0x16FF`): now written as a map is entered and as the guardian's exit
opens.

What stays read a frame at a time, where a hook would only move the read:
whether MegaMan walks the map, battles or reads a menu (`on_map`, the
game mode, GAME OVER among them); the exit pad's warp under way and its
arrival; the frames on a map not the layer's before he is warped back;
the duel's DeleteTime, which its HUD shows as it runs.

Where a table written into the core's copy is simpler than a hook, the
table stays (issue #32): a layer's shop stock goes to the ROM's initial
shop table beside RAM, as the shop screen lists only entries found there
(one write per entry, made again with the layer after a CONTINUE); a
Chip Trader's map takes the first entry of the prize pools and of the
trader kinds (two writes), and its pool's records are rewritten in the
folder's codes either way; the BugFrag Trader's trade waits for its chat
to hold, a few reads while a chat is open, and runs as game calls.

`tests/test_emu.c` runs a ROM of the test's own bytes on mGBA (its
`GBAIsROM` wants `0xEA` at offset 3 and `0x96` at `0xB2`, nothing more):
routines called and their results stored, with a hook of every kind on
them, an ARM-state one, an event posted from an answer hook, the board's
`BKPT #1` passed on, the hooks taken off again, one in RAM written again
after a reset, and a halt from a hook. Then two cores side by side, run in
turn as BN6's and the guest's are (issue #61): BN6's made by `emu_init`
with a hook on it, the other as `src/emu/guest.c` makes one, from a copy
of the ROM that also plays a tone. BN6's hook runs in BN6's frames alone;
the guest's copy of the code stays unpatched, and a `BKPT #0xCE` written
into it goes to its own board, which skips it, never to BN6's hooks (the
hook table is BN6's core's: the guest has none); each one's RAM is its
own; the guest's tone reaches the ring BN6's silence fills, and a frame of
it let go (`emu_audio_drop_from`) does not; its state saves and loads with
BN6's RAM untouched. Then a block of Thumb code of the test's own, made as
if it stood in another game, is moved into BN6's copy with `emu_write`
(`src/emu/thumb_reloc.c`, issue #82): its call within itself left alone,
its two calls out aimed at stubs of the ROM's own 11 MiB away, beyond a
BL's 4 MiB, through 12-byte veneers (`push {r0,r1}; ldr r0,[pc,#4];
str r0,[sp,#4]; pop {r0,pc}` and the target). Each stub, ARM code, keeps
what it met: r0-r12, sp and the flags as the block set them before its
BL, and lr its way back into the block; and the copy's writes outlast the
core's reset that starts it. Then the second-level dispatch's code
(`xvirus_dispatch_code`) run on the core with a driver of the test's own
that stands in for BN6's spawner, its free and its runner: a moved type
spawned as its kind's dispatch type, its number in the side table by the
object's slot and its MOVED routine run with r1 the number times four; a
twin spawned as BN6's own number (its number in the side table too,
where nothing reads it); a type no family named let go as it first runs.
Then RAM kept patched (`emu_keep`): the patch made where the game's own
bytes are, a state saved with the game's own there (another core loading
it reads them), the patch made again as the state is loaded, and bytes
that read neither left alone. Last, the installers' claims through
`emu.c`'s writes and hooks (`src/emu/claims.c`): an installer's bytes its
own, another's write over them told, a write outside any installer
nobody's, and a hook set on an installer's bytes told. Then the
installs' record (`src/emu/xinstall_log.c`): its bytes in another core's
ROM copy (the ROM's alone, the mirror's at its byte, RAM's and those
outside the record left out), its hooks on that core's own table, each
reading the core it runs on, its hash the same once it ends. `build.py
test` runs it with the address and undefined-behaviour sanitizers, as CI
does.

## Testing

`CYBERWORLD_AUTOPILOT=1` walks MegaMan to each exit (into a guardian's arena
and to its Guardian Data first; by the layer's heal when he is hurt, once a
layer, then below half his HP or with the guardian next) and fights its
battles from their state (docs/ROM_DATA.md, the autopilot's fight): on the
Custom screen the chips that go together worth the most, each used from a
panel where it reaches an enemy, off poison and away from the attacks it
can read (a lit panel, a shot coming down the row, a mine's landing panel),
the buster charged in between. Over seeds 1-12 and 30000 frames it won 43
of 44 virus battles and reached act 1's guardian in 11 runs, but beat him
once: 400-600 HP against MegaMan's 100. `CYBERWORLD_AUTOPILOT=weak` keeps
the first autopilot's blind button rhythm and walk, which the docs'
pictures are timed by, and keeps enemies at 1 HP, so what follows a won
guardian battle can be tested. The autopilot never picks a DarkChip of
either net and never presses A on UNITE or the Beast Out emblem
(`custom_guard`). A step it could take that leaves MegaMan where he stood
for half a second marks that panel blocked for the battle (a rock of
BN5's fields: in BN6's engine since the last round, an autopilot that
held UP into one stood 13000 frames, `--net-biome x4 --seed 12`), and a
step into the row it is after, blocked in his column, goes aside first.
Before issue #82's last round it fought the guest
core's BN5 battles too, from BN5's memory (99 of 100 won over seeds 1-12
in three of BN5's areas); BN5's areas and guardians are BN6's battles
now.
`tools/device_run.py` runs a build on the device from `/tmp`. `CYBERWORLD_EMU_DEBUG=1` prints the depth, game mode, position,
map and the hooks' hits so far every 30 frames, prints the generated walls, and writes the tile map to
`.build/gen_tilemap.bin`.
`--net-biome N` puts every layer in one area, `--net-layout N` builds every
layer in one layout (`LAYOUT_*` in `src/net/net_layouts.h`).
`CYBERWORLD_BTRACE=1` writes each frame of a battle's memory, EWRAM and
IWRAM, into the data folder's `btrace.bin`, as what changed since the
frame before (`src/emu/battle_trace.c`, its format in its header), for
`tools/xcontrol.py`: the control family's differential (issue #82; docs/
MULTIROM.md, Transplanted families). Each of its two specs runs three
headless battles, each from the same boot, layer and keys and each against
one Mettaur at the same panel, started by the foes step (`0:foes 01@52`):
BN6's own, BN6's own with the control installed (`--dev xcontrol`), and
the control's moved Mettaur (`0:foes 200@52`); it compares them frame by
frame, every byte, and each byte that differs must have its reason (a
pointer into what was moved, read through the installer's map; the enemy
id and AI index where the battle keeps them; the text archive the name
plate reads its name from, BN6's viruses' for 0x01 and the families' for
0x200; the delete count the
installer's hook keeps AI 32 out of; the interrupt stack below its
`sp_irq`, `0x03007F60`, dead between frames, where an interrupt's entry
lands a few cycles apart as the moved code runs; the main stack below the
main loop's pointer as it waits for the next frame, `0x03007DFC` (read
by a hook at its DISPSTAT loop), also dead between frames, where the
VBlank handler's nested frames save what they interrupted (the BIOS's
decompression of the battle's sprites) a few cycles apart where the
lava's and the sea's install has the panels' flags read from ROM; and,
with BN5's ROM read, the code in IWRAM that install keeps patched,
`0x0300791C`-`0x0300791F`, `0x03007742`-`0x03007771` and
`0x03007678`-`0x0300768D`, and the sea's submerge's, `0x0300724A`-`0x03007257`
and `0x0300728C`-`0x0300729B`). The values of the enemy
id and AI index are allowed wherever they stand, not at fixed addresses;
the tool prints each address they explained (the battle's three copies of
the id, each a u16: `0x020348D4`, `0x0203AAB0`, `0x0203EB56`; the name
plate's archive at `0x0200A260`, the AI index at `0x02034181`, and the index times 4 that the
dispatcher leaves in r4: in the deferred-call queue at `0x0200A4A4`, and
in the parameters of an ice block it spawns, `0x02036874`). The `fight`
spec: seed 1, MegaMan at 1000 HP (`--dev hp=1000`): 3899 frames of battle
each, the native and installed runs the same at every byte (BN6's tables
grown for the families and the control installed, BN6's battle reads them
as its own: not one byte differs), and the moved
Mettaur the same as BN6's to the results (2903 frames: it changes rows 6
times and attacks 21 times, its shockwaves taking 110 of MegaMan's HP, and
takes 12 shots and 3 charged ones to its deletion), where the reward comes
from each id's own drop row (`0x4064` both: the control's rows are
Mettaur's, copied to its ids). One immediate changed
in the copy (the counter time at `0x0810B142`, `0x1E` to `0x1F`) shows at
frame 365. The fight never leaves the Mettaur's own states (8-12), so the
`status` spec gives it, by the status step (`0:status freeze 90`,
docs/DEVTOOLS.md), each status a hit leaves for the engine to act on, at
the same frames in all three runs: frozen 90 frames (shot at twice),
paralysed and bubbled the same, dragged, pushed along, frozen 200 frames
left to thaw, then the fight's keys: 6299 frames each, native and
installed the same at every byte, and the moved Mettaur the
same as BN6's to the results (3565 frames), through BN6's generic states 4
(paralysis), 5 (drag, its 25 phases), 6 (frozen, twice) and 7 (bubble),
their hooks of tables 6, 7, 8 and 10 called through the grown tables, and
both ice blocks of size 0. Before the ice blocks' sizes were grown the
moved Mettaur's block was size 1 (it read past BN6's 30 viruses into the
Navis') and the run differed at the first freeze, frame 342; with the
control's recipe set to `.ice = 1` it differs there the same way. Not
reached: CurAction 3 (no way in for a virus, docs/ROM_DATA.md) and its
table-9 hook, a hit's own way to the statuses (the step sets what a hit
leaves; the resolution before it is BN6's own code, which reads a family
only through the tables the installer grows), and the rest of a battle's
statuses that are no state (confusion, blindness, holds).
`tools/xfamily.py` checks BN5's CanGard, the first family of another
game's (docs/MULTIROM.md, Transplanted families), in BN6's own battles:
headless runs with both ROMs (copied into `.build/xfamily/rom`), seed 15
(whose first area is BN6's own, not dressed as BN5's), each battle started
by the foes step and traced. `fight`: CanGard V1 at (5,2) beside a Mettaur
at (4,3), MegaMan idle at (2,2): its states 8, 9 and 10 alone after its
spawn, its reticle (BN5's type `0x25` through the second-level dispatch: the
third kind's `0xD2`, `0x25` in its side table) spawned on its panel and walking a panel
at a time to MegaMan's, then spawned on his, striking him for 10 each time
he was not still flashing from a hit (4 times in 1500 frames; Mettaur's
shockwaves the rest), its muzzle's blast (effect `0x6C`) drawn as it
fires, its MaxHP 60, element 0 and a shadow. `status`: freeze, bubble,
paralysis and drag, each by the status step while its reticle is out
(battle frames 43, 140, 237 and 334): each the frame after in BN6's
state for it (6, 7, 4, 5), its AttackVars +0x30 cleared and the reticle
gone two frames on (the release hook in tables 7, 8, 6 and 10), back in
its own states when the status ends and firing again; its ice block size
0; the battle on to the end. `delete`: each version alone, `--dev
onehit`, MegaMan shooting, event flag 0x1D8 set before the battle (the
gate of BN6's rare path, which versions 4 and 5 take): each spawned with
its struct2 row's MaxHP, deleted, its reticle gone, the results reached,
BN6's delete counts (`0x02000070`-`0x0200009F`, versions 0-3's path) and
the rare path's byte (`0x0203EAEA`, the chat of new virus data's index)
unchanged. Then the control, BN6's own Mettaur deleted the same way:
version 0 (id `0x01`) raises `0x02000071`, version 4 (id `0x05`) puts 1
at `0x0203EAEA`. With the installer's hooks on the delete count left out,
versions 0-3 raise `0x02000090` and versions 4 and 5 put 32 at
`0x0203EAEA`, and the check fails for each. `native`: BN6's Mettaur fought
by `xcontrol.py`'s fight keys with CanGard installed and with `--dev
nofamilies`: 3899 frames the same at every byte. Beside it, the autopilot (`CYBERWORLD_AUTOPILOT=1`) with both
ROMs, CanGard installed and not: seed 1's 15000 frames the same run log
and last frame, seed 15's 40000 (`--dev god`, two of BN6's battles won)
the same run log and its checkpoint's `run.state` the same byte for byte,
so a state saved on a map carries nothing of the install (again with
BN6's tables grown for the families, against `--dev nofamilies`: run
log, `run.state`, `run.sav` and `battle.sav` the same). Not checked there: a real
chip's hit on it, its guard against each kind of chip.
`tools/xdiff.py` (`build.py xdiff`; docs/DEVTOOLS.md, The differential)
checks CanGard against BN5's own CanGard, frame by frame: one battle fought
in BN5's engine on the guest core (`--xdiff`'s reference, BN5's battle as
BN5 has it) and in BN6's with the family installed, aligned by each
battle's timer, compared by role through each game's numbers and layouts,
each difference either an engine difference explained with both games'
routines in `tests/xdiff/legit.json` (never on the family's own
records, where the engine's numbers are read as BN6's) or a failure,
MegaMan's own state stopping a spec as
inconclusive. Its specs and what they showed are in docs/MULTIROM.md,
Transplanted families; `--mutations` breaks the install eleven ways a
transplant bug would and each break fails its spec.
