/* NPC scripts in the game's bytecode (bn6f npc_script.inc), written into
 * the free ROM space for a layer's NPC list. */
#ifndef CW_NPC_H
#define CW_NPC_H

#include <stdbool.h>
#include <stdint.h>

#include "mapslot.h"   /* NpcList */

/* How far back in depth a floor sprite stands (npc.c). */
#define NPC_FLOOR_BACK 64

/* A Mystery Data crystal for flag MAPSLOT_MD_FLAG + index; the game places
 * it at its placement and gives its content when taken. */
uint32_t npc_mystery(int index);
/* A floor sprite (a pad) at world (x, y, z) playing `anim`: no collision,
 * drawn under MegaMan and the NPCs; category is the sprite list (7
 * overworld objects), index its sprite. */
uint32_t npc_prop(int category, int index, int x, int y, int z, int anim);

/* A sprite standing at world (x, y, z) playing `anim`, nothing to walk
 * into or talk to (a mark over someone's head: z his height), gone once
 * event flag `gone_flag` is set (-1: never). */
uint32_t npc_mark(int category, int index, int x, int y, int z, int anim, int gone_flag);

/* A standing NPC that talks with `script` of the text archive at `archive`,
 * and leaves once event flag `gone_flag` is set (-1: never); a `floor` one
 * (a pad) is drawn under MegaMan. */
uint32_t npc_talker(int category, int index, int x, int y, int z, int anim, uint32_t archive, int script, int gone_flag, bool floor);
/* ... standing behind a counter: drawn under the second layer (the
 * counter's art covers its legs), spoken to across it at its talk centre
 * (sx, sy) world units from where it stands, as the originals' counter
 * navis are. */
uint32_t npc_counter_talker(int category, int index, int x, int y, int z, int anim, uint32_t archive, int script, int sx, int sy);

/* A talker who paces `steps` along facing `face` (1, 3, 5, 7: +x, +y, -x,
 * -y) and back, looking about at each end. */
uint32_t npc_walker(int category, int index, int x, int y, int face, int steps, uint32_t archive, int script);

/* A compressed sprite the people or objects of `npcs` use, which the map
 * must then load: false where its list has no room left (12 of them,
 * 0x8800 bytes decompressed, the game's loader), and it must not be
 * shown. */
bool npc_need_sprite(NpcList *npcs, int category, int index);
/* ... and those of the map objects in `npcs->objects` (BN6's own,
 * OverworldMapObjects: a map taken over keeps its furniture). */
void npc_objects_sprites(NpcList *npcs);

/* MegaMan's reach for talking widened (`wide`, in the Net) or BN6's own
 * (the real world, where A's checks look along it too), in the core's ROM
 * copy; npc_reach_install widens it as the core starts. */
void npc_reach(bool wide);
void npc_reach_install(void);
/* Where BN6's own probe for A looks from the player facing `face` (0-7):
 * whole world units ahead. */
void npc_probe(int face, int *dx, int *dy);

#endif
