#ifndef CW_DEV_ONLINE_CAPTURE_H
#define CW_DEV_ONLINE_CAPTURE_H

#include "online_snapshot.h"

/* Optional local diagnostic selected by CYBERWORLD_LOADOUT_TRACE. */
void online_capture_loadout(const OnlineLoadout *loadout, bool checkpoint);

#endif
