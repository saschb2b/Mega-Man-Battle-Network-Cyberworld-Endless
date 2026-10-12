/* xnavi_sprites.h: each Navi's sprite at its place, its arm points'
 * record, its number in list 2; checked first, written once. */
#include "xnavi_sprites.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "xnavi.h"
#include "xvirus.h"
#include "xvirus_grow.h"

#define BUS 0x08000000u

/* Each Navi's place: BN5's AI index, list 2's number in BN6, where its
 * sprite goes and the room it has there (each sprite's bytes, raw in
 * BN5: Colonel 46780, ShadowMan 32657, NumberMan 34731, TomahawkMan
 * 34443, KnightMan 85259, ToadMan 30394, BN5's MegaMan 24849) */
static const struct { uint8_t navi, number; uint32_t at, room; } PLACES[] = {
	{ BN5_NAVI_COLONEL, XNAVI_SPRITE_FIRST + 0, FREE_NAVI_SPRITES + 0x00000u, 0xC000u },
	{ BN5_NAVI_SHADOWMAN, XNAVI_SPRITE_FIRST + 1, FREE_NAVI_SPRITES + 0x0C000u, 0x9000u },
	{ BN5_NAVI_NUMBERMAN, XNAVI_SPRITE_FIRST + 2, FREE_NAVI_SPRITES + 0x15000u, 0x9000u },
	{ BN5_NAVI_TOMAHAWKMAN, XNAVI_SPRITE_FIRST + 3, FREE_NAVI_SPRITES + 0x1E000u, 0x9000u },
	{ BN5_NAVI_KNIGHTMAN, XNAVI_SPRITE_FIRST + 4, FREE_NAVI_SPRITES + 0x27000u, 0x15000u },
	{ BN5_NAVI_TOADMAN, XNAVI_SPRITE_FIRST + 5, FREE_NAVI_SPRITES + 0x3C000u, 0x8000u },
	{ BN5_NAVI_DARK_SOUL, XNAVI_SPRITE_FIRST + 6, FREE_NAVI_SPRITES + 0x44000u, 0x7000u },
};
#define NPLACES ((int)(sizeof PLACES / sizeof *PLACES))
_Static_assert(FREE_NAVI_SPRITES + 0x4B000u <= XNAVI_ARMS, "the sprites' places end before the arm points'");
_Static_assert(XNAVI_ARMS + XNAVI_ARMS_COUNT * BN6_ARM_RECORD <= XNAVI_SPRITES_END, "the arm points' records within the sprites' room");

static uint32_t done[NPLACES];   /* (each sprite's place once installed: one core a process) */

static int place_of(int n) {
	for (int i = 0; i < NPLACES; ++i)
		if (PLACES[i].navi == n) return i;
	return -1;
}

int xnavi_sprite_number(int n) {
	int i = place_of(n);
	return i < 0 ? -1 : PLACES[i].number;
}

static bool in_rom(uint32_t a, uint32_t n) { return a >= BUS && a - BUS <= ROM_SIZE && n <= ROM_SIZE - (a - BUS); }

/* The Navis' records grown once a core: BN6's, then zeros to every number
 * of list 2; its word pointed at them. False where the word is neither
 * BN6's nor already ours, or the place is not free. */
static bool arms_grown(void) {
	uint32_t was = get32(R.data + (BN6_ARM_NAVIS_REF - BUS)), now = emu_read32(BN6_ARM_NAVIS_REF);
	if (now == XNAVI_ARMS) return true;
	if (now != was || !in_rom(was, BN6_ARM_NAVIS_COUNT * BN6_ARM_RECORD)) {
		fprintf(stderr, "xnavi: the Navis' arm points' word reads %08x, not BN6's %08x\n", (unsigned)now, (unsigned)was);
		return false;
	}
	for (uint32_t i = 0; i < XNAVI_ARMS_COUNT * BN6_ARM_RECORD; ++i)
		if (emu_read8(XNAVI_ARMS + i) != 0xFF) {
			fprintf(stderr, "xnavi: %08x is not free for the arm points\n", (unsigned)(XNAVI_ARMS + i));
			return false;
		}
	static uint8_t t[XNAVI_ARMS_COUNT * BN6_ARM_RECORD];
	memset(t, 0, sizeof t);
	memcpy(t, R.data + (was - BUS), BN6_ARM_NAVIS_COUNT * BN6_ARM_RECORD);
	emu_write(XNAVI_ARMS, t, sizeof t);
	emu_write32(BN6_ARM_NAVIS_REF, XNAVI_ARMS);
	return true;
}

/* BN5's Navi n's battle sprite as its struct1 names it: its list and number there */
static bool struct1_of(int n, int *list, int *index) {
	const uint8_t *rom = XR[XROM_BN5_COLONEL_US].data;
	uint32_t table = BN5_NAVI_TABLES + 2u * BN5_NAVI_TABLE_STEP + 4u * (uint32_t)n;
	if (n < 0 || n >= BN5_NAVIS) return false;
	uint32_t s1 = get32(rom + (table - BUS));
	if (!in_rom(s1, BN6_TRAITS_SIZE)) return false;
	*list = rom[s1 - BUS + BN6_TRAITS_LIST] / 4;
	*index = rom[s1 - BUS + BN6_TRAITS_SPRITE];
	return *list < 3;
}

/* its arm points' record: BN5's 30 points for its list and number, then
 * BN6's four more as BN6 has them for MegaMan (no Navi of BN5's has them) */
static bool record_of(int list, int index, uint8_t out[BN6_ARM_RECORD]) {
	const uint8_t *rom = XR[XROM_BN5_COLONEL_US].data;
	uint32_t table = get32(rom + (BN5_ARM_LISTS - BUS) + 4u * (uint32_t)list), at = table + (uint32_t)index * BN5_ARM_RECORD;
	if (!in_rom(at, BN5_ARM_RECORD)) return false;
	memcpy(out, rom + (at - BUS), BN5_ARM_RECORD);
	memcpy(out + BN5_ARM_RECORD, R.data + (BN6_MEGAMAN_ARMS - BUS) + BN5_ARM_RECORD, BN6_ARM_RECORD - BN5_ARM_RECORD);
	return true;
}

/* the sprite copied and its record written, every byte checked first */
static bool place_in(int i, int list, int index) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	uint32_t len = 0, entry = XV_LISTS + (uint32_t)xvirus_list_slot(BN6_SPRITES_NAVIS) * XV_STEP + 4u * PLACES[i].number;
	uint8_t rec[BN6_ARM_RECORD];
	uint8_t *d = xnavi_sprite_bytes(x->data, x->layout->sprite_lists, list, index, &len);
	bool ok = d && len <= PLACES[i].room && record_of(list, index, rec) && emu_read32(entry) == BN6_SPRITE_PLACEHOLDER;
	for (uint32_t k = 0; ok && k < len; ++k) ok = emu_read8(PLACES[i].at + k) == 0xFF;
	if (!ok) {
		fprintf(stderr, "xnavi: BN5's Navi %d's sprite (list %d %#x, %u bytes) not placed at list 2 %#x\n", PLACES[i].navi, list, index,
			(unsigned)len, PLACES[i].number);
		free(d);
		return false;
	}
	claims_begin(XNAVI_SPRITES_CLAIMS);
	bool arms = arms_grown();
	if (arms) {
		emu_write(PLACES[i].at, d, len);
		emu_write(XNAVI_ARMS + (uint32_t)PLACES[i].number * BN6_ARM_RECORD, rec, BN6_ARM_RECORD);
	}
	claims_end();
	free(d);
	if (!arms) return false;
	/* (list 2's entry is the families' room's: claimed as its own) */
	claims_begin(XV_CLAIMS);
	emu_write32(entry, PLACES[i].at);
	claims_end();
	printf("xnavi: BN5's Navi %d's battle sprite at list 2 %#x (%u bytes)\n", PLACES[i].navi, PLACES[i].number, (unsigned)len);
	return true;
}

bool xnavi_sprite_install(int n) {
	int i = place_of(n), list, index;
	if (i < 0) return false;
	if (done[i]) return true;
	if (!R.data || !R.layout || !XR[XROM_BN5_COLONEL_US].data || !XR[XROM_BN5_COLONEL_US].layout || !emu_ready() || !xg_grow()) return false;
	if (!struct1_of(n, &list, &index)) {
		fprintf(stderr, "xnavi: BN5's Navi %d has no battle sprite in its struct1\n", n);
		return false;
	}
	if (!place_in(i, list, index)) return false;
	done[i] = PLACES[i].at;
	return true;
}
