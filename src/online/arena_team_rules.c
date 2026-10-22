/* --dev arena2v2 under full rules (arena_team_rules.h). */
#include "arena_team_rules.h"

#include "arena_team_navi.h"
#include "bn6.h"
#include "darkinvs.h"
#include "devtools.h"
#include "hook.h"
#include "soullife.h"

#define SIDES 2

uint32_t arena_team_rules_stats(uint32_t obj) {
	for (unsigned side = 0; side < SIDES; ++side) {
		if (!obj || obj != hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + BN6_SIDE_PARTNER)) continue;
		/* (his NaviStats, then his hand, in his room: arena_team_navi.c) */
		uint32_t hand = arena_team_navi_hand(side);
		return hand ? hand - BN6_BATTLE_NAVI_SIZE : 0;
	}
	return 0;
}

void arena_team_rules_begin(void) {
	soul_navi_stats_by(arena_team_rules_stats);
	/* (the audit's mutations, DEVTOOLS.md: chipaibreak=N a core's rooms of BN5's chip AI refused, partnerbreak=N its partners
	 * taken for their sides' players) */
	darkinvs_room_break(dev.chipai_break > 0 ? dev.chipai_break : -1);
	soul_partner_break(dev.partner_break > 0 ? dev.partner_break : -1);
}
