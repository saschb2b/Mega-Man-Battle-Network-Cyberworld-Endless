/* xchips_bn4_art.h: BN4's chip units' art in a room filled on demand. */
#include "xchips_bn4_art.h"

#include <stdio.h>
#include <stdlib.h>

#include "bn6.h"
#include "bn6_chips.h"
#include "bn6_families.h"
#include "claims.h"
#include "debug.h"
#include "emu.h"
#include "rom.h"
#include "xchips_code.h"
#include "xnavi.h"
#include "xvirus_grow.h"
#include "xvirus_tx.h"

/* a recipe's row of the room: its number, -1 for any other place */
static int row_of(uint32_t at) {
	if (at < XC4_ROOM_ROW(0) || at >= XC4_ROOM_ROW(XC4_ROWS) || (at & 3)) return -1;
	return (int)((at - XC4_ROOM_ROW(0)) / 4);
}

/* row k's sprite among the units', NULL none */
static const XvSprite *sprite_of(int row) {
	for (int u = 0; u < xchips_bn4_count; ++u)
		for (int i = 0; i < xchips_bn4[u].code.nsprites; ++i)
			if (row_of(xchips_bn4[u].code.sprites[i].at) == row) return &xchips_bn4[u].code.sprites[i];
	return NULL;
}

/* the row BN6's list `list`'s number `index` is, -1 none of the room's */
static int row_listed(int list, int index) {
	for (int u = 0; u < xchips_bn4_count; ++u)
		for (int i = 0; i < xchips_bn4[u].code.nsprites; ++i) {
			const XvSprite *s = &xchips_bn4[u].code.sprites[i];
			if (s->to_list == list && s->to_index == index) return row_of(s->at);
		}
	return -1;
}

bool xchips_bn4_art_row(const XvSprite *s, const uint8_t *rom, uint32_t lists) {
	int row = row_of(s->at);
	if (row < 0) return false;
	uint32_t len = 0, entry = xvirus_list_entry(s->to_list, s->to_index);
	uint8_t *d = xnavi_sprite_bytes(rom, lists, s->list, s->index, &len);
	free(d);
	if (!d || len > s->room || !entry || xchips_bn4_art_slots(row, s->room) > (int)XC4_SLOTS)
		xv_fail("its sprite %d:%#x (%u bytes, %u of room, row %d of the art's room)", s->list, s->index, (unsigned)len, (unsigned)s->room, row);
	else xg_set_word(XG_LIST + xvirus_list_slot(s->to_list), s->to_index, BN6_SPRITE_PLACEHOLDER);
	return true;
}

/* ---- the room, from a hook: the core that runs it ---- */

static uint32_t tag_at(int slot) { return XC4_ROOM + 2u * (uint32_t)slot; }

/* the slot and row a sprite's data pointer (an object's) names, where it lies in the room; false for any other */
static bool named(uint32_t data, int *slot, int *row) {
	uint32_t at = data - 4;
	if (data < 4 || at < XC4_ROOM_SLOT(0) || at >= XC4_ROOM_SLOT(XC4_SLOTS)) return false;
	*slot = (int)((at - XC4_ROOM_SLOT(0)) / XC4_SLOT);
	*row = (int)((at - XC4_ROOM_SLOT(0)) % XC4_SLOT / 4);
	return true;
}

/* row `row`'s sprite's bytes at its place in slot `slot`, where its slots' tags name another: read from the player's BN4 ROM
 * (unpacked where BN4 keeps it LZ77'd), the tags of what it overwrites cleared, its own set */
static void fill(int slot, int row) {
	const XvSprite *s = sprite_of(row);
	const XRom *x = &XR[XROM_BN4_RED_SUN_US];
	if (!s || !x->data || !x->layout) return;
	int n = xchips_bn4_art_slots(row, s->room);
	bool held = slot + n <= (int)XC4_SLOTS;
	for (int i = 0; held && i < n; ++i) held = hook_read16(tag_at(slot + i)) == (uint16_t)(row + 1);
	if (held || slot + n > (int)XC4_SLOTS) return;
	uint32_t len = 0, at = XC4_ROOM_SLOT(slot) + 4u * (uint32_t)row;
	uint8_t *d = xnavi_sprite_bytes(x->data, x->layout->sprite_lists, s->list, s->index, &len);
	if (!d || len > s->room) {
		free(d);
		return;
	}
	/* (another row's sprite that lay in these slots is whole no more: every tag of its gone) */
	for (int i = 0; i < n; ++i) {
		uint16_t old = hook_read16(tag_at(slot + i));
		for (int k = 0; old && old != (uint16_t)(row + 1) && k < (int)XC4_SLOTS; ++k)
			if (hook_read16(tag_at(k)) == old) hook_write16(tag_at(k), 0);
	}
	for (uint32_t b = 0; b < len; ++b) hook_write8(at + b, d[b]);
	for (int i = 0; i < n; ++i) hook_write16(tag_at(slot + i), (uint16_t)(row + 1));
	free(d);
	if (emu_debug_on()) fprintf(stderr, "xchips: the art's room: row %d (%d:%#x, %u bytes) in slot %d\n", row, s->list, s->index, (unsigned)len, slot);
}

/* every object in play of a kind (its array at `base`, `size` each, `count` of them): its sprite's slots into *used, its slot
 * into *at where it holds `row`; each one's sprite put right in the room where `mend` */
static void objects(uint32_t base, uint32_t size, int count, uint64_t *used, int row, int *at, bool mend) {
	for (int i = 0; i < count; ++i) {
		uint32_t obj = base + size * (uint32_t)i;
		uint8_t flags = hook_read8(obj);
		int slot, r;
		if (!(flags & 1) || !named(hook_read32(obj + ((uint32_t)(hook_read8(obj + BN6_OBJ_SPRITE_BLOCK) >> 4) << 4) + BN6_SPRITE_DATA), &slot, &r)) continue;
		const XvSprite *s = sprite_of(r);
		int n = s ? xchips_bn4_art_slots(r, s->room) : 1;
		for (int k = 0; k < n && slot + k < (int)XC4_SLOTS; ++k) *used |= (uint64_t)1 << (slot + k);
		if (r == row && *at < 0) *at = slot;
		if (mend) fill(slot, r);
	}
}

/* the slots in play, and the slot `row` is in play at (-1 none) */
static uint64_t in_play(int row, int *at, bool mend) {
	uint64_t used = 0;
	*at = -1;
	objects(BN6_T1_OBJECTS, BN6_T1_SIZE, BN6_T1_COUNT, &used, row, at, mend);
	objects(BN6_T3_OBJECTS, BN6_T1_SIZE, BN6_T3_COUNT, &used, row, at, mend);
	objects(BN6_T4_OBJECTS, BN6_T4_SIZE, BN6_T4_COUNT, &used, row, at, mend);
	return used;
}

/* (BN6_SPRITE_LOAD_IN: r1 the list's offset, r2 the number, r5 the object) a row of the room's put there
 * and its list's entry pointed at it, before the routine reads the entry */
static HookAct load(HookRegs *r, void *user) {
	(void)user;
	int list = (int)(r->r[1] / 4), row = (r->r[1] & 3) || r->r[2] > 0xFF ? -1 : row_listed(list, (int)r->r[2]);
	const XvSprite *s = row < 0 ? NULL : sprite_of(row);
	uint32_t entry = s ? xvirus_list_entry(s->to_list, s->to_index) : 0;
	if (!entry) return HOOK_CONTINUE;
	int at;
	uint64_t used = in_play(row, &at, false);
	if (at < 0) at = xchips_bn4_art_place(used, xchips_bn4_art_slots(row, s->room), (int)XC4_SLOTS);
	if (at < 0) {
		/* (every slot holds a sprite in play: BN6's placeholder, as it leaves an unloaded sprite) */
		hook_write32(entry, BN6_SPRITE_PLACEHOLDER);
		if (emu_debug_on()) fprintf(stderr, "xchips: the art's room has no run of slots for row %d (%d:%#x)\n", row, s->list, s->index);
		return HOOK_CONTINUE;
	}
	fill(at, row);
	hook_write32(entry, XC4_ROOM_SLOT(at) + 4u * (uint32_t)row);
	return HOOK_CONTINUE;
}

/* (BN6_OBJECT_RUNNER_IN, before the run list's objects run) every sprite an object in play points at in the
 * room, there: after a rollback or a loaded state the objects are another moment's and the room's bytes are not */
static HookAct frame(HookRegs *r, void *user) {
	(void)r;
	(void)user;
	int at;
	in_play(-1, &at, true);
	return HOOK_CONTINUE;
}

bool xchips_bn4_art_install(void) {
	static bool done;
	if (done) return true;
	if (!XR[XROM_BN4_RED_SUN_US].data || !emu_ready()) return false;
	claims_begin(XC_CLAIMS);
	done = emu_hook(BN6_SPRITE_LOAD_IN, load, NULL) && emu_hook(BN6_OBJECT_RUNNER_IN, frame, NULL);
	claims_end();
	if (!done) fprintf(stderr, "xchips: the art's room NOT hooked\n");
	return done;
}
