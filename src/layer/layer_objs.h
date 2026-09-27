/* A generated layer's objects in its game map: the exit pad, Mystery Data,
 * talkers, services, shops and the choices the director acts on. */
#ifndef CW_LAYER_OBJS_H
#define CW_LAYER_OBJS_H

#include <stdbool.h>
#include <stdint.h>

#include "guardian_objs.h"
#include "shop.h"

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

typedef struct {
	int start_x, start_y;      /* world position of the warp in */
	int exit_x, exit_y;        /* the exit (or return) pad */
	uint32_t archive;          /* the layer's text archive */
	GuardianStage guardian;    /* the layer's guardian and its sequence */
	int nchoices;
	struct { int type, flag; } choice[LAYER_MAX_CHOICES];   /* type: OBJ_* */
	int challenge_reward;      /* the script a won challenge runs, -1 for none */
	int script_of[OBJ_GIFT + 1];   /* each kind's first talker's script, -1 none (for --talk) */
	ShopItem dealer[SHOP_MAX_ITEMS], programs[SHOP_MAX_ITEMS];   /* the shops' stock */
	int ndealer, nprograms;
} LayerObjs;

/* The layer's shop stock written again (after a state load, whose RAM
 * holds the shop data as it was saved). */
void layer_objs_shops(const LayerObjs *o);

/* Installs the current layer's objects in map (group, number). */
bool layer_objs_install(int group, int number, LayerObjs *out);

#endif
