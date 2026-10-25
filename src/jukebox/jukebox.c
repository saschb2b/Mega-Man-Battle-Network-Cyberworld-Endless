/* jukebox.h. */
#include "jukebox.h"

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "buttons.h"
#include "compat.h"
#include "game.h"
#include "jukebox_draw.h"
#include "jukebox_text.h"
#include "pet_look.h"
#include "platform.h"
#include "rom.h"

#define SECTIONS_MOST 12
#define ROUNDS 2        /* a looping song's times round before the next, in order or shuffled */
#define FADE_SPEED 10   /* ... then its fade: 16 steps of 10 frames */
#define GAP 45          /* frames of quiet between two songs */
#define SILENT 180      /* a song that has sounded and is then silent this long has ended: BN6's
                         * jingles hold a minute of rests before their FINE */

static struct {
	bool shown, letting_go;
	struct { uint8_t game, kind; } sec[SECTIONS_MOST];
	int nsec, section;
	int at, top;
	int playing;         /* jukebox_songs' index, -1 none */
	int mode;
	int frames, quiet;   /* frames it has played; of quiet after it ended */
	int silent;          /* frames none of its tracks sounded */
	bool fading;
	uint32_t rng;
	JukeView v;
} J = { .playing = -1 };

static bool game_there(int game) {
	if (game == 0) return R.data != NULL;
	return game - 1 < XROM_COUNT && XR[game - 1].data && XR[game - 1].layout;
}

/* the sections of the games there, each with a song, by game then kind */
static void sections_find(void) {
	J.nsec = 0;
	for (int g = 0; g < 2; ++g) {
		if (!game_there(g)) continue;
		for (int k = 0; k < JUKE_KINDS && J.nsec < SECTIONS_MOST; ++k)
			for (int i = 0; i < jukebox_song_count; ++i)
				if (jukebox_songs[i].game == g && jukebox_songs[i].kind == k) {
					J.sec[J.nsec].game = (uint8_t)g;
					J.sec[J.nsec++].kind = (uint8_t)k;
					break;
				}
	}
}

/* section s's songs, in their order, into rows: how many */
static int section_rows(int s, int *rows) {
	int n = 0;
	for (int i = 0; i < jukebox_song_count && n < JUKE_SECTION_MOST; ++i)
		if (jukebox_songs[i].game == J.sec[s].game && jukebox_songs[i].kind == J.sec[s].kind) rows[n++] = i;
	return n;
}

static int section_of(int song) {
	for (int s = 0; s < J.nsec; ++s)
		if (jukebox_songs[song].game == J.sec[s].game && jukebox_songs[song].kind == J.sec[s].kind) return s;
	return -1;
}

/* ---- what is kept: jukebox.ini ---- */

static void kept_path(char *path, size_t n) { snprintf(path, n, "%s/jukebox.ini", g_data_dir); }

static void kept_read(void) {
	char path[600], line[96];
	kept_path(path, sizeof path);
	FILE *f = fopen(path, "r");
	int game = -1, song = -1, mode = 0;
	while (f && fgets(line, sizeof line, f)) {
		if (sscanf(line, "song = %d %x", &game, &song) == 2) continue;
		(void)sscanf(line, "mode = %d", &mode);
	}
	if (f) fclose(f);
	J.mode = mode >= 0 && mode < JUKE_MODES ? mode : JUKE_REPEAT;
	for (int i = 0; i < jukebox_song_count; ++i)
		if (jukebox_songs[i].game == game && jukebox_songs[i].song == song && section_of(i) >= 0) {
			int rows[JUKE_SECTION_MOST], n;
			J.section = section_of(i);
			n = section_rows(J.section, rows);
			for (int r = 0; r < n; ++r)
				if (rows[r] == i) J.at = r;
		}
}

static void kept_write(void) {
	int rows[JUKE_SECTION_MOST], n = section_rows(J.section, rows);
	if (!n) return;
	const JukeSong *s = &jukebox_songs[rows[J.at < n ? J.at : 0]];
	char path[600], tmp[610];
	kept_path(path, sizeof path);
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	FILE *f = fopen(tmp, "w");
	if (!f) return;
	fprintf(f, "# The Jukebox's last song (its game: 0 BN6, 1 BN5; its number) and play mode (docs/game/pet.md, The Jukebox).\n"
		"song = %d %02X\nmode = %d\n", s->game, s->song, J.mode);
	bool ok = !ferror(f) && cw_fsync_file(f);
	if (fclose(f)) ok = false;
	if (ok) ok = cw_rename(tmp, path);
	if (!ok) remove(tmp);
	platform_persist();
}

/* ---- playing ---- */

static void play(int song) {
	if (J.playing >= 0) audio_song_stop(jukebox_songs[J.playing].game, jukebox_songs[J.playing].song);
	J.playing = song;
	J.frames = J.quiet = J.silent = 0;
	J.fading = false;
	if (song >= 0) audio_song_start(jukebox_songs[song].game, jukebox_songs[song].song);
}

static uint32_t next_random(void) {
	J.rng ^= J.rng << 13;
	J.rng ^= J.rng >> 17;
	J.rng ^= J.rng << 5;
	return J.rng;
}

/* the song after the one playing, by the mode, in its own section */
static int next_song(void) {
	int s = section_of(J.playing), rows[JUKE_SECTION_MOST];
	int n = s < 0 ? 0 : section_rows(s, rows), at = 0;
	if (n < 2 || J.mode == JUKE_REPEAT) return J.playing;
	for (int r = 0; r < n; ++r)
		if (rows[r] == J.playing) at = r;
	if (J.mode == JUKE_IN_ORDER) return rows[(at + 1) % n];
	int pick = (int)(next_random() % (uint32_t)(n - 1));
	return rows[pick >= at ? pick + 1 : pick];
}

/* the frame's playing, its tracks' loudness read for the lights: a
 * looping song round twice, then faded; an ended or faded one, after a
 * quiet, the next */
static void playing_frame(void) {
	J.v.tracks = 0;
	memset(J.v.levels, 0, sizeof J.v.levels);
	if (J.playing < 0) return;
	const JukeSong *s = &jukebox_songs[J.playing];
	int loops = audio_song_state(s->game, s->song, J.v.levels, JUKE_TRACKS, &J.v.tracks);
	bool sounds = false;
	for (int i = 0; i < J.v.tracks; ++i) sounds |= J.v.levels[i] != 0;
	/* (no tracks read: no sound to tell its silence by) */
	J.silent = sounds || !J.v.tracks ? 0 : J.silent + 1;
	if (loops >= 0 && J.silent >= SILENT && J.frames > SILENT) {
		audio_song_stop(s->game, s->song);
		loops = -1;
	}
	if (loops >= 0) {
		++J.frames;
		J.v.loops = loops;
		if (J.mode != JUKE_REPEAT && loops >= ROUNDS && !J.fading) {
			audio_song_fade(s->game, s->song, FADE_SPEED);
			J.fading = true;
		}
		return;
	}
	if (++J.quiet >= GAP) play(next_song());
}

/* ---- the screen ---- */

static void cursor_to(int at, int n) {
	J.at = at < 0 ? 0 : at >= n ? n - 1 : at;
	if (J.at < J.top) J.top = J.at;
	if (J.at >= J.top + JUKE_ROWS) J.top = J.at - JUKE_ROWS + 1;
}

static void turn(int by) {
	if (J.nsec < 2) return;
	J.section = (J.section + by + J.nsec) % J.nsec;
	int rows[JUKE_SECTION_MOST], n = section_rows(J.section, rows);
	J.at = J.top = 0;
	for (int r = 0; r < n; ++r)
		if (rows[r] == J.playing) J.at = r;
	cursor_to(J.at, n);
	audio_sfx(SFX_SELECT);
}

static void mode_next(void) {
	J.mode = (J.mode + 1) % JUKE_MODES;
	audio_sfx(SFX_SELECT);
}

static void close_it(void) {
	play(-1);
	kept_write();
	J.shown = false;
	J.letting_go = true;
	platform_picture_taps(false);
	audio_sfx(SFX_DESC_CLOSE);
}

static bool inside(SDL_Rect r, int x, int y) { return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h; }

/* a tap on the picture: a row plays, the arrows turn, the mode's hint turns the mode */
static void taps(const int *rows, int n) {
	int x, y;
	if (!platform_tap(&x, &y)) return;
	x -= P.core_x;
	y -= P.core_y;
	for (int i = 0; i < JUKE_ROWS && J.top + i < n; ++i)
		if (inside(jukebox_row_rect(i), x, y)) { cursor_to(J.top + i, n); play(rows[J.at]); return; }
	for (int r = 0; r < 2; ++r)
		if (inside(jukebox_turn_rect(r), x, y)) { turn(r ? 1 : -1); return; }
	if (inside(jukebox_mode_rect(&J.v), x, y)) mode_next();
}

static void keys(const int *rows, int n) {
	if (btn_repeat(BTN_UP) && J.at > 0) { cursor_to(J.at - 1, n); audio_sfx(SFX_CURSOR); }
	else if (btn_repeat(BTN_DOWN) && J.at < n - 1) { cursor_to(J.at + 1, n); audio_sfx(SFX_CURSOR); }
	if (btn_pressed(BTN_L | BTN_LEFT)) turn(-1);
	else if (btn_pressed(BTN_R | BTN_RIGHT)) turn(1);
	if (btn_pressed(BTN_START | BTN_SELECT)) mode_next();
	if (btn_pressed(BTN_A) && n) play(rows[J.at]);
	else if (btn_pressed(BTN_B)) close_it();
}

/* the view the draw reads */
static void view(const int *rows, int n) {
	JukeView *v = &J.v;
	++v->t;
	v->game = J.sec[J.section].game;
	v->section = jukebox_kind_names[J.sec[J.section].kind];
	v->section_at = J.section;
	v->sections = J.nsec;
	memcpy(v->rows, rows, (size_t)n * sizeof *rows);
	v->n = n;
	v->at = J.at;
	v->top = J.top;
	v->playing = J.playing;
	v->mode = J.mode;
	v->seconds = J.frames / 60;
	if (J.playing < 0) v->loops = 0;
}

void jukebox_open(void) {
	sections_find();
	if (!J.nsec) return;
	J.shown = true;
	J.section = J.at = J.top = 0;
	J.playing = -1;
	memset(&J.v, 0, sizeof J.v);
	J.v.playing = -1;
	kept_read();
	int rows[JUKE_SECTION_MOST], n = section_rows(J.section, rows);
	J.top = 0;
	cursor_to(J.at, n);
	J.rng = (uint32_t)SDL_GetPerformanceCounter() | 1u;
	platform_picture_taps(true);
	audio_sfx(SFX_DESC_OPEN);
	printf("jukebox: open, %d sections\n", J.nsec);
}

bool jukebox_shown(void) { return J.shown; }

void jukebox_shut(void) {
	if (!J.shown) return;
	play(-1);
	J.shown = J.letting_go = false;
	platform_picture_taps(false);
}

bool jukebox_update(void) {
	if (!J.shown) {
		if (J.letting_go && btn_held(BTN_A | BTN_B)) return true;
		J.letting_go = false;
		return false;
	}
	int rows[JUKE_SECTION_MOST], n = section_rows(J.section, rows);
	/* (its first frame's keys are the ones that opened it) */
	if (J.v.t > 0) {
		keys(rows, n);
		if (J.shown) taps(rows, n);
	}
	if (!J.shown) return true;
	n = section_rows(J.section, rows);
	playing_frame();
	view(rows, n);
	return true;
}

void jukebox_draw(void) {
	if (!J.shown) return;
	jukebox_draw_view(P.core_x, P.core_y, &J.v);
	look_fade_in(P.core_x, P.core_y, CORE_W, CORE_H, J.v.t);
}
