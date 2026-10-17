#ifndef CW_DEV_ONLINE_CAPTURE_H
#define CW_DEV_ONLINE_CAPTURE_H

#include "online_snapshot.h"

/* Optional local diagnostic selected by CYBERWORLD_LOADOUT_TRACE. */
void online_capture_loadout(const OnlineLoadout *loadout, bool checkpoint);
/* Static screen captures only; never send a trade message or change a Pack.
 * CYBERWORLD_TRADE_PREVIEW is a panel phase, 0..7. */
void online_capture_start(void);
bool online_capture_preview(void);
void online_capture_draw(void);

#endif
