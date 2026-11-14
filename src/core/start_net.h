/* The start's network parts, once the window and settings.ini are there
 * (startup.c): the anonymous statistics' thread (analytics_net.h) and
 * SAVES' saves sent by a code (transfer_link.h), over the official
 * Square's server or --dev online's. Neither opens a connection here. */
#ifndef CW_START_NET_H
#define CW_START_NET_H

#include <stdbool.h>

/* `headless` and `plain` as analytics_net_start takes them */
void start_net(bool headless, bool plain);

#endif
