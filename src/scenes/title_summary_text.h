/* The run summary's words (title_summary_text.c): its two screens' names,
 * the rows of the run and of the records, in the plain style of the game's
 * menus (docs/voice/rules.md), apart from the logic that says when. MegaMan's
 * words in its NAVIGATOR box are title_lines.c's; what a run opened and the
 * closest goal are meta.c's. */
#ifndef CW_TITLE_SUMMARY_TEXT_H
#define CW_TITLE_SUMMARY_TEXT_H

#include <stddef.h>

typedef enum {
	SM_RESULT,        /* the first screen's band: the run, as BN6's battle's RESULT */
	SM_RECORDS,       /* the second's: what it leaves the next run */
	SM_NAVIGATOR,     /* MegaMan's box's label */
	SM_INFORMATION,   /* the closest goal's box's label, KeyItem's */
	SM_DELETED,       /* how a lost run ended */
	SM_FALLEN,        /* ... a won one */
	SM_REACHED, SM_VIRUSES, SM_NAVIS,   /* the run's rows */
	SM_LIBRARY, SM_BEST, SM_WEEK,       /* the records' rows (a dive's best is its week's) */
	SM_NEW,           /* what is new, as BN6 marks a new mail */
	SM_DIVE,          /* a dive's end's band, its RESULT (issue #769) */
	SM_DIVE_OVER,     /* how a dive ended deleted */
	SM_CREW, SM_PLACE,                  /* its rows beside Reached: the crew's battles, the Board's place */
	SM_COUNT
} SummaryText;
const char *summary_text(SummaryText w);

/* the crew's battles, "2 of 3 won", or "None" */
const char *summary_crew(char *buf, size_t n, int fought, int won);
/* the Board's place as the server says it: "3rd", "Asking...", "No link" (0 asked, -1 no answer) */
const char *summary_place(char *buf, size_t n, int place);
/* a layer, as the rows give it: "Layer 5" */
const char *summary_layer(char *buf, size_t n, int layer);
/* the Library's count and what the run added: "12 (+2)", or "12" */
const char *summary_library(char *buf, size_t n, int count, int added);
/* a helped run's helpers on the band, small capitals, and the extras it
 * left out: by their names, one after another ("HELP: HP+ All *  OFF:
 * Glitch"; NULL for none), or how many where the names run long ("HELP: 5
 * ON  OFF: 2"; 0 for none) */
const char *summary_band(char *buf, size_t n, const char *helpers, const char *off);
const char *summary_band_count(char *buf, size_t n, int on, int off);

#endif
