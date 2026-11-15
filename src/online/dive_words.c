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
