/* xvirus_grow.h: BN6's tables grown in the core's ROM copy, as one step of
 * staged writes (xvirus_tx.h), and the hooks that keep the families' AI
 * indexes and ids to what BN6 has room for. */
#include "xvirus_grow.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "data.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "xvirus.h"
#include "xvirus_dispatch.h"
#include "xvirus_tx.h"

#define BUS          0x08000000u
#define TEXT_END     0xE6   /* a text archive's entry ends with it */
#define NAME_MAX     32     /* a name's bytes at most, its end among them */
#define ENTRY_MAX    BN6_ARM_VIRUS_SIZE   /* the largest entry, an arm points' record's (a reward row's 2 x ENEMY_DROPS) */
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
	int kind;   /* a kind's types (1, 3, 4): its dispatch type's entry is the dispatch's routine (xvirus_dispatch.h); 0 none */
} Grow;

static Grow G[XG_TABLES];
_Static_assert(ENTRY_MAX >= 2u * ENEMY_DROPS, "a reward row within an entry's most");

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
	/* (in xvirus_list_slot's order; list 3, the chips' attacks, the last, at a place of its own) */
	static const uint32_t lists[XV_LISTS_GROWN][2] = {
		{ BN6_SPRITES_VIRUSES, BN6_SPRITES_VIRUSES_COUNT }, { BN6_SPRITES_NAVIS, BN6_SPRITES_NAVIS_COUNT },
		{ BN6_SPRITES_ATTACKS, BN6_SPRITES_ATTACKS_COUNT }, { BN6_SPRITES_LIST5, BN6_SPRITES_LIST5_COUNT },
		{ BN6_SPRITES_LIST3, BN6_SPRITES_LIST3_COUNT },
	};
	for (uint32_t i = 0; i < XV_LISTS_GROWN; ++i)
		G[XG_LIST + i] = (Grow){ "a sprite list", BUS + R.layout->sprite_lists + 4 * lists[i][0], lists[i][1], 4,
			lists[i][0] == BN6_SPRITES_LIST3 ? XV_LIST3 : XV_LISTS + i * XV_STEP, XV_LIST_MAX, -1, BN6_SPRITE_PLACEHOLDER };
	G[XG_EFFECTS] = (Grow){ "the effects", BN6_EFFECTS_REF, BN6_EFFECT_COUNT, 4, XV_EFFECTS, XV_EFFECTS_MAX, 0, 0 };
	G[XG_T1] = (Grow){ "the first kind's types", BN6_T1_TYPES_REF, BN6_TYPES_T1_COUNT, 4, XV_TYPES, XV_TYPES_MAX, -1, 0, 1 };
	G[XG_T3] = (Grow){ "the third kind's types", BN6_T3_TYPES_REF, BN6_TYPES_T3_COUNT, 4, XV_TYPES + XV_STEP, XV_TYPES_MAX, -1, 0, 3 };
	G[XG_T4] = (Grow){ "the fourth kind's types", BN6_T4_TYPES_REF, BN6_TYPES_T4_COUNT, 4, XV_TYPES + 2 * XV_STEP, XV_TYPES_MAX, -1, 0, 4 };
	G[XG_IDS] = (Grow){ "the enemy ids", BN6_ENEMY_IDS_REF, BN6_ENEMY_IDS_COUNT, 3, XV_IDS_AT, XV_IDS, BN6_FILLER_ID, 0 };
	G[XG_ROWS] = (Grow){ "the reward rows", BN6_DROP_ROWS_REF, BN6_DROP_ROW_COUNT, 2u * ENEMY_DROPS, XV_ROWS, XV_IDS, BN6_FILLER_ID, 0 };
	/* (the arm points' records of every number of list 1: BN6's bytes for each number its list holds, as it reads them, its 31
	 * records and for its 32nd the Navis' first, which lies after them; record 0's for the numbers past; a family's at its
	 * battle sprite's number, xvirus.c) */
	G[XG_ARMS] = (Grow){ "the viruses' arm points", BN6_ARM_VIRUSES_REF, BN6_SPRITES_VIRUSES_COUNT, BN6_ARM_VIRUS_SIZE, XV_ARMS, XV_LIST_MAX, 0, 0 };
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
	if (g->kind) {
		/* (the dispatch type's entry BN6's placeholder first: the kind's type 0 routine, the first's and the fourth's; 0, the third's) */
		uint8_t *e = t + 4u * (uint32_t)xvirus_dispatch_type(g->kind);
		uint32_t was = get32(e), placeholder = g->kind == 3 ? 0 : get32(t);
		if (was != placeholder) { xv_fail("%s: the dispatch's type holds %08x, not BN6's placeholder %08x", g->name, (unsigned)was, (unsigned)placeholder); return; }
		put32(e, xvirus_dispatch_routine(g->kind) | 1);
	}
	xv_word(g->ref, from, g->to);
}

/* The second-level dispatch's code, and its tables blank: every MOVED
 * entry the stray's routine, every TWIN entry XV_TYPE_MOVED */
static void dispatch_make(void) {
	uint8_t *code = xv_put(XV_STUBS, XV_STUBS_SIZE, true);
	if (code) xvirus_dispatch_code(code);
	uint8_t *moved = xv_put(XV_MOVED_TABLES, XV_DISPATCH_KINDS * XV_STEP, true);
	for (uint32_t i = 0; moved && i < XV_DISPATCH_KINDS * XV_STEP / 4; ++i) put32(moved + 4 * i, XV_STUB_NONE | 1);
	uint8_t *twin = xv_put(XV_TWIN_TABLES, XV_DISPATCH_KINDS * 0x100u, true);
	if (twin) memset(twin, XV_TYPE_MOVED, XV_DISPATCH_KINDS * 0x100u);
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
	if (!xv_failed()) dispatch_make();
	/* (the families' pre-steps by AI index, none until a family's install names its own: xvirus_shim.h) */
	if (!xv_failed()) xv_put(XV_PRESTEP, 4u * XV_AIS, true);
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

/* entry i of a grown table still BN6's own entry where it has one, else the fill: BN6's entry fill_of, each as the
 * player's ROM has it, whatever a family set over a placeholder of BN6's since */
static bool entry_free(const Grow *g, uint32_t i) {
	uint8_t fill[ENTRY_MAX] = { 0 };
	uint32_t size = g->size, table = get32(R.data + (g->ref - BUS)), own = table + (i < g->count ? i : (uint32_t)g->fill_of) * size;
	if ((i < g->count || g->fill_of >= 0) && in_rom(own, size)) memcpy(fill, R.data + (own - BUS), size);
	else put32(fill, g->fill);
	for (uint32_t b = 0; b < size; ++b)
		if (emu_read8(g->to + i * size + b) != fill[b]) return false;
	return true;
}

/* entry i of grown table t given n bytes, its entry's size */
static void entry_set(int t, uint32_t i, const uint8_t *entry, uint32_t n) {
	if (t < 0 || t >= XG_TABLES || !grown || i >= G[t].cap || G[t].size != n || n > ENTRY_MAX) {
		xv_fail("entry %#x of grown table %d", (unsigned)i, t);
		return;
	}
	const Grow *g = &G[t];
	uint32_t at = g->to + i * n;
	if (!entry_free(g, i)) { xv_fail("%s's entry %#x is taken (%08x)", g->name, (unsigned)i, (unsigned)at); return; }
	uint8_t *p = xv_put(at, n, false);
	if (p) memcpy(p, entry, n);
}

void xg_set(int t, uint32_t i, const uint8_t *entry) {
	entry_set(t, i, entry, t >= 0 && t < XG_TABLES ? G[t].size : 0);
}

void xg_set_word(int t, uint32_t i, uint32_t v) {
	uint8_t w[4];
	put32(w, v);
	if (t >= 0 && t < XG_TABLES && G[t].size != 4) { xv_fail("grown table %d is no table of words", t); return; }
	entry_set(t, i, w, sizeof w);
}

void xg_type(int kind, uint8_t from, uint8_t to, uint32_t routine) {
	uint32_t twin = xvirus_dispatch_twin_at(kind, from), moved = xvirus_dispatch_moved_at(kind, from);
	if (!grown || !twin || !moved || (to == XV_TYPE_MOVED) != (routine != 0)) { xv_fail("the type %d:%#x for the dispatch", kind, from); return; }
	/* (each entry blank, or what another family set there already: the same) */
	uint8_t was = emu_read8(twin);
	if (was != XV_TYPE_MOVED && was != to) { xv_fail("the type %d:%#x is BN6's %#x already", kind, from, was); return; }
	if (to == XV_TYPE_MOVED) {
		uint32_t r = emu_read32(moved);
		if (was != XV_TYPE_MOVED || (r != (XV_STUB_NONE | 1) && r != (routine | 1))) { xv_fail("the type %d:%#x is moved already (%08x)", kind, from, (unsigned)r); return; }
		uint8_t *p = xv_put(moved, 4, false);
		if (p) put32(p, routine | 1);
		return;
	}
	uint8_t *p = xv_put(twin, 1, false);
	if (p) *p = to;
}

bool xg_bn6_entry(int t, uint32_t i, const uint8_t *was, const uint8_t *now) {
	if (!R.data || !R.layout || !emu_ready() || t < 0 || t >= XG_TABLES) return false;
	specs();
	const Grow *g = &G[t];
	if (i >= g->count || g->size > ENTRY_MAX) return false;
	uint32_t at = (grown ? g->to : get32(R.data + (g->ref - BUS))) + i * g->size;
	for (uint32_t b = 0; b < g->size; ++b)
		if (emu_read8(at + b) != was[b]) return false;
	if (grown) claims_begin(XV_CLAIMS);
	emu_write(at, now, g->size);
	if (grown) claims_end();
	return true;
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
