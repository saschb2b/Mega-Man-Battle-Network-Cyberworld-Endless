/* transfer_link.h. */
#include "transfer_link.h"

static const TransferLink *linked;

void transfer_link_set(const TransferLink *link) { linked = link; }

const TransferLink *transfer_link(void) { return linked; }
