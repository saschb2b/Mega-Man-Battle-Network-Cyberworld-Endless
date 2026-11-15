/* --npc-anims (npc_anims.c; build.py npc-anims, docs/devtools/npc-anims.md):
 * every NPC the game places in another game's sprite (BN5's and BN4's
 * guardians, BN5's Darkloids, teammates, Liberation guardians and map
 * objects, the bystanders of another game's areas), each animation our
 * code can tell it to play checked against the sprite copied into the
 * core: there, and drawn where it is one to stand in (issue #614). */
#ifndef CW_NPC_ANIMS_H
#define CW_NPC_ANIMS_H

/* A line per NPC into `path`, FAIL ones to stderr too, the core started
 * and the other games' Navis copied in as a run's are; 0 where none
 * fails */
int npc_anims_run(const char *path);

#endif
