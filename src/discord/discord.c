/* discord.h: the setting, the connection's course and the pace. The
 * course: away (no Discord found; looked for again after
 * DISCORD_RETRY_MS), hello (the handshake sent, its READY awaited),
 * ready (activities sent as they change, one in DISCORD_GAP_MS at most);
 * a pipe that breaks goes back to away. Refused (Discord does not know
 * the application) stays so for the session. */
#include "discord.h"

#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "discord_ipc.h"
#include "discord_text.h"
#include "game.h"
#include "platform.h"
#include "settings_line.h"

enum { STAGE_OFF, STAGE_AWAY, STAGE_HELLO, STAGE_READY, STAGE_REFUSED };

static struct {
	bool here, on;
	char app[32];
	int stage;
	uint32_t next_try;         /* (ticks) when Discord is looked for again */
	uint32_t hello_at;         /* ... the handshake was sent */
	uint32_t sent_at;          /* ... the last activity was */
	bool wanted, sent;         /* an activity asked for; one shown */
	DiscordActivity want, shown;
	unsigned nonce;
	uint8_t in[DISCORD_HEADER + DISCORD_FRAME_MAX];
	size_t in_len;
} S = { .on = true };

static void settings_file(char *path, size_t n) { snprintf(path, n, "%s/settings.ini", g_data_dir); }

/* (SDL's ticks wrap after 49 days: differences alone are compared) */
static bool due(uint32_t now, uint32_t at) { return (int32_t)(now - at) >= 0; }

void discord_start(bool plain) {
	char path[600], v[32];
	settings_file(path, sizeof path);
	snprintf(S.app, sizeof S.app, "%s", DISCORD_APP_ID);
	if (settings_line_read(path, "discord_app", v, sizeof v) && v[0]) snprintf(S.app, sizeof S.app, "%s", v);
	if (settings_line_read(path, "discord", v, sizeof v)) S.on = strcmp(v, "off") && strcmp(v, "no");
#ifdef DISCORD_IPC
	S.here = plain && S.app[0];
#else
	(void)plain;
	S.here = false;
#endif
	S.stage = S.here && S.on ? STAGE_AWAY : STAGE_OFF;
	S.next_try = SDL_GetTicks();
}

bool discord_here(void) { return S.here; }

bool discord_on(void) { return S.on; }

/* ---- the pipe ---- */

static bool send_json(int op, const char *json, size_t len) {
	uint8_t frame[DISCORD_HEADER + DISCORD_FRAME_MAX];
	size_t n = discord_frame_put(frame, sizeof frame, op, json, len);
	return n && discord_ipc_send(frame, n);
}

/* The pipe let go: looked for again after a while, or never (refused) */
static void drop(int stage) {
	discord_ipc_close();
	S.stage = stage;
	S.in_len = 0;
	S.sent = false;
	S.next_try = SDL_GetTicks() + DISCORD_RETRY_MS;
}

static void hello(void) {
	char json[128];
	S.next_try = SDL_GetTicks() + DISCORD_RETRY_MS;
	if (!discord_ipc_open()) return;
	size_t n = discord_hello_json(json, sizeof json, S.app);
	if (!send_json(DISCORD_OP_HANDSHAKE, json, n)) {
		drop(STAGE_AWAY);
		return;
	}
	S.stage = STAGE_HELLO;
	S.hello_at = SDL_GetTicks();
}

/* A frame from Discord: its READY, an error, its CLOSE, a PING */
static void heard(int op, const char *json, size_t len) {
	char text[DISCORD_FRAME_MAX + 1];
	memcpy(text, json, len);
	text[len] = 0;
	if (op == DISCORD_OP_PING) {
		if (!send_json(DISCORD_OP_PONG, json, len)) drop(STAGE_AWAY);
	} else if (op == DISCORD_OP_CLOSE) {
		/* (4000: an application Discord does not know; looking again
		 * would only be refused again) */
		fprintf(stderr, "discord: closed: %.200s\n", text);
		drop(strstr(text, "\"code\":4000") ? STAGE_REFUSED : STAGE_AWAY);
	} else if (strstr(text, "\"evt\":\"READY\"")) {
		if (S.stage == STAGE_HELLO) fprintf(stderr, "discord: ready\n");
		S.stage = STAGE_READY;
	} else if (strstr(text, "\"evt\":\"ERROR\"")) {
		fprintf(stderr, "discord: %.200s\n", text);
	}
}

/* What Discord has sent, each whole frame heard */
static void read_pipe(void) {
	for (;;) {
		long k = discord_ipc_recv(S.in + S.in_len, sizeof S.in - S.in_len);
		if (k < 0) {
			drop(STAGE_AWAY);
			return;
		}
		S.in_len += (size_t)k;
		int op;
		const char *json;
		size_t len;
		long whole = 0;
		while (S.stage != STAGE_AWAY && (whole = discord_frame_take(S.in, S.in_len, &op, &json, &len)) > 0) {
			heard(op, json, len);
			memmove(S.in, S.in + whole, S.in_len - (size_t)whole);
			S.in_len -= (size_t)whole;
		}
		if (whole < 0) drop(STAGE_AWAY);
		if (S.stage == STAGE_AWAY || S.stage == STAGE_REFUSED || k == 0) return;
	}
}

/* The activity asked for, where it differs from the one shown and the
 * pace allows (`a` NULL clears it) */
static void send_activity(const DiscordActivity *a) {
	char json[DISCORD_FRAME_MAX];
	size_t n = discord_activity_json(json, sizeof json, a, discord_ipc_pid(), ++S.nonce);
	if (!send_json(DISCORD_OP_FRAME, json, n)) {
		drop(STAGE_AWAY);
		return;
	}
	S.sent_at = SDL_GetTicks();
	S.sent = a != NULL;
	if (a) S.shown = *a;
}

/* ---- the session ---- */

void discord_show(const DiscordActivity *a) {
	S.want = *a;
	S.wanted = true;
}

void discord_frame(void) {
	if (S.stage == STAGE_OFF || S.stage == STAGE_REFUSED) return;
	uint32_t now = SDL_GetTicks();
	if (S.stage == STAGE_AWAY) {
		if (due(now, S.next_try)) hello();
		return;
	}
	read_pipe();
	if (S.stage == STAGE_HELLO && due(now, S.hello_at + DISCORD_HELLO_MS)) drop(STAGE_AWAY);
	if (S.stage != STAGE_READY || !S.wanted) return;
	if (S.sent && (discord_same(&S.want, &S.shown) || !due(now, S.sent_at + DISCORD_GAP_MS))) return;
	send_activity(&S.want);
}

void discord_set_on(bool on) {
	char path[600];
	if (on == S.on) return;
	S.on = on;
	settings_file(path, sizeof path);
	settings_line_write(path, "discord", on ? "on" : "off", discord_setting_note());
	platform_persist();
	if (!on) {
		discord_quit();
		S.stage = STAGE_OFF;
	} else if (S.here) {
		S.stage = STAGE_AWAY;
		S.next_try = SDL_GetTicks();
	}
}

void discord_quit(void) {
	/* (Discord clears a closed pipe's activity itself; cleared first, a
	 * friend's list says so at once) */
	if (S.stage == STAGE_READY && S.sent) send_activity(NULL);
	discord_ipc_close();
	S.stage = STAGE_OFF;
	S.sent = false;
	S.in_len = 0;
}
