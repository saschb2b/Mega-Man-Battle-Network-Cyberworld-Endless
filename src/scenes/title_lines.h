/* What MegaMan says on the run's summary (title_lines.c). */
#ifndef CW_TITLE_LINES_H
#define CW_TITLE_LINES_H

/* a won run: how he came home, and his cheer; a lost one: how Dad's
 * backup brought him home, and his word: the battle data the loss gave, a
 * new best, a rough start, or on */
enum { SUMMARY_WON_HOW, SUMMARY_WON, SUMMARY_LOST_HOW, SUMMARY_LEARNED, SUMMARY_BEST, SUMMARY_ROUGH, SUMMARY_FURTHER, SUMMARY_LINES };
const char *summary_words(int which);

#endif
