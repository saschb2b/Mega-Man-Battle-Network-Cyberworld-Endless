/* xnavi_bn4_chip.h: a BN4 Navi chip's Navi's sprite, art and songs in the chips' rooms, filled as his chip runs. */
#include "xnavi_bn4_chip.h"

#include <stdio.h>
#include <stdlib.h>

#include "bn6.h"
#include "bn6_chips.h"
#include "claims.h"
#include "debug.h"
#include "emu.h"
#include "rom.h"
#include "xchips_bn4.h"
#include "xchips_bn5.h"
#include "xnavi.h"
#include "xnavi_bn4_art.h"
#include "xnavi_bn4_room.h"
#include "xnavi_bn4_sounds.h"
#include "xnavi_recipe.h"
#include "xnavi_room.h"
#include "xnavi_sprites.h"
#include "xsong.h"
#include "xvirus_recipe.h"

#define NONE 0xFF
#define SHIFT (XN_BN4_CHIP_ART - XN_BN4_ART)   /* (a place of the battle's art room as the chips' room has it) */

/* a unit's seat by its number in BN6's grown Navi routines, -1 none of BN4's Navi chips' */
static int chip_seat(int number) {
	for (int i = 0; i < xchips_bn4_count; ++i)
		if (xchips_bn4[i].kind == XCHIP_NAVI && xchips_bn4[i].number == number && xchips_bn4[i].navis) return xchips_bn4[i].navis - 1;
	return -1;
}

/* the seat of the BN4 Navi chip whose Navi object (BN6's, the fourth kind's BN6_T4_CHIP_NAVI) is past its turn's wait, in its
 * own states or done: -1 none. One at a time: a second side's object waits in its step 0 for the first's end */
static int running(void) {
	uint8_t mode = emu_read8(BN6_GAMESTATE);
	if (mode != BN6_SUB_BATTLE && mode != BN6_SUB_BATTLE_INIT) return -1;
	for (uint32_t k = 0; k < BN6_T4_COUNT; ++k) {
		uint32_t o = BN6_T4_OBJECTS + k * BN6_T4_SIZE;
		if (!(emu_read8(o) & 1) || emu_read8(o + BN6_T1_TYPE) != BN6_T4_CHIP_NAVI || emu_read8(o + BN6_T1_ACTION) < BN6_CHIP_NAVI_OWN) continue;
		int seat = chip_seat(emu_read8(o + BN6_CHIP_NAVI_SUB));
		if (seat >= 0) return seat;
	}
	return -1;
}

/* whether the battle's Navi `foe` (NULL none) lists the sprite number `s` does: the battle's room holds it then, at his place */
static bool listed(const XNavi *foe, const XvSprite *s) {
	for (int i = 0; foe && i < foe->code.nsprites; ++i)
		if (foe->code.sprites[i].to_list == s->to_list && foe->code.sprites[i].to_index == s->to_index) return true;
	return false;
}

/* a sprite of BN4's at `at`, `room` bytes: a raw one straight from the ROM's bytes, an LZ77'd one unpacked and let go */
static bool sprite_at(int list, int number, uint32_t at, uint32_t room) {
	const XRom *x = &XR[XROM_BN4_RED_SUN_US];
	uint32_t len = 0;
	const uint8_t *raw = xnavi_bn4_raw(list, number, &len);
	uint8_t *d = raw ? NULL : xnavi_sprite_bytes(x->data, x->layout->sprite_lists, list, number, &len);
	bool ok = (raw || d) && len <= room;
	if (ok) emu_write(at, raw ? raw : d, len);
	free(d);
	return ok;
}

static void entry_set(int list, int number, uint32_t to) {
	uint32_t entry = xvirus_list_entry(list, (uint32_t)number);
	if (!entry || emu_read32(entry) == to) return;
	claims_begin(xn_writer(entry));
	emu_write32(entry, to);
	claims_end();
}

/* whether sprite `s` of his art is one the chips' art room holds: a row of the battle's art room the battle's Navi `other`
 * (NULL none) does not list too */
static bool art_row(const XvSprite *s, const XNavi *other) {
	return s->at >= XN_BN4_ART && s->at < XN_BN4_END && !listed(other, s);
}

/* his art's sprites copied into the chips' art room, at their places there */
static bool art_in(const XVirusFamily *f, const XNavi *other) {
	bool ok = true;
	for (int i = 1; i < f->nsprites && ok; ++i) {
		const XvSprite *s = &f->sprites[i];
		if (art_row(s, other)) ok = s->room <= XN_BN4_CHIP_ART_END - (s->at + SHIFT) && sprite_at(s->list, s->index, s->at + SHIFT, s->room);
	}
	return ok;
}

/* his pool's songs packed at their places in the chips' art room (written where `write`), their headers into `headers` */
static bool songs_in(const XVirusFamily *f, bool write, uint32_t headers[XNAVI_BN4_POOL_SLOTS]) {
	bool ok = true;
	for (int i = 0; i < f->nsongs && i < XNAVI_BN4_POOL_SLOTS && ok; ++i) {
		const XvSong *s = &f->songs[i];
		bool fits = s->at >= XN_BN4_ART && s->at < XN_BN4_END && s->room <= XN_BN4_CHIP_ART_END - (s->at + SHIFT);
		uint8_t *p = fits ? malloc(s->room) : NULL;
		headers[i] = p ? xsong_pack(f->game, s->from, s->at + SHIFT, p, s->room) : 0;
		if (headers[i] && write) emu_write(s->at + SHIFT, p, s->room);
		ok = headers[i] != 0;
		free(p);
	}
	return ok;
}

/* the chips' rooms made Navi `n`'s (seat `seat`), the battle's rooms holding seat `foe`'s (NONE: nobody's): his battle
 * sprite, each sprite of his art the battle's Navi does not list, his pool's songs; `headers` their headers there; his
 * entries pointed at them */
static bool fill(const XNavi *n, int seat, int foe, uint32_t headers[XNAVI_BN4_POOL_SLOTS]) {
	const XRom *x = &XR[XROM_BN4_RED_SUN_US];
	const XVirusFamily *f = &n->code;
	const XNavi *other = foe == NONE ? NULL : xnavi_bn4_seat(foe);
	if (!x->data || !x->layout || !x->layout->sprite_lists || !f->nsprites) return false;
	bool held = emu_read8(XN_BN4_HELD + XN_BN4_HELD_CHIP) == seat && emu_read8(XN_BN4_HELD + XN_BN4_HELD_WITH) == foe;
	claims_begin(XN_BN4_CHIP_CLAIMS);
	bool ok = held || (sprite_at(f->sprites[0].list, f->sprites[0].index, XN_BN4_CHIP_SPRITE, XN_BN4_CHIP_SPRITE_SIZE) && art_in(f, other));
	ok = ok && songs_in(f, !held, headers);
	if (ok && !held) {
		uint8_t now[2] = { (uint8_t)seat, (uint8_t)foe };
		emu_write(XN_BN4_HELD + XN_BN4_HELD_CHIP, now, 2);
	}
	claims_end();
	/* (his entries pointed at the chips' rooms, whoever pointed them last: a battle's fill points its Navi's at its own) */
	if (ok) {
		entry_set(f->sprites[0].to_list, f->sprites[0].to_index, XN_BN4_CHIP_SPRITE);
		for (int i = 1; i < f->nsprites; ++i)
			if (art_row(&f->sprites[i], other)) entry_set(f->sprites[i].to_list, f->sprites[i].to_index, f->sprites[i].at + SHIFT);
	}
	if (!ok) fprintf(stderr, "xnavi: %s's sprite, art and songs not all put in the chips' rooms\n", f->name);
	else if (emu_debug_on() && !held) fprintf(stderr, "xnavi: the chips' rooms hold %s's\n", f->name);
	return ok;
}

void xnavi_bn4_chip_frame(void) {
	/* (the seat the pool's override names and the battle's it was set beside, -1 none: while both hold and the core's own
	 * record of its rooms agrees, a frame of his chip's run does nothing more) */
	static int pointed = -1, beside = -1;
	if (!emu_ready() || !XR[XROM_BN4_RED_SUN_US].data) return;
	int seat = running();
	if (seat < 0) {
		if (pointed >= 0) xnavi_bn4_sounds_chip(-1, NULL);
		pointed = -1;
		return;
	}
	const XNavi *n = xnavi_bn4_seat(seat);
	if (!n || !n->code.nsprites) return;
	int foe = emu_read8(XN_BN4_HELD + XN_BN4_HELD_BATTLE);
	uint32_t entry = xvirus_list_entry(n->code.sprites[0].to_list, n->code.sprites[0].to_index);
	bool rooms = foe == seat ? emu_read32(entry) == XN_BN4_SPRITE :
		emu_read32(entry) == XN_BN4_CHIP_SPRITE && emu_read8(XN_BN4_HELD + XN_BN4_HELD_CHIP) == seat && emu_read8(XN_BN4_HELD + XN_BN4_HELD_WITH) == foe;
	if (pointed == seat && beside == foe && rooms) return;
	if (foe == seat) {
		/* (the battle's rooms hold him, his entries theirs since its fill: his songs there) */
		xnavi_bn4_sounds_chip(n->code.first_id, NULL);
	} else {
		uint32_t headers[XNAVI_BN4_POOL_SLOTS] = { 0 };
		if (!fill(n, seat, foe, headers)) return;
		xnavi_bn4_sounds_chip(n->code.first_id, headers);
	}
	pointed = seat;
	beside = foe;
}
