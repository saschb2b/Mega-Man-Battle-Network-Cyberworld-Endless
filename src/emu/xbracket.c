/* BN4 Red Sun's bracket screen in BN6 (xbracket.h). */
#include "xbracket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bn6_shop.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "gamecall.h"
#include "rom.h"
#include "thumb_reloc.h"
#include "xbracket_bn4.h"
#include "xsong.h"
#include "xvirus.h"
#include "xvirus_tx.h"

#define XBR_CLAIMS "BN4's bracket screen"
#define BUS 0x08000000u
#define SONG_FALLBACK 0x15      /* (BN6's own song where BN4's cannot be copied: the shop's) */

static bool installed;

/* the tournaments' place the shims give: XBR_TOURNAMENTS; with CYBERWORLD_XBRACKET_MUTATE=tournaments the next record's (the move
 * broken on purpose, a RAM word placed wrong: tools/xbracket.py --move-mutation must catch it) */
static uint32_t tournaments_word(void) {
	const char *m = getenv("CYBERWORLD_XBRACKET_MUTATE");
	return XBR_TOURNAMENTS + (m && !strcmp(m, "tournaments") ? BN4_TOURNAMENT_SIZE : 0u);
}

/* its stubs, veneers and calls' targets: its code and art in FREE_BRACKET */
static const XvRegion region = { XBR_CODE, FREE_BRACKET_END, XBR_ART, FREE_BRACKET_END, XBR_AT, XBR_STUBS_END, true };

/* XBR_LIST: BN6_MAP_TO_SHOP's calls in its order, each with the r0 a movs before it gives (0 where none does), but its fade test
 * and the beq after it (a game call's entry runs once, the screen's own fade to come), to its store of BN6_MODE_SHOP; then BN4's
 * set-up moved, and the end */
static void list_stage(uint8_t *out) {
	int n = 0;
	uint16_t arg = 0;
	for (uint32_t a = BN6_MAP_TO_SHOP; a < BN6_MAP_TO_SHOP_END && !xv_failed(); a += 2) {
		uint16_t op = rom_u16(a - BUS), next = rom_u16(a + 2 - BUS);
		uint32_t t = 0;
		if (op == (0x2100u | BN6_MODE_SHOP) && next == 0x7001) {
			/* (then BN4_MAP_LEAVE's calls BN6's way lacks, by their twins: BN4's BG controls' preset among them; then the set-up) */
			for (int k = 0; k < xbracket_leave_count && n + 3 < XBR_LIST_MAX; ++k, ++n) {
				const XbrCall *l = &xbracket_leave[k];
				put32(out + 8 * n, l->arg);
				put32(out + 8 * n + 4, l->own ? xvirus_code_at(&xbracket_bn4, l->fn | 1u) : l->fn);
			}
			put32(out + 8 * n, 0);
			put32(out + 8 * n + 4, xvirus_code_at(&xbracket_bn4, BN4_BRACKET_SETUP | 1u));
			put32(out + 8 * n + 8, 0);
			put32(out + 8 * n + 12, 0);
			return;
		}
		if ((op & 0xFF00) == 0x2000) { arg = op & 0xFF; continue; }
		if (!thumb_is_bl(op, next)) continue;
		thumb_bl_decode(a, op, next, &t);
		a += 2;
		if (t == BN6_FADE_TEST) {
			if ((rom_u16(a + 2 - BUS) & 0xFF00) != 0xD000) break;
			a += 2;
			continue;
		}
		if (n + 2 >= XBR_LIST_MAX || t < BUS || t >= BUS + 0x800000u) break;
		put32(out + 8 * n, arg);
		put32(out + 8 * n + 4, t | 1u);
		arg = 0;
		++n;
	}
	xv_fail("BN6_MAP_TO_SHOP (%08x) is not the routine expected", (unsigned)BN6_MAP_TO_SHOP);
}

/* its stubs and its entry (the project's own Thumb) */
static void stubs_stage(void) {
	uint8_t *p = xv_put(XBR_AT, XBR_ENTER_END - XBR_AT, true);
	if (!p) return;
	put16(p + 0x00, 0x4770);   /* XBR_STUB_NONE: bx lr */
	put16(p + 0x02, 0x46C0);
	put16(p + 0x10, 0x4900);   /* XBR_SHIM_R1: ldr r1,[pc,#0]; bx lr; the tournaments' place */
	put16(p + 0x12, 0x4770);
	put32(p + 0x14, tournaments_word());
	put16(p + 0x20, 0x4A00);   /* XBR_SHIM_R2: ldr r2,[pc,#0]; bx lr; the word */
	put16(p + 0x22, 0x4770);
	put32(p + 0x24, tournaments_word());
	/* XBR_STUB_RETURN: BN6's main mode the game's (BN6_MODE_GAME), its game state's sub-mode BN6_SUB_ENTER (the map entered again
	 * as BN6's own screens leave: eToolkit +0 its mode's byte, +0x3C its game state) */
	static const uint16_t ret[] = { 0x4650, 0x6801, 0x2204, 0x700A, 0x6BC1, 0x2200, 0x700A, 0x4770 };
	for (int i = 0; i < 8; ++i) put16(p + 0x30 + 2 * i, ret[i]);
	/* XBR_ENTER: each (r0, routine) of XBR_LIST called in turn through a trampoline, then BN6's main mode made XBR_MODE: push {r4,lr};
	 * adr r4,XBR_LIST; loop: ldmia r4!,{r0,r1}; cmp r1,#0; beq done; bl tramp; b loop; nop; done: mov r0,r10; ldr r0,[r0];
	 * movs r1,#XBR_MODE; strb r1,[r0]; pop {r4,pc}; tramp: bx r1 */
	static const uint16_t enter[] = { 0xB510, 0xA407, 0xCC03, 0x2900, 0xD003, 0, 0, 0xE7F9, 0x46C0, 0x4650, 0x6800, 0x2100 | XBR_MODE,
		0x7001, 0xBD10, 0x4708, 0x46C0 };
	uint16_t hi = 0, lo = 0;
	if (!thumb_bl_encode(XBR_ENTER + 0x0A, XBR_ENTER + 0x1C, &hi, &lo)) return;
	for (int i = 0; i < 16; ++i) put16(p + (XBR_ENTER - XBR_AT) + 2 * i, i == 5 ? hi : i == 6 ? lo : enter[i]);
	list_stage(p + (XBR_LIST - XBR_AT));
	/* XBR_STUB_DISPLAY: mov r1,r10; ldr r1,[r1,#8]; strh r0,[r1]; bx lr */
	static const uint16_t disp[] = { 0x4651, 0x6889, 0x8008, 0x4770 };
	for (int i = 0; i < 4; ++i) put16(p + (XBR_STUB_DISPLAY - XBR_AT) + 2 * i, disp[i]);
	/* XBR_STUB_HBLANK: push {r4-r6,lr}; ldr r0,=XBR_BLEND_DMA; adds r4,r0,#0; movs r5,#0xA0; lsls r5,r5,#1; ldr r6,=BN6's set; bl tramp;
	 * pop {r4-r6,pc}; tramp: bx r6; the words: the DMA's source the copy XBR_STUB_FRAME makes, its own line buffer too (BN6's copy of r5
	 * bytes from r4 into the source, which BN4's set has not, a copy onto itself) */
	static const uint16_t hb[] = { 0xB570, 0x4804, 0x1C04, 0x25A0, 0x006D, 0x4E03, 0, 0, 0xBD70, 0x4730 };
	if (!thumb_bl_encode(XBR_STUB_HBLANK + 0x0C, XBR_STUB_HBLANK + 0x12, &hi, &lo)) return;
	for (int i = 0; i < 10; ++i) put16(p + (XBR_STUB_HBLANK - XBR_AT) + 2 * i, i == 6 ? hi : i == 7 ? lo : hb[i]);
	put32(p + (XBR_STUB_HBLANK - XBR_AT) + 0x14, XBR_BLEND_DMA);
	put32(p + (XBR_STUB_HBLANK - XBR_AT) + 0x18, BN6_HBLANK_DMA_SET | 1u);
	/* XBR_STUB_FRAME, the mode's routine: push {lr}; the line values the frame before wrote copied for the DMA (BN6_COPY_WORDS, r0 from,
	 * r1 to, r2 bytes); the screen's routine; pop {pc}; tramp: bx r3; the words. A frame shows the values the one before it wrote, as BN4's
	 * shows them (tools/xbracket.py, measured; BN6's DMA reads its source live, BN6's own set copying nothing into it) */
	static const uint16_t fr[] = { 0xB500, 0x4806, 0x4906, 0x22A0, 0x0052, 0x4B06, 0, 0, 0x4B05, 0, 0, 0xBD00, 0x4718, 0x46C0 };
	uint16_t hi2 = 0, lo2 = 0;
	if (!thumb_bl_encode(XBR_STUB_FRAME + 0x0C, XBR_STUB_FRAME + 0x18, &hi, &lo) || !thumb_bl_encode(XBR_STUB_FRAME + 0x12, XBR_STUB_FRAME + 0x18, &hi2, &lo2))
		return;
	for (int i = 0; i < 14; ++i)
		put16(p + (XBR_STUB_FRAME - XBR_AT) + 2 * i, i == 6 ? hi : i == 7 ? lo : i == 9 ? hi2 : i == 10 ? lo2 : fr[i]);
	put32(p + (XBR_STUB_FRAME - XBR_AT) + 0x1C, XBR_BLEND);
	put32(p + (XBR_STUB_FRAME - XBR_AT) + 0x20, XBR_BLEND_DMA);
	put32(p + (XBR_STUB_FRAME - XBR_AT) + 0x24, BN6_COPY_WORDS | 1u);
	put32(p + (XBR_STUB_FRAME - XBR_AT) + 0x28, xvirus_code_at(&xbracket_bn4, BN4_BRACKET_SCREEN | 1u));
}

/* its reads of eToolkit +0x3C, each made a call to the shim of its register (the routine pushes lr: the generator checks) */
static void sites_stage(void) {
	for (int i = 0; i < xbracket_site_count && !xv_failed(); ++i) {
		const XbrSite *s = &xbracket_sites[i];
		uint32_t at = xvirus_code_at(&xbracket_bn4, s->at), shim = s->reg == 1 ? XBR_SHIM_R1 : s->reg == 2 ? XBR_SHIM_R2 : 0;
		const uint8_t *rom = XR[XROM_BN4_RED_SUN_US].data + (s->at - BUS);
		uint16_t hi = 0, lo = 0;
		if (!at || !shim || get16(rom) != (0x4650u | s->reg) || get16(rom + 2) != (0x6BC0u | (uint16_t)(s->reg << 3) | s->reg) ||
			!thumb_bl_encode(at, shim, &hi, &lo)) {
			xv_fail("its read of eToolkit +0x3C at %08x", (unsigned)s->at);
			return;
		}
		uint8_t *p = xv_put(at, 4, false);
		if (!p) return;
		put16(p, hi);
		put16(p + 2, lo);
	}
}

/* its song: BN4's BN4_SONG_BRACKET copied into a BN6 slot (xsong.c), the slot its immediate */
static void song_stage(void) {
	const uint8_t *rom = XR[XROM_BN4_RED_SUN_US].data + (BN4_BRACKET_SONG_SITE - BUS);
	uint32_t at = xvirus_code_at(&xbracket_bn4, BN4_BRACKET_SONG_SITE);
	if (!at || get16(rom) != (0x2000u | BN4_SONG_BRACKET)) {
		xv_fail("its song's immediate at %08x", (unsigned)BN4_BRACKET_SONG_SITE);
		return;
	}
	int slot = xsong_install(XROM_BN4_RED_SUN_US, BN4_SONG_BRACKET, SONG_FALLBACK);
	uint8_t *p = xv_put(at, 2, false);
	if (p) put16(p, (uint16_t)(0x2000u | (slot & 0xFF)));
}

bool xbracket_install(void) {
	if (installed) return true;
	if (!emu_ready() || !XR[XROM_BN4_RED_SUN_US].data || !R.data) return false;
	xv_tx_begin();
	if (xvirus_code_stage(&xbracket_bn4, &region, XBR_VENEERS, XBR_NVENEERS)) {
		stubs_stage();
		sites_stage();
		song_stage();
	}
	/* (its mode in BN6's main modes' table, empty there) */
	if (!xv_failed()) xv_word(XBR_MODES + XBR_MODE, 0, XBR_STUB_FRAME | 1u);
	if (xv_failed()) {
		fprintf(stderr, "xbracket: BN4's bracket screen not installed: %s\n", xv_why());
		xv_tx_end();
		return false;
	}
	uint32_t used = xv_used();
	claims_begin(XBR_CLAIMS);
	xv_commit();
	claims_end();
	xv_tx_end();
	for (int i = 0; i < xbracket_bn4.nblocks; ++i)
		xv_moved_add(xbracket_bn4.blocks[i].start, xbracket_bn4.blocks[i].at, xbracket_bn4.blocks[i].end - xbracket_bn4.blocks[i].start);
	installed = true;
	printf("xbracket: BN4's bracket screen as BN6's main mode %#x (%u bytes written)\n", XBR_MODE, (unsigned)used);
	return true;
}

bool xbracket_open(int t, const XbrEntrant entrants[BN4_ENTRANTS]) {
	if (t < BN4_TOURNAMENT_DEN || t > BN4_TOURNAMENT_FREE || !xbracket_install()) return false;
	uint32_t at = XBR_TOURNAMENTS + (uint32_t)t * BN4_TOURNAMENT_SIZE;
	for (int i = 0; i < BN4_ENTRANTS; ++i) {
		uint32_t e = at + (uint32_t)i * BN4_ENTRANT_SIZE;
		emu_write8(e + BN4_ENTRANT_ID, entrants[i].id);
		emu_write8(e + BN4_ENTRANT_FLAGS, entrants[i].flags);
		emu_write8(e + BN4_ENTRANT_RANK, entrants[i].seed);
		emu_write8(e + BN4_ENTRANT_MARK, 0);
	}
	emu_write8(XBR_STATE + BN4_BRACKET_TOURNAMENT, (uint8_t)t);
	emu_write8(XBR_STATE + BN4_BRACKET_FROM_SCRIPT, 0);
	emu_write8(XBR_STATE + BN4_BRACKET_SCRIPT_ARG, 0);
	return game_call(XBR_ENTER | 1u, 0, 0);
}

bool xbracket_dev_step(const char *spec) {
	XbrEntrant e[BN4_ENTRANTS];
	char *end = NULL;
	long t = strtol(spec, &end, 0);
	for (int i = 0; i < BN4_ENTRANTS; ++i) {
		while (end && (*end == ' ' || *end == '/')) ++end;
		unsigned long v = end && *end ? strtoul(end, &end, 16) : 0x1000000ul;
		if (v > 0xFFFFFFul) {
			printf("bracket: %s names no eight entrants (T IIFFSS/... in hex)\n", spec);
			return false;
		}
		e[i] = (XbrEntrant){ (uint8_t)(v >> 16), (uint8_t)(v >> 8), (uint8_t)v };
	}
	bool ok = xbracket_open((int)t, e);
	printf("bracket: tournament %ld %s\n", t, ok ? "shown" : "not shown (BN4's ROM not read, or the screen not installed)");
	return ok;
}
