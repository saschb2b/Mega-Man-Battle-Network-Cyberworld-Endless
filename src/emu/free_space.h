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
#define FREE_NAVIS          (EMU_FREE + 0x500000u)   /* BN5's Navis: first their battle sprites in BN6's list 2 (xnavi_sprites.c), */
#define FREE_NAVI_SPRITES   FREE_NAVIS                /* ... a fixed place each, */
#define FREE_NAVI_SPRITES_END (FREE_NAVIS + 0x080000u) /* ... then kept for the Navis, to: */
#define FREE_CHIP_CODE      (EMU_FREE + 0x600000u)   /* the code of BN5's chips and Souls: first BN6's attack families grown (xattack_grow.c), */
#define FREE_ATTACK_FAMILIES FREE_CHIP_CODE           /* ... 240 words and the stub of an empty one, */
#define FREE_ATTACK_FAMILIES_END (FREE_CHIP_CODE + 0x000400u) /* ... then kept for the chips' code, to: */
#define FREE_TAIL           (EMU_FREE + 0x700000u)   /* free to FREE_END */

/* ---- BN6's RAM it leaves alone (docs/EMULATION.md, BN6's RAM) ---- */
/* Persistent: only the power-on clear writes 0x02010CDC-0x020117FF */
#define RAM_PACK_EXT        0x02010CE0u   /* the chip pack's extension, ids 320-511 (BN6_PACK_EXT; xchips_install.c), to: */
#define RAM_PACK_EXT_END    0x020115E0u   /* ... then spare, persistent, to: */
#define RAM_PERSISTENT_END  0x02011800u   /* (the battle sprite pool from here) */
/* Per battle: the battle's setup clears 0x0203CDA8-0x0203FFFF, and nothing
 * else of BN6's reads or writes 0x0203FB50 on */
#define RAM_BATTLE          0x0203FB50u   /* kept for the moved families' and Navis' own state, to: */
#define RAM_DISPATCH_SIDE   0x0203FF00u   /* the second-level dispatch's side tables (BN6_DISPATCH_SIDE; xvirus_dispatch.c), to: */
#define RAM_COLONEL         0x0203FF60u   /* kept for BN5's Colonel's table, to: */
#define RAM_SOULS           0x0203FF70u   /* kept for the Souls' state and BN5's mood (the Souls' track), to: */
#define RAM_BATTLE_SPARE    0x0203FFF0u   /* spare (the game call's old mark, unused since issue #33), to: */
#define RAM_BATTLE_END      0x02040000u

#endif
