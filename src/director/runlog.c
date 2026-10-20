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
	char line[320];
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
	/* (the zenny and BugFrags on hand: the economy's course over a run,
	 * docs/ECONOMY.md) */
	int n = snprintf(L.line, sizeof L.line, "seed %08x depth %d area %d %s hp %d/%d z %u bf %u", (unsigned)run.seed, run.depth,
		run.biome, kind, megaman_hp(), megaman_max(), (unsigned)emu_read32(BN6_ZENNY), (unsigned)emu_read32(BN6_BUGFRAGS));
	L.guardian = 0;
	L.duel = !strcmp(kind, "duel");
	/* (an older net's record fought in BN6's engine: "x" and its address
	 * there, docs/MULTIROM.md; a visitor of the other net "+" before it) */
	if (e && e->xrecord) n += snprintf(L.line + n, sizeof L.line - (size_t)n, " x%07X", (unsigned)(e->xrecord & 0xFFFFFFFu));
	int total = 0;
	for (int i = 0; e && i < e->nfoes && n < (int)sizeof L.line - 16; ++i) {
		const Foe *f = &e->foes[i];
		int hp, dmg;
		if (f->kind == FOE_ROCK || !enemy_stats(f->id, &hp, &dmg)) continue;
		total += hp;
		n += snprintf(L.line + n, sizeof L.line - (size_t)n, " %s%c%d.%d", e->visitor == i + 1 ? "+" : "", f->kind == FOE_NAVI ? 'n' : 'v', f->family,
			f->version);
	}
	if (e && n < (int)sizeof L.line - 16) n += snprintf(L.line + n, sizeof L.line - (size_t)n, " foehp %d", total);
	L.len = n < (int)sizeof L.line ? n : (int)sizeof L.line - 1;
	L.open = true;
}

void runlog_guardian_start(int navi, const Encounter *e, int hp) {
	runlog_battle_start(NULL, "guardian");
	L.guardian = navi;
	/* (his number here, and his foe as the engine has it: BN5's by its
	 * family moved in, docs/MULTIROM.md) */
	if (e && e->nfoes > 0)
		L.len += snprintf(L.line + L.len, sizeof L.line - (size_t)L.len, " g%d n%d.%d foehp %d", navi, e->foes[0].family, e->foes[0].version, hp);
	if (L.len >= (int)sizeof L.line) L.len = (int)sizeof L.line - 1;
}

void runlog_battle_souls(uint32_t used) {
	/* (each Soul by BN5's number, Team Colonel's 7-12; "c" its Chaos Unison) */
	for (int n = 7; L.open && n <= 12 && L.len < (int)sizeof L.line - 12; ++n)
		if (used >> n & 1)
			L.len += snprintf(L.line + L.len, sizeof L.line - (size_t)L.len, " unite %d%s", n, used >> (16 + n) & 1 ? "c" : "");
}

/* (a guardian's battle over, won, left or MegaMan deleted: the statistics'
 * event, MegaMan's HP as the battle left it) */
static void guardian_over(int result, int frames) {
	if (L.guardian) analytics_guardian(L.guardian, result, megaman_hp(), megaman_max(), frames);
}

void runlog_battle_end(bool won, int frames) {
	if (!L.open) return;
	L.open = false;
	/* (a win's busting level and the reward it picked, BN6's encoding:
	 * docs/ECONOMY.md's battle drops) */
	if (won)
		snprintf(L.line + L.len, sizeof L.line - (size_t)L.len, " -> won hp %d bust %d reward %04x", megaman_hp(), emu_read8(BN6_BATTLE_BUSTING),
			emu_read16(BN6_BATTLE_REWARD));
	else snprintf(L.line + L.len, sizeof L.line - (size_t)L.len, " -> left hp %d", megaman_hp());
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
