/* The actors of a guardian's sequence (docs/BOSSES.md): NPC scripts that
 * wait hidden for the director's event flags. */
#ifndef CW_STAGE_NPC_H
#define CW_STAGE_NPC_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	int appear;   /* the guardian logs in */
	int gone;     /* the guardian logs out, deleted */
	int reward;   /* its Guardian Data shows */
	int taken;    /* ... and has been taken */
} StageFlags;

/* A guardian's body on the net: a sprite (its list and number), the
 * animation it stands in, mirrored or not, the animation it logs in by and
 * the pose it strikes for the title card (-1 none). */
typedef struct {
	int list, index, anim;
	bool mirror;
	int log_in, pose;
} NpcBody;
#define NPC_ANIM_LOG_IN 25   /* every Navi's overworld sprite: materializing */

/* The guardian: body `b` at world (x, y, z); on `appear` it logs in and
 * strikes its pose; on `gone` it fades out and leaves. Nobody can talk to
 * it. */
uint32_t npc_guardian(const NpcBody *b, int x, int y, int z, const StageFlags *f);
/* The Guardian Data it leaves: a Mystery Data crystal (animation `anim`)
 * that shows on `reward` and runs text `script` of `archive` when checked,
 * which should set `taken`. */
uint32_t npc_guardian_data(int x, int y, int z, int anim, uint32_t archive, int script, const StageFlags *f);
/* A floor pad (sprite list `category`, `index`) that appears once
 * `open_flag` is set. */
uint32_t npc_sealed_pad(int category, int index, int x, int y, int z, int open_flag);

#endif
