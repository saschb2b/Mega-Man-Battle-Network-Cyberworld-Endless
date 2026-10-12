/* xvirus_grow.h: BN6's tables grown in the core's ROM copy, as one step of
 * staged writes (xvirus_tx.h), and the hooks that keep the families' AI
 * indexes and ids to what BN6 has room for. */
#include "xvirus_grow.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "data.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "xvirus.h"
#include "xvirus_tx.h"

#define BUS          0x08000000u
#define TEXT_END     0xE6   /* a text archive's entry ends with it */
#define NAME_MAX     32     /* a name's bytes at most, its end among them */
#define ENTRY_MAX    (2u * ENEMY_DROPS)   /* the largest entry, a reward row's */
#define ID_HIGH_BITS 4      /* the bits of an id's high byte the spawn reads (0x080073F4), its preload made to read as many */

/* One of BN6's tables as it grows: what it is (for the log), the one word
 * naming it, its entries and their size, where it grows to and to how many,
 * and what the entries past BN6's hold: a copy of BN6's entry `fill_of`,
 * or the word `fill` where that is -1 */
typedef struct {
	const char *name;
	uint32_t ref, count, size, to, cap;
	int fill_of;
	uint32_t fill;
} Grow;

static Grow G[XG_TABLES];

/* the table above, from the player's ROM's layout (the two per-family
 * tables RomLayout names, the sprite lists' words) */
static void specs(void) {
	static const uint32_t family_refs[XV_TABLES] = {
		BN6_FAMILY_REF_STATES, 0, 0, BN6_FAMILY_REF_FRAME, BN6_FAMILY_REF_SPAWN, BN6_FAMILY_REF_SPAWNED, BN6_FAMILY_REF_PARALYSIS,
		BN6_FAMILY_REF_FREEZE, BN6_FAMILY_REF_BUBBLE, BN6_FAMILY_REF_FLINCH, BN6_FAMILY_REF_DRAG,
	};
	for (uint32_t k = 0; k < XV_TABLES; ++k) {
		uint32_t ref = k == 1 ? BUS + R.layout->enemy_traits : k == 2 ? BUS + R.layout->enemy_stats : family_refs[k];
		G[XG_FAMILY + k] = (Grow){ "a per-family table", ref, BN6_FAMILIES, 4, XV_GROWN + k * XV_STEP, XV_AIS, 0, 0 };
	}
	/* (the ice blocks' sizes as BN6 reads them for its 32 AI indexes: AI 30's and 31's past its viruses' 30, in the Navis') */
	G[XG_ICE] = (Grow){ "the ice blocks' sizes", BN6_ICE_SIZES_REF, BN6_FAMILIES, 1, XV_ICE, XV_AIS, 0, 0 };
	G[XG_AI_HOOKS] = (Grow){ "a table of hooks by AI index", BN6_AI_HOOKS_REF, BN6_HELD_AIS, 4, XV_PER_AI, XV_AIS, 0, 0 };
	G[XG_AI_HOOKS2] = (Grow){ "a table of hooks by AI index", BN6_AI_HOOKS2_REF, BN6_HELD_AIS, 4, XV_PER_AI + XV_STEP, XV_AIS, 0, 0 };
	G[XG_HELD] = (Grow){ "the held chips", BN6_HELD_CHIPS_REF, BN6_HELD_AIS, 4, XV_PER_AI + 2 * XV_STEP, XV_AIS, 0, 0 };
	static const uint32_t lists[XV_LISTS_GROWN][2] = {
		{ BN6_SPRITES_VIRUSES, BN6_SPRITES_VIRUSES_COUNT }, { BN6_SPRITES_NAVIS, BN6_SPRITES_NAVIS_COUNT },
		{ BN6_SPRITES_ATTACKS, BN6_SPRITES_ATTACKS_COUNT }, { BN6_SPRITES_LIST5, BN6_SPRITES_LIST5_COUNT },
	};
	for (uint32_t i = 0; i < XV_LISTS_GROWN; ++i)
		G[XG_LIST + i] = (Grow){ "a sprite list", BUS + R.layout->sprite_lists + 4 * lists[i][0], lists[i][1], 4, XV_LISTS + i * XV_STEP,
			XV_LIST_MAX, -1, BN6_SPRITE_PLACEHOLDER };
	G[XG_EFFECTS] = (Grow){ "the effects", BN6_EFFECTS_REF, BN6_EFFECT_COUNT, 4, XV_EFFECTS, XV_EFFECTS_MAX, 0, 0 };
	G[XG_T1] = (Grow){ "the first kind's types", BN6_T1_TYPES_REF, BN6_TYPES_T1_COUNT, 4, XV_TYPES, XV_TYPES_MAX, -1, 0 };
	G[XG_T3] = (Grow){ "the third kind's types", BN6_T3_TYPES_REF, BN6_TYPES_T3_COUNT, 4, XV_TYPES + XV_STEP, XV_TYPES_MAX, -1, 0 };
	G[XG_T4] = (Grow){ "the fourth kind's types", BN6_T4_TYPES_REF, BN6_TYPES_T4_COUNT, 4, XV_TYPES + 2 * XV_STEP, XV_TYPES_MAX, -1, 0 };
	G[XG_IDS] = (Grow){ "the enemy ids", BN6_ENEMY_IDS_REF, BN6_ENEMY_IDS_COUNT, 3, XV_IDS_AT, XV_IDS, BN6_FILLER_ID, 0 };
	G[XG_ROWS] = (Grow){ "the reward rows", BN6_DROP_ROWS_REF, BN6_DROP_ROW_COUNT, ENTRY_MAX, XV_ROWS, XV_IDS, BN6_FILLER_ID, 0 };
}

static bool in_rom(uint32_t a, uint32_t n) { return a >= BUS && a - BUS <= ROM_SIZE && n <= ROM_SIZE - (a - BUS); }

/* BN6's table, as the player's ROM has it and the core's copy still does,
 * staged at its grown place with the fill after it; the word naming it
 * pointed there */
static void grow_one(const Grow *g) {
	uint32_t from = get32(R.data + (g->ref - BUS)), len = g->count * g->size;
	if (!in_rom(from, len)) { xv_fail("%s at %08x is out of the ROM", g->name, (unsigned)from); return; }
	uint8_t *t = xv_put(g->to, g->size * g->cap, true);
	if (!t) return;
	for (uint32_t b = 0; b < len; ++b) {
		t[b] = emu_read8(from + b);
		if (t[b] != R.data[from - BUS + b]) { xv_fail("BN6's %s at %08x was changed (%08x)", g->name, (unsigned)from, (unsigned)(from + b)); return; }
	}
	for (uint32_t i = g->count; i < g->cap; ++i) {
		if (g->fill_of >= 0) memcpy(t + i * g->size, t + (uint32_t)g->fill_of * g->size, g->size);
		else put32(t + i * g->size, g->fill);
	}
	xv_word(g->ref, from, g->to);
}

/* The names of the ids from XV_FIRST_ID: every entry the empty name after
 * the offsets */
static void names_make(void) {
	uint8_t *p = xv_put(XV_NAMES, XG_NAME_NONE + 1, true);
	if (!p) return;
	for (uint32_t i = 0; i < XV_IDS - XV_FIRST_ID; ++i) put16(p + 2 * i, (uint16_t)XG_NAME_NONE);
	p[XG_NAME_NONE] = TEXT_END;
}

/* ---- the hooks: an id's name, and the delete counts ---- */

/* BN6_NAME_OF for an id from XV_FIRST_ID: the families' names, at its
 * place among them (BN6 takes an id past 0xFF for a Navi's, by its low
 * byte) */
static HookAct name_of(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] < XV_FIRST_ID || r->r[0] >= XV_IDS) return HOOK_CONTINUE;
	r->r[1] = r->r[0] - XV_FIRST_ID;
	r->r[0] = XV_NAMES;
	return HOOK_RETURN;
}

/* BN6's delete count is 32 bytes, one per AI index, and a rare version's
 * AI index names a chat after the results (BN6_DELETE_RARE): none for a
 * family's, from 32 */
static HookAct counted(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] < BN6_FAMILIES) return HOOK_CONTINUE;
	r->r[0] = 0;   /* (the read's answer: none) */
	return HOOK_RETURN;
}

static void hooks(void) {
	static const uint32_t at[3] = { BN6_DELETE_COUNT, BN6_DELETE_COUNT_READ, BN6_DELETE_RARE };
	for (int i = 0; i < 3; ++i)
		if (!emu_hook(at[i], counted, NULL)) fprintf(stderr, "xvirus: no hook at %08x: a moved family's deletion counts past BN6's\n", (unsigned)at[i]);
	if (!emu_hook(BN6_NAME_OF, name_of, NULL)) fprintf(stderr, "xvirus: no hook at %08x: the families' ids nameless\n", (unsigned)BN6_NAME_OF);
}

/* ---- grown ---- */

static bool grown;   /* (set once committed: what xvirus_drop_row reads, from a hook too) */

bool xg_grow(void) {
	if (grown) return true;
	if (!R.data || !R.layout || !emu_ready()) return false;
	specs();
	xv_tx_begin();
	for (int t = 0; t < XG_TABLES && !xv_failed(); ++t) grow_one(&G[t]);
	if (!xv_failed()) names_make();
	/* (the preload as the spawn reads an id: lsls #0x1C, lsrs #0x14) */
	xv_insn(BN6_PRELOAD_ID_LSL, THUMB_LSLS, 2, 2, 0x1F, 32 - ID_HIGH_BITS);
	xv_insn(BN6_PRELOAD_ID_LSR, THUMB_LSRS, 2, 2, 0x17, 32 - 8 - ID_HIGH_BITS);
	if (xv_failed()) {
		fprintf(stderr, "xvirus: BN6's tables not grown, no family installed: %s\n", xv_why());
		xv_tx_end();
		return false;
	}
	uint32_t used = xv_used();
	xv_commit();
	xv_tx_end();
	for (int t = 0; t < XG_TABLES; ++t) xv_moved_add(get32(R.data + (G[t].ref - BUS)), G[t].to, G[t].count * G[t].size);
	hooks();
	grown = true;
	printf("xvirus: BN6's tables grown for the families (AI indexes to %d, enemy ids to %#x, their names from %#x; %u bytes)\n", XV_AIS, XV_IDS,
		XV_FIRST_ID, (unsigned)used);
	return true;
}

uint32_t xvirus_drop_row(int id) {
	uint32_t n = grown ? XV_IDS : BN6_DROP_ROW_COUNT, at = grown ? XV_ROWS : BN6_DROP_ROWS;
	return id >= 0 && (uint32_t)id < n ? at + (uint32_t)id * 2u * ENEMY_DROPS : 0;
}

/* ---- a family's entries ---- */

void xg_set(int t, uint32_t i, const uint8_t *entry) {
	if (t < 0 || t >= XG_TABLES || !grown || i >= G[t].cap || G[t].size > ENTRY_MAX) { xv_fail("entry %#x of grown table %d", (unsigned)i, t); return; }
	const Grow *g = &G[t];
	uint32_t size = g->size;
	uint8_t fill[ENTRY_MAX] = { 0 };
	if (g->fill_of >= 0)
		for (uint32_t b = 0; b < size; ++b) fill[b] = emu_read8(g->to + (uint32_t)g->fill_of * size + b);
	else put32(fill, g->fill);
	uint32_t at = g->to + i * size;
	for (uint32_t b = 0; b < size; ++b)
		if (emu_read8(at + b) != fill[b]) { xv_fail("%s's entry %#x is taken (%08x)", g->name, (unsigned)i, (unsigned)at); return; }
	uint8_t *p = xv_put(at, size, false);
	if (p) memcpy(p, entry, size);
}

void xg_set_word(int t, uint32_t i, uint32_t v) {
	uint8_t w[ENTRY_MAX] = { 0 };
	put32(w, v);
	if (t >= 0 && t < XG_TABLES && G[t].size != 4) { xv_fail("grown table %d is no table of words", t); return; }
	xg_set(t, i, w);
}

uint32_t xg_names_end(void) {
	uint32_t last = XG_NAME_NONE;
	for (uint32_t i = 0; i < XV_IDS - XV_FIRST_ID; ++i) {
		uint32_t off = emu_read16(XV_NAMES + 2 * i);
		if (off > last && off < XV_NAMES_ROOM) last = off;
	}
	uint32_t n = 0;
	while (n < NAME_MAX && last + n < XV_NAMES_ROOM && emu_read8(XV_NAMES + last + n) != TEXT_END) ++n;
	return last + n + 1;
}
