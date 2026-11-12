/* xnavi_bn4_art.h: BN4's Navis' attacks' art and pool songs in one room, filled for the battle's Navi from the player's
 * BN4 ROM. */
#include "xnavi_bn4_art.h"

#include <stdio.h>
#include <stdlib.h>

#include "claims.h"
#include "debug.h"
#include "emu.h"
#include "rom.h"
#include "xnavi.h"
#include "xnavi_bn4.h"
#include "xnavi_bn4_room.h"
#include "xnavi_room.h"
#include "xsong.h"
#include "xvirus_grow.h"
#include "xvirus_tx.h"

#define NONE 0xFF   /* a seat's byte where no Navi of it is installed (the free space's padding) */

/* (a place of the room, `n` bytes from it) */
static bool in_room(uint32_t at, uint32_t n) { return at >= XN_BN4_ART && at < XN_BN4_END && n <= XN_BN4_END - at; }

/* a BN4 Navi's seat by an id of his, -1 none */
static int seat_of(uint32_t id) {
	return id >= XN_BN4_FIRST_ID && id < XN_BN4_FIRST_ID + 4u * XN_BN4_SEATS ? (int)((id - XN_BN4_FIRST_ID) / 4) : -1;
}

bool xnavi_bn4_art_row(const XvSprite *s, const uint8_t *rom, uint32_t lists) {
	if (!in_room(s->at, s->room)) return false;
	uint32_t len, entry = xvirus_list_entry(s->to_list, s->to_index);
	uint8_t *d = xnavi_sprite_bytes(rom, lists, s->list, s->index, &len);
	free(d);
	if (!d || len > s->room || !entry) {
		xv_fail("its sprite %d:%#x (%u bytes, %u of room)", s->list, s->index, (unsigned)len, (unsigned)s->room);
		return true;
	}
	/* (an entry another Navi pointed into the room: the number two list, each at his own place, pointed by the fill) */
	uint32_t now = emu_read32(entry);
	if (!in_room(now, 1)) xg_set_word(XG_LIST + xvirus_list_slot(s->to_list), s->to_index, s->at);
	return true;
}

void xnavi_bn4_art_list(const XVirusFamily *f) {
	int seat = seat_of((uint32_t)f->first_id);
	uint8_t *p = seat >= 0 ? xv_put(XN_BN4_ARTS + (uint32_t)seat, 1, true) : NULL;
	if (p) *p = (uint8_t)seat;
	else if (seat < 0) xv_fail("its ids are no BN4 Navi's seat's");
}

const XNavi *xnavi_bn4_seat(int seat) {
	for (int i = 0; i < xnavi_bn4_count; ++i)
		if (seat_of((uint32_t)xnavi_bn4[i]->code.first_id) == seat) return xnavi_bn4[i];
	return NULL;
}

/* its sprite `s` at its place, its entry pointed at it: a raw one straight from the ROM's bytes, an LZ77'd one unpacked (no
 * heap but the unpacking's, the 3DS's tight) */
static bool sprite_in(const XRom *x, const XvSprite *s) {
	uint32_t len = 0, entry = xvirus_list_entry(s->to_list, s->to_index);
	const uint8_t *raw = xnavi_bn4_raw(s->list, s->index, &len);
	uint8_t *d = raw ? NULL : xnavi_sprite_bytes(x->data, x->layout->sprite_lists, s->list, s->index, &len);
	bool ok = (raw || d) && len <= s->room && entry;
	if (ok) {
		emu_write(s->at, raw ? raw : d, len);
		claims_begin(xn_writer(entry));
		emu_write32(entry, s->at);
		claims_end();
	}
	free(d);
	return ok;
}

/* its pool's song `s` packed at its place (xsong.h: the header its slot names, as its install made it) */
static bool song_in(int game, const XvSong *s) {
	uint8_t *p = malloc(s->room);
	bool ok = p && xsong_pack(game, s->from, s->at, p, s->room);
	if (ok) emu_write(s->at, p, s->room);
	free(p);
	return ok;
}

bool xnavi_bn4_art_for(int seat) {
	const XRom *x = &XR[XROM_BN4_RED_SUN_US];
	int which = seat >= 0 && seat < XN_BN4_SEATS ? emu_read8(XN_BN4_ARTS + (uint32_t)seat) : NONE;
	const XNavi *n = which != NONE ? xnavi_bn4_seat(which) : NULL;
	if (!n || !x->data || !x->layout || !x->layout->sprite_lists) return false;
	const XVirusFamily *f = &n->code;
	bool ok = true;
	claims_begin(XN_CLAIMS);
	for (int i = 0; i < f->nsprites; ++i)
		if (in_room(f->sprites[i].at, f->sprites[i].room)) ok = sprite_in(x, &f->sprites[i]) && ok;
	for (int i = 0; i < f->nsongs; ++i)
		if (in_room(f->songs[i].at, f->songs[i].room)) ok = song_in(f->game, &f->songs[i]) && ok;
	claims_end();
	if (!ok) fprintf(stderr, "xnavi: %s's art not all put in the room\n", f->name);
	else if (emu_debug_on()) fprintf(stderr, "xnavi: the art's room holds %s's\n", f->name);
	return ok;
}
