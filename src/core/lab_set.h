/* The Battle Lab's sets (docs/game/battle-lab.md): what a test battle is
 * made of, and a set kept in the profile with its best. Bytes alone, so
 * the profile holds them as they are. */
#ifndef CW_LAB_SET_H
#define CW_LAB_SET_H

#include <stdint.h>

#define LAB_FOES 4       /* a battle's foes at most: BN6's spawn takes four */
#define LAB_COLS 6
#define LAB_ROWS 3
#define LAB_PANELS (LAB_COLS * LAB_ROWS)
#define LAB_SETS 4       /* the sets a profile keeps */

#define LAB_AREA_SECOND 0x80
/* A song: its game (0 BN6, 1 + XRomId another, as the Jukebox's) in the top two bits over its number in that game's
 * song table; BN6's are their numbers alone */
#define LAB_SONG_GAMES 4
#define LAB_SONG_MOST 63
#define LAB_SONG(game, number) ((game) << 6 | (number))
#define LAB_SONG_GAME(song) ((song) >> 6)
#define LAB_SONG_NUMBER(song) ((song) & LAB_SONG_MOST)

/* A foe set down: its family (the engine's AI index), version and panel
 * (column 3-5 and row 0-2, from 0) */
typedef struct { uint8_t family, version, col, row; } LabFoe;

/* A set: what a test battle is made of */
typedef struct {
	uint8_t nfoes;
	LabFoe foes[LAB_FOES];
	uint8_t panels[LAB_PANELS];   /* BN6_PANEL_TYPE of each, row by row from the top left */
	uint8_t area;                 /* its background: an area of the net (BIOME_*, another game's from NET_AREAS), with
	                               * LAB_AREA_SECOND the second of a BN6 area's two */
	uint8_t song;                 /* its song: LAB_SONG of its game and its number there; 0 the battle's own */
} LabSet;

/* A set kept, and its best win: the busting level (1-10, 11 S; 0 none
 * yet), its time in frames and the HP lost in it */
typedef struct {
	LabSet set;
	uint8_t bust;
	uint16_t frames, hp_lost;
} LabSaved;

#endif
