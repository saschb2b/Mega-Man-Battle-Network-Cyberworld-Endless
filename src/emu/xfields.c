/* xfields.h: BN5's battlefields in BN6's numbers. */
#include "xfields.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "emu.h"
#include "rom.h"
#include "xpanels.h"

/* BN5's panel types as BN6's (docs/ROM_DATA.md, Battlefields): both games'
 * flags by type (BN5_PANEL_FLAGS_OF, BN6_PANEL_FLAGS_OF) agree for 0-4 and
 * 6-8, their code checks those numbers alike, and the differential fights
 * poison, grass and ice alike in both (tests/xdiff/field_*); BN5's holy is
 * 9 where BN6's is 5. BN6 has none of BN5's lava (8: BN6's 8 is another
 * panel, BN5_PANEL_LAVA) or its sea (0xA): those are the types xpanels.c
 * gives them in BN6's engine, where it could; BN5's 5 none */
static const int8_t types[16] = { 0, BN6_PANEL_HOLE, BN6_PANEL_PLAIN, BN6_PANEL_CRACKED, BN6_PANEL_POISON, -1, BN6_PANEL_GRASS, BN6_PANEL_ICE,
	XPANEL_LAVA, BN6_PANEL_HOLY, XPANEL_SEA, -1, -1, -1, -1, -1 };

int xfield_type(int type) {
	if ((type == BN5_PANEL_LAVA || type == BN5_PANEL_SEA) && !xpanels_ready()) return -1;
	return type >= 0 && type < 16 ? types[type] : -1;
}

/* The fields BN5's areas' records name (rom.c, NetAreaDef.xbattles), each
 * BN6's own of the same panels (`own`), or written over a field of BN6's
 * that no BattleSettings of BN6's names (its random battles' four tables,
 * its two lists of fixed battles, any record in the ROM: docs/ROM_DATA.md).
 * End Area's 0x42-0x45 (sea and grass) and the Undernet's and Nebula
 * Area's 0x73-0x76 (lava) where xpanels.c brought their panels in. */
static const struct { uint8_t bn5, bn6; bool own; } fields[] = {
	{ 0x00, 0x00, true }, { 0x1B, 0xC9, true }, { 0x20, 0xBD, true }, { 0x52, 0xDE, true }, { 0x54, 0xDF, true }, { 0x5B, 0x71, true },
	{ 0x5F, 0xE7, true },
	{ 0x15, 0x09, false }, { 0x16, 0x0D, false }, { 0x21, 0x0E, false }, { 0x22, 0x0F, false }, { 0x24, 0x11, false }, { 0x27, 0x1D, false },
	{ 0x28, 0x22, false }, { 0x29, 0x23, false }, { 0x5C, 0x24, false }, { 0x5E, 0x25, false }, { 0x62, 0x28, false }, { 0x63, 0x2F, false },
	{ 0x64, 0x30, false }, { 0x65, 0x31, false }, { 0x6B, 0x46, false }, { 0x6E, 0x55, false }, { 0x9B, 0x63, false }, { 0x9C, 0x64, false },
	{ 0x73, 0x56, false }, { 0x74, 0x5F, false }, { 0x75, 0x61, false }, { 0x76, 0x65, false }, { 0x42, 0x66, false }, { 0x43, 0x67, false },
	{ 0x44, 0x69, false }, { 0x45, 0x6A, false },
};
#define NFIELDS (sizeof fields / sizeof *fields)
static int16_t installed[NFIELDS];   /* BN6's number of each as installed, -1 none (0 before the install: none either) */
static bool done;

/* BN5's field `f` with its panels' types as BN6's, into out; false where a
 * panel is of a type BN6 has none of */
static bool translate(const uint8_t *d, int f, uint8_t out[BN6_FIELD_SIZE]) {
	const uint8_t *in = d + (BN5_FIELDS - 0x08000000u) + (uint32_t)f * BN6_FIELD_SIZE;
	for (int i = 0; i < BN6_FIELD_SIZE; ++i) {
		int lo = xfield_type(in[i] & 15), hi = xfield_type(in[i] >> 4);
		if (lo < 0 || hi < 0) return false;
		out[i] = (uint8_t)(lo | hi << 4);
	}
	return true;
}

void xfields_install(int xrom) {
	if (done || xrom != XROM_BN5_COLONEL_US || !XR[xrom].data || !R.data || !emu_ready()) return;
	done = true;
	xpanels_install(xrom);   /* (lava and sea first: the fields with them want them) */
	int own = 0, written = 0;
	for (size_t i = 0; i < NFIELDS; ++i) {
		installed[i] = -1;
		uint8_t t[BN6_FIELD_SIZE], now[BN6_FIELD_SIZE];
		uint32_t at = BN6_FIELDS + (uint32_t)fields[i].bn6 * BN6_FIELD_SIZE;
		const uint8_t *bn6 = R.data + (at - 0x08000000u);
		for (uint32_t k = 0; k < BN6_FIELD_SIZE; ++k) now[k] = emu_read8(at + k);
		/* (BN6's own of the same panels; or one BN6 leaves unnamed, still as
		 * the player's ROM has it) */
		if (fields[i].bn6 >= BN6_FIELD_COUNT || !translate(XR[xrom].data, fields[i].bn5, t) || memcmp(now, bn6, sizeof now) ||
			(fields[i].own && memcmp(t, bn6, sizeof t))) {
			fprintf(stderr, "xfields: BN5's field %#x not installed as BN6's %#x\n", fields[i].bn5, fields[i].bn6);
			continue;
		}
		if (!fields[i].own) emu_write(at, t, sizeof t);
		installed[i] = fields[i].bn6;
		own += fields[i].own;
		written += !fields[i].own;
	}
	printf("xfields: BN5's battlefields as BN6's, %d its own of the same panels and %d written\n", own, written);
}

int xfield_bn6(int xrom, int field) {
	if (xrom != XROM_BN5_COLONEL_US || !done) return -1;
	for (size_t i = 0; i < NFIELDS; ++i)
		if (fields[i].bn5 == field) return installed[i];
	return -1;
}
