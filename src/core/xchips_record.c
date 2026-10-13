/* A registered chip's record in BN6's layout (xchips_record.h). */
#include "xchips_record.h"

#include <string.h>

#include "bytes.h"

int xrec_icon_element(int bn5) {
	/* (BN5: 0-3 the elements, 4 Recovery, 5 Plus, 6 Sword, 7 Invisible, 8
	 * Cursor, 9 Obstacle, 10 Wind, 11 Break, 12 Null, 13 a Program
	 * Advance; measured over the chips both games name alike) */
	static const uint8_t to6[14] = { 0, 1, 2, 3, 10, 4, 5, 10, 6, 7, 8, 9, 10, 11 };
	return bn5 >= 0 && bn5 < 14 ? to6[bn5] : 10;
}

/* its art's three pointers */
static void art_put(uint8_t out[XREC_SIZE], const uint32_t art[3]) {
	put32(out + BN6_CHIP_ICON_PTR, art[0]);
	put32(out + BN6_CHIP_IMAGE_PTR, art[1]);
	put32(out + BN6_CHIP_PALETTE_PTR, art[2]);
}

/* the behaviour: the twin's family, lock-on and lock-on type (and its
 * subfamily but the row's), or inert where it sits out for want of code */
static void behaviour(const XRecordFrom *in, uint8_t out[XREC_SIZE]) {
	if (in->moved) {
		out[BN6_CHIP_FAMILY] = in->moved->family;
		out[BN6_CHIP_SUBFAMILY] = in->moved->sub;
		out[BN6_CHIP_LOCK_ON] = out[BN6_CHIP_LOCK_ON_TYPE] = 0;
		return;
	}
	if (!in->twin) {
		out[BN6_CHIP_FAMILY] = XREC_INERT_FAMILY;
		out[BN6_CHIP_SUBFAMILY] = XREC_INERT_SUB;
		out[BN6_CHIP_LOCK_ON] = out[BN6_CHIP_LOCK_ON_TYPE] = 0;
		memset(out + BN6_CHIP_PARAMS, 0, 4);
		return;
	}
	out[BN6_CHIP_FAMILY] = in->twin[BN6_CHIP_FAMILY];
	out[BN6_CHIP_SUBFAMILY] = in->reg->sub != 0xFF ? in->reg->sub : in->twin[BN6_CHIP_SUBFAMILY];
	out[BN6_CHIP_LOCK_ON] = in->twin[BN6_CHIP_LOCK_ON];
	out[BN6_CHIP_LOCK_ON_TYPE] = in->twin[BN6_CHIP_LOCK_ON_TYPE];
}

void xrec_from_bn5(const XRecordFrom *in, uint8_t out[XREC_SIZE]) {
	memcpy(out, in->bn5, XREC_SIZE);
	out[BN6_CHIP_ELEMENT] = (uint8_t)xrec_icon_element(in->bn5[BN6_CHIP_ELEMENT]);
	uint8_t fx = in->bn5[BN6_CHIP_EFFECT_FLAGS] & (uint8_t)~BN6_CHIP_IN_LIBRARY;
	/* (a power of 1000 and up is an index into the game's own table of
	 * computed powers, BN5's not BN6's: a playing chip's where a helper
	 * moved its routine into BN6's grown table, xchips_power_of; else
	 * none shown) */
	int power = get16(in->bn5 + BN6_CHIP_ATTACK_POWER);
	if (power >= 1000) {
		int to = in->moved || in->twin ? xchips_power_of(power) : 0;
		put16(out + BN6_CHIP_ATTACK_POWER, (uint16_t)to);
		if (!to) fx &= (uint8_t)~(BN6_CHIP_COMPUTED_POWER | BN6_CHIP_SHOWS_POWER);
	}
	out[BN6_CHIP_EFFECT_FLAGS] = fx;
	behaviour(in, out);
	out[BN6_CHIP_LIBRARY_NUMBER] = out[BN6_CHIP_LIBRARY_FLAGS] = 0;
	put16(out + BN6_CHIP_ALPHA_SORT, in->abc);
	put16(out + BN6_CHIP_ID_SORT, in->reg->bn6);
	out[BN6_CHIP_DARK_ID] = 0xFF;
	art_put(out, in->art);
}

#define REC_DAMAGE_HIGH 0x0A   /* (the damage's upper half: its element and hit flags, AttackVars +0xA by the chip writer, BN6 0x0801270A) */
#define REC_AFTER_SUB 0x0D     /* (two bytes no chip use reads: BN5's taken with the subfamily) */

void xrec_same_moved(const uint8_t bn5[XREC_SIZE], const XChipMoved *moved, uint8_t out[XREC_SIZE]) {
	out[BN6_CHIP_ATTACK_ELEMENT] = bn5[BN6_CHIP_ATTACK_ELEMENT];
	out[BN6_CHIP_EFFECT_FLAGS] = (uint8_t)((bn5[BN6_CHIP_EFFECT_FLAGS] & ~BN6_CHIP_IN_LIBRARY) | (out[BN6_CHIP_EFFECT_FLAGS] & BN6_CHIP_IN_LIBRARY));
	out[REC_DAMAGE_HIGH] = bn5[REC_DAMAGE_HIGH];
	out[BN6_CHIP_FAMILY] = moved->family;
	out[BN6_CHIP_SUBFAMILY] = moved->sub;
	memcpy(out + REC_AFTER_SUB, bn5 + REC_AFTER_SUB, 2);
	out[BN6_CHIP_LOCK_ON] = out[BN6_CHIP_LOCK_ON_TYPE] = 0;
	memcpy(out + BN6_CHIP_PARAMS, bn5 + BN6_CHIP_PARAMS, 5);
	out[BN6_CHIP_DARK_ID] = 0xFF;
}

void xrec_placeholder(const XChipReg *reg, uint16_t abc, const uint32_t blank[3], uint8_t out[XREC_SIZE]) {
	memset(out, 0, XREC_SIZE);
	memcpy(out + BN6_CHIP_CODES, reg->codes, 4);
	out[BN6_CHIP_ELEMENT] = 10;   /* (Null) */
	out[BN6_CHIP_LIBRARY_TYPE] = reg->type;
	out[BN6_CHIP_MB] = reg->mb;
	out[BN6_CHIP_EFFECT_FLAGS] = reg->how == XREG_DARK ? BN6_CHIP_DARK_CLASS : 0;
	out[BN6_CHIP_FAMILY] = XREC_INERT_FAMILY;
	out[BN6_CHIP_SUBFAMILY] = XREC_INERT_SUB;
	put16(out + BN6_CHIP_ALPHA_SORT, abc);
	put16(out + BN6_CHIP_ID_SORT, reg->bn6);
	out[BN6_CHIP_GATE] = 1;
	out[BN6_CHIP_DARK_ID] = 0xFF;
	art_put(out, blank);
}

/* names compared as the editor's ABC sort reads them, case aside */
static int name_cmp(const char *a, const char *b) {
	for (;; ++a, ++b) {
		int x = *a >= 'a' && *a <= 'z' ? *a - 32 : *a, y = *b >= 'a' && *b <= 'z' ? *b - 32 : *b;
		if (x != y || !x) return x - y;
	}
}

uint16_t xrec_abc_place(const char *name, const char *const *names, const uint16_t *places, int n) {
	const char *best = NULL;
	uint16_t place = 1;
	for (int i = 0; i < n; ++i) {
		if (!places[i] || !names[i] || !names[i][0]) continue;
		if (name_cmp(names[i], name) < 0 && (!best || name_cmp(names[i], best) > 0)) {
			best = names[i];
			place = places[i];
		}
	}
	return place;
}

void xrec_icon_colours(const uint8_t bn5_pal[32], const uint8_t bn6_pal[32], uint8_t map[16]) {
	for (int i = 0; i < 16; ++i) {
		map[i] = (uint8_t)i;
		if (i < 10) continue;
		unsigned c = get16(bn5_pal + 2 * i), best = ~0u;
		for (int k = 1; k < 10; ++k) {
			unsigned d = get16(bn6_pal + 2 * k), dist = 0;
			for (int s = 0; s < 15; s += 5) {
				int a = (int)(c >> s & 31) - (int)(d >> s & 31);
				dist += (unsigned)(a * a);
			}
			if (dist < best) { best = dist; map[i] = (uint8_t)k; }
		}
	}
}

void xrec_icon_recolour(uint8_t icon[0x80], const uint8_t map[16]) {
	for (int i = 0; i < 0x80; ++i) icon[i] = (uint8_t)(map[icon[i] & 15] | map[icon[i] >> 4] << 4);
}

int xrec_deck_sit_out(uint16_t *deck, int n, const uint8_t sits[64], int keep, unsigned *first) {
	int w = 0, taken = 0;
	*first = 0;
	for (int i = 0; i < n; ++i) {
		uint16_t e = deck[i];
		unsigned id = e & 0x1FFu;
		if (e != 0xFFFF && sits[id >> 3] & (1u << (id & 7))) {
			++taken;
			if (i < keep && i < 8) *first |= 1u << i;
			continue;
		}
		deck[w++] = e;
	}
	for (; w < n; ++w) deck[w] = 0xFFFF;
	return taken;
}
