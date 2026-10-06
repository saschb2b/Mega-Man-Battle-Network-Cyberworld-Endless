/* A layer's plan (net_plan.h): its layout and its signature, and what the
 * run's layers before it have shown. */
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

/* Layer k (0-2) of the act drawn from `s` in area `b`, its draws turned on
 * by turn_l and turn_s. Where the act would close in its first layer's
 * layout and signature both (two layouts and two signatures, each drawn in
 * turn), its second layer takes the first's signature and the third the
 * other: three places, the guardian's unlike the one before it in both,
 * should its layout not fit and another be built. */
static void act_layer(int b, uint32_t s, int k, int turn_l, int turn_s, int *layout, int *sig) {
	*layout = layout_in_act(b, s, k + turn_l);
	*sig = sig_in_act(b, s, k + turn_s);
	int l0 = layout_in_act(b, s, turn_l), s0 = sig_in_act(b, s, turn_s);
	if (k == 0 || layout_in_act(b, s, 2 + turn_l) != l0 || sig_in_act(b, s, 2 + turn_s) != s0 || layout_in_act(b, s, 1 + turn_l) == l0) return;
	*sig = k == 1 ? s0 : sig_in_act(b, s, 1 + turn_s);
}

/* A normal layer's (of area `biome`): its act's draw, turned one on where
 * the act would open in the layout (or the signature) the act before closed
 * in, so that no two layers in a row share one where the area has others (a
 * hub in Sky Area after a hub on a homepage reads as the same layer twice).
 * From the cycle's first act on, each act's turn known from the one before
 * it. */
static void act_plan(int depth, int biome, int *layout, int *sig) {
	int cycle0 = depth - 1 - (depth - 1) % CYCLE_LAYERS, prev_layout = -1, prev_sig = -1;
	for (int first = cycle0 + 1;; first += 3) {
		int b = depth < first + 3 ? biome : biome_for_depth(first);
		uint32_t s = act_seed(first);
		int turn_l = layout_in_act(b, s, 0) == prev_layout, turn_s = sig_in_act(b, s, 0) == prev_sig;
		if (depth < first + 3) {
			act_layer(b, s, depth - first, turn_l, turn_s, layout, sig);
			return;
		}
		act_layer(b, s, 2, turn_l, turn_s, &prev_layout, &prev_sig);
	}
}

void layer_plan(uint32_t seed, int depth, int biome, int kind, const LayerKit *kit, int *layout, int *sig) {
	if (kind == LAYER_NORMAL) act_plan(depth, biome, layout, sig);
	else {
		*layout = layout_pick(biome);
		*sig = sig_for_seed(biome, seed);
	}
	if (kit && kit->hub_rooms) *sig = SIG_NONE;
}
