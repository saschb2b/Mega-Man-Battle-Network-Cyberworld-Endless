/* SearchSoul's SHUFFLE on BN6's Custom screen (soulshuffle.h). */
#include "soulshuffle.h"

#include <stdint.h>
#include <stdio.h>

#include "bn6.h"
#include "bn6_forms.h"
#include "emu.h"
#include "rom.h"
#include "soullife.h"
#include "soulpower.h"
#include "soulteam.h"

#define BUS 0x08000000u
#define SHUFFLES 3          /* BN5's a turn for SearchSoul (0x08023D0C: movs r0,#3), less those taken (the screen's +0x16) */

static int breaks;
static bool done;

static bool searching(void) {
	int side = hook_read8(BN6_BATTLE_SIDE) & 1;
	return done && !(breaks & SOULPOWER_BREAK_SHUFFLE) && soul_united(side) && hook_read8(soul_field(side, BN6_SOUL_ID)) == SOUL_SEARCH;
}

/* (BN6_SHUFFLE_TEST, its first instruction) the button stands for SearchSoul (BN5's 0x08023CFC: the united Soul 3) */
static HookAct test(HookRegs *r, void *user) {
	(void)user;
	if (!searching()) return HOOK_CONTINUE;
	r->r[0] = 1;
	return HOOK_RETURN;
}

/* (BN6_SHUFFLE_COUNT) BN5's three a turn for him, on to the subtraction */
static HookAct count(HookRegs *r, void *user) {
	(void)user;
	if (!searching()) return HOOK_CONTINUE;
	r->r[0] = SHUFFLES;
	r->r[12] = BN6_SHUFFLE_COUNT_NEXT | 1;
	return HOOK_JUMP;
}

bool soulshuffle_install(int brk) {
	if (done) return true;
	if (rom_u16(BN6_SHUFFLE_TEST - BUS) != 0xB510 || rom_u16(BN6_SHUFFLE_COUNT - BUS) != 0x2001) {
		fprintf(stderr, "soulshuffle: SearchSoul's SHUFFLE not in BN6's engine: Gregar's test or count is not as expected\n");
		return false;
	}
	breaks = brk;
	done = emu_hook(BN6_SHUFFLE_TEST, test, NULL) && emu_hook(BN6_SHUFFLE_COUNT, count, NULL);
	if (done) printf("soulshuffle: SearchSoul's SHUFFLE: BN6's own button, three a turn\n");
	return done;
}
