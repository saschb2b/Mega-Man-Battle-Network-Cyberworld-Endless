/* The run's end (director_state.h): MegaMan deleted, or the short net
 * won; the summary's facts, the profile's records, the saves, and a dive
 * of the week's net kept for the Board (weekly.h). */
#include "director_state.h"

#include <stdio.h>

#include "director_folder.h"
#include "director_jobs.h"
#include "game.h"
#include "guardians.h"
#include "meta.h"
#include "rivals.h"
#include "runlog.h"
#include "save.h"
#include "weekly.h"

/* where MegaMan was deleted, and by whom, the area named `area` */
static void cause(char *out, size_t n, const char *area) {
	if (D.lost_to) snprintf(out, n, "by %s in %s", guardian(D.lost_to)->name, area);
	else if (D.lost_duel) snprintf(out, n, "in ProtoMan's duel");
	else snprintf(out, n, "in %s", area);
}

void end_run(void) {
	/* what the summary tells: where, and by whom; and with the area's short
	 * name, for a line the whole does not fit ("by TomahawkMan in Robot
	 * Control Comp" ran off the summary's panel) */
	int b = run.side_kind == LAYER_UNDERNET ? BIOME_UNDERNET : run.side_kind == LAYER_SECRET ? BIOME_SECRET : run.biome;
	cause(title_cause, sizeof title_cause, guardian_area_in_text(run.biome, run.side_kind));
	cause(title_cause_short, sizeof title_cause_short, guardian_area_short(b));
	/* (the guardian's first battle, lost: its battle data is what the run
	 * leaves for the next briefing) */
	const Rival *rv = D.lost_to ? rival(D.lost_to) : NULL;
	snprintf(title_learned, sizeof title_learned, "%s", rv && rv->megaman_won + rv->navi_won == 1 ? guardian(D.lost_to)->name : "");
	/* (a first run is no record to beat) */
	title_new_best = profile.runs > 0 && run_reached() > profile.best_depth;
	title_won = false;
	profile.last_lost_to = (uint8_t)D.lost_to;
	profile.last_won = 0;
	/* (a request held: its asker's word at the next run, job_words.c) */
	profile.last_job = run.job.kind != JOB_NONE ? (uint8_t)(run.job.asker + 1) : 0;
	home_jobs_md_counted();
	runlog_run_end(false);
	library_from_game();
	programs_from_game();
	/* (a dive of the week's net opens nothing: its own profile's, weekly.h) */
	bool dive = weekly_on();
	if (dive) meta_run_quiet(); else meta_run_over(false);
	profile_record_run();
	save_delete();
	title_dive = 0;
	if (dive) weekly_ended(false);
	run.active = false;
	D.active = false;
	title_summary = true;
	scene_set(&scene_title);
}

/* The short net won: its Nest's guardian fell and MegaMan stepped on its
 * exit. The run ends on the title's summary of a win, and what it opened
 * for the next (docs/META.md). */
void win_run(void) {
	snprintf(title_cause, sizeof title_cause, "on layer %d", run.depth);
	title_cause_short[0] = 0;
	title_learned[0] = 0;
	title_new_best = run.depth > profile.best_depth;
	title_won = true;
	profile.last_lost_to = 0;
	profile.last_won = 1;
	profile.last_job = 0;
	home_jobs_md_counted();
	runlog_run_end(true);
	library_from_game();
	programs_from_game();
	/* (before the clear counts: it names what the win opened; a dive of
	 * the week's net opens nothing) */
	bool dive = weekly_on();
	if (dive) meta_run_quiet(); else meta_run_over(true);
	profile.nest_clears++;
	profile_record_run();
	save_delete();
	title_dive = 0;
	if (dive) weekly_ended(true);
	run.active = false;
	D.active = false;
	title_summary = true;
	scene_set(&scene_title);
}
