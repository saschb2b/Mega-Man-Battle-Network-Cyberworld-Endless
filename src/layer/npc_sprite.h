/* An NPC's sprite as the core's ROM copy holds it, the other games'
 * copied in among them (docs/multirom/): its animations and which draw
 * anything, for the scripts that tell it which to play (npc.c,
 * stage_npc.c, guardians.c; build.py npc-anims checks them,
 * docs/devtools/npc-anims.md). */
#ifndef CW_NPC_SPRITE_H
#define CW_NPC_SPRITE_H

#include <stdbool.h>

/* Sprite `index` of list `category`: how many animations it has, -1
 * where it cannot be read (a compressed one). BN6 reads an animation past
 * them as stray bytes, never ending one it waits on (issue #614). */
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
int npc_sprite_stand(int category, int index, int anim);
/* Whether a talker of that sprite turns to MegaMan when spoken to, as
 * BN6 turns one to 1, 3, 5 or 7 (bn6f npc_inChatbox_curAction_init_809EB20):
 * not an object (list 7) or a battle sprite (list 0), nor one whose sprite
 * lacks one of those four facings drawn */
bool npc_sprite_turns(int category, int index);

#endif
