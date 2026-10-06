/* Run checkpoints and the permanent profile, stored beside the game. */
#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>
#include <stddef.h>

#include "run.h"

typedef struct {
	int runs;
	int best_depth;
	int last_depth;   /* where the last run ended (was an unused score) */
	int bosses;
	int viruses;
	int secret_clears;
	int nest_clears;
	int music_volume; /* 0-10, stored +1 so a fresh profile reads as default */
	int sfx_volume;
	bool seen_intro;
	/* (fields after here are new since profiles began: an older, shorter
	 * profile reads them as 0) */
	uint8_t first_guardian;   /* the last new run's act 1 guardian, navi + 1 */
	uint8_t navicust_taught;  /* a Guardian Data's draft has said how the NaviCust's board works */
	uint8_t marks_taught;     /* the map's violet marks and its counters L has explained (MARK_*) */
	/* the meta layer (docs/META.md): the highest threat rung open to a new
	 * run, the starting folders unlocked (a bit each), short nets won */
	uint8_t threat_open;
	uint16_t folders_open;
	uint16_t short_wins;
	uint8_t last_net, last_folder, last_threat, last_helpers;   /* the setup screen starts where the last run's did */
	uint16_t marks;           /* the title's marks earned (MARK_*, BN6's own bits: meta.h) */
	uint8_t first_area;       /* the last new run's act 1 area, biome + 1 */
	uint8_t crosses_open;     /* the Cross starts announced (a bit per navi 1-5, meta.h) */
	uint8_t last_cross;
	/* the Library (docs/META.md): a bit per chip id held in any run, and
	 * how many it held as the current run began */
	uint8_t library[40];
	uint16_t library_start;
	uint8_t last_town;        /* (unused since Central Town is home, docs/HOME.md: the last new run's town, style + 1) */
	uint8_t programs_found[8];   /* NaviCust programs MegaMan has had in any run, a bit each (docs/NAVICUST.md, 7) */
	uint32_t library_run;     /* the run library_start was counted for (its seed) */
	uint8_t setup_new;        /* the setup's rows with an option the last summary announced (SETUP_NEW_*, meta.h) */
	uint8_t area_before, guardian_before;   /* the run before the last one's first area and guardian (+ 1) */
	/* the virus families MegaMan has battled in any run, a bit each: the
	 * PET's battle data on them, which his warnings read (a guardian's is
	 * its rivals.sav record) */
	uint32_t families_fought[2];
	uint8_t gem_taught;       /* MegaMan has said what a Mystery Data on the battlefield is */
	/* the NaviCust's Spins found in the net, a bit per colour 1-6 (bit
	 * c - 1: docs/META.md), and the run that found one (its seed) with that
	 * colour, so that a run finds at most one */
	uint8_t spins;
	uint8_t spin_colour;
	uint32_t spin_run;
	uint32_t records_mark;    /* the guardians' records as Dad's Records mail last showed them (a hash): NEW when they change */
	/* the rival's duels (docs/RIVAL.md), in any run: won and lost */
	uint16_t duel_won, duel_lost;
	/* the last duel's run (its seed) and layer, and whether it was won: a
	 * CONTINUE from the checkpoint before it finds it fought, not to be
	 * fought and counted again (issue #20) */
	uint32_t duel_run;
	uint16_t duel_depth;
	uint8_t duel_beat;
	uint8_t pieces_taught;    /* the set pieces L has explained (SENSED_*, briefing_words.c; issue #48) */
	/* the NaviCust programs whose compression code was entered in any run,
	 * a bit each (issue #50: Dad's Compression mail lists them), and how
	 * many that mail showed when last marked NEW */
	uint8_t codes_entered[8];
	uint8_t codes_mailed;
	/* MegaMan has said what Reg memory is for (the first RegUp found) and
	 * what the TagChip system Chaud's first clearance brings does (issue #51) */
	uint8_t reg_taught, tag_taught;
	uint8_t bbs_seen;         /* the Endless Net BBS's posts when its mail was last marked NEW (rumors) */
	uint8_t pack_taught;      /* MegaMan has said a chip bought or traded goes to the Pack */
	uint8_t guest_taught;     /* ... and what an older net's battle is (docs/MULTIROM.md, Guest battles) */
	uint8_t dark_taught;      /* ... and all of a DarkChip's price, at the first flame of darkness (docs/META.md) */
	uint8_t recode_taught;    /* ... and that the older net reads chip codes its own way: 1 going in, 2 coming back */
	uint8_t cross_old_told;   /* ... and, arriving where its battles are, that the older net had no Crosses */
	uint8_t soul_taught;      /* ... and what a Soul of the older net does, at the first Guardian Data that gave one (docs/META.md, Souls) */
	uint8_t dark6_taught;     /* ... and BN6's own DarkChips (docs/META.md): all of a BN6 flame's words, all of their price after a battle (DARK6_*) */
	uint8_t back_taught;      /* ... and what a trip back costs, the Net's clock, at the first older portal (docs/HOME.md) */
	/* how the last run ended, for the town's news (docs/HOME.md, a town
	 * that remembers): the guardian who deleted MegaMan (navi, 0 none), or
	 * the Nest won */
	uint8_t last_lost_to;
	uint8_t last_won;
	uint8_t hp_taught;        /* MegaMan has said what Lan's HP is, at a run's first jack-in (said short after) */
	uint8_t last_job;         /* the asker whose request the last run ended holding, plus one (0 none): their word at the next run's start */
	uint8_t exit_taught;      /* MegaMan has named a layer's exit pad, near one the first time (issue #101) */
} Profile;

enum { DARK6_FLAME_TAUGHT = 1, DARK6_PRICE_TAUGHT = 2 };

enum { MARK_SERVER = 1, MARK_WARP = 2, MARK_GATE = 4, MARK_NAVI_GATE = 8, MARK_VAULT = 16, MARK_COUNTS = 32 /* the Mystery Data counters */ };

extern Profile profile;

/* Chaud's clearance (docs/RIVAL.md): 1 after a first duel won, 2 once
 * ProtoMan himself has been beaten (the third rung, a third win). */
static inline int rival_clearance(void) { return profile.duel_won >= 3 ? 2 : profile.duel_won >= 1 ? 1 : 0; }

/* Whether MegaMan has battled virus family `fam` in any run; noted as a
 * battle with it ends. */
bool profile_family_fought(int fam);
void profile_family_note(int fam);
/* Whether NaviCust program `program`'s compression code has been entered
 * in any run, and how many have (issue #50); noted as the NaviCust
 * compresses it. */
bool profile_code_entered(int program);
int profile_codes_entered(void);
void profile_code_note(int program);

void save_init(void);
bool save_exists(void);
bool save_run(void);
bool load_run(void);
/* The saved run without loading it (none from before the current save format). */
bool peek_run(Run *out);
/* Deletes the run's save and its game state. */
void save_delete(void);
/* Where the run's checkpoint keeps the game's state. */
void save_state_path(char *out, size_t n);
void profile_save(void);
/* A new run from the title: the seed's, or the next seed's where its act 1
 * guardian is the last run's (BlastMan six runs running). */
void run_new_varied(uint32_t seed);
void profile_record_run(void);

#endif
