/* GutsSoul's rapid buster in BN6's engine (soulrapid.h). */
#include "soulrapid.h"

#include <stdint.h>
#include <stdio.h>

#include "bn4_souls.h"
#include "bn6.h"
#include "bn6_forms.h"
#include "bn6_link.h"
#include "bytes.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "soulbn4.h"
#include "soullife.h"
#include "xattack_grow.h"
#include "xchips_code.h"

#define BUS 0x08000000u
#define FORM_GUTS 2
#define KEY_B 2u              /* BN6_AI_PRESSED's B (BN4's AI +0x1E the same bit) */
#define REQUEST_B 2u          /* BN6_AI_FLAGS: the charged B's request, which BN6's requests answer at 0x080F17D6 */
#define OP_CMP_R7 0x2F00u     /* cmp r7,#imm8 */
#define OP_MOVS_R0 0x2000u    /* movs r0,#imm8 */
#define OP_CMP_R0 0x2800u     /* cmp r0,#imm8 */

static bool done;
static int breaks;
static int window_top;   /* BN4's window, read from its code (10) */
static int rapid_at;     /* ... the presses that ask it (6) */
static int family;       /* BN6's attack family of the unit, found by its routine (xattack_entry) */

/* whether `obj` is united with GutsSoul */
static bool guts(uint32_t obj) {
	int k = soulbn4_k(soul_id_of(obj));
	return k >= 0 && soulbn4_form(k) == FORM_GUTS && !soul_partner(obj);
}

/* (BN6_RAPID_COUNT, r4 his AI data, r5 him) BN4's 0x0800BE50-0x0800BE94: while its request does not stand, a B pressed counts
 * and opens the window, the window runs out to a count of 0, and six or more ask the rapid buster */
static HookAct count(HookRegs *r, void *user) {
	(void)user;
	uint32_t me = r->r[5], ai = r->r[4];
	if (!guts(me)) return HOOK_CONTINUE;
	uint32_t at_n = soul_field_of(me, BN6_SOUL_PRESSES), at_w = soul_field_of(me, BN6_SOUL_WINDOW);
	uint32_t req = hook_read32(ai + BN6_AI_FLAGS);
	int n = hook_read8(at_n), ask = rapid_at + ((breaks & SOULRAPID_BREAK_SEVEN) ? 1 : 0);
	if (n >= ask && (req & REQUEST_B)) return HOOK_CONTINUE;   /* (BN4's tst r6,#0x20: its request stands) */
	if (hook_read16(ai + BN6_AI_PRESSED) & KEY_B) {
		n = (n + 1) & 0xFF;
		hook_write8(at_n, (uint8_t)n);
		hook_write8(at_w, (uint8_t)window_top);
	}
	int w = hook_read8(at_w);
	if (w) {
		hook_write8(at_w, (uint8_t)--w);
		if (!w) hook_write8(at_n, (uint8_t)(n = 0));
	}
	if (n >= ask) {
		hook_write8(at_w, (uint8_t)n);
		hook_write32(ai + BN6_AI_FLAGS, req | REQUEST_B);
	}
	return HOOK_CONTINUE;
}

/* (BN6_RAPID_POWER, r5 him) the charged B's request answered: the rapid buster's family where his count stands (BN4's
 * request 0x20, answered first), set as his attack past the charged shot's power attack (BN6_RAPID_SET) */
static HookAct power(HookRegs *r, void *user) {
	(void)user;
	uint32_t me = r->r[5];
	if (!guts(me) || hook_read8(soul_field_of(me, BN6_SOUL_PRESSES)) < rapid_at + ((breaks & SOULRAPID_BREAK_SEVEN) ? 1 : 0))
		return HOOK_CONTINUE;
	r->r[0] = (uint32_t)family;
	r->r[12] = BN6_RAPID_SET | 1u;
	return HOOK_JUMP;
}

/* BN4's numbers read from its code: false where an instruction is not the one read */
static bool bn4_read(const uint8_t *b4) {
	uint16_t form = get16(b4 + (BN4_GUTS_RAPID_FORM - BUS)), win = get16(b4 + (BN4_GUTS_RAPID_WINDOW - BUS));
	uint16_t at = get16(b4 + (BN4_GUTS_RAPID_AT - BUS)), ask = get16(b4 + (BN4_GUTS_RAPID_ASK - BUS));
	if (form != (OP_CMP_R7 | FORM_GUTS) || (win & 0xFF00u) != OP_MOVS_R0 || (at & 0xFF00u) != OP_CMP_R0 || (ask & 0xFF00u) != OP_MOVS_R0)
		return false;
	window_top = win & 0xFF;
	rapid_at = at & 0xFF;
	return true;
}

/* BN6's family whose entry is the unit's copy of BN4's state, -1 none */
static int family_find(void) {
	uint32_t copy = xchips_bn4_moved_at(BN4_GUTS_RAPID | 1u);
	for (int f = XATTACK_BN4; copy && f < XATTACK_BN4_END; ++f)
		if ((emu_read32(xattack_entry(f)) | 1u) == (copy | 1u)) return f;
	return -1;
}

bool soulrapid_install(int brk) {
	if (done) return true;
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	if (!b4 || !emu_ready()) return true;
	/* (BN6's: ldrb r0,[r4,#8]; the bl 0x080117A4 as one word, its first halfword the low) */
	if (rom_u16(BN6_RAPID_COUNT - BUS) != 0x7A20 || rom_u32(BN6_RAPID_POWER - BUS) != 0xFFD7F71Fu || rom_u16(BN6_RAPID_SET - BUS) != 0x79F1) {
		fprintf(stderr, "soulrapid: BN6's code is not the code read\n");
		return false;
	}
	if (!bn4_read(b4)) {
		fprintf(stderr, "soulrapid: BN4's key handler is not the code read\n");
		return false;
	}
	family = family_find();
	if (family < 0) {
		printf("soulrapid: GutsSoul's rapid buster not moved (BN4's chips' units not installed)\n");
		return true;
	}
	breaks = brk;
	bool ok = emu_hook(BN6_RAPID_COUNT, count, NULL) && emu_hook(BN6_RAPID_POWER, power, NULL);
	done = ok;
	printf("soulrapid: GutsSoul's rapid buster%s (family %#x, %d presses, a window of %d)\n", ok ? "" : " NOT placed", family, rapid_at,
		window_top);
	return ok;
}
