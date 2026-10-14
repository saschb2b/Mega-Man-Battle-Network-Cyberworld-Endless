/* Hostile server records, delayed movement and persistent local moderation. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "online_motion.h"
#include "online_people.h"
#include "online_wire.h"
#include "platform.h"

char g_data_dir[512];
static unsigned persists;
void platform_persist(void) { ++persists; }

static void welcome(OnlineSnapshot *out) {
	uint8_t bytes[32] = { ONLINE_WELCOME };
	online_put32(bytes + 1, 120); online_put32(bytes + 5, 1);
	online_put16(bytes + 9, 20); bytes[11] = 8;
	online_put16(bytes + 12, 1); bytes[14] = 99; online_put16(bytes + 30, 8);
	assert(online_wire_message(bytes, sizeof bytes, 100, out));
	assert(out->connected && out->channel == 1 && out->channels == 8 && out->identity[0] == 99);
	for (size_t size = 0; size < sizeof bytes; ++size) assert(!online_wire_message(bytes, size, 100, out));
	bytes[11] = 9; assert(!online_wire_message(bytes, sizeof bytes, 100, out));
}

static void positions(void) {
	OnlineSnapshot out = { 0 };
	welcome(&out);
	uint8_t bytes[2 + ONLINE_PEER_BYTES * 2] = { ONLINE_SNAPSHOT, 1 };
	online_put32(bytes + 2, 2); bytes[6] = 42;
	online_put32(bytes + 22, (uint32_t)-500); online_put32(bytes + 26, 400);
	bytes[34] = 7; memcpy(bytes + 35, "Player-2", 8);
	assert(online_wire_message(bytes, 51, 100, &out));
	assert(out.peer_count == 1 && out.peers[0].x == -500 && out.peers[0].y == 400 && out.peers[0].identity[0] == 42);
	for (size_t size = 0; size < 51; ++size) assert(!online_wire_message(bytes, size, 100, &out));
	bytes[34] = 8; assert(!online_wire_message(bytes, 51, 100, &out)); bytes[34] = 7;
	online_put32(bytes + 22, 8193); assert(!online_wire_message(bytes, 51, 100, &out));
	online_put32(bytes + 22, 1);
	bytes[1] = 2; memcpy(bytes + 51, bytes + 2, ONLINE_PEER_BYTES);
	assert(!online_wire_message(bytes, sizeof bytes, 100, &out));
	online_put32(bytes + 51, 3); assert(!online_wire_message(bytes, sizeof bytes, 100, &out));
	bytes[55] = 43; assert(online_wire_message(bytes, sizeof bytes, 100, &out));
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

int main(void) {
	char directory[] = "/tmp/cyberworld-people-XXXXXX";
	assert(mkdtemp(directory)); snprintf(g_data_dir, sizeof g_data_dir, "%s", directory);
	online_people_load(); positions(); cards_and_events(); motion(); fuzz();
	char path[600]; snprintf(path, sizeof path, "%s/network-people.ini", directory); unlink(path); rmdir(directory);
	puts("Online protocol, malformed messages, motion and local moderation passed");
	return 0;
}
