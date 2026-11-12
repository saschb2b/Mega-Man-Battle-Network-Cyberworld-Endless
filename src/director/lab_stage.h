/* The Battle Lab's stage (docs/game/battle-lab.md, The builder's screen):
 * the backgrounds and the songs a set may take, BN6's and those of the
 * other games read that a run brings into BN6 (docs/multirom/). */
#ifndef CW_LAB_STAGE_H
#define CW_LAB_STAGE_H

#include <stddef.h>

/* The lists made, as the builder opens and before its battle's snapshot:
 * the first look installs the other games' backgrounds and songs where no
 * layer has yet. */
void lab_stage_make(void);

/* The backgrounds, each once: a LabSet's `area` (lab_set.h) */
int lab_stage_areas(void);
int lab_stage_area(int i);
/* ... the one of the list that shows as `area` does (an area whose
 * background another listed has, a set kept by an older build) */
int lab_stage_area_like(int area);
/* ... the number BN6's battle record names it by, and the BN6 area
 * (BIOME_*) its record is an area's */
int lab_stage_backdrop(int area);
int lab_stage_biome(int area);
void lab_stage_area_name(int area, char *out, size_t n);

/* The songs: a LabSet's `song`, 0 the battle's own first */
int lab_stage_songs(void);
int lab_stage_song(int i);
/* ... the song of BN6's table that plays it, 0 the battle's own (its
 * game's ROM not read) */
int lab_stage_song_slot(int song);
void lab_stage_song_name(int song, char *out, size_t n);

#endif
