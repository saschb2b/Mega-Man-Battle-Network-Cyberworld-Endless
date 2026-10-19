/* The channel's NetBattles as the game holds them (online_stands.h). */
#include "online_stands.h"

#include <string.h>

static struct {
	StandsEntry entry[ONLINE_STANDS_MAX];
	int n;
	uint32_t serial;
} S;

void online_stands_reset(void) { memset(&S, 0, sizeof S); }

bool online_stands_mail(const uint8_t *data, size_t size) {
	StandsEntry e[ONLINE_STANDS_MAX];
	int n = stands_wire_read(data, size, e);
	if (n < 0) return false;
	memcpy(S.entry, e, sizeof e);
	S.n = n;
	++S.serial;
	return true;
}

int online_stands_entries(const StandsEntry **out) {
	*out = S.entry;
	return S.n;
}

uint32_t online_stands_serial(void) { return S.serial; }
