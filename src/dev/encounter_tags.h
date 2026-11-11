/* The encounter groups' measuring dump (encounter_tags.c, --encounter-tags;
 * tools/encounter_tags.py reads it; docs/game/progression/encounter-groups.md):
 * every virus family a run's engine holds, of every ROM read, as numbers. */
#ifndef CW_ENCOUNTER_TAGS_H
#define CW_ENCOUNTER_TAGS_H

/* Writes it to `path`, the core started and the families installed first
 * (as a run's core starts); 0 when written. */
int encounter_tags_run(const char *path);

#endif
