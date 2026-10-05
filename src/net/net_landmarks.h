/* A layer's landmarks and props (net_landmarks.c), for the generator's other files. */
#ifndef CW_NET_LANDMARKS_H
#define CW_NET_LANDMARKS_H

#include <stdbool.h>

#include "net_shapes.h"

void ng_emblems(const LayerKit *kit);
bool ng_near_stair(int x, int y);
bool ng_object_at(int x, int y);
NetObj *ng_counter(int r, int type, const LayerKit *kit);
bool ng_prop_at_cell(int x, int y);
int ng_landmark(const LayerKit *kit);
void ng_rows(const LayerKit *kit, int skip, const int *order, int n);
void ng_signs(const LayerKit *kit, int skip, const int *order, int n);
bool ng_landmark_foot(int *ox, int *oy);

#endif
