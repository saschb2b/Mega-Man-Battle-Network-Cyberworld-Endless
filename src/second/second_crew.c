/* second_crew.h */
#include "second_crew.h"

#include <string.h>

#include "arena_view.h"
#include "crew_look.h"
#include "gfx.h"
#include "online_transport.h"
#include "second_frame.h"
#include "second_state.h"

#define ROW   18   /* a crewmate's row: the battle panel's slot height */
#define GAP   2
#define BARS  12   /* the link's badge (arena_view_bars) */

void second_read_crew(void) {
	S2.ncrew = dive_crew_view_rows(S2.crew);
	memset(S2.crew_link, 0, sizeof S2.crew_link);
	if (!S2.ncrew) return;
	/* (his link's bars where the server lists him among MegaMan's layer's divers; elsewhere none) */
	static OnlineSnapshot s;   /* (large: kept off the stack) */
	online_transport_snapshot(&s);
	for (int i = 0; i < S2.ncrew; ++i)
		for (size_t k = 0; k < s.peer_count; ++k)
			if (s.peers[k].id == S2.crew[i].session) S2.crew_link[i] = s.peers[k].link;
}

SDL_Rect second_crew_draw(SDL_Rect body) {
	if (!S2.ncrew) return body;
	for (int i = 0; i < S2.ncrew; ++i) {
		const CrewRow *r = &S2.crew[i];
		int x = body.x, y = body.y + i * (ROW + GAP), w = body.w, right = x + w - 3;
		fill_rect(x, y, w, ROW, PET_SLOT);
		fill_rect(x, y, 2, ROW, CREW_COLOUR);
		text_draw(x + 6, y + 3, r->full, CREW_COLOUR, TEXT_LEFT);
		if (S2.crew_link[i]) {
			arena_view_bars(right - BARS, y + (ROW - 11) / 2, S2.crew_link[i]);
			right -= BARS + 4;
		}
		text_draw(right, y + 3, r->hp, PET_WHITE, TEXT_RIGHT);
		if (r->layer[0]) text_draw(right - text_width(r->hp) - 8, y + 3, r->layer, PET_CYAN_HI, TEXT_RIGHT);
		dive_crew_view_bar(x + 2, y + ROW - 2, w - 2, r);
	}
	int used = S2.ncrew * (ROW + GAP) + GAP;
	return (SDL_Rect){ body.x, body.y + used, body.w, body.h - used };
}
