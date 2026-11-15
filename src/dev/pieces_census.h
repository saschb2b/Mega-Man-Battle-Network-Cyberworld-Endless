/* The set pieces a player meets (pieces_census.c,
 * docs/world/level-design/set-pieces.md). */
#ifndef CW_PIECES_CENSUS_H
#define CW_PIECES_CENSUS_H

/* Runs of seeds 1 to `runs`, each normal layer of the net's cycle built as
 * the game builds it (its area dressed by the seed, its kit the area's):
 * which set pieces the act's plan gave each and which stand, by the layer's
 * place in its act, by BN6's areas and the dressed ones, and by area,
 * written to DIR/pieces.txt. */
int pieces_census_run(const char *dir, int runs);

#endif
