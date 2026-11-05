/* A guardian's title card, and BN4's bracket screen between a tournament's
 * rounds (boss_rounds.h). */
#include "boss_rounds.h"

#include <stdio.h>

#include "bn6_fields.h"
#include "bn6_shop.h"
#include "bracket.h"
#include "cinema.h"
#include "debug.h"
#include "emu.h"
#include "encounter.h"
#include "gfx.h"
#include "guardians.h"
#include "net.h"
#include "platform.h"
#include "story_words.h"

static uint32_t opened;      /* the frame the bracket opened at */
static uint32_t enter[4];    /* where the map entered again put MegaMan, before it was made his place */
static const uint32_t ENTER[4] = { BN6_ENTER_X, BN6_ENTER_Y, BN6_ENTER_Z, BN6_ENTER_FACING };

void boss_title(int navi, int version, int round) {
	const Guardian *gd = guardian(navi);
	static const char *const suffix[3] = { "", " EX", " SP" }, *const older[4] = { "", " V2", " V3", " SP" };
	char name[40], top[48];
	/* (BN5's by its own versions' names) */
	if (guardian_older(navi) || guardian_bn4(navi)) snprintf(name, sizeof name, "%s%s", gd->name, older[version < 0 || version > 3 ? 0 : version]);
	else snprintf(name, sizeof name, "%s%s", gd->name, suffix[version < 0 || version > 2 ? 0 : version]);
	snprintf(top, sizeof top, "Guardian of %s", guardian_area_in_text(layer.biome, run.side_kind));
	/* (a tournament's round over his name: docs/game/tournament.md, The card) */
	if (round >= 0) snprintf(top, sizeof top, "%s", bracket_round_label(round));
	cinema_title(top, name, gd->epithet, rgba(gd->r, gd->g, gd->b, 255), 170);
}

void boss_bracket_open(const RunBracket *b, int won) {
	XbrEntrant e[BN4_ENTRANTS];
	bracket_entrants(b, won, e);
	cinema_letterbox(false);
	emu_encounters_hold(true);
	opened = P.frame;
	/* (the map entered again after it brings MegaMan back where he stands,
	 * not to the layer's arrival: measured, he came back at its start) */
	const uint32_t here[4] = { emu_read32(BN6_PLAYER_X), emu_read32(BN6_PLAYER_Y), emu_read32(BN6_PLAYER_Z), emu_read8(BN6_PLAYER_FACING) };
	for (int i = 0; i < 4; ++i) {
		enter[i] = emu_read32(ENTER[i]);
		emu_write32(ENTER[i], here[i]);
	}
	bool shown = xbracket_open(bracket_tournament((run.depth - 1) % CYCLE_LAYERS / 3), e);
	if (emu_debug_on()) fprintf(stderr, "bracket: %d rounds won, %s at frame %u\n", won, shown ? "shown" : "not shown", (unsigned)P.frame);
}

bool boss_bracket_back(int t) {
	uint32_t away = P.frame - opened - (uint32_t)t;
	if (!((away > 30 && t > 20) || t > 240)) return false;
	for (int i = 0; i < 4; ++i) emu_write32(ENTER[i], enter[i]);
	return true;
}
