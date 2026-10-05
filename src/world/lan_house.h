/* Lan's house and his room (lan_house.c, docs/HOME.md): BN6's own maps as
 * the run's home, his PC its jack-in. */
#ifndef CW_LAN_HOUSE_H
#define CW_LAN_HOUSE_H

#include <stdbool.h>
#include <stdint.h>

#define LAN_HOUSE_GROUP 0x01   /* Central Town's (bn6f constants/enums/GameAreas.inc) */
#define LAN_HOUSE       0x01
#define LAN_ROOM        0x02

/* The house and the room installed: no people or map scripts of BN6's
 * story, its furniture and checks kept, its front door and stairs as BN6
 * has them, the bathroom's door left out, their song, and the room's PC a
 * jack-in to world (x, y) of map (to_group, to_number). False where their
 * doors cannot be read. */
bool lan_house_install(int to_group, int to_number, int x, int y);
/* BN6's own warp list of Central Town's map `number` (0 the town, the
 * house, the room), read before the run's first install took them over. */
uint32_t lan_house_bn6_warps(int number);
/* Whether map (group, number) is the house or the room. */
bool lan_house_map(int group, int number);
/* Where a run starts: the top of the room's stairs, as BN6 sets Lan down
 * coming up them. */
void lan_room_start(int *x, int *y, int *face);
/* The way on in map `number` (the house or the room): the room's PC, the
 * house's stairs up; false for none. */
bool lan_house_goal(int number, int *x, int *y);
/* Whether world (x, y) of the room stands on the PC's jack-in cells. */
bool lan_room_on_pc(int x, int y);
/* Each frame in the house: the bathroom's door shut (BN6 clears the
 * warp-off flags as it enters a map). */
void lan_house_frame(int number);

#endif
