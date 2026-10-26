/* What a layer's rolls put on it: its services, gates and the rival
 * (net_services.h). */
#include "net_services.h"

#include "game.h"
#include "net.h"
#include "pacing.h"
#include "run.h"

/* Points of interest in the other rooms. */
static void roll_services(Services *s, int depth, int biome, int kind, bool boss) {
	/* each act's middle layer has the Net Dealer and a heal, and so does its
	 * first after the run's first (the guardian's zenny to spend on what the
	 * new act calls for); docs/game/progression/ */
	int biome_layer = layer_in_act(depth);
	/* (a trip back as its act's middle layer: the dealer, the heal and the
	 * vendor's chance, docs/world/home/) */
	bool own = kind == LAYER_NORMAL || kind == LAYER_BACK;
	s->shop = own && (biome_layer == 1 || (biome_layer == 0 && depth > 1) || rng_range(0, 99) < (biome_layer == 0 ? 50 : 25));
	/* (threat 2, docs/game/meta/: only the heals an act is sure of; the heals
	 * helper: one on every layer) */
	s->heal = (rng_range(0, 99) < (boss ? 70 : 30) && run.threat < 2) || (own && pacing_heal_certain(depth)) ||
		(run.helpers & HELP_HEALS);
	s->trader = rng_range(0, 99) < (run.threat >= 7 ? 12 : 25);   /* (threat 7: half as often) */
	/* (the NaviCust vendor on every act's middle layer, BugFrags' sink:
	 * docs/game/navicust.md 4; his old 60% chance still rolled, so the rolls
	 * after it roll as they did) */
	s->programs = own && biome_layer == 1;
	if (s->programs) (void)rng_range(0, 99);
	s->bugtrader = kind == LAYER_UNDERNET || (biome == BIOME_GRAVEYARD && rng_range(0, 99) < 40);
	s->trader &= !s->bugtrader;   /* the trade screen serves one trader per map */
	s->challenge = depth > 1 && rng_range(0, 99) < 20 + depth;
	/* the rival's duel (docs/game/rival.md): on each act's second layer, where
	 * no strong virus signal stands (one battle to seek out a layer) */
	s->duel = kind == LAYER_NORMAL && biome_layer == 1 && biome != BIOME_NEST;
	s->challenge &= !s->duel;
	s->undernet = kind == LAYER_NORMAL && depth >= 4 && !boss &&
		rng_range(0, 99) < (biome == BIOME_GRAVEYARD ? 40 : 12);
	s->secret = kind == LAYER_UNDERNET && !run.secret_cleared;
}

/* The gates' rolls, after the services'. */
static void roll_gates(Services *s, uint32_t seed, int depth, int kind, bool boss) {
	/* a gate sealed with a Navi's code (docs/game/meta/phase-two.md, gates): from act 3,
	 * before the guardian's layer, where no dark warp stands; its Navi one
	 * of the guardians (but ProtoMan, the Secret Area's) */
	s->navi_gate = kind == LAYER_NORMAL && pacing_act(depth) >= 2 && !boss && !s->undernet && rng_range(0, 99) < 30;
	static const uint8_t gate_navis[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 18 };
	s->gate_navi = s->navi_gate ? gate_navis[rng_range(0, (int)sizeof gate_navis - 1)] : 0;
	/* a collector's vault (docs/game/meta/phase-two.md, gates): from act 2, before the
	 * guardian's layer, where no other gate or dark warp stands */
	s->vault = kind == LAYER_NORMAL && pacing_act(depth) >= 1 && !boss && !s->undernet && !s->navi_gate &&
		rng_range(0, 99) < 20;
	/* an official gate (docs/game/rival.md): from act 2, where no other gate or
	 * dark warp stands; sealed until Chaud's clearance reaches its level
	 * (from the seed, not the rolls: two rolls more reshuffled the rooms
	 * of every layer after them). And on every act's duel layer, the prize
	 * where the duel is offered, its level the act's (1 to the third, then
	 * 2), the only gate there (the rolls kept): a playtester's promise of
	 * official gates pointed at none he had met */
	if (s->duel) s->navi_gate = s->vault = false;
	uint32_t oh = (seed ^ 0x0FF1C1A1u) * 2654435761u;
	s->official = kind == LAYER_NORMAL && !boss &&
		(s->duel || (pacing_act(depth) >= 1 && !s->undernet && !s->navi_gate && !s->vault && (oh >> 16) % 100 < 25));
	s->official_level = !s->official ? 0 : s->duel ? (pacing_act(depth) >= 2 ? 2 : 1) : 1 + (int)((oh >> 8) & 1);
}

/* A guardian's layer, as layer_generate makes it */
static bool boss_of(int depth, int kind) {
	if (kind == LAYER_SECRET) return !run.secret_cleared;
	return kind == LAYER_NORMAL && is_boss_depth(depth);
}

/* The services and gates, rolled on their own numbers from the seed and
 * the area, apart from the floor's: known before the layer is built
 * (layer_holds), the main numbers put back after */
void ng_roll_services(Services *s, uint32_t seed, int depth, int biome, int kind) {
	uint32_t keep = rng_state();
	rng_seed(seed ^ 0x5E5F1CE5u ^ (uint32_t)biome * 0x2545F491u);
	bool boss = boss_of(depth, kind);
	roll_services(s, depth, biome, kind, boss);
	roll_gates(s, seed, depth, kind, boss);
	rng_restore(keep);
}

unsigned layer_holds(uint32_t seed, int depth, int biome, int kind) {
	Services s;
	ng_roll_services(&s, seed, depth, biome, kind);
	bool boss = boss_of(depth, kind);
	/* (a guardian's layer: its last stop's dealer and heal left unread, the
	 * guardian's signal said first and the last stop telling them there,
	 * briefing_words.c; the dealer's counter found no room on one in a few
	 * hundred, and a read never promises what is not built) */
	if (boss) s.shop = s.heal = false;
	return (s.shop ? HOLDS_DEALER : 0u) | (s.heal ? HOLDS_HEAL : 0u) | (s.trader ? HOLDS_TRADER : 0u) |
		(s.programs ? HOLDS_PROGRAMS : 0u) | (s.bugtrader ? HOLDS_BUGTRADER : 0u) | (s.challenge ? HOLDS_SERVER : 0u) |
		(s.duel ? HOLDS_DUEL : 0u) | (s.undernet ? HOLDS_UNDERNET : 0u) | (s.navi_gate || s.vault || s.official ? HOLDS_GATE : 0u) |
		(boss ? HOLDS_GUARDIAN : 0u);
}
