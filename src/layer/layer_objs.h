/* A generated layer's objects in its game map: the exit pad, Mystery Data,
 * talkers, services, shops and the choices the director acts on. */
#ifndef CW_LAYER_OBJS_H
#define CW_LAYER_OBJS_H

#include <stdbool.h>
#include <stdint.h>

#include "guardian_objs.h"
#include "scripts.h"
#include "shop.h"

/* A Navi gate's code: its Navi deleted this many times as a guardian, in
 * any runs (rivals.sav). */
#define GATE_CODE 2
/* Event flags the layer's choices set (Yes), one per choice. */
#define LAYER_FLAG_BASE 0x1440
#define LAYER_MAX_CHOICES 8
/* The first layer's gift was chosen. */
#define LAYER_GIFT_FLAG 0x144D
/* L has told where they are on this layer, so its next L says the rest
 * (a CONTINUE clears it: L starts over there). */
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
#define LAYER_RUSH_TOLD_FLAG   0x1464   /* (0x1458-0x145F: the obstacles' and cubes' present flags, issue #42, and another game's prop's) */
/* A navi of the layer has told its security cube's P-Code (issue #45). */
#define LAYER_PCODE_FLAG       0x1465
/* A number door of the layer was answered wrong, and sealed (issue #47). */
#define LAYER_NUMBER_SEALED_FLAG 0x1466
/* The layer's flame of darkness gave its DarkChip, and left (docs/META.md,
 * DarkChips in BN5 territory). (0x1467-0x1469: the draft's fit flags,
 * guardian_objs.h) */
#define LAYER_DARK_TAKEN_FLAG  0x146D
/* The run's chips that sit out of the older net's battles were named
 * (a run's, never cleared by a layer; a new run starts without it). */
#define RUN_OUT_NAMED_FLAG     0x146E

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
	int dark_flame;            /* the flame of darkness's script, -1 none (for --talk) */
	int dark_flame_obj;        /* ... and the layer object it stands in place of, -1 none */
	ShopItem dealer[SHOP_MAX_ITEMS], programs[SHOP_MAX_ITEMS];   /* the shops' stock */
	int ndealer, nprograms;
	int nmd;                   /* the Mystery Data placed, each one's MYSTERY_* colour and layer.obj index (its flag MAPSLOT_MD_FLAG + its index) */
	uint8_t md_colour[16], md_obj[16];
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
 * or another game's on its area's layers, and the one they take turns
 * with (the same, or another the area lends; set before
 * layer_objs_install). */
#define LAYER_BYSTANDER 67
extern int layer_objs_bystander, layer_objs_bystander2;
/* The map objects another game's area lends its layers (NetAreaDef.xlooks,
 * XLOOK_*: its look for BN6's set pieces and props), 0 on BN6's own (set
 * before layer_objs_install). */
extern unsigned layer_objs_xlooks;
/* The level of the layer's official gate (docs/RIVAL.md), 0 for none. */
extern int layer_objs_official_level;
/* Set before layer_objs_install on a layer that holds a flame of darkness:
 * its list-7 sprite (-1: none), the DarkChip in it, and for BN5's flame
 * whether MegaMan says all of its price (a profile's first) and whether
 * BN6's battles play its kind too (layer_objs_dark_first, _ours). It
 * stands in its last bystander's place. */
extern int layer_objs_dark_flame;
extern const char *layer_objs_dark_chip;
/* ... a flame of BN6's own (docs/META.md, BN6's own DarkChips): its words'
 * pieces, NULL for BN5's */
extern const ScriptsDark6 *layer_objs_dark6;
/* Set before layer_objs_install where the layer's Server holds a Navi (its
 * battle rolled with the layer): the words its signal is named by ("" for
 * viruses). */
extern const char *layer_objs_server_navi;
extern bool layer_objs_dark_first, layer_objs_dark_ours;
extern bool layer_objs_duel_later;

#endif
