/* xnavi_room.h: BN6's Navi tables grown in the core's ROM copy, as one step
 * of staged writes (xvirus_tx.h). */
#include "xnavi_room.h"

#include <stdio.h>
#include <string.h>

#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "xnavi_recipe.h"
#include "xvirus.h"
#include "xvirus_grow.h"
#include "xvirus_tx.h"

#define BUS      0x08000000u
#define TEXT_END 0xE6   /* a text archive's entry ends with it */
#define NAME_MAX 32     /* a name's bytes at most, its end among them */

/* One of BN6's tables as it grows: the word naming it, its entries and
 * their size, where it grows to and to how many; the entries past BN6's
 * a copy of its entry 0 (AI 0's, a story MegaMan's: the Navis' families'
 * room keeps AI 0's in the rest, as the viruses' does) */
typedef struct { uint32_t ref, count, size, to, cap; } Room;

static Room T[XN_GROWN];
_Static_assert(XN_OVL_SPRITES + 2 * XN_KINDS <= XN_NAMES, "the overlay's grown kinds meet the names");

static void specs(void) {
	static const uint32_t refs[BN6_NAVI_TABLE_COUNT] = {
		BN6_NAVI_REF_STATES, BN6_NAVI_REF_PRESTEP, BN6_NAVI_REF_HOOK_A, BN6_NAVI_REF_STRUCT1, BN6_NAVI_REF_STRUCT2, BN6_NAVI_REF_POST,
		BN6_NAVI_REF_SPAWN, BN6_NAVI_REF_SPAWNED, BN6_NAVI_REF_PARALYSIS, BN6_NAVI_REF_FREEZE, BN6_NAVI_REF_BUBBLE, BN6_NAVI_REF_FLINCH,
		BN6_NAVI_REF_DRAG,
	};
	for (uint32_t k = 0; k < BN6_NAVI_TABLE_COUNT; ++k) T[XN_TABLE + k] = (Room){ refs[k], BN6_NAVIS, 4, XN_TABLES + k * XN_STEP, XN_AIS };
	T[XN_T_ICE] = (Room){ BN6_NAVI_ICE_REF, BN6_NAVIS, 1, XN_ICE, XN_AIS };
	T[XN_T_HELD] = (Room){ BN6_NAVI_HELD_REF, BN6_NAVIS, 4, XN_HELD, XN_AIS };
	T[XN_T_COMPANION] = (Room){ BN6_NAVI_COMPANION_REF, BN6_NAVIS, 4, XN_COMPANION, XN_AIS };
	T[XN_T_COMPANION_END] = (Room){ BN6_NAVI_COMPANION_END_REF, BN6_NAVIS, 4, XN_COMPANION_END, XN_AIS };
	T[XN_T_OVL_MASKS] = (Room){ BN6_OVERLAY_MASKS_REF, BN6_OVERLAY_KINDS, 4, XN_OVL_MASKS, XN_KINDS };
	T[XN_T_OVL_SPRITES] = (Room){ BN6_OVERLAY_SPRITES_REF, BN6_OVERLAY_KINDS, 2, XN_OVL_SPRITES, XN_KINDS };
}

static bool in_rom(uint32_t a, uint32_t n) { return a >= BUS && a - BUS <= ROM_SIZE && n <= ROM_SIZE - (a - BUS); }

/* the table BN6's word names, as the player's ROM has it and the core's
 * copy still does, staged at its grown place with the fill after it */
static void grow_one(const Room *t) {
	uint32_t from = get32(R.data + (t->ref - BUS)), len = t->count * t->size;
	if (!in_rom(from, len)) { xv_fail("a Navi table at %08x is out of the ROM", (unsigned)from); return; }
	uint8_t *p = xv_put(t->to, t->size * t->cap, true);
	for (uint32_t b = 0; p && b < len && !xv_failed(); ++b) {
		p[b] = emu_read8(from + b);
		if (p[b] != R.data[from - BUS + b]) xv_fail("BN6's Navi table at %08x was changed (%08x)", (unsigned)from, (unsigned)(from + b));
	}
	for (uint32_t i = t->count; p && i < t->cap; ++i) memcpy(p + i * t->size, p, t->size);
	if (!xv_failed()) xv_word(t->ref, from, t->to);
}

/* ---- the names: an archive of the moved Navis' own, by an id's low byte ---- */

/* every entry the empty name, after the offsets */
static void names_make(void) {
	uint8_t *p = xv_put(XN_NAMES, XN_NAMES_ROOM, true);
	if (!p) return;
	memset(p, 0xFF, XN_NAMES_ROOM);
	for (uint32_t i = 0; i < XN_NAMES_MAX; ++i) put16(p + 2 * i, (uint16_t)(2u * XN_NAMES_MAX));
	p[2 * XN_NAMES_MAX] = TEXT_END;
}

/* BN6_NAME_OF past its choice of archive (r2, the Navis' for an id past
 * 0xFF; r0 the id): a moved Navi's id (XN_FIRST_ID to XV_FIRST_ID) reads
 * the moved Navis' archive, BN6's own Navis still BN6's */
static HookAct name_of(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] >= XN_FIRST_ID && r->r[0] < XN_IDS_END) r->r[2] = XN_NAMES;
	return HOOK_CONTINUE;
}

/* the empty name's offset: what every entry holds until a Navi names it */
static uint32_t name_none(void) { return 2u * XN_NAMES_MAX; }

uint32_t xn_names_end(void) {
	uint32_t last = name_none();
	for (uint32_t i = 0; i < XN_NAMES_MAX; ++i) {
		uint32_t off = emu_read16(XN_NAMES + 2 * i);
		if (off > last && off < XN_NAMES_ROOM) last = off;
	}
	uint32_t n = 0;
	while (n < NAME_MAX && last + n < XN_NAMES_ROOM && emu_read8(XN_NAMES + last + n) != TEXT_END) ++n;
	return last + n + 1;
}

bool xn_name(uint32_t i, uint32_t off, const uint8_t *text, uint32_t len) {
	if (i >= XN_NAMES_MAX || !len || off + len > XN_NAMES_ROOM) return xv_fail("a Navi's name %#x", (unsigned)i);
	uint8_t *p = xv_put(XN_NAMES + off, len, true);
	if (p) memcpy(p, text, len);
	xv_half(XN_NAMES + 2 * i, (uint16_t)name_none(), (uint16_t)off);
	return !xv_failed();
}

/* ---- the stubs a Navi's calls to what BN6's engine lacks go to (xnavi_recipe.h) ---- */

#define MOVS_R0_0 0x2000u   /* movs r0, #0 */
#define BX_LR     0x4770u   /* bx lr */

static void stubs_make(void) {
	uint8_t *p = xv_put(XN_STUBS, XN_STUBS_END - XN_STUBS, true);
	if (!p) return;
	memset(p, 0xFF, XN_STUBS_END - XN_STUBS);
	put16(p + (XN_STUB_ZERO - XN_STUBS), MOVS_R0_0);
	put16(p + (XN_STUB_ZERO - XN_STUBS) + 2, BX_LR);
	put16(p + (XN_STUB_DS - XN_STUBS), BX_LR);
}

/* The DS version's chip AI reached: no version a recipe installs (V1, V2,
 * V3, SP) reaches it in BN5's code; told, then returned from */
static HookAct ds_reached(HookRegs *r, void *user) {
	(void)user;
	static int told;
	if (told++ < 8) fprintf(stderr, "xnavi: a DS version's chip AI reached from %08x (not installed: versions 0-3 only)\n", (unsigned)r->lr);
	return HOOK_RETURN;
}

/* ---- grown ---- */

static bool grown;

bool xn_grow(void) {
	if (grown) return true;
	if (!R.data || !R.layout || !emu_ready() || !xg_grow()) return false;
	specs();
	xv_tx_begin();
	for (int t = 0; t < XN_GROWN && !xv_failed(); ++t) grow_one(&T[t]);
	if (!xv_failed()) names_make();
	if (!xv_failed()) stubs_make();
	if (xv_failed()) {
		fprintf(stderr, "xnavi: BN6's Navi tables not grown, no Navi installed: %s\n", xv_why());
		xv_tx_end();
		return false;
	}
	uint32_t used = xv_used();
	claims_begin(XN_CLAIMS);
	xv_commit();
	if (!emu_hook(XN_STUB_DS, ds_reached, NULL)) fprintf(stderr, "xnavi: no hook at the DS chip AI's stub\n");
	if (!emu_hook(BN6_NAME_OF_NAVIS, name_of, NULL)) fprintf(stderr, "xnavi: no hook at %08x: the moved Navis nameless\n", (unsigned)BN6_NAME_OF_NAVIS);
	claims_end();
	xv_tx_end();
	for (int t = 0; t < XN_GROWN; ++t) xv_moved_add(get32(R.data + (T[t].ref - BUS)), T[t].to, T[t].count * T[t].size);
	grown = true;
	printf("xnavi: BN6's Navi tables grown (AI indexes to %d, overlay kinds to %d, the names rebuilt; %u bytes)\n", XN_AIS, XN_KINDS, (unsigned)used);
	return true;
}

/* where entry i of grown table t is, the table's fill still there; 0 (the step failed) where not */
static uint32_t entry_at(int t, uint32_t i) {
	if (t < 0 || t >= XN_GROWN || !grown || i >= T[t].cap) { xv_fail("entry %#x of grown Navi table %d", (unsigned)i, t); return 0; }
	const uint32_t size = T[t].size, to = T[t].to, at = to + i * size;
	for (uint32_t b = 0; b < size; ++b)
		if (emu_read8(at + b) != emu_read8(to + b)) { xv_fail("a grown Navi table's entry %#x is taken (%08x)", (unsigned)i, (unsigned)at); return 0; }
	return at;
}

void xn_set(int t, uint32_t i, const uint8_t *entry) {
	uint32_t at = entry_at(t, i);
	uint8_t *p = at ? xv_put(at, T[t].size, false) : NULL;
	if (p) memcpy(p, entry, T[t].size);
}

void xn_set_word(int t, uint32_t i, uint32_t v) {
	if (t >= 0 && t < XN_GROWN && T[t].size != 4) { xv_fail("grown Navi table %d is no table of words", t); return; }
	uint32_t at = entry_at(t, i);
	uint8_t *p = at ? xv_put(at, 4, false) : NULL;
	if (p) put32(p, v);
}

const char *xn_writer(uint32_t at) { return at >= FREE_VIRUS_ROOM && at < FREE_FAMILIES ? XV_CLAIMS : XN_CLAIMS; }
