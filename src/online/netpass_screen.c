/* netpass_screen.h. */
#include "netpass_screen.h"

#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "buttons.h"
#include "compat.h"
#include "game.h"
#include "netpass.h"
#include "netpass_draw.h"
#include "netpass_text.h"
#include "pet_look.h"
#include "platform.h"

#define FIRST_LANDS 24   /* frames before the first new stamp lands */
#define LANDS_EVERY 34   /* ... and between the next */
#define FULL_AFTER  26   /* the full pass after the last */
#define HELD_AT_END 20   /* ... and A or B taken from then */

static struct {
	bool shown, stamping;
	bool letting_go;     /* closed, its A or B still held: kept from the game (BN6 would take it as a new press) */
	int t;
	int from;            /* the stamping: the stamps already seen (the Square's at least) */
	int lands[STAMP_COUNT], n;   /* ... those that land, in their order: the pass's own past the seen */
	int end;             /* ... the frame the full pass shows */
	NetPassView view;
} N;

#define SET_BEFORE_BN4 2   /* the set's games a netpass.ini without `games=` was written for: BN6's and BN5's (protocol v16) */

/* what the Square has seen of the pass, from the data folder's netpass.ini:
 * the most stamps stamped (0 none), and how many games the set held then */
typedef struct { int stamps, games; } Seen;

static Seen seen_read(void) {
	char path[600], line[64];
	snprintf(path, sizeof path, "%s/netpass.ini", g_data_dir);
	FILE *f = fopen(path, "r");
	Seen seen = { 0, 0 };
	int games = -1;
	while (f && fgets(line, sizeof line, f)) {
		(void)sscanf(line, "stamps=%d", &seen.stamps);
		(void)sscanf(line, "games=%d", &games);
	}
	if (f) fclose(f);
	seen.stamps = seen.stamps < 0 ? 0 : seen.stamps > STAMP_COUNT ? STAMP_COUNT : seen.stamps;
	seen.games = games >= 0 ? games : seen.stamps ? SET_BEFORE_BN4 : 0;
	return seen;
}

/* (written with the set's games as they are now) */
static void seen_write(int seen) {
	char path[600], tmp[610];
	snprintf(path, sizeof path, "%s/netpass.ini", g_data_dir);
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	FILE *f = fopen(tmp, "w");
	if (!f) return;
	fprintf(f, "# The NetPass's stamps the greeter has stamped in the Net Square, and the set's games then (docs/online/netpass.md).\n"
		"stamps=%d\ngames=%d\n", seen, netpass_games());
	bool ok = !ferror(f) && cw_fsync_file(f);
	if (fclose(f)) ok = false;
	if (ok) ok = cw_rename(tmp, path);
	if (!ok) remove(tmp);
	platform_persist();
}

void netpass_screen_open(void) {
	memset(&N, 0, sizeof N);
	N.shown = true;
	netpass_view_now(&N.view);
	audio_sfx(SFX_DESC_OPEN);
}

bool netpass_screen_shown(void) { return N.shown; }

/* (a set grown since, a game more in it: the stamps it took away land again) */
static bool grown(Seen seen) { return seen.games < netpass_games(); }

/* (a full pass holds every stamp, or, on a system of one extra ROM, the Square's and Trade's: netpass_stamps) */
bool netpass_stamp_due(void) {
	Seen seen = seen_read();
	int held = netpass_stamps();
	return netpass_full() && (seen.stamps < held || (grown(seen) && seen.stamps == held));
}

/* (stamps taken by the set's growth: all but the Square's from a game whose pass is no longer full, the Arena's and the
 * DeepNet's from a full pass on a system of one extra ROM) */
bool netpass_grew_due(void) {
	Seen seen = seen_read();
	return seen.stamps == STAMP_COUNT && seen.stamps > netpass_stamps() && grown(seen);
}

void netpass_grew_begin(void) {
	netpass_screen_open();
	seen_write(netpass_stamps());
	printf("netpass: the set grew to %d games, its stamps gone till the pass is full\n", netpass_games());
}

void netpass_stamp_begin(void) {
	Seen was = seen_read();
	int seen = grown(was) ? 1 : was.stamps;
	netpass_screen_open();
	N.stamping = true;
	N.from = seen < 1 ? 1 : seen;
	/* (the pass's stamps past those seen land: every one, or Trade's alone on a system of one extra ROM) */
	for (int s = 0, k = 0; s < STAMP_COUNT; ++s) {
		if (!N.view.held[s] || k++ < N.from) continue;
		N.lands[N.n++] = s;
		N.view.held[s] = false;
	}
	N.end = FIRST_LANDS + (N.n - 1) * LANDS_EVERY + FULL_AFTER;
	N.view.full = false;
	seen_write(netpass_stamps());
	printf("netpass: the stamp moment, %d stamps seen before\n", seen);
}

/* the stamping's frame: a new stamp landing every so often, BN6's jingle with each, then the full pass */
static void stamping(void) {
	for (int i = 0; i < N.n; ++i) {
		int s = N.lands[i], at = FIRST_LANDS + i * LANDS_EVERY;
		if (N.t == at) {
			N.view.held[s] = true;
			N.view.landing = s;
			audio_sfx(SFX_ITEM);
		}
		if (N.view.landing == s) N.view.landing_t = N.t - at;
	}
	if (N.t == N.end) {
		N.view.full = true;
		N.view.landing = -1;
		audio_sfx(SFX_GOT);
	}
}

bool netpass_screen_update(void) {
	if (!N.shown) {
		if (N.letting_go && btn_held(BTN_A | BTN_B)) return true;
		N.letting_go = false;
		return false;
	}
	++N.t;
	N.view.t = N.t;
	if (N.stamping && N.t <= N.end) stamping();
	/* (A or B while the stamps land: all of them at once, the full pass's sound, the time the player's) */
	if (N.stamping && N.t > 1 && N.t < N.end && (btn_pressed(BTN_A) || btn_pressed(BTN_B))) {
		for (int i = 0; i < N.n; ++i) N.view.held[N.lands[i]] = true;
		N.t = N.end;
		stamping();
	}
	bool done = N.t > 1 && (!N.stamping || N.t >= N.end + HELD_AT_END);
	if (done && (btn_pressed(BTN_A) || btn_pressed(BTN_B))) {
		N.shown = false;
		N.letting_go = true;
		audio_sfx(SFX_DESC_CLOSE);
	}
	return true;
}

void netpass_screen_draw(void) {
	if (!N.shown) return;
	netpass_draw(P.core_x, P.core_y, &N.view);
	look_fade_in(P.core_x, P.core_y, NETPASS_W, NETPASS_H, N.view.t);
}

void netpass_screen_key_hint(void) {
	look_hint(P.core_x + NETPASS_W - 20 - look_hint_width("R", netpass_key_hint), P.core_y + 4, "R", netpass_key_hint);
}

void netpass_screen_strip(void) {
	NetPassView v;
	netpass_view_now(&v);
	netpass_draw_strip(P.core_x, P.core_y, &v);
}
