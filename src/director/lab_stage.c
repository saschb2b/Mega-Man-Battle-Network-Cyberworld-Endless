/* The Battle Lab's stage (lab_stage.h). */
#include "lab_stage.h"

#include <stdint.h>

#include "guardians.h"
#include "jukebox_text.h"
#include "lab_set.h"
#include "lab_text.h"
#include "rom.h"
#include "run.h"
#include "xbackdrop.h"
#include "xsong.h"

static struct {
	uint8_t area[2 * NET_AREAS + XAREAS_MAX], song[1 + 3 * 64];
	int nareas, nsongs;
} S;

int lab_stage_backdrop(int area) {
	int a = area & ~LAB_AREA_SECOND;
	if (a < NET_AREAS) return biome_backdrop(a, area & LAB_AREA_SECOND ? 1 : 0);
	/* (another game's: its own background, copied into BN6; one kept out of runs is not offered) */
	const NetAreaDef *x = net_area_def(a);
	return x && x->xrom > 0 && x->xbg && !x->held && !(area & LAB_AREA_SECOND) ? xbackdrop_install(x->xrom - 1, x->xbg, -1) : -1;
}

int lab_stage_biome(int area) {
	int a = area & ~LAB_AREA_SECOND;
	const NetAreaDef *x = a >= NET_AREAS ? net_area_def(a) : NULL;
	return a < NET_AREAS ? a : x ? x->like : 0;
}

static void area_add(int area) {
	int bg = lab_stage_backdrop(area);
	if (bg < 0) return;
	for (int i = 0; i < S.nareas; ++i)
		if (lab_stage_backdrop(S.area[i]) == bg) return;
	S.area[S.nareas++] = (uint8_t)area;
}

void lab_stage_make(void) {
	S.nareas = S.nsongs = 0;
	/* (BN6's areas, the first of each one's two and then its second, then the other games' in their order) */
	for (int a = 0; a < NET_AREAS; ++a) {
		area_add(a);
		area_add(a | LAB_AREA_SECOND);
	}
	for (int k = 0; k < XAREAS_MAX; ++k) area_add(NET_AREAS + k);
	/* (the Jukebox's list: BN6's songs, then those of the other games' that stand in BN6's song table, the themes of
	 * their areas, their battles and BN5's missions; src/audio/xsong.h) */
	S.song[S.nsongs++] = 0;
	for (int i = 0; i < jukebox_song_count && S.nsongs < (int)sizeof S.song; ++i) {
		const JukeSong *j = &jukebox_songs[i];
		if (j->song > LAB_SONG_MOST || j->game > LAB_SONG_GAMES - 1 || !LAB_SONG(j->game, j->song)) continue;
		if (j->game && xsong_install(j->game - 1, j->song, 0) <= 0) continue;
		S.song[S.nsongs++] = (uint8_t)LAB_SONG(j->game, j->song);
	}
}

int lab_stage_areas(void) { return S.nareas; }
int lab_stage_area(int i) { return i >= 0 && i < S.nareas ? S.area[i] : 0; }

int lab_stage_area_like(int area) {
	int bg = lab_stage_backdrop(area);
	for (int i = 0; i < S.nareas; ++i)
		if (S.area[i] == area) return area;
	for (int i = 0; i < S.nareas; ++i)
		if (lab_stage_backdrop(S.area[i]) == bg) return S.area[i];
	return 0;
}

void lab_stage_area_name(int area, char *out, size_t n) {
	int a = area & ~LAB_AREA_SECOND;
	const NetAreaDef *x = a >= NET_AREAS ? net_area_def(a) : NULL;
	if (x) lab_stage_words(out, n, x->xrom < JUKE_GAMES ? jukebox_game_tags[x->xrom] : "", x->short_name, false);
	else lab_stage_words(out, n, "", guardian_area_original_short(a), area & LAB_AREA_SECOND);
}

int lab_stage_songs(void) { return S.nsongs; }
int lab_stage_song(int i) { return i >= 0 && i < S.nsongs ? S.song[i] : 0; }

int lab_stage_song_slot(int song) {
	int game = LAB_SONG_GAME(song), number = LAB_SONG_NUMBER(song);
	if (!song || !game) return number;
	int slot = xsong_install(game - 1, number, 0);
	return slot > 0 ? slot : 0;
}

void lab_stage_song_name(int song, char *out, size_t n) {
	int game = LAB_SONG_GAME(song), number = LAB_SONG_NUMBER(song);
	for (int i = 0; song && i < jukebox_song_count; ++i)
		if (jukebox_songs[i].game == game && jukebox_songs[i].song == number) {
			lab_stage_words(out, n, game && game < JUKE_GAMES ? jukebox_game_tags[game] : "", jukebox_songs[i].title, false);
			return;
		}
	lab_stage_words(out, n, "", lab_own_song, false);
}
