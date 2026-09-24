/* NPC scripts in the game's bytecode (bn6f npc_script.inc), written into
 * the free ROM space for a layer's NPC list. */
#ifndef CW_NPC_H
#define CW_NPC_H

#include <stdbool.h>
#include <stdint.h>

/* How far back in depth a floor sprite stands (npc.c). */
#define NPC_FLOOR_BACK 64

/* A Mystery Data crystal for flag MAPSLOT_MD_FLAG + index; the game places
 * it at its placement and gives its content when taken. */
uint32_t npc_mystery(int index);
/* A floor sprite (a pad) at world (x, y, z) playing `anim`: no collision,
 * drawn under MegaMan and the NPCs; category is the sprite list (7
 * overworld objects), index its sprite. */
uint32_t npc_prop(int category, int index, int x, int y, int z, int anim);

/* A standing NPC that talks with `script` of the text archive at `archive`,
 * and leaves once event flag `gone_flag` is set (-1: never); a `floor` one
 * (a pad) is drawn under MegaMan. */
uint32_t npc_talker(int category, int index, int x, int y, int z, int anim, uint32_t archive, int script, int gone_flag, bool floor);

#endif
