/* What a layer's rolls put on it (net_services.c): its services, gates and
 * the rival, rolled from its seed apart from its floor, so that a fork can
 * read the layer behind a way before it is built (layer_holds, net.h;
 * docs/world/level-design/fork-read.md). */
#ifndef CW_NET_SERVICES_H
#define CW_NET_SERVICES_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	bool shop, heal, trader, programs, bugtrader, challenge, duel, undernet, secret, navi_gate, vault, official;
	int gate_navi, official_level;
} Services;

/* A layer's rolls, on their own numbers (the generator's put back after) */
void ng_roll_services(Services *s, uint32_t seed, int depth, int biome, int kind);

#endif
