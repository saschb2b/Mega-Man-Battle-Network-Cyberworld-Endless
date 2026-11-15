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
/* ... drawn on the floor at world (x, y), as npc_prop's pads. */
uint32_t npc_floor_mark(int category, int index, int x, int y, int anim, int gone_flag);

/* ... a floor sprite whose animation follows the byte at *anim_at (the
 * caller writes it between frames: a gate's still and swirling looks). */
uint32_t npc_floor_live(int category, int index, int x, int y, int anim, uint32_t *anim_at);
/* ... and standing at world (x, y, z), as npc_mark's: a dark layer's
 * units, their animations the board's (docs/game/liberation/) */
uint32_t npc_live(int category, int index, int x, int y, int z, int anim, uint32_t *anim_at);

/* A standing NPC that talks with `script` of the text archive at `archive`,
 * and leaves once event flag `gone_flag` is set (-1: never); a `floor` one
 * (a pad) is drawn under MegaMan. */
uint32_t npc_talker(int category, int index, int x, int y, int z, int anim, uint32_t archive, int script, int gone_flag, bool floor);
/* ... standing behind a counter: drawn under the second layer (the
 * counter's art covers its legs), spoken to across it at its talk centre
 * (sx, sy) world units from where it stands, as the originals' counter
 * navis are. */
uint32_t npc_counter_talker(int category, int index, int x, int y, int z, int anim, uint32_t archive, int script, int sx, int sy);
/* ... a floor talker (an object of list `category`, sprite `index`) whose
 * look the director may change between frames (a Glitch warp's,
 * docs/game/glitch-layer.md): its idle loop's jump pointed at `look`, after
 * writing a sprite's number at its +1 and a mosaic's size at its +6 and +7
 * (ns_set_sprite and ns_set_mosaic), or at `blur`, the mosaic's size at
 * its +1 and +2, runs that snippet once a frame and back to `loop`; the
 * jump pointed at `loop` again, as it was */
typedef struct { int category, index, x, y, z; uint32_t archive; int script, gone_flag; } NpcTalk;
typedef struct { uint32_t jump_at, loop, look, blur; } NpcLooks;
uint32_t npc_talker_looks(const NpcTalk *t, NpcLooks *looks);
/* A list 6 talker's sprite (script `at`, npc_talker's) set by BN6's own
 * ns_set_sprite, which dresses it in its variation: the palette and hidden
 * pieces BN6's NPC sprite variations give that sprite (a Virus Battler's
 * Mr.Prog: bn6f OverworldNPCSpriteVariations) */
void npc_variation(uint32_t at);

/* ... a wide object (a desk) spoken to anywhere along it: sprite `index`
 * of list `category` at world (x, y) playing `anim`, talking with `script`
 * of `archive`; its middle (cx, cy) world units from where it stands,
 * walked into within `radius` of that. */
typedef struct {
	int category, index, x, y, anim;
	uint32_t archive;
	int script, cx, cy, radius;
} NpcDesk;
uint32_t npc_desk(const NpcDesk *desk);

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

/* Sprite `index` of list `category` as the core's ROM copy holds it, the
 * other games' copied in among them (docs/multirom/): how many animations
 * it has, -1 where it cannot be read (a compressed one). BN6 reads an
 * animation past them as stray bytes, never ending one it waits on (issue
 * #614). */
int npc_sprite_anims(int category, int index);
/* ... whether it has animation `anim` (true where it cannot be read) */
bool npc_sprite_has(int category, int index, int anim);
/* ... whether that animation's first frame draws anything: false past its
 * animations and where the frame is empty (BN4's TopMan facing away);
 * true where it cannot be read */
bool npc_sprite_shows(int category, int index, int anim);
/* ... a Navi's (list 6) `anim` where it shows, else the first facing
 * towards the camera that does (3, 5, 4, 2), else `anim`; another list's
 * `anim` */
int npc_shown_anim(int category, int index, int anim);
/* Whether a talker of that sprite turns to MegaMan when spoken to, as
 * BN6 turns one to 1, 3, 5 or 7 (bn6f npc_inChatbox_curAction_init_809EB20):
 * not an object (list 7) or a battle sprite (list 0), nor one whose sprite
 * lacks one of those four facings drawn */
bool npc_sprite_turns(int category, int index);

/* MegaMan's reach for talking widened (`wide`, in the Net) or BN6's own
 * (the real world, where A's checks look along it too), in the core's ROM
 * copy; npc_reach_install widens it as the core starts. */
void npc_reach(bool wide);
void npc_reach_install(void);
/* Where BN6's own probe for A looks from the player facing `face` (0-7):
 * whole world units ahead. */
void npc_probe(int face, int *dx, int *dy);

#endif
