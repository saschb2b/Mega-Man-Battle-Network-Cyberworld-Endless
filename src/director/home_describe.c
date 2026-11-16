/* Home in words (director_home.h's home_describe and home_describe_pos):
 * the lines a headless run's state has at home, read by tests/scenarios
 * (docs/devtools/scenarios.md, docs/devtools/agents-tools.md). They only
 * read: nothing here changes the game or the run. */
#include "director_home.h"

#include <stdio.h>
#include <stdlib.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "director.h"
#include "director_state.h"
#include "director_way.h"
#include "emu.h"
#include "flags.h"
#include "home_places.h"
#include "job_words.h"
#include "lanhp.h"
#include "run.h"
#include "shop.h"
#include "town.h"

void home_describe(FILE *f) {
	static const char *const hours[] = { "morning", "afternoon", "evening", "night" };
	home_jobs_describe(f);
	if (!D.town) return;
	int on = 0, back = 0;
	bool sealed = false;
	home_ways(&on, &sealed);
	for (unsigned lit = home_lit_back(); lit; lit >>= 1) back += lit & 1;
	/* (BN6's map he stands on, its group and number: the Academy's rooms share a place's name) */
	fprintf(f, "at %02x:%02x\nportals on %d back %d\nahead layer %d\n", emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER), on, back, run.depth);
	/* (whether BN6 holds MegaMan to be in the PET: where it does not, R at a jack-in answers "MegaMan isn't in the PET...", issue #682) */
	fprintf(f, "megaman %s\n", flag_get(BN6_FLAG_NAVI_IN_PET) ? "in" : "out");
	if (run.clock) fprintf(f, "clock %d\n", run.clock);
	fprintf(f, "order %s\ncourier %s\nhour %s\n", flag_get(HOME_ORDER_FLAG) ? "made" : "open",
		flag_get(LANHP_COURIER_GONE_FLAG) ? "gone" : flag_get(LANHP_COURIER_TOLD_FLAG) ? "heard" : "waiting", hours[home_hour() & 3]);
	home_lab_describe(f);
}

static void print_near(int id, int x, int y, void *ctx) {
	int px = bn6_player_x(), py = bn6_player_y();
	if (abs(x - px) < 48 && abs(y - py) < 48) fprintf(ctx, "near %s %d %d\n", id < 0 ? "folk" : "object", x, y);
}

void home_describe_pos(FILE *f) {
	int group = emu_read8(BN6_MAP_GROUP), number = emu_read8(BN6_MAP_NUMBER), x, y;
	fprintf(f, "map %02x:%02x\n", group, number);
	/* (the jobs' flags, job_words.h: taken 0-2, held, settled; the run's job) */
	fprintf(f, "jobs %d%d%d held %d settled %d job %d from %d state %d got %d\n", flag_get(JOB_TAKE_FLAG), flag_get(JOB_TAKE_FLAG + 1),
		flag_get(JOB_TAKE_FLAG + 2), flag_get(JOB_HELD_FLAG), flag_get(JOB_SETTLED_FLAG), run.job.kind, run.job.asker, run.job.state, run.job.got);
	if (home_places_way(group, number, &x, &y)) fprintf(f, "goal %d %d\n", x, y);
	else if (director_in_town()) { town_objects(print_near, f); fprintf(f, "port %d %d\n", town_info()->port_x, town_info()->port_y); }
	/* (each lit portal's spot in Lan's HP, to place MegaMan by) */
	for (int k = 0; k < LANHP_PORTALS && home_in_hp(); ++k) {
		lanhp_portal_spot(k, &x, &y);
		if (home_lit() >> k & 1) fprintf(f, "portal %d %s at %d %d\n", k, home_lit_back() >> k & 1 ? "back" : "on", x, y);
	}
	/* (the arrow's way, as on a layer: a playtester's arrow turning at Lan's
	 * HP's wall was read from pictures alone, session 76) */
	goal_way();
	fprintf(f, "way %d %s\n", way_last(), ways[way_last()]);
}
