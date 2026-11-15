/* dive_words.h */
#include "dive_words.h"

#include <stdio.h>

#include "protocol.h"

const char *dive_feed_words(unsigned kind, const char *name, unsigned depth, unsigned party) {
	static char s[64];
	switch (kind) {
	case ONLINE_FEED_DELETED: snprintf(s, sizeof s, "%s deleted on layer %u", name, depth); break;
	case ONLINE_FEED_WON: snprintf(s, sizeof s, "%s cleared the Deep Net!", name); break;
	case ONLINE_FEED_GUARDIAN:
		if (party > 1) snprintf(s, sizeof s, "%s's party of %u beat layer %u", name, party, depth);
		else snprintf(s, sizeof s, "%s beat layer %u's guardian", name, depth);
		break;
	case ONLINE_FEED_FLARE: snprintf(s, sizeof s, "%s waits at layer %u's door", name, depth); break;
	default: s[0] = 0; break;
	}
	return s;
}

const char *dive_door_words(unsigned count, bool leads, const char *leader) {
	static char s[64];
	if (count < 2) snprintf(s, sizeof s, leads ? "Waiting for a party  A:Go in" : "Joining the party...");
	else if (leads) snprintf(s, sizeof s, "Party of %u!  A:Go in  B:Alone", count);
	else snprintf(s, sizeof s, "%s leads %u  B:Go alone", leader, count);
	return s;
}
