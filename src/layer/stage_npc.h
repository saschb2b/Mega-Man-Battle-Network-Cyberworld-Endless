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

/* The guardian: overworld sprite `sprite` (list 6) at world (x, y, z)
 * facing `face`; on `appear` it logs in (its materializing animation when
 * `logs_in`) and strikes animation `pose` (-1 none); on `gone` it fades
 * out and leaves. Nobody can talk to it. */
uint32_t npc_guardian(int sprite, int x, int y, int z, int face, int pose, bool logs_in, const StageFlags *f);
/* The Guardian Data it leaves: a Mystery Data crystal (animation `anim`)
 * that shows on `reward` and runs text `script` of `archive` when checked,
 * which should set `taken`. */
uint32_t npc_guardian_data(int x, int y, int z, int anim, uint32_t archive, int script, const StageFlags *f);
/* A floor pad (sprite list `category`, `index`) that appears once
 * `open_flag` is set. */
uint32_t npc_sealed_pad(int category, int index, int x, int y, int z, int open_flag);

#endif
