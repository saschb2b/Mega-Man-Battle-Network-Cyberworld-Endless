/* The Jukebox's picture (jukebox.h), in BN6's PET screens' look (pet_look.h,
 * as its Library): the card of the song playing on the left with its
 * tracks' lights, the section's songs on the right; and where each part
 * a tap takes lies. */
#ifndef CW_JUKEBOX_DRAW_H
#define CW_JUKEBOX_DRAW_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

#include "jukebox_text.h"

#define JUKE_ROWS 7          /* songs shown at once */
#define JUKE_TRACKS 8        /* the music player's tracks */

/* what the picture shows, read from the Jukebox's state each frame */
typedef struct {
	int t;
	int game;                         /* the section's game: 0 BN6, 1 BN5 */
	const char *section;              /* its name */
	int section_at, sections;         /* its place among the sections */
	int rows[JUKE_SECTION_MOST];      /* its songs, indexes of jukebox_songs */
	int n, at, top;                   /* their count, the cursor, the first shown */
	int playing;                      /* the song playing (jukebox_songs), -1 none */
	int mode;                         /* JukeMode */
	int seconds, loops;               /* how long it has played, how often round */
	uint8_t levels[JUKE_TRACKS];      /* its tracks' loudness, 0-255 */
	int tracks;
} JukeView;

void jukebox_draw_view(int x, int y, const JukeView *v);

/* where a tap takes each part, from the picture's top left */
SDL_Rect jukebox_row_rect(int i);           /* row i shown */
SDL_Rect jukebox_turn_rect(int right);      /* the section's arrows */
SDL_Rect jukebox_mode_rect(const JukeView *v);   /* START's hint, the mode */

#endif
