/* --dev rollbench (issue #128, docs/online/arena/rollback/overview.md): what a
 * rollback of the Arena would cost on this machine, measured on the
 * player's ROM. The game's core alone first, then the pair at a Practice
 * NetBattle's battle by full rules, scripted keys on both sides: the snapshot's size,
 * its save and load, a pair frame's emulation, a rollback of eight frames
 * timed and its hash checked against the frames run straight, and from
 * them the largest rollback window a 60th of a second holds at delays 1,
 * 2 and 3. Said on the screen and in the output (log.txt on a handheld);
 * a headless run ends once it is done. */
#ifndef CW_ROLLBENCH_H
#define CW_ROLLBENCH_H

#include "game.h"   /* (scene_rollbench) */

#endif
