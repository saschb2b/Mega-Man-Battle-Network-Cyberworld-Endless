/* The shared dive's messages from the server, each to its part (issue
 * #589; dive_link_mail in dive_link.h): the room, the feed, the pack, the
 * party, the crew, the traces, a split's place, the Board's records; and
 * the dive's company counted for the statistics. */
#ifndef CW_DIVE_MAIL_H
#define CW_DIVE_MAIL_H

void dive_mail_reset(void);
/* The dive's end told: its company sent (analytics_dive_together). */
void dive_mail_told(void);

#endif
