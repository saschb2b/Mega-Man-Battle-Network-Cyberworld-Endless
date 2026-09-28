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
	int navi;            /* the guardian whose deletion opens it, 0 none */
} FolderInfo;

const FolderInfo *meta_folder(int folder);
/* A folder's 30 chips as BN6 keeps them (chip id | code << 9, code 26 for
 * *), or NULL for the Standard folder, which is the game's own. */
const uint16_t *meta_folder_chips(int folder);
bool meta_folder_open(int folder);
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
	MARK_GIGA = 0x10,     /* GIGA COMP, MEGA COMP, STD COMP: the Library, later */
	MARK_MEGA = 0x20,
	MARK_STD = 0x40,
	MARK_WIN = 0x80,      /* Gregar: a short net won (BN6's for its ending) */
	MARK_NEST = 0x100,    /* Bass in Gregar's form: the endless net's own Nest cleared */
};

/* At a run's end, won or lost: the profile's unlocks from what the run and
 * the ones before it did, and the summary's lines for them. */
void meta_run_over(bool won);
/* What the run just over unlocked, up to `max` lines; how many. */
int meta_unlocked(const char **out, int max);
/* The closest unlock still ahead, a line, or NULL. */
const char *meta_next_goal(void);

#endif
