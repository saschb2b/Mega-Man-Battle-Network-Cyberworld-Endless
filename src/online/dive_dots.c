/* dive_dots.h. The room's divers kept from the snapshot as it comes, and
 * read by the map as it is drawn: neither changes the game. */
#include "dive_dots.h"

#include <SDL.h>

#include "bn6_fields.h"
#include "crew_look.h"
#include "director.h"
#include "dive_crew.h"
#include "map_divers.h"
#include "netmap.h"
#include "run.h"

/* a diver outside the crew: the PET's rails' grey, dimmer than MegaMan's white */
#define DIVER_COLOUR LOOK_METAL
#define STALE_MS 500   /* a snapshot of the room this old shows no one */

static struct {
	OnlinePeer peer[ONLINE_MAX_PLAYERS];
	int n;
	uint32_t seed;   /* the layer they were seen on */
	int depth;
	uint32_t at;     /* the ticks it was seen at */
} T;

static bool here(void) {
	return T.n && SDL_GetTicks() - T.at < STALE_MS && director_on_layer() && T.seed == run.layer_seed && T.depth == run.depth;
}

static int lend(MapDiver *out, int most) {
	int n = 0;
	if (!here()) return 0;
	for (int i = 0; i < T.n && n < most; ++i) {
		bool crew = dive_crew_has(T.peer[i].id);
		out[n++] = (MapDiver){ (int)T.peer[i].x, (int)T.peer[i].y, crew, crew ? CREW_COLOUR : DIVER_COLOUR };
	}
	return n;
}

void dive_dots_on(bool on) {
	map_divers = on ? lend : NULL;
	T.n = 0;
}

void dive_dots_frame(const OnlineSnapshot *s) {
	T.n = 0;
	if (!s->connected || !director_on_layer()) return;
	T.seed = run.layer_seed;
	T.depth = run.depth;
	T.at = SDL_GetTicks();
	for (size_t i = 0; i < s->peer_count && T.n < ONLINE_MAX_PLAYERS; ++i) T.peer[T.n++] = s->peers[i];
}

void dive_dots_describe(FILE *f) {
	MapDiver d[MAP_DIVERS];
	int n = map_divers ? map_divers(d, MAP_DIVERS) : 0, mx, my, x, y;
	fprintf(f, "mapdots %d", n);
	netmap_panel(bn6_player_x(), bn6_player_y(), &mx, &my);
	for (int i = 0; i < n; ++i) {
		netmap_panel(d[i].x, d[i].y, &x, &y);
		fprintf(f, "%s%s(%d,%d)%s", i ? " " : ": ", T.peer[i].name, x - mx, y - my, d[i].crew ? "*" : "");
	}
	fputc('\n', f);
}
