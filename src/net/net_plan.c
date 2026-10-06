/* A layer's plan (net_plan.h): its layout, its signature and the side it
 * is arrived at, and what the run's layers before it have shown. */
#include "net_plan.h"

#include "net.h"
#include "net_layouts.h"
#include "net_signature.h"
#include "run.h"

/* The draw an act's layouts and signatures come in, from the run's seed:
 * one per act of each cycle. */
static uint32_t act_seed(int depth) {
	int p = (depth - 1) % CYCLE_LAYERS;
	return run.seed ^ (uint32_t)((depth - 1) / CYCLE_LAYERS * 7 + p / 3 + 1) * 0x9E3779B9u;
}

/* The sides an act's layers arrive at, the four in an order drawn from its
 * seed `s` (BN6's own maps arrive at each about as often: docs/
 * LEVEL_DESIGN.md, Arrivals), so an act's three arrive at three. */
static int side_in_act(uint32_t s, int index) {
	int order[SIDE_COUNT] = { SIDE_TOP, SIDE_BOTTOM, SIDE_LEFT, SIDE_RIGHT };
	uint32_t r = (s ^ 0xA771E5u) | 1;
	for (int i = SIDE_COUNT - 1; i > 0; --i) {
		r = r * 1103515245u + 12345u;
		int j = (int)((r >> 16) % (uint32_t)(i + 1)), t = order[i];
		order[i] = order[j];
		order[j] = t;
	}
	return order[index % SIDE_COUNT];
}

/* Layer k (0-2) of the act drawn from `s` in area `b`, its draws turned on
 * by turn_l and turn_s (and turn_d, its side's). Where the act would close
 * in its first layer's layout and signature both (two layouts and two
 * signatures, each drawn in turn), its second layer takes the first's
 * signature and the third the other: three places, the guardian's unlike
 * the one before it in both, should its layout not fit and another be
 * built. */
static void act_layer(int b, uint32_t s, int k, const int turn[3], LayerPlan *out) {
	out->layout = layout_in_act(b, s, k + turn[0]);
	out->sig = sig_in_act(b, s, k + turn[1]);
	out->side = side_in_act(s, k + turn[2]);
	int l0 = layout_in_act(b, s, turn[0]), s0 = sig_in_act(b, s, turn[1]);
	if (k == 0 || layout_in_act(b, s, 2 + turn[0]) != l0 || sig_in_act(b, s, 2 + turn[1]) != s0 || layout_in_act(b, s, 1 + turn[0]) == l0) return;
	out->sig = k == 1 ? s0 : sig_in_act(b, s, 1 + turn[1]);
}

/* A normal layer's (of area `biome`): its act's draw, turned one on where
 * the act would open in the layout (the signature, the side) the act before
 * closed in, so that no two layers in a row share one where the area has
 * others (a hub in Sky Area after a hub on a homepage reads as the same
 * layer twice). From the cycle's first act on, each act's turn known from
 * the one before it. */
static void act_plan(int depth, int biome, LayerPlan *out) {
	int cycle0 = depth - 1 - (depth - 1) % CYCLE_LAYERS;
	LayerPlan prev = { -1, -1, -1 };
	for (int first = cycle0 + 1;; first += 3) {
		int b = depth < first + 3 ? biome : biome_for_depth(first);
		uint32_t s = act_seed(first);
		int turn[3] = { layout_in_act(b, s, 0) == prev.layout, sig_in_act(b, s, 0) == prev.sig, side_in_act(s, 0) == prev.side };
		if (depth < first + 3) {
			act_layer(b, s, depth - first, turn, out);
			return;
		}
		act_layer(b, s, 2, turn, &prev);
	}
}

int arrive_forced = -1;

void layer_plan(uint32_t seed, int depth, int biome, int kind, const LayerKit *kit, LayerPlan *out) {
	if (kind == LAYER_NORMAL) act_plan(depth, biome, out);
	else {
		out->layout = layout_pick(biome);
		out->sig = sig_for_seed(biome, seed);
		out->side = (int)((seed * 2654435761u) >> 30);
	}
	if (kit && kit->hub_rooms) out->sig = SIG_NONE;
	if (arrive_forced >= 0) out->side = arrive_forced;
}
