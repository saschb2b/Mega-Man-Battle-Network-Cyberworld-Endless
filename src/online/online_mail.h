/* The transport's inbox, drained once a frame in the Square and handed
 * out by kind (online_transport_receive's messages: Trade Central's and
 * the Arena's). During an Arena battle, the Square held, the battle
 * drains it the same way. */
#ifndef CW_ONLINE_MAIL_H
#define CW_ONLINE_MAIL_H

void online_mail_frame(void);

#endif
