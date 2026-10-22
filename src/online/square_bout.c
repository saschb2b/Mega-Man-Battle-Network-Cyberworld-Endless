/* The pair a NetBattle from the Net Square builds, a 2v2's seats among it (square_bout.h). */
#include "square_bout.h"

#include <stdio.h>
#include <string.h>

#include "arena_seats.h"
#include "arena_team_client.h"
#include "bn6.h"
#include "devtools.h"
#include "emu.h"
#include "online_visit.h"
#include "rom.h"
#include "square_arena_state.h"
#include "square_team.h"

void square_bout_seats(const ArenaWireState *w) {
	SA.four = w->seats[2] && w->seats[3];
	SA.seat = SA.four ? w->side : w->side & 1u;
	memset(SA.seated_in, 0, sizeof SA.seated_in);
	memset(SA.seated_carried, 0, sizeof SA.seated_carried);
	for (unsigned k = 0; k < ONLINE_SEATS; ++k) {
		SA.seats[k] = w->seats[k];
		const char *n = k == SA.seat ? online_visit_name() : square_arena_name_of(w->seats[k]);
		if (!n) n = square_team_name(w->seats[k]);
		snprintf(SA.names[k], sizeof SA.names[k], "%s", n ? n : "?");
	}
}

bool square_bout_build(void) {
	const OnlineLoadout *loadout[2];
	unsigned side = SA.link.link.side;
	loadout[side] = SA.mine_carried ? &SA.mine : NULL;
	loadout[side ^ 1] = SA.theirs_carried ? &SA.theirs : NULL;
	/* (a 2v2: the GBAs' MegaMen seats 0 and 1's, the partners' seats 2 and 3's, the same on every game) */
	if (SA.four) {
		const OnlineLoadout *partner[2];
		for (unsigned k = 0; k < 2; ++k) {
			loadout[k] = SA.seated_carried[k] ? &SA.seated[k] : NULL;
			partner[k] = SA.seated_carried[2 + k] ? &SA.seated[2 + k] : NULL;
		}
		arena_seats_partners(partner);
	}
	arena_seats_set(SA.four);
	/* (the bout's seed its match, the same on both games and every
	 * spectator; arenaseedoff=N moves it on this side alone, a mutation) */
	uint32_t seed = SA.match + (uint32_t)dev.arena_seed_off;
	SA.arena = arena_new_bout(R.data, ROM_SIZE, loadout, &SA.link.link, seed, SA.kept);
	SA.kept = NULL;
	if (!SA.arena) return false;
	SA.counted = false;
	arena_window(SA.arena, dev.arena_window);
	/* (a 2v2: four streams; a partner's own made from my keys, his Custom screen's picks among them) */
	if (SA.four) arena_players(SA.arena, ARENA_PLAYERS);
	if (SA.four && SA.seat >= 2) arena_stream(SA.arena, team_client_stream, NULL);
	SA.stage = SA_CONNECTING;
	return true;
}

void square_bout_mine(OnlineLoadout *l) {
	/* (teampa=1 on a partner's game: his folder a Program Advance's first, as he carries it) */
	if (dev.team_pa > 0 && SA.four && SA.seat >= 2) arena_seats_pa_folder(l);
	/* (teamunite=1 on a partner's game: his MegaMan with NumberSoul and a Plus chip first, a UNITE in his first hand) */
	if (dev.team_unite > 0 && SA.four && SA.seat >= 2) arena_seats_unite_folder(l);
}

bool square_bout_ready(void) {
	return !SA.four || team_client_seat(SA.arena, SA.seat, SA.seated_carried[SA.seat & 3u] ? &SA.seated[SA.seat & 3u] : NULL);
}

uint32_t square_bout_bot(uint32_t keys, uint32_t tick) {
	keys = arena_bot_keys(SA.arena, SA.link.link.side, tick);
	/* (side by side: side 0's lead taps R every 600 frames of the fight, the next turn opened once the gauge is full, so
	 * that the partners pick again) */
	if (SA.four && arena_link_seat(&SA.link.link) == 0 && arena_phase(SA.arena, 0) != BN6_PHASE_CUSTOM && tick % 600 < 4) keys |= KEY_R;
	return keys;
}

void square_bout_after(ArenaStage stage) {
	/* (a 2v2's end: the two GBAs compared iteration by iteration, the partners' turns and my own screen's, in the log) */
	if (SA.four && stage != ARENA_BATTLE) team_client_report();
}

const char *square_bout_whose(void) {
	if (!SA.four) return SA.name;
	if (SA.stage == SA_BATTLE && SA.arena) return SA.names[arena_waiting_on(SA.arena) & 3u];
	for (unsigned k = 0; k < ONLINE_SEATS; ++k)
		if (k != SA.seat && (SA.stage == SA_LOADOUT ? !SA.seated_in[k] : !arena_link_net_seat_ready(&SA.link, k))) return SA.names[k];
	return SA.name;
}

void square_bout_free(void) { team_client_free(); }
