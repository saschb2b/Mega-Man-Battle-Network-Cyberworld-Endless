/* The update box (docs/engine/updates.md, What the player sees): a newer
 * version's news, its CHANGELOG headlines from update.json, in SubChip's
 * flat panel under the PET's band, then BN6's own chat box asking, as the
 * statistics' question asks (analytics_ask.c): Update now? with the cursor
 * on Yes, or Open the download page?, or where it comes from and OK. The
 * title opens it once a version as its menu shows (a No or B remembered:
 * update.h's update_box_done), and A on Options' update line opens it
 * again. Yes from the title shows Options on the update line, where the
 * download's share in shows. */
#ifndef CW_OPTIONS_NEWS_H
#define CW_OPTIONS_NEWS_H

#include <stdbool.h>

/* The title's, each frame its menu shows: the box, where it is due */
void news_box_offer(void);
/* Options' update line: the box again (false where there is no update) */
bool news_box_open(void);
bool news_box_shown(void);
/* Its frame's keys and its picture (options.c's, over all else) */
void news_box_update(void);
void news_box_draw(void);
/* B or Back: a No */
bool news_box_back(void);
/* A tap at canvas pixel (x, y) from the picture's corner */
void news_box_tap(int x, int y);

#endif
