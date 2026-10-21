/* Hostile server records, delayed movement and persistent local moderation. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "netpass.h"
#include "netpass_text.h"
#include "online_motion.h"
#include "online_people.h"
#include "online_transport.h"
#include "online_week.h"
#include "online_wire.h"
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
	bytes[51] = 2; assert(!online_wire_message(bytes, ONE, 100, &out)); bytes[51] = 0;
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
	uint8_t event[7] = { ONLINE_EVENT, 0, 0, 0, 2, 1, 5 };
	assert(online_wire_message(event, sizeof event, 100, &out));
	assert(out.events[1].player_id == 2 && out.events[1].value == 5);
	event[6] = 6; assert(!online_wire_message(event, sizeof event, 100, &out));
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
		if (x >= p->stairs[i].x && x < p->stairs[i].x + 2 && y >= p->stairs[i].y && y < p->stairs[i].y + 2) return true;
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

/* every floor panel of plan `p` reached from the arrival, a step between
 * heights only on a stair, and every place on the floor */
static void square_reach(const SquarePlan *p) {
	static uint8_t seen[SQUARE_H][SQUARE_W];
	memset(seen, 0, sizeof seen);
	static int stack[SQUARE_W * SQUARE_H][2];
	int n = 0, reached = 0, floor = 0;
	stack[n][0] = p->at[SQ_ARRIVE][0]; stack[n++][1] = p->at[SQ_ARRIVE][1];
	seen[p->at[SQ_ARRIVE][1]][p->at[SQ_ARRIVE][0]] = 1;
	while (n) {
		--n;
		int x = stack[n][0], y = stack[n][1];
		++reached;
		static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int nx = x + d[k][0], ny = y + d[k][1];
			if (nx < 0 || ny < 0 || nx >= SQUARE_W || ny >= SQUARE_H || seen[ny][nx] || p->cell[ny][nx] != C_PATH) continue;
			if (p->level[ny][nx] != p->level[y][x] && !on_stair(p, x, y) && !on_stair(p, nx, ny)) continue;
			seen[ny][nx] = 1;
			stack[n][0] = nx; stack[n++][1] = ny;
		}
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
				assert(p->cell[y][x] == C_PATH && !p->level[y][x]);
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
	assert(netpass_games() == 2 && !strcmp(netpass_game(0)->label, "BN6") && !strcmp(netpass_game(1)->label, "BN5"));
	assert(netpass_game(1)->bit == ONLINE_ROMS_BN5 && !netpass_game(2));
	/* (BN6's ROM alone: the Square's stamp) */
	assert(netpass_game_in(0) && !netpass_game_in(1) && !netpass_full());
	assert(netpass_has(STAMP_SQUARE) && !netpass_has(STAMP_ARENA) && !netpass_has(STAMP_TRADE) && !netpass_has(STAMP_DEEPNET));
	/* (BN5's beside it: every stamp) */
	static uint8_t bn5;
	XR[XROM_BN5_COLONEL_US].data = &bn5;
	assert(netpass_full() && netpass_has(STAMP_ARENA) && netpass_has(STAMP_TRADE) && netpass_has(STAMP_DEEPNET));
	XR[XROM_BN5_COLONEL_US].data = NULL;
	/* (another game's pass, by the bits every full set needs: protocol.json's roms_full) */
	assert(!netpass_peer_full(0) && netpass_peer_full(ONLINE_ROMS_BN5) && netpass_peer_full(ONLINE_ROMS_FULL));
	assert(!strcmp(netpass_missing_words(netpass_game(1)->name), "Team Colonel's ROM stamps Arena, Trade and DeepNet"));
	/* (characters: stamps, never a game) */
	const char *said[] = { trade_no_stamp(), trade_none_stamped(), square_greeter_stamp_words(),
		square_peer_words("Alpha", NULL, false), square_week_post(WEEK_OFFER_STAMP, 2965, 0) };
	for (size_t i = 0; i < sizeof said / sizeof *said; ++i) assert(!strstr(said[i], "BN5") && !strstr(said[i], "BN6") && !strstr(said[i], "ROM"));
	assert(strstr(trade_no_stamp(), "NO TRADE\nSTAMP") && strstr(square_week_post(WEEK_OFFER_STAMP, 2965, 0), "NO DEEPNET\nSTAMP"));
	OnlineProfile card = { .hp = 80, .max_hp = 100, .act = 1, .depth = 2 };
	assert(!strstr(square_peer_words("Alpha", &card, true), "NetPass") && strstr(square_peer_words("Alpha", &card, false), "just the Square"));
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
	const char *s = square_week_board(b, 2963);
	assert(strstr(s, "Oct 19-Oct 25") && strstr(s, "1.Gamma:10!\n2.Beta:5") && strstr(s, "layer 5,#2") && strstr(s, "1.Alpha:7"));
	assert(!online_week_mail(m, sizeof m - 1));
	m[3] = 9; assert(!online_week_mail(m, sizeof m)); m[3] = 2;
	online_put16(m + 6 + ONLINE_WEEK_ENTRY_BYTES + 16, 11); assert(!online_week_mail(m, sizeof m));
	assert(online_week_board()->serial == 1);
	/* (a week across a month's end, and a post's bytes) */
	assert(strstr(square_week_post(WEEK_OFFER_NEW, 2965, 0), "NOV 2-NOV 8"));
	assert(online_week_post(2963, 0xF521FE36u, 5, 2, false, 12, 0x1234) && sent_size == ONLINE_WEEK_POST_BYTES);
	assert(sent[0] == ONLINE_WEEK_POST && sent[1] == 0x0B && sent[2] == 0x93 && sent[8] == 5 && sent[9] == 2 && sent[10] == 0 && sent[12] == 12);
}

int main(void) {
	char directory[] = "/tmp/cyberworld-people-XXXXXX";
	assert(mkdtemp(directory)); snprintf(g_data_dir, sizeof g_data_dir, "%s", directory);
	online_people_load(); positions(); cards_and_events(); motion(); fuzz(); square(); referee(); week(); netpass(); spares_list();
	char path[600]; snprintf(path, sizeof path, "%s/network-people.ini", directory); unlink(path); rmdir(directory);
	puts("Online protocol, malformed messages, motion, local moderation, the Square's plan, the referee, the week's net, the NetPass and Trade Central's spares list passed");
	return 0;
}
