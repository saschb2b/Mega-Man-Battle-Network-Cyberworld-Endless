/* The free space past the game's 8 MB in the core's ROM copy, and the RAM
 * BN6 leaves alone: one plan every part of the engine and every track of
 * issue #82 follows (docs/EMULATION.md, Memory the engine writes and BN6's
 * RAM). Each region is its own writer's, [its start, its end); a span no
 * region names is free. tests/test_core.c checks the regions in order and
 * apart, and each installer's own layout inside its region; what the
 * installers write as the core starts is claimed byte by byte as they
 * write it (claims.h), so that no two write the same bytes. emu.h includes
 * this: the places are bus addresses of the core's copy. */
#ifndef CW_FREE_SPACE_H
#define CW_FREE_SPACE_H

#define EMU_FREE 0x08800000u      /* first free bus address past an 8 MB game */
#define FREE_END (EMU_FREE + 0x800000u)   /* the copy's end: 16 MB (EMU_ROM_SIZE) */

/* ---- the ROM copy, by its writers ---- */
#define FREE_WARP           (EMU_FREE + 0x000000u)   /* a warp record and a one-entry warp list (gamecall.c), to: */
#define FREE_CALL_RETURN    (EMU_FREE + 0x000100u)   /* where a game call's routine returns: a hook (gamecall.c), to: */
#define FREE_NO_MYSTERY     (EMU_FREE + 0x000180u)   /* an empty Mystery Data list (mapslot.c), to: */
#define FREE_RECORDS        (EMU_FREE + 0x000200u)   /* two battle records in turn, 0x80 apart (encounter.c), to: */
#define FREE_RECORDS_END    (EMU_FREE + 0x000300u)
#define FREE_RUSH           (EMU_FREE + 0x000400u)   /* Rush's gaps (rush.c), to: */
#define FREE_RUSH_END       (EMU_FREE + 0x000640u)
#define FREE_BLOCKERS       (EMU_FREE + 0x000700u)   /* map-object handler 3's records (blockers.c), to: */
#define FREE_BLOCKERS_END   (EMU_FREE + 0x0012E0u)
#define FREE_WARP_LIST      (EMU_FREE + 0x002F00u)   /* the layer map's warp list (mapslot.c), to: */
#define FREE_LAYER          (EMU_FREE + 0x003000u)   /* layer data in two halves (mapslot.c), to: */
#define FREE_TILE_MAP       (EMU_FREE + 0x010000u)   /* the generated tile map (netmap.c), to: */
#define FREE_COORDS         (EMU_FREE + 0x060000u)   /* the generated coordinate data (coords.c), to: */
#define FREE_TOWN_TILE_MAP  (EMU_FREE + 0x100000u)   /* the town's tile map (town.c), to: */
#define FREE_TOWN_COORDS    (EMU_FREE + 0x130000u)   /* the town's coordinate data (coords.c), to: */
#define FREE_TOWN           (EMU_FREE + 0x140000u)   /* the town's scripts, text and tables (mapslot.c), to: */
#define FREE_LAN_HP         (EMU_FREE + 0x148000u)   /* Lan's HP's, then Lan's house's in its second half (mapslot.c), to: */
#define FREE_TALK           (EMU_FREE + 0x150000u)   /* the director's conversations (talk.c), to: */
#define FREE_LABEL          (EMU_FREE + 0x152000u)   /* the map-name label's archive (director_layer.c), to: */
#define FREE_PET_LABEL      (EMU_FREE + 0x153000u)   /* the PET's PLACE (director_layer.c), to: */
#define FREE_PET_TEXT       (EMU_FREE + 0x154000u)   /* the PET's words (pet_text.c): the key items' names, */
#define FREE_PET_DESCS      (EMU_FREE + 0x155000u)   /* ... their descriptions, */
#define FREE_PET_MAILS      (EMU_FREE + 0x158000u)   /* ... the mails' senders and subjects, */
#define FREE_PET_BODIES     (EMU_FREE + 0x15A000u)   /* ... their bodies, to: */
#define FREE_PORTRAITS      (EMU_FREE + 0x166000u)   /* portraits of Falzar's Navis (portrait.c), to: */
#define FREE_PORTRAITS_END  (EMU_FREE + 0x167900u)
#define FREE_TRADER_LIST    (EMU_FREE + 0x168000u)   /* a layer's Chip Trader's prizes of both nets (trader.c), to: */
#define FREE_TRADER_LIST_END (EMU_FREE + 0x169000u)
#define FREE_XAREA          (EMU_FREE + 0x170000u)   /* another game's net area (netmap.c), to: */
#define FREE_SONGS          (EMU_FREE + 0x1A0000u)   /* other games' songs (xsong.c), to: */
#define FREE_XNAVIS         (EMU_FREE + 0x260000u)   /* other games' Navis and map objects (xnavi.c), to: */
#define FREE_BACKDROPS      (EMU_FREE + 0x2A0000u)   /* other games' battle backgrounds (xbackdrop.c), to: */
#define FREE_FLAME          (EMU_FREE + 0x2E0000u)   /* BN6's own flame of darkness (xnavi.c), to: */
#define FREE_FLAME_END      (EMU_FREE + 0x2E1000u)
#define FREE_MAP_BACKDROPS  (EMU_FREE + 0x310000u)   /* other games' net maps' backdrops (xbackdrop.c), to: */
#define FREE_INDOORS        (EMU_FREE + 0x320000u)   /* home's other places (mapslot.c), to: */
/* issue #82's, fixed places the same at every boot with the same ROMs */
#define FREE_VIRUS_ROOM     (EMU_FREE + 0x330000u)   /* BN6's tables grown for the families, the second-level dispatch (xvirus_grow.c), to: */
#define FREE_FAMILIES       (EMU_FREE + 0x348000u)   /* the families: their homes, code and art (xvirus.c), to: */
#define FREE_CHIPS          (EMU_FREE + 0x400000u)   /* the chip registry: records, names, routines, art (chips.c, xchips_install.c), to: */
#define FREE_SOULS          (EMU_FREE + 0x480000u)   /* BN5's Souls in BN6's engine (soulunite.c, soullook.c, soulcode.c; soul_room.h lays it out), to: */
#define FREE_PANELS         (EMU_FREE + 0x4C0000u)   /* BN5's lava and sea as BN6's panels (xpanels.c, xpanels_look.c), to: */
#define FREE_PANELS_END     (EMU_FREE + 0x4D0000u)
#define FREE_NAVIS          (EMU_FREE + 0x500000u)   /* BN5's Navis in BN6's engine (xnavi_*.c), to FREE_CHIP_CODE: */
#define FREE_NAVI_SPRITES   (EMU_FREE + 0x500000u)   /* ... their battle sprites, a fixed place each, and the arm points grown (xnavi_sprites.c), to: */
#define FREE_NAVI_SPRITES_END (EMU_FREE + 0x580000u) /* ... (the Navi chips' and the Souls' one copy of each Navi's sprite there too) */
#define FREE_NAVI_ROOM      FREE_NAVI_SPRITES_END    /* ... BN6's Navi tables grown for them (xnavi_room.c), to: */
#define FREE_NAVI_HOMES     (EMU_FREE + 0x588000u)   /* ... their homes: state table, records, veneers (xnavi_install.c), to: */
#define FREE_NAVI_CODE      (EMU_FREE + 0x590000u)   /* ... their code and their objects', block by block, to: */
#define FREE_NAVI_ART       (EMU_FREE + 0x5B0000u)   /* ... what their attacks draw, to FREE_CHIP_CODE */
#define FREE_CHIP_CODE      (EMU_FREE + 0x600000u)   /* the code of BN5's chips: first */
#define FREE_ATTACK_FAMILIES FREE_CHIP_CODE          /* ... BN6's attack families grown to 240 (xattack_grow.c: the chips' and the Souls' one copy), */
#define FREE_ATTACK_FAMILIES_END (FREE_CHIP_CODE + 0x400u)
#define FREE_CHIP_MOVED     FREE_ATTACK_FAMILIES_END /* ... then the chips' moved code, its tables and what it draws (xchips_code.c), to: */
#define FREE_SOULS_ART      (EMU_FREE + 0x700000u)   /* the Souls' art past their room: BN5's effects BN6 redrew, copied (soulcode.c), to: */
#define FREE_SOULS_ART_END  (EMU_FREE + 0x710000u)
#define FREE_CHIP_PAS       (EMU_FREE + 0x710000u)   /* BN6's Program Advances' list grown with BN5's (xchips_code.c): their recipes, then the list, to: */
#define FREE_CHIP_PAS_END   (EMU_FREE + 0x710400u)
#define FREE_CHIP_AI        (EMU_FREE + 0x710400u)   /* the player's chip AI's stub (darkinvs.c), to: */
#define FREE_CHIP_AI_END    (EMU_FREE + 0x710440u)
#define FREE_TAIL           (EMU_FREE + 0x710440u)   /* free to FREE_END */

/* ---- BN6's RAM it leaves alone (docs/EMULATION.md, BN6's RAM) ---- */
/* Persistent: only the power-on clear writes 0x02010CDC-0x020117FF */
#define RAM_PACK_EXT        0x02010CE0u   /* the chip pack's extension, ids 320-511 (BN6_PACK_EXT; xchips_install.c), to: */
#define RAM_PACK_EXT_END    0x020115E0u   /* ... then: */
#define RAM_PLAYER_AI       0x020115E0u   /* MegaMan's AI data under BN5's chip AI, BN5's 0x140 bytes, then his own's place (darkinvs.c), to: */
#define RAM_PLAYER_AI_END   0x02011730u   /* ... then: */
#define RAM_TEAM            0x02011730u   /* --dev arena2v2's list of what the Practice NetBattle spawns (arena_team.c, epic #167), to: */
#define RAM_TEAM_END        0x02011750u   /* ... then: */
#define RAM_TEAM_RING       0x02011750u   /* ... the partners' keys queued by the battle loop's iteration, 8 a side (arena_team.c), to: */
#define RAM_TEAM_RING_END   0x02011790u   /* ... then spare, persistent, to: */
#define RAM_PERSISTENT_END  0x02011800u   /* (the battle sprite pool from here) */
/* Per battle: the battle's setup clears 0x0203CDA8-0x0203FFFF, and nothing
 * else of BN6's reads or writes 0x0203FB50 on */
#define RAM_SEA             0x0203FB50u   /* BN5's sea's submerge (xsea.h: the submerged, their markers), to: */
#define RAM_DARK_LISTS      0x0203FBE0u   /* the Dark Soul's chip AI's lists, 0xE0 a side (BN6_DARK_LISTS), to: */
#define RAM_NAVIS           0x0203FDA0u   /* the moved Navis' own state: Colonel's sides' bits (BN6_COLONEL_SIDES), the Dark Soul's AI
                                             data's holder (BN6_DARK_AI_OWNER, BN6_DARK_AI_POOL), to: */
#define RAM_DARK_AI         0x0203FDB0u   /* the Dark Soul's AI data, BN5's 0x140 bytes (BN6_DARK_AI), to: */
#define RAM_BATTLE          0x0203FEF0u   /* kept for the moved families' and Navis' own state, to: */
#define RAM_DISPATCH_SIDE   0x0203FF00u   /* the second-level dispatch's side tables (BN6_DISPATCH_SIDE; xvirus_dispatch.c), to: */
#define RAM_COLONEL         0x0203FF60u   /* BN5's Colonel's table moved (BN6_COLONEL_TABLE; xnavi_install.c), to: */
#define RAM_SOULS           0x0203FF70u   /* kept for the Souls' state and BN5's mood (the Souls' track), to: */
#define RAM_TEAM_TICK       0x0203FFF0u   /* u32 --dev arena2v2's count of the battle loop's iterations (arena_team.c), to: */
#define RAM_BATTLE_SPARE    0x0203FFF4u   /* spare (the game call's old mark, unused since issue #33), to: */
#define RAM_BATTLE_END      0x02040000u

#endif
