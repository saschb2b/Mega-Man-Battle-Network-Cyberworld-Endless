/* The Jukebox's words (docs/PET.md, The Jukebox): every song of the music
 * player of the games in use (BN6's, BN5's where its ROM is beside it),
 * by game and section, in our own names for where each plays, and the
 * screen's labels. */
#ifndef CW_JUKEBOX_TEXT_H
#define CW_JUKEBOX_TEXT_H

#include <stdint.h>

#define JUKE_SECTION_MOST 40   /* songs in a section at most */
#define JUKE_TITLE_MOST 15     /* a name's letters at most: a row of the list holds as many */

/* a song's section: where it plays */
typedef enum { JUKE_TOWNS, JUKE_NET, JUKE_BATTLES, JUKE_STORY, JUKE_SCREENS, JUKE_MORE, JUKE_KINDS } JukeKind;

/* a song: its game (0 BN6, 1 + XRomId another: mp2k.h), its number in
 * that game's song table, its section and its name */
typedef struct {
	uint8_t game, song, kind;
	const char *title;
} JukeSong;

extern const JukeSong jukebox_songs[];
extern const int jukebox_song_count;
/* the sections' names, and each game's short name on its cartridge */
extern const char *const jukebox_kind_names[JUKE_KINDS];
extern const char *const jukebox_game_tags[2];

/* the play modes' names (jukebox.h's JukeMode order) */
extern const char *const jukebox_mode_names[3];

extern const char jukebox_title[];
extern const char jukebox_now_label[], jukebox_idle_label[], jukebox_pick_hint[];
extern const char jukebox_key_play[], jukebox_key_back[], jukebox_key_section[], jukebox_key_mode[];
/* "No. 13" as the card shows a song's number */
void jukebox_number_words(char *out, int n, int song);
/* how often it has gone round ("Once round", "3 times round"), "" for none */
void jukebox_loops_words(char *out, int n, int loops);

#endif
