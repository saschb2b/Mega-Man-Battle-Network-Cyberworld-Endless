/* Saves sent by a code (docs/engine/saves.md, Sending by code): the game's
 * side of shared/online/README.md's TRANSFER exchange, over the Net
 * Square's own WebSocket (online_socket.h on a thread of its own; the
 * browser's online_browser.h on its main thread), never while a visit is
 * on: SAVES opens only over the title and the ROMs screen. */
#ifndef CW_TRANSFER_H
#define CW_TRANSFER_H

#include "transfer_link.h"

/* The link for SAVES (transfer_link_set), its server `url` (the official
 * Square's, or --dev online's); NULL where the system has no network for
 * it */
const TransferLink *online_transfer_link(const char *url);

#endif
