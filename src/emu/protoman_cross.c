/* ProtoMan's cross (protoman_cross.h). At half his HP or less, with MegaMan
 * on the middle row's middle or front panel, BN6's ProtoMan lights an X round
 * MegaMan and dashes its two strokes (his action 0x0E, bn6f sub_80FC9C8),
 * his body the blade. Each stroke clears his collision region when
 * anything touches him, so the blade strikes once, and sets it again as he
 * crosses into the next column (bn6f sub_80FCB64). As he lands back on his
 * panel (bn6f sub_80FCC4C) BN6 restores his collision data but not his
 * region. A touch after the last stroke's last column leaves it cleared:
 * Silence's music, which blinds him every few frames, or MegaMan standing
 * where a stroke ends. With no region he is on no panel, and every attack
 * passes through him until his next cross. Seen on BN6 alone
 * (docs/ROM_DATA.md, ProtoMan's cross). BN6's own reset for an attack cut
 * short (bn6f sub_80FBCA2) sets his region to his panel, and the Cybeast's
 * charge sets its own as it ends; this hook does the same as the cross
 * ends, and nothing where the region is already there. */
#include "protoman_cross.h"

#include <stdbool.h>

#include "bn6.h"
#include "emu.h"
#include "rom.h"

#define REGION_NONE      0        /* BN6_COLL_REGION: no panel, nothing touches him */
#define REGION_OWN_PANEL 1        /* ... his own panel */
#define CROSS_END_OP     0x85D0   /* BN6_PROTOMAN_CROSS_END's strh r0,[r2,#0x2E] */

uint32_t protoman_cross_mended;

/* (r2 his collision data, the landing's just restored) */
static HookAct cross_ends(HookRegs *r, void *user) {
	(void)user;
	uint32_t coll = r->r[2];
	if (coll < BN6_EWRAM || coll >= BN6_EWRAM_END || hook_read8(coll + BN6_COLL_REGION) != REGION_NONE) return HOOK_CONTINUE;
	hook_write8(coll + BN6_COLL_REGION, REGION_OWN_PANEL);
	++protoman_cross_mended;
	return HOOK_CONTINUE;
}

void protoman_cross_install(void) {
	static bool done;
	if (done || !R.data) return;
	done = true;
	/* (only where the game is the one read: the strh the hook stands on) */
	if (rom_u16(BN6_PROTOMAN_CROSS_END - 0x08000000u) == CROSS_END_OP) emu_hook(BN6_PROTOMAN_CROSS_END, cross_ends, NULL);
}
