/* dive_band.h; dive_link_line and dive_link_draw (dive_link.h) */
#include "dive_band.h"

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "director.h"
#include "director_crew.h"
#include "dive_crew_view.h"
#include "dive_join.h"
#include "dive_link.h"
#include "dive_party.h"
#include "dive_people.h"
#include "dive_traces.h"
#include "dive_words.h"
#include "party_play.h"
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

/* whose battle MegaMan was taken into, at the picture's foot */
static void named(void) {
	const char *join = dive_join_band();
	if (join) look_band(P.core_x, P.core_y + 160 - LOOK_BAND_H - 1, 240, join);
}

void dive_band_reset(void) {
	memset(&B, 0, sizeof B);
	/* (over a crew's battle as it starts too: party_play.h) */
	party_play_draw_more = named;
	/* (the crew's gifts after a battle told here: director_crew.h) */
	director_crew_band = dive_link_line;
}

void dive_band_describe(FILE *f) {
	/* (as drawn: whose battle MegaMan was taken into first, issue #750) */
	const char *join = dive_join_band();
	fprintf(f, "band %s\n", join ? join : B.nfeed ? B.feed[0] : "none");
}

void dive_band_frame(uint32_t now, bool linked, uint32_t me) {
	/* (MegaMan's HP to his crew as it changes, and the crew's plates: dive_crew_view.h) */
	dive_crew_view_frame(linked, me, now);
	dive_band_tick(now);
}

void dive_band_tick(uint32_t now) {
	if (B.nfeed && B.shown_at && now - B.shown_at > FEED_MS) {
		memmove(B.feed, B.feed + 1, sizeof B.feed[0] * (size_t)(B.nfeed - 1));
		B.shown_at = --B.nfeed ? now : 0;
	}
	if (B.nfeed && !B.shown_at) B.shown_at = now;
}

void dive_band_back(void) { dive_link_line(dive_back_words()); }

void dive_band_week(void) { dive_link_line(dive_back_week_words()); }

void dive_band_alone(void) {
	dive_link_line(dive_alone_words());
	dive_link_line(dive_alone_back_words());
}

void dive_link_line(const char *line) {
	if (!*line) return;
	if (B.nfeed == FEED_MOST) { memmove(B.feed, B.feed + 1, sizeof B.feed[0] * (FEED_MOST - 1)); --B.nfeed; B.shown_at = 0; }
	snprintf(B.feed[B.nfeed++], sizeof B.feed[0], "%s", line);
	fprintf(stderr, "dive: feed: %s\n", line);
}

void dive_link_draw(void) {
	/* (none of it over SELECT's map: a name and a ring had stood on it, issue #728) */
	if (director_map_over()) return;
	dive_traces_draw();
	/* (whose battle MegaMan was taken into, over the battle's start too, issue #750) */
	const char *join = dive_link_on() ? dive_join_band() : NULL;
	if (!director_on_map()) {
		if (join) named();
		return;
	}
	bool chat = emu_read8(BN6_CHATBOX) != 0;
	/* (a dive with no link: the feed alone, CONTINUE's word on the link it did not get, issue #759) */
	if (!dive_link_on()) {
		if (dive_link_diving() && B.nfeed && !chat) look_band(P.core_x, P.core_y + 160 - LOOK_BAND_H - 1, 240, B.feed[0]);
		return;
	}
	/* (the divers' names over their heads: dive_people.h) */
	dive_people_draw(chat);
	/* (the crew's plates under BN6's HP box: dive_crew_view.h) */
	dive_crew_view_draw();
	/* (never over BN6's chat box: its words first) */
	if (chat) return;
	int y = P.core_y + 160 - LOOK_BAND_H - 1;
	/* (a crewmate's battle's offer first: it stands a few seconds only) */
	/* (the link down past a second: said until it is back, its dots moving, issue #751) */
	bool away = false;
	uint32_t now = SDL_GetTicks();
	const char *down = dive_link_down(now, &away) ? dive_down_words(away, (int)(now / 400 % 4)) : NULL;
	if (join) look_band(P.core_x, y, 240, join);
	else if (down) look_band(P.core_x, y, 240, down);
	else if (B.nfeed) look_band(P.core_x, y, 240, B.feed[0]);
	else dive_party_draw(P.core_x, P.core_y);
}
