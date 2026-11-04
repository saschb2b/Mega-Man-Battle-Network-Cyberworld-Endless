/* xdiff_bn4_trace.h: BN4's battle's parts in BN6's units and order. */
#include "xdiff_bn4_trace.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "bn4.h"
#include "bn6.h"
#include "bn6_ai.h"
#include "bn6_forms.h"
#include "bn6_link.h"
#include "bn6_panels.h"
#include "guest.h"

/* a field BN4 keeps where BN6 keeps it otherwise: BN4's offset, BN6's, its bytes (ELSEWHERE: carried in another part) */
typedef struct { uint16_t from, to, len; } Seg;
#define ELSEWHERE 0xFFFF

/* an object's (from its list node: BN6_OBJECT_NODE + the field's offset): its element at BN6's +0x0E, its enemy id (a u8)
 * at NameID's low byte (a u16 in BN6; BN4's bytes at those places move to the ones these leave, xd4_translate) */
static const Seg obj_segs[] = {
	{ BN6_OBJECT_NODE + BN4_T1_ELEMENT, BN6_OBJECT_NODE + BN6_T1_ELEMENT, 1 },
	{ BN6_OBJECT_NODE + BN4_T1_NAME_ID, BN6_OBJECT_NODE + BN6_T1_NAME_ID, 1 },
};
/* a collision data's: its side, its panel, the statuses' timers, the flash and invulnerability, the final damage, its
 * flags (bn4.h, BN4_COLL_*; their BN6 twins bn6.h's); then what the collision setup (BN4 0x080129AC, BN6's twin
 * 0x08019FB4) and the pool's maker (0x08012470, BN6's 0x0801986C) write, store by store: its object's +0x2E, the setup's
 * argument, the damage, the alliance words, its object, its index bit */
static const Seg coll_segs[] = {
	{ BN4_COLL_HIT_KIND, BN6_COLL_HIT_KIND, 1 },
	{ BN4_COLL_HIT_BITS, BN6_COLL_HIT, 1 },
	{ BN4_COLL_STATUS_GIVEN, BN6_COLL_STATUS_GIVEN, 1 },
	{ BN4_COLL_STATUS_TAKEN, BN6_COLL_STATUS_TAKEN, 1 },
	{ BN4_COLL_MOOD_LOSS, BN6_COLL_MOOD_LOSS, 2 },
	{ BN4_COLL_MOOD_GAIN, BN6_COLL_MOOD_GAIN, 2 },
	{ BN4_COLL_HITS_TAKEN, BN6_COLL_HITS_TAKEN, 2 },
	{ BN4_COLL_HITS_WORD, BN6_COLL_HITS_WORD, 4 },
	{ BN4_COLL_MET, BN6_COLL_MET, 4 },
	{ BN4_COLL_HITS_BACK, BN6_COLL_HITS_BACK, 4 },
	{ BN4_COLL_PUSH_TAKEN, BN6_COLL_PUSH_TAKEN, 2 },
	{ BN4_COLL_PUSH, BN6_COLL_PUSH, 2 },
	{ BN4_COLL_PANEL_BYTE, BN6_COLL_PANEL_BYTE, 1 },
	{ BN4_COLL_COUNTER, BN6_COLL_COUNTER, 1 },
	{ BN4_COLL_OBJ_2E, BN6_COLL_OBJ_2E, 1 },
	{ BN4_COLL_SETUP_ARG, BN6_COLL_SETUP_ARG, 1 },
	{ BN4_COLL_DAMAGE, BN6_COLL_DAMAGE, 2 },
	{ BN4_COLL_HIT_MASKS, BN6_COLL_HIT_MASKS, 8 },
	{ BN4_COLL_OBJECT, BN6_COLL_OBJECT, 4 },
	{ BN4_COLL_INDEX_BIT, BN6_COLL_INDEX_BIT, 4 },
	{ BN4_COLL_ALLIANCE, BN6_COLL_ALLIANCE, 1 },
	{ BN4_COLL_PANEL_X, BN6_COLL_PANEL_X, 2 },
	{ BN4_COLL_PARALYSIS, BN6_COLL_PARALYSIS, 2 },
	{ BN4_COLL_CONFUSE, BN6_COLL_CONFUSE, 2 },
	{ BN4_COLL_BLIND, BN6_COLL_BLIND, 2 },
	{ BN4_COLL_HOLD, BN6_COLL_IMMOBILE, 2 },
	{ BN4_COLL_FLASH, BN6_COLL_FLASH, 2 },
	{ BN4_COLL_INVULNERABLE, BN6_COLL_INVULNERABLE, 2 },
	{ BN4_COLL_FINAL_DAMAGE, BN6_COLL_FINAL_DAMAGE, 2 },
	{ BN4_COLL_FLAGS, BN6_COLL_FLAGS, 4 },
	{ BN4_COLL_STRUCK, BN6_COLL_STRUCK, 4 },
};
/* an AI's data: its attack's variables at BN6's +0xA0; MegaMan's power attacks, keys, flags, HP bug's count, damage taken
 * and frames struck, each paired by both games' code (bn4_battle.h, BN4_AI_*) */
static const Seg ai_segs[] = { { BN4_AI_ATTACK_VARS, BN6_AI_ATTACK_VARS, BN4_AI_DATA_SIZE - BN4_AI_ATTACK_VARS },
	{ BN4_AI_POWER_A, BN6_AI_POWER_A, 1 }, { BN4_AI_POWER_CHARGE, BN6_AI_POWER_CHARGE, 1 }, { BN4_AI_POWER_C, BN6_AI_POWER_C, 1 },
	{ BN4_AI_POWER_CHIP, BN6_AI_POWER_CHIP, 1 }, { BN4_AI_KEYS, BN6_AI_HELD, BN4_AI_KEYS_SIZE }, { BN4_AI_MODES, BN6_AI_MODES, 4 },
	{ BN4_AI_HP_BUG, BN6_AI_HP_BUG, 1 }, { BN4_AI_DAMAGE_TAKEN, BN6_AI_DAMAGE_TAKEN, 2 }, { BN4_AI_STRUCK_FRAMES, BN6_AI_STRUCK_FRAMES, 4 },
	{ BN4_AI_PENDING_FORM, ELSEWHERE, 1 }, { BN4_AI_ANGER, BN6_AI_ANGER, 2 },
	{ BN4_AI_CHARGE_TIMES, BN6_AI_SPARE_LAST, 4 } };   /* (BN4's own cache, in the spare's last word: legit.json's bn4 allowed) */
/* MegaMan's pending form, BN6's outside his AI data: BN4's AI data +0x10 at BN6_CROSS_PENDING's +0 (the form, 0xFF none),
 * the record's other bytes BN6's own (docs/devtools/differential.md, MegaMan's AI data and his object) */
static const Seg pending_segs[] = { { BN4_AI_PENDING_FORM, BN6_RECORD_FORM, 1 } };
/* a panel's: its type, side, flags and reserver */
static const Seg panel_segs[] = {
	{ BN4_PANEL_TYPE, BN6_PANEL_TYPE, 1 },
	{ BN4_PANEL_ALLIANCE, BN6_PANEL_ALLIANCE, 1 },
	{ BN4_PANEL_FLAGS, BN6_PANEL_FLAGS, 4 },
	{ BN4_PANEL_RESERVER, BN6_PANEL_RESERVER, 4 },
	{ BN4_PANEL_STATE, BN6_PANEL_STATE, 2 },
	{ BN4_PANEL_TIMER_A, BN6_PANEL_SIDE_TIMER, 2 },
	{ BN4_PANEL_TIMER_B, BN6_PANEL_TIMER, 2 },
};

/* (spare: where BN4's bytes left over begin, BN6's places left taken from there on and then from 0. An AI data's from
 * BN6_AI_SPARE, the 16 bytes past its attack's variables, which BN6's AI take never lays out (bn6_ai.h): so BN6's own fields
 * meet BN4's nothing, never a BN4 byte its own place could not keep. NONE: the part takes its fields alone, the AI data
 * keeping the rest) */
#define NONE 0xFFFFFFFFu
typedef struct { const Seg *segs; int nsegs; uint32_t in, out, spare; } Layout;

static const Layout layouts[] = {
	[XD4_OBJECT] = { obj_segs, 2, BN6_T1_SIZE, BN6_T1_SIZE, 0 },
	[XD4_COLL] = { coll_segs, 31, BN4_COLL_SIZE, BN6_COLL_SIZE, 0 },
	[XD4_AI] = { ai_segs, 13, BN4_AI_DATA_SIZE, BN6_AI_DATA_SIZE, BN6_AI_SPARE },
	[XD4_PANEL] = { panel_segs, 7, BN4_PANEL_SIZE, BN6_PANEL_SIZE, 0 },
	[XD4_PENDING] = { pending_segs, 1, BN4_AI_DATA_SIZE, BN6_CROSS_PENDING_SIZE, NONE },
};
_Static_assert(sizeof obj_segs / sizeof *obj_segs == 2 && sizeof coll_segs / sizeof *coll_segs == 31 &&
	sizeof ai_segs / sizeof *ai_segs == 13 && sizeof panel_segs / sizeof *panel_segs == 7, "the layouts' counts");

#define ROOM 0x100   /* the largest part, BN6's AI data */

uint32_t xd4_translate(int part, const uint8_t *in, uint8_t *out) {
	const Layout *l = &layouts[part];
	bool moved[ROOM] = { false }, taken[ROOM] = { false };
	memset(out, 0, l->out);
	/* (the fields mapped, at BN6's places) */
	for (int k = 0; k < l->nsegs; ++k)
		for (uint32_t i = 0; i < l->segs[k].len; ++i) {
			moved[l->segs[k].from + i] = true;
			if (l->segs[k].to == ELSEWHERE) continue;
			out[l->segs[k].to + i] = in[l->segs[k].from + i];
			taken[l->segs[k].to + i] = true;
		}
	if (l->spare == NONE) return l->out;
	/* (BN4's other bytes at their own offsets where BN6's are free there; those left, into the places left, in order from
	 * the layout's spare) */
	uint32_t tried = 0;
	for (int pass = 0; pass < 2; ++pass)
		for (uint32_t i = 0; i < l->in; ++i) {
			if (moved[i]) continue;
			uint32_t at = i;
			if (pass == 1) {
				while (tried < l->out && taken[(l->spare + tried) % l->out]) ++tried;
				if (tried == l->out) break;
				at = (l->spare + tried) % l->out;
			} else if (i >= l->out || taken[i]) continue;
			out[at] = in[i];
			moved[i] = taken[at] = true;
		}
	return l->out;
}

uint32_t xd4_size(int part) { return layouts[part].in; }

const Xd4Places xd4_places = { BN4_BATTLE_STATE, BN4_CUSTOM_WINDOW, { BN4_RNG_PRIMARY, BN4_RNG_SECONDARY }, BN4_T1_OBJECTS, BN4_T3_OBJECTS,
	BN4_T4_OBJECTS, BN4_T1_COUNT, BN4_FIELD_PANELS, BN4_OBJECT_LIST, BN4_OBJECT_LIST_END, BN4_MPLAYS, BN4_BATTLE_NAVI, BN4_NAVI_SIZE,
	BN4_BATTLE_SIDE, BN4_COLL_PARALYSIS, BN4_COLL_FLAGS, BN4_COLL_STRUCK, BN4_AI_DATA, BN4_AI_DATA_SIZE, BN4_SPRITE_CACHE,
	BN4_SPRITE_CACHE_SIZE };

/* (bn4.h's names, each BN6's twin by its body: docs/rom/bn4.md, The differential's hooks. BN4's viruses keep no AI data, so
 * its setAttack, which writes an AI data's attack variables in BN5 and BN6, is not hooked; its blink hides on its flash
 * timer's bit 1 clear, as BN5's; no mood) */
const XdGame xd4_routines = { { BN4_RNG_DRAW, BN4_RNG_DRAW_POSITIVE, BN4_RNG_DRAW_SECOND, BN4_RNG_DRAW_SECOND_POSITIVE }, BN4_SPAWN_COMMON,
	BN4_SPAWN_COMMON_END, BN4_PLAY_SOUND, BN4_SPAWN_EFFECT, 0, BN4_RESERVE_PANEL_DONE, BN4_BLINK_FLASHING, BN4_SPRITE_LOAD, BN4_T1_OBJECTS,
	BN4_OBJECT_RUNNER_RET, BN4_OBJECT_RUNNING, { BN4_RNG_PRIMARY, BN4_RNG_SECONDARY }, BN4_COLL_FLASH, 0, 0, BN4_BATTLE_NAVI, BN4_NAVI_SIZE,
	0, 0 };

/* (BN4_SUBSYSTEM_BYTE: the frames it read nonzero, the first's timer) */
static uint32_t watch_frames, watch_first, watch_value;

void xd4_watch(void) {
	uint8_t b = guest_read8(BN4_SUBSYSTEM_BYTE);
	if (!b || xd4_sub() != BN6_SUB_BATTLE) return;
	if (!watch_frames++) watch_first = guest_read32(BN4_BATTLE_TIMER), watch_value = b;
}

void xd4_watched(void) {
	if (!watch_frames) printf("xdiff: BN4's %08x read 0 at every frame of its battle (Billy's recipe aims 0x08044AEC's test of it at "
		"BN6's routine that returns 0)\n", (unsigned)BN4_SUBSYSTEM_BYTE);
	else printf("xdiff: BN4's %08x read nonzero at %u frames of its battle, first %#x at t%u: Billy's recipe's stub returning 0 is not "
		"BN4's path there\n", (unsigned)BN4_SUBSYSTEM_BYTE, (unsigned)watch_frames, (unsigned)watch_value, (unsigned)watch_first);
}

uint8_t xd4_sub(void) {
	uint8_t mode = guest_read8(guest_read32(BN4_TOOLKIT));
	return mode == BN4_MODE_BATTLE ? BN6_SUB_BATTLE : mode;
}
