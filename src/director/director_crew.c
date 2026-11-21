/* director_crew.h */
#include "director_crew.h"

#include <stdio.h>

#include "bn6.h"
#include "bytes.h"
#include "crew_words.h"
#include "chip_entry.h"
#include "director.h"
#include "director_layer.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "game.h"
#include "gamecall.h"
#include "loot.h"
#include "run.h"
#include "weekly.h"

#define CREW_SALT 0xC3E5B47Du   /* the crew's battles' draws, apart from every other aside's */

int director_crew_index(void) { return D.battles; }

/* (a Mystery Data on its field takes the record's own row, which each game
 * of a party holds in another record: none on a crew's battle) */
static void no_gem(Encounter *e) {
	int n = 0;
	for (int i = 0; i < e->nobj && i < MAX_FIELD_OBJS; ++i)
		if (e->obj[i].kind >> 4 != FIELD_GEM) e->obj[n++] = e->obj[i];
	e->nobj = n;
}

bool dive_battles(void) { return weekly_on(); }

Encounter dive_battle(int index) {
	uint32_t saved = rng_state();
	LootMemory fought, none = LOOT_MEMORY_NONE;
	loot_memory(&fought);
	loot_memory_set(&none);
	rng_seed(run.layer_seed ^ CREW_SALT ^ (uint32_t)index * 2654435761u);
	/* (a Glitch layer's hardening left out: each game's comes apart at its own pace) */
	Encounter e = loot_random_battle(run.depth, run.biome, encounter_kind(index), run.layer_seed, index);
	/* (its background too, which the run's own RNG rolled as the record was set: two games of a crew drew two
	 * backgrounds in an area that has two, and built their fields apart, issue #715) */
	e.backdrop = biome_bg(e.biome);
	e.backdrop_set = true;
	loot_memory_set(&fought);
	rng_restore(saved);
	return e;
}

Encounter director_crew_battle(int index) {
	Encounter e = dive_battle(index);
	no_gem(&e);
	fprintf(stderr, "crew battle %d on depth %d (layer seed %08x):", index, run.depth, (unsigned)run.layer_seed);
	for (int i = 0; i < e.nfoes; ++i) fprintf(stderr, " %d/%d/%d@%d,%d", e.foes[i].kind, e.foes[i].family, e.foes[i].version, e.foes[i].col, e.foes[i].row);
	fprintf(stderr, "\n");
	return e;
}

void director_crew_record(const Encounter *e) { set_encounter(e, false); }

int director_crew_steps_reset(void) {
	uint32_t steps = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_STEPS);
	if (steps < BN6_EWRAM || steps >= BN6_EWRAM_END) return -1;
	int walked = emu_read16(steps + BN6_STEPS_WALKED);
	const uint8_t zero[2] = { 0, 0 };
	emu_write(steps + BN6_STEPS_WALKED, zero, sizeof zero);
	emu_write(steps + BN6_STEPS_CHECKED, zero, sizeof zero);
	return walked;
}

/* BN6's results' entry given in the run, as its results' exit gives it */
static void give(uint16_t reward) {
	unsigned amount = reward_amount(reward);
	switch (reward == REWARD_NONE ? 4u : reward_kind(reward)) {
	case REWARD_CHIP: game_call_ret(BN6_GIVE_CHIPS, reward_chip_id(reward), reward_chip_code(reward), 1, NULL); break;
	case REWARD_ZENNY: game_call(BN6_GIVE_ZENNY, amount, 0); break;
	case REWARD_BUGFRAGS: game_call(BN6_GIVE_BUGFRAGS, amount, 0); break;
	case REWARD_HP: {
		unsigned hp = emu_read16(BN6_NAVI_HP) + amount, max = emu_read16(BN6_NAVI_MAX_HP);
		uint8_t b[2];
		put16(b, (uint16_t)(hp > max ? max : hp));
		emu_write(BN6_NAVI_HP, b, sizeof b);
		break;
	}
	default: break;
	}
	fprintf(stderr, "crew battle: the reward %04x given in the run\n", reward);
}

void (*director_crew_band)(const char *line);

/* the last crew's battle's end, for the state's words */
static struct { bool ended, won, pulled; int hp; unsigned gave; } last;

bool director_crew_pull_spent(void) { return weekly_record()->pulled != 0; }

void director_crew_describe(FILE *f) {
	if (!weekly_on()) return;
	fprintf(f, "pullback %s\n", director_crew_pull_spent() ? "spent" : "free");
	if (!last.ended) { fprintf(f, "crewend none\n"); return; }
	fprintf(f, "crewend %s hp %d", last.won ? "won" : "lost", last.hp);
	if (last.pulled) fprintf(f, " pulled");
	else if (last.gave) fprintf(f, " gave %u", last.gave);
	fputc('\n', f);
}

/* the crew's gifts worked out from the field's seats, alike on every game
 * (crew_pull.h), and told in the band */
static CrewPullOut gifts(const CrewOver *o, unsigned me) {
	CrewPullOut out;
	crew_pull(&o->field, o->won, &out);
	for (unsigned k = 0; k < o->field.n && k < CREW_PULL_SEATS; ++k)
		if (out.pulled[k]) fprintf(stderr, "crew battle: seat %u deleted and pulled back by the crew with HP %u\n", k, out.hp[k]);
	const char *line = out.pulled[me] ? crew_pull_back_words() : out.gave[me] ? crew_pull_gave_words() : NULL;
	if (line && director_crew_band) director_crew_band(line);
	last.ended = true;
	last.won = o->won;
	last.pulled = out.pulled[me];
	last.hp = (int)out.hp[me];
	last.gave = out.gave[me];
	return out;
}

void director_crew_over(const Encounter *e, const CrewOver *o) {
	emu_encounter_drop();
	unsigned me = o->seat < CREW_PULL_SEATS ? o->seat : 0;
	CrewPullOut out = gifts(o, me);
	bool pulled = out.pulled[me];
	int max = emu_read16(BN6_NAVI_MAX_HP), hp = (int)out.hp[me];
	uint8_t b[2];
	put16(b, (uint16_t)(hp < 0 ? 0 : hp > max ? max : hp));
	emu_write(BN6_NAVI_HP, b, sizeof b);
	/* (deleted with nobody to pull him back: the run over as a deletion ends it alone, the owner's call) */
	if (hp <= 0) {
		fprintf(stderr, "crew battle: MegaMan deleted; the run is over\n");
		D.lost_to = 0;
		end_run();
		return;
	}
	++D.battles;
	loot_battle_fought(e);
	/* (pulled back by the crew, once a dive and marked on the Board: the battle's prize stays the standing ones', issue #718) */
	if (pulled) weekly_pulled();
	else if (o->won) {
		for (int i = 0; i < e->nfoes; ++i) run.viruses_deleted += e->foes[i].kind == FOE_VIRUS;
		give(o->reward);
	}
	fprintf(stderr, "crew battle: %s, MegaMan's HP %d%s, the layer's battles %d\n", o->won ? "won" : "lost", hp,
		pulled ? ", pulled back by the crew" : "", D.battles);
	if (out.gave[me]) fprintf(stderr, "crew battle: %u HP given to pull a crewmate back\n", out.gave[me]);
	roll_encounter();
}
