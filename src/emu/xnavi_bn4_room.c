/* xnavi_bn4_room.h: BN4's Navis' battle sprites in one room, filled for the battle's Navi from the player's BN4 ROM. */
#include "xnavi_bn4_room.h"

#include <stdio.h>
#include <stdlib.h>

#include "bn4_navis.h"
#include "bytes.h"
#include "claims.h"
#include "debug.h"
#include "emu.h"
#include "rom.h"
#include "xnavi.h"
#include "xnavi_bn4.h"
#include "xnavi_bn4_art.h"
#include "xnavi_recipe.h"
#include "xnavi_room.h"
#include "xvirus_grow.h"
#include "xvirus_bn4.h"
#include "xvirus_tx.h"

#define BUS   0x08000000u
#define LIST2 (BN4_NAVI_SPRITES / 4)   /* (BN4's list of its Navis' battle sprites, by its enemy records' kind) */
#define NONE  0xFF                     /* a seat's byte where no Navi of it is installed (the free space's padding) */

/* (a recipe's sprite at the room, whole, in list 2) */
static bool room_sprite(const XvSprite *s) { return s && s->at == XN_BN4_SPRITE && s->room == XN_BN4_SPRITE_SIZE && s->to_list == 2; }

/* a BN4 Navi's seat by an id of his, -1 none */
static int seat_of(uint32_t id) {
	return id >= XN_BN4_FIRST_ID && id < XN_BN4_FIRST_ID + 4u * XN_BN4_SEATS ? (int)((id - XN_BN4_FIRST_ID) / 4) : -1;
}

void xnavi_bn4_room_list(const XVirusFamily *f, const uint8_t *rom, uint32_t lists) {
	const XvSprite *s = f->nsprites ? &f->sprites[0] : NULL;
	int seat = seat_of((uint32_t)f->first_id);
	if (!room_sprite(s) || s->list != LIST2 || s->index == NONE || seat < 0) {
		xv_fail("its battle sprite is not one of the room's (xnavi_bn4_room.h)");
		return;
	}
	uint32_t len;
	uint8_t *d = xnavi_sprite_bytes(rom, lists, s->list, s->index, &len);
	free(d);
	if (!d || len > XN_BN4_SPRITE_SIZE) {
		xv_fail("its battle sprite %d:%#x (%u bytes) past the room's %u", s->list, s->index, (unsigned)len, (unsigned)XN_BN4_SPRITE_SIZE);
		return;
	}
	uint8_t *p = xv_put(XN_BN4_SPRITES + (uint32_t)seat, 1, true);
	if (p) *p = s->index;
	xnavi_bn4_art_list(f);   /* (and his art's: xnavi_bn4_art.h) */
}

const uint8_t *xnavi_bn4_raw(int list, int number, uint32_t *len) {
	const XRom *x = &XR[XROM_BN4_RED_SUN_US];
	if (!x->data || !x->layout || !x->layout->sprite_lists) return NULL;
	uint32_t lists = x->layout->sprite_lists, at = lists + 4u * (uint32_t)list + 4 <= ROM_SIZE ? get32(x->data + lists + 4u * (uint32_t)list) - BUS : ROM_SIZE;
	uint32_t p = at + 4u * (uint32_t)number + 4 <= ROM_SIZE ? get32(x->data + at + 4u * (uint32_t)number) : 0x80000000u;
	if ((p & 0x80000000u) || p < BUS || p - BUS >= ROM_SIZE) return NULL;
	*len = xnavi_sprite_len(x->data + (p - BUS), ROM_SIZE - (p - BUS));
	return *len ? x->data + (p - BUS) : NULL;
}

/* the room filled with BN4's list 2 sprite `number` (a raw one straight from the ROM's bytes, an LZ77'd one unpacked: no
 * heap but the unpacking's, the 3DS's tight); false where it cannot be read or does not fit */
static bool fill(int number) {
	const XRom *x = &XR[XROM_BN4_RED_SUN_US];
	if (!x->data || !x->layout || !x->layout->sprite_lists) return false;
	uint32_t len = 0;
	const uint8_t *raw = xnavi_bn4_raw(LIST2, number, &len);
	uint8_t *d = raw ? NULL : xnavi_sprite_bytes(x->data, x->layout->sprite_lists, LIST2, number, &len);
	bool ok = (raw || d) && len <= XN_BN4_SPRITE_SIZE;
	if (ok) {
		claims_begin(XN_CLAIMS);
		emu_write(XN_BN4_SPRITE, raw ? raw : d, len);
		claims_end();
	}
	free(d);
	if (!ok) fprintf(stderr, "xnavi: BN4's battle sprite %d:%#x not put in the room\n", LIST2, (unsigned)number);
	else if (emu_debug_on()) fprintf(stderr, "xnavi: the room holds BN4's battle sprite %d:%#x (%u bytes)\n", LIST2, (unsigned)number, (unsigned)len);
	return ok;
}

bool xnavi_bn4_room_row(const XvSprite *s, const uint8_t *rom, uint32_t lists) {
	if (s->at != XN_BN4_SPRITE) return xnavi_bn4_art_row(s, rom, lists);
	xg_set_word(XG_LIST + xvirus_list_slot(s->to_list), s->to_index, s->at);
	return true;
}

uint32_t xnavi_bn4_summoner(uint32_t id) {
	for (int i = 0; i < xvirus_bn4_count; ++i) {
		const XVirusFamily *f = xvirus_bn4[i];
		if (!f->summoned || id < (uint32_t)f->first_id || id >= (uint32_t)f->first_id + XV_VERSIONS) continue;
		for (int k = 0; k < xnavi_bn4_count; ++k)
			for (int j = 0; xnavi_bn4[k]->summon && j < xnavi_bn4[k]->summon->nids; ++j)
				if (xnavi_bn4[k]->summon->ids[j].to == id) return (uint32_t)xnavi_bn4[k]->code.first_id;
	}
	return 0;
}

/* (the seat of the battle's BN4 Navi among its entities, or where there is none, of the Navi a family of it is summoned by
 * alone: -1 none) */
static int battle_seat(const uint8_t *entities, size_t len, bool summoned) {
	for (size_t i = 0; i + 4 <= len && entities[i] != 0xF0; i += 4) {
		uint32_t id = (uint32_t)(entities[i + 2] | entities[i + 3] << 8);
		int seat = entities[i] == 0x11 ? seat_of(summoned ? xnavi_bn4_summoner(id) : id) : -1;
		if (seat >= 0 && emu_read8(XN_BN4_SPRITES + (uint32_t)seat) != NONE) return seat;
	}
	return -1;
}

bool xnavi_bn4_room_for(const uint8_t *entities, size_t len) {
	if (!emu_ready() || !XR[XROM_BN4_RED_SUN_US].data) return false;
	int seat = battle_seat(entities, len, false);
	if (seat < 0) seat = battle_seat(entities, len, true);
	if (seat < 0) return false;
	/* (his attacks' art and songs in theirs: xnavi_bn4_art.h) */
	bool art = xnavi_bn4_art_for(seat);
	return fill(emu_read8(XN_BN4_SPRITES + (uint32_t)seat)) && art;
}
