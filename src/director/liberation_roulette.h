/* A BonusPanel's flashes and roulette drawn as BN5 draws them
 * (liberation_roulette.c), from what liberation_bonus.c shows: the band
 * blended over the board, the slot's icon and its words, BN5's own
 * sprites read from its ROM. Nothing where its ROM is not there. */
#ifndef CW_LIBERATION_ROULETTE_H
#define CW_LIBERATION_ROULETTE_H

/* The picture, over the board and its pieces. Draws only. */
void lib_roulette_draw(void);

#endif
