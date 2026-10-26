/* A battle MegaMan is spared in (battle_spare.h).
 *
 * BN6 deletes MegaMan in ProtoMan's netbattle as anywhere, and its GAME
 * OVER starts inside the battle as he falls (docs/RIVAL.md). The
 * statistics of October 2026 had that netbattle ending 30 of 88 lost runs,
 * for 24 players, 12 of them on layer 8, the third act's duel layer, where
 * it first stands (docs/CORE_LOOP.md): the owner's call, a lost netbattle
 * costs the HP and the clearance, never the run. So the deleting blow is
 * taken short of it, as a liberation's teammate is spared
 * (liberation_battle.c), and two hooks end the battle at the fight's next
 * frame: BN6's draw, its banner passed by. Both stand where a
 * liberation's hooks never lead (docs/ROM_DATA.md, BN6's battle loop and
 * banners), and only read what the director armed. */
#include "battle_spare.h"

#include <stdio.h>

#include "bn6_battle_loop.h"
#include "emu.h"
#include "hook.h"

static volatile bool armed;   /* (the director's) this battle spares MegaMan */
static volatile bool down;    /* (the hooks') its sparing blow came */

/* The fight's frame, nothing decided or asked: the blow come, on to the
 * draw, this frame's fight left as a liberation's time up leaves it */
static HookAct fight_going(HookRegs *r, void *user) {
	(void)user;
	if (!armed || !down) return HOOK_CONTINUE;
	hook_write32(BN6_LOOP, BN6_LOOP_DRAW);
	r->r[12] = BN6_FIGHT_RETURN | 1;
	return HOOK_JUMP;
}

/* The draw's banner: none, and the draw goes on to its result in the same
 * frame (no banner up to wait for) */
static HookAct draw_banner(HookRegs *r, void *user) {
	(void)user;
	if (!armed || !down) return HOOK_CONTINUE;
	r->r[12] = BN6_DRAW_TICK | 1;
	return HOOK_JUMP;
}

void battle_spare_install(void) {
	static bool done;
	if (done) return;
	done = true;
	if (!emu_hook(BN6_FIGHT_GOING, fight_going, NULL) || !emu_hook(BN6_DRAW_BANNER_SHOW, draw_banner, NULL))
		fprintf(stderr, "battle spare: the battle loop's hooks did not fit\n");
}

void battle_spare_arm(bool on) {
	armed = on;
	down = false;
}

bool battle_spare_down(void) { return down; }

uint32_t battle_spare_blow(uint32_t damage, uint32_t hp) {
	if (!armed || damage < hp) return damage;
	down = true;
	return hp - 1;
}
