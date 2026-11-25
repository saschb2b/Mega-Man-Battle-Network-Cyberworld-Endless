/* A BN4 Soul's own attack for a chip he charged (soulcharged.h). */
#include "soulcharged.h"

#include <stdint.h>

#include "bn6_attacks.h"
#include "bn6_forms.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "soul_room.h"
#include "soulbn4.h"
#include "soullife.h"
#include "thumb_reloc.h"

#define BUS 0x08000000u
#define STUB_BL1 0x02          /* the stub's bls to its bx r3 */
#define STUB_BL2 0x08
#define STUB_BX 0x10
#define STUB_LITS 0x14         /* its literals: the attack's copy, BN6's family set, back past the hooked bl */

static uint32_t copies[SOULS_BN4];   /* each BN4 Soul's attack's copy, 0 none */

/* (BN6_CHARGED_CHIP_SET, r5 MegaMan, r7 his attack's variables, r0 the family the chip's setup returned) a chip charged in a BN4
 * Soul that has his own attack for it: on to the stub, which runs it and sets its family (BN4_CHARGED_FORM's test) */
static HookAct charged(HookRegs *r, void *user) {
	int k = soulbn4_k(soul_id_of(r->r[5]));
	if (k < 0 || !copies[k] || !hook_read8(r->r[7] + BN6_ATTACK_CHARGED) || soul_partner(r->r[5])) return HOOK_CONTINUE;
	r->r[3] = copies[k];
	r->r[12] = (uintptr_t)user | 1;
	return HOOK_JUMP;
}

bool soulcharged_install(void) {
	static bool done;
	if (done) return true;
	int n = 0;
	for (int k = 0; k < SOULS_BN4; ++k) n += (copies[k] = soulbn4_charged_copy(k)) != 0;
	if (!n) return true;
	/* (r3 the attack's copy, as the hook set it: bl it, then BN6's family set with what it returned, then back past the hooked
	 * bl; r0-r3 and lr free there, as the chip's setup left them) */
	static const uint16_t code[] = {
		0x46C0,           /* 00 nop */
		0, 0,             /* 02 bl the bx r3: the attack, r0 its family */
		0x4B03,           /* 06 ldr r3,[pc,#0xC]: BN6's family set */
		0, 0,             /* 08 bl the bx r3 */
		0x4B02,           /* 0C ldr r3,[pc,#8]: back */
		0x4718,           /* 0E bx r3 */
		0x4718,           /* 10 bx r3 */
		0x46C0,           /* 12 nop */
	};
	uint8_t out[SOULS_BN4_CHARGED_END - SOULS_BN4_CHARGED] = { 0 };
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i) put16(out + 2 * i, code[i]);
	put32(out + STUB_LITS, BN6_ATTACK_FAMILY_SET | 1);
	put32(out + STUB_LITS + 4, (BN6_CHARGED_CHIP_SET + 4) | 1);
	if (!thumb_bl_write(out, STUB_BL1, SOULS_BN4_CHARGED, SOULS_BN4_CHARGED + STUB_BX) ||
		!thumb_bl_write(out, STUB_BL2, SOULS_BN4_CHARGED, SOULS_BN4_CHARGED + STUB_BX))
		return false;
	/* (BN6's code the hook stands on: the bl to the family's set) */
	uint32_t to;
	if (!thumb_bl_read(R.data + (BN6_CHARGED_CHIP_SET - BUS), 0, BN6_CHARGED_CHIP_SET, &to) || to != BN6_ATTACK_FAMILY_SET) return false;
	claims_begin("BN4's Souls' own attacks for a chip charged (soulcharged.c)");
	emu_write(SOULS_BN4_CHARGED, out, sizeof out);
	bool ok = emu_hook(BN6_CHARGED_CHIP_SET, charged, (void *)(uintptr_t)SOULS_BN4_CHARGED);
	claims_end();
	return done = ok;
}
