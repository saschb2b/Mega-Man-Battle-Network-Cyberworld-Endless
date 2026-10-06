/* The generator's own parts (net_gen.c), for its other files (net_detours.c, net_landmarks.c, net_set_pieces.c). */
#ifndef CW_NET_GEN_H
#define CW_NET_GEN_H

#include <stdbool.h>
#include <stdint.h>

#include "net_shapes.h"

NetObj *ng_add_obj(int type, int x, int y);
extern uint8_t ng_reserved[MAP_H][MAP_W];
extern uint8_t ng_hushed[MAP_H][MAP_W];
bool ng_cell_free(int x, int y);
bool ng_cuts(int n, const int *xs, const int *ys);
bool ng_cuts_way(int x, int y);
bool ng_beside_narrow(int x, int y);
bool ng_behind_gap(int x, int y);
bool ng_by_walkway(int x, int y);
extern uint8_t ng_way_band[MAP_H][MAP_W];
/* (net_way.c) */
void ng_mark_way(int sx, int sy, int gx, int gy);
#define SIG_PAST 10000
void ng_walk_past_signature(int16_t d[MAP_H][MAP_W]);
bool ng_near_talker(int x, int y);
bool ng_navi_near(int x, int y);
void ng_hush(int x, int y);
bool ng_room_spot_in(const Room *r, int *ox, int *oy, bool open);
bool ng_fits(int rise);
bool ng_floor_cell(int x, int y);
bool ng_void_cell(int x, int y);
void ng_route_distances(void);
int ng_far_from_way(int x, int y);
int ng_npcs_for(int want);

#endif
