/* The atlas's reports that draw nothing: specs of `--atlas DIR:SPEC:SEEDS`
 * that build layers as runs would and count what they hold
 * (docs/devtools/atlas.md). */
#include "atlas_reports.h"

#include <string.h>

#include "layer_sizes.h"
#include "pieces_census.h"

int atlas_report_run(const char *dir, const char *spec, int seeds) {
	if (!strncmp(spec, "sizes", 5)) return layer_sizes_run(dir, spec + 5, seeds);
	if (!strcmp(spec, "pieces")) return pieces_census_run(dir, seeds);
	return -1;
}
