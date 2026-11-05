/* xnavi_bn4_room.h: BN4's Navis' battle sprites in one room, filled for the battle's Navi from the player's BN4 ROM. */
#include "xnavi_bn4_room.h"

#include <stdio.h>
#include <stdlib.h>

#include "bn4_navis.h"
#include "claims.h"
#include "debug.h"
#include "emu.h"
#include "rom.h"
#include "xnavi.h"
#include "xnavi_recipe.h"
#include "xnavi_room.h"
#include "xvirus_tx.h"

#define LIST2 (BN4_NAVI_SPRITES / 4)   /* (BN4's list of its Navis' battle sprites, by its enemy records' kind) */
#define NONE  0xFF                     /* a seat's byte where no Navi of it is installed (the free space's padding) */

bool xnavi_bn4_room_sprite(const XvSprite *s) { return s && s->at == XN_BN4_SPRITE && s->room == XN_BN4_SPRITE_SIZE && s->to_list == 2; }

/* a BN4 Navi's seat by an id of his, -1 none */
static int seat_of(uint32_t id) {
	return id >= XN_BN4_FIRST_ID && id < XN_BN4_FIRST_ID + 4u * XN_BN4_SEATS ? (int)((id - XN_BN4_FIRST_ID) / 4) : -1;
}

void xnavi_bn4_room_list(const XVirusFamily *f, const uint8_t *rom, uint32_t lists) {
	const XvSprite *s = f->nsprites ? &f->sprites[0] : NULL;
	int seat = seat_of((uint32_t)f->first_id);
	if (!xnavi_bn4_room_sprite(s) || s->list != LIST2 || s->index == NONE || seat < 0) {
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
}

/* the room filled with BN4's list 2 sprite `number`; false where it cannot be read or does not fit */
static bool fill(int number) {
	const XRom *x = &XR[XROM_BN4_RED_SUN_US];
	if (!x->data || !x->layout || !x->layout->sprite_lists) return false;
	uint32_t len;
	uint8_t *d = xnavi_sprite_bytes(x->data, x->layout->sprite_lists, LIST2, number, &len);
	bool ok = d && len <= XN_BN4_SPRITE_SIZE;
	if (ok) {
		claims_begin(XN_CLAIMS);
		emu_write(XN_BN4_SPRITE, d, len);
		claims_end();
	}
	free(d);
	if (!ok) fprintf(stderr, "xnavi: BN4's battle sprite %d:%#x not put in the room\n", LIST2, (unsigned)number);
	else if (emu_debug_on()) fprintf(stderr, "xnavi: the room holds BN4's battle sprite %d:%#x (%u bytes)\n", LIST2, (unsigned)number, (unsigned)len);
	return ok;
}

bool xnavi_bn4_room_for(const uint8_t *entities, size_t len) {
	if (!emu_ready() || !XR[XROM_BN4_RED_SUN_US].data) return false;
	for (size_t i = 0; i + 4 <= len && entities[i] != 0xF0; i += 4) {
		int seat = entities[i] == 0x11 ? seat_of((uint32_t)(entities[i + 2] | entities[i + 3] << 8)) : -1;
		int number = seat >= 0 ? emu_read8(XN_BN4_SPRITES + (uint32_t)seat) : NONE;
		if (number != NONE) return fill(number);
	}
	return false;
}
