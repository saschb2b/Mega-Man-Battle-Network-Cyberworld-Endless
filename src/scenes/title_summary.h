/* The finished run's summary over the title (docs/game/meta/), as BN6's
 * battle's RESULT in two beats: the run (how it ended, how far, MegaMan's
 * word, title_lines.h), then the records (the Library, the best, what it
 * opened and the closest goal); a helped run's helpers on the band. */
#ifndef CW_TITLE_SUMMARY_H
#define CW_TITLE_SUMMARY_H

#include <stdbool.h>

/* the summary opens on the run */
void title_summary_open(void);
/* a frame of it: whether the player closed it (A or START turns to the
 * records, then closes them) */
bool title_summary_update(void);
/* over the title's picture, its top left at (x0, y0) */
void title_summary_draw(int x0, int y0);

#endif
