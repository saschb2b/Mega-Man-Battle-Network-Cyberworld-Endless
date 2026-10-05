/* The rival's duel on a layer (docs/RIVAL.md): its squad and ProtoMan's
 * time, rolled aside from the layer's own rolls, the official gate's
 * opening, a CONTINUE's duel as it was, and the verdict. */
#include "director_duel.h"

#include <stdio.h>

#include "data.h"
#include "debug.h"
#include "director_folder.h"
#include "director_state.h"
#include "duel_words.h"
#include "game.h"
#include "loot.h"
#include "pacing.h"
#include "save.h"

/* Whether this run's duel on this layer is fought (the profile keeps the
 * last one's layer). */
static bool duel_fought(void) {
	return run.side_kind == LAYER_NORMAL && profile.duel_depth == run.depth && profile.duel_run == run.seed;
}

/* ProtoMan's duel as a CONTINUE finds it: Chaud's call, made before the
 * save, not made again (every CONTINUE on a duel layer had replayed it,
 * after the duel too); and a duel fought after the checkpoint stays
 * fought, ProtoMan gone as he goes when it begins (its choice made), the
 * gate beside him as the verdict left it (issue #20: a CONTINUE met him
 * again, and each duel won anew counted in the record). */
void resume_duel(void) {
	if (flag_get(LAYER_DUEL_CALLED_FLAG)) D.duel_call_due = false;
	if (!duel_fought()) return;
	for (int i = 0; i < D.objs.nchoices; ++i)
		if (D.objs.choice[i].type == OBJ_DUEL) flag_set(D.objs.choice[i].flag);
	if (profile.duel_beat && layer_objs_official_level && rival_clearance() >= layer_objs_official_level) flag_set(LAYER_CLEARED_FLAG);
	D.duel_call_due = false;
}

/* Whether ProtoMan's duel stands on the layer. */
static bool duel_layer(void) {
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_DUEL) return true;
	return false;
}

/* The layer's official gate open where Chaud's clearance reaches its level
 * (its script reads LAYER_CLEARED_FLAG, docs/RIVAL.md); the one beside
 * ProtoMan's duel opens to its winner alone, as the duel is won, and stays
 * as a CONTINUE (`resumed`) finds it (a playtester with the clearance found
 * it open before the duel, which then paid nothing but the record). Where
 * his netbattle waits for a later act, there is no duel to win: the
 * clearance's, as elsewhere. */
void official_sync(bool resumed) {
	if (duel_layer() && !layer_objs_duel_later) {
		if (!resumed) flag_clear(LAYER_CLEARED_FLAG);
		return;
	}
	if (layer_objs_official_level && rival_clearance() >= layer_objs_official_level) flag_set(LAYER_CLEARED_FLAG);
	else flag_clear(LAYER_CLEARED_FLAG);
}

/* The rival's time to beat for a squad of `hp` (docs/RIVAL.md): three
 * seconds and one for every thirty HP, eight percent faster for every two
 * duels ProtoMan has lost (a rung's round), never under six tenths of it. */
static int duel_frames(int hp) {
	/* (loose at first: a playtester's act 1 hand took 27.5 s to ProtoMan's
	 * 11, and a good hand should beat the first rung) */
	double t = (4.0 + hp / 20.0) * 60.0, k = 1.0;
	for (int i = 0; i < profile.duel_won / 2; ++i) k *= 0.92;
	if (k < 0.6) k = 0.6;
	return (int)(t * k);
}

/* A race's squad holds no virus that decides when it can be hit. */
#define FAMILY_QUAKER 6   /* (the families are BN6's sprite categories less 0x0E) */
static bool duel_race_fair(const Encounter *e) {
	for (int i = 0; i < e->nfoes; ++i)
		if (e->foes[i].kind == FOE_VIRUS && e->foes[i].family == FAMILY_QUAKER) return false;
	return true;
}

static int encounter_hp(const Encounter *e) {
	int total = 0;
	for (int i = 0; i < e->nfoes; ++i) {
		const Foe *f = &e->foes[i];
		if (f->kind == FOE_ROCK) continue;
		int id = f->id >= 0 ? f->id : enemy_id(f->kind == FOE_NAVI ? 1 : 0, f->family, f->version), hp = 0, dmg = 0;
		if (id >= 0 && enemy_stats(id, &hp, &dmg)) total += hp;
	}
	return total;
}

/* The rival's duel on the layer being built (docs/RIVAL.md): its squad,
 * rolled aside from the layer's own rolls, and ProtoMan's time for it. The
 * squad is the layer seed's alone, not the last battles fought: they steer
 * a random battle away from their viruses, and a CONTINUE brings them back
 * as the quit left them, not as MegaMan arrived (a playtester's ProtoMan
 * "busted a pair of viruses here in 0:12.00", and after a CONTINUE on the
 * same layer three in 0:14.50, session 64). Its rung and time read the
 * record, which only a duel's verdict moves, and the layer's duel is done
 * after one (issue #20). */
void duel_roll(void) {
	uint32_t saved = rng_state();
	LootMemory fought, none = { -1, -1, 0, 0, 0 };
	loot_memory(&fought);
	loot_memory_set(&none);
	rng_seed(run.layer_seed ^ 0xD0E15EEDu);
	/* (rung 2: no race, ProtoMan himself, from the third act, where
	 * MegaMan can stand his hits: his attacks are his 1800 HP version's,
	 * his HP the act's guardian band at most; before it, he names the act) */
	layer_objs_duel_rung = profile.duel_won % 3;
	layer_objs_duel_later = layer_objs_duel_rung == 2 && pacing_loop(run.depth) == 0 && pacing_act(run.depth) < 2;
	/* (a race's squad is one of the act's own battles: its time is the
	 * test, not its strength; one above the band deleted a playtester at
	 * 100 of 140 HP on layer 2, his run over) */
	D.duel_enc = layer_objs_duel_rung == 2 ? make_boss(run.depth, run.biome, 11) : make_encounter(run.depth, run.biome, ENC_NORMAL);
	/* (and none a race can't hurry: a Quaker is out of reach in the air
	 * until it lands, so the clock times its hops, not the player; three of
	 * a playtester's four duels were Quakers, "a Quaker lottery") */
	for (int tries = 0; layer_objs_duel_rung != 2 && tries < 8 && !duel_race_fair(&D.duel_enc); ++tries)
		D.duel_enc = make_encounter(run.depth, run.biome, ENC_NORMAL);
	layer_objs_duel_foes = D.duel_enc.nfoes;
	if (emu_debug_on()) {
		fprintf(stderr, "duel squad (rung %d):", layer_objs_duel_rung);
		for (int k = 0; k < D.duel_enc.nfoes; ++k) fprintf(stderr, " %d/%d/%d", D.duel_enc.foes[k].kind, D.duel_enc.foes[k].family, D.duel_enc.foes[k].version);
		fprintf(stderr, "\n");
	}
	/* (the netbattle's ProtoMan: half the act's guardian band's top at
	 * most, as his attacks stay his 1800 HP version's, ten times a
	 * guardian's damage a second: at the top, 1000 in act 3, a playtester's
	 * MegaMan ran after one hand, docs/RIVAL.md) */
	int lo, hi;
	pacing_guardian_band(pacing_act(run.depth), &lo, &hi);
	D.duel_cap = layer_objs_duel_rung == 2 && pacing_loop(run.depth) == 0 ? hi / 2 : 0;
	loot_memory_set(&fought);
	rng_restore(saved);
	layer_objs_duel_frames = duel_frames(encounter_hp(&D.duel_enc));
	D.duel_call_due = true;
}

/* The duel's verdict (docs/RIVAL.md): MegaMan's DeleteTime against
 * ProtoMan's, and on rung 1 no hit taken; the record kept, and Chaud's
 * words queued, with both times as the results screen shows them. */
void duel_verdict(bool won) {
	D.duel = false;
	int mine = D.duel_time, his = layer_objs_duel_frames, rung = layer_objs_duel_rung, before = rival_clearance();
	bool beat = won && (rung == 2 || (mine < his && !(rung == 1 && D.duel_hit)));

	if (beat) profile.duel_won++;
	else profile.duel_lost++;
	profile.duel_run = run.seed;
	profile.duel_depth = (uint16_t)run.depth;
	profile.duel_beat = beat;
	profile_save();
	int size = (int)sizeof D.duel_verdict, k = verdict_result(D.duel_verdict, sizeof D.duel_verdict, won, beat, rung, mine, his);
	#define ADD(...) (k += snprintf(D.duel_verdict + k, k < size ? (size_t)(size - k) : 0, __VA_ARGS__))
	/* (what his respect opens: docs/RIVAL.md) */
	int after = rival_clearance();
	if (after > before && after == 1) {
		ADD("@C You've earned my first clearance,Lan.|@C The official Chip Orders are open to you now.|");
		/* (and the TagChip system, for good: issue #51) */
		ADD("@C And a NetBattler's trick. The TagChip system.|@C Tag two chips with SELECT in your Folder's EDIT.|"
			"@C They'll come to your hand together.|@C As long as they total under 60 MB.|");
		profile.tag_taught = 1;
		profile_save();
	}
	else if (after > before) ADD("@C My full clearance,Lan.|@C Every official gate opens for you now.|");
	ADD("@C That's %d-%d between us.", profile.duel_won, profile.duel_lost);
	/* (and the next rung, the door it leads to: a playtester's second win
	 * read as for the record alone) */
	if (beat && rung == 1 && after < 2) ADD("|@C Next time,no race. ProtoMan faces MegaMan himself.|@C Beat him,and my full clearance is yours.");
	/* (Lan answers a win too: a playtester's first, after five losses, met
	 * silence where every loss had had his "Next time, Chaud!") */
	if (beat) ADD("|@L %s", rung == 2 ? "Good battle,ProtoMan! See ya,Chaud!"
		: profile.duel_won == 1 && profile.duel_lost ? "We finally beat his time!! See ya,Chaud!"
		: "Yes!! See ya,Chaud!");
	/* (and the gate beside the duel opens at once to its winner: the prize
	 * where it was offered) */
	bool opened = beat && layer_objs_official_level && after >= layer_objs_official_level;
	if (opened) {
		flag_set(LAYER_CLEARED_FLAG);
		ADD("|@M Lan! The official gate here opens for us now!");
	}
	/* (Lan answers a loss, as he took the duel: Chaud had the last word) */
	if (!beat) ADD("|@L Grr... Next time,Chaud!");
	#undef ADD
	D.duel_verdict_due = true;
}
