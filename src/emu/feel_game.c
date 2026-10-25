#include "feel_game.h"

#include "bn6.h"
#include "bn6_battle_loop.h"
#include "bn6_map.h"
#include "director.h"
#include "emu.h"
#include "fast_forward_game.h"
#include "feel_out.h"

/* the first of the battle's objects in play on a side with HP: MegaMan's (0) */
static uint32_t megaman(void) {
	for (uint32_t k = 0; k < BN6_T1_COUNT; ++k) {
		uint32_t o = BN6_T1_OBJECTS + k * BN6_T1_SIZE;
		if ((emu_read8(o + BN6_T1_IN_PLAY) & 1) && emu_read8(o + BN6_T1_ALLIANCE) == 0) return o;
	}
	return 0;
}

/* the enemies with HP left, viruses and Navis apart (a Navi's NameID past
 * the viruses' 0xFF; a guardian's helpers with none, "TestVirs", not counted) */
static void enemies(FeelGame *g) {
	for (uint32_t k = 0; k < BN6_T1_COUNT; ++k) {
		uint32_t o = BN6_T1_OBJECTS + k * BN6_T1_SIZE;
		if (!(emu_read8(o + BN6_T1_IN_PLAY) & 1) || emu_read8(o + BN6_T1_ALLIANCE) != 1 || !emu_read16(o + BN6_T1_HP)) continue;
		if (emu_read16(o + BN6_T1_NAME_ID) > 0xFF) ++g->navis;
		else ++g->viruses;
	}
}

/* (none of it without MegaMan's object: a battle setting up) */
static bool read_fight(FeelGame *g) {
	uint32_t m = megaman();
	if (!m) return false;
	g->hp = emu_read16(m + BN6_T1_HP);
	g->max_hp = emu_read16(m + BN6_T1_MAX_HP);
	uint32_t ai = emu_read32(m + BN6_T1_AI_DATA);
	g->charge = ai ? emu_read8(ai + BN6_AI_CHARGE) : 0;
	g->synchro = emu_read8(BN6_BATTLE_MOOD) == BN6_MOOD_SYNCHRO;
	g->form = emu_read8(BN6_BATTLE_FORM);
	enemies(g);
	return true;
}

/* L or R opens the Custom screen: the gauge full, the fight on, not
 * paused; not a Virus Battle's, whose gauge is its clock (full, TIME UP) */
static bool gauge_full(int phase) {
	if (phase != BN6_PHASE_FIGHT || emu_read16(BN6_CUSTOM_GAUGE) < BN6_GAUGE_FULL) return false;
	uint32_t settings = emu_read32(BN6_BATTLE_SETTINGS);
	if (settings >= 0x08000000u && settings < 0x0A000000u && emu_read8(settings + BN6_SETTINGS_MODE) == BN6_MODE_VIRUS_BATTLER) return false;
	return !emu_read8(emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PAUSE) + BN6_PAUSED);
}

void feel_game_frame(void) {
	static FeelGame was;
	FeelGame now = { 0 };
	int sub = emu_read8(BN6_GAMESTATE), phase = emu_read8(BN6_BATTLE_PHASE);
	now.fight = sub == BN6_SUB_BATTLE && (phase == BN6_PHASE_CUSTOM || phase == BN6_PHASE_FIGHT);
	if (now.fight) now.fight = read_fight(&now);
	/* (the map's group, not the director's place: it calls the town home
	 * a few frames into a layer's first map, Lan's HP's pad left) */
	bool run = director_in_town() || director_on_layer() || director_in_hp();
	int group = emu_read8(BN6_MAP_GROUP);
	now.real = run && group < BN6_GROUP_INTERNET;
	now.net = run && group >= BN6_GROUP_INTERNET;
	FeelBeat beats[FEEL_BEATS_MOST];
	int n = feel_beats(&was, &now, beats);
	/* (fast forward's frames go by unfelt: a walk at 3x, a jack-in in it) */
	if (!fast_forward_running())
		for (int i = 0; i < n; ++i) feel_beat(beats[i].cue, beats[i].amount);
	was = now;
	feel_want_triggers(now.fight && gauge_full(phase), fast_forward_available());
}
