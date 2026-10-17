/* The Network screen, opened deliberately from the title. */
#ifndef CW_NETWORK_H
#define CW_NETWORK_H

#include "game.h"

void network_open(void);
/* The between-act PC has just written and verified its checkpoint. */
void network_open_checkpoint(void);
/* Return from a failed or ended visit with an actionable message. */
void network_message(const char *message);

#endif
