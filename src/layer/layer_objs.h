/* A generated layer's objects in its game map: the exit pad, Mystery Data,
 * talkers, services, shops and the choices the director acts on. */
#ifndef CW_LAYER_OBJS_H
#define CW_LAYER_OBJS_H

#include <stdbool.h>
#include <stdint.h>

#include "guardian_objs.h"
#include "shop.h"

/* A Navi gate's code: its Navi deleted this many times as a guardian, in
 * any runs (rivals.sav). */
#define GATE_CODE 2
/* Event flags the layer's choices set (Yes), one per choice. */
#define LAYER_FLAG_BASE 0x1440
#define LAYER_MAX_CHOICES 8
/* The first layer's gift was chosen. */
#define LAYER_GIFT_FLAG 0x144D
/* L has told where they are on this layer (in the saved RAM, so a CONTINUE
 * mid-layer does not tell it all again). */
#define LAYER_TOLD_FLAG 0x144E
/* The Net Dealer has said his words on this layer (a later talk is a line
 * and the list). */
#define LAYER_DEALER_TOLD_FLAG 0x144F
/* ... and the NaviCust vendor his, and a Recovery Mr. Prog. */
#define LAYER_VENDOR_TOLD_FLAG 0x1450
#define LAYER_HEAL_TOLD_FLAG   0x1451
/* The layer's vault gave its chip (docs/META.md, gates). */
#define LAYER_VAULT_FLAG       0x1453
/* The layer's official gate gave its chip (docs/RIVAL.md). */
#define LAYER_OFFICIAL_FLAG    0x1455
/* ... and Chaud's clearance reaches its level: it opens (the director sets
 * it as the layer begins, and as a duel on it is won). */
#define LAYER_CLEARED_FLAG     0x1456
/* Chaud's call on a duel layer was made (a CONTINUE does not make it
 * again). */
#define LAYER_DUEL_CALLED_FLAG 0x1457
/* MegaMan has named the layer's Rush gap (issue #14). */
#define LAYER_RUSH_TOLD_FLAG   0x1458

typedef struct {
	int start_x, start_y;      /* world position of the warp in */
	int exit_x, exit_y;        /* the exit (or return) pad */
	uint32_t archive;          /* the layer's text archive */
	GuardianStage guardian;    /* the layer's guardian and its sequence */
	int nchoices;
	struct { int type, flag; } choice[LAYER_MAX_CHOICES];   /* type: OBJ_* */
	int challenge_reward;      /* the script a won challenge runs, -1 for none */
	int fragment_found;        /* what MegaMan says when the layer's ScrtData is picked up, -1 for none */
	int spin_found;            /* ... and the run's Spin (docs/META.md), -1 for none */
	int spin_colour;           /* its colour (1-6), 0 for none on this layer */
	int script_of[OBJ_OFFICIAL + 1];   /* each kind's first talker's script, -1 none (for --talk); OBJ_OFFICIAL the last kind */
	int gate_navi, gate_reward;    /* the Navi gate's Navi and the script his SP chip is given by, -1 none */
	int trader_kind;           /* the layer's trader's script in the game's trader archive (TraderKind), -1 none (for --talk) */
	ShopItem dealer[SHOP_MAX_ITEMS], programs[SHOP_MAX_ITEMS];   /* the shops' stock */
	int ndealer, nprograms;
} LayerObjs;

/* The layer's shop stock written (again after a state load, whose RAM
 * holds the shop data as it was saved: `saved`, what was bought there
 * stays bought). */
void layer_objs_shops(const LayerObjs *o, bool saved);

/* Installs the current layer's objects in map (group, number). */
bool layer_objs_install(int group, int number, LayerObjs *out);
/* Set before layer_objs_install: this act's Net Dealer has already spoken,
 * so this one greets in a line. */
extern bool layer_objs_dealer_again;
/* Set by layer_objs_install: this layer's Net Dealer names the act's
 * guardian (from the act's second layer, or once battled). */
extern bool layer_objs_dealer_named;
/* Set before layer_objs_install on a duel's layer (docs/RIVAL.md): ProtoMan's
 * time to beat, in frames, the rivalry's rung (0 his time, 1 his time
 * without a hit, 2 a netbattle with him), and whether the netbattle waits
 * for a later act (ProtoMan then names it, and asks nothing). */
extern int layer_objs_duel_frames, layer_objs_duel_rung, layer_objs_duel_foes;
/* The bystanders' Navi, its list-6 sprite and mugshot: BN6's HeelNavi,
 * or another game's on its area's layers (set before layer_objs_install). */
#define LAYER_BYSTANDER 67
extern int layer_objs_bystander;
/* The level of the layer's official gate (docs/RIVAL.md), 0 for none. */
extern int layer_objs_official_level;
extern bool layer_objs_duel_later;

#endif
