/* The dev pacing report: every act's random battles and guardians against
 * their bands, without the game (pacing_report.c, docs/devtools/). */
#ifndef CW_PACING_REPORT_H
#define CW_PACING_REPORT_H

#include <stdio.h>

/* Writes the report to `path`; 0 when written. */
int pacing_report_run(const char *path);

/* Its part on the encounter groups (pacing_groups.c): each ROM set's
 * pool by act, how many ways each template fills, and group battles
 * rolled against the band, the cap on one hit and the pressure cap. How
 * many lie past a limit. */
int pacing_groups_report(FILE *out);
/* ... and on the acts' arcs (pacing_groups.c; group_arc.h): for every
 * act's areas, runs' arcs rolled by their seeds: how many acts have one,
 * the share with a signature of the area's own, the signatures' variety,
 * each beat and the Server's challenge against its limits. How many lie
 * past one. */
int pacing_arcs_report(FILE *out);

#endif
