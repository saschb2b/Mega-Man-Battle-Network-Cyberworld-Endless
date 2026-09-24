/* One line per battle and one per finished run, appended to runlog.txt
 * beside the saves; past 512 KB the log moves to runlog.old. */
#include "runlog.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "data.h"
#include "emu.h"
#include "game.h"
#include "run.h"

#define LOG_LIMIT (512 * 1024)

static struct {
	bool open;
	char line[256];
	int len;
} L;

static int megaman_hp(void) { return emu_read16(BN6_NAVI_STATS + 0x40); }
static int megaman_max(void) { return emu_read16(BN6_NAVI_STATS + 0x42); }

static void append(const char *line) {
	char path[600], old[600];
	snprintf(path, sizeof path, "%s/runlog.txt", g_data_dir);
	FILE *f = fopen(path, "a");
	if (!f) return;
	fprintf(f, "%s\n", line);
	long size = ftell(f);
	fclose(f);
	if (size > LOG_LIMIT) {
		snprintf(old, sizeof old, "%s/runlog.old", g_data_dir);
		remove(old);
		rename(path, old);
	}
}

void runlog_battle_start(const Encounter *e, const char *kind) {
	int n = snprintf(L.line, sizeof L.line, "seed %08x depth %d area %d %s hp %d/%d", (unsigned)run.seed, run.depth,
		run.biome, kind, megaman_hp(), megaman_max());
	int total = 0;
	for (int i = 0; e && i < e->nfoes && n < (int)sizeof L.line - 16; ++i) {
		const Foe *f = &e->foes[i];
		int hp, dmg;
		if (f->kind == FOE_ROCK || !enemy_stats(f->id, &hp, &dmg)) continue;
		total += hp;
		n += snprintf(L.line + n, sizeof L.line - (size_t)n, " %c%d.%d", f->kind == FOE_NAVI ? 'n' : 'v', f->family, f->version);
	}
	if (e && n < (int)sizeof L.line - 16) n += snprintf(L.line + n, sizeof L.line - (size_t)n, " foehp %d", total);
	L.len = n < (int)sizeof L.line ? n : (int)sizeof L.line - 1;
	L.open = true;
}

void runlog_battle_end(bool won) {
	if (!L.open) return;
	L.open = false;
	snprintf(L.line + L.len, sizeof L.line - (size_t)L.len, " -> %s hp %d", won ? "won" : "left", megaman_hp());
	append(L.line);
}

void runlog_run_end(void) {
	if (L.open) {
		L.open = false;
		snprintf(L.line + L.len, sizeof L.line - (size_t)L.len, " -> deleted");
		append(L.line);
	}
	char line[128];
	snprintf(line, sizeof line, "seed %08x run over at depth %d area %d viruses %d navis %d", (unsigned)run.seed,
		run.depth, run.biome, run.viruses_deleted, run.bosses_beaten);
	append(line);
}
