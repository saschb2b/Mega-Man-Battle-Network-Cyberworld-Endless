#include "online_text.h"

#include <stdio.h>

const char online_title[] = "Net Square / PHASE 0";
const char online_map_failed[] = "The square could not be built.";
const char online_connecting[] = "Connecting...";
const char online_waiting[] = "Waiting for another Navi...";

void online_peer_text(char *out, size_t size, const char *name, unsigned count) {
	snprintf(out, size, "%s / %u peer(s)", name, count);
}
