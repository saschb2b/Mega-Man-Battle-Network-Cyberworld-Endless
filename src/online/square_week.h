/* The week's net in the Net Square (square_week.c, docs/ONLINE.md, The
 * week's net): a dive's end the game kept (weekly.h) posted as the visit
 * begins; what the greeter Mr.Prog by the Board offers (a new dive, the
 * unfinished one, or nothing while MegaMan is on one or a trade waits to
 * go home); his Yes taking MegaMan home to dive, and the dive begun or
 * taken up again as the visit ends there. */
#ifndef CW_SQUARE_WEEK_H
#define CW_SQUARE_WEEK_H

#include <stdbool.h>

#include "online_types.h"
#include "square_week_words.h"

/* The greeter's flags for his choice (the Square's own, docs/EMULATION.md:
 * the one left free there): a dive, the new one or the unfinished one as
 * he offered; with the way home's flag beside it, the unfinished one
 * begun again (a pair of the Square's flags, cleared as the chat closes) */
#define SQUARE_DIVE_FLAG 0x14AF
#define SQUARE_DIVE_OVER_FLAG 0x1493

/* As MegaMan arrives in the Square. */
void square_week_reset(void);
/* After each frame there: the kept end posted, its answer noted. */
void square_week_frame(const OnlineSnapshot *s);
/* What the greeter offers now. */
WeekOffer square_week_offer(void);
/* The greeter's archive: his welcome (`welcome`, pages), the week's net's
 * post after it, and his question or options with their flags; true where
 * it was put in his place (online_square_prog_say). */
bool square_week_greeter(const char *welcome);
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
