/* DarkChips on the run (director_dark.c), for the director's other parts. */
#ifndef CW_DIRECTOR_DARK_H
#define CW_DIRECTOR_DARK_H

#include <stdbool.h>

extern bool dark_price_told;
void dark_flame_setup(void);
void dark_flame_watch(void);
void dark_price(bool bn6);
extern bool dark_base_told;
void dark6_after_battle(void);
void dark_pack(void);

#endif
