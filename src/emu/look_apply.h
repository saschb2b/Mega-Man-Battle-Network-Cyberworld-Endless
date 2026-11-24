/* MegaMan's look written onto BN6's palettes of his plain body in the
 * core's ROM copy (issue #854, docs/game/looks.md; the indexes in
 * docs/rom/megaman-colours.md): BN6 reads those palettes raw every frame,
 * so a look is a write of their bytes, and a channel at LOOK_NONE puts
 * BN6's own back. */
#ifndef CW_LOOK_APPLY_H
#define CW_LOOK_APPLY_H

#include <stdbool.h>
#include <stdint.h>

#include "looks.h"

/* Which of his sprites' palettes (look_palette_of) */
enum {
	LOOK_SPRITE_BATTLE,    /* his battle body, palette 0 (BN6_LOOK_PAL_PLAIN) */
	LOOK_SPRITE_FIELD,     /* the field's and the real world's maps' palette 0 */
	LOOK_SPRITE_MUGSHOT,   /* a chat box's mugshot */
	LOOK_SPRITE_FACE,      /* the emotion window's face (its window's indexes kept as BN6's) */
	LOOK_SPRITE_BATTLE_PALE,      /* battle palette 1, palette 0 paled (BN6_LOOK_PAL_PALE) */
	LOOK_SPRITE_BATTLE_SYNCHRO,   /* battle palette 4, Full Synchro (BN6_LOOK_PAL_SYNCHRO) */
	LOOK_SPRITE_COUNT
};

/* `look` (NULL or invalid: BN6's own) written onto his plain palettes in
 * the ROM copy: the battle's (0, 1 and 4), the field's, the mugshot's and
 * the face's; nothing without a core. A Cross, a Soul and BeastOut draw
 * palettes of their own and keep them; the plain body takes the look again
 * as the form ends. The state's `lookseen` line says which of them the
 * frame's OBJ palettes hold. */
void look_apply(const Look *look);
/* The profile's worn look applied (scene_emu's start: a run's start and a
 * CONTINUE), and applied again whenever the worn look changes (a slot set
 * or selected, the dev switch) from now on */
void look_apply_worn(void);
/* `look`'s sixteen BGR555 colours for sprite `which` (LOOK_SPRITE_*) into
 * `out16`, BN6's own where a channel is LOOK_NONE: for a peer's MegaMan in
 * the Square and a NetBattle's two sides. False, `out16` untouched, where
 * `which` is out of range or there is no ROM. */
bool look_palette_of(const Look *look, int which, uint16_t out16[16]);

#endif
