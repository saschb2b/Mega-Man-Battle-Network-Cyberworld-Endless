/* One line per battle and one per finished run, appended to runlog.txt
 * beside the saves; past 512 KB the log moves to runlog.old. */
#include "runlog.h"

#include <stdio.h>
#include <string.h>

#include "analytics.h"
#include "bn6.h"
#include "compat.h"
#include "data.h"
#include "emu.h"
#include "game.h"
#include "guardians.h"
#include "platform.h"
#include "run.h"

#define LOG_LIMIT (512 * 1024)

static struct {
	bool open;
	char line[256];
	int len;
	int guardian;    /* the open battle's guardian (navi), 0 none */
	bool duel;       /* ... or ProtoMan's duel */
} L;

static int megaman_hp(void) { return emu_read16(BN6_NAVI_HP); }
static int megaman_max(void) { return emu_read16(BN6_NAVI_MAX_HP); }

static void append(const char *line) {
	char path[600], old[600];
	snprintf(path, sizeof path, "%s/runlog.txt", g_data_dir);
	FILE *f = fopen(path, "a");
	if (!f) return;
	fprintf(f, "%s\n", line);
	long size = ftell(f);
	fclose(f);
	platform_persist();
	if (size > LOG_LIMIT) {
		snprintf(old, sizeof old, "%s/runlog.old", g_data_dir);
		remove(old);
		cw_rename(path, old);
	}
}

void runlog_battle_start(const Encounter *e, const char *kind) {
	int n = snprintf(L.line, sizeof L.line, "seed %08x depth %d area %d %s hp %d/%d", (unsigned)run.seed, run.depth,
		run.biome, kind, megaman_hp(), megaman_max());
	L.guardian = 0;
	L.duel = !strcmp(kind, "duel");
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

void runlog_guardian_start(int navi) {
	runlog_battle_start(NULL, "guardian");
	L.guardian = navi;
}

/* (a guardian's battle over, won, left or MegaMan deleted: the statistics'
 * event, MegaMan's HP as the battle left it) */
static void guardian_over(int result, int frames) {
	if (L.guardian) analytics_guardian(L.guardian, result, megaman_hp(), megaman_max(), frames);
}

void runlog_battle_end(bool won, int frames) {
	if (!L.open) return;
	L.open = false;
	snprintf(L.line + L.len, sizeof L.line - (size_t)L.len, " -> %s hp %d", won ? "won" : "left", megaman_hp());
	append(L.line);
	guardian_over(won ? ANALYTICS_WON : ANALYTICS_LEFT, frames);
}

/* (a battle as BN6's lines have it, "guest" and its record after the HP,
 * its enemies as "x" and that game's id) */
static void guest_start_line(const char *kind, uint32_t record, const int *ids, int n, int foehp) {
	int k = snprintf(L.line, sizeof L.line, "seed %08x depth %d area %d %s hp %d/%d guest %08x", (unsigned)run.seed, run.depth, run.biome, kind,
		megaman_hp(), megaman_max(), (unsigned)record);
	for (int i = 0; i < n && k < (int)sizeof L.line - 16; ++i) k += snprintf(L.line + k, sizeof L.line - (size_t)k, " x%d", ids[i]);
	if (k < (int)sizeof L.line - 16) k += snprintf(L.line + k, sizeof L.line - (size_t)k, " foehp %d", foehp);
	L.len = k < (int)sizeof L.line ? k : (int)sizeof L.line - 1;
	L.open = true;
	L.guardian = 0;
	L.duel = false;
}

void runlog_guest_start(uint32_t record, const int *ids, int n, int foehp) { guest_start_line("battle", record, ids, n, foehp); }

void runlog_guest_guardian_start(uint32_t record, int navi, int id, int foehp) {
	guest_start_line("guardian", record, &id, 1, foehp);
	L.guardian = navi;
}

void runlog_guest_end(bool won, const char *reward, int frames) {
	if (!L.open) return;
	L.open = false;
	snprintf(L.line + L.len, sizeof L.line - (size_t)L.len, " -> %s hp %d%s%s", won ? "won" : "left", megaman_hp(), won ? " reward " : "", won ? reward : "");
	append(L.line);
	guardian_over(won ? ANALYTICS_WON : ANALYTICS_LEFT, frames);
}

void runlog_run_end(bool won) {
	/* (who deleted MegaMan, for the statistics: the battle he was in) */
	const char *by = !L.open ? "viruses" : L.guardian ? guardian(L.guardian)->name : L.duel ? "ProtoMan" : "viruses";
	if (L.open) {
		L.open = false;
		snprintf(L.line + L.len, sizeof L.line - (size_t)L.len, " -> deleted");
		append(L.line);
		if (!won) guardian_over(ANALYTICS_LOST, 0);
	}
	char line[128];
	snprintf(line, sizeof line, "seed %08x run %s at depth %d area %d viruses %d navis %d", (unsigned)run.seed,
		won ? "won" : "over", run.depth, run.biome, run.viruses_deleted, run.bosses_beaten);
	append(line);
	analytics_run_end(won, by);
}
