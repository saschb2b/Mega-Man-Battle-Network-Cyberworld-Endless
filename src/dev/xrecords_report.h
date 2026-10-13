/* The dev listing of another game's battle records in BN6's engine
 * (xrecords_report.c, --xrecords; tools/xrecords.py, build.py xrecords;
 * issue #82): every record of the territories' maps, as xbattles.h
 * translates it, fightable or not and why. */
#ifndef CW_XRECORDS_REPORT_H
#define CW_XRECORDS_REPORT_H

/* Writes it to `path`, the core started and the families installed first
 * (as a run's core starts); 0 when written. */
int xrecords_report_run(const char *path);

#endif
