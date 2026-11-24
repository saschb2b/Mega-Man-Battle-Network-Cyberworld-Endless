/* xchips_folder.h. Another game's NEW GAME folder as the run's: read from
 * its ROM where its own GiveFolder reads it, each chip the run's chip of
 * it (xchips.h), each code one the run's record gives. */
#include "xchips_folder.h"

#include <stddef.h>

#include "bn4.h"
#include "bn5.h"
#include "bn6.h"
#include "chip_entry.h"
#include "chips.h"
#include "rom.h"
#include "xchips.h"

#define BUS 0x08000000u
#define ROM_SPAN 0x02000000u   /* a cartridge's bus, 32 MB */
#define CODE_STAR 26

int xfolder_code(const uint8_t codes[4], int code) {
	int best = -1, star = 0;
	for (int k = 0; k < 4; ++k) {
		int c = codes[k];
		if (c == code) return code;
		if (c == CODE_STAR) star = 1;
		else if (c < CODE_STAR) {
			int d = c > code ? c - code : code - c, b = best > code ? best - code : code - best;
			if (best < 0 || d < b || (d == b && c < best)) best = c;
		}
	}
	return code < CODE_STAR && best >= 0 ? best : star ? CODE_STAR : best >= 0 ? best : code;
}

bool xfolder_as_run(const uint16_t in[XFOLDER_ENTRIES], int (*run_of)(int), void (*codes_of)(int, uint8_t[4]),
	uint16_t out[XFOLDER_ENTRIES], int *swapped) {
	int n = 0;
	for (int i = 0; i < XFOLDER_ENTRIES; ++i) {
		int id = run_of((int)chip9_id(in[i])), code = (int)chip9_code(in[i]);
		if (id <= 0 || (unsigned)id > CHIP_ID_MASK) return false;
		uint8_t codes[4];
		codes_of(id, codes);
		int to = xfolder_code(codes, code);
		n += to != code;
		out[i] = chip_entry((unsigned)id, (unsigned)to);
	}
	if (swapped) *swapped = n;
	return true;
}

/* (the run's record's codes, as the pack counts them: chips.h) */
static void run_codes(int id, uint8_t out[4]) {
	const uint8_t *rec = chips_record(id);
	for (int k = 0; k < 4; ++k) out[k] = rec ? rec[BN6_CHIP_CODES + k] : 0xFF;
}

static uint16_t u16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }
static uint32_t u32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }

bool xfolder_start(int game, uint16_t out[XFOLDER_ENTRIES], int *swapped) {
	if (game != XROM_BN5_COLONEL_US && game != XROM_BN4_RED_SUN_US) return false;
	const uint8_t *d = XR[game].data;
	if (!d) return false;
	/* (BN5's GiveFolder takes folder 0's address from its table, BN4's
	 * finds its twelve in a row, folder 0 first) */
	uint32_t at = game == XROM_BN5_COLONEL_US ? u32(d + (BN5_FOLDER_TABLE - BUS)) : BN4_FOLDERS;
	if (at < BUS || at - BUS > ROM_SPAN - 2 * XFOLDER_ENTRIES) return false;
	uint16_t in[XFOLDER_ENTRIES];
	for (int i = 0; i < XFOLDER_ENTRIES; ++i) in[i] = u16(d + (at - BUS) + 2u * (uint32_t)i);
	return xfolder_as_run(in, game == XROM_BN5_COLONEL_US ? xchips_run_chip : xchips_bn4_paid, run_codes, out, swapped);
}
