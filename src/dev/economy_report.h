/* The dev economy report: what a run hands out, layer by layer and act by
 * act, against its bands, without the game (economy_report.c,
 * docs/ECONOMY.md, docs/DEVTOOLS.md). */
#ifndef CW_ECONOMY_REPORT_H
#define CW_ECONOMY_REPORT_H

/* Writes the report to `spec`'s path (PATH[:RUNS]); 0 when written. */
int economy_report_run(const char *spec);

#endif
