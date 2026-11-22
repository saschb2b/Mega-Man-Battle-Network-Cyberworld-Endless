/* dive_band.h; dive_link_line and dive_link_draw (dive_link.h) */
#include "dive_band.h"

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "director.h"
#include "director_crew.h"
#include "dive_crew_view.h"
#include "dive_comings.h"
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


static void named(void);

void dive_band_reset(void) {
	memset(&B, 0, sizeof B);
	/* (over a crew's battle as it starts too: party_play.h) */
	party_play_draw_more = named;
	/* (the crew's gifts after a battle told here: director_crew.h) */
	director_crew_band = dive_link_line;
}

/* The band's line as drawn now, NULL none: whose battle MegaMan is taken
 * into first (issue #750), the link down (#751), the feed; nothing under a
 * chat box, a card, a fade or off the field (director_cover, issue #781) */
static const char *band_line(uint32_t now) {
	/* (a party's battle holding the game: whose battle, over its staging's still picture only; party_play_draw) */
	if (party_play_held()) return party_play_drawn_over() ? dive_join_band() : NULL;
	if (director_cover() != COVER_NONE) return NULL;
	/* (a dive with no link: the feed alone, CONTINUE's word on the link it did not get, issue #759) */
	if (!dive_link_on()) return dive_link_diving() && B.nfeed ? B.feed[0] : NULL;
	const char *join = dive_join_band();
	if (join) return join;
	bool away = false;
	if (dive_link_down(now, &away)) return dive_down_words(away, (int)(now / 400 % 4));
	/* (at a door the queue's own line before the feed's: issue #778) */
	const char *door = dive_party_band();
	if (door) return door;
	return B.nfeed ? B.feed[0] : NULL;
}

/* whose battle MegaMan was taken into, at the picture's foot over the
 * staging's still picture of the map, before the battle's field (issue
 * #781: never over its Custom screen) */
static void named(void) {
	const char *line = band_line(SDL_GetTicks());
	if (line) look_band(P.core_x, P.core_y + 160 - LOOK_BAND_H - 1, 240, line);
}

void dive_band_describe(FILE *f) {
	/* (what the band shows, as drawn; and what stands over the field, issue #781) */
	const char *line = band_line(SDL_GetTicks());
	fprintf(f, "band %s\ncover %s\n", line ? line : "none", director_cover_name(director_cover()));
}

void dive_band_frame(uint32_t now, bool linked, uint32_t me) {
	/* (MegaMan's HP to his crew as it changes, and the crew's plates: dive_crew_view.h) */
	dive_crew_view_frame(linked, me, now);
	/* (and the crew's comings and goings said: dive_comings.h) */
	if (linked) dive_comings_frame(now);
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
	/* (nothing over a card, a fade or what is not the field, and over a chat box only above it: director_cover, issue
	 * #781; SELECT's map among what is not, issue #728) */
	int cover = director_cover();
	if (cover == COVER_CARD || cover == COVER_FADE || cover == COVER_AWAY) return;
	bool chat = cover == COVER_CHAT;
	dive_traces_draw();
	int y = P.core_y + 160 - LOOK_BAND_H - 1;
	const char *line = band_line(SDL_GetTicks());
	if (!dive_link_on()) {
		if (line) look_band(P.core_x, y, 240, line);
		return;
	}
	/* (the divers' names over their heads: dive_people.h) */
	dive_people_draw(chat);
	/* (the crew's plates under BN6's HP box: dive_crew_view.h) */
	dive_crew_view_draw();
	/* (never over BN6's chat box: its words first) */
	if (chat) return;
	if (line) look_band(P.core_x, y, 240, line);
}
