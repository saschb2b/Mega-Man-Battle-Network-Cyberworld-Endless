/* title_version.h. Drawing changes nothing. */
#include "title_version.h"

#include <stdio.h>
#include <string.h>

#include "gfx.h"
#include "minifont.h"
#include "platform.h"
#include "save.h"
#include "update.h"
#include "update_text.h"
#include "version.h"   /* CW_VERSION, from the build (Makefile) */

#define LINE_Y 138   /* the line's top in the picture: over the copyright */

void title_version_tick(void) { update_tick(); }

/* The build, for a report (v0.5.0 beta, v0.5.0+12 a dozen commits on; 0.1
 * to 0.4 were alphas) */
static const char *version_words(char *v, size_t n) {
	const char *cw = CW_VERSION;
	if (!strncmp(cw, "dev", 3) || !strncmp(cw, "0.0.1+git", 9)) snprintf(v, n, "dev");
	else {
		const char *g = strstr(cw, ".g");
		snprintf(v, n, "v%.*s%s", g ? (int)(g - cw) : (int)strlen(cw), cw, !strncmp(cw, "0.", 2) ? " beta" : "");
	}
	return v;
}

void title_version_draw(int x0, int y0, int saved_depth, bool best) {
	char v[40];
	minifont_draw(x0 + 4, y0 + LINE_Y, version_words(v, sizeof v), rgba(150, 160, 190, 255), 1);
	/* (a newer version found, beside it, in the PET's green: Options has its line) */
	if (update_version()[0]) {
		int at = x0 + 4 + minifont_width(v, 1) + 6;
		snprintf(v, sizeof v, "%s v%s", update_word(UW_NEW), update_version());
		minifont_draw(at, y0 + LINE_Y, v, rgba(120, 230, 140, 255), 1);
	}
	int depth = profile.best_depth > saved_depth ? profile.best_depth : saved_depth;
	if (depth > 0 && best) {
		snprintf(v, sizeof v, "Best: Layer %d", depth);
		minifont_draw(x0 + CORE_W - 4 - minifont_width(v, 1), y0 + LINE_Y, v, rgba(255, 230, 90, 255), 1);
	}
}
