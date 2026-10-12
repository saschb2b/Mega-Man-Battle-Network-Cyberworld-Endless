/* xnavi_dark.h: the chip AI's lists, BN5's chip ids as BN6's; the Dark
 * Soul's id's rules in BN6's engine and the chip records its chip AI reads. */
#include "xnavi_dark.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "chips.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "xchips_registry.h"
#include "xnavi_recipe.h"

#define NONE     0xFFFFu
#define SEQUENCE 0x8000u   /* an entry naming a sequence (its number below it) */
#define SEQ_SIZE 16u

/* a list's entry (or a sequence's chip) of BN5's as BN6's */
static uint16_t dark_chip(uint16_t bn5) {
	if (bn5 == NONE || (bn5 & SEQUENCE)) return bn5;
	const XChipReg *r = xreg_of_bn5(bn5);
	return r ? r->bn6 : NONE;
}

static int one(uint8_t *at) {
	uint16_t v = get16(at), w = dark_chip(v);
	put16(at, w);
	return w == NONE && v != NONE;
}

int xnavi_dark_lists_from(const uint8_t *bn5, uint8_t *out, uint32_t len) {
	int lost = 0;
	memcpy(out, bn5, len);
	for (uint32_t side = 0; side + BN5_DARK_LIST_SIZE <= len; side += BN5_DARK_LIST_SIZE) {
		uint8_t *l = out + side;
		for (uint32_t k = 0; k < XNAVI_DARK_CHIPS; ++k) lost += one(l + 2 * k);
		for (uint32_t q = 0; q < XNAVI_DARK_SEQUENCES; ++q)
			for (uint32_t k = 0; k < XNAVI_DARK_SEQ_CHIPS; ++k) lost += one(l + BN5_DARK_SEQUENCES + SEQ_SIZE * q + 2 + 2 * k);
	}
	return lost;
}

/* ---- the Dark Soul's id's rules in BN6's engine ---- */

#define BUS        0x08000000u
#define MOVS_R3_3  0x2303u
#define MOVS_R0_0  0x2000u
#define DARK_R2    2   /* (its r2, as BN5's test leaves it) */

static int dark_id = -1;

static bool is_dark(uint32_t obj) { return hook_read16(obj + BN6_T1_NAME_ID) == (uint16_t)dark_id; }

/* its AI data moved to BN6_DARK_AI (BN5's 0x140 bytes: the chip AI keeps its state from +0xF0 to +0x13F, past BN6's 0x100,
 * where the next slot's AI data begins), its pool's 0x100 bytes as its set-up has them so far; a Dark Soul still out
 * there given its pool's back (its first 0x100 bytes: it leaves, its timer ended) */
static void adopt(HookRegs *r, uint32_t obj) {
	uint32_t ai = hook_read32(obj + BN6_T1_AI_DATA), old = hook_read32(BN6_DARK_AI_OWNER), pool = hook_read32(BN6_DARK_AI_POOL);
	if (ai == BN6_DARK_AI || ai < BN6_EWRAM || ai >= BN6_EWRAM_END) return;
	if (old && old != obj && hook_read32(old + BN6_T1_AI_DATA) == BN6_DARK_AI && pool) {
		for (uint32_t k = 0; k < BN6_AI_DATA_SIZE; k += 4) hook_write32(pool + k, hook_read32(BN6_DARK_AI + k));
		hook_write32(old + BN6_T1_AI_DATA, pool);
	}
	for (uint32_t k = 0; k < BN5_AI_DATA_SIZE; k += 4) hook_write32(BN6_DARK_AI + k, k < BN6_AI_DATA_SIZE ? hook_read32(ai + k) : 0);
	hook_write32(obj + BN6_T1_AI_DATA, BN6_DARK_AI);
	hook_write32(BN6_DARK_AI_OWNER, obj);
	hook_write32(BN6_DARK_AI_POOL, ai);
	if (r->r[6] == ai) r->r[6] = BN6_DARK_AI;
}

/* (BN6_NAVI_REGION: the actor's set-up, its AI data in r6, laid out and filled so far) */
static HookAct region(HookRegs *r, void *user) {
	(void)user;
	if (is_dark(r->r[5])) {
		r->r[1] = BN6_DARK_REGION;
		r->r[2] = DARK_R2;
		adopt(r, r->r[5]);
	}
	return HOOK_CONTINUE;
}

static HookAct plate(HookRegs *r, void *user) {
	(void)user;
	if (!is_dark(r->r[5])) return HOOK_CONTINUE;
	r->r[12] = BN6_NAVI_PLATE_END | 1u;
	return HOOK_JUMP;
}

/* ---- its chip AI's records ---- */

/* the bytes of a record its chip AI reads as BN5 means them */
static const uint8_t ds_bytes[] = { BN5_CHIP_DS_FLAGS, BN5_CHIP_DS_USAGE, BN5_CHIP_DS_PICKS, BN5_CHIP_DS_REGION };
#define RECORDS_SIZE (XREG_IDS * BN6_CHIP_RECORD_SIZE)
_Static_assert(XN_DARK_RECORDS + RECORDS_SIZE <= FREE_CHIP_CODE, "the Dark Soul's records past their room (xnavi_recipe.h)");

/* The records XN_STUB_CHIPDATA hands its chip AI, by BN6's id: BN6's own as the core's copy holds them now (the chips'
 * 512 where their tables are grown, xchips_install.h, else BN6's 411 and the rest zero), each chip of BN5's (the
 * registry's rows) with BN5's four bytes; NULL, or what is not as expected */
static const char *records_make(uint8_t *out) {
	const uint8_t *rom5 = XR[XROM_BN5_COLONEL_US].data;
	if (!rom5) return "BN5's ROM";
	uint32_t base = emu_read32(BN6_CHIP_TABLE_REF);
	uint32_t n = base == CHIPS_RECORDS ? XREG_IDS : base == get32(R.data + (BN6_CHIP_TABLE_REF - BUS)) ? CHIPS_BN6_RECORDS : 0;
	if (!n) return "BN6's chip records' word";
	memset(out, 0, RECORDS_SIZE);
	for (uint32_t i = 0; i < n * BN6_CHIP_RECORD_SIZE; ++i) out[i] = emu_read8(base + i);
	for (int k = 0; k < xreg_count; ++k) {
		const XChipReg *r = &xreg_rows[k];
		if (r->bn5 >= BN5_CHIPS || r->bn6 >= XREG_IDS) return "a registry row";
		const uint8_t *from = rom5 + BN5_CHIP_RECORDS + BN6_CHIP_RECORD_SIZE * r->bn5;
		for (size_t b = 0; b < sizeof ds_bytes; ++b) out[BN6_CHIP_RECORD_SIZE * r->bn6 + ds_bytes[b]] = from[ds_bytes[b]];
	}
	for (uint32_t a = XN_DARK_RECORDS; a < XN_DARK_RECORDS + RECORDS_SIZE; ++a)
		if (emu_read8(a) != 0xFF) return "its records' room";
	return NULL;
}

bool xnavi_dark_install(int id) {
	static bool done;
	static uint8_t records[RECORDS_SIZE];
	if (done) return dark_id == id;
	done = true;
	const char *why = !R.data || get16(R.data + (BN6_NAVI_REGION - BUS)) != MOVS_R3_3 || get16(R.data + (BN6_NAVI_PLATE - BUS)) != MOVS_R0_0
		? "BN6's sites" : records_make(records);
	if (why) {
		fprintf(stderr, "xnavi: the Dark Soul's rules not made: %s not as expected\n", why);
		return false;
	}
	dark_id = id;
	claims_begin("the Dark Soul's rules (xnavi_dark.c)");
	emu_write(XN_DARK_RECORDS, records, sizeof records);
	bool set = emu_hook(BN6_NAVI_REGION, region, NULL) && emu_hook(BN6_NAVI_PLATE, plate, NULL);
	claims_end();
	if (!set) {
		fprintf(stderr, "xnavi: the Dark Soul's rules not made: a hook not set\n");
		emu_unhook(BN6_NAVI_REGION);
		dark_id = -1;
		return false;
	}
	return true;
}
