/* xdiff_bn4_trace.h: BN4's battle's parts in BN6's units and order. */
#include "xdiff_bn4_trace.h"

#include <stdbool.h>
#include <string.h>

#include "bn4.h"
#include "bn6.h"
#include "bn6_forms.h"
#include "bn6_panels.h"
#include "guest.h"

/* a field BN4 keeps where BN6 keeps it otherwise: BN4's offset, BN6's, its bytes */
typedef struct { uint16_t from, to, len; } Seg;

/* an object's (from its list node: BN6_OBJECT_NODE + the field's offset): its element at BN6's +0x0E, its enemy id (a u8)
 * at NameID's low byte (a u16 in BN6; BN4's bytes at those places move to the ones these leave, xd4_translate) */
static const Seg obj_segs[] = {
	{ BN6_OBJECT_NODE + BN4_T1_ELEMENT, BN6_OBJECT_NODE + BN6_T1_ELEMENT, 1 },
	{ BN6_OBJECT_NODE + BN4_T1_NAME_ID, BN6_OBJECT_NODE + BN6_T1_NAME_ID, 1 },
};
/* a collision data's: its side, its panel, the statuses' timers, the flash and invulnerability, the final damage, its
 * flags (bn4.h, BN4_COLL_*; their BN6 twins bn6.h's) */
static const Seg coll_segs[] = {
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
/* an AI's data: its attack's variables at BN6's +0xA0 */
static const Seg ai_segs[] = { { BN4_AI_ATTACK_VARS, BN6_AI_ATTACK_VARS, BN4_AI_DATA_SIZE - BN4_AI_ATTACK_VARS } };
/* a panel's: its type, side, flags and reserver */
static const Seg panel_segs[] = {
	{ BN4_PANEL_TYPE, BN6_PANEL_TYPE, 1 },
	{ BN4_PANEL_ALLIANCE, BN6_PANEL_ALLIANCE, 1 },
	{ BN4_PANEL_FLAGS, BN6_PANEL_FLAGS, 4 },
	{ BN4_PANEL_RESERVER, BN6_PANEL_RESERVER, 4 },
};

typedef struct { const Seg *segs; int nsegs; uint32_t in, out; } Layout;

static const Layout layouts[] = {
	[XD4_OBJECT] = { obj_segs, 2, BN6_T1_SIZE, BN6_T1_SIZE },
	[XD4_COLL] = { coll_segs, 11, BN4_COLL_SIZE, BN6_COLL_SIZE },
	[XD4_AI] = { ai_segs, 1, BN4_AI_DATA_SIZE, BN6_AI_DATA_SIZE },
	[XD4_PANEL] = { panel_segs, 4, BN4_PANEL_SIZE, BN6_PANEL_SIZE },
};
_Static_assert(sizeof obj_segs / sizeof *obj_segs == 2 && sizeof coll_segs / sizeof *coll_segs == 11 &&
	sizeof panel_segs / sizeof *panel_segs == 4, "the layouts' counts");

#define ROOM 0x100   /* the largest part, BN6's AI data */

uint32_t xd4_translate(int part, const uint8_t *in, uint8_t *out) {
	const Layout *l = &layouts[part];
	bool moved[ROOM] = { false }, taken[ROOM] = { false };
	memset(out, 0, l->out);
	/* (the fields mapped, at BN6's places) */
	for (int k = 0; k < l->nsegs; ++k)
		for (uint32_t i = 0; i < l->segs[k].len; ++i) {
			out[l->segs[k].to + i] = in[l->segs[k].from + i];
			moved[l->segs[k].from + i] = taken[l->segs[k].to + i] = true;
		}
	/* (BN4's other bytes at their own offsets where BN6's are free there; those left, into the places left, in order) */
	uint32_t spare = 0;
	for (int pass = 0; pass < 2; ++pass)
		for (uint32_t i = 0; i < l->in; ++i) {
			if (moved[i]) continue;
			uint32_t at = i;
			if (pass == 1) {
				while (spare < l->out && taken[spare]) ++spare;
				if (spare == l->out) break;
				at = spare;
			} else if (i >= l->out || taken[i]) continue;
			out[at] = in[i];
			moved[i] = taken[at] = true;
		}
	return l->out;
}

uint32_t xd4_size(int part) { return layouts[part].in; }

const Xd4Places xd4_places = { BN4_BATTLE_STATE, BN4_CUSTOM_WINDOW, { BN4_RNG_PRIMARY, BN4_RNG_SECONDARY }, BN4_T1_OBJECTS, BN4_T3_OBJECTS,
	BN4_T4_OBJECTS, BN4_T1_COUNT, BN4_FIELD_PANELS, BN4_OBJECT_LIST, BN4_OBJECT_LIST_END, BN4_MPLAYS, BN4_BATTLE_NAVI, BN4_NAVI_SIZE,
	BN4_BATTLE_SIDE, BN4_COLL_PARALYSIS, BN4_COLL_FLAGS, BN4_COLL_STRUCK };

uint8_t xd4_sub(void) {
	uint8_t mode = guest_read8(guest_read32(BN4_TOOLKIT));
	return mode == BN4_MODE_BATTLE ? BN6_SUB_BATTLE : mode;
}
