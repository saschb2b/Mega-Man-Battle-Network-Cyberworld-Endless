/* The meta layer (docs/META.md): what a run leaves for the next one.
 * Options, never power: starting folders, threat rungs, the endless net. */
#ifndef META_H
#define META_H

#include <stdbool.h>
#include <stdint.h>

enum { FOLDER_STANDARD, FOLDER_BLADE, FOLDER_STORM, FOLDER_COUNT };

typedef struct {
	const char *name;
	const char *about;   /* its style and its cost, a line */
	const char *opens;   /* how it opens ("delete any guardian"), NULL: open from the start */
} FolderInfo;

const FolderInfo *meta_folder(int folder);
/* A folder's 30 chips as BN6 keeps them (chip id | code << 9, code 26 for
 * *), or NULL for the Standard folder, which is the game's own. */
const uint16_t *meta_folder_chips(int folder);
bool meta_folder_open(int folder);
/* Whether a run may bring `navi`'s Cross (HeatMan 1 .. ChargeMan 5): open
 * once that navi has been deleted as a guardian, in any run. */
bool meta_cross_open(int navi);
bool meta_endless_open(void);
/* The highest threat rung a new run may take. */
int meta_threat_open(void);
/* What rung `rung` (1-THREAT_MAX) adds, a line. */
const char *meta_threat_rule(int rung);
/* Whether the run's threat reaches rung `rung`: each adds to those below. */
bool meta_threat(int rung);

/* The title's marks, BN6's own (its GetTitleScreenIconCount bits, drawn
 * with its sprites): each for a milestone here, never power. */
enum {
	MARK_THREAT = 0x02,   /* the green disc: a win on the top threat rung */
	MARK_PA = 0x04,       /* P.A. COMP (not yet earned here) */
	MARK_SECRET = 0x08,   /* the S: the Secret Area cleared */
	MARK_GIGA = 0x10,     /* GIGA COMP, MEGA COMP, STD COMP: a Library class complete (meta_library_count) */
	MARK_MEGA = 0x20,
	MARK_STD = 0x40,
	MARK_WIN = 0x80,      /* Gregar: a short net won (BN6's for its ending) */
	MARK_NEST = 0x100,    /* Bass in Gregar's form: the endless net's own Nest cleared */
};

/* The profile's Library: every chip MegaMan has held, in any run (BN6's
 * own Library, which each run's game is given). */
bool meta_library_has(int id);
/* Adds chip `id`; whether it was new. */
bool meta_library_add(int id);
/* The Library's chips of a class (0 standard, 1 Mega, 2 Giga; -1 all), and
 * how many the current run added. */
int meta_library_count(int cls);
int meta_library_new(void);
/* How many chips a collector's vault at `depth` wants in the Library
 * (docs/META.md, gates): more in each act. */
int meta_vault_need(int depth);

/* A new run's start: what earlier runs opened is no news on its summary
 * (a folder earned in a run that never reached one was announced late, as
 * if the run after had opened it). */
void meta_run_begun(void);
/* At a run's end, won or lost: the profile's unlocks from what the run and
 * the ones before it did, and the summary's lines for them. */
void meta_run_over(bool won);
/* The marks the run just over earned (MARK_* bits): the title shows them
 * arriving. */
uint16_t meta_marks_new(void);
/* What the run just over unlocked, up to `max` lines; how many. */
int meta_unlocked(const char **out, int max);
/* The closest unlock still ahead, a line, or NULL. */
const char *meta_next_goal(void);

#endif
