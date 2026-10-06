/* MegaMan held (issue #23). Two runs continued from their saves came back
 * with MegaMan standing on the map, the pad and START dead while the
 * director's own L, map and dev menu worked: a state written while BN6
 * held him, a hold the CONTINUE's warp into the map does not undo (its
 * conveyor flag, a cutscene's pad, a fade's mark, a chat's flag; bn6f
 * sub_809D9E0 and sub_8005AF4 test them before the pad and START). Every
 * save now waits for him free, and a CONTINUE is watched: held with
 * nothing under way, or the pad pushed with no step taken, he is let go as
 * BN6's own routines let go, and the map entered again (then the layer's
 * start). */
#include "director_hold.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6_fields.h"
#include "boss.h"
#include "cinema.h"
#include "director.h"
#include "director_save.h"
#include "director_state.h"
#include "flags.h"
#include "gamecall.h"
#include "talk.h"

static const char *const names[HELD_COUNT] = {
	"off-map", "fade", "cutscene", "chat", "lock", "script", "conveyor", "npc", "warp", "pet", "paused", "player",
};

unsigned bn6_held(void) {
	unsigned h = 0;
	if (main_mode() != BN6_MODE_GAME || emu_read8(BN6_GAMESTATE) != BN6_SUB_MAP) h |= HELD_OFF_MAP;
	if (emu_read8(BN6_FADE_ACTIVE) == 1) h |= HELD_FADE;
	if (emu_read32(BN6_CUTSCENE_POS)) h |= HELD_CUTSCENE;
	if (emu_read8(BN6_CHATBOX) || (emu_read32(BN6_CHATBOX_FLAGS) & BN6_CHAT_ACTIVE)) h |= HELD_CHAT;
	if (emu_read8(BN6_DIALOGUE_LOCK) || !flag_get(BN6_FLAG_PLAYER_CAN_MOVE)) h |= HELD_LOCK;
	if (emu_read8(BN6_SCRIPTED_WALK)) h |= HELD_SCRIPT;
	if (flag_get(BN6_FLAG_CONVEYOR)) h |= HELD_CONVEYOR;
	if (emu_read8(BN6_NPC_CHAT)) h |= HELD_NPC;
	if (emu_read8(BN6_WARP_PENDING)) h |= HELD_WARP;
	if ((emu_read8(BN6_PET_MENU_OPEN) & 1) || flag_get(BN6_FLAG_PET_OFF)) h |= HELD_PET;
	if (emu_read8(BN6_MAP_PAUSED)) h |= HELD_PAUSED;
	int state = emu_read8(BN6_PLAYER_STATE);
	if (emu_read8(BN6_PLAYER_MAIN) != 4 || (state != 0 && state != 4 && state != 8)) h |= HELD_PLAYER;
	return h;
}

const char *held_names(unsigned held) {
	static char buf[128];
	buf[0] = 0;
	for (int k = 0; k < HELD_COUNT; ++k)
		if (held >> k & 1) snprintf(buf + strlen(buf), sizeof buf - strlen(buf), "%s%s", buf[0] ? " " : "", names[k]);
	return buf[0] ? buf : "none";
}

unsigned held_parse(const char *spec) {
	if (!strcmp(spec, "all")) return (1u << HELD_COUNT) - 1;
	char *end;
	unsigned n = (unsigned)strtoul(spec, &end, 0);
	if (end != spec && !*end) return n;
	unsigned mask = 0;
	char buf[128];
	snprintf(buf, sizeof buf, "%s", spec);
	for (char *t = strtok(buf, ","); t; t = strtok(NULL, ","))
		for (int k = 0; k < HELD_COUNT; ++k)
			if (!strcmp(t, names[k])) mask |= 1u << k;
	return mask;
}

bool hold_save_ok(void) { return !talk_busy() && !bn6_held(); }

/* ---- the watch after a CONTINUE ---- */

#define WATCH_HELD   120   /* frames held from the CONTINUE on, nothing under way, before he is let go */
#define WATCH_PUSHED 180   /* frames the pad pushes with no step taken (a hold bn6_held does not name) */
#define WATCH_FADE   90    /* frames a map's entry waits on a fade's mark no fade runs (a black screen) */
#define WATCH_TRIES  3     /* let go where he stands, then at the layer's start, then the watch ends */

static struct {
	bool armed, was_free;
	int held, pushed, masked, fade, tries;
} W;

void hold_watch_arm(void) {
	memset(&W, 0, sizeof W);
	W.armed = true;
}

/* the director under way with him: a talk, a chat, a guardian's staging,
 * a warp, an arrival's card that holds him */
static bool director_busy(void) {
	return talk_busy() || emu_read8(BN6_CHATBOX) || boss_cinematic() || D.warping || D.arrival_hold;
}

/* BN6's holds let go as its own routines let go of them (bn6f
 * owPlayer_unlockPlayerAfterNonNPCDialogue_809E122, owPlayer_809E114, the
 * conveyor's end in sub_809D9A0, a chat's in sub_809D7D8) */
static void release(void) {
	emu_write8(BN6_DIALOGUE_LOCK, 0);
	flag_set(BN6_FLAG_PLAYER_CAN_MOVE);
	flag_clear(BN6_FLAG_DIALOGUE_1719);
	emu_write8(BN6_SCRIPTED_WALK, 0);
	flag_clear(BN6_FLAG_CONVEYOR);
	flag_clear(BN6_FLAG_PET_OFF);
	emu_write8(BN6_NPC_CHAT, 0);
	emu_write8(BN6_MAP_PAUSED, 0);
	if (!emu_read8(BN6_CHATBOX)) emu_write32(BN6_CHATBOX_FLAGS, emu_read32(BN6_CHATBOX_FLAGS) & ~(uint32_t)BN6_CHAT_ACTIVE);
	if (emu_read8(BN6_SCREEN_FADE) != 1) emu_write8(BN6_FADE_ACTIVE, 0);
	if (cinema_input_mode() != CINEMA_FREE) cinema_input(CINEMA_FREE);
}

/* let go, and the map entered again: a fresh player object, its cutscene
 * and warp ended (bn6f EnterMap) */
static void free_him(void) {
	unsigned h = bn6_held();
	++W.tries;
	fprintf(stderr, "continue: MegaMan held (%s), let go (%d)\n", held_names(h), W.tries);
	release();
	if (D.town)
		emu_warp(emu_read8(BN6_MAP_GROUP), emu_read8(BN6_MAP_NUMBER), bn6_player_x(), bn6_player_y(), emu_read8(BN6_PLAYER_FACING));
	else
		layer_reenter(W.tries > 1);
	W.held = W.pushed = W.masked = 0;
	W.was_free = false;
	if (W.tries >= WATCH_TRIES) W.armed = false;
}

/* a map's entry that waits on a fade's mark with no fade to end it: the
 * mark let go (true while it waits) */
static bool entry_waits(void) {
	if (main_mode() != BN6_MODE_GAME || emu_read8(BN6_GAMESTATE) != BN6_SUB_ENTER || emu_read8(BN6_FADE_ACTIVE) != 1 ||
		emu_read8(BN6_SCREEN_FADE) == 1) {
		W.fade = 0;
		return false;
	}
	if (++W.fade >= WATCH_FADE) {
		fprintf(stderr, "continue: the map's entry waited on a fade, let go\n");
		emu_write8(BN6_FADE_ACTIVE, 0);
		W.fade = 0;
	}
	return true;
}

void director_hold_watch(void) {
	if (!W.armed || !D.active || entry_waits()) return;
	if (!on_map() || director_busy()) {
		W.held = W.pushed = W.masked = 0;
		return;
	}
	unsigned h = bn6_held();
	bool stepped = (emu_read16(BN6_PLAYER_PAD) & BN6_PAD_STEP) != 0;
	/* free, and walking: the CONTINUE went well */
	if (!h && stepped) {
		W.armed = false;
		return;
	}
	if (!h) W.was_free = true;
	W.held = h && !W.was_free ? W.held + 1 : 0;
	W.pushed = D.dir_held && cinema_input_mode() == CINEMA_FREE && !stepped ? W.pushed + 1 : 0;
	W.masked = cinema_input_mode() != CINEMA_FREE ? W.masked + 1 : 0;
	if (W.held >= WATCH_HELD || W.pushed >= WATCH_PUSHED || W.masked >= WATCH_HELD) free_him();
}

/* the dev step's save of a held MegaMan (play.py's "stuck NAME", --input's
 * "0:stuck NAME"): the holds of the mask set and the run saved with them,
 * as no save of the run's writes it, for a CONTINUE to let him go */
void director_dev_hold(unsigned mask) {
	if (!D.active) return;
	if (mask & HELD_FADE) emu_write8(BN6_FADE_ACTIVE, 1);
	if (mask & HELD_CHAT) emu_write32(BN6_CHATBOX_FLAGS, emu_read32(BN6_CHATBOX_FLAGS) | BN6_CHAT_ACTIVE);
	if (mask & HELD_LOCK) {
		emu_write8(BN6_DIALOGUE_LOCK, 1);
		flag_clear(BN6_FLAG_PLAYER_CAN_MOVE);
	}
	if (mask & HELD_SCRIPT) emu_write8(BN6_SCRIPTED_WALK, 1);
	if (mask & HELD_CONVEYOR) flag_set(BN6_FLAG_CONVEYOR);
	if (mask & HELD_NPC) emu_write8(BN6_NPC_CHAT, 1);
	if (mask & HELD_PET) flag_set(BN6_FLAG_PET_OFF);
	if (mask & HELD_PAUSED) emu_write8(BN6_MAP_PAUSED, 1);
	save_checkpoint();
	fprintf(stderr, "dev: the run saved with MegaMan held (%s)\n", held_names(bn6_held()));
}
