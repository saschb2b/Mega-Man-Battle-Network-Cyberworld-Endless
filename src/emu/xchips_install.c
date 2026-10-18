/* BN6's chip tables in the core's copy (xchips_install.h). */
#include "xchips_install.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "chips.h"
#include "claims.h"
#include "debug.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "xchips_code.h"
#include "xchips_registry.h"
#include "xinstall_log.h"
#include "xchips_record.h"
#include "xvirus_tx.h"

#define BUS 0x08000000u

/* The engine's own Thumb routines, at CHIPS_CODE (numbers: written for
 * this, assembled with devkitARM's as for the ARM7TDMI; docs/EMULATION.md):
 * +0x00 getOffsetToQuantityOfChipCode's tail (r2 the chip, r3 its code's
 * place: the count's address in BN6's pack below 320, in its extension
 * from 320, as if those ids followed on; pop {r4,r7,pc}); +0x16
 * GetTotalChipCount's (r0 the chip: its four counts summed; pop {r7,pc});
 * +0x38 the pack totals' count (r2, r3: r1 the count; mov pc,lr); +0x50
 * the pack's clear (BN6's ZeroFillByWord on the pack, then on the
 * extension's 0x900 bytes; pop {pc}); +0x62 the pseudo-chips' test (r2 the
 * chip: r1 1 for 0x170-0x19A, BN6's own pseudo-chips, else 0; mov pc,lr);
 * from +0x70 their literals: 320, the extension less 320 x 12, the
 * extension, 0x900, ZeroFillByWord (0x080008E1), 0x170. As they read:
 *   00 mov r7,r10; ldr r7,[r7,#0x4C]; ldr r1,=320; cmp r2,r1; blt 0c; ldr r7,=ext-320*12
 *   0c movs r1,#12; muls r1,r2; adds r7,r7,r1; adds r0,r7,r3; pop {r4,r7,pc}
 *   16 mov r7,r10; ldr r7,[r7,#0x4C]; ldr r1,=320; cmp r0,r1; blt 22; ldr r7,=ext-320*12
 *   22 movs r1,#12; muls r1,r0; adds r7,r7,r1; ldrb r0,[r7]; ldrb r1,[r7,#1]; adds r0,r0,r1;
 *      ldrb r1,[r7,#2]; adds r0,r0,r1; ldrb r1,[r7,#3]; adds r0,r0,r1; pop {r7,pc}
 *   38 mov r7,r10; ldr r7,[r7,#0x4C]; ldr r1,=320; cmp r2,r1; blt 44; ldr r7,=ext-320*12
 *   44 movs r1,#12; muls r1,r2; adds r7,r7,r1; adds r1,r7,r3; ldrb r1,[r1]; mov pc,lr
 *   50 bl 5e; ldr r0,=ext; ldr r1,=0x900; bl 5e; pop {pc}
 *   5e ldr r3,=ZeroFillByWord; bx r3
 *   62 movs r1,#0; ldr r3,=0x170; subs r3,r2,r3; cmp r3,#0x2B; bcs 6e; movs r1,#1
 *   6e mov pc,lr */
static const uint8_t code[136] = {
	0x57, 0x46, 0xff, 0x6c, 0x1a, 0x49, 0x8a, 0x42, 0x00, 0xdb, 0x1a, 0x4f, 0x0c, 0x21, 0x51, 0x43, 0x7f, 0x18, 0xf8, 0x18, 0x90, 0xbd,
	0x57, 0x46, 0xff, 0x6c, 0x15, 0x49, 0x88, 0x42, 0x00, 0xdb, 0x14, 0x4f, 0x0c, 0x21, 0x41, 0x43, 0x7f, 0x18, 0x38, 0x78, 0x79, 0x78,
	0x40, 0x18, 0xb9, 0x78, 0x40, 0x18, 0xf9, 0x78, 0x40, 0x18, 0x80, 0xbd, 0x57, 0x46, 0xff, 0x6c, 0x0c, 0x49, 0x8a, 0x42, 0x00, 0xdb,
	0x0c, 0x4f, 0x0c, 0x21, 0x51, 0x43, 0x7f, 0x18, 0xf9, 0x18, 0x09, 0x78, 0xf7, 0x46, 0x00, 0xf0, 0x05, 0xf8, 0x08, 0x48, 0x09, 0x49,
	0x00, 0xf0, 0x01, 0xf8, 0x00, 0xbd, 0x08, 0x4b, 0x18, 0x47, 0x00, 0x21, 0x07, 0x4b, 0xd3, 0x1a, 0x2b, 0x2b, 0x00, 0xd2, 0x01, 0x21,
	0xf7, 0x46, 0x40, 0x01, 0x00, 0x00, 0xe0, 0xfd, 0x00, 0x02, 0xe0, 0x0c, 0x01, 0x02, 0x00, 0x09, 0x00, 0x00, 0xe1, 0x08, 0x00, 0x08,
	0x70, 0x01, 0x00, 0x00,
};
enum { CODE_OFFSET = 0x00, CODE_TOTAL = 0x16, CODE_COUNT = 0x38, CODE_CLEAR = 0x50, CODE_PSEUDO = 0x62 };
#define ROUTINE(at) (CHIPS_CODE + (at) + 1u)   /* (Thumb) */
#define LO(v) (uint16_t)((v) & 0xFFFF)
#define HI(v) (uint16_t)((v) >> 16)

/* One change of BN6's: halfwords at `at`, BN6's own (`was`), then the
 * install's (`now`) */
typedef struct { uint32_t at; int n; uint16_t was[7], now[7]; } Change;

#define NOP 0x46C0          /* mov r8,r8 */
#define MOV_LR_PC 0x46FE    /* mov lr,pc: the call's return past the bx */

static const Change code_changes[] = {
	/* ldr r7,[pc,#4]; bx r7; nop; .word: on to the routine */
	{ BN6_PACK_OFFSET_TAIL, 5, { 0x4657, 0x6CFF, 0x210C, 0x4351, 0x187F }, { 0x4F01, 0x4738, NOP, LO(ROUTINE(CODE_OFFSET)), HI(ROUTINE(CODE_OFFSET)) } },
	/* ldr r7,[pc,#0]; bx r7; .word */
	{ BN6_PACK_TOTAL_TAIL, 4, { 0x4657, 0x6CFF, 0x210C, 0x4341 }, { 0x4F00, 0x4738, LO(ROUTINE(CODE_TOTAL)), HI(ROUTINE(CODE_TOTAL)) } },
	/* ldr r7,[pc,#8]; mov lr,pc; bx r7; b past the word; nop; .word */
	{ BN6_PACK_SUM_COUNT, 7, { 0x4657, 0x6CFF, 0x210C, 0x4351, 0x187F, 0x18F9, 0x7809 },
		{ 0x4F02, MOV_LR_PC, 0x4738, 0xE002, NOP, LO(ROUTINE(CODE_COUNT)), HI(ROUTINE(CODE_COUNT)) } },
	{ BN6_PACK_SUM_END, 2, { 0x0140, 0 }, { 0x0200, 0 } },
	{ BN6_PACK_LIST_END, 2, { 0x013A, 0 }, { 0x01FF, 0 } },
	/* ldr r2,[pc,#0]; bx r2; .word, for bl ZeroFillByWord; pop {pc} */
	{ BN6_PACK_CLEAR_CALL, 4, { 0xF7DE, 0xFE36, 0xBD00, 0x0000 }, { 0x4A00, 0x4710, LO(ROUTINE(CODE_CLEAR)), HI(ROUTINE(CODE_CLEAR)) } },
	/* (its ldr r3 now loads the routine) mov lr,pc; bx r3; nop for cmp; blt; movs r1,#1 */
	{ BN6_PSEUDO_TEST, 3, { 0x429A, 0xDB00, 0x2101 }, { MOV_LR_PC, 0x4718, NOP } },
	{ BN6_PSEUDO_POOL, 2, { 0x0170, 0 }, { LO(ROUTINE(CODE_PSEUDO)), HI(ROUTINE(CODE_PSEUDO)) } },
	{ BN6_PSEUDO_TEST_NAVI, 3, { 0x429A, 0xDB00, 0x2101 }, { MOV_LR_PC, 0x4718, NOP } },
	{ BN6_PSEUDO_POOL_NAVI, 2, { 0x0170, 0 }, { LO(ROUTINE(CODE_PSEUDO)), HI(ROUTINE(CODE_PSEUDO)) } },
	/* (the Library's P.A. tab's builder to 349: the registry's 350 on are Program Advances of library type 4 and no library
	 * number, which it would list at the slot before its buffer once formed: docs/ROM_DATA.md, BN5's Program Advances) */
	{ BN6_PA_LIBRARY_BOUND, 2, { 0x001E, 0 }, { 0x001D, 0 } },
};
#define NCODE (int)(sizeof code_changes / sizeof *code_changes)

/* The words that name the chip records and the names' and descriptions'
 * archives (each found by a scan of Gregar for the word; docs/ROM_DATA.md,
 * The chip registry): which (0 the records, 1-2 the names' archives, 3-4
 * the descriptions'), where */
static const struct { uint8_t which; uint32_t at; } refs[] = {
	{ 0, BN6_CHIP_TABLE_REF },
	{ 1, 0x08027D2Cu }, { 1, 0x0802C7C4u }, { 1, 0x08042038u }, { 1, 0x08042050u }, { 1, 0x080476B4u }, { 1, 0x08121F54u },
	{ 1, 0x08121FF4u }, { 1, 0x0812204Cu }, { 1, 0x08128194u },
	{ 2, 0x08027D30u }, { 2, 0x0802C7C8u }, { 2, 0x0804203Cu }, { 2, 0x080476B8u }, { 2, 0x08121F58u }, { 2, 0x08121FF8u },
	{ 2, 0x08122050u }, { 2, 0x08128198u },
	{ 3, 0x08027D50u }, { 3, 0x0811A9BCu }, { 3, 0x0811B3BCu },
	{ 4, 0x08027D54u }, { 4, 0x0811A9C0u }, { 4, 0x0811B3C0u },
};
#define NREFS (int)(sizeof refs / sizeof *refs)

/* a word's value in BN6 (`own`) or the install's */
static uint32_t ref_value(int which, bool own) {
	if (own) {
		const uint32_t bn6[5] = { R.layout->chip_data, R.layout->chip_names[0], R.layout->chip_names[1], R.layout->chip_descs[0], R.layout->chip_descs[1] };
		return BUS + bn6[which];
	}
	static const uint32_t ours[5] = { CHIPS_RECORDS, CHIPS_NAMES0, CHIPS_NAMES1, CHIPS_DESCS0, CHIPS_DESCS1 };
	return ours[which];
}

/* ---- the checks: every change BN6's own or every one the install's ---- */

static char why[160];

/* a change's state in the core's copy: 0 BN6's (the player's ROM's too),
 * 1 the install's, -1 neither */
static int change_state(const Change *c) {
	bool bn6 = true, ours = true;
	for (int i = 0; i < c->n; ++i) {
		uint32_t a = c->at + 2u * (uint32_t)i;
		uint16_t now = emu_read16(a), file = rom_u16(a - BUS);
		bn6 &= now == c->was[i] && file == c->was[i];
		ours &= now == c->now[i];
	}
	return bn6 ? 0 : ours ? 1 : -1;
}

static int ref_state(int i) {
	uint32_t now = emu_read32(refs[i].at);
	bool bn6 = now == ref_value(refs[i].which, true) && rom_u32(refs[i].at - BUS) == now;
	return bn6 ? 0 : now == ref_value(refs[i].which, false) ? 1 : -1;
}

/* every change and word in one state: 0 BN6's, 1 the install's, -1 not */
static int changes_state(void) {
	int all = -2;
	for (int i = 0; i < NCODE + NREFS; ++i) {
		int s = i < NCODE ? change_state(&code_changes[i]) : ref_state(i - NCODE);
		uint32_t at = i < NCODE ? code_changes[i].at : refs[i - NCODE].at;
		if (s < 0 || (all != -2 && s != all)) {
			snprintf(why, sizeof why, "%08x is neither BN6's own nor the install's (or not as the rest)", (unsigned)at);
			return -1;
		}
		all = s;
	}
	return all;
}

/* the place from `from` (`n` bytes) free: 0xFF, as emu.c pads the copy */
static bool place_free(uint32_t from, uint32_t n) {
	for (uint32_t a = from; a < from + n; a += 4)
		if (emu_read32(a) != 0xFFFFFFFFu) {
			snprintf(why, sizeof why, "%08x is not free", (unsigned)a);
			return false;
		}
	return true;
}

/* ---- the deck as it is dealt ---- */

static uint8_t sits[XREG_IDS / 8];   /* (the hook's copy, written between frames) */
static bool dark_shelved;            /* (BN5's DarkChips of their own kept in the deck: soulmood.c's shelf has them) */

/* the registry's own DarkChips taken out of the copy, where they are shelved */
static void sits_shelved(void) {
	for (int i = 0; dark_shelved && i < xreg_count; ++i)
		if (xreg_rows[i].how == XREG_DARK) sits[xreg_rows[i].bn6 >> 3] &= (uint8_t)~(1u << (xreg_rows[i].bn6 & 7));
}

void xchips_dark_shelved(bool on) {
	dark_shelved = on;
	const uint8_t *bits = chips_sit_bits();
	if (bits) memcpy(sits, bits, sizeof sits);
	sits_shelved();
}

static HookAct deal(HookRegs *r, void *user) {
	(void)r;
	(void)user;
	uint16_t deck[BN6_DECK_SIZE];
	for (uint32_t i = 0; i < BN6_DECK_SIZE; ++i) deck[i] = hook_read16(BN6_BATTLE_DECK + 2 * i);
	int reg = hook_read8(BN6_BATTLE_REGULAR_FIRST) ? 1 : 0, tags = hook_read8(BN6_BATTLE_TAGS_FIRST) ? 2 : 0;
	unsigned gone;
	int taken = xrec_deck_sit_out(deck, BN6_DECK_SIZE, sits, reg + tags, &gone);
	if (!taken) return HOOK_CONTINUE;
	for (uint32_t i = 0; i < BN6_DECK_SIZE; ++i) hook_write16(BN6_BATTLE_DECK + 2 * i, deck[i]);
	if (emu_debug_on()) fprintf(stderr, "chips: %d of the folder's chips sat out of the battle's deck\n", taken);
	/* (the Regular chip or a TagChip sat out: the deck's first chips are
	 * others now) */
	if (reg && (gone & 1)) hook_write8(BN6_BATTLE_REGULAR_FIRST, 0);
	if (tags && (gone >> reg & 3)) hook_write8(BN6_BATTLE_TAGS_FIRST, 0);
	return HOOK_CONTINUE;
}

/* ---- the install ---- */

static const uint8_t *installed_rom5;   /* the BN5 ROM the core's image was made with */
static uint32_t installed_len;          /* the image's bytes in the core's copy, 0 none */

/* the image, the routines and the changes written (all checked) */
static void write_all(const uint8_t *img, uint32_t len) {
	emu_write(CHIPS_FREE, img, len);
	emu_write(CHIPS_CODE, code, sizeof code);
	for (int i = 0; i < NCODE; ++i)
		for (int k = 0; k < code_changes[i].n; ++k) {
			uint16_t v = code_changes[i].now[k];
			uint8_t b[2] = { (uint8_t)v, (uint8_t)(v >> 8) };
			emu_write(code_changes[i].at + 2u * (uint32_t)k, b, 2);
		}
	for (int i = 0; i < NREFS; ++i) {
		uint32_t v = ref_value(refs[i].which, false);
		uint8_t b[4] = { (uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24) };
		emu_write(refs[i].at, b, sizeof b);
	}
}

static bool install(void) {
	if (!emu_ready() || !R.data || !R.layout) return false;
	/* (BN5's chips' moved code where its ROM is read: the families its moved chips' records name; again it does nothing) */
	xchips_code_install();
	const uint8_t *rom5 = XR[XROM_BN5_COLONEL_US].data;
	if (installed_len && installed_rom5 == rom5) return true;
	uint32_t len;
	const uint8_t *img = chips_image(&len);
	why[0] = 0;
	int state = img && len <= CHIPS_FREE_END - CHIPS_FREE ? changes_state() : -1;
	/* (the place: free past what an earlier install wrote, its code's room too) */
	bool fresh = state >= 0 && (installed_len ? place_free(CHIPS_FREE + installed_len, len > installed_len ? len - installed_len : 0)
	                                          : place_free(CHIPS_FREE, len));
	if (state < 0 || !fresh || (state == 1) != (installed_len != 0)) {
		fprintf(stderr, "chips: BN6's chip tables not moved: %s\n", why[0] ? why : "no image, or the core's copy changed since");
		return false;
	}
	claims_begin("the chip registry (xchips_install.c)");
	write_all(img, len);
	/* (BN6's own records where the game reads them now, as a range moved: what reads BN6's record at its own place, a
	 * trace or the differential's pokes, reads the copy) */
	xv_moved_add(ref_value(0, true), CHIPS_RECORDS, CHIPS_BN6_RECORDS * BN6_CHIP_RECORD_SIZE);
	memcpy(sits, chips_sit_bits(), sizeof sits);
	sits_shelved();
	if (!installed_len && !emu_hook(BN6_DECK_DEALT, deal, NULL)) fprintf(stderr, "chips: no hook on the deck's deal: BN5's chips play where they sit out\n");
	claims_end();
	installed_len = len;
	installed_rom5 = rom5;
	chips_installed(true);
	if (emu_debug_on()) fprintf(stderr, "chips: 512 records, names and art in the core's copy (%u bytes, BN5's %s)\n", (unsigned)len, rom5 ? "read" : "placeholders");
	return true;
}

bool xchips_install(void) {
	/* (written down for the Arena's cores as they are made: xinstall_log.h) */
	xlog_begin();
	bool ok = install();
	xlog_end();
	return ok;
}
