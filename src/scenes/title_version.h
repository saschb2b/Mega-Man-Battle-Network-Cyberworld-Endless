/* The title's line above the copyright (scene_title.c): the build at the
 * left (v0.21.1 beta, for a report), a newer version beside it where the
 * updater found one (update.h, whose line the title's Options has), and
 * the best depth at the right. */
#ifndef CW_TITLE_VERSION_H
#define CW_TITLE_VERSION_H

#include <stdbool.h>

/* A frame of the title's: the updater's (its look at the start, and the
 * game's end once a download is in place) */
void title_version_tick(void);
/* The line at the picture's corner (x0, y0): the best depth among the
 * profile's record and `saved_depth`, where `best` asks for it */
void title_version_draw(int x0, int y0, int saved_depth, bool best);

#endif
