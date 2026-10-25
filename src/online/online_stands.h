/* The channel's NetBattles as the game holds them (issue #128,
 * docs/online/arena-in-brief.md, The stands): the server's last STANDS, each battle on
 * or the last one over with who fights whom, by which rules, how many
 * watch and cheer, and the cable's first game's report (HP, the last chip
 * each used, the winner). Every game holds it, one that cannot watch
 * among them: the referee and the Board read it. */
#ifndef CW_ONLINE_STANDS_H
#define CW_ONLINE_STANDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "stands_wire.h"

/* As MegaMan arrives in the Square: none. */
void online_stands_reset(void);
/* A STANDS message come in; false where it is malformed. */
bool online_stands_mail(const uint8_t *data, size_t size);
/* The battles as last said; how many. */
int online_stands_entries(const StandsEntry **out);
/* Changed with every STANDS (0 none yet). */
uint32_t online_stands_serial(void);

#endif
