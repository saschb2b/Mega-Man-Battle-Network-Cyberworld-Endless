/* The differential's BN4 trace (docs/devtools/differential.md, BN4): BN4
 * Red Sun's battle on the guest core traced as BN5's is (src/dev/xdiff.c's
 * format, its game 4), each part read from BN4's layout (bn4.h) and written
 * in BN6's units and order, so the comparator reads it as it reads BN6's:
 * a first or third kind's object with its element and enemy id at BN6's
 * places, its collision data, its AI's data and each panel by the fields
 * docs/rom/bn4.md maps to BN6's. A field mapped goes to BN6's place; BN4's
 * other bytes keep their own offset where BN6's place there is free, and
 * else fill the places left, in order: nothing of BN4's is dropped, so two
 * BN4 traces differ where their battles do. */
#ifndef CW_XDIFF_BN4_TRACE_H
#define CW_XDIFF_BN4_TRACE_H

#include <stdint.h>

#include "xdiff_events.h"   /* (XdGame: BN4's routines the differential's hooks are placed on) */

/* the parts a BN4 trace translates */
enum { XD4_OBJECT, XD4_COLL, XD4_AI, XD4_PANEL, XD4_PENDING };

/* After each frame of BN4's guest core: BN4_SUBSYSTEM_BYTE watched while its battle is on, which Billy's recipe takes for 0
 * (its test at 0x08044AEC aimed at BN6's routine that returns 0); and once the battle is over, what it read said */
void xd4_watch(void);
void xd4_watched(void);

/* BN4's game state as BN6's sub-mode (BN6_SUB_BATTLE while its main mode is a battle's; its main mode else) */
uint8_t xd4_sub(void);

/* part's bytes `in` (BN4's: from an object's list node, BN6_T1_SIZE; a collision data's BN4_COLL_SIZE; an AI's
 * BN4_AI_DATA_SIZE; a panel's BN4_PANEL_SIZE; MegaMan's pending form, from his AI data) into `out` in BN6's layout
 * (BN6_T1_SIZE, BN6_COLL_SIZE, BN6_AI_DATA_SIZE, BN6_PANEL_SIZE, BN6_CROSS_PENDING_SIZE), the rest of `out` zero; how many
 * bytes `out` holds */
uint32_t xd4_translate(int part, const uint8_t *in, uint8_t *out);
/* ... how many bytes of BN4's a part is (the `in` xd4_translate reads) */
uint32_t xd4_size(int part);
/* A panel's `in` past its BN4_PANEL_SIZE bytes (the panel at `at`): its blink's type and side, the low nibbles of BN4's own
 * tables (BN4_PANEL_BLINKS, BN4_PANEL_BLINK_SIDES), which BN6 keeps in the panel (BN6_PANEL_BLINK_TYPE, BN6_PANEL_BLINK_SIDE) */
void xd4_panel_blinks(uint32_t at, uint8_t *in);

/* Where xdiff.c's trace reads BN4's battle (bn4.h's names): its state, Custom window, RNG words, the three kinds'
 * tables, the panels, the run list, the music players, the battle NaviStats and side, a collision data's paralysis, flags
 * and struck words */
typedef struct {
	uint32_t state, window, rng[2], t1, t3, t4, nt1, panels, list, list_end, mplays, navi, navi_size, side;
	uint32_t paralysis, coll_flags, coll_struck;
	uint32_t ai, ai_size;   /* (MegaMan's AI data, a side's: his pending form, carried in the player block, XD4_PENDING) */
	uint32_t sprites, sprites_size;   /* (the battle's sprite cache, BN4_SPRITE_CACHE: its compressed sprites in RAM, carried in
	                                     the block BN6 keeps apart, xdiff.c apart4_out) */
} Xd4Places;
extern const Xd4Places xd4_places;
/* BN4's routines the differential's hooks are placed on (bn4.h: RNG draws, spawns, sounds, effects, reservations, the blink,
 * sprites; xd_log_bn4) */
extern const XdGame xd4_routines;

#endif
