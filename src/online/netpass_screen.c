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
	int end;             /* ... the frame the full pass shows */
	NetPassView view;
} N;

/* the most stamps seen in the Square, from the data folder's netpass.ini (0 none) */
static int seen_read(void) {
	char path[600], line[64];
	snprintf(path, sizeof path, "%s/netpass.ini", g_data_dir);
	FILE *f = fopen(path, "r");
	int seen = 0;
	while (f && fgets(line, sizeof line, f))
		if (sscanf(line, "stamps=%d", &seen) == 1) break;
	if (f) fclose(f);
	return seen < 0 ? 0 : seen > STAMP_COUNT ? STAMP_COUNT : seen;
}

static void seen_write(int seen) {
	char path[600], tmp[610];
	snprintf(path, sizeof path, "%s/netpass.ini", g_data_dir);
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	FILE *f = fopen(tmp, "w");
	if (!f) return;
	fprintf(f, "# The NetPass's stamps the greeter has stamped in the Net Square (docs/ONLINE.md, The NetPass).\nstamps=%d\n", seen);
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

bool netpass_stamp_due(void) { return netpass_full() && seen_read() < STAMP_COUNT; }

void netpass_stamp_begin(void) {
	int seen = seen_read();
	netpass_screen_open();
	N.stamping = true;
	N.from = seen < 1 ? 1 : seen;
	N.end = FIRST_LANDS + (STAMP_COUNT - N.from - 1) * LANDS_EVERY + FULL_AFTER;
	for (int s = N.from; s < STAMP_COUNT; ++s) N.view.held[s] = false;
	N.view.full = false;
	seen_write(STAMP_COUNT);
	printf("netpass: the stamp moment, %d stamps seen before\n", seen);
}

/* the stamping's frame: a new stamp landing every so often, BN6's jingle with each, then the full pass */
static void stamping(void) {
	for (int s = N.from; s < STAMP_COUNT; ++s) {
		int at = FIRST_LANDS + (s - N.from) * LANDS_EVERY;
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
}

void netpass_screen_strip(void) {
	NetPassView v;
	netpass_view_now(&v);
	netpass_draw_strip(P.core_x, P.core_y, &v);
}
