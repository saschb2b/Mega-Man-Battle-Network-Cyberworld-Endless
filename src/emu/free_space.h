/* The free space past the game's 8 MB in the core's ROM copy, and the RAM
 * BN6 leaves alone: one plan every part of the engine follows. WRITTEN BY
 * tools/free_space.py FROM src/emu/free_space.plan: edit the plan, run the
 * tool, and build.py lint keeps the two in step (docs/engine/emulation/
 * free-space.md: the zones, their headroom and how a region is placed).
 * Each region is its writer's, [its start, its end), inside its zone; a span
 * no region names is free. tests/test_core.c checks the regions in order
 * and apart from FREE_SPACE_ROM and FREE_SPACE_RAM, emu.c's writes into the
 * ROM copy's free space are told where they run past a region (free_space.c),
 * and what the installers write as the core starts is claimed byte by byte
 * as they write it (claims.h). emu.h includes this: the places are bus
 * addresses of the core's copy. */
#ifndef CW_FREE_SPACE_H
#define CW_FREE_SPACE_H

#define EMU_FREE             0x08800000u             /* first free bus address past an 8 MB game */
#define FREE_END             (EMU_FREE + 0x800000u)  /* the copy's end: 16 MB (EMU_ROM_SIZE) */

/* ---- engine: +0x000000-+0x002F00, the engine's own records, hooks and stubs, and BN6's small tables grown ---- */
#define FREE_WARP            (EMU_FREE + 0x000000u)  /* a warp record and a one-entry warp list (gamecall.c) */
#define FREE_CALL_RETURN     (EMU_FREE + 0x000100u)  /* where a game call's routine returns: a hook (gamecall.c) */
#define FREE_NO_MYSTERY      (EMU_FREE + 0x000180u)  /* an empty Mystery Data list (mapslot.c) */
#define FREE_RECORDS         (EMU_FREE + 0x000200u)  /* two battle records in turn, 0x80 apart (encounter.c) */
#define FREE_RECORDS_END     (EMU_FREE + 0x000300u)
#define FREE_RUSH            (EMU_FREE + 0x000400u)  /* Rush's gaps (rush.c) */
#define FREE_RUSH_END        (EMU_FREE + 0x000640u)
#define FREE_BLOCKERS        (EMU_FREE + 0x000700u)  /* map-object handler 3's records (blockers.c) */
#define FREE_BLOCKERS_END    (EMU_FREE + 0x0012E0u)
#define FREE_CHIP_PAS        (EMU_FREE + 0x001300u)  /* BN6's Program Advances' list grown with BN5's: their recipes, then the list (xchips_code.c) */
#define FREE_CHIP_PAS_END    (EMU_FREE + 0x001700u)
#define FREE_CHIP_AI         (EMU_FREE + 0x001700u)  /* the player's chip AI's stub (darkinvs.c) */
#define FREE_CHIP_AI_END     (EMU_FREE + 0x001740u)
#define FREE_SWAP_RETURN     (EMU_FREE + 0x001740u)  /* where the swap's spawn returns mid-battle: a hook (battle_swap.c) */
#define FREE_SWAP_RETURN_END (EMU_FREE + 0x001760u)
#define FREE_NCP             (EMU_FREE + 0x001800u)  /* BN6's NaviCust programs' tables grown for program 48, BN5's SoulT+1: its routine and shapes, the list's stub, the effects, codes and records (xncp.c) */
#define FREE_NCP_END         (EMU_FREE + 0x002800u)

/* ---- layers: +0x002F00-+0x090000, layers and the town: their maps, rewritten as a layer or the town is entered; home's other places ---- */
#define FREE_WARP_LIST       (EMU_FREE + 0x002F00u)  /* the layer map's warp list (mapslot.c) */
#define FREE_LAYER           (EMU_FREE + 0x003000u)  /* layer data in two halves (mapslot.c) */
#define FREE_TILE_MAP        (EMU_FREE + 0x010000u)  /* the generated tile map (netmap.c) */
#define FREE_TILE_MAP_END    (EMU_FREE + 0x028000u)
#define FREE_COORDS          (EMU_FREE + 0x028000u)  /* the generated coordinate data (coords.c) */
#define FREE_COORDS_END      (EMU_FREE + 0x030000u)
#define FREE_TOWN_TILE_MAP   (EMU_FREE + 0x030000u)  /* the town's tile map (town.c) */
#define FREE_TOWN_TILE_MAP_END (EMU_FREE + 0x048000u)
#define FREE_TOWN_COORDS     (EMU_FREE + 0x048000u)  /* the town's coordinate data (coords.c) */
#define FREE_TOWN_COORDS_END (EMU_FREE + 0x058000u)
#define FREE_TOWN            (EMU_FREE + 0x058000u)  /* the town's scripts, text and tables (mapslot.c) */
#define FREE_LAN_HP          (EMU_FREE + 0x060000u)  /* Lan's HP's, then Lan's house's in its second half (mapslot.c, lanhp.c, lan_house.c) */
#define FREE_INDOORS         (EMU_FREE + 0x068000u)  /* home's other places (mapslot.c, indoors.c, aster_land.c, ...) */

/* ---- code-low: +0x090000-+0x150000, other games' code, art and per-battle rooms that the code zone has no room for: fixed places, the same at every boot with the same ROMs ---- */
#define FREE_TEAM_NAVIS      (EMU_FREE + 0x090000u)  /* BN5's Team ProtoMan in BN6's engine (xnavi_recipe.h XN_TEAM_*, seats 11-16): homes, code, art and sprites in a room of their own (xnavi_install.c, xnavi_sprites.c) */
#define FREE_TEAM_HOMES      (EMU_FREE + 0x090000u)  /* their homes, XV_HOME_SIZE each (xnavi_install.c) */
#define FREE_TEAM_NAVI_CODE  (EMU_FREE + 0x093000u)  /* their code, a room of XN_TEAM_SLOT_CODE each (the largest walk, ProtoMan's, 8648 bytes) (xnavi_install.c) */
#define FREE_TEAM_NAVI_ART   (EMU_FREE + 0x0A2000u)  /* what their attacks draw, a room each (xnavi_install.c) */
#define FREE_TEAM_SHARED     (EMU_FREE + 0x0A3800u)  /* the Navis' shared art's second room (a sprite of any Navi's once XN_SHARED_ART's is full) (xnavi_install.c) */
#define FREE_TEAM_SPRITES    (EMU_FREE + 0x0AB000u)  /* their battle sprites, a place each, room for all six (the largest, MagnetMan's, 49,776 bytes) (xnavi_sprites.c) */
#define FREE_TEAM_SPRITES_END (EMU_FREE + 0x0EF000u)
#define FREE_BN4_CHIPS       (EMU_FREE + 0x0EF000u)  /* the code of BN4's chips (xchips_bn4.h XC4_*): their units' homes and code, and the room their art is put into as an object loads a sprite (xchips_bn4_art.h) (xchips_code.c, xchips_bn4.c, xchips_bn4_art.c) */
#define FREE_BN4_CHIPS_END   (EMU_FREE + 0x13F000u)
#define FREE_SONG_TABLE      (EMU_FREE + 0x13F000u)  /* BN6's song table grown: its 474 entries, then SONG_GROWN_SLOTS more for other games' sounds (song_table.c) */
#define FREE_SONG_TABLE_END  (EMU_FREE + 0x141000u)
#define FREE_TEAM_SHARED_MORE (EMU_FREE + 0x141000u) /* the Navis' shared art's third room (a sprite of any Navi's once FREE_TEAM_SHARED is full: GyroMan's overlay first) (xnavi_install.c) */
#define FREE_TEAM_SHARED_MORE_END (EMU_FREE + 0x150000u)

/* ---- words: +0x150000-+0x170000, the director's and the PET's words: text archives, labels, lists ---- */
#define FREE_TALK            (EMU_FREE + 0x150000u)  /* the director's conversations (talk.c) */
#define FREE_LABEL           (EMU_FREE + 0x152000u)  /* the map-name label's archive (director_layer.c) */
#define FREE_PET_LABEL       (EMU_FREE + 0x153000u)  /* the PET's PLACE (director_layer.c) */
#define FREE_PET_TEXT        (EMU_FREE + 0x154000u)  /* the PET's words: the key items' names (pet_text.c) */
#define FREE_PET_DESCS       (EMU_FREE + 0x155000u)  /* the key items' descriptions (pet_text.c) */
#define FREE_PET_MAILS       (EMU_FREE + 0x158000u)  /* the mails' senders and subjects (pet_text.c) */
#define FREE_PET_BODIES      (EMU_FREE + 0x15A000u)  /* the mails' bodies (pet_text.c) */
#define FREE_PORTRAITS       (EMU_FREE + 0x166000u)  /* portraits of Falzar's Navis (portrait.c) */
#define FREE_PORTRAITS_END   (EMU_FREE + 0x167900u)
#define FREE_TRADER_LIST     (EMU_FREE + 0x168000u)  /* a layer's Chip Trader's prizes of both nets (trader.c) */
#define FREE_TRADER_LIST_END (EMU_FREE + 0x169000u)
#define FREE_PET_OPTIONS     (EMU_FREE + 0x169000u)  /* the PET menu's labels with Comm's made Options (pet_options.c) */
#define FREE_PET_OPTIONS_END (EMU_FREE + 0x16A000u)
#define FREE_BBS             (EMU_FREE + 0x16A000u)  /* the visit's requests on BN6's Request BBS: their titles and posts, LZ77 (request_bbs.c) */
#define FREE_BBS_END         (EMU_FREE + 0x16B000u)
#define FREE_NCP_TEXT        (EMU_FREE + 0x16B000u)  /* the NaviCust programs' names with program 48's, then their descriptions with its own, LZ77 (xncp.c) */
#define FREE_NCP_TEXT_END    (EMU_FREE + 0x16BC00u)

/* ---- world: +0x170000-+0x330000, other games' world, copied as their areas are entered: tiles, songs, overworld art, backgrounds ---- */
#define FREE_XAREA           (EMU_FREE + 0x170000u)  /* another game's net area (netmap.c) */
#define FREE_SONGS           (EMU_FREE + 0x1A0000u)  /* other games' songs (xsong.c) */
#define FREE_XNAVIS          (EMU_FREE + 0x260000u)  /* other games' Navis and map objects (xnavi.c) */
#define FREE_BACKDROPS       (EMU_FREE + 0x2A0000u)  /* other games' battle backgrounds (xbackdrop.c) */
#define FREE_FLAME           (EMU_FREE + 0x2E0000u)  /* BN6's own flame of darkness (xnavi.c) */
#define FREE_FLAME_END       (EMU_FREE + 0x2E1000u)
#define FREE_MAP_BACKDROPS   (EMU_FREE + 0x2F0000u)  /* other games' net maps' backdrops (xbackdrop.c) */
#define FREE_MAP_BACKDROPS_END (EMU_FREE + 0x320000u)

/* ---- code: +0x330000-+0x7A0000, other games' code homes and their per-battle rooms (issue #82): fixed places, the same at every boot with the same ROMs ---- */
#define FREE_VIRUS_ROOM      (EMU_FREE + 0x330000u)  /* BN6's tables grown for the families, the second-level dispatch (xvirus_grow.c) */
#define FREE_FAMILIES        (EMU_FREE + 0x348000u)  /* the families: their homes, code and art (xvirus.c, xvirus_families.c) */
#define FREE_CHIPS           (EMU_FREE + 0x400000u)  /* the chip registry: records, names, routines, art (chips.c, xchips_install.c) */
#define FREE_SOULS           (EMU_FREE + 0x480000u)  /* BN5's Souls in BN6's engine (soul_room.h lays it out) (soulproxy.c, soulunite.c, soullife.c, ...) */
#define FREE_PANELS          (EMU_FREE + 0x4C0000u)  /* BN5's lava and sea as BN6's panels (xpanels.c, xpanels_look.c) */
#define FREE_PANELS_END      (EMU_FREE + 0x4D0000u)
#define FREE_LATE_FAMILIES   (EMU_FREE + 0x4D0000u)  /* the late families' art (BN4's past its first eight): their sprites, and the pool's songs of their sounds (xvirus.c) */
#define FREE_LATE_FAMILIES_END (EMU_FREE + 0x500000u)
#define FREE_NAVIS           (EMU_FREE + 0x500000u)  /* BN5's Navis in BN6's engine (xnavi_sprites.c, xnavi_room.c, xnavi_install.c, ...) */
#define FREE_NAVI_SPRITES    (EMU_FREE + 0x500000u)  /* their battle sprites, a fixed place each, and the arm points grown (the Navi chips' and the Souls' one copy of each Navi's sprite there too) (xnavi_sprites.c) */
#define FREE_NAVI_SPRITES_END (EMU_FREE + 0x580000u)
#define FREE_NAVI_ROOM       (EMU_FREE + 0x580000u)  /* BN6's Navi tables grown for them (xnavi_room.c) */
#define FREE_NAVI_HOMES      (EMU_FREE + 0x588000u)  /* their homes: state table, records, veneers (xnavi_install.c) */
#define FREE_NAVI_CODE       (EMU_FREE + 0x590000u)  /* their code and their objects', block by block (xnavi_install.c) */
#define FREE_NAVI_ART        (EMU_FREE + 0x5B0000u)  /* what their attacks draw (xnavi_install.c) */
#define FREE_CHIP_CODE       (EMU_FREE + 0x600000u)  /* the code of BN5's chips (xattack_grow.c, xchips_code.c, xchips_bn5.c) */
#define FREE_ATTACK_FAMILIES (EMU_FREE + 0x600000u)  /* BN6's attack families grown to 240 (the chips' and the Souls' one copy) (xattack_grow.c) */
#define FREE_ATTACK_FAMILIES_END (EMU_FREE + 0x600400u)
#define FREE_CHIP_MOVED      (EMU_FREE + 0x600400u)  /* the chips' moved code, its tables and what it draws (xchips_code.c) */
#define FREE_SOULS_ART       (EMU_FREE + 0x700000u)  /* the Souls' art past their room: BN5's effects BN6 redrew, copied (soulcode.c, soulcolonel.c) */
#define FREE_SOULS_ART_END   (EMU_FREE + 0x710000u)
#define FREE_TAIL            (EMU_FREE + 0x710000u)  /* free to FREE_LATE_CODE (free) */
#define FREE_LATE_CODE       (EMU_FREE + 0x711000u)  /* the late families' code, block by block, past the families' own room (xvirus.c) */
#define FREE_LATE_CODE_END   (EMU_FREE + 0x720000u)
#define FREE_BN4_NAVIS       (EMU_FREE + 0x720000u)  /* BN4's Navis in BN6's engine (xnavi_recipe.h XN_BN4_*): their homes (xnavi_bn4.c) */
#define FREE_BN4_NAVI_CODE   (EMU_FREE + 0x730000u)  /* BN4's Navis' code and their objects', block by block (xnavi_bn4.c) */
#define FREE_BN4_NAVI_ART    (EMU_FREE + 0x750000u)  /* the coming battle's BN4 Navi's attacks' art and songs, one room (xnavi_bn4_art.c) */
#define FREE_BN4_NAVI_SPRITE (EMU_FREE + 0x77C000u)  /* the coming battle's BN4 Navi's battle sprite, one room (xnavi_bn4_room.c) */
#define FREE_BRACKET_FACES   (EMU_FREE + 0x78C000u)  /* BN6's and BN5's Navis' plates on BN4's bracket: its grown tables, two sheets (xbracket_faces.c) */
#define FREE_BRACKET         (EMU_FREE + 0x790000u)  /* BN4's bracket screen moved: its stubs, veneers, code and art (xbracket.c) */
#define FREE_BRACKET_END     (EMU_FREE + 0x7A0000u)

/* ---- art: +0x7A0000-+0x800000, art copied whole, a place each ---- */
#define FREE_LIB_TEAM        (EMU_FREE + 0x7A0000u)  /* a liberation's teammates: BN5's Navis' overworld sprites and mugshots, a place each (xnavi.c) */
#define FREE_LIB_TEAM_END    (EMU_FREE + 0x7FA000u)
#define FREE_NAVI_SHARED_FOUR (EMU_FREE + 0x7FA000u) /* the Navis' shared art's fourth room (a sprite of any Navi's once FREE_TEAM_SHARED_MORE is full: Meddy's last two first, CloudMan's before hers) (xnavi_install.c) */
#define FREE_NAVI_SHARED_FOUR_END (EMU_FREE + 0x800000u)

/* ---- persistent: 0x02010CE0-0x02011800, persistent: only the power-on clear writes 0x02010CDC-0x020117FF (the battle sprite pool from its end) ---- */
#define RAM_PACK_EXT         0x02010CE0u             /* the chip pack's extension: ids 411-511 in place, 512-602 in the entries of 320-410 (BN6_PACK_EXT, bn6_pack_entry) (xchips_install.c) */
#define RAM_PACK_EXT_END     0x020115E0u
#define RAM_PLAYER_AI        0x020115E0u             /* MegaMan's AI data under BN5's chip AI, BN5's 0x140 bytes, then his own's place (darkinvs.c) */
#define RAM_PLAYER_AI_END    0x02011730u
#define RAM_TEAM             0x02011730u             /* --dev arena2v2's list of what the Practice NetBattle spawns (epic #167) (arena_team.c) */
#define RAM_TEAM_END         0x02011750u
#define RAM_TEAM_LINK        0x02011750u             /* the partners' words for and from BN6's packets, 0x20 bytes a side (arena_team_link.c) */
#define RAM_TEAM_LINK_END    0x02011790u
#define RAM_TEAM_CUSTOM      0x02011790u             /* each partner's deck and own Custom screen, 0x38 bytes a side (arena_team_custom.c) */
#define RAM_TEAM_CUSTOM_END  0x02011800u
#define RAM_PERSISTENT_END   0x02011800u

/* ---- mode: 0x02029A00-0x0202DA00, a main mode's own while the map waits (BN6's shop, mode 0x2C, unpacks into 0x02029A00, 0x0202BA00 and 0x0202DA00 as it runs, and the map goes on after it): a moved screen's, run as a mode of its own from the map ---- */
#define RAM_BRACKET          0x02029A00u             /* BN4's bracket screen moved: its state, tournaments, blend values, menu sprites and its tiles unpacked (xbracket.h) (xbracket.c) */
#define RAM_BRACKET_END      0x0202DA00u

/* ---- battle: 0x0203FB50-0x02040000, per battle: the battle's setup clears 0x0203CDA8-0x0203FFFF, and nothing else of BN6's reads or writes 0x0203FB50 on ---- */
#define RAM_SEA              0x0203FB50u             /* BN5's sea's submerge (xsea.h: the submerged, their markers) (xsea.c) */
#define RAM_DARK_LISTS       0x0203FBE0u             /* the Dark Soul's chip AI's lists, 0xE0 a side (BN6_DARK_LISTS) (xnavi_dark.c, xnavi_bn5.c) */
#define RAM_NAVIS            0x0203FDA0u             /* the moved Navis' own state: Colonel's sides' bits (BN6_COLONEL_SIDES), the Dark Soul's AI data's holder (BN6_DARK_AI_OWNER, BN6_DARK_AI_POOL) (xnavi_room.c, xnavi_dark.c) */
#define RAM_DARK_AI          0x0203FDB0u             /* the Dark Soul's AI data, BN5's 0x140 bytes (BN6_DARK_AI) (xnavi_dark.c) */
#define RAM_BATTLE           0x0203FEF0u             /* kept for the moved families' and Navis' own state (xchips_flash.c) */
#define RAM_DISPATCH_SIDE    0x0203FF00u             /* the second-level dispatch's side tables (BN6_DISPATCH_SIDE) (xvirus_dispatch.c) */
#define RAM_COLONEL          0x0203FF60u             /* BN5's Colonel's table moved (BN6_COLONEL_TABLE) (xnavi_bn5.c) */
#define RAM_SOULS            0x0203FF70u             /* kept for the Souls' state and BN5's mood (the Souls' track) (soulunite.c, soullife.c, soulmood.c, ...) */
#define RAM_TEAM_TICK        0x0203FFF0u             /* u32 --dev arena2v2's count of the battle loop's iterations (arena_team.c) */
#define RAM_CHIP_AI_ROOMS    0x0203FFF4u             /* u32 the entries of BN6's AI data's pool BN5's chip AI took for its rooms (BN6_AI_POOL_USED's bits; darkinvs_room.h, issue #182) (darkinvs.c) */
#define RAM_TEAM_CLOCK       0x0203FFF8u             /* u16 two MegaMen a side's turn's clock: frames since the first OK of the turn this core has seen, then u8 1 once seen (team_hurry.h) (arena_team_custom.c) */
#define RAM_PA_WIDE          0x0203FFFCu             /* u32 the Program Advances past nine bits formed this battle, a bit an id from BN6_PA_WIDE_FIRST (side 0's: BN6's own bits end at id 383) (xchips_wide.c) */
#define RAM_BATTLE_END       0x02040000u

/* The plan as tables: X(name, start, end, parent, zone), in address order,
 * a part after its parent ("" none); X(key, start, end) the zones */
#define FREE_SPACE_ROM(X) \
	X("FREE_WARP", (EMU_FREE + 0x000000u), (EMU_FREE + 0x000100u), "", "engine") \
	X("FREE_CALL_RETURN", (EMU_FREE + 0x000100u), (EMU_FREE + 0x000180u), "", "engine") \
	X("FREE_NO_MYSTERY", (EMU_FREE + 0x000180u), (EMU_FREE + 0x000200u), "", "engine") \
	X("FREE_RECORDS", (EMU_FREE + 0x000200u), (EMU_FREE + 0x000300u), "", "engine") \
	X("FREE_RUSH", (EMU_FREE + 0x000400u), (EMU_FREE + 0x000640u), "", "engine") \
	X("FREE_BLOCKERS", (EMU_FREE + 0x000700u), (EMU_FREE + 0x0012E0u), "", "engine") \
	X("FREE_CHIP_PAS", (EMU_FREE + 0x001300u), (EMU_FREE + 0x001700u), "", "engine") \
	X("FREE_CHIP_AI", (EMU_FREE + 0x001700u), (EMU_FREE + 0x001740u), "", "engine") \
	X("FREE_SWAP_RETURN", (EMU_FREE + 0x001740u), (EMU_FREE + 0x001760u), "", "engine") \
	X("FREE_NCP", (EMU_FREE + 0x001800u), (EMU_FREE + 0x002800u), "", "engine") \
	X("FREE_WARP_LIST", (EMU_FREE + 0x002F00u), (EMU_FREE + 0x003000u), "", "layers") \
	X("FREE_LAYER", (EMU_FREE + 0x003000u), (EMU_FREE + 0x010000u), "", "layers") \
	X("FREE_TILE_MAP", (EMU_FREE + 0x010000u), (EMU_FREE + 0x028000u), "", "layers") \
	X("FREE_COORDS", (EMU_FREE + 0x028000u), (EMU_FREE + 0x030000u), "", "layers") \
	X("FREE_TOWN_TILE_MAP", (EMU_FREE + 0x030000u), (EMU_FREE + 0x048000u), "", "layers") \
	X("FREE_TOWN_COORDS", (EMU_FREE + 0x048000u), (EMU_FREE + 0x058000u), "", "layers") \
	X("FREE_TOWN", (EMU_FREE + 0x058000u), (EMU_FREE + 0x060000u), "", "layers") \
	X("FREE_LAN_HP", (EMU_FREE + 0x060000u), (EMU_FREE + 0x068000u), "", "layers") \
	X("FREE_INDOORS", (EMU_FREE + 0x068000u), (EMU_FREE + 0x078000u), "", "layers") \
	X("FREE_TEAM_NAVIS", (EMU_FREE + 0x090000u), (EMU_FREE + 0x0EF000u), "", "code-low") \
	X("FREE_TEAM_HOMES", (EMU_FREE + 0x090000u), (EMU_FREE + 0x093000u), "FREE_TEAM_NAVIS", "code-low") \
	X("FREE_TEAM_NAVI_CODE", (EMU_FREE + 0x093000u), (EMU_FREE + 0x0A2000u), "FREE_TEAM_NAVIS", "code-low") \
	X("FREE_TEAM_NAVI_ART", (EMU_FREE + 0x0A2000u), (EMU_FREE + 0x0A3800u), "FREE_TEAM_NAVIS", "code-low") \
	X("FREE_TEAM_SHARED", (EMU_FREE + 0x0A3800u), (EMU_FREE + 0x0AB000u), "FREE_TEAM_NAVIS", "code-low") \
	X("FREE_TEAM_SPRITES", (EMU_FREE + 0x0AB000u), (EMU_FREE + 0x0EF000u), "FREE_TEAM_NAVIS", "code-low") \
	X("FREE_BN4_CHIPS", (EMU_FREE + 0x0EF000u), (EMU_FREE + 0x13F000u), "", "code-low") \
	X("FREE_SONG_TABLE", (EMU_FREE + 0x13F000u), (EMU_FREE + 0x141000u), "", "code-low") \
	X("FREE_TEAM_SHARED_MORE", (EMU_FREE + 0x141000u), (EMU_FREE + 0x150000u), "", "code-low") \
	X("FREE_TALK", (EMU_FREE + 0x150000u), (EMU_FREE + 0x152000u), "", "words") \
	X("FREE_LABEL", (EMU_FREE + 0x152000u), (EMU_FREE + 0x153000u), "", "words") \
	X("FREE_PET_LABEL", (EMU_FREE + 0x153000u), (EMU_FREE + 0x154000u), "", "words") \
	X("FREE_PET_TEXT", (EMU_FREE + 0x154000u), (EMU_FREE + 0x155000u), "", "words") \
	X("FREE_PET_DESCS", (EMU_FREE + 0x155000u), (EMU_FREE + 0x158000u), "", "words") \
	X("FREE_PET_MAILS", (EMU_FREE + 0x158000u), (EMU_FREE + 0x15A000u), "", "words") \
	X("FREE_PET_BODIES", (EMU_FREE + 0x15A000u), (EMU_FREE + 0x166000u), "", "words") \
	X("FREE_PORTRAITS", (EMU_FREE + 0x166000u), (EMU_FREE + 0x167900u), "", "words") \
	X("FREE_TRADER_LIST", (EMU_FREE + 0x168000u), (EMU_FREE + 0x169000u), "", "words") \
	X("FREE_PET_OPTIONS", (EMU_FREE + 0x169000u), (EMU_FREE + 0x16A000u), "", "words") \
	X("FREE_BBS", (EMU_FREE + 0x16A000u), (EMU_FREE + 0x16B000u), "", "words") \
	X("FREE_NCP_TEXT", (EMU_FREE + 0x16B000u), (EMU_FREE + 0x16BC00u), "", "words") \
	X("FREE_XAREA", (EMU_FREE + 0x170000u), (EMU_FREE + 0x1A0000u), "", "world") \
	X("FREE_SONGS", (EMU_FREE + 0x1A0000u), (EMU_FREE + 0x260000u), "", "world") \
	X("FREE_XNAVIS", (EMU_FREE + 0x260000u), (EMU_FREE + 0x2A0000u), "", "world") \
	X("FREE_BACKDROPS", (EMU_FREE + 0x2A0000u), (EMU_FREE + 0x2E0000u), "", "world") \
	X("FREE_FLAME", (EMU_FREE + 0x2E0000u), (EMU_FREE + 0x2E1000u), "", "world") \
	X("FREE_MAP_BACKDROPS", (EMU_FREE + 0x2F0000u), (EMU_FREE + 0x320000u), "", "world") \
	X("FREE_VIRUS_ROOM", (EMU_FREE + 0x330000u), (EMU_FREE + 0x348000u), "", "code") \
	X("FREE_FAMILIES", (EMU_FREE + 0x348000u), (EMU_FREE + 0x400000u), "", "code") \
	X("FREE_CHIPS", (EMU_FREE + 0x400000u), (EMU_FREE + 0x480000u), "", "code") \
	X("FREE_SOULS", (EMU_FREE + 0x480000u), (EMU_FREE + 0x4C0000u), "", "code") \
	X("FREE_PANELS", (EMU_FREE + 0x4C0000u), (EMU_FREE + 0x4D0000u), "", "code") \
	X("FREE_LATE_FAMILIES", (EMU_FREE + 0x4D0000u), (EMU_FREE + 0x500000u), "", "code") \
	X("FREE_NAVIS", (EMU_FREE + 0x500000u), (EMU_FREE + 0x600000u), "", "code") \
	X("FREE_NAVI_SPRITES", (EMU_FREE + 0x500000u), (EMU_FREE + 0x580000u), "FREE_NAVIS", "code") \
	X("FREE_NAVI_ROOM", (EMU_FREE + 0x580000u), (EMU_FREE + 0x588000u), "FREE_NAVIS", "code") \
	X("FREE_NAVI_HOMES", (EMU_FREE + 0x588000u), (EMU_FREE + 0x590000u), "FREE_NAVIS", "code") \
	X("FREE_NAVI_CODE", (EMU_FREE + 0x590000u), (EMU_FREE + 0x5B0000u), "FREE_NAVIS", "code") \
	X("FREE_NAVI_ART", (EMU_FREE + 0x5B0000u), (EMU_FREE + 0x600000u), "FREE_NAVIS", "code") \
	X("FREE_CHIP_CODE", (EMU_FREE + 0x600000u), (EMU_FREE + 0x700000u), "", "code") \
	X("FREE_ATTACK_FAMILIES", (EMU_FREE + 0x600000u), (EMU_FREE + 0x600400u), "FREE_CHIP_CODE", "code") \
	X("FREE_CHIP_MOVED", (EMU_FREE + 0x600400u), (EMU_FREE + 0x700000u), "FREE_CHIP_CODE", "code") \
	X("FREE_SOULS_ART", (EMU_FREE + 0x700000u), (EMU_FREE + 0x710000u), "", "code") \
	X("FREE_TAIL", (EMU_FREE + 0x710000u), (EMU_FREE + 0x711000u), "", "code") \
	X("FREE_LATE_CODE", (EMU_FREE + 0x711000u), (EMU_FREE + 0x720000u), "", "code") \
	X("FREE_BN4_NAVIS", (EMU_FREE + 0x720000u), (EMU_FREE + 0x730000u), "", "code") \
	X("FREE_BN4_NAVI_CODE", (EMU_FREE + 0x730000u), (EMU_FREE + 0x750000u), "", "code") \
	X("FREE_BN4_NAVI_ART", (EMU_FREE + 0x750000u), (EMU_FREE + 0x77C000u), "", "code") \
	X("FREE_BN4_NAVI_SPRITE", (EMU_FREE + 0x77C000u), (EMU_FREE + 0x78C000u), "", "code") \
	X("FREE_BRACKET_FACES", (EMU_FREE + 0x78C000u), (EMU_FREE + 0x790000u), "", "code") \
	X("FREE_BRACKET", (EMU_FREE + 0x790000u), (EMU_FREE + 0x7A0000u), "", "code") \
	X("FREE_LIB_TEAM", (EMU_FREE + 0x7A0000u), (EMU_FREE + 0x7FA000u), "", "art") \
	X("FREE_NAVI_SHARED_FOUR", (EMU_FREE + 0x7FA000u), (EMU_FREE + 0x800000u), "", "art")
#define FREE_SPACE_ROM_ZONES(X) \
	X("engine", (EMU_FREE + 0x000000u), (EMU_FREE + 0x002F00u)) \
	X("layers", (EMU_FREE + 0x002F00u), (EMU_FREE + 0x090000u)) \
	X("code-low", (EMU_FREE + 0x090000u), (EMU_FREE + 0x150000u)) \
	X("words", (EMU_FREE + 0x150000u), (EMU_FREE + 0x170000u)) \
	X("world", (EMU_FREE + 0x170000u), (EMU_FREE + 0x330000u)) \
	X("code", (EMU_FREE + 0x330000u), (EMU_FREE + 0x7A0000u)) \
	X("art", (EMU_FREE + 0x7A0000u), (EMU_FREE + 0x800000u))
#define FREE_SPACE_RAM(X) \
	X("RAM_PACK_EXT", 0x02010CE0u, 0x020115E0u, "", "persistent") \
	X("RAM_PLAYER_AI", 0x020115E0u, 0x02011730u, "", "persistent") \
	X("RAM_TEAM", 0x02011730u, 0x02011750u, "", "persistent") \
	X("RAM_TEAM_LINK", 0x02011750u, 0x02011790u, "", "persistent") \
	X("RAM_TEAM_CUSTOM", 0x02011790u, 0x02011800u, "", "persistent") \
	X("RAM_BRACKET", 0x02029A00u, 0x0202DA00u, "", "mode") \
	X("RAM_SEA", 0x0203FB50u, 0x0203FBE0u, "", "battle") \
	X("RAM_DARK_LISTS", 0x0203FBE0u, 0x0203FDA0u, "", "battle") \
	X("RAM_NAVIS", 0x0203FDA0u, 0x0203FDB0u, "", "battle") \
	X("RAM_DARK_AI", 0x0203FDB0u, 0x0203FEF0u, "", "battle") \
	X("RAM_BATTLE", 0x0203FEF0u, 0x0203FF00u, "", "battle") \
	X("RAM_DISPATCH_SIDE", 0x0203FF00u, 0x0203FF60u, "", "battle") \
	X("RAM_COLONEL", 0x0203FF60u, 0x0203FF70u, "", "battle") \
	X("RAM_SOULS", 0x0203FF70u, 0x0203FFF0u, "", "battle") \
	X("RAM_TEAM_TICK", 0x0203FFF0u, 0x0203FFF4u, "", "battle") \
	X("RAM_CHIP_AI_ROOMS", 0x0203FFF4u, 0x0203FFF8u, "", "battle") \
	X("RAM_TEAM_CLOCK", 0x0203FFF8u, 0x0203FFFCu, "", "battle") \
	X("RAM_PA_WIDE", 0x0203FFFCu, 0x02040000u, "", "battle")
#define FREE_SPACE_RAM_ZONES(X) \
	X("persistent", 0x02010CE0u, 0x02011800u) \
	X("mode", 0x02029A00u, 0x0202DA00u) \
	X("battle", 0x0203FB50u, 0x02040000u)

#endif
