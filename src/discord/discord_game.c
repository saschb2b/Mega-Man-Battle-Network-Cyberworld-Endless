/* discord_game.h: the game's facts for discord_text.c's words. */
#include "discord_game.h"

#include <string.h>
#include <time.h>

#include "director.h"
#include "discord.h"
#include "discord_text.h"
#include "game.h"
#include "guardians.h"
#include "lab_battle.h"
#include "net.h"
#include "online_visit.h"
#include "run.h"
#include "save.h"
#include "title_setup.h"
#include "title_summary.h"
#include "version.h"

#define EVERY 30   /* frames between two reads of the game: twice a second */

/* The layer's area by its name in the PET ("Sky Area"); a side layer's
 * own (the Undernet, the Secret Area) */
static const char *area(void) {
	int biome = run.side_kind == LAYER_UNDERNET ? BIOME_UNDERNET : run.side_kind == LAYER_SECRET ? BIOME_SECRET : run.biome;
	return guardian_area_name(biome);
}

/* What MegaMan does in the run's game */
static DiscordDoing doing(int *foe) {
	*foe = director_guardian_battle();
	if (lab_battle_on()) return DISCORD_LAB;
	if (*foe) return DISCORD_GUARDIAN;
	if (director_dueling()) return DISCORD_DUEL;
	return director_in_battle() ? DISCORD_BATTLE : DISCORD_WALKING;
}

/* The game scene: the Net Square, home, a layer (before the run's
 * director has a layer, BN6's own start: the menus) */
static void game_facts(DiscordFacts *f) {
	int foe = 0;
	if (online_visit_active()) {
		f->place = DISCORD_SQUARE;
		f->doing = online_visit_held() ? DISCORD_NETBATTLE : DISCORD_WALKING;
		return;
	}
	f->doing = doing(&foe);
	if (director_in_town() || director_in_hp()) {
		f->place = DISCORD_HOME;
		f->layer = run_reached();
		f->home = director_place_name();
		return;
	}
	if (!director_on_layer()) return;
	f->place = DISCORD_LAYER;
	f->layer = run.depth;
	f->last = run.mode == RUN_SHORT ? SHORT_LAYERS + (run.threat >= 10) : 0;
	f->threat = run.threat;
	f->area = area();
	f->foe = foe && guardian(foe) ? guardian(foe)->name : "a guardian";
}

/* The title: its summary of the run just over, the setup, or the title */
static void title_facts(DiscordFacts *f) {
	if (title_summary_shown()) {
		f->place = DISCORD_SUMMARY;
		f->layer = run_reached();
		f->won = title_won;
		f->new_best = title_new_best;
	} else f->place = title_setup_shown() ? DISCORD_SETUP : DISCORD_TITLE;
}

void discord_game_frame(void) {
	static int frames;
	static int64_t start;
	if (!discord_live()) return;
	/* (the time played counts from the game's start: one clock for the
	 * whole session, never reset by a screen) */
	if (!start) start = (int64_t)time(NULL);
	if (frames++ % EVERY == 0) {
		DiscordFacts f = { .place = DISCORD_MENUS, .area = "", .home = "", .foe = "", .version = CW_VERSION };
		f.best = profile.best_depth;
		f.runs = profile.runs;
		const Scene *s = scene_current();
		if (s == &scene_emu) game_facts(&f);
		else if (s == &scene_title) title_facts(&f);
		else if (s == &scene_arena) {
			f.place = DISCORD_SQUARE;
			f.doing = DISCORD_NETBATTLE;
		}
		DiscordActivity a;
		memset(&a, 0, sizeof a);
		discord_words(&f, &a);
		a.start = start;
		discord_show(&a);
	}
	discord_frame();
}

void discord_game_quit(void) { discord_quit(); }
