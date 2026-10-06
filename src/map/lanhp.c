/* lanhp.h. BN6's own Lan's HP (map 0x88:00) is a little hub already: its
 * blue pad, where the jack-in from Lan's PC lands (destination 1) and
 * whose warp jacks MegaMan out (warp entry 1, departure 0x10), its pink pad
 * to Central Area 1 (2) and four link squares on its floor to the
 * Aquarium, Green, Sky and ACDC HPs (3 to 6; docs/ROM_DATA.md, Lan's HP).
 * The run's portals are the pink pad and the links; where each leads is
 * the director's. The blue pad stays BN6's way out. */
#include "lanhp.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "bn6.h"
#include "emu.h"
#include "flags.h"
#include "mapslot.h"
#include "npc.h"

#define SONG 0x13          /* BN6's for every homepage, Lan's too */
#define ARRIVAL_ENTRY 1    /* the blue pad */
#define ENTRIES 6
/* A link square's marker (map objects 0xCB-0xCE, sprite list 7's 0x88)
 * shows while its story flag is set, else it turns its link's warp off:
 * flag LINK_OPEN + n for warp entry 3 + n (docs/ROM_DATA.md, Lan's HP) */
#define FIRST_LINK 3
#define LINK_OPEN 0x017D

/* the warp entry (its trigger cells' value) of each portal: the pink pad,
 * then the floor's link squares, nearest the pink pad's side first */
static const int portal_entry[LANHP_PORTALS] = { 2, 5, 4, 3, 6 };

/* The courier (docs/HOME.md, piece 2): a Mr.Prog (sprite list 6's 60) on
 * the floor below the way from the blue pad to the corridor (y -14), 50
 * units and more from every link square, facing the blue pad (-x), BN6's
 * "!!" burst (list 5's 0x07, the counter's sign in AsterLand) over his
 * head; his words an archive of their own, which the director writes */
#define COURIER_X (-84)
#define COURIER_Y 38
#define COURIER_FACE 5
#define COURIER_SPRITE 60
#define MARK_SPRITE 0x07
#define MARK_Z 46
#define COURIER_TEXT 768   /* the bytes his words may take */

static struct {
	bool read;
	int x[ENTRIES + 1], y[ENTRIES + 1];   /* each entry's spot: its cells' middle */
	int x0[ENTRIES + 1], y0[ENTRIES + 1], x1[ENTRIES + 1], y1[ENTRIES + 1];   /* ... and their bounds */
	int arrive_x, arrive_y;
	uint32_t warps;                       /* the HP's own warp list */
	uint8_t jack_out[16];                 /* BN6's blue pad's entry: its jack-out */
	uint32_t courier_text;                /* the courier's words' room, 0 none */
} H;

static bool read_spots(void) {
	if (H.read) return true;
	int g, n;
	if (!mapslot_jack_record(1, &g, &n, &H.arrive_x, &H.arrive_y) || g != LANHP_GROUP || n != LANHP_NUMBER) return false;
	AreaSrc a;
	if (!area_src_load(LANHP_GROUP, LANHP_NUMBER, &a)) return false;
	int count[ENTRIES + 1] = { 0 };
	long sx[ENTRIES + 1] = { 0 }, sy[ENTRIES + 1] = { 0 };
	for (int i = 0; i < a.nsec[3]; ++i) {
		const CoordCell *c = &a.sec[3][i];
		int v = c->value;
		if (v < 1 || v > ENTRIES) continue;
		if (!count[v]++) { H.x0[v] = H.x1[v] = c->x; H.y0[v] = H.y1[v] = c->y; }
		if (c->x < H.x0[v]) H.x0[v] = c->x;
		if (c->y < H.y0[v]) H.y0[v] = c->y;
		if (c->x > H.x1[v]) H.x1[v] = c->x;
		if (c->y > H.y1[v]) H.y1[v] = c->y;
		sx[v] += c->x + 4;
		sy[v] += c->y + 4;
	}
	area_src_free(&a);
	/* (the blue pad's warp as BN6 has it, before the HP is taken over) */
	uint32_t list = mapslot_warps(LANHP_GROUP, LANHP_NUMBER);
	if (!list) return false;
	for (int i = 0; i < 16; ++i) H.jack_out[i] = emu_read8(list + (uint32_t)i);
	for (int v = 1; v <= ENTRIES; ++v) {
		if (!count[v]) return false;
		H.x[v] = (int)(sx[v] / count[v]);
		H.y[v] = (int)(sy[v] / count[v]);
	}
	H.read = true;
	return true;
}

/* the courier and his mark into `npcs`, both gone while
 * LANHP_COURIER_GONE_FLAG is set; his words' room cleared (an archive of
 * one empty script) */
static void courier(NpcList *npcs) {
	static const uint8_t empty[COURIER_TEXT] = { 2, 0, 0xE6 };   /* (one script: ts_end) */
	H.courier_text = mapslot_alloc(empty, sizeof empty);
	if (!H.courier_text || !npc_need_sprite(npcs, 6, COURIER_SPRITE) || !npc_need_sprite(npcs, 5, MARK_SPRITE)) return;
	uint32_t prog = npc_talker(6, COURIER_SPRITE, COURIER_X, COURIER_Y, 0, COURIER_FACE, H.courier_text, 0, LANHP_COURIER_GONE_FLAG, false);
	/* (a step before him in depth, drawn over him where they meet) */
	uint32_t mark = npc_mark(5, MARK_SPRITE, COURIER_X - 1, COURIER_Y + 1, MARK_Z, 0, LANHP_COURIER_GONE_FLAG);
	if (prog) npcs->script[npcs->n++] = prog;
	if (prog && mark) npcs->script[npcs->n++] = mark;
}

bool lanhp_install(void) {
	if (!read_spots()) return false;
	NpcList npcs;
	memset(&npcs, 0, sizeof npcs);
	/* (its own decorations, BN6's four objects under the floor) */
	npcs.objects = mapslot_objects(LANHP_GROUP, LANHP_NUMBER);
	mapslot_hp(true);
	courier(&npcs);
	bool ok = mapslot_install(LANHP_GROUP, LANHP_NUMBER, &npcs, NULL, 0) &&
		(H.warps = mapslot_own_warps(LANHP_GROUP, LANHP_NUMBER, H.arrive_x, H.arrive_y, 1)) != 0 &&
		mapslot_music_home(0, LANHP_GROUP, LANHP_NUMBER, SONG);
	mapslot_hp(false);
	if (ok) emu_write(H.warps + 16u * (ARRIVAL_ENTRY - 1), H.jack_out, sizeof H.jack_out);
	return ok;
}

bool lanhp_courier_say(const uint8_t *archive, int n) {
	if (!H.courier_text || n <= 0 || n > COURIER_TEXT) return false;
	emu_write(H.courier_text, archive, (size_t)n);
	return true;
}

void lanhp_arrival(int *x, int *y) {
	if (!read_spots()) { *x = *y = 0; return; }
	*x = H.arrive_x;
	*y = H.arrive_y;
}

void lanhp_portal(int k, int group, int number, int x, int y, int facing) {
	if (k < 0 || k >= LANHP_PORTALS) return;
	mapslot_warp(H.warps, portal_entry[k], group, number, x, y, facing);
}

int lanhp_portal_of(int entry) {
	for (int k = 0; k < LANHP_PORTALS; ++k) if (portal_entry[k] == entry) return k;
	return -1;
}

void lanhp_lit(unsigned lit) {
	for (int k = 0; k < LANHP_PORTALS; ++k) {
		int e = portal_entry[k];
		bool on = lit >> k & 1;
		if (on) flag_clear(BN6_FLAG_WARP_OFF + e);
		else flag_set(BN6_FLAG_WARP_OFF + e);
		if (e < FIRST_LINK) continue;
		if (on) flag_set(LINK_OPEN + e - FIRST_LINK);
		else flag_clear(LINK_OPEN + e - FIRST_LINK);
	}
}

void lanhp_portal_spot(int k, int *x, int *y) {
	if (k < 0 || k >= LANHP_PORTALS || !read_spots()) { *x = *y = 0; return; }
	*x = H.x[portal_entry[k]];
	*y = H.y[portal_entry[k]];
}

int lanhp_portal_near(int x, int y, int reach) {
	if (!read_spots()) return -1;
	for (int k = 0; k < LANHP_PORTALS; ++k) {
		int e = portal_entry[k];
		if (x >= H.x0[e] - reach && y >= H.y0[e] - reach && x < H.x1[e] + 8 + reach && y < H.y1[e] + 8 + reach) return k;
	}
	return -1;
}
