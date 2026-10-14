/* The Network screen, opened deliberately from the title. */
#ifndef CW_NETWORK_H
#define CW_NETWORK_H

#include "game.h"

void network_open(void);
/* Return from a failed or ended visit with an actionable message. */
void network_message(const char *message);

#endif
