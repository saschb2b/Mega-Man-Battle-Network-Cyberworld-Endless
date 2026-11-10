/* BN5's NaviCust program SoulT+1 as BN6's program 48 (xncp.h). */
#include "xncp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "debug.h"
#include "emu.h"
#include "free_space.h"
#include "lz.h"
#include "rom.h"
#include "xprograms.h"

#define BUS 0x08000000u

/* Its rooms: FREE_NCP the effect, the list's stub, the two shapes, then
 * the three tables grown; FREE_NCP_TEXT the names' archive, then the
 * descriptions */
#define AT_EFFECT  FREE_NCP
#define AT_LIST    (FREE_NCP + 0x30u)
#define AT_SHAPE   (FREE_NCP + 0x60u)
#define AT_SMALL   (FREE_NCP + 0xA0u)
#define AT_EFFECTS (FREE_NCP + 0xE0u)
#define AT_CODES   (FREE_NCP + 0x1B0u)
#define AT_RECORDS (FREE_NCP + 0x3A0u)
#define RECORDS    (4 * XPROGRAM_PROGRAMS)                 /* the records with program 48's four */
#define AT_NAMES   FREE_NCP_TEXT
#define NAMES_MAX  0x300u
#define AT_DESCS   (FREE_NCP_TEXT + NAMES_MAX)
#define DESCS_MAX  (FREE_NCP_TEXT_END - AT_DESCS)
_Static_assert(AT_RECORDS + RECORDS * BN6_NCP_RECORD <= FREE_NCP_END, "the grown tables fit their room");
_Static_assert(AT_CODES + XPROGRAM_PROGRAMS * BN6_NCP_CODE_ROW <= AT_RECORDS && AT_EFFECTS + 4 * XPROGRAM_PROGRAMS <= AT_CODES, "the grown tables lie apart");
_Static_assert(XPROGRAM_SOULT - 1 == BN6_NCP_RUN && XPROGRAM_SOULT == BN6_NCP_CODE_ROWS && XPROGRAM_SOULT == BN6_NCP_DESC_ENTRIES, "program 48 is each table's next");

/* The effect, the engine's own Thumb (numbers: written for this, assembled
 * with devkitARM's as for the ARM7TDMI): what BN5's routine 0x08140038
 * does, through BN6's accessors, called through r3 (out of a bl's reach
 * from here). As it reads:
 *   00 push {lr}; movs r0,#0; movs r1,#0x32; ldr r3,=get; bl 20; adds r2,r0,#1; cmp r2,#6; ble 14; movs r2,#6
 *   14 movs r0,#0; movs r1,#0x32; ldr r3,=set; bl 20; pop {pc}
 *   20 bx r3; nop
 *   24 .word BN6_NAVI_STAT_GET | 1, BN6_NAVI_STAT_SET | 1 */
static const uint8_t effect[0x24] = {
	0x00, 0xb5, 0x00, 0x20, 0x32, 0x21, 0x07, 0x4b, 0x00, 0xf0, 0x0a, 0xf8, 0x42, 0x1c, 0x06, 0x2a, 0x00, 0xdd, 0x06, 0x22,
	0x00, 0x20, 0x32, 0x21, 0x03, 0x4b, 0x00, 0xf0, 0x01, 0xf8, 0x00, 0xbd, 0x18, 0x47, 0xc0, 0x46,
};
_Static_assert(BN6_NAVI_TURN_EXTRA == 0x32, "the routine's offset is the extra turns'");

/* The list's stub, the engine's own Thumb: BN6's loop lists the key items
 * 0x91 up to RUN's (r4, just raised by one) and ends; jumped to from its
 * compare with r4 the next item, it sends the loop on past RUN's to
 * program 48's four, then back to RUN's, which stays the list's last, and
 * ends after it. As it reads:
 *   00 ldr r0,=run; cmp r4,r0; bne 0a; adds r4,#4; b 18            (RUN's turn: program 48's first)
 *   0a adds r0,#1; cmp r4,r0; beq 1c                                (RUN listed: the end)
 *   10 adds r0,#7; cmp r4,r0; bne 18; subs r4,#8                    (program 48's four listed: RUN)
 *   18 ldr r0,=loop; bx r0
 *   1c ldr r0,=done; bx r0
 *   20 .word BN6_NCMENU_RUN, BN6_NCMENU_LIST_LOOP | 1, BN6_NCMENU_LIST_DONE | 1 */
static const uint8_t list[0x20] = {
	0x07, 0x48, 0x84, 0x42, 0x01, 0xd1, 0x04, 0x34, 0x06, 0xe0, 0x01, 0x30, 0x84, 0x42, 0x05, 0xd0,
	0x07, 0x30, 0x84, 0x42, 0x00, 0xd1, 0x08, 0x3c, 0x02, 0x48, 0x00, 0x47, 0x02, 0x48, 0x00, 0x47,
};
_Static_assert(4 * XPROGRAM_SOULT - 4 == BN6_NCMENU_RUN - BN6_PROGRAM_ITEMS, "program 48's key items are the four after RUN's");
#define LIST_CMP 0x4284u   /* BN6's `cmp r4, r0` at BN6_NCMENU_LIST_NEXT, */
#define LIST_BX  0x4700u   /* ... made `bx r0`: the bound's word holds the stub */

static const uint32_t record_refs[] = BN6_NCP_RECORD_REFS, name_refs[] = BN6_NCP_NAME_REFS;
#define NRECORD_REFS (int)(sizeof record_refs / sizeof *record_refs)
#define NNAME_REFS   (int)(sizeof name_refs / sizeof *name_refs)

static bool installed;

bool xncp_installed(void) { return installed; }

static bool fail(const char *why, uint32_t at) {
	fprintf(stderr, "navicust: program 48 not installed: %s (%08x)\n", why, (unsigned)at);
	return false;
}

/* BN6's own everywhere a word or a table is about to change, in the core's
 * copy (another installer's bytes there, or a ROM that is not Gregar's,
 * and nothing is written) */
static bool all_bn6(uint32_t records, uint32_t codes, uint32_t names) {
	if (emu_read32(BN6_NCP_EFFECTS_REF) != BN6_NCP_EFFECTS) return fail("the effects' word is not BN6's", BN6_NCP_EFFECTS_REF);
	if (emu_read32(BN6_NCP_CODES_REF) != codes) return fail("the codes' word is not BN6's", BN6_NCP_CODES_REF);
	if (emu_read32(BN6_NCP_DESCS_SHOP_REF) != BN6_NCP_DESCS) return fail("a shop's word is not the descriptions'", BN6_NCP_DESCS_SHOP_REF);
	if (emu_read32(BN6_NCP_DESCS_MENU_REF) != (BN6_NCP_DESCS | 0x80000000u)) return fail("the NaviCust's word is not the descriptions'", BN6_NCP_DESCS_MENU_REF);
	for (int i = 0; i < NRECORD_REFS; ++i)
		if (emu_read32(record_refs[i]) != records) return fail("a word is not the records'", record_refs[i]);
	if (emu_read16(BN6_NCMENU_LIST_NEXT) != LIST_CMP || emu_read32(BN6_NCMENU_LIST_BOUND) != BN6_NCMENU_RUN) return fail("the list's loop is not BN6's", BN6_NCMENU_LIST_NEXT);
	for (uint32_t i = 0; i < BN6_NCP_RECORDS_OWN * BN6_NCP_RECORD; ++i)
		if (emu_read8(records + i) != R.data[records - BUS + i]) return fail("a record is not BN6's", records + i);
	for (int i = 0; i < NNAME_REFS; ++i)
		if (emu_read32(name_refs[i]) != names) return fail("a word is not the names' archive's", name_refs[i]);
	for (uint32_t i = FREE_NCP; i < FREE_NCP_END; ++i)
		if (emu_read8(i) != 0xFF) return fail("its room is not free", i);
	for (uint32_t i = FREE_NCP_TEXT; i < FREE_NCP_TEXT_END; ++i)
		if (emu_read8(i) != 0xFF) return fail("its words' room is not free", i);
	return true;
}

/* BN6's descriptions with program 48's after them, as the game unpacks
 * them: LZ77 of literal blocks into `out` (DESCS_MAX); its length, 0 where
 * BN6's do not read or the whole does not fit */
static size_t descs_make(const XProgram *p, uint8_t *out) {
	size_t n = 0, len = 0;
	uint8_t *raw = lz77_decompress(R.data + (BN6_NCP_DESCS - BUS), ROM_SIZE - (BN6_NCP_DESCS - BUS), &n);
	uint8_t grown[2048];
	const XArchive own = { raw, n, BN6_NCP_DESC_ENTRIES };
	size_t gn = raw ? xprogram_descs_grow(&own, BN6_NCP_DESC_FRAMED, p->about, p->about_len, grown, sizeof grown) : 0;
	/* (lz_literal writes whole blocks of eight, a flag byte each, then pads to four) */
	if (gn && 4 + 9 * ((gn + 7) / 8) + 3 <= DESCS_MAX) len = lz_literal(grown, gn, out);
	free(raw);
	return len;
}

bool xncp_install(void) {
	installed = false;
	const XProgram *p = xprogram_soult();
	if (!emu_ready() || !R.data || !R.layout || !R.layout->navicust_programs || !R.layout->navicust_codes || !R.layout->navicust_names) return true;
	if (!XR[XROM_BN5_COLONEL_US].data) return true;   /* (BN6 alone: its 48 numbers as they are) */
	if (!p) return fail("BN5's record of SoulT+1 does not read as one", 0);
	uint32_t records = BUS + R.layout->navicust_programs, codes = BUS + R.layout->navicust_codes, names = BUS + R.layout->navicust_names;
	if (emu_read32(BN6_NCP_EFFECTS_REF) == AT_EFFECTS) return installed = true;   /* (ours already: a core written twice) */
	if (!all_bn6(records, codes, names)) return false;
	/* (everything made before a byte is written) */
	static uint8_t table[RECORDS * BN6_NCP_RECORD], descs[DESCS_MAX], archive[NAMES_MAX];
	uint8_t block[0x2C], stub[0x2C], effects[4 * XPROGRAM_PROGRAMS], rows[XPROGRAM_PROGRAMS * BN6_NCP_CODE_ROW];
	memcpy(block, effect, sizeof effect);
	put32(block + 0x24, BN6_NAVI_STAT_GET | 1u);
	put32(block + 0x28, BN6_NAVI_STAT_SET | 1u);
	memcpy(stub, list, sizeof list);
	put32(stub + 0x20, BN6_NCMENU_RUN);
	put32(stub + 0x24, BN6_NCMENU_LIST_LOOP | 1u);
	put32(stub + 0x28, BN6_NCMENU_LIST_DONE | 1u);
	/* (the effects: BN6's 47, RUN's number given program 0's routine, which does nothing, then ours) */
	for (uint32_t i = 0; i < BN6_NCP_EFFECTS_OWN; ++i) put32(effects + 4 * i, emu_read32(BN6_NCP_EFFECTS + 4 * i));
	put32(effects + 4 * BN6_NCP_RUN, emu_read32(BN6_NCP_EFFECTS));
	put32(effects + 4 * XPROGRAM_SOULT, AT_EFFECT | 1u);
	/* (the codes: BN6's 48 rows, then a row of none) */
	memcpy(rows, R.data + R.layout->navicust_codes, BN6_NCP_CODE_ROWS * BN6_NCP_CODE_ROW);
	memset(rows + BN6_NCP_CODE_ROWS * BN6_NCP_CODE_ROW, 0xFF, BN6_NCP_CODE_ROW);
	/* (the records: BN6's 189, RUN's other three of no colour, then program 48's four with its shapes) */
	memset(table, 0, sizeof table);
	memcpy(table, R.data + R.layout->navicust_programs, BN6_NCP_RECORDS_OWN * BN6_NCP_RECORD);
	for (uint32_t v = 0; v < XPROGRAM_VARIANTS; ++v) {
		uint8_t *rec = table + (4u * XPROGRAM_SOULT + v) * BN6_NCP_RECORD;
		memcpy(rec, p->head[v], 8);
		put32(rec + BN6_NCP_RECORD_SHAPE, AT_SHAPE);
		put32(rec + BN6_NCP_RECORD_SMALL, AT_SMALL);
	}
	size_t dlen = descs_make(p, descs);
	if (!dlen) return fail("the descriptions do not read, or do not fit with one more", BN6_NCP_DESCS);
	const XArchive own = { R.data + R.layout->navicust_names, BN6_NCP_NAMES_SIZE, BN6_NCP_NAME_ENTRIES };
	size_t nlen = xprogram_name_add(&own, XPROGRAM_SOULT, p->name, p->name_len, archive, sizeof archive);
	if (!nlen) return fail("the names' archive does not take the name", names);
	claims_begin("BN5's SoulT+1 as program 48 (xncp.c)");
	emu_write(AT_EFFECT, block, sizeof block);
	emu_write(AT_LIST, stub, sizeof stub);
	emu_write(AT_SHAPE, p->shape, XPROGRAM_SHAPE);
	emu_write(AT_SMALL, p->small, XPROGRAM_SHAPE);
	emu_write(AT_EFFECTS, effects, sizeof effects);
	emu_write(AT_CODES, rows, sizeof rows);
	emu_write(AT_RECORDS, table, sizeof table);
	emu_write(AT_NAMES, archive, nlen);
	emu_write(AT_DESCS, descs, dlen);
	emu_write32(BN6_NCP_EFFECTS_REF, AT_EFFECTS);
	emu_write32(BN6_NCP_CODES_REF, AT_CODES);
	emu_write32(BN6_NCP_DESCS_SHOP_REF, AT_DESCS);
	emu_write32(BN6_NCP_DESCS_MENU_REF, AT_DESCS | 0x80000000u);
	for (int i = 0; i < NRECORD_REFS; ++i) emu_write32(record_refs[i], AT_RECORDS);
	for (int i = 0; i < NNAME_REFS; ++i) emu_write32(name_refs[i], AT_NAMES);
	emu_write32(BN6_NCMENU_LIST_BOUND, AT_LIST | 1u);
	uint8_t bx[2];
	put16(bx, LIST_BX);
	emu_write(BN6_NCMENU_LIST_NEXT, bx, sizeof bx);
	claims_end();
	if (emu_debug_on()) fprintf(stderr, "navicust: BN5's SoulT+1 as program %d (its descriptions %u bytes)\n", XPROGRAM_SOULT, (unsigned)dlen);
	return installed = true;
}
