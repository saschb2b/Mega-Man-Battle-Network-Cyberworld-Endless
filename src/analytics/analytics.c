/* analytics.h. The consent, the events and their payloads, and the queue
 * the systems send from; no system of its own, so the unit tests read it
 * all (tests/test_core.c). A payload is Umami's tracker's own:
 *
 *   {"type":"event","payload":{"website":"...","hostname":"...",
 *    "url":"/linux","title":"Cyberworld Endless 0.10.0","screen":"1280x960",
 *    "name":"run-end","data":{"won":false,...}}}
 *
 * a view (the game's start) without name and data. */
#include "analytics.h"

#include <stdio.h>
#include <string.h>

#include "analytics_text.h"
#include "compat.h"
#include "guardians.h"
#include "meta.h"
#include "net.h"
#include "powers.h"
#include "rivals.h"
#include "rom.h"
#include "run.h"
#include "save.h"

static struct {
	AnalyticsStart s;
	bool begun;
	bool system;          /* the system can send (s.ready) */
	int consent;
	bool viewed;          /* the start's view sent this session */
	char path[32];        /* "/linux" */
	char title[64];       /* "Cyberworld Endless 0.10.0" */
	char screen[24];      /* "1280x960", "" unknown */
	/* (--dev: a test's answer, ANALYTICS_UNASKED for ask, -1 none;
	 * statsurl's address; statscheck's one event) */
	int forced;
	char url[256];
	bool check;
} A = { .forced = -1 };

/* ---- the answer ---- */

/* settings.ini's statistics line: on, off, or none (not asked) */
static int answer_read(const char *path) {
	FILE *f = path ? fopen(path, "r") : NULL;
	if (!f) return ANALYTICS_UNASKED;
	char line[256], key[32], val[32];
	int got = ANALYTICS_UNASKED;
	while (fgets(line, sizeof line, f)) {
		if (line[0] == '#' || sscanf(line, " %31[a-z_] = %31s", key, val) != 2 || strcmp(key, "statistics")) continue;
		got = !strcmp(val, "on") || !strcmp(val, "yes") ? ANALYTICS_ON : !strcmp(val, "off") || !strcmp(val, "no") ? ANALYTICS_OFF : ANALYTICS_UNASKED;
	}
	fclose(f);
	return got;
}

/* ... written: the file as it was, its statistics line replaced, or the
 * line and what it is added at its end */
static bool answer_write(const char *path, bool on) {
	char tmp[620], line[512], key[32];
	if (!path) return false;
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	FILE *in = fopen(path, "r"), *out = fopen(tmp, "w");
	if (!out) {
		if (in) fclose(in);
		return false;
	}
	bool kept = false;
	while (in && fgets(line, sizeof line, in)) {
		bool ours = line[0] != '#' && sscanf(line, " %31[a-z_]", key) == 1 && !strcmp(key, "statistics");
		if (!ours) fputs(line, out);
		else if (!kept) fprintf(out, "statistics = %s\n", on ? "on" : "off");
		kept |= ours;
	}
	/* (what it is, after a line apart from the lines before it, if any) */
	const char *note = analytics_setting_note();
	if (!in && *note == '\n') ++note;
	if (in) fclose(in);
	if (!kept) fprintf(out, "%sstatistics = %s\n", note, on ? "on" : "off");
	bool ok = fclose(out) == 0 && cw_rename(tmp, path);
	if (A.s.persist) A.s.persist();
	return ok;
}

/* ---- the session ---- */

bool analytics_dev(const char *option) {
	if (!strncmp(option, "statistics=", 11)) {
		const char *v = option + 11;
		A.forced = !strcmp(v, "on") ? ANALYTICS_ON : !strcmp(v, "off") ? ANALYTICS_OFF : ANALYTICS_UNASKED;
		return true;
	}
	if (!strncmp(option, "statsurl=", 9)) {
		snprintf(A.url, sizeof A.url, "%s", option + 9);
		return true;
	}
	if (!strcmp(option, "statscheck")) {
		A.check = true;
		return true;
	}
	return false;
}

const char *analytics_url(void) { return A.url[0] ? A.url : GAME_UMAMI_SEND; }

/* (a build whose website is left empty never asks and never sends) */
static bool website_set(void) { return GAME_UMAMI_WEBSITE[0] != '\0'; }

/* Whether the session may ask and keep an answer: the build can send
 * (its website, a system that sends), and a player started it, or a test
 * switched it on */
bool analytics_here(void) {
	return A.begun && A.system && A.s.post && website_set() && (A.s.plain || A.forced >= 0 || A.url[0]);
}

/* ... and may send: a player's start, or a test's to its own address
 * (a test that asks, a capture, sends nothing) */
static bool sends(void) { return analytics_here() && A.consent == ANALYTICS_ON && (A.s.plain || A.url[0]); }

int analytics_consent(void) { return A.consent; }

bool analytics_unasked(void) { return analytics_here() && A.consent == ANALYTICS_UNASKED; }

static void view(void);
static void check(void);

void analytics_begin(const AnalyticsStart *s) {
	A.s = *s;
	A.begun = true;
	A.viewed = false;
	/* (the system's library looked for only where the session may send) */
	A.system = (s->plain || A.forced >= 0 || A.url[0] || A.check) && s->ready && s->ready();
	A.consent = A.forced >= 0 ? A.forced : answer_read(s->settings);
	snprintf(A.path, sizeof A.path, "/%s", s->platform ? s->platform : "unknown");
	snprintf(A.title, sizeof A.title, "Cyberworld Endless %s", s->version ? s->version : "dev");
	if (s->screen_w > 0 && s->screen_h > 0) snprintf(A.screen, sizeof A.screen, "%dx%d", s->screen_w, s->screen_h);
	else A.screen[0] = 0;
	view();
	if (A.check) check();
}

void analytics_answer(bool yes) {
	if (!analytics_here()) return;
	A.consent = yes ? ANALYTICS_ON : ANALYTICS_OFF;
	answer_write(A.s.settings, yes);
	if (yes) view();
	else if (A.s.drop) A.s.drop();
}

/* ---- the payloads ---- */

/* JSON written into a buffer: members after the first get their comma;
 * what does not fit makes the whole payload unsent */
typedef struct {
	char *s;
	size_t n, len;
	bool first, full;
} Json;

static void json_putn(Json *j, const char *t, size_t k) {
	if (j->full || j->len + k >= j->n) { j->full = true; return; }
	memcpy(j->s + j->len, t, k);
	j->len += k;
	j->s[j->len] = 0;
}

static void json_put(Json *j, const char *t) { json_putn(j, t, strlen(t)); }

/* `v` quoted: its quotes and backslashes escaped, a control character as
 * \u00XX */
static void json_string(Json *j, const char *v) {
	static const char hex[] = "0123456789abcdef";
	json_put(j, "\"");
	for (const unsigned char *c = (const unsigned char *)v; *c; ++c) {
		if (*c == '"') json_put(j, "\\\"");
		else if (*c == '\\') json_put(j, "\\\\");
		else if (*c < 0x20) {
			const char e[6] = { '\\', 'u', '0', '0', hex[*c >> 4], hex[*c & 15] };
			json_putn(j, e, sizeof e);
		} else json_putn(j, (const char *)c, 1);
	}
	json_put(j, "\"");
}

static void json_key(Json *j, const char *k) {
	if (!j->first) json_put(j, ",");
	j->first = false;
	json_string(j, k);
	json_put(j, ":");
}

static void json_member(Json *j, const char *k, const char *v) { json_key(j, k); json_string(j, v); }

static void json_int(Json *j, const char *k, int v) {
	char b[16] = "";
	snprintf(b, sizeof b, "%d", v);
	json_key(j, k);
	json_put(j, b);
}

static void json_bool(Json *j, const char *k, bool v) { json_key(j, k); json_put(j, v ? "true" : "false"); }

static void json_open(Json *j, const char *k) {
	if (k) json_key(j, k);
	json_put(j, "{");
	j->first = true;
}

static void json_close(Json *j) {
	json_put(j, "}");
	j->first = false;
}

/* The payload's head: what every event says (its address's path, its name,
 * NULL for a view), and the data's object opened where it has a name */
static void payload_begin(Json *j, char *out, size_t n, const char *path, const char *name) {
	*j = (Json){ out, n, 0, true, false };
	out[0] = 0;
	json_open(j, NULL);
	json_member(j, "type", "event");
	json_open(j, "payload");
	json_member(j, "website", GAME_UMAMI_WEBSITE);
	json_member(j, "hostname", GAME_UMAMI_HOSTNAME);
	json_member(j, "url", path);
	json_member(j, "title", A.title);
	if (A.screen[0]) json_member(j, "screen", A.screen);
	if (!name) return;
	json_member(j, "name", name);
	json_open(j, "data");
}

/* ... closed, and handed to the system where the session sends (or,
 * `always`, the developer's check) */
static void payload_send(Json *j, bool named, bool always) {
	if (named) json_close(j);
	json_close(j);
	json_close(j);
	if (!j->full && (always || sends())) A.s.post(j->s);
}

/* The game's start, once a session, once the answer is yes */
static void view(void) {
	if (A.viewed || !sends()) return;
	char out[ANALYTICS_PAYLOAD];
	Json j;
	payload_begin(&j, out, sizeof out, A.path, NULL);
	payload_send(&j, false, false);
	A.viewed = true;
}

/* --dev statscheck: one event, named as a test's at a path of its own,
 * whatever the answer and nothing else: a system's way to Umami checked
 * (the threaded systems print how it went) */
static void check(void) {
	if (!A.system || !A.s.post || !website_set()) return;
	char out[ANALYTICS_PAYLOAD];
	Json j;
	payload_begin(&j, out, sizeof out, "/dev-check", "game-dev-check");
	json_member(&j, "platform", A.path + 1);
	payload_send(&j, true, true);
}

bool analytics_checking(void) { return A.check; }

/* ---- the events ---- */

static const char *net_name(void) { return run.mode == RUN_SHORT ? "short" : "endless"; }

/* The helpers switched on, by the setup's names ("HP+, Heals"), "none" */
static void helpers_words(char *s, size_t n) {
	s[0] = 0;
	for (int h = 0; h < HELPERS; ++h) {
		size_t m = strlen(s);
		if (run.helpers >> h & 1) snprintf(s + m, n - m, "%s%s", m ? ", " : "", meta_helper(h)->name);
	}
	if (!s[0]) snprintf(s, n, "none");
}

/* What the player brought, as the setup's rows name it */
static void setup_json(Json *j) {
	char helpers[64];
	const char *cross = run.cross ? powers_cross_name(run.cross) : NULL;
	helpers_words(helpers, sizeof helpers);
	json_member(j, "net", net_name());
	json_member(j, "folder", meta_folder(run.folder < FOLDER_COUNT ? run.folder : FOLDER_STANDARD)->name);
	json_member(j, "cross", cross ? cross : "none");
	json_int(j, "threat", run.threat);
	json_member(j, "helpers", helpers);
}

void analytics_run_start(void) {
	if (!sends()) return;
	char out[ANALYTICS_PAYLOAD];
	Json j;
	payload_begin(&j, out, sizeof out, A.path, "run-start");
	setup_json(&j);
	json_bool(&j, "bn5", XR[XROM_BN5_COLONEL_US].data != NULL);
	json_int(&j, "runs", profile.runs);
	payload_send(&j, true, false);
}

void analytics_guardian(int navi, int result, int hp, int max_hp, int frames) {
	if (!sends()) return;
	char out[ANALYTICS_PAYLOAD];
	Json j;
	payload_begin(&j, out, sizeof out, A.path, "guardian");
	json_member(&j, "guardian", guardian(navi)->name);
	json_member(&j, "result", result == ANALYTICS_WON ? "won" : result == ANALYTICS_LOST ? "lost" : "left");
	json_int(&j, "layer", run_reached());
	json_member(&j, "net", net_name());
	json_int(&j, "threat", run.threat);
	if (result != ANALYTICS_LOST && max_hp > 0) json_int(&j, "hp", (hp < 0 ? 0 : hp > max_hp ? max_hp : hp) * 100 / max_hp);
	if (frames > 0) json_int(&j, "seconds", (frames + 30) / 60);
	/* (met as he stepped up: this meeting counted) */
	json_int(&j, "attempt", rival(navi)->met > 0 ? rival(navi)->met : 1);
	payload_send(&j, true, false);
}

/* The area MegaMan stands in, by its title card's name */
static const char *area_name(void) {
	int b = run.side_kind == LAYER_UNDERNET ? BIOME_UNDERNET : run.side_kind == LAYER_SECRET ? BIOME_SECRET : run.biome;
	return guardian_area_name(b);
}

void analytics_run_end(bool won, const char *by) {
	if (!sends()) return;
	char out[ANALYTICS_PAYLOAD];
	Json j;
	payload_begin(&j, out, sizeof out, A.path, "run-end");
	json_bool(&j, "won", won);
	json_int(&j, "layer", run_reached());
	json_member(&j, "area", area_name());
	if (!won) json_member(&j, "by", by && *by ? by : "viruses");
	/* (the run's own frames, every session's, where the profile kept them:
	 * profile_played_frame) */
	if (profile.played_run == run.seed && profile.played_frames) json_int(&j, "minutes", (int)((profile.played_frames + 1800) / 3600));
	setup_json(&j);
	payload_send(&j, true, false);
}

/* ---- the systems' queue (under their lock) ---- */

bool analytics_queue_put(AnalyticsQueue *q, const char *json) {
	size_t n = strlen(json);
	if (q->count >= ANALYTICS_QUEUE || n >= ANALYTICS_PAYLOAD) return false;
	memcpy(q->body[(q->head + q->count) % ANALYTICS_QUEUE], json, n + 1);
	++q->count;
	return true;
}

bool analytics_queue_take(AnalyticsQueue *q, char *out, size_t n) {
	if (!q->count) return false;
	snprintf(out, n, "%s", q->body[q->head]);
	q->head = (q->head + 1) % ANALYTICS_QUEUE;
	--q->count;
	return true;
}

void analytics_queue_clear(AnalyticsQueue *q) { q->head = q->count = 0; }

bool analytics_tried(AnalyticsTries *t, bool ok) {
	if (t->gave_up) return false;
	t->failures = ok ? 0 : t->failures + 1;
	if (t->failures >= ANALYTICS_TRIES) t->gave_up = true;
	return !t->gave_up;
}
