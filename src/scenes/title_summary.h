/* The finished run's summary over the title (docs/game/meta/), as BN6's
 * battle's RESULT in two beats: the run (how it ended, how far, MegaMan's
 * word, title_lines.h), then the records (the Library, the best, what it
 * opened and the closest goal); a helped run's helpers on the band. */
#ifndef CW_TITLE_SUMMARY_H
#define CW_TITLE_SUMMARY_H

#include <stdbool.h>
#include <stdio.h>

/* the summary opens on the run */
void title_summary_open(void);
/* a frame of it: whether the player closed it (A or START turns to the
 * records, then closes them) */
bool title_summary_update(void);
/* over the title's picture, its top left at (x0, y0) */
void title_summary_draw(int x0, int y0);
/* The summary in a headless run's state where it is up, a line each: its
 * screen (`summary` result or records), how the run ended (`ending` won
 * or deleted), the layer it reached (`reached`), whether that is a new
 * best (`newbest`) and what it opened for the next (`opened`, apart by
 * commas, none). Reads only. */
void title_summary_describe(FILE *f, bool shown);

#endif
