/* A generated layer's objects in its game map: the exit pad, Mystery Data,
 * talkers, services, shops and the choices the director acts on. */
#ifndef CW_LAYER_OBJS_H
#define CW_LAYER_OBJS_H

#include <stdbool.h>
#include <stdint.h>

#include "guardian_objs.h"

/* Event flags the layer's choices set (Yes), one per choice. */
#define LAYER_FLAG_BASE 0x1440
#define LAYER_MAX_CHOICES 8
/* The first layer's gift was chosen. */
#define LAYER_GIFT_FLAG 0x144D

typedef struct {
	int start_x, start_y;      /* world position of the warp in */
	int exit_x, exit_y;        /* the exit (or return) pad */
	uint32_t archive;          /* the layer's text archive */
	GuardianStage guardian;    /* the layer's guardian and its sequence */
	int nchoices;
	struct { int type, flag; } choice[LAYER_MAX_CHOICES];   /* type: OBJ_* */
	int challenge_reward;      /* the script a won challenge runs, -1 for none */
} LayerObjs;

/* Installs the current layer's objects in map (group, number). */
bool layer_objs_install(int group, int number, LayerObjs *out);

#endif
