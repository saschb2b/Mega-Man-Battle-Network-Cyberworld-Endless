/* Hostile server records, delayed movement and persistent local moderation. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "netpass.h"
#include "netpass_text.h"
#include "crew_pull.h"
#include "dive_seats.h"
#include "dive_wire.h"
#include "online_motion.h"
#include "online_people.h"
#include "online_transport.h"
#include "online_week.h"
#include "online_wire.h"
#include "square_looks_pick.h"
#include "pet_carry.h"
#include "platform.h"
#include "rom.h"
#include "square_life_words.h"
#include "square_plan.h"
#include "square_week_words.h"
#include "square_words.h"
#include "trade_list.h"
#include "trade_screen_text.h"
#include "trade_words.h"

char g_data_dir[512];
/* (the extra ROMs read: none until the NetPass's test puts one in, rom.c's) */
XRom XR[XROM_COUNT];
int xroms_most = XROMS_MOST;
static unsigned persists;
void platform_persist(void) { ++persists; }
/* (the week's post, online_week.c: what would go to the server) */
static uint8_t sent[64];
static size_t sent_size;
bool online_transport_send(const uint8_t *data, size_t size) {
	sent_size = size < sizeof sent ? size : sizeof sent;
	memcpy(sent, data, sent_size);
	return true;
}

static void welcome(OnlineSnapshot *out) {
	uint8_t bytes[ONLINE_WELCOME_BYTES] = { ONLINE_WELCOME };
	online_put32(bytes + 1, 120); online_put32(bytes + 5, 1);
	online_put16(bytes + 9, 20); bytes[11] = 8;
	online_put16(bytes + 12, 1); bytes[14] = 99; online_put16(bytes + 30, 8);
	/* (protocol v6: the week's net, its seed and number) */
	online_put32(bytes + 32, 0xF521FE36u); online_put16(bytes + 36, 2963);
	assert(online_wire_message(bytes, sizeof bytes, 100, out));
	assert(out->connected && out->channel == 1 && out->channels == 8 && out->identity[0] == 99);
	assert(out->week_seed == 0xF521FE36u && out->week == 2963);
	for (size_t size = 0; size < sizeof bytes; ++size) assert(!online_wire_message(bytes, size, 100, out));
	bytes[11] = 9; assert(!online_wire_message(bytes, sizeof bytes, 100, out));
}

static void positions(void) {
	OnlineSnapshot out = { 0 };
	welcome(&out);
	enum { ONE = 2 + ONLINE_PEER_BYTES };
	uint8_t bytes[2 + ONLINE_PEER_BYTES * 2] = { ONLINE_SNAPSHOT, 1 };
	online_put32(bytes + 2, 2); bytes[6] = 42;
	online_put32(bytes + 22, (uint32_t)-500); online_put32(bytes + 26, 400);
	bytes[34] = 7; memcpy(bytes + 35, "Player-2", 8);
	assert(online_wire_message(bytes, ONE, 100, &out));
	assert(out.peer_count == 1 && out.peers[0].x == -500 && out.peers[0].y == 400 && out.peers[0].identity[0] == 42);
	assert(out.peers[0].roms == 0);
	/* (his ROMs besides BN6's: BN5's bit, and no other) */
	bytes[51] = ONLINE_ROMS_BN5; assert(online_wire_message(bytes, ONE, 100, &out) && out.peers[0].roms == ONLINE_ROMS_BN5);
	/* (v13: beside them what he is busy with and his link's bars; no bit past those) */
	bytes[51] = (uint8_t)(ONLINE_ROMS_BN5 | ONLINE_BUSY_WATCH << ONLINE_BUSY_SHIFT | 2 << ONLINE_LINK_SHIFT);
	assert(online_wire_message(bytes, ONE, 100, &out) && out.peers[0].roms == ONLINE_ROMS_BN5 && out.peers[0].busy == ONLINE_BUSY_WATCH &&
		out.peers[0].link == 2);
	/* (v17: BN4's bit, 0x20, beside BN5's; no bit past it) */
	bytes[51] = (uint8_t)(ONLINE_ROMS_BN4 | ONLINE_ROMS_BN5 | 1 << ONLINE_LINK_SHIFT);
	assert(online_wire_message(bytes, ONE, 100, &out) && out.peers[0].roms == ONLINE_ROMS_FULL && out.peers[0].link == 1);
	/* (v18: bit 6, his system reads one extra ROM at most, beside the game it has; no bit past it) */
	bytes[51] = (uint8_t)(ONLINE_ROMS_ONE_EXTRA | ONLINE_ROMS_BN5 | ONLINE_BUSY_TRADE << ONLINE_BUSY_SHIFT);
	assert(online_wire_message(bytes, ONE, 100, &out) && out.peers[0].roms == (ONLINE_ROMS_ONE_EXTRA | ONLINE_ROMS_BN5) && out.peers[0].busy == ONLINE_BUSY_TRADE);
	/* (v19: bit 7, he dives, the last bit free; a Square's peers never have it, a dive room's all do) */
	bytes[51] = ONLINE_PEER_DIVING; assert(online_wire_message(bytes, ONE, 100, &out) && out.peers[0].diving && !out.peers[0].roms);
	bytes[51] = 0; assert(online_wire_message(bytes, ONE, 100, &out) && !out.peers[0].diving);
	/* (v30: his look's six bytes after them, three palettes then three colours: a channel BN6's own, or in range) */
	static const uint8_t look[6] = { 3, ONLINE_LOOK_NONE, 11, 15, ONLINE_LOOK_NONE, 0 };
	memcpy(bytes + 52, look, sizeof look);
	assert(online_wire_message(bytes, ONE, 100, &out) && !memcmp(out.peers[0].look, look, sizeof look));
	bytes[52] = ONLINE_LOOK_PALETTES; assert(!online_wire_message(bytes, ONE, 100, &out)); bytes[52] = 3;
	bytes[55] = ONLINE_LOOK_COLOURS; assert(!online_wire_message(bytes, ONE, 100, &out)); bytes[55] = 15;
	bytes[53] = 0; assert(!online_wire_message(bytes, ONE, 100, &out)); bytes[53] = ONLINE_LOOK_NONE;
	for (size_t size = 0; size < ONE; ++size) assert(!online_wire_message(bytes, size, 100, &out));
	bytes[34] = 8; assert(!online_wire_message(bytes, ONE, 100, &out)); bytes[34] = 7;
	online_put32(bytes + 22, 8193); assert(!online_wire_message(bytes, ONE, 100, &out));
	online_put32(bytes + 22, 1);
	bytes[1] = 2; memcpy(bytes + ONE, bytes + 2, ONLINE_PEER_BYTES);
	assert(!online_wire_message(bytes, sizeof bytes, 100, &out));
	online_put32(bytes + ONE, 3); assert(!online_wire_message(bytes, sizeof bytes, 100, &out));
	bytes[ONE + 4] = 43; assert(online_wire_message(bytes, sizeof bytes, 100, &out));
	bytes[1] = 8; assert(!online_wire_message(bytes, sizeof bytes, 100, &out));
}

static void cards_and_events(void) {
	OnlineSnapshot out = { 0 };
	welcome(&out);
	OnlineProfile profile = { .hp = 75, .max_hp = 300, .attack = 4, .speed = 3, .charge = 2,
		.souls = 63, .crosses = 31, .depth = 65535, .act = 255, .best_depth = 65535, .marks = 65535 };
	profile.board[48] = 6;
	uint8_t encoded[70], card[90] = { ONLINE_CARD };
	assert(online_wire_profile(encoded, &profile));
	online_put32(card + 1, 2); card[5] = 42;
	memcpy(card + 21, encoded + 1, ONLINE_PROFILE_BYTES);
	assert(online_wire_message(card, sizeof card, 100, &out));
	assert(out.card.hp == 75 && out.card.max_hp == 300 && out.card.depth == 65535 && out.card.board[48] == 6);
	for (size_t size = 1; size < sizeof card; ++size) assert(!online_wire_message(card, size, 100, &out));
	card[89] = 7; assert(!online_wire_message(card, sizeof card, 100, &out));
	profile.hp = 301; assert(!online_wire_profile(encoded, &profile));
	/* (v14: aimed at player 9, answered, its beat 250 ms on; half the round trip of 40 ms before it came) */
	uint8_t event[ONLINE_EVENT_BYTES] = { ONLINE_EVENT, 0, 0, 0, 2, 1, 5, 0, 0, 0, 9, ONLINE_EVENT_ANSWERED, 0, 250 };
	out.latency_ms = 40;
	assert(online_wire_message(event, sizeof event, 100, &out));
	assert(out.events[1].player_id == 2 && out.events[1].value == 5);
	assert(out.events[1].target == 9 && out.events[1].flags == ONLINE_EVENT_ANSWERED && out.events[1].start == 100 + 250 - 20);
	for (size_t size = 1; size < sizeof event; ++size) assert(!online_wire_message(event, size, 100, &out));
	event[6] = ONLINE_EMOTE_COUNT; assert(!online_wire_message(event, sizeof event, 100, &out));
	uint8_t notice[] = { ONLINE_NOTICE, 0, 12, 'W', 'a', 'i', 't' };
	assert(online_wire_message(notice, sizeof notice, 100, &out) && out.connected && !strcmp(out.notice, "Wait"));
	notice[3] = '\n'; assert(!online_wire_message(notice, sizeof notice, 100, &out));
}

static void motion(void) {
	OnlineSnapshot a = { .connected = true, .channel = 1, .position_serial = 1, .position_ticks = 1000, .peer_count = 1 }, out;
	a.peers[0] = (OnlinePeer){ .id = 2, .identity = { 42 } };
	online_motion_reset(); online_motion_update(&a, 1000, &out);
	a.position_serial = 2; a.position_ticks = 1050; a.peers[0].x = 100;
	online_motion_update(&a, 1050, &out);
	a.position_serial = 3; a.position_ticks = 1100; a.peers[0].x = 200;
	online_motion_update(&a, 1125, &out);
	assert(out.peers[0].x == 50); /* halfway between the two older samples */
	assert(online_people_toggle(a.peers[0].identity, ONLINE_BLOCKED));
	assert(persists == 1);
	online_motion_update(&a, 1150, &out); assert(!out.peer_count);
	online_people_load(); assert(online_people_flags(a.peers[0].identity) == ONLINE_BLOCKED);
	assert(online_people_toggle(a.peers[0].identity, ONLINE_BLOCKED));
	assert(persists == 2);
	online_motion_update(&a, 1150, &out); assert(out.peers[0].x == 100);
	a.peer_count = 0; a.position_serial = 4; a.position_ticks = 1150;
	online_motion_update(&a, 1150, &out); assert(!out.peer_count); /* departure is immediate */
}

static void fuzz(void) {
	uint32_t random = 0x91827364;
	uint8_t bytes[ONLINE_MAX_MESSAGE + 1];
	for (unsigned test = 0; test < 12000; ++test) {
		size_t size = test % sizeof bytes;
		for (size_t i = 0; i < size; ++i) { random = random * 1664525u + 1013904223u; bytes[i] = (uint8_t)(random >> 24); }
		OnlineSnapshot out = { .connected = true, .player_id = 1 };
		(void)online_wire_message(bytes, size, 1000, &out);
	}
}

static bool on_stair(const SquarePlan *p, int x, int y) {
	for (int i = 0; i < p->nstairs; ++i)
		if (stair_covers(&p->stairs[i], x, y)) return true;
	return false;
}

/* each stair climbs from ground floor to raised floor, and each raised
 * room is drawn where no other floor is (a panel up the screen) */
static void square_heights(const SquarePlan *p) {
	for (int i = 0; i < p->nstairs; ++i) {
		const Stair *st = &p->stairs[i];
		assert(st->dir == STAIR_UP_NY);
		for (int dx = 0; dx < 2; ++dx) {
			assert(p->cell[st->y - 1][st->x + dx] == C_PATH && p->level[st->y - 1][st->x + dx]);
			assert(p->cell[st->y + 2][st->x + dx] == C_PATH && !p->level[st->y + 2][st->x + dx]);
		}
	}
	for (int y = 2; y < SQUARE_H; ++y)
		for (int x = 2; x < SQUARE_W; ++x) {
			if (p->cell[y][x] != C_PATH || !p->level[y][x]) continue;
			for (int dy = -2; dy <= 0; ++dy)
				for (int dx = -2; dx <= 0; ++dx) {
					int ox = x + dx, oy = y + dy;
					assert(!p->cell[oy][ox] || p->level[oy][ox] || on_stair(p, ox, oy));
				}
		}
}

/* the warp gem at panel (x, y) leads to (*tx, *ty), its pair */
static bool square_gem(const SquarePlan *p, int x, int y, int *tx, int *ty) {
	for (int k = 0; k < SQUARE_WARPS; ++k)
		for (int e = 0; e < 2; ++e)
			if (p->gem[k][e][0] == x && p->gem[k][e][1] == y) { *tx = p->gem[k][!e][0]; *ty = p->gem[k][!e][1]; return true; }
	return false;
}

/* every floor panel of plan `p` reached from the arrival, a step between
 * heights only on a stair and from part to part only by a warp (the
 * steps walked from the arrival to each gem in `steps`), and every place
 * on the floor */
static void square_reach(const SquarePlan *p) {
	static uint8_t seen[SQUARE_H][SQUARE_W];
	static int dist[SQUARE_H][SQUARE_W];
	memset(seen, 0, sizeof seen);
	static int queue[SQUARE_W * SQUARE_H][2];
	int head = 0, n = 0, reached = 0, floor = 0;
	queue[n][0] = p->at[SQ_ARRIVE][0]; queue[n++][1] = p->at[SQ_ARRIVE][1];
	seen[p->at[SQ_ARRIVE][1]][p->at[SQ_ARRIVE][0]] = 1;
	dist[p->at[SQ_ARRIVE][1]][p->at[SQ_ARRIVE][0]] = 0;
	while (head < n) {
		int x = queue[head][0], y = queue[head++][1];
		++reached;
		static const int d[5][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 0, 0 } };
		for (int k = 0; k < 5; ++k) {
			int nx = x + d[k][0], ny = y + d[k][1];
			/* (a gem beams to its pair) */
			if (k == 4 && !square_gem(p, x, y, &nx, &ny)) continue;
			if (nx < 0 || ny < 0 || nx >= SQUARE_W || ny >= SQUARE_H || seen[ny][nx] || p->cell[ny][nx] != C_PATH) continue;
			if (k < 4 && p->level[ny][nx] != p->level[y][x] && !on_stair(p, x, y) && !on_stair(p, nx, ny)) continue;
			if (k < 4) assert(square_plan_part(p, nx, ny) == square_plan_part(p, x, y));
			seen[ny][nx] = 1;
			dist[ny][nx] = dist[y][x] + 1;
			queue[n][0] = nx; queue[n++][1] = ny;
		}
	}
	/* (every part's floor in its bounds, two void panels or more from
	 * another's; each warp's gem on ground floor with its pad's all round
	 * it, the plaza's a short walk from the arrival, no place nor
	 * walker's waypoint on a pad) */
	for (int y = 0; y < SQUARE_H; ++y)
		for (int x = 0; x < SQUARE_W; ++x) {
			if (p->cell[y][x] == C_VOID) continue;
			int k = square_plan_part(p, x, y);
			assert(k >= 0 || p->cell[y][x] == C_SOLID);
			for (int j = y - 2; j <= y + 2 && k >= 0; ++j)
				for (int i = x - 2; i <= x + 2; ++i)
					if (i >= 0 && j >= 0 && i < SQUARE_W && j < SQUARE_H && p->cell[j][i] == C_PATH) assert(square_plan_part(p, i, j) == k);
		}
	for (int k = 0; k < SQUARE_WARPS; ++k)
		for (int e = 0; e < 2; ++e) {
			int gx = p->gem[k][e][0], gy = p->gem[k][e][1];
			assert(square_plan_part(p, gx, gy) == (e ? k + 1 : SQ_PART_PLAZA));
			for (int j = gy - 1; j <= gy + 1; ++j)
				for (int i = gx - 1; i <= gx + 1; ++i) assert(p->cell[j][i] == C_PATH && !p->level[j][i] && p->pad[j][i] && !on_stair(p, i, j));
			if (!e) assert(dist[gy][gx] <= 16);
			/* (landing, BN6 steps MegaMan off the gem along a screen diagonal, onto its pad) */
			assert(p->gem_face[k][e] % 2 == 0);   /* (steps along the grid; MegaMan walks the diagonals too) */
			for (int q = 0; q < SQ_PLACES; ++q) assert(abs(p->at[q][0] - gx) > 1 || abs(p->at[q][1] - gy) > 1);
			for (int w = 0; w < SQUARE_WALKERS; ++w)
				for (int q = 0; q < p->route_len[w]; ++q) assert(abs(p->route[w][q][0] - gx) > 1 || abs(p->route[w][q][1] - gy) > 1);
		}
	for (int y = 0; y < SQUARE_H; ++y) for (int x = 0; x < SQUARE_W; ++x) floor += p->cell[y][x] == C_PATH;
	assert(reached == floor);
	for (int k = 0; k < SQ_PLACES; ++k) assert(p->cell[p->at[k][1]][p->at[k][0]] != C_VOID);
	/* each walker's legs, and the parade's, along a grid axis over
	 * ground floor, none through a hole */
	for (int w = 0; w <= SQUARE_WALKERS; ++w) {
		int len = w < SQUARE_WALKERS ? p->route_len[w] : 4;
		assert(len >= 2 && len <= SQUARE_ROUTE);
		for (int k = 0; k < len; ++k) {
			const int *a = w < SQUARE_WALKERS ? p->route[w][k] : p->parade[k];
			const int *b = w < SQUARE_WALKERS ? p->route[w][(k + 1) % len] : p->parade[(k + 1) % len];
			assert(a[0] == b[0] || a[1] == b[1]);
			int steps = abs(b[0] - a[0]) + abs(b[1] - a[1]);
			for (int i = 0; i <= steps; ++i) {
				int x = a[0] + (b[0] - a[0]) * i / (steps ? steps : 1), y = a[1] + (b[1] - a[1]) * i / (steps ? steps : 1);
				assert(p->cell[y][x] == C_PATH && !p->level[y][x] && !p->pad[y][x]);
			}
		}
	}
}

/* The Square's plan: the same for a seed on every client (its hash the
 * one protocol.json and the server hold), every floor panel reached from
 * the arrival, its stage and ring up their stairs, over many seeds. */
static void square(void) {
	SquarePlan p, q;
	square_plan(120, &p);
	square_plan(120, &q);
	assert(!memcmp(&p, &q, sizeof p));
	if (square_plan_hash(&p) != SQUARE_HASH) {
		fprintf(stderr, "the Square's plan changed: its hash is 0x%08Xu; set SQUARE_HASH (and protocol.json's square_hash) and bump SQUARE_MAKE\n",
			square_plan_hash(&p));
		assert(0);
	}
	for (uint32_t seed = 0; seed < 64; ++seed) {
		square_plan(seed, &q);
		assert(q.area == 0 || q.area == 2 || q.area == 3);
		assert(q.nstairs == SQUARE_STAIRS && q.nprops <= SQUARE_PROPS);
		square_reach(&q);
		square_heights(&q);
	}
}

/* the referee by the ring (issue #128): the ring's tip until a bout, then
 * who beat whom, and the count from a rematch on */
static void referee(void) {
	SquareFacts f;
	memset(&f, 0, sizeof f);
	f.arena = true;
	f.navis = 1;
	/* (no Arena stamp on this game's NetPass, issue #131: the stamp named, never a ROM, no tip) */
	const char *closed = square_referee_words(&f);
	assert(strstr(closed, "NO ARENA STAMP ON\nYOUR NETPASS!") && !strstr(closed, "PICK") && !strstr(closed, "BN5"));
	f.stamp = true;
	assert(strstr(square_referee_words(&f), "WHO'S FIRST?") && strstr(square_referee_words(&f), "PICK"));
	/* (another pair's NetBattle on in the channel: no "none yet", and no last bout of this game's) */
	f.battle_on = true;
	const char *on = square_referee_words(&f);
	assert(strstr(on, "PICK") && !strstr(on, "WHO'S FIRST?") && !strstr(on, "LAST BOUT"));
	f.battle_on = false;
	f.fought = "Alpha";
	f.result = 1; f.won = 1;
	const char *s = square_referee_words(&f);
	assert(strstr(s, "LAST BOUT: YOU\nBEAT Alpha!") && !strstr(s, "PICK") && !strstr(s, " TO "));
	f.won = 2; f.lost = 1;
	assert(strstr(square_referee_words(&f), "THAT'S 2 TO 1.\nYOU LEAD!"));
	f.result = 2; f.won = 1; f.lost = 2;
	s = square_referee_words(&f);
	assert(strstr(s, "LAST BOUT:\nAlpha BEAT\nYOU!") && strstr(s, "THAT'S 2 TO 1.\nAlpha\nLEADS!"));
	f.won = 2; f.lost = 2;
	assert(strstr(square_referee_words(&f), "2 TO 2! ALL EVEN"));
	assert(!strcmp(square_referee_bark(1, "Alpha"), "YOU WIN!"));
	assert(!strcmp(square_referee_bark(2, "Alpha"), "ALPHA WINS!"));
	assert(!strcmp(square_referee_bark(3, "Alpha"), "WHAT A BOUT!"));
	/* (a bout broken off: neither won nor lost, the count as it was) */
	assert(!strcmp(square_referee_bark(4, "Alpha"), "BROKE OFF! NO CALL!"));
	f.result = 4;
	s = square_referee_words(&f);
	assert(strstr(s, "Alpha BROKE OFF!") && strstr(s, "2 TO 2! ALL EVEN"));
	/* (the Arena not built here, the stamp held: the PET's word) */
	f.arena = false;
	assert(strstr(square_referee_words(&f), "NO NETBATTLES\nFROM THIS PET!"));
}

/* the NetPass (issue #131): the set, the stamps by the ROMs read, a peer's
 * by its SNAPSHOT bits; the words that name a game are the pass's alone,
 * the characters' name stamps */
static void netpass(void) {
	assert(netpass_games() == 3 && !strcmp(netpass_game(0)->label, "BN6") && !strcmp(netpass_game(1)->label, "BN5") &&
		!strcmp(netpass_game(2)->label, "BN4"));
	assert(netpass_game(1)->bit == ONLINE_ROMS_BN5 && netpass_game(2)->bit == ONLINE_ROMS_BN4 && !netpass_game(3));
	assert((ONLINE_ROMS_BN5 | ONLINE_ROMS_BN4) == ONLINE_ROMS_FULL);
	/* (BN6's ROM alone: the Square's stamp) */
	assert(netpass_game_in(0) && !netpass_game_in(1) && !netpass_full());
	assert(netpass_has(STAMP_SQUARE) && !netpass_has(STAMP_ARENA) && !netpass_has(STAMP_TRADE) && !netpass_has(STAMP_DEEPNET));
	/* (BN5's beside it, v16's full set: the Square's stamp alone now, BN4's slot open; then BN4's too: every stamp) */
	static uint8_t bn5, bn4;
	XR[XROM_BN5_COLONEL_US].data = &bn5;
	assert(!netpass_full() && !netpass_has(STAMP_ARENA) && netpass_game_in(1) && !netpass_game_in(2));
	XR[XROM_BN4_RED_SUN_US].data = &bn4;
	assert(netpass_full() && netpass_has(STAMP_ARENA) && netpass_has(STAMP_TRADE) && netpass_has(STAMP_DEEPNET));
	XR[XROM_BN5_COLONEL_US].data = NULL;
	assert(!netpass_full() && !netpass_has(STAMP_TRADE));
	XR[XROM_BN4_RED_SUN_US].data = NULL;
	assert(!netpass_one_extra() && netpass_stamps() == 1 && netpass_game_fits(1) && netpass_game_fits(2));
	/* (a system that reads one extra ROM at most, the New 3DS, protocol v18: no game beside BN6's, the Square's stamp,
	 * either game's slot open; one of them, BN5's or BN4's, a full pass of the Square's and Trade's stamps, no room for
	 * the other; the Arena and the DeepNet take every game, which it never holds) */
	xroms_most = 1;
	assert(netpass_one_extra() && !netpass_full() && netpass_stamps() == 1 && netpass_game_fits(1) && netpass_game_fits(2));
	for (int x = 0; x < XROM_COUNT; ++x) {
		XR[x].data = &bn5;
		assert(netpass_full() && netpass_has(STAMP_SQUARE) && netpass_has(STAMP_TRADE) && !netpass_has(STAMP_ARENA) && !netpass_has(STAMP_DEEPNET));
		assert(netpass_stamps() == 2 && netpass_game_fits(0) && netpass_game_fits(1 + x) && !netpass_game_fits(2 - x));
		XR[x].data = NULL;
	}
	/* (... and were it to read both after all, every stamp) */
	XR[XROM_BN5_COLONEL_US].data = &bn5; XR[XROM_BN4_RED_SUN_US].data = &bn4;
	assert(netpass_full() && netpass_stamps() == STAMP_COUNT);
	XR[XROM_BN5_COLONEL_US].data = XR[XROM_BN4_RED_SUN_US].data = NULL;
	xroms_most = XROMS_MOST;
	/* (another game's pass, by the bits every full set needs: protocol.json's roms_full) */
	assert(!netpass_peer_full(0) && !netpass_peer_full(ONLINE_ROMS_BN5) && !netpass_peer_full(ONLINE_ROMS_BN4) && netpass_peer_full(ONLINE_ROMS_FULL));
	/* (... or, his system reading one extra ROM at most, one game beside BN6's: he trades, and neither battles nor is watched with) */
	const uint8_t one5 = ONLINE_ROMS_ONE_EXTRA | ONLINE_ROMS_BN5, one4 = ONLINE_ROMS_ONE_EXTRA | ONLINE_ROMS_BN4;
	assert(!netpass_peer_full(ONLINE_ROMS_ONE_EXTRA) && netpass_peer_full(one5) && netpass_peer_full(one4));
	assert(netpass_peer_has(one5, STAMP_TRADE) && netpass_peer_has(one4, STAMP_TRADE) && !netpass_peer_has(one5, STAMP_ARENA) && !netpass_peer_has(one4, STAMP_DEEPNET));
	assert(!netpass_peer_has(ONLINE_ROMS_BN5, STAMP_TRADE) && !netpass_peer_has(ONLINE_ROMS_ONE_EXTRA, STAMP_TRADE) && netpass_peer_has(0, STAMP_SQUARE));
	assert(netpass_peer_has(ONLINE_ROMS_FULL, STAMP_ARENA) && netpass_peer_has(ONLINE_ROMS_FULL, STAMP_TRADE) && netpass_peer_has(ONLINE_ROMS_FULL, STAMP_DEEPNET));
	/* (whose game a chip is by the wire's numbers, as the server counts it, and a trade's common ground: BN6's own
	 * every game holds, BN5's a game with BN5's ROM, BN4's own, from 527, one with BN4's) */
	assert(netpass_peer_holds(0, ONLINE_BN6_CHIP_LAST) && !netpass_peer_holds(0, 210) && netpass_peer_holds(ONLINE_ROMS_BN5, 210));
	assert(netpass_peer_holds(ONLINE_ROMS_BN5, ONLINE_BN4_CHIP_FIRST - 1) && !netpass_peer_holds(ONLINE_ROMS_BN4, ONLINE_BN4_CHIP_FIRST - 1));
	assert(netpass_peer_holds(ONLINE_ROMS_BN4, ONLINE_BN4_CHIP_FIRST) && !netpass_peer_holds(ONLINE_ROMS_BN5, ONLINE_BN4_CHIP_FIRST));
	assert(netpass_peer_holds(0, 5) && netpass_peer_holds(one5, 5) && netpass_peer_holds(one5, 411) && !netpass_peer_holds(one5, 700));
	assert(netpass_peer_holds(one4, 700) && !netpass_peer_holds(one4, 411) && !netpass_peer_holds(0, 411) && netpass_peer_holds(ONLINE_ROMS_FULL, 700));
	assert(!strcmp(netpass_missing_either_words(netpass_game(1)->name, netpass_game(2)->name), "Team Colonel's or Red Sun's ROM stamps Trade"));
	assert(strstr(netpass_complete_one_extra, "this PET") && !strstr(netpass_complete_one_extra, "3DS"));
	assert(!strcmp(netpass_missing_words(netpass_game(1)->name), "Team Colonel's ROM stamps Arena, Trade and DeepNet"));
	assert(!strcmp(netpass_missing_words(netpass_game(2)->name), "Red Sun's ROM stamps Arena, Trade and DeepNet"));
	/* (characters: stamps, never a game) */
	const char *said[] = { trade_no_stamp(), trade_none_stamped(), square_greeter_stamp_words(true), square_greeter_grew_words(false),
		square_greeter_stamp_words(false), square_greeter_grew_words(true), trade_none_shared("Alpha"),
		square_peer_words("Alpha", NULL, false, false), square_week_post(WEEK_OFFER_STAMP, 2965, 0, true) };
	for (size_t i = 0; i < sizeof said / sizeof *said; ++i) assert(!strstr(said[i], "BN5") && !strstr(said[i], "BN6") && !strstr(said[i], "BN4") &&
		!strstr(said[i], "ROM") && !strstr(said[i], "Red Sun"));
	assert(strstr(trade_no_stamp(), "NO TRADE\nSTAMP") && strstr(square_week_post(WEEK_OFFER_STAMP, 2965, 0, true), "NO DEEPNET\nSTAMP"));
	OnlineProfile card = { .hp = 80, .max_hp = 100, .act = 1, .depth = 2 };
	assert(!strstr(square_peer_words("Alpha", &card, true, true), "NetPass") && strstr(square_peer_words("Alpha", &card, false, false), "just the Square"));
	/* (a full pass without the Arena stamp, a PET of one extra ROM: why Battle is not offered with him) */
	assert(strstr(square_peer_words("Alpha", &card, true, false), "no Arena stamp") && !strstr(square_peer_words("Alpha", &card, true, false), "just the Square"));
	assert(strstr(square_greeter_stamp_words(false), "TRADE STAMP") && strstr(square_greeter_grew_words(true), "TRADE STAMP\nSTAYS"));
	/* (the pass's level: the Square's, Trade's on a system of one extra ROM, the full pass's with every stamp) */
	assert(!strcmp(netpass_level_trade, "Trade pass") && strcmp(netpass_level_trade, netpass_level_full));
}

/* a made-up want for the spares list's filter: 1 the even ids, 2 code A */
static bool meets(int want, TradeChip c) { return want == 0 || (want == 1 && !(c.id & 1)) || (want == 2 && c.code == 0); }

/* Trade Central's spares list (issue #145): spares only (the copies past
 * the folders'), a board offer's want, BN6's three sorts with their ties,
 * and the list's words */
static void spares_list(void) {
	assert(trade_list_spare(3, 1) == 2 && trade_list_spare(2, 2) == 0 && trade_list_spare(1, 3) == 0 && trade_list_spare(150, 0) == TRADE_COUNT_MOST);
	/* (Cannon A, its id order 1; AirShot *, 9; a BN5 chip past 319 on its own id; one of none) */
	TradeRow all[] = {
		{ { 411, 12 }, 1, 411, 50 }, { { 1, 0 }, 2, 1, 30 }, { { 9, 26 }, 1, 9, 10 }, { { 1, 2 }, 1, 1, 30 }, { { 20, 5 }, 0, 20, 5 },
		{ { 300, 0 }, 3, 300, 40 },
	};
	TradeRow out[8];
	int n = trade_list_filter(all, 6, -1, meets, out);
	/* (a row of no spare never listed) */
	assert(n == 5);
	for (int i = 0; i < n; ++i) assert(out[i].count > 0 && out[i].chip.id != 20);
	trade_list_sort(out, n, TRADE_SORT_ID);
	assert(out[0].chip.id == 1 && out[0].chip.code == 0 && out[1].chip.id == 1 && out[1].chip.code == 2 && out[2].chip.id == 9 && out[3].chip.id == 300 && out[4].chip.id == 411);
	trade_list_sort(out, n, TRADE_SORT_ABCDE);
	assert(out[0].chip.id == 9 && out[1].chip.id == 1 && out[1].chip.code == 0 && out[2].chip.code == 2 && out[3].chip.id == 300 && out[4].chip.id == 411);
	/* (Code: A to Z, then *; A's ties by ID) */
	trade_list_sort(out, n, TRADE_SORT_CODE);
	assert(out[0].chip.id == 1 && out[1].chip.id == 300 && out[2].chip.code == 2 && out[3].chip.id == 411 && out[4].chip.code == 26);
	assert(trade_list_next_key(TRADE_SORT_ID) == TRADE_SORT_ABCDE && trade_list_next_key(TRADE_SORT_CODE) == TRADE_SORT_ID);
	/* (a want: only the spares that meet it, none where none does) */
	n = trade_list_filter(all, 6, 1, meets, out);
	assert(n == 1 && out[0].chip.id == 300);
	n = trade_list_filter(all, 6, 2, meets, out);
	assert(n == 2 && out[0].chip.id == 1 && out[1].chip.id == 300);
	assert(trade_list_filter(all, 6, 3, meets, out) == 0 && trade_list_filter(all, 6, 1, NULL, out) == 0);
	/* (the screen's words: what is listed, and for whom) */
	assert(strstr(trade_text_list_note(TRADE_FOR_TAKE, 1, "Alpha", ""), "a Fire chip"));
	assert(strstr(trade_text_list_note(TRADE_FOR_ANSWER, -1, "Alpha", "Cannon A"), "Alpha offers Cannon A"));
	assert(!strcmp(trade_text_list_for(TRADE_FOR_POST, NULL), "Trade board") && !strcmp(trade_text_list_for(TRADE_FOR_OFFER, "Beta"), "To Beta"));
}

/* the week's net (issue #127): WEEK_BOARD read and refused where it is
 * malformed, a post's bytes, the Board's words and its dates */
static void week(void) {
	uint8_t m[6 + 4 * ONLINE_WEEK_ENTRY_BYTES] = { ONLINE_WEEK_BOARD };
	online_put16(m + 1, 2963); m[3] = 2; m[4] = 1; m[5] = 2;
	uint8_t *e = m + 6;
	static const char *const names[4] = { "Beta", "Gamma", "Beta", "Alpha" };
	static const int depths[4] = { 5, 10, 5, 7 };
	for (int k = 0; k < 4; ++k, e += ONLINE_WEEK_ENTRY_BYTES) {
		memcpy(e, names[k], strlen(names[k]));
		online_put16(e + 16, (uint16_t)depths[k]); e[18] = (uint8_t)((depths[k] - 1) / 3 + 1 > 4 ? 4 : (depths[k] - 1) / 3 + 1);
		e[19] = depths[k] == 10; online_put16(e + 20, 40);
	}
	online_week_reset();
	assert(online_week_mail(m, sizeof m));
	const WeekBoard *b = online_week_board();
	assert(b->serial == 1 && b->week == 2963 && b->place == 2 && b->ntop == 2 && b->nlast == 1);
	assert(!strcmp(b->top[0].name, "Gamma") && b->top[0].won && b->mine.depth == 5 && !strcmp(b->last[0].name, "Alpha"));
	const char *s = square_week_board(b, 2963, NULL, NULL);
	assert(strstr(s, "Oct 19-Oct 25") && strstr(s, "1.Gamma:10!\n2.Beta:5") && strstr(s, "layer 5,#2") && strstr(s, "1.Alpha:7"));
	assert(!online_week_mail(m, sizeof m - 1));
	m[3] = 9; assert(!online_week_mail(m, sizeof m)); m[3] = 2;
	online_put16(m + 6 + ONLINE_WEEK_ENTRY_BYTES + 16, 11); assert(!online_week_mail(m, sizeof m));
	assert(online_week_board()->serial == 1);
	/* (a week across a month's end, and a post's bytes) */
	assert(strstr(square_week_post(WEEK_OFFER_NEW, 2965, 0, true), "NOV 2-NOV 8") && !*square_week_post(WEEK_OFFER_NEW, 2965, 0, false));
	/* (issue #779: one box, the count the server's, none said where it has not told it) */
	assert(!strcmp(square_week_question(true, 3), "3 NAVIS DIVING!\nDIVE IN?\n") && strstr(square_week_question(true, 1), "1 NAVI DIVING!") &&
		strstr(square_week_question(true, 0), "NO NAVI") && !strstr(square_week_question(false, 0), "NAVI"));
	assert(online_week_post(2963, 0xF521FE36u, 5, 2, 0, 12, 0x1234) && sent_size == ONLINE_WEEK_POST_BYTES);
	assert(sent[0] == ONLINE_WEEK_POST && sent[1] == 0x0B && sent[2] == 0x93 && sent[8] == 5 && sent[9] == 2 && sent[10] == 0 && sent[12] == 12);
	/* (v19: a post's last byte the acts fought in a party, none yet; an entry's bit 1 a party's mark, no bit past it) */
	assert(ONLINE_WEEK_POST_BYTES == 18 && sent[17] == 0);
	online_put16(m + 6 + ONLINE_WEEK_ENTRY_BYTES + 16, 10);
	m[6 + ONLINE_WEEK_ENTRY_BYTES + 19] = ONLINE_WEEK_PARTY;
	assert(online_week_mail(m, sizeof m) && online_week_board()->top[0].party && !online_week_board()->top[0].won);
	m[6 + ONLINE_WEEK_ENTRY_BYTES + 19] = 8; assert(!online_week_mail(m, sizeof m));
	/* (v22: a dive pulled back once by its crew, its post's flags and the Board's mark) */
	assert(online_week_post(2963, 0xF521FE36u, 5, 2, ONLINE_WEEK_PULLED, 12, 0x1234) && sent[10] == ONLINE_WEEK_PULLED);
	assert(online_week_post(2963, 0xF521FE36u, 10, 4, WEEK_POST_WON | 8u, 12, 0x1234) && sent[10] == 1);
	m[6 + ONLINE_WEEK_ENTRY_BYTES + 19] = ONLINE_WEEK_PULLED;
	assert(online_week_mail(m, sizeof m) && online_week_board()->top[0].pulled && !online_week_board()->top[0].party);
	s = square_week_board(online_week_board(), 2963, NULL, NULL);
	assert(strstr(s, "1.Gamma:10*") && strstr(s, "|>*:pulled back once\nby the crew.") && !strstr(s, "|>+:"));
	m[6 + ONLINE_WEEK_ENTRY_BYTES + 19] = ONLINE_WEEK_PARTY;
	assert(online_week_mail(m, sizeof m));
	/* (the Board's v19 posts: the party's mark, who dives now, the week's split records) */
	static DiveDivers d;
	static WeekSplits w;
	d.total = 2; d.count = 2;
	snprintf(d.e[0].name, sizeof d.e[0].name, "Beta"); d.e[0].depth = 4;
	snprintf(d.e[1].name, sizeof d.e[1].name, "Gamma"); d.e[1].depth = 6; d.e[1].flags = ONLINE_DIVERS_FIGHTING;
	w.best[0][0].tenths = 123; snprintf(w.best[0][0].name, sizeof w.best[0][0].name, "Beta");
	w.best[3][2].tenths = 98; snprintf(w.best[3][2].name, sizeof w.best[3][2].name, "Gamma");
	s = square_week_board(online_week_board(), 2963, &d, &w);
	assert(strstr(s, "1.Gamma:10+") && strstr(s, "|>+:a guardian beaten"));
	assert(strstr(s, "|>Diving now:2|Beta:layer 4\nGamma:layer 6!"));
	assert(strstr(s, "|>Fastest guardian\nbusts this week|L3 solo:12.3s Beta\nL10 x3:9.8s Gamma"));
}

/* ---- the shared dive's wire (protocol v19, dive_wire.h): what the game sends, byte for byte as the server's
 * protocol.py packs it, and what the server says, its bytes from protocol.py's encoder, read and refused ---- */

static void dive_sent(void) {
	uint8_t m[ONLINE_MAX_MESSAGE];
	static const uint8_t enter[] = { 41, 1, 11, 147, 245, 33, 254, 54, 3, 2, 7 };
	static const uint8_t end[] = { 41, 3, 1, 255, 251, 0, 6, 0, 0, 0, 145, 0, 42, ONLINE_WEEK_PULLED, 5 };
	static const uint8_t phrase[] = { 41, 5, 4, 0, 10, 255, 236, 0, 0 };
	static const uint8_t split[] = { 41, 7, 0, 1, 44, 9, 1, 2 };
	static const uint8_t crew[] = { 47, 5, 0, 0, 0, 0, 4, 0, 0, 0 };
	static const uint8_t queue[] = { 49, 1, 3, 0, 1 };
	static const uint8_t damage[] = { 51, 0, 0, 0, 77, 0, 120, 0, 80 };
	DiveWay way = { 3, 2, 7 }, none = { 0, 0, 0 };
	assert(dive_wire_enter(m, 2963, 0xF521FE36u, way) == sizeof enter && !memcmp(m, enter, sizeof enter));
	DiveEnd e = { ONLINE_DIVE_DELETED, -5, 6, 0, 0x91, 42, ONLINE_WEEK_PULLED, 5 };
	assert(dive_wire_end(m, &e) == sizeof end && !memcmp(m, end, sizeof end));
	assert(dive_wire_phrase(m, 4, 10, -20, 0) == sizeof phrase && !memcmp(m, phrase, sizeof phrase));
	DiveSplit s = { .index = 0, .party = 2, .tenths = 300, .level = 9, .guardian = true };
	assert(dive_wire_split(m, &s) == sizeof split && !memcmp(m, split, sizeof split));
	DiveWay door = { 3, 0, 1 };
	assert(dive_wire_crew(m, ONLINE_CREW_BATTLE, 0, 4) == sizeof crew && !memcmp(m, crew, sizeof crew));
	assert(dive_wire_queue(m, ONLINE_QUEUE_JOIN, door) == sizeof queue && !memcmp(m, queue, sizeof queue));
	assert(dive_wire_party(m, ONLINE_PARTY_DAMAGE, 77, 120, 80) == sizeof damage && !memcmp(m, damage, sizeof damage));
	assert(dive_wire_leave(m) == ONLINE_DIVE_LEAVE_BYTES && m[0] == ONLINE_DIVE && m[1] == ONLINE_DIVE_LEAVE);
	assert(dive_wire_found(m, 6) == ONLINE_DIVE_FOUND_BYTES && m[2] == 6 && !dive_wire_found(m, 256));
	assert(dive_wire_wave(m, 0x01020304u) == ONLINE_DIVE_WAVE_BYTES && m[2] == 1 && m[5] == 4 && !dive_wire_wave(m, 0));
	/* (each bound the server's: a layer past the net, a coordinate past the limit, a phrase past the Square's, a split's
	 * party, level and time, an ask with nobody, a join with no door, HP past its max) */
	DiveWay deep = { 11, 0, 0 };
	assert(!dive_wire_enter(m, 1, 1, none) && !dive_wire_enter(m, 1, 1, deep));
	DiveEnd bad[] = { { 3, 0, 0, 0, 0, 0, 0, 0 }, { ONLINE_DIVE_WON, 8193, 0, 0, 0, 0, 0, 0 }, { 1, 0, 0, 0, 0x10000, 0, 0, 0 },
		{ 1, 0, 0, 0, 0, 0x10000, 0, 0 }, { 1, 0, 0, 0, 0, 0, 1, 0 }, { 1, 0, 0, 0, 0, 0, 0, 0x10 } };
	for (size_t k = 0; k < sizeof bad / sizeof *bad; ++k) assert(!dive_wire_end(m, &bad[k]));
	assert(!dive_wire_phrase(m, ONLINE_PHRASE_COUNT, 0, 0, 0) && !dive_wire_phrase(m, 0, 0, -8193, 0));
	s.party = 5; assert(!dive_wire_split(m, &s)); s.party = 1; s.level = 12; assert(!dive_wire_split(m, &s));
	s.level = 11; s.tenths = 0; assert(!dive_wire_split(m, &s)); s.tenths = 1; assert(dive_wire_split(m, &s) && m[5] == ONLINE_BUSTING_MOST);
	/* (v20: a crew by nearness, its asks and ways gone; a join names its owner) */
	assert(!dive_wire_crew(m, 1, 2, 0) && !dive_wire_crew(m, 4, 0, 0) && !dive_wire_crew(m, 8, 1, 0) && !dive_wire_crew(m, ONLINE_CREW_JOIN, 0, 0));
	assert(dive_wire_crew(m, ONLINE_CREW_BATTLE, 0, 200) && m[6] == 200 && !dive_wire_crew(m, ONLINE_CREW_BATTLE, 0, 256));
	assert(dive_wire_crew(m, ONLINE_CREW_JOIN, 9, 0) && m[5] == 9 && !dive_wire_crew(m, ONLINE_CREW_JOIN, 9, 1));
	/* (v23: a busy game's answer names the owner too) */
	assert(dive_wire_crew(m, ONLINE_CREW_BUSY, 9, 0) && m[1] == ONLINE_CREW_BUSY && m[5] == 9 && !dive_wire_crew(m, ONLINE_CREW_BUSY, 0, 0));
	assert(!dive_wire_queue(m, ONLINE_QUEUE_JOIN, none) && !dive_wire_queue(m, 5, door) && dive_wire_queue(m, ONLINE_QUEUE_PASS, none));
	assert(!dive_wire_party(m, ONLINE_PARTY_HP, 77, 10, 5) && !dive_wire_party(m, ONLINE_PARTY_DAMAGE, 0, 1, 1) && !dive_wire_party(m, 45, 1, 1, 1));
}

static void dive_heard_traces(void) {
	static const uint8_t phrases[] = { 43, 2, 1, 0, 0, 0, 7, 65, 108, 112, 104, 97, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 10, 255,
		236, 0, 0, 0, 1 };
	static const uint8_t marks[] = { 43, 1, 1, 66, 101, 116, 97, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 251, 0, 6, 0, 0, 0, 145, 1 };
	static DiveTraces t;
	assert(dive_wire_traces(phrases, sizeof phrases, &t) && t.kind == ONLINE_TRACE_PHRASES && t.count == 1);
	assert(t.u.phrases[0].id == 7 && !strcmp(t.u.phrases[0].name, "Alpha") && t.u.phrases[0].phrase == 4 && t.u.phrases[0].y == -20 &&
		t.u.phrases[0].waves == 1);
	assert(dive_wire_traces(marks, sizeof marks, &t) && t.kind == ONLINE_TRACE_MARKS && t.u.marks[0].x == -5 && t.u.marks[0].cause == 0x91 &&
		t.u.marks[0].how == ONLINE_DIVE_DELETED && !strcmp(t.u.marks[0].name, "Beta"));
	assert(!dive_wire_traces(marks, sizeof marks - 1, &t) && !dive_wire_traces(phrases, 2, &t));
	uint8_t m[ONLINE_MAX_MESSAGE];
	memcpy(m, marks, sizeof marks);
	m[27] = 3; assert(!dive_wire_traces(m, sizeof marks, &t)); m[27] = 1;
	m[1] = 7; assert(!dive_wire_traces(m, sizeof marks, &t)); m[1] = 1;
	m[10] = 'x'; assert(!dive_wire_traces(m, sizeof marks, &t));        /* (a name with a byte past its end) */
	/* (the opened set: as many indexes as a message holds; the gone phrases, never id 0) */
	m[0] = ONLINE_DIVE_TRACES; m[1] = ONLINE_TRACE_OPENED; m[2] = 255;
	for (int k = 0; k < 255; ++k) m[3 + k] = (uint8_t)k;
	assert(dive_wire_traces(m, 258, &t) && t.count == 255 && t.u.opened[254] == 254);
	m[1] = ONLINE_TRACE_GONE; m[2] = 2;
	memset(m + 3, 0, 8); m[6] = 9; m[10] = 1;
	assert(dive_wire_traces(m, 11, &t) && t.u.gone[0] == 9 && t.u.gone[1] == 1);
	m[10] = 0; assert(!dive_wire_traces(m, 11, &t));
	/* (a split: a guardian's, BN6's level 11 for S at most) */
	m[1] = ONLINE_TRACE_SPLITS; m[2] = 1;
	static const uint8_t split[] = { 9, 2, 1, 154, 11, 1, 'G', 'a', 'm', 'm', 'a' };
	memset(m + 3, 0, ONLINE_TRACE_SPLIT_BYTES); memcpy(m + 3, split, sizeof split);
	assert(dive_wire_traces(m, 3 + ONLINE_TRACE_SPLIT_BYTES, &t) && t.u.splits[0].tenths == 410 && t.u.splits[0].guardian &&
		t.u.splits[0].party == 2 && !strcmp(t.u.splits[0].name, "Gamma"));
	m[7] = 12; assert(!dive_wire_traces(m, 3 + ONLINE_TRACE_SPLIT_BYTES, &t));
}

static void dive_heard_rooms_and_feed(void) {
	static const uint8_t room[] = { 42, 11, 147, 3, 2, 7, 0, 1, 2 };
	static const uint8_t feed[] = { 45, 3, 66, 101, 116, 97, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 1, 0, 0, 3 };
	static const uint8_t place[] = { 44, 0, 1, 0, 250, 0, 2, 0, 2, 0, 200, 66, 101, 116, 97, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	static const uint8_t divers[] = { 46, 0, 3, 2, 71, 97, 109, 109, 97, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 0, 65, 108, 112, 104,
		97, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 1 };
	DiveRoom r;
	assert(dive_wire_room(room, sizeof room, &r) && r.week == 2963 && r.way.depth == 3 && r.way.side == 2 && r.way.way == 7 &&
		r.instance == 1 && r.divers == 2);
	uint8_t m[ONLINE_MAX_MESSAGE];
	memcpy(m, room, sizeof room); m[8] = 9; assert(!dive_wire_room(m, sizeof room, &r));
	m[8] = 2; m[7] = 0; assert(!dive_wire_room(m, sizeof room, &r));
	DiveFeed f;
	assert(dive_wire_feed(feed, sizeof feed, &f) && f.kind == ONLINE_FEED_GUARDIAN && !strcmp(f.name, "Beta") && f.way.depth == 3 &&
		f.way.way == 1 && f.party == 3);
	memcpy(m, feed, sizeof feed); m[23] = 1; assert(!dive_wire_feed(m, sizeof feed, &f));
	m[23] = 0; m[1] = ONLINE_FEED_FLARE; assert(dive_wire_feed(m, sizeof feed, &f)); m[1] = 5; assert(!dive_wire_feed(m, sizeof feed, &f));
	DivePlace p;
	assert(dive_wire_place(place, sizeof place, &p) && p.tenths == 250 && p.place == 2 && p.count == 2 && p.best == 200 &&
		!strcmp(p.best_name, "Beta"));
	memcpy(m, place, sizeof place); m[6] = 3; assert(!dive_wire_place(m, sizeof place, &p));
	DiveDivers d;
	assert(dive_wire_divers(divers, sizeof divers, &d) && d.total == 3 && d.count == 2 && !strcmp(d.e[0].name, "Gamma") &&
		d.e[0].depth == 4 && d.e[0].act == 2 && d.e[1].flags == ONLINE_DIVERS_FIGHTING);
	assert(!dive_wire_divers(divers, sizeof divers - 1, &d));
	memcpy(m, divers, sizeof divers); m[2] = 1; assert(!dive_wire_divers(m, sizeof divers, &d));
	static const uint8_t none[] = { ONLINE_DIVERS, 0, 0, 0 };
	assert(dive_wire_divers(none, sizeof none, &d) && !d.total && !d.count);
}

static void dive_heard_crews_and_parties(void) {
	uint8_t m[ONLINE_MAX_MESSAGE] = { 0 };
	/* (CREW_STATE formed, two members: protocol.crew_state's bytes, its zeros past them; v24, each one's HP, max HP and layer; v26, where he is) */
	static const uint8_t crew[] = { 48, 3, 0, 0, 0, 2, 0, 0, 0, 0, 1, 65, 108, 112, 104, 97 };
	static const uint8_t mate[] = { 0, 0, 0, 2, 66, 101, 116, 97, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 120, 0, 200, 3, ONLINE_WHERE_HOME };
	memcpy(m, crew, sizeof crew); memcpy(m + 7 + ONLINE_CREW_MEMBER_BYTES, mate, sizeof mate);
	CrewState c;
	assert(dive_wire_crew_state(m, ONLINE_CREW_STATE_BYTES, &c) && c.stage == ONLINE_CREW_FORMED && c.other == 2 && c.count == 2 &&
		c.m[0].session == 1 && c.m[1].session == 2 && !strcmp(c.m[1].name, "Beta") && c.m[1].hp == 120 && c.m[1].max_hp == 200 &&
		c.m[1].depth == 3 && c.m[1].where == ONLINE_WHERE_HOME && !c.m[0].max_hp && c.m[0].where == ONLINE_WHERE_LAYER);
	m[1] = ONLINE_CREW_VITALS; assert(dive_wire_crew_state(m, ONLINE_CREW_STATE_BYTES, &c) && c.stage == ONLINE_CREW_VITALS);
	m[1] = ONLINE_CREW_FULL; assert(dive_wire_crew_state(m, ONLINE_CREW_STATE_BYTES, &c)); m[1] = 9; assert(!dive_wire_crew_state(m, ONLINE_CREW_STATE_BYTES, &c)); m[1] = 1; assert(!dive_wire_crew_state(m, ONLINE_CREW_STATE_BYTES, &c)); m[1] = 3;
	m[7 + ONLINE_CREW_MEMBER_BYTES + 21] = 201; assert(!dive_wire_crew_state(m, ONLINE_CREW_STATE_BYTES, &c)); m[7 + ONLINE_CREW_MEMBER_BYTES + 21] = 120;   /* (HP past his max) */
	m[7 + 2 * ONLINE_CREW_MEMBER_BYTES + 24] = 1; assert(!dive_wire_crew_state(m, ONLINE_CREW_STATE_BYTES, &c)); m[7 + 2 * ONLINE_CREW_MEMBER_BYTES + 24] = 0;   /* (a layer past the members) */
	m[7 + ONLINE_CREW_MEMBER_BYTES + 25] = 4; assert(!dive_wire_crew_state(m, ONLINE_CREW_STATE_BYTES, &c)); m[7 + ONLINE_CREW_MEMBER_BYTES + 25] = 1;   /* (a where past busy) */
	m[7 + 2 * ONLINE_CREW_MEMBER_BYTES + 25] = 1; assert(!dive_wire_crew_state(m, ONLINE_CREW_STATE_BYTES, &c)); m[7 + 2 * ONLINE_CREW_MEMBER_BYTES + 25] = 0;   /* (a where past the members) */
	uint8_t wh[ONLINE_DIVE_WHERE_BYTES];
	assert(dive_wire_where(wh, ONLINE_WHERE_HOME) == sizeof wh && wh[0] == ONLINE_DIVE && wh[1] == ONLINE_DIVE_WHERE && wh[2] == 1 && !dive_wire_where(wh, 4) && dive_wire_where(wh, ONLINE_WHERE_BUSY));
	uint8_t v[ONLINE_DIVE_VITALS_BYTES];
	static const uint8_t vitals[] = { ONLINE_DIVE, ONLINE_DIVE_VITALS, 0, 120, 0, 200 };
	assert(dive_wire_vitals(v, 120, 200) == sizeof v && !memcmp(v, vitals, sizeof v) && !dive_wire_vitals(v, 201, 200) && !dive_wire_vitals(v, 0, 0) &&
		!dive_wire_vitals(v, 10, 10000));
	m[7 + 3] = 0; assert(!dive_wire_crew_state(m, ONLINE_CREW_STATE_BYTES, &c)); m[7 + 3] = 1;   /* (a hole before a member) */
	/* (QUEUE_STATE waiting: Beta leads, his system runs the Arena; Gamma's does not; the recipient second) */
	static const uint8_t queue[] = { 50, 1, 3, 0, 1, 2, 2, 0, 0, 0, 5, 66, 101, 116, 97 };
	static const uint8_t second[] = { 0, 0, 0, 6, 71, 97, 109, 109, 97 };
	memset(m, 0, sizeof m); memcpy(m, queue, sizeof queue); m[7 + 20] = ONLINE_QUEUE_ARENA;
	memcpy(m + 7 + ONLINE_QUEUE_ENTRY_BYTES, second, sizeof second);
	QueueState q;
	assert(dive_wire_queue_state(m, ONLINE_QUEUE_STATE_BYTES, &q) && q.stage == ONLINE_QUEUE_WAITING && q.place == 2 && q.count == 2 &&
		q.e[0].session == 5 && q.e[0].flags == ONLINE_QUEUE_ARENA && !q.e[1].flags && q.door.depth == 3 && q.door.way == 1);
	m[6] = 3; assert(!dive_wire_queue_state(m, ONLINE_QUEUE_STATE_BYTES, &q)); m[6] = 2;
	m[5] = 3; assert(!dive_wire_queue_state(m, ONLINE_QUEUE_STATE_BYTES, &q));
	/* (a party's START: protocol.arena_state's bytes, the recipient's seat 1 of three, the party's format) */
	static const uint8_t start[] = { 18, 3, 0, 0, 0, 77, 0, 0, 0, 9, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171, 171,
		171, 171, 171, 1, 2, 2, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 10, 0, 0, 0, 0 };
	PartyState s;
	assert(dive_wire_party_state(start, sizeof start, &s) && s.stage == 3 && s.battle == 77 && s.seat == 1 && s.delay == 2 &&
		s.form == ONLINE_ARENA_PARTY && s.seats[2] == 10 && !s.seats[3]);
	memcpy(m, start, sizeof start); m[28] = 1; assert(!dive_wire_party_state(m, sizeof start, &s));      /* (a 2v2's, not a party's) */
	m[28] = ONLINE_ARENA_LINKED; m[26] = 3; assert(!dive_wire_party_state(m, sizeof start, &s));        /* (a seat past the party) */
	m[26] = 2; m[1] = ONLINE_ARENA_SEAT_GONE; assert(dive_wire_party_state(m, sizeof start, &s) && s.seat == 2);
	m[1] = 2; assert(!dive_wire_party_state(m, sizeof start, &s));
	/* (a linked fighter's damage at seat 0, seat 0's HP at the linked) */
	static const uint8_t damage[] = { ONLINE_ARENA_SEAT, 2, ONLINE_PARTY_DAMAGE, 0, 0, 0, 77, 0, 120, 0, 80 };
	unsigned seat, dealt, hp, max_hp;
	uint32_t battle;
	assert(dive_wire_damage(damage, sizeof damage, &seat, &battle, &dealt, &hp) && seat == 2 && battle == 77 && dealt == 120 && hp == 80);
	memcpy(m, damage, sizeof damage); m[1] = 0; assert(!dive_wire_damage(m, sizeof damage, &seat, &battle, &dealt, &hp));
	uint8_t heard[ONLINE_PARTY_BYTES];
	assert(dive_wire_party(heard, ONLINE_PARTY_HP, 77, 880, 1000) && dive_wire_party_hp(heard, sizeof heard, &battle, &hp, &max_hp) &&
		hp == 880 && max_hp == 1000);
	heard[5] = 0xFF; assert(!dive_wire_party_hp(heard, sizeof heard, &battle, &hp, &max_hp));
	/* (WEEK_SPLITS: act 1's guardian by a party of two, the rest none) */
	memset(m, 0, sizeof m);
	m[0] = ONLINE_WEEK_SPLITS; online_put16(m + 1, 2963); online_put16(m + 3 + 18, 410); memcpy(m + 3 + 18 + 2, "Alpha", 5);
	static WeekSplits w;
	assert(dive_wire_splits(m, ONLINE_WEEK_SPLITS_BYTES, &w) && w.week == 2963 && w.best[0][1].tenths == 410 &&
		!strcmp(w.best[0][1].name, "Alpha") && !w.best[3][3].tenths);
	m[3 + 2] = 'Z'; assert(!dive_wire_splits(m, ONLINE_WEEK_SPLITS_BYTES, &w));    /* (a name where no bust is) */
	assert(!dive_wire_splits(m, ONLINE_WEEK_SPLITS_BYTES - 1, &w));
}

/* What BN6's PET changed in the Square, carried home (pet_carry.h): a folder's bytes whole and only where home
 * stands as the PET found it; a flag a bit at a time, the Square's own bits in its byte left alone; a change
 * changed back is none; a later session's bytes over an earlier one's; a record that lies refused */
static void pet_carried(void) {
	static PetCarry c;
	static uint8_t open[PET_CARRY_BYTES], shut[PET_CARRY_BYTES], home[PET_CARRY_BYTES];
	const unsigned bits = 0x4F0, folder = bits + 6, flag = 0x40;
	for (unsigned k = 0; k < PET_CARRY_BYTES; ++k) open[k] = home[k] = (uint8_t)(k * 7);
	assert(pet_carry_regions[0].at == BN6_EVENT_FLAGS && pet_carry_regions[0].bytes == PET_CARRY_MAIN && bits < PET_CARRY_MAIN);
	unsigned total = 0;
	for (int r = 0; r < PET_CARRY_REGIONS; ++r) total += pet_carry_regions[r].bytes;
	assert(total == PET_CARRY_BYTES);
	pet_carry_clear(&c, 77, 7, bits);
	/* (a session: a folder's entry, and a flag set in a byte where the Square holds another of its own) */
	memcpy(shut, open, sizeof shut);
	shut[folder] = 0x14; shut[folder + 1] = 0x14;
	open[flag] = 0x02; shut[flag] = 0x42;
	assert(pet_carry_note(&c, open, shut) == 3 && c.count == 3 && pet_carry_sound(&c));
	/* (home: the Square's own bit not set there, another set; the PET's bit alone comes) */
	home[flag] = 0x01;
	assert(pet_carry_fits(&c, home));
	uint8_t was = home[folder + 2];
	pet_carry_apply(&c, home);
	assert(home[folder] == 0x14 && home[folder + 1] == 0x14 && home[folder + 2] == was && home[flag] == 0x41);
	/* (a checkpoint whose folder is another: nothing fits) */
	home[folder] = 0x99;
	assert(!pet_carry_fits(&c, home));
	/* (a second session over the first: the entry again, as the first session found it; then changed back) */
	memcpy(open, shut, sizeof open);
	shut[folder] = 0x15;
	uint8_t first = (uint8_t)(folder * 7);
	assert(pet_carry_note(&c, open, shut) == 1 && c.count == 3 && c.old[folder] == first && c.now[folder] == 0x15);
	memcpy(open, shut, sizeof open);
	shut[folder] = first;
	assert(pet_carry_note(&c, open, shut) == 1 && c.count == 2 && !c.mask[folder]);
	/* (a file's record: its count as its bytes say, a kept byte whole or not at all) */
	c.count = 9; assert(!pet_carry_sound(&c)); c.count = 2;
	c.mask[folder + 1] = 0x0F; assert(!pet_carry_sound(&c)); c.mask[folder + 1] = 0xFF;
	c.traded = 1; assert(pet_carry_sound(&c));
}

/* ---- a crewmate pulled back (issue #718, crew_pull.h): the owner's calls of 21 November 2026 ---- */

static void crew_pulled(void) {
	CrewPullOut o;
	/* (the proposal's example: 600 max HP, two crewmates ending at 480 and 400: they give 120 and 100, he is back with 220) */
	CrewPullIn in = { .n = 3, .hp = { 480, 0, 400 }, .max_hp = { 600, 600, 500 } };
	crew_pull(&in, true, &o);
	assert(o.pulled[1] && o.hp[1] == 220 && o.gave[0] == 120 && o.gave[2] == 100 && o.hp[0] == 360 && o.hp[2] == 300);
	assert(!o.pulled[0] && !o.pulled[2] && !o.gave[1]);
	/* (the sum half his max at most: the last gift cut to it) */
	CrewPullIn rich = { .n = 4, .hp = { 0, 1000, 1000, 1000 }, .max_hp = { 400, 1000, 1000, 1000 } };
	crew_pull(&rich, true, &o);
	assert(o.hp[0] == 200 && o.gave[1] == 200 && !o.gave[2] && !o.gave[3] && o.hp[1] == 800 && o.hp[2] == 1000);
	/* (a giver never deleted by it; 1 HP at least for the fallen) */
	CrewPullIn poor = { .n = 2, .hp = { 3, 0 }, .max_hp = { 500, 500 } };
	crew_pull(&poor, true, &o);
	assert(o.hp[0] == 3 && o.hp[1] == 1 && o.pulled[1] && !o.gave[0]);
	/* (once a dive, a battle lost, a game gone: as the field ended) */
	CrewPullIn spent = in;
	spent.spent[1] = true;
	crew_pull(&spent, true, &o);
	assert(!o.pulled[1] && !o.hp[1] && o.hp[0] == 480 && !o.gave[0]);
	crew_pull(&in, false, &o);
	assert(!o.pulled[1] && !o.hp[1] && o.hp[2] == 400);
	CrewPullIn none = { .n = 2, .max_hp = { 500, 500 } };
	crew_pull(&none, true, &o);
	assert(!o.pulled[0] && !o.pulled[1] && !o.hp[0] && !o.hp[1]);
	CrewPullIn gone = in;
	gone.gone[1] = true;
	crew_pull(&gone, true, &o);
	assert(!o.pulled[1] && !o.gave[0]);
	/* (two fallen: each in seat order, the givers' quarters of what they have left; one just pulled back gives nothing) */
	CrewPullIn two = { .n = 4, .hp = { 0, 800, 0, 400 }, .max_hp = { 600, 800, 600, 400 } };
	crew_pull(&two, true, &o);
	assert(o.hp[0] == 300);
	assert(o.hp[2] == 225 && o.gave[1] == 350 && o.gave[3] == 175 && o.hp[1] == 450 && o.hp[3] == 225);
}

/* Who gets a layer's puppets (issue #755): the crew first, then the nearest; a diver keeps his */
static void dive_seated(void) {
	/* (two puppets, four others: a stranger nearest, his crew far) */
	SeatDiver d[4] = { { 11, 10, 0, false }, { 12, 20, 0, false }, { 13, 500, 0, true }, { 14, 900, 0, true } };
	uint32_t seat[2] = { 11, 12 };
	assert(dive_seats(seat, 2, d, 4, 0, 0) == 2);
	assert(seat[0] == 13 && seat[1] == 14);
	/* (one crewmate: he, then the nearer stranger, who keeps the puppet he had) */
	d[3].crew = false;
	seat[0] = 0; seat[1] = 11;
	assert(dive_seats(seat, 2, d, 4, 0, 0) == 2);
	assert(seat[1] == 11 && seat[0] == 13);
	/* (strangers walk: the nearer two seated, the one still chosen keeps his) */
	d[2].crew = false;
	d[0].x = 1000;
	assert(dive_seats(seat, 2, d, 4, 0, 0) == 2);
	assert(seat[0] == 12 || seat[1] == 12);
	assert(seat[0] == 13 && seat[1] == 12);
	/* (room for all: each seated, none left out; none twice) */
	uint32_t four[5] = { 0, 0, 0, 0, 0 };
	assert(dive_seats(four, 5, d, 4, 0, 0) == 0);
	int seen = 0;
	for (int k = 0; k < 5; ++k) seen += four[k] != 0;
	assert(seen == 4 && four[4] == 0);
	/* (no room, or nobody) */
	assert(dive_seats(four, 0, d, 4, 0, 0) == 4);
	assert(dive_seats(four, 5, d, 0, 0, 0) == 0 && !four[0] && !four[3]);
}


/* the Square's peers' looks (issue #854): as many as the part has room for, the nearest first */
static Look look_of(int p) {
	Look l;
	for (int c = 0; c < LOOK_CHANNELS; ++c) { l.palette[c] = (uint8_t)p; l.colour[c] = (uint8_t)c; }
	return l;
}

static void peer_looks(void) {
	Look plain;
	for (int c = 0; c < LOOK_CHANNELS; ++c) plain.palette[c] = plain.colour[c] = LOOK_NONE;
	LookPeer peers[5];
	for (int i = 0; i < 5; ++i) peers[i] = (LookPeer){ look_of(i + 1), 100 - i * 10, false };
	bool shown[5];
	/* (MegaMan in BN6's own colours: three looks, the nearest three; the rest drawn as MegaMan, no palette more) */
	square_looks_pick(peers, 5, &plain, 3, 0, shown);
	assert(!shown[0] && !shown[1] && shown[2] && shown[3] && shown[4]);
	/* (MegaMan in a look of his own: BN6's own colours for those left over take one of the three) */
	Look own = look_of(9);
	square_looks_pick(peers, 5, &own, 3, 0, shown);
	assert(!shown[0] && !shown[1] && !shown[2] && shown[3] && shown[4]);
	/* (a peer in MegaMan's look, or in one drawn already, costs nothing) */
	peers[0].look = own; peers[1].look = look_of(5);
	square_looks_pick(peers, 5, &own, 3, 0, shown);
	for (int i = 0; i < 5; ++i) assert(shown[i]);
	/* (one look more than the room: the farthest in BN6's own colours, which take a palette, so two of them) */
	peers[0].look = look_of(7);
	square_looks_pick(peers, 5, &own, 3, 0, shown);
	assert(!shown[0] && shown[1] && !shown[2] && shown[3] && shown[4]);
	/* (one drawn in his look keeps it against one a step nearer) */
	for (int i = 0; i < 5; ++i) peers[i] = (LookPeer){ look_of(i + 1), 100 + i, i == 4 };
	square_looks_pick(peers, 5, &plain, 1, 24, shown);
	assert(shown[4] && !shown[0]);
	square_looks_pick(peers, 5, &plain, 1, 0, shown);
	assert(shown[0] && !shown[4]);
	/* (a peer in BN6's own colours beside MegaMan in them: free) */
	peers[1].look = plain;
	square_looks_pick(peers, 5, &plain, 1, 0, shown);
	assert(shown[0] && shown[1]);
}

int main(void) {
	peer_looks();
	char directory[] = "/tmp/cyberworld-people-XXXXXX";
	assert(mkdtemp(directory)); snprintf(g_data_dir, sizeof g_data_dir, "%s", directory);
	online_people_load(); positions(); cards_and_events(); motion(); fuzz(); square(); referee(); week(); netpass(); spares_list(); pet_carried();
	dive_sent(); dive_heard_traces(); dive_heard_rooms_and_feed(); dive_heard_crews_and_parties(); crew_pulled(); dive_seated();
	char path[600]; snprintf(path, sizeof path, "%s/network-people.ini", directory); unlink(path); rmdir(directory);
	puts("Online protocol, malformed messages, motion, local moderation, the Square's plan, the referee, the week's net, the NetPass, Trade Central's spares list, the PET's changes carried home and the shared dive's wire passed");
	return 0;
}
