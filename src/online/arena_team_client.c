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
	uint16_t out[TEAM_CUSTOM_PICKS + 2];  /* his Cross, Program Advance and pick messages to go out, one a frame */
	uint16_t deck[BN6_FOLDER_ENTRIES];    /* his deck as his turn opened (his screen dealt from it) */
	unsigned dealt;
	unsigned queued, sent;
	unsigned opened, oks, closed_first, at_limit, crossed, pas;
	unsigned frames;                  /* his turn's frames so far (teamslow's) */
	unsigned t;                       /* ... every one (teamcross's script) */
} Client;

/* teamcross=N's Cross on his screen's CROSSSELECT, its row of his Crosses (his alone: N's) */
#define CROSS_UP    40    /* the turn's frame he takes the cursor up to the emblem, */
#define CROSS_OPEN  70    /* ... A opens CROSSSELECT, */
#define CROSS_TAKE  170   /* ... A takes his Cross, */
#define CROSS_CHIPS 210   /* ... then his chips as every turn */

static Client clients[SIDES];
static unsigned my_seat;   /* a 2v2 over the network: my seat (team_client_seat), 0 none */

/* the seat shown: mine in a 2v2 over the network, else teamseat's (0 the first GBA's player, as without it, to 3) */
static unsigned seat(void) { return my_seat ? my_seat : dev.team_seat > 0 && dev.team_seat < 4 ? (unsigned)dev.team_seat : 0; }

static unsigned u16_at(const uint8_t *bytes, unsigned at) { return bytes[at] | (unsigned)bytes[at + 1] << 8; }

void team_client_compile(unsigned side, const OnlineLoadout *l, bool again) {
	static bool tried[SIDES], made[SIDES];
	static uint8_t stats[SIDES][BN6_BATTLE_NAVI_SIZE];
	side &= 1u;
	if (again) tried[side] = false;
	if (!tried[side] && l) {
		tried[side] = true;
		size_t n;
		uint8_t *state = arena_carry_state(&n);
		char why[128] = "";
		made[side] = state && team_navicust_compile(state, n, l, stats[side], why, sizeof why);
		free(state);
		const uint8_t *s = stats[side];
		if (made[side])
			printf("arena2v2: side %u's partner's NaviCust compiled by BN6 (%08X) on his own core: HP %u (base %u), Custom %u, buster "
				"%u %u %u, SuperArmor %u, FloatShoe %u, AirShoe %u, UnderShirt %u\n", side, BN6_NAVICUST_RELOAD,
				u16_at(s, BN6_NS_MAX_HP), u16_at(s, BN6_NS_BASE_HP), s[BN6_NS_CUSTOM],
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
	if (all && dev.team_cust) team_client_compile(1, all, false);
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
		team_client_compile(0, &own, false);
		printf("arena2v2: side 0's partner carries the run's folder %d, his lead folder %d\n", k + 1, first + 1);
		break;
	}
	if (seat()) arena_show(a, (int)arena_team_view_side(seat()));
}

void team_client_free(void) {
	for (unsigned s = 0; s < SIDES; ++s) team_screen_free(clients[s].screen);
	memset(clients, 0, sizeof clients);
	my_seat = 0;
}

/* side's partner's own Custom screen on a plain copy of the pair, his Crosses on it: his own MegaMan's (`mine`, over the
 * network), and teamcross=N's Cross N and teambeast's Beast Out (a stood-in client's alone where none is given) */
static TeamScreen *new_screen(Arena *a, unsigned side, const OnlineLoadout *mine) {
	TeamScreen *t = team_screen_new(R.data, ROM_SIZE, arena_pair_of(a), side);
	int n = dev.team_cross < 0 ? -dev.team_cross : dev.team_cross;
	unsigned crosses = (n >= 1 && n <= 5 ? 1u << (n - 1) : 0u) | (dev.team_beast ? TEAM_SCREEN_BEAST : 0u);
	if (mine) crosses |= (mine->crosses & 0x1Fu) | (mine->beast ? TEAM_SCREEN_BEAST : 0u);
	if (t) team_screen_crosses(t, (uint8_t)crosses);
	return t;
}

bool team_client_seat(Arena *a, unsigned seat_of, const OnlineLoadout *mine) {
	team_client_free();
	my_seat = seat_of;
	if (my_seat < 2) return true;
	unsigned side = my_seat & 1u;
	arena_show(a, (int)side);
	/* (his screen made now, the pair at its first Custom screen as the way in ends: its cost outside the battle) */
	Client *c = &clients[side];
	c->screen = new_screen(a, side, mine);
	printf("arena: our seat %u, side %u's partner: his own Custom screen %s\n", my_seat, side, c->screen ? "made" : "not made");
	return c->screen != NULL;
}

/* his keys on his screen, by the frame's number alone: the cursor a chip
 * right and A every 20 frames (BN6 takes the chips that fit, and at last OK) */
static uint16_t screen_keys(uint32_t frame) {
	unsigned t = frame % 20;
	return t < 2 ? KEY_A : t >= 10 && t < 12 ? KEY_RIGHT : 0;
}

/* teambeast: at his first turn the cursor right to OK, down to the emblem under it, A (Beast Out), up and OK */
static uint16_t beast_keys(const Client *c) {
	static const struct { uint16_t t, key; } steps[] = {
		{ 40, KEY_RIGHT }, { 52, KEY_RIGHT }, { 64, KEY_RIGHT }, { 76, KEY_RIGHT }, { 88, KEY_RIGHT }, { 100, KEY_DOWN },
		{ 112, KEY_A }, { 240, KEY_UP }, { 260, KEY_A },
	};
	for (size_t i = 0; i < sizeof steps / sizeof *steps; ++i)
		if (c->t >= steps[i].t && c->t < steps[i].t + 3u) return steps[i].key;
	return 0;
}

/* teamcross: at his first turn the cursor up to the emblem, CROSSSELECT opened and his one Cross taken, then his chips */
static uint16_t cross_keys(const Client *c, uint32_t frame) {
	if (dev.team_beast && (c->opened == 1 || dev.team_beast > 1)) return beast_keys(c);
	if (dev.team_cross == 0 || c->opened != 1 || c->t >= CROSS_CHIPS) return screen_keys(frame);
	unsigned t = c->t;
	return t >= CROSS_UP && t < CROSS_UP + 3 ? KEY_UP : (t >= CROSS_OPEN && t < CROSS_OPEN + 3) || (t >= CROSS_TAKE && t < CROSS_TAKE + 3)
		? KEY_A : 0;
}

/* a Program Advance his OK made: the picks it took (from at, took of them) found as the hand BN6 built has them, each place
 * and count tried through the pair's own build; false where none gives BN6's hand */
static bool pa_found(const Client *c, const uint8_t *picks, unsigned n, uint16_t pa, const uint16_t *built, unsigned *at,
	unsigned *took) {
	uint16_t entries[TEAM_CUSTOM_PICKS];
	for (unsigned i = 0; i < n && i < TEAM_CUSTOM_PICKS; ++i) entries[i] = picks[i] < c->dealt ? c->deck[picks[i]] : 0xFFFFu;
	for (*took = 3; *took <= 4; ++*took)
		for (*at = 0; *at + *took <= n; ++*at) {
			TeamHand h;
			unsigned k = team_hand_build(entries, n, *at + 1, *took, pa, &h), same = 1;
			for (unsigned i = 0; i < BN6_HAND_MAX && same; ++i) same = (i < k ? h.id[i] : 0xFFFFu) == built[i] || i > k;
			if (same) return true;
		}
	return false;
}

/* his Cross (where his CROSSSELECT committed one), a Program Advance (where his OK made one) and his picks into his stream,
 * one a frame */
static void say(Client *c, const uint8_t *picks, unsigned n) {
	c->queued = c->sent = 0;
	uint8_t form = team_screen_cross(c->screen);
	if (form) {
		c->out[c->queued++] = TEAM_CROSS(form);
		++c->crossed;
	}
	uint16_t built[BN6_HAND_MAX], pa = team_screen_pa(c->screen, built);
	unsigned at, took;
	if (pa && pa_found(c, picks, n, pa, built, &at, &took)) {
		c->out[c->queued++] = TEAM_PA(at, took, pa);
		++c->pas;
		printf("arena2v2: a partner's Program Advance %03X from his picks %u to %u, as BN6's OK built it on his screen\n", pa, at,
			at + took - 1);
	} else if (pa) printf("arena2v2: a partner's Program Advance %03X not found among his picks: his picks cross alone\n", pa);
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

/* player's (2 or 3) stream for `frame`: on his Custom screen while his
 * turn is on his `own` keys (a player's), else the stood-in client's
 * script; his `battle` keys outside it; or a pick message */
static uint16_t stream(Arena *a, unsigned player, uint32_t frame, bool own, uint16_t screen, uint16_t battle) {
	unsigned side = (player - 2) & 1u;
	Client *c = &clients[side];
	struct mCore *gba = arena_core(a, side);
	/* (a partner deleted has no turn) */
	bool custom = gba && gba->rawRead8(gba, BN6_BATTLE_PHASE, -1) == BN6_PHASE_CUSTOM && standing(gba, side);
	/* (his screen made once, from the pair as it is at a Custom screen) */
	if (custom && !c->screen) c->screen = new_screen(a, side, NULL);
	if (!custom || !c->screen) {
		closed_first(c);
		c->turn = false;
	} else if (!c->turn) {
		/* (once the pair has dealt his deck: its first frame of the battle) */
		unsigned n = c->dealt = arena_team_custom_deck(gba, side, c->deck);
		c->turn = n && team_screen_open(c->screen, arena_pair_of(a), c->deck, n);
		c->queued = c->sent = 0;
		c->frames = c->t = 0;
		c->opened += c->turn;
	}
	if (c->sent < c->queued) return c->out[c->sent++];
	if (!c->turn) return battle & ARENA_KEYS_MASK;
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
	uint16_t keys = idle ? 0 : own ? screen : cross_keys(c, frame);
	++c->t;
	if (team_screen_frame(c->screen, keys, picks, &n)) {
		++c->oks;
		say(c, picks, n);
		return c->out[c->sent++];
	}
	return 0;
}

uint16_t team_client_keys(Arena *a, unsigned player, uint32_t frame) {
	return stream(a, player, frame, false, 0, arena_team_script(player, frame));
}

uint16_t team_client_stream(void *context, Arena *a, uint32_t frame, uint16_t keys) {
	(void)context;
	if (my_seat < 2) return keys & ARENA_KEYS_MASK;
	/* (--dev arenaboth: a partner who plays himself, as the stood-in ones) */
	if (dev.arena_both) return team_client_keys(a, my_seat, frame);
	return stream(a, my_seat, frame, true, keys, keys);
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
			"it %.2f ms (the worst %.2f); his Cross sent %u times, a Program Advance %u\n", s, c->opened, team_screen_copy_ms(c->screen, false), team_screen_copy_ms(c->screen, true),
			c->oks, c->closed_first, c->at_limit, team_screen_ms(c->screen, false), team_screen_ms(c->screen, true), c->crossed, c->pas);
	}
}
