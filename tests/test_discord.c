/* test_discord.h. */
#include "test_discord.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "discord.h"
#include "discord_text.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

#define DOT " \xC2\xB7 "

static bool starts(const char *s, const char *head) { return !strncmp(s, head, strlen(head)); }

static bool ends(const char *s, const char *tail) {
	size_t a = strlen(s), b = strlen(tail);
	return a >= b && !strcmp(s + a - b, tail);
}

/* A frame written, then read back whole, and not before all of it came */
static void frames(void) {
	uint8_t buf[64];
	const char *json = "{\"v\":1}";
	size_t n = discord_frame_put(buf, sizeof buf, DISCORD_OP_HANDSHAKE, json, strlen(json));
	CHECK(n == DISCORD_HEADER + 7, "a frame is its header and its JSON: %zu", n);
	CHECK(buf[0] == 0 && buf[4] == 7 && buf[5] == 0 && !memcmp(buf + 8, json, 7), "the header is little-endian: op, then length");
	int op = -1;
	const char *body;
	size_t len = 0;
	CHECK(discord_frame_take(buf, n, &op, &body, &len) == (long)n && op == DISCORD_OP_HANDSHAKE && len == 7 && !memcmp(body, json, 7),
		"a frame reads back as written");
	CHECK(discord_frame_take(buf, n - 1, &op, &body, &len) == 0, "a frame cut short waits for the rest");
	CHECK(discord_frame_take(buf, 5, &op, &body, &len) == 0, "half a header waits for the rest");
	CHECK(discord_frame_put(buf, 10, DISCORD_OP_FRAME, json, 7) == 0, "a frame refused where it does not fit");
	/* (a length past the most: no frame of Discord's, the pipe dropped) */
	uint8_t big[DISCORD_HEADER] = { 1, 0, 0, 0, 0xFF, 0xFF, 0, 0 };
	CHECK(discord_frame_take(big, sizeof big, &op, &body, &len) == -1, "a frame too long is no frame");
}

static void hello(void) {
	char out[128];
	size_t n = discord_hello_json(out, sizeof out, "123456789012345678");
	CHECK(n == strlen(out) && !strcmp(out, "{\"v\":1,\"client_id\":\"123456789012345678\"}"), "the handshake: %s", out);
	CHECK(discord_hello_json(out, 8, "123") == 0, "a handshake that does not fit is refused");
}

static void activity(void) {
	char out[DISCORD_FRAME_MAX];
	DiscordActivity a;
	memset(&a, 0, sizeof a);
	snprintf(a.details, sizeof a.details, "Layer 3" DOT "Sky \"Area\"\\");
	snprintf(a.state, sizeof a.state, "x");
	a.start = 1700000000;
	size_t n = discord_activity_json(out, sizeof out, &a, 4242, 7);
	CHECK(n == strlen(out), "the command's length is its JSON's");
	CHECK(starts(out, "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":4242,\"activity\":{"), "SET_ACTIVITY for the game's process: %s", out);
	CHECK(strstr(out, "\"details\":\"Layer 3" DOT "Sky \\\"Area\\\"\\\\\"") != NULL, "quotes and backslashes escaped, UTF-8 kept: %s", out);
	CHECK(strstr(out, "\"state\"") == NULL, "a line under two bytes left out (Discord refuses it)");
	CHECK(strstr(out, "\"timestamps\":{\"start\":1700000000}") != NULL, "the time played in Unix seconds");
	CHECK(strstr(out, "\"large_image\":\"" DISCORD_IMAGE "\"") != NULL && strstr(out, "\"large_text\"") == NULL, "the picture, no empty tooltip");
	CHECK(strstr(out, "\"buttons\":[{\"label\":\"" DISCORD_BUTTON "\",\"url\":\"" DISCORD_URL "\"}]") != NULL, "one button to the game's page");
	CHECK(strlen(DISCORD_BUTTON) <= DISCORD_LABEL, "the button's label within Discord's 32");
	CHECK(ends(out, "},\"nonce\":\"7\"}"), "the nonce last: %s", out);
	/* (a control character, as \u) */
	snprintf(a.state, sizeof a.state, "a\tb");
	discord_activity_json(out, sizeof out, &a, 1, 1);
	CHECK(strstr(out, "\"state\":\"a\\u0009b\"") != NULL, "a tab escaped: %s", out);
	n = discord_activity_json(out, sizeof out, NULL, 99, 3);
	CHECK(!strcmp(out, "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":99},\"nonce\":\"3\"}") && n == strlen(out), "no activity clears it: %s", out);
	CHECK(discord_activity_json(out, 40, &a, 1, 1) == 0, "an activity that does not fit is refused");
	DiscordActivity b = a;
	CHECK(discord_same(&a, &b), "an activity is the same as its copy");
	b.start++;
	CHECK(!discord_same(&a, &b), "another start is another activity");
}

/* What friends read, place by place */
static void words(void) {
	DiscordActivity a;
	DiscordFacts f = { .place = DISCORD_LAYER, .doing = DISCORD_WALKING, .layer = 14, .area = "Sky Area", .home = "", .foe = "",
		.best = 22, .runs = 12, .version = "0.23.0" };
	discord_words(&f, &a);
	CHECK(!strcmp(a.details, "Layer 14" DOT "Sky Area") && !strcmp(a.state, "Exploring"), "a layer: %s / %s", a.details, a.state);
	CHECK(!strcmp(a.large_text, "Best: layer 22" DOT "12 runs"), "the tooltip holds the best: %s", a.large_text);
	f.last = 10;
	f.layer = 7;
	f.threat = 3;
	f.doing = DISCORD_GUARDIAN;
	f.foe = "ElecMan";
	discord_words(&f, &a);
	CHECK(!strcmp(a.details, "Layer 7 of 10" DOT "Sky Area") && !strcmp(a.state, "Fighting ElecMan" DOT "Threat 3"),
		"the short net, a guardian, a threat: %s / %s", a.details, a.state);
	f.doing = DISCORD_BATTLE;
	f.threat = 0;
	discord_words(&f, &a);
	CHECK(!strcmp(a.state, "Busting viruses"), "a battle: %s", a.state);
	f.doing = DISCORD_DUEL;
	discord_words(&f, &a);
	CHECK(!strcmp(a.state, "Dueling ProtoMan"), "a duel: %s", a.state);
	f = (DiscordFacts){ .place = DISCORD_HOME, .layer = 4, .home = "Lan's HP", .area = "", .foe = "", .version = "0.23.0" };
	discord_words(&f, &a);
	CHECK(!strcmp(a.details, "Home" DOT "layer 4 next") && !strcmp(a.state, "Lan's HP"), "home: %s / %s", a.details, a.state);
	CHECK(!strcmp(a.large_text, "Cyberworld Endless 0.23.0"), "no run finished: the version: %s", a.large_text);
	f.doing = DISCORD_LAB;
	discord_words(&f, &a);
	CHECK(!strcmp(a.state, "Battle Lab"), "the Battle Lab: %s", a.state);
	f = (DiscordFacts){ .place = DISCORD_SUMMARY, .layer = 10, .won = true, .new_best = true, .best = 10, .runs = 1, .version = "x" };
	discord_words(&f, &a);
	CHECK(!strcmp(a.details, "Won the run on layer 10") && !strcmp(a.state, "New best!") && !strcmp(a.large_text, "Best: layer 10" DOT "1 run"),
		"a run won: %s / %s / %s", a.details, a.state, a.large_text);
	f.won = f.new_best = false;
	discord_words(&f, &a);
	CHECK(!strcmp(a.details, "Run over on layer 10") && !a.state[0], "a run lost: %s / %s", a.details, a.state);
	f.place = DISCORD_SQUARE;
	f.doing = DISCORD_NETBATTLE;
	discord_words(&f, &a);
	CHECK(!strcmp(a.details, "In the Net Square") && !strcmp(a.state, "NetBattling"), "a NetBattle: %s / %s", a.details, a.state);
	f.place = DISCORD_SETUP;
	discord_words(&f, &a);
	CHECK(!strcmp(a.details, "Setting up a run") && !a.state[0], "the setup: %s", a.details);
	f.place = DISCORD_MENUS;
	discord_words(&f, &a);
	CHECK(!strcmp(a.details, "In the menus"), "the menus: %s", a.details);
	/* (every line within Discord's 128, the longest names) */
	f = (DiscordFacts){ .place = DISCORD_LAYER, .doing = DISCORD_GUARDIAN, .layer = 99999, .last = 0, .threat = 10,
		.area = "Undernet of the Undernet", .foe = "CybeastGregar", .home = "", .best = 99999, .runs = 99999, .version = "0.99.99" };
	discord_words(&f, &a);
	CHECK(strlen(a.details) < DISCORD_TEXT && strlen(a.state) < DISCORD_TEXT && strlen(a.large_text) < DISCORD_TEXT, "the lines fit");
}

int test_discord(void) {
	failures = 0;
	frames();
	hello();
	activity();
	words();
	CHECK(discord_setting_note()[0] == '\n', "settings.ini's note opens on a blank line, apart from the line before");
	return failures;
}
