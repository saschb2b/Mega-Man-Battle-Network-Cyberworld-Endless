/* --dev arena2v2: the partners' own clients, stood in for (arena_team_client.h). */
#include "arena_team_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/core.h>

#include "arena_carry.h"
#include "arena_format.h"
#include "arena_pair.h"
#include "arena_team.h"
#include "arena_team_custom.h"
#include "arena_team_navi.h"
#include "arena_team_navicust.h"
#include "arena_team_screen.h"
#include "arena_team_view.h"
#include "arena_view.h"
#include "bn6.h"
#include "devtools.h"
#include "emu.h"
#include "rom.h"

#define SIDES 2

typedef struct {
	TeamScreen *screen;
	bool turn;                        /* his side's GBA's Custom screen open, his own set up for it */
	uint16_t out[TEAM_CUSTOM_PICKS];  /* his pick messages to go out, one a frame */
	unsigned queued, sent;
	unsigned opened, oks, closed_first, at_limit;
	unsigned frames;                  /* his turn's frames so far */
} Client;

static Client clients[SIDES];

/* teamseat's seat: 0 the first GBA's player (as without it) to 3 */
static unsigned seat(void) { return dev.team_seat > 0 && dev.team_seat < 4 ? (unsigned)dev.team_seat : 0; }

/* side's partner's NaviCust compiled by BN6's own code on a core of his
 * client from his loadout and the checkpoint's state (once a run of the
 * game: a rematch's partners the same), said in the output */
static void compile(unsigned side, const OnlineLoadout *l) {
	static bool tried[SIDES], made[SIDES];
	static uint8_t stats[SIDES][BN6_BATTLE_NAVI_SIZE];
	if (!tried[side]) {
		tried[side] = true;
		size_t n;
		uint8_t *state = arena_carry_state(&n);
		char why[128] = "";
		made[side] = state && team_navicust_compile(R.data, ROM_SIZE, state, n, l, stats[side], why, sizeof why);
		free(state);
		const uint8_t *s = stats[side];
		if (made[side])
			printf("arena2v2: side %u's partner's NaviCust compiled by BN6 (%08X) on his own core: HP %u (base %u), Custom %u, buster "
				"%u %u %u, SuperArmor %u, FloatShoe %u, AirShoe %u, UnderShirt %u\n", side, BN6_NAVICUST_RELOAD,
				s[BN6_NS_MAX_HP] | s[BN6_NS_MAX_HP + 1] << 8, s[BN6_NS_BASE_HP] | s[BN6_NS_BASE_HP + 1] << 8, s[BN6_NS_CUSTOM],
				s[BN6_NS_ATTACK] + 1, s[BN6_NS_SPEED] + 1, s[BN6_NS_CHARGE] + 1, s[BN6_NS_SUPER_ARMOR], s[BN6_NS_FLOAT_SHOE],
				s[BN6_NS_AIR_SHOE], s[BN6_NS_UNDER_SHIRT]);
		else printf("arena2v2: side %u's partner's NaviCust not compiled (%s): his record his loadout's HP and buster\n", side,
			*why ? why : "no checkpoint");
	}
	arena_team_navi_compiled(side, made[side] ? stats[side] : NULL);
}

void team_client_begin(Arena *a, const OnlineLoadout *all) {
	/* (teambuster=N: both partners' buster at Attack N, their HP their sides', the loadout's numbers seen in the battle) */
	const uint8_t buster[3] = { (uint8_t)(dev.team_buster - 1), 0, 0 };
	for (unsigned side = 0; side < SIDES; ++side) {
		arena_team_custom_folder(side, NULL, 0, NULL);
		arena_team_navi_loadout(side, 0, 0, dev.team_buster > 0 && dev.team_buster <= 5 ? buster : NULL);
		arena_team_navi_compiled(side, NULL);
	}
	/* (teamcust: side 1's partner the checkpoint's MegaMan's NaviCust too, his folder his lead's) */
	if (all && dev.team_cust) compile(1, all);
	unsigned fits = all ? arena_folders_fitting(all) : 0;
	/* (the folder after the one his lead carries: arena_scene's, the first that holds chips) */
	for (int k = 0, first = -1; k < ONLINE_FOLDERS; ++k) {
		if (!(fits >> k & 1)) continue;
		if (first < 0) { first = k; continue; }
		OnlineLoadout own;
		arena_format_carry(all, k, &own);
		arena_team_custom_folder(0, own.folder, own.regular, own.tag);
		const uint8_t his[3] = { own.attack, own.speed, own.charge };
		arena_team_navi_loadout(0, own.max_hp, own.base_hp, his);
		compile(0, &own);
		printf("arena2v2: side 0's partner carries the run's folder %d, his lead folder %d\n", k + 1, first + 1);
		break;
	}
	if (seat()) arena_show(a, (int)arena_team_view_side(seat()));
}

void team_client_free(void) {
	for (unsigned s = 0; s < SIDES; ++s) team_screen_free(clients[s].screen);
	memset(clients, 0, sizeof clients);
}

/* his keys on his screen, by the frame's number alone: the cursor a chip
 * right and A every 20 frames (BN6 takes the chips that fit, and at last OK) */
static uint16_t screen_keys(uint32_t frame) {
	unsigned t = frame % 20;
	return t < 2 ? KEY_A : t >= 10 && t < 12 ? KEY_RIGHT : 0;
}

static void say(Client *c, const uint8_t *picks, unsigned n) {
	c->queued = c->sent = 0;
	if (!n) c->out[c->queued++] = TEAM_PICK(0, 0, 0);
	for (unsigned i = 0; i < n && i < TEAM_CUSTOM_PICKS; ++i) c->out[c->queued++] = TEAM_PICK(n, i, picks[i]);
}

/* his side's GBA's screen closed before his OK: his picks so far go out */
static void closed_first(Client *c) {
	if (!c->turn || !team_screen_open_now(c->screen)) return;
	uint8_t picks[TEAM_CUSTOM_PICKS];
	unsigned n = team_screen_picks(c->screen, picks);
	++c->closed_first;
	say(c, picks, n);
	team_screen_close(c->screen);
}

/* whether side's partner is on the field: his object there, HP left */
static bool standing(struct mCore *gba, unsigned side) {
	uint32_t o = (uint32_t)gba->rawRead32(gba, BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + BN6_SIDE_PARTNER, -1);
	return o >= BN6_EWRAM && o < BN6_EWRAM_END && gba->rawRead16(gba, o + BN6_T1_HP, -1);
}

uint16_t team_client_keys(Arena *a, unsigned player, uint32_t frame) {
	unsigned side = (player - 2) & 1u;
	Client *c = &clients[side];
	struct mCore *gba = arena_core(a, side);
	/* (a partner deleted has no turn) */
	bool custom = gba && gba->rawRead8(gba, BN6_BATTLE_PHASE, -1) == BN6_PHASE_CUSTOM && standing(gba, side);
	/* (his screen made once, from the pair as it is at a Custom screen) */
	if (custom && !c->screen) c->screen = team_screen_new(R.data, ROM_SIZE, arena_pair_of(a), side);
	if (!custom || !c->screen) {
		closed_first(c);
		c->turn = false;
	} else if (!c->turn) {
		/* (once the pair has dealt his deck: its first frame of the battle) */
		uint16_t deck[BN6_FOLDER_ENTRIES];
		unsigned n = arena_team_custom_deck(gba, side, deck);
		c->turn = n && team_screen_open(c->screen, arena_pair_of(a), deck, n);
		c->queued = c->sent = 0;
		c->frames = 0;
		c->opened += c->turn;
	}
	if (c->sent < c->queued) return c->out[c->sent++];
	if (!c->turn) return arena_team_script(player, frame);
	uint8_t picks[TEAM_CUSTOM_PICKS];
	unsigned n;
	/* (his lead held for him near TEAM_WAIT_LIMIT: the picks he has go out, in time to be his; teamslow=-1, a client gone
	 * quiet, sends none, and the pair's own limit ends the wait) */
	bool gone = !side && dev.team_slow < 0;
	if (!gone && team_screen_open_now(c->screen) && arena_team_custom_waited(gba, side) + TEAM_WAIT_SAY >= TEAM_WAIT_LIMIT) {
		n = team_screen_picks(c->screen, picks);
		++c->at_limit;
		printf("arena2v2: side %u's partner's client: %u frames held for him, his %u picks so far sent before the wait's limit\n", side,
			arena_team_custom_waited(gba, side), n);
		say(c, picks, n);
		team_screen_close(c->screen);
		return c->out[c->sent++];
	}
	/* (teamslow=N: side 0's partner's hands off his screen for its first N frames, a slow partner) */
	bool idle = gone || (!side && dev.team_slow > 0 && c->frames++ < (unsigned)dev.team_slow);
	if (team_screen_frame(c->screen, idle ? 0 : screen_keys(frame), picks, &n)) {
		++c->oks;
		say(c, picks, n);
		return c->out[c->sent++];
	}
	return 0;
}

void team_client_draw(Arena *a) {
	unsigned s = seat(), side = arena_team_view_side(s);
	const Client *c = &clients[side];
	/* (a partner's seat: his own screen while his turn is on; the scene shows his side's GBA otherwise, arena_show) */
	const uint32_t *v = s >= 2 && c->turn && c->screen ? team_screen_video(c->screen) : arena_video(a);
	if (v) arena_view_core(v);
	arena_team_view(arena_core(a, side), s);
}

void team_client_report(void) {
	arena_team_report();
	for (unsigned s = 0; s < SIDES; ++s) {
		const Client *c = &clients[s];
		if (!c->screen) continue;
		printf("arena2v2: side %u's partner's client: BN6's Custom screen on a plain copy of the pair opened %u times (a copy %.2f ms, "
			"the worst %.2f), OK pressed %u times, %u times his side's closed first, %u times sent at the wait's limit; a frame of "
			"it %.2f ms (the worst %.2f)\n", s, c->opened, team_screen_copy_ms(c->screen, false), team_screen_copy_ms(c->screen, true),
			c->oks, c->closed_first, c->at_limit, team_screen_ms(c->screen, false), team_screen_ms(c->screen, true));
	}
}
