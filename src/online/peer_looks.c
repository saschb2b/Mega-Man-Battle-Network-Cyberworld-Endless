/* peer_looks.h. */
#include "peer_looks.h"

#include <string.h>

#define KEPT 32   /* players remembered, the oldest seen replaced first */

static struct {
	uint32_t session, seen;
	uint8_t identity[ONLINE_IDENTITY_BYTES];
	Look look;
} known[KEPT];
static uint32_t serial;

_Static_assert(sizeof(Look) == 2 * ONLINE_LOOK_CHANNELS && LOOK_CHANNELS == ONLINE_LOOK_CHANNELS,
	"a Look is the wire's six bytes (protocol v30)");
_Static_assert(LOOK_PALETTES == ONLINE_LOOK_PALETTES && LOOK_COLOURS == ONLINE_LOOK_COLOURS && LOOK_NONE == ONLINE_LOOK_NONE,
	"the wire's bounds are the model's (shared/online/protocol.h, src/core/looks.h)");

void peer_looks_reset(void) { memset(known, 0, sizeof known); serial = 0; }

static void note(const OnlinePeer *p) {
	int at = 0;
	for (int k = 0; k < KEPT; ++k) {
		if (known[k].session == p->id || !memcmp(known[k].identity, p->identity, ONLINE_IDENTITY_BYTES)) { at = k; break; }
		if (known[k].seen < known[at].seen) at = k;
	}
	known[at].session = p->id;
	known[at].seen = serial;
	memcpy(known[at].identity, p->identity, ONLINE_IDENTITY_BYTES);
	memcpy(&known[at].look, p->look, sizeof known[at].look);
}

void peer_looks_note(const OnlineSnapshot *snapshot) {
	++serial;
	for (size_t i = 0; i < snapshot->peer_count; ++i) note(&snapshot->peers[i]);
}

static bool found(int k, Look *out) {
	if (k < 0 || !look_valid(&known[k].look)) { *out = look_plain(); return false; }
	*out = known[k].look;
	return true;
}

bool peer_looks_of(uint32_t session, Look *out) {
	int at = -1;
	for (int k = 0; k < KEPT && session; ++k) if (known[k].seen && known[k].session == session) at = k;
	return found(at, out);
}
