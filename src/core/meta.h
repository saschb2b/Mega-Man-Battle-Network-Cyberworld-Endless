/* The meta layer (docs/META.md): what a run leaves for the next one.
 * Options, never power: starting folders, threat rungs, the endless net. */
#ifndef META_H
#define META_H

#include <stdbool.h>

enum { FOLDER_STANDARD, FOLDER_BLADE, FOLDER_STORM, FOLDER_COUNT };

typedef struct {
	const char *name;
	const char *about;   /* its style and its cost, a line */
	int navi;            /* the guardian whose deletion opens it, 0 none */
} FolderInfo;

const FolderInfo *meta_folder(int folder);
bool meta_folder_open(int folder);
bool meta_endless_open(void);
/* The highest threat rung a new run may take. */
int meta_threat_open(void);
/* What rung `rung` (1-THREAT_MAX) adds, a line. */
const char *meta_threat_rule(int rung);
/* Whether the run's threat reaches rung `rung`: each adds to those below. */
bool meta_threat(int rung);

/* At a run's end, won or lost: the profile's unlocks from what the run and
 * the ones before it did, and the summary's lines for them. */
void meta_run_over(bool won);
/* What the run just over unlocked, up to `max` lines; how many. */
int meta_unlocked(const char **out, int max);
/* The closest unlock still ahead, a line, or NULL. */
const char *meta_next_goal(void);

#endif
