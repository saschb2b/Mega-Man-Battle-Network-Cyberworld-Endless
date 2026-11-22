/* The shared dive's messages from the server, each to its part (issue
 * #589; dive_link_mail in dive_link.h): the room, the feed, the pack, the
 * party, the crew, the traces, a split's place, the Board's records; and
 * the dive's company counted for the statistics. */
#ifndef CW_DIVE_MAIL_H
#define CW_DIVE_MAIL_H

#include <stdbool.h>

void dive_mail_reset(void);
/* The dive's end told: its company sent (analytics_dive_together); and,
 * `linked`, the link kept a few seconds for the Board's answer, the dive's
 * place on it handed to its end screen (weekly_end_placed; issue #769).
 * True where the link is kept for it (it ends it itself). */
bool dive_mail_told(bool linked);
/* The Board's answer no longer waited for: the link let go now */
void dive_mail_end_stop(void);

#endif
