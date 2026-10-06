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
#define LAYER_DRAFT_FIT_FLAG     0x1467   /* (+k) the draft's k-th program fits the board's free space as it stands (the director keeps them) */
#define LAYER_SUPER_SEAL_FLAG    0x1452   /* a super boss's staging (docs/BOSSES.md): Bass's stone shakes */
#define LAYER_SUPER_POSE_FLAG    0x1454   /* ... and he strikes his pose, out of the white */

typedef struct {
	int navi;                  /* 0: the layer has no guardian */
	int version;               /* 0-2, as make_boss sets it (a super boss's: BN6's version index, super_boss.h) */
	int x, y, z, face;         /* where it stands (world) and its animation */
	/* where MegaMan steps up to meet him, beside him (guardian_stand), and
	 * the eighth he faces there; stand_face -1 where the layer has no arena,
	 * and he walks straight at him */
	int stand_x, stand_y, stand_face;
	int intro, defeat, reward; /* its scripts in the layer's archive */
	int prelude, hush, theme;  /* music: the boss prelude, silence, the area's */
	/* a super boss's too (docs/BOSSES.md, Super bosses; -1 for a guardian):
	 * the area's theme fading out, BN6's rumble (0xE3), the white's sound
	 * (0x100) and Bass's going (0xD7) */
	int fade, rumble, reveal, depart;
	uint8_t draft[3];          /* the draft's programs (variants), 0 none: LAYER_DRAFT_FIT_FLAG + k says whether each fits */
} GuardianStage;

/* The guardian object `o` at world (wx, wy, wz): its scripts into `text`. */
void guardian_scripts(TextArchive *text, const NetObj *o, int wx, int wy, int wz, GuardianStage *g);
/* Its actors, once `archive` holds the scripts: the guardian in his own
 * shape (guardian_body) and the Guardian Data he leaves. */
void guardian_actors(NpcList *npcs, uint32_t archive, const GuardianStage *g);

#endif
