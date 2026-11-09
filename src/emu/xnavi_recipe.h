/* A Navi's recipe (issue #82; docs/multirom/navis.md): what
 * a Navi of another game's (or BN6's own, the control) brings into BN6's
 * engine, numbers alone. Its code, records and art as a virus family's
 * recipe has them (xvirus_recipe.h: the families' code mover moves both),
 * placed in the Navis' room (FREE_NAVIS), and what a Navi has more: the
 * Navi tables' pre-step and hook A, the companion's start and end by its
 * AI index, its battle sprite at its number (xnavi_sprites.h), its
 * versions. */
#ifndef CW_XNAVI_RECIPE_H
#define CW_XNAVI_RECIPE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "free_space.h"
#include "xvirus_recipe.h"

/* The Navis' numbers, fixed (the static model): an AI index in the grown
 * Navi tables past BN6's 25, ids past BN6's (its Navis end at 0x190, the
 * player's forms at 0x1C3) and before the families' (XV_FIRST_ID): the
 * Navis' names are the Navi archive's (BN6_NAME_OF, an id's low byte),
 * rebuilt (xnavi_room.h). BN5's Navi n (Team Colonel's 7-12, the Dark
 * Soul's 0x16, the Darkloids 13-16: ShadeMan, BlizzardMan, CloudMan,
 * CosmoMan) has its seat XN_SEAT(n) (0-5, 6, 7-10): AI XN_AI_OF(n), its
 * V1 id XN_ID_OF(n), its versions' after it */
#define XN_SEAT(n)    ((n) == 0x16 ? 6 : (n) >= 13 ? (n) - 6 : (n) - 7)
#define XN_FIRST_AI   25                  /* the first AI index past BN6's (the control's) */
#define XN_AI_OF(n)   (32 + XN_SEAT(n))
#define XN_FIRST_ID   0x1C4               /* the Navis' ids, from here (the control's first), */
#define XN_IDS_END    XV_FIRST_ID         /* ... to here */
#define XN_ID_OF(n)   (XN_FIRST_ID + 4 * XN_SEAT(n))
/* ... and its companion's overlay, where its game's kind is one BN6
 * lacks, BN6's kind XN_KIND_OF(n), past BN6's own (the grown overlay
 * tables, xnavi_room.h) */
#define XN_KIND_OF(n) (BN6_OVERLAY_KINDS + XN_SEAT(n))
#define XN_VERSIONS_MAX XV_VERSIONS       /* (the control's six: BN6's V1, EX, SP, RV, BX and a sixth) */
/* Where they go */
#define XN_HOMES      FREE_NAVI_HOMES     /* their homes, XV_HOME_SIZE each (XV_HOME_*'s layout), XN_HOMES_MAX of them */
#define XN_HOMES_MAX  16
#define XN_STUBS      FREE_NAVI_CODE      /* the stubs BN5's Navis' calls to what BN6 lacks go to (xnavi_install.c): */
#define XN_STUB_ZERO  (XN_STUBS + 0x00u)  /* ... r0 0 (a panel claim's spawners, the DS chip AI's test) */
#define XN_STUB_DS    (XN_STUBS + 0x10u)  /* ... the DS chip AI entered or run: logged (versions 0-3 never reach it) */
#define XN_STUB_BUSTER (XN_STUBS + 0x20u) /* ... the Navis' buster's setup (BN5's attack setup 0x3E, which BN6 has none of: AttackVars +8
                                             its buster's level + 1, +0xA 0, r0 its family XN_FAMILY_OF(0x16)), 0x20 bytes */
#define XN_STUB_SIDES (XN_STUBS + 0x40u)  /* ... BattleState +0x5C's bits r0 ORed into the moved Colonel's own (BN6_COLONEL_SIDES) */
#define XN_STUB_CHIPDATA (XN_STUBS + 0x50u) /* ... the Dark Soul's chip AI's record of BN6's chip r0 (BN5's 0x0801DF0C): XN_DARK_RECORDS' */
#define XN_STUBS_END  (XN_STUBS + 0x60u)
#define XN_SEA        XN_STUBS_END        /* BN5's sea's submerge's code (xsea.c), to: */
#define XN_SEA_END    (XN_STUBS + 0x100u)
#define XN_OBJECTS    XN_SEA_END          /* BN5's engine objects moved once for every track (xnavi_objects.h), to: */
#define XN_OBJECTS_END (FREE_NAVI_CODE + 0x2000u)
#define XN_OBJECTS_HOME (XN_HOMES + (XN_HOMES_MAX - 1) * XV_HOME_SIZE)   /* ... their home (their veneers), the last of the homes */
#define XN_CODE       XN_OBJECTS_END      /* the Navis' code, block by block, a room each (slots 0-7), to: */
#define XN_ART        FREE_NAVI_ART       /* what their attacks draw: a room each (their overlays' masks), then: */
#define XN_SHARED_ART (FREE_NAVI_ART + 0x2000u)  /* ... their sprites, each at a place and number of its own whichever Navi draws it,
                                                      copied by the first install that needs it, to: */
#define XN_END        XN_MORE
#define XN_MORE       (FREE_NAVI_ART + 0x38000u)  /* the Darkloids' code, a room each (slots 8-11, XN_SLOT_CODE bytes), to: */
#define XN_MORE_ART   (XN_MORE + 0xF000u) /* ... what their attacks draw, a room each (XN_SLOT_ART bytes), to: */
#define XN_MORE_END   (XN_MORE_ART + 0x1000u)   /* (before XN_DARK_RECORDS) */
#define XN_SLOT_CODE  0x3C00u             /* a Navi's code room */
#define XN_SLOT_ART   0x400u              /* ... and its art's */
#define XN_DARK_IDS     512u   /* the ids the Dark Soul's chip AI reads, to 511: its records' room, 0x2C each */
#define XN_DARK_RECORDS (FREE_CHIP_CODE - 0x6000u)   /* the Dark Soul's chip records (xnavi_dark.c): BN6's 512, each with the four bytes
                                                        its chip AI reads BN5's (XN_DARK_FIELDS), 0x2C each, to FREE_CHIP_CODE */

/* BN4 Red Sun's Navis (xnavi_bn4.c, tools/xnavirecipe_bn4.py; docs/multirom/bn4-navis.md, GutsMan built): BN4's Navi of V1 id n
 * at seat (n - BN4_NAVI_FIRST_ID) / 4, a fixed place each, its AI XN_BN4_FIRST_AI + seat, its four ids from XN_BN4_FIRST_ID +
 * 4 x seat (past every family's ids, XV_FIRST_ID + 6 x XV_FAMILIES, to XV_IDS), its home, code and art in a room of their
 * own (FREE_BN4_NAVIS) */
#define XN_BN4_SEATS     25
#define XN_BN4_FIRST_AI  48
#define XN_BN4_FIRST_ID  0x380
#define XN_BN4_IDS_END   XV_IDS
#define XN_BN4_HOMES     FREE_BN4_NAVIS
#define XN_BN4_CODE      FREE_BN4_NAVI_CODE
#define XN_BN4_ART       FREE_BN4_NAVI_ART
#define XN_BN4_END       XN_BN4_SPRITE
/* ... their battle sprites in one room, the coming battle's Navi's copied in from the player's BN4 ROM (xnavi_bn4_room.h), each
 * Navi's list 2 entry pointed at it; the room holds the largest (Duo's, 61040 bytes unpacked) */
#define XN_BN4_SPRITE      FREE_BN4_NAVI_SPRITE
#define XN_BN4_SPRITE_SIZE (FREE_BRACKET_FACES - FREE_BN4_NAVI_SPRITE)
/* ... and which sprite each seat's is: BN4's list 2 number of his battle sprite, a byte a seat (0xFF none), past the homes */
#define XN_BN4_SPRITES     (XN_BN4_HOMES + XN_BN4_SEATS * XV_HOME_SIZE)
/* ... their attacks' art and pool songs (XN_BN4_ART to XN_BN4_END) one room too, the coming battle's Navi's filled in
 * (xnavi_bn4_art.h); whose each seat's battle fills it with, a byte a seat (his own seat, 0xFF none), past the sprites' */
#define XN_BN4_ARTS        (XN_BN4_SPRITES + XN_BN4_SEATS)

/* Whether `a` is in a Navi's code room (XN_CODE's, XN_MORE's or BN4's Navis') */
static inline bool xn_code_at(uint32_t a) {
	return (a >= XN_CODE && a < XN_ART) || (a >= XN_MORE && a < XN_MORE_ART) || (a >= XN_BN4_CODE && a < XN_BN4_ART);
}
/* ... or in one of their art's rooms (XN_ART's or XN_MORE_ART's) */
static inline bool xn_art_room(uint32_t a, uint32_t n) {
	return (a >= XN_ART && a <= XN_SHARED_ART && n <= XN_SHARED_ART - a) || (a >= XN_MORE_ART && a <= XN_MORE_END && n <= XN_MORE_END - a) ||
		(a >= XN_BN4_ART && a <= XN_BN4_END && n <= XN_BN4_END - a);
}

/* BN5's attack family F as BN6's grown table holds it (xattack_grow.h): at
 * 0x50 + F, as the chips' are (issue #82's plan) */
#define XN_FAMILY_OF(f) (0x50 + (f))

/* An attack family of BN5's it runs that BN6 has none of: its routine (in
 * its blocks) put in BN6's attack families grown at `bn6`
 * (XN_FAMILY_OF(bn5)), where the entry still holds the grown table's stub
 * (another track's copy of the same family there is left as it is) */
typedef struct { uint8_t bn5, bn6; XvRef at; } XnFamily;

/* A sound its code starts that BN6 has as another song, or none: its
 * game's song copied in (xsong.h, played by its own players) and the
 * immediate that names it made the copy's slot: movs r0,#lo at `at`, and
 * adds r0,#hi at `at2` where the song is past 0xFF (0 none), each checked */
typedef struct { uint32_t at, at2; uint16_t song; } XnSound;

/* Its companion's overlay (BN6_OVERLAY_*'s object) where its game's kind
 * is one BN6 lacks: BN6's kind for it (XN_KIND_OF), its masks (a byte an
 * animation: 0 drawn behind its Navi) copied from its game's ROM at
 * `masks`, `masks_len` bytes checked by their CRC32, to `masks_at` in its
 * art's room, and its sprite as BN6's lists hold it (one of the recipe's
 * sprites, or BN6's own): its list and number */
typedef struct { uint32_t masks, masks_at, crc; uint16_t masks_len; uint8_t kind, list, number; } XnOverlay;

/* A spawn its game's init makes that BN6's actor init has none of (BN4's Navis' companions; xnavi_init.c; docs/multirom/
 * bn4-navis.md, Companions): `spawner` (its own code, the first-kind type it spawns moved with it) called by a bl with r4
 * `r4` (XN_R4_ANY: left), and where `table` (in its blocks) is not 0 with r0 `hi` << 16 | that table's byte by its version (its object's +4,
 * as BN6's spawn keeps it), r2 that byte and r1 its version; then, where it spawned one (r0 not 0), the companion's
 * +link_at given the Navi's address + link_from (link_at 0: none) and the Navi's +keep_at given the companion (0: none) */
typedef struct { XvRef spawner; uint32_t r4, table; uint8_t hi, link_from, link_at, keep_at; } XnSpawn;
#define XN_R4_ANY 0xFFFFFFFFu   /* (its r4 as BN6 leaves it: its init sets none for the call) */
/* What its game's init does past BN6's actor init (XNavi.init): its spawns, in its init's order, made by a shim of its home
 * at its companion start (BN6's actor init's call after its collision set up, BN6_ACTOR_COMPANION_CALL, which its
 * .companion must leave empty) or, `at_end`, at its spawn's end (its table 7, after the whole of BN6's actor init); and at
 * its spawn's end, by a shim of its home there, its collision's own r3 (XNavi.coll_arg), its +0x11 its +0x10 (`anim`: its
 * init's 0 where BN6's actor init writes 0xFF), then its spawns where `at_end`, its sprite updated (`sprite`, BN6's
 * BN6_SPRITE_UPDATE) and its update run (`frame`, BN6_NAVI_FRAME), in that order; its panel (+0x12) kept at its +6
 * before its spawns (`home`: AquaMan's, his decision's last panel, which BN6's spawn leaves 0) */
typedef struct { const XnSpawn *spawns; int nspawns; bool at_end, anim, sprite, frame, home; } XnInit;

/* A summon its game's code makes (BN4's Roll: her MegaMan's state 47 calls BN4_SPAWN_ENEMY, an enemy by its id on a free panel
 * of her side; docs/multirom/bn4-navis.md, Roll built): that call aimed at a shim at `at` (its code room) that picks the panel
 * as BN4's does, by `panels` (its copy of BN4_FREE_PANELS) with `masks` (wanted, refused: side 0's, then side 1's) and one
 * draw, gives its game's id BN6's by `ids` (to 0: none spawned) and spawns it by BN6_SPAWN_ENEMY_1, then plays `sound`
 * (xnavi_init.c, xvirus_shim_summon) */
typedef struct { uint8_t from; uint16_t to; } XnSummonId;
typedef struct { uint32_t at; XvRef panels; uint32_t masks[4]; const XnSummonId *ids; int nids; uint16_t sound; } XnSummon;
/* A field its code loads by an immediate BN6's place of it is past (`ldrh rd,[rb,#was]` where BN6 keeps the field at `now`,
 * past the 0x3E an ldrh reaches: BN4's collision data's final damage +0x28, BN6's +0x80), then compares with 0 (`cmp rd,#0`):
 * the two instructions made a call to a shim of its home (`movs rd,#now; ldrh rd,[rb,rd]; cmp rd,#0; bx lr`), whose flags
 * the branch after reads (xnavi_init.c; lr on its routine's stack there: the generator checks the routine pushes it;
 * docs/multirom/bn4-navis.md, WoodMan built) */
typedef struct { uint32_t at; uint8_t rd, rb; uint16_t was, now; } XnFar;
#define XN_FAR_SIZE 8u

typedef struct {
	XVirusFamily code;          /* its code and records as a family's: .ai its Navi AI index, .first_id its first id, .home its home,
	                               .src_ai its AI index in its game's Navi tables, .names its versions' ids there (its reward rows'),
	                               .hooks its tables 5-12 (post, spawn start, spawn end, paralysed, frozen, bubbled, flinching, dragged),
	                               .sprites what its attacks draw (its battle sprite is xnavi_sprites.h's), .ice its ice block's size */
	int versions;               /* its versions installed, ids first_id on (BN5's V1, V2, V3, SP: 4; BN6's control 6) */
	XvRef prestep, hook_a;      /* its tables 1 and 2: BN6's pre-step for its kind (A-D), hook A (the Navis' empty hook) */
	XvRef companion, companion_end;   /* the routines its companion starts and ends by (the triples' Navi words at its AI index) */
	XvRef hide;                 /* the routine its attacks hide its companion's overlay by (BN6_NAVI_HIDE_REF's words at its AI index) */
	int sprite_of;              /* BN5's Navi whose battle sprite it draws at its number (xnavi_sprites.h); -1 its struct1's own */
	uint8_t coll_arg;           /* its collision setup's r3 (BN6_COLL_SETUP_ARG: the hit bits its body's hit ORs into the struck's) where
	                               its game's init passes one BN6's actor init (3 for every actor, 0x08016FE8) does not: BN4's Navis'
	                               0x21; written over at its spawn's end by a shim of its home (xnavi_init.c); 0 none */
	uint16_t shown[XV_VERSIONS];   /* each version's name as its game shows it: an entry of its Navi names (BN5 shows Colonel V3 as V1) */
	const XnSound *sounds;      /* the sounds BN6 numbers otherwise */
	int nsounds;
	const XnOverlay *overlay;   /* its companion's overlay of its own kind, NULL none */
	const XvType *needs;        /* the moved objects' types its code spawns (xnavi_objects.h), each installed before it */
	int nneeds;
	const XnFamily *families;   /* BN5's attack families it runs, moved with it */
	int nfamilies;
	const XnInit *init;         /* what its game's init does past BN6's actor init (BN4's Navis' companions), NULL nothing */
	XvRef hide_step;            /* the routine run before each of its states (BN4_HIDE_STEP, copied with it: BN4's Navi runner hides the
	                               invisible and keeps their marker before every state, 0x08013F12, where BN6's status step calls its
	                               twin nowhere), through stubs of its home (xnavi_init.c); addr 0 none. SearchMan's, who hides */
	const XnSummon *summon;     /* its summon's shim (BN4's Roll's), NULL none */
	uint32_t palettes;          /* its palette by version (BN4's Navis': BN4_NAVI_PALETTE's, a byte each, V1's the lowest), set at its
	                               spawn's end over BN6's actor init's (0 for versions 0-3, BN6_NAVI_PALETTE_VERSIONS); 0 none */
	const XnFar *fars;          /* its loads of a field past an immediate's reach in BN6, through shims of its home (BN4's WoodMan's) */
	int nfars;
} XNavi;

/* Whether the recipe holds together (AI index, ids, home, its blocks in
 * the Navis' code room, its data, states, hooks, calls, art); where not,
 * why, into `why`. */
bool xnavi_recipe_ok(const XNavi *n, char *why, size_t len);
/* Whether the moved objects' recipe (xnavi_objects.h) holds together: its
 * block in XN_OBJECTS, its home XN_OBJECTS_HOME, its calls BN6's routines
 * or the dispatch's spawners, its sprites in the shared art, its types
 * moved; where not, why, into `why`. */
bool xnavi_objects_ok(const XVirusFamily *f, char *why, size_t len);

#endif
