/* BN4 Red Sun's tournament bracket (its main mode 0x24) moved into BN6 and
 * run as a main mode of BN6's own (docs/multirom/bn4-tournament.md, Moved
 * into BN6): its code, its data and its art copied from the player's BN4
 * ROM by a recipe of numbers (xbracket_bn4.c, written by
 * tools/xrecipe_bracket.py), its calls into BN4's engine aimed at BN6's
 * routines of the same body, its RAM placed in RAM a mode screen takes
 * while the map waits (free_space.h's RAM_BRACKET), into FREE_BRACKET.
 * Installed wherever BN4's ROM is read, after the families
 * (xinstall.c). */
#ifndef CW_XBRACKET_H
#define CW_XBRACKET_H

#include <stdbool.h>
#include <stdint.h>

#include "bn4_tournament.h"
#include "free_space.h"

/* Its room in the core's copy (FREE_BRACKET) */
#define XBR_AT          FREE_BRACKET
#define XBR_STUB_NONE   (XBR_AT + 0x00u)   /* what a call into BN4's logic around the screen does here: bx lr */
#define XBR_SHIM_R1     (XBR_AT + 0x10u)   /* eToolkit +0x3C read as BN4's tournaments: r1 their place (ldr; bx lr; the word) */
#define XBR_SHIM_R2     (XBR_AT + 0x20u)   /* ... r2 */
#define XBR_STUB_RETURN (XBR_AT + 0x30u)   /* the leave's return to the map: BN6's game mode, its map entered again (BN6_SUB_ENTER) */
#define XBR_STUB_FRAME  (XBR_AT + 0x1A0u)  /* the mode's routine: the line values copied for the HBlank DMA (XBR_BLEND_DMA), then the screen's */
#define XBR_STUB_DISPLAY (XBR_AT + 0x1D0u) /* r0 into the render info's DISPCNT ([eToolkit +8] +0), as BN4_MAP_LEAVE ends */
#define XBR_STUB_HBLANK (XBR_AT + 0x1E0u)  /* BN4's HBlank DMA set as BN6's (BN6_HBLANK_DMA_SET: r4 the line buffer and r5 its bytes, the
                                              source itself and its 0x140) */
#define XBR_STUBS_END   (XBR_AT + 0x200u)
#define XBR_ENTER       (XBR_AT + 0x40u)   /* the entry a game call runs: the map left as BN6's shop leaves it (XBR_LIST's calls), BN4's set-up
                                              moved, then BN6's main mode made XBR_MODE */
#define XBR_LIST        (XBR_AT + 0x60u)   /* ... its calls, r0 and the routine a pair, (0, 0) the end: BN6_MAP_TO_SHOP's, read from it */
#define XBR_LIST_MAX    40
#define XBR_ENTER_END   (XBR_AT + 0x200u)
#define XBR_VENEERS     (XBR_AT + 0x200u)  /* its calls out's veneers, XBR_NVENEERS of THUMB_VENEER_SIZE */
#define XBR_NVENEERS    64
#define XBR_CODE        (XBR_AT + 0x1000u) /* its code and data, a block each in a slot of 0x800 */
#define XBR_ART         (XBR_AT + 0x6000u) /* its art, a block after another (0x40 apart), to FREE_BRACKET_END */
#define XBR_MODE        0x3C               /* BN6's main mode it runs as: one Gregar leaves empty (its table's entry 0) */
#define XBR_MODES       0x0800034Cu        /* BN6's main modes' routines (main_subsystemJumpTable), by the mode's byte */

/* Its RAM (RAM_BRACKET), where BN4's was */
#define XBR_STATE       (RAM_BRACKET + 0x000u)   /* BN4_BRACKET's place: +5 the tournament, +6 from a script, +7 its argument */
#define XBR_TOURNAMENTS (RAM_BRACKET + 0x040u)   /* BN4_TOURNAMENTS': four of BN4_TOURNAMENT_SIZE */
#define XBR_BLEND       (RAM_BRACKET + 0x100u)   /* the blend values sent to BLDALPHA line by line (BN4's 0x02009440, 0x140 bytes) */
#define XBR_SPRITES     (RAM_BRACKET + 0x300u)   /* BN4_MENU_SPRITES': BN4_MENU_SPRITE_COUNT of BN4_MENU_SPRITE_SIZE */
#define XBR_BLEND_DMA   (RAM_BRACKET + 0xC00u)   /* ... the HBlank DMA's source: them as the frame before wrote them (XBR_STUB_FRAME) */
#define XBR_TILES       (RAM_BRACKET + 0x1000u)  /* its tiles unpacked (BN4's 0x0202E000, 0x26C4 bytes), to RAM_BRACKET_END */

/* An entrant as the entry takes it: `id` BN4's own plate (BN4_ENTRANT_ID: 0x00-0x16, BN4_ENTRANT_MEGAMAN, 0xFF none), or
 * XBR_PLATE_NAVI(n) for BN6's and BN5's Navis */
typedef struct { uint8_t id, flags, seed; } XbrEntrant;
/* The plate of the run's Navi `n`, his number in src/layer/guardians.c (guardian(n): BN6's 1-10, 12-16 and 18, BN5's
 * GUARDIAN_OLDER_FIRST to GUARDIAN_OLDER_LAST, 24-29), as the moved bracket draws him: his face made from his own
 * game's ROM at install (xbracket_faces.h), ids 0x21-0x32 and 0x38-0x3D; one whose face was not made (BN5's ROM not
 * read) draws as an empty plate (BN4_PLATE_EMPTY_ID) */
#define XBR_PLATE_NAVI(n) (0x20 + (n))

/* The bracket into the core's copy, once a core, where BN4's ROM is read:
 * true once it is there; false, nothing of it written, where a check
 * failed (stderr says which). */
bool xbracket_install(void);
/* The entry: tournament `t` (BN4_TOURNAMENT_DEN to BN4_TOURNAMENT_FREE)
 * shown with these eight entrants in bracket order, from the map: BN4's
 * set-up, then BN6's main mode XBR_MODE, through a game call; A, B or START
 * fades it out and enters the map again. False where it is not installed
 * or the call never ran. */
bool xbracket_open(int t, const XbrEntrant entrants[BN4_ENTRANTS]);
/* The dev step "bracket T IIFFSS/..." (capture.c): tournament T's eight
 * entrants, each its id, flags and seed in hex, '/' between them. */
bool xbracket_dev_step(const char *spec);

#endif
