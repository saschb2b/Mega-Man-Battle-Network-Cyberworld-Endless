/* A puppet: one of BN6's NPCs whose script the engine moves between frames
 * (the Net Square's peers and walkers, the shared dive's divers on a
 * layer). Its loop is BN6's own NPC bytecode: shown or hidden, an
 * animation, a position, a frame's pause and a jump back, or once through
 * a sound first. A caller writes inside its own claims_begin/claims_end
 * (src/emu/claims.h). */
#ifndef CW_PUPPET_H
#define CW_PUPPET_H

#include <stdbool.h>
#include <stdint.h>

#define PUPPET_MEGAMAN 55   /* list 6's MegaMan, bn6f npcSpritePtrs[0x37] */
#define PUPPET_WALK 8       /* a facing's walking animation is its standing one + this */

/* A puppet of sprite `index` of list `category`, hidden, in the map's
 * slot space (mapslot_alloc); 0 where there is no room. */
uint32_t puppet_make(int category, int index);

void puppet_show(uint32_t at, bool visible);
void puppet_anim(uint32_t at, int anim);
void puppet_place(uint32_t at, int x, int y, int z);
/* Sound `sound` played once on the next frame; puppet_loop after it. */
void puppet_sound(uint32_t at, int sound);
/* Back to the loop, past a sound played. */
void puppet_loop(uint32_t at);

#endif
