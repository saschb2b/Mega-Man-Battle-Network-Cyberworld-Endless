/* pick_fit.h. Each program a run can offer (1-63, those MegaMan has words
 * for) and each chip of the ten-bit ids whose record has a name (BN6's,
 * and BN5's and BN4's where their ROMs are read), its information box's
 * text wrapped as the box wraps it: those past its lines scroll there. */
#include "pick_fit.h"

#include <stdio.h>
#include <string.h>

#include "chips.h"
#include "data.h"
#include "navicust.h"
#include "reward_pick.h"

#define FIT_MOST 16   /* lines counted apart; past them, in the last */

typedef struct {
	int by_lines[FIT_MOST + 1];
	int texts, over, worst;
	char worst_name[24];
} FitTally;

static void tally(FitTally *t, const char *kind, int id, const char *name, const char *text, bool all) {
	int lines = reward_pick_info_lines(text), room = reward_pick_info_room();
	++t->texts;
	++t->by_lines[lines < FIT_MOST ? lines : FIT_MOST];
	if (lines > t->worst) {
		t->worst = lines;
		snprintf(t->worst_name, sizeof t->worst_name, "%s", name);
	}
	if (lines > room) ++t->over;
	if (all || lines > room) printf("  %s %d %s: %d lines%s\n", kind, id, name, lines, lines > room ? ", scrolls" : "");
}

int pick_fit_run(const char *spec) {
	bool all = spec && !strcmp(spec, "all");
	FitTally t = { 0 };
	char text[256], name[24];
	printf("pick-fit: the information box holds %d lines whole\n", reward_pick_info_room());
	for (int p = 1; p < 64; ++p) {
		if (!navicust_about(p)) continue;
		reward_pick_info_text(PICK_PROGRAM, p, text, sizeof text);
		program_name(p, name, sizeof name);
		tally(&t, "program", p, name, text, all);
	}
	for (int id = 1; id < CHIPS_IDS; ++id) {
		if (!chips_record(id)) continue;
		chips_name(id, name, sizeof name);
		if (!name[0]) continue;
		reward_pick_info_text(PICK_CHIP, id, text, sizeof text);
		if (!text[0]) continue;
		tally(&t, "chip", id, name, text, all);
	}
	printf("pick-fit: %d texts, by lines:", t.texts);
	for (int k = 0; k <= FIT_MOST; ++k)
		if (t.by_lines[k]) printf(" %d:%d", k, t.by_lines[k]);
	printf("; the longest %s, %d lines; %d scroll\n", t.worst_name, t.worst, t.over);
	return 0;
}
