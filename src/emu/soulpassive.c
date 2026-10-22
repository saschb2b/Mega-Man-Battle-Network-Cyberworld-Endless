/* A Soul's passives in BN6's engine (soulpassive.h). */
#include "soulpassive.h"

#include <stdint.h>
#include <stdio.h>

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "chips.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "soulcode.h"
#include "soulkinds.h"
#include "soullife.h"
#include "soulproxy.h"

#define BUS 0x08000000u
#define SOULS 6
#define SOUL_FIRST 7
#define THUMB_BX_LR 0x4770u
#define FREE_HALF 0xFFFFu    /* the free space as emu_init pads it */
#define SOUL_NUMBER 9
#define NUMBER_HAND 10       /* BN5's hand with NumberSoul (0x08025C24) */
#define NUMBER_PLUS 10       /* ... and its bonus to an attack chip of no kind (0x0800D0CA) */
#define NUMBER_LOOK 1        /* ... the bonus's mark (r7 there: 0x0800D0C8) */

static int breaks;
static bool done;

static int soul_on(int side) { return done && soul_united(side) ? hook_read8(soul_field(side, BN6_SOUL_ID)) : 0; }

bool soulpassive_bonus(uint32_t obj, int chip, int *bonus, int *look) {
	int soul = done ? soul_id_of(obj) : 0;
	if (!soul) return false;
	*bonus = 0;
	*look = 0;
	/* (BN5's: an attack chip, its power shown (+9's 0x02), of no kind as BN5 has it) */
	if (soul == SOUL_NUMBER && !(breaks & SOULPASSIVE_BREAK_PLUS) && chip > 0 && soulkinds_of(chip) == SOULKIND_NONE) {
		uint32_t rec = chips_table_bus() + (uint32_t)chip * BN6_CHIP_RECORD_SIZE;
		if (hook_read8(rec + BN6_CHIP_EFFECT_FLAGS) & BN6_CHIP_SHOWS_POWER) {
			*bonus = NUMBER_PLUS;
			*look = NUMBER_LOOK;
		}
	}
	return true;
}

/* ---- the passives a Soul gives as he unites, and as his form's state is set again ---- */

static uint32_t proxy_ready, proxy_reapply;            /* the proxy's own entries (HeatCross's, as soulproxy.c wrote them) */
static uint32_t ready_of[SOULS], reapply_of[SOULS];    /* each Soul's, BN5's moved; 0 none */

/* (BN6's per-form passives, entry 8 the stub, r5 MegaMan, called by mov lr,pc; bx) the Soul's, BN5's moved, or none:
 * on to BN6's routine of no overlay (a mov pc,lr: a bx lr would leave Thumb); the proxy's own without a Soul */
static HookAct passive(HookRegs *r, uint32_t own, const uint32_t *of) {
	int soul = done ? soul_id_of(r->r[5]) : 0;
	uint32_t to = own;
	if (soul >= SOUL_FIRST && soul < SOUL_FIRST + SOULS) to = (breaks & SOULPASSIVE_BREAK_READY) ? 0 : of[soul - SOUL_FIRST];
	r->r[12] = (to ? to : BN6_FORM_OVERLAY_NONE) | 1;
	return HOOK_JUMP;
}
static HookAct ready(HookRegs *r, void *user) { (void)user; return passive(r, proxy_ready, ready_of); }
static HookAct reapply(HookRegs *r, void *user) { (void)user; return passive(r, proxy_reapply, reapply_of); }

/* each Soul's routine in BN5's table `table5`, moved (0 for a mov pc,lr alone; ColonelSoul's ready, which the recipe
 * leaves, tools/xsoul.py's UNITS) */
static void souls_of(uint32_t table5, uint32_t out[SOULS]) {
	const uint8_t *b5 = XR[XROM_BN5_COLONEL_US].data;
	for (int k = 0; k < SOULS; ++k) {
		uint32_t e = get32(b5 + (table5 - BUS) + 4u * (uint32_t)(SOUL_FIRST + k));
		out[k] = get16(b5 + ((e & ~1u) - BUS)) == BN5_THUMB_RETURN ? 0 : soulcode_moved(e);
	}
}

/* the proxy's two passive entries pointed at stubs hooked to the Souls' */
static bool routes(void) {
	uint32_t at[2] = { BN6_FORM_PASSIVES + 4u * BN6_FORM_PROXY, BN6_FORM_PASSIVES_COLL + 4u * BN6_FORM_PROXY };
	uint32_t stub[2] = { SOUL_STUB_READY, SOUL_STUB_REAPPLY };
	proxy_ready = emu_read32(at[0]);
	proxy_reapply = emu_read32(at[1]);
	if (proxy_ready != emu_read32(BN6_FORM_PASSIVES + 4u * BN6_FORM_HEAT_CROSS) ||
		proxy_reapply != emu_read32(BN6_FORM_PASSIVES_COLL + 4u * BN6_FORM_HEAT_CROSS) || emu_read16(stub[0]) != FREE_HALF ||
		emu_read16(stub[1]) != FREE_HALF)
		return false;
	souls_of(BN5_SOUL_READY, ready_of);
	souls_of(BN5_SOUL_REAPPLY, reapply_of);
	const uint8_t bx[2] = { THUMB_BX_LR & 0xFF, THUMB_BX_LR >> 8 };
	emu_write(stub[0], bx, 2);
	emu_write(stub[1], bx, 2);
	/* (the entries pointed at the stubs only once both are hooked) */
	if (!emu_hook(stub[0], ready, NULL) || !emu_hook(stub[1], reapply, NULL)) return false;
	/* (the proxy's entries stay the Souls' proxy's, whose they are: claims.h) */
	claims_begin(SOULS_CLAIMS);
	emu_write32(at[0], stub[0] | 1);
	emu_write32(at[1], stub[1] | 1);
	claims_end();
	return true;
}

/* (the Custom screen's hand made, r0 its size: MegaMan's side's) NumberSoul's ten */
static HookAct hand(HookRegs *r, void *user) {
	(void)user;
	if (soul_on(hook_read8(BN6_BATTLE_SIDE) & 1) != SOUL_NUMBER) return HOOK_CONTINUE;
	r->r[0] = (breaks & SOULPASSIVE_BREAK_HAND) ? NUMBER_HAND - 1 : NUMBER_HAND;
	return HOOK_CONTINUE;
}

bool soulpassive_install(int brk) {
	if (done) return true;
	if (!R.data || !emu_ready()) return false;
	if (rom_u16(BN6_HAND_SIZE_SET - BUS) != 0x71A8) {
		fprintf(stderr, "soulpassive: %08x is not the instruction expected\n", (unsigned)BN6_HAND_SIZE_SET);
		return false;
	}
	breaks = brk;
	claims_begin(SOULPASSIVE_CLAIMS);
	bool ok = routes() && emu_hook(BN6_HAND_SIZE_SET, hand, NULL);
	claims_end();
	done = ok;
	printf("soulpassive: the Souls' passives%s%s\n",
		ok ? ": as each unites (ShadowSoul's FloatShoe, TomahawkSoul's grass, ToadSoul's sea), NumberSoul's hand of ten and its bonus" :
		" NOT placed whole", brk ? " (soulbreak: parts of it broken, the differential's own test)" : "");
	return ok;
}
