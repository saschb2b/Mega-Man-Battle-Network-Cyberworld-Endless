/* The layer's Recovery Mr.Prog's patch (layer_objs.h; issue #71,
 * docs/game/progression/before.md, Healing): how much it heals, and that
 * amount set again once a CONTINUE's state holds MegaMan's max HP. */
#include "layer_objs.h"

#include <stdint.h>

#include "bn6.h"
#include "emu.h"
#include "run.h"

/* Half of MegaMan's max HP, as BN6's RAM holds it now, 1 at least (the
 * owner's call on a player's proposal: away from the arena, so a detour's
 * HP carries on) */
static int heal_half(void) {
	int half = emu_read16(BN6_NAVI_MAX_HP) / 2;
	return half > 0 ? half : 1;
}

/* (to full before the arena, where the guardian is met as his band
 * assumes) */
int layer_heal_amount(void) {
	if (run.helpers & HELP_HEALS) return -1;
	if (layer.boss_layer) return 0;
	return heal_half();
}

/* (the archive's offsets first, two bytes a script, then the scripts'
 * bytes, ta_build's layout) */
void layer_heal_place(LayerObjs *out, uint32_t archive, int scripts, const HealSpot *spot) {
	uint32_t bytes = archive + (uint32_t)scripts * 2;
	bool placed = archive && spot->amount_at >= 0 && spot->got_at >= 0;
	out->heal_amount_at = placed ? bytes + (uint32_t)spot->amount_at : 0;
	out->heal_got_at = placed ? bytes + (uint32_t)spot->got_at : 0;
}

/* (the half was read as the layer was built, which a CONTINUE does before
 * its state loads, when the RAM holds no MegaMan: the patch gave 1 HP,
 * session 77) */
void layer_objs_heal_again(const LayerObjs *o) {
	if (!o->heal_amount_at) return;
	uint8_t a[2], got[HEAL_GOT_LEN];
	ta_heal_bytes(heal_half(), a, got);
	emu_write(o->heal_amount_at, a, sizeof a);
	emu_write(o->heal_got_at, got, sizeof got);
}
