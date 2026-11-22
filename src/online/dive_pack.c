/* dive_pack.h */
#include "dive_pack.h"

#include <stdio.h>

static DiveDivers divers;
static WeekSplits records;
static bool heard;

void dive_pack_divers(const DiveDivers *d) {
	divers = *d;
	heard = true;
}

void dive_pack_splits(const WeekSplits *s) {
	records = *s;
	fprintf(stderr, "dive: the week's split records (week %u)\n", s->week);
}

const DiveDivers *dive_pack_now(void) { return &divers; }
bool dive_pack_heard(void) { return heard; }
const WeekSplits *dive_pack_records(void) { return &records; }
