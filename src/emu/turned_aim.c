/* turned_aim.h: MegaMan's side's aim between a Surrounded field's two sides. */
#include "turned_aim.h"

#include <stdio.h>

#include "bn6.h"
#include "bn6_forms.h"
#include "debug.h"
#include "emu.h"
#include "hook.h"

/* (BN6's search of a row, at its start's adds r0,#6: the object pushed with r4 and lr at the routine's entry, at [sp+4]) */
static HookAct row_start(HookRegs *r, void *user) {
	(void)user;
	uint32_t o = hook_read32(r->sp + 4), settings = hook_read32(BN6_BATTLE_SETTINGS);
	if (!settings || !turned_aim_mends(hook_read8(settings + BN6_SETTINGS_SIDES), hook_read8(o + BN6_T1_ALLIANCE))) return HOOK_CONTINUE;
	r->r[0] = hook_read8(o + BN6_T1_PANEL_X);
	if (emu_debug_on()) fprintf(stderr, "row search: from column %u, the object's own, step %d\n", (unsigned)r->r[0], (int)(int32_t)r->r[5]);
	r->r[12] = BN6_ROW_SEARCH_LOOP | 1;
	return HOOK_JUMP;
}

/* (object_getEnemyDirection's first instruction, r5 the object: the way it faces, as object_getFrontDirection gives it,
 * while MegaMan stands turned round; unturned, his battle runs as BN6's, his side's objects that face away by their own
 * flip (DblBeast's) aimed by side as BN6 aims them) */
static HookAct enemy_direction(HookRegs *r, void *user) {
	(void)user;
	uint32_t o = r->r[5], settings = hook_read32(BN6_BATTLE_SETTINGS), megaman = hook_read32(BN6_BATTLE_PLAYERS);
	if (!settings || !turned_aim_mends(hook_read8(settings + BN6_SETTINGS_SIDES), hook_read8(o + BN6_T1_ALLIANCE))) return HOOK_CONTINUE;
	if (!megaman || !(hook_read8(megaman + BN6_T1_FLIP) & 1)) return HOOK_CONTINUE;
	r->r[0] = hook_read8(o + BN6_T1_FLIP) & 1 ? (uint32_t)-1 : 1u;
	return HOOK_RETURN;
}

void turned_aim_install(void) {
	if (!emu_hook(BN6_ENEMY_DIRECTION, enemy_direction, NULL))
		fprintf(stderr, "turned aim: no hook at %08x: chips aim by side alone on a Surrounded field\n", (unsigned)BN6_ENEMY_DIRECTION);
	if (!emu_hook(BN6_ROW_SEARCH_START, row_start, NULL))
		fprintf(stderr, "turned aim: no hook at %08x: GolmHit's fist strikes behind MegaMan on a Surrounded field\n", (unsigned)BN6_ROW_SEARCH_START);
}
