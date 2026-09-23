/* A generated layer's objects in its game map: the exit pad, Mystery Data,
 * talkers, services, shops and the choices the director acts on. */
#ifndef CW_LAYER_OBJS_H
#define CW_LAYER_OBJS_H

#include <stdbool.h>

/* Event flags the layer's choices set (Yes), one per choice. */
#define LAYER_FLAG_BASE 0x1440
#define LAYER_MAX_CHOICES 8

typedef struct {
	int start_x, start_y;      /* world position of the warp in */
	int exit_x, exit_y;        /* the exit (or return) pad */
	int nchoices;
	struct { int type, flag; } choice[LAYER_MAX_CHOICES];   /* type: OBJ_* */
} LayerObjs;

/* Installs the current layer's objects in map (group, number). */
bool layer_objs_install(int group, int number, LayerObjs *out);

#endif
