/* BN6's own net maps measured for the layers' navigation (navstudy.c,
 * docs/DEVTOOLS.md): the way across, counted as a layer's is, and where
 * MegaMan arrives on them. */
#ifndef CW_NAVSTUDY_H
#define CW_NAVSTUDY_H

/* Prints each area's maps' counts; 0. */
int navstudy_run(void);
/* Prints each area's maps' arrivals (every jack-in and warp from another
 * map onto them: the side of the floor on the screen, the way MegaMan
 * faces) and their counts; 0. */
int navstudy_arrivals(void);

#endif
