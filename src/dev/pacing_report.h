/* The dev pacing report: every act's random battles and guardians against
 * their bands, without the game (pacing_report.c, docs/DEVTOOLS.md). */
#ifndef CW_PACING_REPORT_H
#define CW_PACING_REPORT_H

/* Writes the report to `path`; 0 when written. */
int pacing_report_run(const char *path);

#endif
