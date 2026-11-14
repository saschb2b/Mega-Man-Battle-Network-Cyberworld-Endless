/* update.h. Two threads at most, one at a time: the start's look at
 * update.json, and a press's download. Each writes what it found before
 * it sets the state, which the game's thread reads (SDL's atomics are
 * full barriers). */
#include "update.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "game.h"
#include "platform.h"
#include "settings_line.h"
#include "update_install.h"
#include "update_manifest.h"
#include "update_sha256.h"
#include "update_text.h"
#include "version.h"   /* CW_VERSION, from the build (Makefile) */

/* (the systems whose own library fetches it, analytics_http.c's: Linux
 * and the PortMaster handhelds, and Windows) */
#if (defined(__linux__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)) || defined(_WIN32)
#define UPDATE_HERE 1
#include "analytics_http.h"
#endif

/* the newest release's own, which GitHub sends on to that release's file */
#define MANIFEST_URL "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/latest/download/update.json"
#define MANIFEST_MAX (16 * 1024)

static struct {
	bool settled, looked, here, on, offered, ended;
	UpdateWay way;
	const char *key;           /* this copy's file in update.json, where it installs one */
	char url[300];             /* update.json's address (a developer's may differ) */
	char from[24];             /* this build's version */
	char agent[64];            /* the requests' User-Agent */
	UpdateRelease found;       /* the newer release, written before the state says FOUND */
	SDL_atomic_t state, percent, stop, busy;
} U = { .on = true, .url = MANIFEST_URL };

/* ---- the switch ---- */

static void settings_path(char *path, size_t n) { snprintf(path, n, "%s/settings.ini", g_data_dir); }

/* settings.ini's updates line: off, or anything else (on) */
static bool on_read(void) {
	char path[600], line[256], key[32], val[32];
	settings_path(path, sizeof path);
	FILE *f = fopen(path, "r");
	if (!f) return true;
	bool on = true;
	while (fgets(line, sizeof line, f))
		if (line[0] != '#' && sscanf(line, " %31[a-z_] = %31s", key, val) == 2 && !strcmp(key, "updates")) on = strcmp(val, "off") && strcmp(val, "no");
	fclose(f);
	return on;
}

static void settle(void);

void update_set_on(bool on) {
	settle();
	char path[600];
	U.on = on;
	settings_path(path, sizeof path);
	settings_line_write(path, "updates", on ? "on" : "off", update_setting_note());
	platform_persist();
	/* (switched off: what was found is forgotten, a download under way stops) */
	if (!on) {
		SDL_AtomicSet(&U.stop, 1);
		SDL_AtomicSet(&U.state, UPDATE_IDLE);
	}
}

/* ---- the threads ---- */

#ifdef UPDATE_HERE

/* update.json's text as it comes */
typedef struct {
	char text[MANIFEST_MAX];
	size_t n;
} Manifest;

static bool keep_text(void *user, const void *data, size_t n) {
	Manifest *m = user;
	if (m->n + n >= sizeof m->text) return false;
	memcpy(m->text + m->n, data, n);
	m->n += n;
	m->text[m->n] = 0;
	return true;
}

/* The start's look: a newer version, and this copy's file in it where it
 * installs one (a release without that file: the download page instead) */
static int look(void *arg) {
	(void)arg;
	static Manifest m;
	UpdateRelease r;
	bool answered = analytics_http_get(U.url, U.agent, keep_text, &m, &U.stop), read = answered && update_manifest_read(m.text, NULL, &r);
	bool newer = read && update_version_cmp(r.version, U.from) > 0 && !SDL_AtomicGet(&U.stop);
	if (newer) {
		if (U.key && !update_manifest_read(m.text, U.key, &U.found)) {
			U.way = UPDATE_WAY_PAGE;
			U.found = r;
		}
		if (!U.key) U.found = r;
		SDL_AtomicSet(&U.state, UPDATE_FOUND);
	}
	printf("update: %s %s %s\n", U.from, newer ? "has a newer version:" : read ? "is the newest" : answered ? "read no update.json" : "had no answer",
		newer ? U.found.version : "");
	SDL_AtomicSet(&U.busy, 0);
	return 0;
}

/* the download as it comes: written, hashed, counted */
typedef struct {
	FILE *f;
	Sha256 hash;
	long long got;
} Download;

static bool keep_file(void *user, const void *data, size_t n) {
	Download *d = user;
	if (d->got + (long long)n > U.found.size || fwrite(data, 1, n, d->f) != n) return false;
	sha256_feed(&d->hash, data, n);
	d->got += (long long)n;
	SDL_AtomicSet(&U.percent, (int)(d->got * 100 / U.found.size));
	return true;
}

/* A press's download: whole, its digest the release's, then put in place */
static int fetch(void *arg) {
	(void)arg;
	Download d = { update_install_open(), { { 0 }, 0, { 0 } }, 0 };
	sha256_begin(&d.hash);
	bool ok = d.f && analytics_http_get(U.found.url, U.agent, keep_file, &d, &U.stop);
	if (d.f && fclose(d.f) != 0) ok = false;
	char hex[65];
	sha256_end(&d.hash, hex);
	ok = ok && d.got == U.found.size && !strcmp(hex, U.found.sha256) && !SDL_AtomicGet(&U.stop) && update_install_place(U.found.version);
	if (!ok) update_install_drop();
	printf("update: %s %s\n", U.found.version, ok ? "downloaded and in place" : "not downloaded");
	if (SDL_AtomicGet(&U.state) == UPDATE_GETTING) SDL_AtomicSet(&U.state, ok ? UPDATE_READY : UPDATE_FAILED);
	SDL_AtomicSet(&U.busy, 0);
	return 0;
}

/* `work` on a thread of its own, where none is under way */
static bool thread_start(SDL_ThreadFunction work, const char *name) {
	if (!SDL_AtomicCAS(&U.busy, 0, 1)) return false;
	SDL_Thread *t = SDL_CreateThreadWithStackSize(work, name, 256 * 1024, NULL);
	if (t) SDL_DetachThread(t);
	else SDL_AtomicSet(&U.busy, 0);
	return t != NULL;
}

#endif

/* ---- the game's side ---- */

/* (as the game exits, after its own end: a download under way stops
 * within a second, else the game ends without it, the next start tidying
 * what it wrote; one done starts the new version) */
static void at_exit(void) {
	SDL_AtomicSet(&U.stop, 1);
	for (int i = 0; i < 30 && SDL_AtomicGet(&U.busy); ++i) SDL_Delay(50);
	if (SDL_AtomicGet(&U.state) != UPDATE_READY) return;
	printf("update: starting %s\n", U.found.version);
	update_install_start();
}

/* What this copy is and may do, once (settings.ini read by the first
 * caller: the title's first frame, or Options') */
static void settle(void) {
	if (U.settled) return;
	U.settled = true;
	const char *url = getenv("CYBERWORLD_UPDATE_URL"), *from = getenv("CYBERWORLD_UPDATE_FROM");
	if (url && url[0]) snprintf(U.url, sizeof U.url, "%s", url);
	snprintf(U.from, sizeof U.from, "%s", from && from[0] ? from : CW_VERSION);
	U.way = update_install_way(&U.key);
	U.on = on_read();
#ifdef UPDATE_HERE
	bool developer = strcmp(U.url, MANIFEST_URL) != 0;
	U.here = U.way != UPDATE_WAY_NONE && (developer || (!P.headless && update_version_release(U.from)));
#endif
}

/* The start's look, where this copy looks and may */
static void look_start(void) {
	if (!U.here) return;
	update_install_tidy();
	atexit(at_exit);
#ifdef UPDATE_HERE
	if (!U.on || !analytics_http_ready()) return;
	snprintf(U.agent, sizeof U.agent, "CyberworldEndless/%s", U.from);
	if (!thread_start(look, "update look")) printf("update: no thread to look with\n");
#endif
}

void update_tick(void) {
	if (!U.looked) {
		U.looked = true;
		settle();
		look_start();
	}
	if (SDL_AtomicGet(&U.state) == UPDATE_READY && !U.ended) {
		U.ended = true;
		P.quit = true;
	}
}

bool update_here(void) {
	settle();
	return U.here;
}

bool update_on(void) {
	settle();
	return U.on;
}

UpdateState update_state(void) { return U.on ? (UpdateState)SDL_AtomicGet(&U.state) : UPDATE_IDLE; }

UpdateWay update_way(void) { return U.way; }

const char *update_version(void) { return update_state() != UPDATE_IDLE ? U.found.version : ""; }

int update_percent(void) { return SDL_AtomicGet(&U.percent); }

/* (the title under Options: never a run's PET, whose run the game's end would cut short) */
static bool at_title(void) {
	const Scene *s = scene_current();
	return s && s->name && !strcmp(s->name, "title");
}

bool update_line_shown(void) { return update_state() != UPDATE_IDLE && at_title(); }

bool update_line_first(void) {
	if (U.offered || !update_line_shown()) return false;
	U.offered = true;
	return true;
}

bool update_press(void) {
	UpdateState s = update_state();
	if (!at_title() || (s != UPDATE_FOUND && s != UPDATE_FAILED)) return false;
	if (U.way == UPDATE_WAY_PAGE) return update_install_page();
#ifdef UPDATE_HERE
	if (U.way != UPDATE_WAY_INSTALL) return false;
	SDL_AtomicSet(&U.percent, 0);
	SDL_AtomicSet(&U.stop, 0);
	SDL_AtomicSet(&U.state, UPDATE_GETTING);
	if (thread_start(fetch, "update download")) return true;
	SDL_AtomicSet(&U.state, UPDATE_FAILED);
#endif
	return false;
}
