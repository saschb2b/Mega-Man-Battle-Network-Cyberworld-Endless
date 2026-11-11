/* The battle data on virus families: the run's tally and the profile's
 * counts (battle_data.h; docs/game/battle-lab.md, The gauge). */
#include "battle_data.h"

#include <string.h>

#include "save.h"
#include "save_blob.h"

#define DATA_MAGIC 0x43574431u   /* "CWD1": run.data, a run's deletions by family */

static DataTally T;

static int sum255(int a, int b) { return a + b > 255 ? 255 : a + b; }

static bool known(int family) { return family > 0 && family < DATA_FAMILIES; }

/* A tally into the profile: how many families it completes */
static int fold(const DataTally *t) {
	int completed = 0;
	for (int f = 1; f < DATA_FAMILIES; ++f) {
		if (!t->count[f] && !t->versions[f]) continue;
		int was = profile.data_count[f], now = sum255(was, t->count[f]);
		if (was < DATA_COMPLETE && now >= DATA_COMPLETE) ++completed;
		profile.data_count[f] = (uint8_t)now;
		profile.data_versions[f] |= t->versions[f];
	}
	return completed;
}

void battle_data_save(void) { save_write_blob("run.data", DATA_MAGIC, &T, sizeof T); }

void battle_data_begin(uint32_t seed) {
	DataTally left;
	if (save_read_blob("run.data", DATA_MAGIC, &left, sizeof left) && left.seed && left.seed != seed) {
		fold(&left);
		profile_save();
	}
	memset(&T, 0, sizeof T);
	T.seed = seed;
	/* (at once: the tally left is counted, and not to be read again) */
	battle_data_save();
}

void battle_data_load(uint32_t seed) {
	/* (none, a run saved before its tally: from here on) */
	if (!save_read_blob("run.data", DATA_MAGIC, &T, sizeof T) || T.seed != seed) {
		memset(&T, 0, sizeof T);
		T.seed = seed;
	}
}

void battle_data_deleted(int family, int version) {
	if (!known(family) || version < 0 || version >= DATA_VERSIONS) return;
	T.count[family] = (uint8_t)sum255(T.count[family], 1);
	T.versions[family] |= (uint8_t)(1u << version);
}

int battle_data_run_end(void) {
	int completed = fold(&T);
	memset(&T, 0, sizeof T);
	battle_data_save();
	return completed;
}

int battle_data_count(int family) { return known(family) ? sum255(profile.data_count[family], T.count[family]) : 0; }

unsigned battle_data_versions(int family) { return known(family) ? (unsigned)(profile.data_versions[family] | T.versions[family]) : 0; }

bool battle_data_complete(int family) { return battle_data_count(family) >= DATA_COMPLETE; }

int battle_data_families_complete(void) {
	int n = 0;
	for (int f = 1; f < DATA_FAMILIES; ++f) n += battle_data_complete(f);
	return n;
}
