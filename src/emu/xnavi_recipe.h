/* A Navi's recipe (issue #82; docs/MULTIROM.md, Transplanted Navis): what
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
 * Soul's 0x16) is AI XN_AI_OF(n), its V1 id XN_ID_OF(n), its versions'
 * after it */
#define XN_FIRST_AI   25                  /* the first AI index past BN6's (the control's) */
#define XN_AI_OF(n)   (32 + ((n) == 0x16 ? 6 : (n) - 7))
#define XN_FIRST_ID   0x1C4               /* the Navis' ids, from here (the control's first), */
#define XN_IDS_END    XV_FIRST_ID         /* ... to here */
#define XN_ID_OF(n)   (XN_FIRST_ID + 4 * ((n) == 0x16 ? 6 : (n) - 7))
/* ... and its companion's overlay, where its game's kind is one BN6
 * lacks, BN6's kind XN_KIND_OF(n), past BN6's own (the grown overlay
 * tables, xnavi_room.h) */
#define XN_KIND_OF(n) (BN6_OVERLAY_KINDS + ((n) == 0x16 ? 6 : (n) - 7))
#define XN_VERSIONS_MAX XV_VERSIONS       /* (the control's six: BN6's V1, EX, SP, RV, BX and a sixth) */
/* Where they go */
#define XN_HOMES      FREE_NAVI_HOMES     /* their homes, XV_HOME_SIZE each (XV_HOME_*'s layout), XN_HOMES_MAX of them */
#define XN_HOMES_MAX  16
#define XN_STUBS      FREE_NAVI_CODE      /* the stubs BN5's Navis' calls to what BN6 lacks go to (xnavi_install.c): */
#define XN_STUB_ZERO  (XN_STUBS + 0x00u)  /* ... r0 0 (a panel claim's spawners, the DS chip AI's test) */
#define XN_STUB_DS    (XN_STUBS + 0x10u)  /* ... the DS chip AI entered or run: logged (versions 0-3 never reach it) */
#define XN_STUBS_END  (XN_STUBS + 0x100u)
#define XN_OBJECTS    XN_STUBS_END        /* BN5's engine objects moved once for every track (xnavi_objects.h), to: */
#define XN_OBJECTS_END (FREE_NAVI_CODE + 0x2000u)
#define XN_OBJECTS_HOME (XN_HOMES + (XN_HOMES_MAX - 1) * XV_HOME_SIZE)   /* ... their home (their veneers), the last of the homes */
#define XN_CODE       XN_OBJECTS_END      /* the Navis' code, block by block, a room each, to: */
#define XN_ART        FREE_NAVI_ART       /* what their attacks draw: a room each (their overlays' masks), then: */
#define XN_SHARED_ART (FREE_NAVI_ART + 0x2000u)  /* ... their sprites, each at a place and number of its own whichever Navi draws it,
                                                      copied by the first install that needs it, to: */
#define XN_END        FREE_CHIP_CODE

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

typedef struct {
	XVirusFamily code;          /* its code and records as a family's: .ai its Navi AI index, .first_id its first id, .home its home,
	                               .src_ai its AI index in its game's Navi tables, .names its versions' ids there (its reward rows'),
	                               .hooks its tables 5-12 (post, spawn start, spawn end, paralysed, frozen, bubbled, flinching, dragged),
	                               .sprites what its attacks draw (its battle sprite is xnavi_sprites.h's), .ice its ice block's size */
	int versions;               /* its versions installed, ids first_id on (BN5's V1, V2, V3, SP: 4; BN6's control 6) */
	XvRef prestep, hook_a;      /* its tables 1 and 2: BN6's pre-step for its kind (A-D), hook A (the Navis' empty hook) */
	XvRef companion, companion_end;   /* the routines its companion starts and ends by (the triples' Navi words at its AI index) */
	int sprite_of;              /* BN5's Navi whose battle sprite it draws at its number (xnavi_sprites.h); -1 its struct1's own */
	uint16_t shown[XV_VERSIONS];   /* each version's name as its game shows it: an entry of its Navi names (BN5 shows Colonel V3 as V1) */
	const XnSound *sounds;      /* the sounds BN6 numbers otherwise */
	int nsounds;
	const XnOverlay *overlay;   /* its companion's overlay of its own kind, NULL none */
	const XvType *needs;        /* the moved objects' types its code spawns (xnavi_objects.h), each installed before it */
	int nneeds;
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
