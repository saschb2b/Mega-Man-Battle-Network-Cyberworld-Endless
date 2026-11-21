/* dive_band.h; dive_link_line and dive_link_draw (dive_link.h) */
#include "dive_band.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "director.h"
#include "dive_crew_view.h"
#include "dive_join.h"
#include "dive_link.h"
#include "dive_party.h"
#include "dive_people.h"
#include "dive_traces.h"
#include "dive_words.h"
#include "emu.h"
#include "pet_look.h"
#include "platform.h"

#define FEED_MOST 4          /* lines waiting their turn */
#define FEED_MS 3500         /* each shown so long */

static struct {
	char feed[FEED_MOST][64];
	int nfeed;
	uint32_t shown_at;       /* the first line's start, 0 not yet */
} B;

void dive_band_reset(void) { memset(&B, 0, sizeof B); }

void dive_band_frame(uint32_t now) {
	if (B.nfeed && B.shown_at && now - B.shown_at > FEED_MS) {
		memmove(B.feed, B.feed + 1, sizeof B.feed[0] * (size_t)(B.nfeed - 1));
		B.shown_at = --B.nfeed ? now : 0;
	}
	if (B.nfeed && !B.shown_at) B.shown_at = now;
}

void dive_link_line(const char *line) {
	if (!*line) return;
	if (B.nfeed == FEED_MOST) { memmove(B.feed, B.feed + 1, sizeof B.feed[0] * (FEED_MOST - 1)); --B.nfeed; B.shown_at = 0; }
	snprintf(B.feed[B.nfeed++], sizeof B.feed[0], "%s", line);
	fprintf(stderr, "dive: feed: %s\n", line);
}

void dive_link_draw(void) {
	dive_traces_draw();
	if (!dive_link_on() || !director_on_map()) return;
	bool chat = emu_read8(BN6_CHATBOX) != 0;
	/* (the divers' names over their heads: dive_people.h) */
	dive_people_draw(chat);
	/* (the crew's plates under BN6's HP box: dive_crew_view.h) */
	dive_crew_view_draw();
	/* (never over BN6's chat box: its words first) */
	if (chat) return;
	int y = P.core_y + 160 - LOOK_BAND_H - 1;
	/* (a crewmate's battle's offer first: it stands a few seconds only) */
	const char *join = dive_join_band();
	if (join) look_band(P.core_x, y, 240, join);
	else if (B.nfeed) look_band(P.core_x, y, 240, B.feed[0]);
	else dive_party_draw(P.core_x, P.core_y);
}
