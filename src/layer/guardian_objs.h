/* A layer guardian's lines, music scripts and actors (guardian_objs.c). */
#ifndef CW_GUARDIAN_OBJS_H
#define CW_GUARDIAN_OBJS_H

#include <stdint.h>

#include "mapslot.h"
#include "net.h"
#include "text.h"

/* Event flags of the sequence, after the layer's choices (layer_objs.h). */
#define LAYER_BOSS_GONE_FLAG     0x1448   /* deleted: the guardian logs out */
#define LAYER_BOSS_APPEAR_FLAG   0x1449   /* the guardian logs in */
#define LAYER_REWARD_FLAG        0x144A   /* its Guardian Data shows */
#define LAYER_REWARD_TAKEN_FLAG  0x144B   /* ... and was taken */
#define LAYER_EXIT_OPEN_FLAG     0x144C   /* the exit pad shows */

typedef struct {
	int navi;                  /* 0: the layer has no guardian */
	int version;               /* 0-2, as make_boss sets it */
	int x, y, z, face;         /* where it stands (world) and its animation */
	int intro, defeat, reward; /* its scripts in the layer's archive */
	int prelude, hush, theme;  /* music: the boss prelude, silence, the area's */
} GuardianStage;

/* The guardian object `o` at world (wx, wy, wz): its scripts into `text`. */
void guardian_scripts(TextArchive *text, const NetObj *o, int wx, int wy, int wz, GuardianStage *g);
/* Its actors, once `archive` holds the scripts: the guardian (overworld
 * sprite `sprite`) and the Guardian Data it leaves. */
void guardian_actors(NpcList *npcs, uint32_t archive, int sprite, const GuardianStage *g);

#endif
