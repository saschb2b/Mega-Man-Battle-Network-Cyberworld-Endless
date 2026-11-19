/* discord.h's wire: the local pipe's frames (an opcode and a length, each
 * four bytes little-endian, then that many bytes of JSON) and the JSON of
 * the two commands the game sends, as Discord's RPC takes them
 * (docs/engine/discord.md, The wire). Numbers and text only, ROM-free. */
#include "discord.h"

#include <stdio.h>
#include <string.h>

/* ---- frames ---- */

static void put32(uint8_t *p, uint32_t v) {
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
}

static uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }

size_t discord_frame_put(uint8_t *out, size_t n, int op, const char *json, size_t len) {
	if (len > DISCORD_FRAME_MAX || n < DISCORD_HEADER + len) return 0;
	put32(out, (uint32_t)op);
	put32(out + 4, (uint32_t)len);
	memcpy(out + DISCORD_HEADER, json, len);
	return DISCORD_HEADER + len;
}

long discord_frame_take(const uint8_t *in, size_t len, int *op, const char **json, size_t *json_len) {
	if (len < DISCORD_HEADER) return 0;
	uint32_t body = get32(in + 4);
	/* (Discord's own frames stay far under it: a READY with the user's
	 * record is under 2 KB) */
	if (body > DISCORD_FRAME_MAX) return -1;
	if (len < DISCORD_HEADER + body) return 0;
	*op = (int)get32(in);
	*json = (const char *)in + DISCORD_HEADER;
	*json_len = body;
	return (long)(DISCORD_HEADER + body);
}

/* ---- JSON ---- */

/* A growing string: its bytes, its room and length; `full` once
 * something did not fit, which nothing after mends */
typedef struct {
	char *s;
	size_t n, len;
	bool full;
} Json;

static void raw(Json *j, const char *t) {
	size_t k = strlen(t);
	if (j->full || j->len + k >= j->n) {
		j->full = true;
		return;
	}
	memcpy(j->s + j->len, t, k + 1);
	j->len += k;
}

/* `t` as a JSON string, quoted: the quote, the backslash and the control
 * characters escaped, the rest (UTF-8) as it is */
static void str(Json *j, const char *t) {
	raw(j, "\"");
	for (const unsigned char *c = (const unsigned char *)t; *c; ++c) {
		char e[8];
		if (*c == '"' || *c == '\\') snprintf(e, sizeof e, "\\%c", *c);
		else if (*c < 0x20) snprintf(e, sizeof e, "\\u%04x", (unsigned)*c);
		else {
			e[0] = (char)*c;
			e[1] = 0;
		}
		raw(j, e);
	}
	raw(j, "\"");
}

/* `"key":"value"`, after a comma where `comma` */
static void field(Json *j, const char *key, const char *value, bool comma) {
	if (comma) raw(j, ",");
	str(j, key);
	raw(j, ":");
	str(j, value);
}

/* Discord refuses a line under two bytes: an empty one is left out, a
 * one-letter one too */
static bool shown(const char *t) { return t[0] && t[1]; }

static size_t done(Json *j) { return j->full ? 0 : j->len; }

size_t discord_hello_json(char *out, size_t n, const char *app) {
	Json j = { out, n, 0, false };
	if (n) out[0] = 0;
	raw(&j, "{\"v\":1,");
	field(&j, "client_id", app, false);
	raw(&j, "}");
	return done(&j);
}

/* the activity's own object: its lines, the time, the picture, the button */
static void activity(Json *j, const DiscordActivity *a) {
	char num[64];
	raw(j, "{");
	bool comma = false;
	if (shown(a->details)) {
		field(j, "details", a->details, comma);
		comma = true;
	}
	if (shown(a->state)) {
		field(j, "state", a->state, comma);
		comma = true;
	}
	if (a->start > 0) {
		snprintf(num, sizeof num, "%s\"timestamps\":{\"start\":%lld}", comma ? "," : "", (long long)a->start);
		raw(j, num);
		comma = true;
	}
	raw(j, comma ? ",\"assets\":{" : "\"assets\":{");
	field(j, "large_image", DISCORD_IMAGE, false);
	if (shown(a->large_text)) field(j, "large_text", a->large_text, true);
	raw(j, "},\"buttons\":[{");
	field(j, "label", DISCORD_BUTTON, false);
	field(j, "url", DISCORD_URL, true);
	raw(j, "}],\"instance\":false}");
}

size_t discord_activity_json(char *out, size_t n, const DiscordActivity *a, long pid, unsigned nonce) {
	char num[80];
	Json j = { out, n, 0, false };
	if (n) out[0] = 0;
	snprintf(num, sizeof num, "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%ld", pid);
	raw(&j, num);
	/* (no activity: Discord clears what it showed) */
	if (a) {
		raw(&j, ",\"activity\":");
		activity(&j, a);
	}
	snprintf(num, sizeof num, "},\"nonce\":\"%u\"}", nonce);
	raw(&j, num);
	return done(&j);
}

bool discord_same(const DiscordActivity *a, const DiscordActivity *b) {
	return a->start == b->start && !strcmp(a->details, b->details) && !strcmp(a->state, b->state) && !strcmp(a->large_text, b->large_text);
}
