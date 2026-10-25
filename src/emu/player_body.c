/* BN6's player object in another Navi's sprite (player_body.h). */
#include "player_body.h"

#include <stdio.h>

#include "bn6.h"
#include "debug.h"
#include "emu.h"
#include "hook.h"

static volatile int body = -1;   /* (the hook's answer, set between frames) */
static bool hooked;

/* (the spawn's sprite_load: r6 the list x 4, r4 the number) */
static HookAct spawn_sprite(HookRegs *r, void *user) {
	(void)user;
	if (body >= 0 && r->r[6] == BN6_OW_NPC_LIST) r->r[4] = (uint32_t)body;
	return HOOK_CONTINUE;
}

void player_body(int sprite) {
	if (sprite == body) return;
	if (!hooked) {
		hooked = emu_hook(BN6_PLAYER_SPRITE_LOAD, spawn_sprite, NULL);
		if (!hooked) { fprintf(stderr, "player_body: the spawn's hook did not fit\n"); return; }
	}
	body = sprite;
	/* (the spawn again where it stands: its sprite loaded, its place and
	 * facing kept; it locks its interaction as a spawn does, and the next
	 * frame's update runs on) */
	emu_write8(BN6_PLAYER_MAIN, 0);
	if (emu_debug_on()) fprintf(stderr, "player_body: sprite %d\n", sprite);
}
