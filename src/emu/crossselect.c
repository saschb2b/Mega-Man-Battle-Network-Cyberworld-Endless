/* crossselect.h. Gregar's CROSSSELECT and Falzar's are one code
 * (docs/rom/souls.md, CROSSSELECT): the list (BN6_CROSS_LIST) takes Cross
 * i of five whose flag in the table BN6_CROSS_FLAGS is set, whose used bit
 * is clear and whose form is not the one he began the battle in; the
 * commit writes i + 1 (0xD + i from a Beast Out) and ORs bit i into
 * BN6_CROSSES_USED. Falzar's reads its own flags, adds 6 and shifts the
 * bit by 5: so Gregar's own, made to read ten Crosses, Falzar's after its
 * five, commits Falzar's as forms 6-10 (0x12-0x16 from a Beast Out, its
 * Cross Beasts) and marks them in bits 5-9, Falzar's own. What reads the
 * Cross's number past the list is answered here for 5-9: its row's art
 * and palette, read from the free space in Gregar's place, and R's help,
 * whose text Gregar has for its five alone. */
#include "crossselect.h"

#include <stdio.h>
#include <string.h>

#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "free_space.h"
#include "rom.h"

#define BUS 0x08000000u
#define FLAGS_AT  FREE_CROSSSELECT                    /* the flags' table, a word a Cross */
#define PALS_AT   (FREE_CROSSSELECT + 0x40u)          /* Falzar's palettes, the five, then the five chosen */
#define ART_AT    (FREE_CROSSSELECT + 0x200u)         /* Falzar's rows' art, the five, then the five grey */
#define PAL_SIZE  0x20u
#define GREY      BN6_CROSS_ROWS                       /* the art's and the palettes' second five: grey, or the row chosen */
#define PAL_FALZAR (2 * BN6_CROSS_ROWS)                /* the palette number past Gregar's ten a Falzar Cross's is passed as */

_Static_assert(ART_AT + CROSSSELECT_ART_BYTES <= FREE_CROSSSELECT_END, "CROSSSELECT's art past its region");
_Static_assert(PALS_AT + CROSSSELECT_PAL_BYTES <= ART_AT, "CROSSSELECT's palettes past their room");

/* the instructions the hooks and patches stand on, as Gregar has them */
#define CMP_R4_5   0x2C05u  /* BN6_CROSS_LIST_BOUND, BN6_CROSS_COUNT_BOUND */
#define CMP_R4_10  0x2C0Au
#define LDR_R2_PC  0x4A0Cu  /* BN6_CROSS_LIST_STORE: ldr r2,[pc,#0x30] (=0x50) */
#define ADDS_R0_R6 0x1C30u  /* BN6_CROSS_COUNT_END */
#define ADDS_R0_R2 0x1880u  /* BN6_CROSS_PAL_PICK */

static bool installed;

bool crossselect_falzar(void) { return installed; }

/* (r6 the rows listed: five already, the next Cross is not) */
static HookAct list_store(HookRegs *r, void *user) {
	(void)user;
	if (r->r[6] < BN6_CROSS_ROWS) return HOOK_CONTINUE;
	r->r[12] = BN6_CROSS_LIST_NEXT | 1;
	return HOOK_JUMP;
}

/* (r6 the Crosses to list: five at most, as the list) */
static HookAct count_end(HookRegs *r, void *user) {
	(void)user;
	if (r->r[6] > BN6_CROSS_ROWS) r->r[6] = BN6_CROSS_ROWS;
	return HOOK_CONTINUE;
}

/* (r7 the Cross, r0 its art in Gregar's, grey past the cursor's row: a Falzar Cross's from the free space) */
static HookAct art_send(HookRegs *r, void *user) {
	(void)user;
	uint32_t cross = r->r[7];
	if (cross < BN6_CROSS_ROWS || cross >= BN6_CROSSES) return HOOK_CONTINUE;
	uint32_t grey = (r->r[0] - BN6_CROSS_ART) / BN6_CROSS_ART_SIZE >= cross + GREY ? GREY : 0;
	r->r[0] = ART_AT + (cross - BN6_CROSS_ROWS + grey) * BN6_CROSS_ART_SIZE;
	return HOOK_CONTINUE;
}

/* (r0 the Cross, r2 5 where its row is chosen: a Falzar Cross's palette passed on past Gregar's ten) */
static HookAct pal_pick(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] >= BN6_CROSS_ROWS && r->r[0] < BN6_CROSSES) r->r[0] += PAL_FALZAR - BN6_CROSS_ROWS;
	return HOOK_CONTINUE;
}

/* (r0 the palette's address by that number: a Falzar Cross's from the free space) */
static HookAct pal_send(HookRegs *r, void *user) {
	(void)user;
	uint32_t first = BN6_CROSS_PALS + PAL_FALZAR * PAL_SIZE;
	if (r->r[0] >= first && r->r[0] < first + 2u * CROSSSELECT_FALZAR * PAL_SIZE) r->r[0] = PALS_AT + (r->r[0] - first);
	return HOOK_CONTINUE;
}

/* (r1 the cursor's row's Cross: Gregar's text is its five's, R does nothing on Falzar's) */
static HookAct help(HookRegs *r, void *user) {
	(void)user;
	if (r->r[1] < BN6_CROSS_ROWS) return HOOK_CONTINUE;
	r->r[12] = BN6_CROSS_HELP_SKIP | 1;
	return HOOK_JUMP;
}

/* Gregar's code where the hooks and patches go, as Gregar has it */
static bool gregars(void) {
	return rom_u32(BN6_CROSS_FLAGS_REF - BUS) == BN6_CROSS_FLAGS && rom_u32(BN6_CROSS_FLAGS - BUS) == BN6_FLAG_HEAT_CROSS &&
		rom_u16(BN6_CROSS_LIST_BOUND - BUS) == CMP_R4_5 && rom_u16(BN6_CROSS_COUNT_BOUND - BUS) == CMP_R4_5 &&
		rom_u16(BN6_CROSS_LIST_STORE - BUS) == LDR_R2_PC && rom_u16(BN6_CROSS_COUNT_END - BUS) == ADDS_R0_R6 &&
		rom_u16(BN6_CROSS_PAL_PICK - BUS) == ADDS_R0_R2;
}

bool crossselect_install(const uint8_t *art, const uint8_t *pals) {
	if (installed) return true;
	if (!R.data || !emu_ready() || !art || !pals) return false;
	if (!gregars()) {
		fprintf(stderr, "crossselect: CROSSSELECT's code is not Gregar's as expected: Falzar's Crosses not listed\n");
		return false;
	}
	claims_begin(CROSSSELECT_CLAIMS);
	uint8_t flags[4 * BN6_CROSSES] = { 0 };
	for (int i = 0; i < BN6_CROSSES; ++i)
		put32(flags + 4 * i, (uint32_t)(i < BN6_CROSS_ROWS ? BN6_FLAG_HEAT_CROSS + i : BN6_FLAG_SPOUT_CROSS + i - BN6_CROSS_ROWS));
	emu_write(FLAGS_AT, flags, sizeof flags);
	emu_write(PALS_AT, pals, CROSSSELECT_PAL_BYTES);
	emu_write(ART_AT, art, CROSSSELECT_ART_BYTES);
	bool hooked = emu_hook(BN6_CROSS_LIST_STORE, list_store, NULL) && emu_hook(BN6_CROSS_COUNT_END, count_end, NULL) &&
		emu_hook(BN6_CROSS_ART_SEND, art_send, NULL) && emu_hook(BN6_CROSS_PAL_PICK, pal_pick, NULL) &&
		emu_hook(BN6_CROSS_PAL_SEND, pal_send, NULL) && emu_hook(BN6_CROSS_HELP, help, NULL);
	if (hooked) {
		/* (the table and the bounds last: nothing reads past Gregar's five
		 * before every reader of a Cross 5-9 is answered) */
		emu_write32(BN6_CROSS_FLAGS_REF, FLAGS_AT);
		uint8_t bound[2];
		put16(bound, CMP_R4_10);
		emu_write(BN6_CROSS_LIST_BOUND, bound, 2);
		emu_write(BN6_CROSS_COUNT_BOUND, bound, 2);
	}
	claims_end();
	installed = hooked;
	printf("crossselect: %s\n", hooked ? "Falzar's five listed beside Gregar's (flags 0xE7-0xEB, forms 6-10)" : "a hook NOT placed: Gregar's five alone");
	return hooked;
}
