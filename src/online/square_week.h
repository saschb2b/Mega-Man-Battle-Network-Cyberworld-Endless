/* The week's net in the Net Square (square_week.c, docs/online/weekly-net.md, The
 * week's net): a dive's end the game kept (weekly.h) posted as the visit
 * begins; what the greeter Mr.Prog by the Board offers (a new dive, the
 * unfinished one, or nothing while MegaMan is on one or a trade waits to
 * go home); his Yes taking MegaMan home to dive, and the dive begun or
 * taken up again as the visit ends there. */
#ifndef CW_SQUARE_WEEK_H
#define CW_SQUARE_WEEK_H

#include <stdbool.h>
#include <stdio.h>

#include "online_types.h"
#include "square_week_words.h"

/* The greeter's flags for his choice (the Square's own, docs/engine/emulation/:
 * the one left free there): a dive, the new one or the unfinished one as
 * he offered; with the way home's flag beside it, the unfinished one
 * begun again (a pair of the Square's flags, cleared as the chat closes) */
#define SQUARE_DIVE_FLAG 0x14AF
#define SQUARE_DIVE_OVER_FLAG 0x1493

/* The week's net in the state's words (online_describe.h): what the portal
 * offers (`weekoffer none|new|again|on|trade|stamp`), the week (`week N`)
 * and the Board's posts of it as the plaza's Board lists them (`weekboard
 * WORDS`, its boxes and lines apart by spaces) */
void square_week_describe(FILE *f);
/* As MegaMan arrives in the Square. */
void square_week_reset(void);
/* After each frame there: the kept end posted, its answer noted. */
void square_week_frame(const OnlineSnapshot *s);
/* What the greeter offers now. */
WeekOffer square_week_offer(void);
/* The week's portal's words as the week stands (in the greeter's place:
 * the shared dive, docs/online/shared-dive.md), its question or options
 * after them; false where they could not be written. */
bool square_week_portal(void);
/* What the greeter's words were made of, to write them again as it changes */
unsigned square_week_said(void);
/* The Board's posts of the week's net, as the server last said them */
const char *square_week_board_words(void);
/* The greeter's choice acted on (online_chat.c's flags): the dive he
 * offered, or (`over`) the unfinished one begun again; MegaMan heads home. */
void square_week_choose(bool over);
/* MegaMan heads home to dive (the visit's end is VISIT_END_DIVE). */
bool square_week_leaving(void);
/* That end: the dive begun or taken up; false where none could be. */
bool square_week_dive(void);

#endif
