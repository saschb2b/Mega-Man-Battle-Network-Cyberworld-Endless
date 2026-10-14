#ifndef CW_ONLINE_MOTION_H
#define CW_ONLINE_MOTION_H

#include "online_types.h"

void online_motion_reset(void);
/* A 100ms delayed view, interpolated between received snapshots. */
void online_motion_update(const OnlineSnapshot *latest, uint32_t now, OnlineSnapshot *shown);

#endif
