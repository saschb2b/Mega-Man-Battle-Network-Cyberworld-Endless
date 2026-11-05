/* The run's extras (run.h's EXTRA_*; docs/game/meta/extras.md): which the
 * setup's EXTRAS shows, and the names of those a run left out, for its
 * summary and the anonymous statistics. Their words are extras_text.c's. */
#ifndef CW_EXTRAS_H
#define CW_EXTRAS_H

#include <stdbool.h>
#include <stddef.h>

/* Whether the setup shows extra `e`: every one, but the older nets only
 * where another game's ROM is read, as nothing of theirs comes without */
bool extra_shown(int e);
/* The names of the extras switched off in `off` (1 << EXTRA_* each), apart
 * by `sep`, into buf; how many */
int extras_off_names(char *buf, size_t n, unsigned off, const char *sep);

#endif
