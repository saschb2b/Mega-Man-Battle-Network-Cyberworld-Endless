/* The ROM's pictures on the second screen (second_art.c): drawn from the
 * player's ROM as the game runs, never kept in a file (AGENTS.md). */
#ifndef CW_SECOND_ART_H
#define CW_SECOND_ART_H

#include <stdint.h>

/* A chip's rank, as BN5 DS frames its chips' pictures: standard, Mega,
 * Giga, Dark */
enum { RANK_STANDARD, RANK_MEGA, RANK_GIGA, RANK_DARK };
int second_chip_rank(int chip);
/* A chip's element (its record's, by its icon), -1 for none */
int second_chip_element(int chip);
/* A chip's icon, 16 x 16 from (x, y), as the folder's list shows it */
void second_chip_icon(int chip, int x, int y);
/* An element's icon, 16 x 16 from (x, y), as the folder's list shows it */
void second_element_icon(int element, int x, int y);

#endif
