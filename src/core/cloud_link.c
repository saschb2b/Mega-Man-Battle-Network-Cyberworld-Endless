/* cloud_link.h. */
#include "cloud_link.h"

static const CloudLink *linked;

void cloud_link_set(const CloudLink *link) { linked = link; }

const CloudLink *cloud_link(void) { return linked; }
