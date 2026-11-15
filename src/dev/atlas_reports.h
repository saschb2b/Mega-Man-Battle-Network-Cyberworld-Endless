/* The atlas's reports that draw nothing (atlas_reports.c). */
#ifndef CW_ATLAS_REPORTS_H
#define CW_ATLAS_REPORTS_H

/* A spec of the atlas's that counts rather than draws: `sizes[=AREAS]`
 * the layers' sizes against their regions (layer_sizes.c), `pieces` the
 * set pieces runs meet (pieces_census.c), over `seeds` runs into DIR.
 * Its exit code, or -1 where `spec` is neither. */
int atlas_report_run(const char *dir, const char *spec, int seeds);

#endif
