#ifndef CW_ONLINE_UI_H
#define CW_ONLINE_UI_H

#include "online_types.h"

void online_ui_reset(void);
void online_ui_update(const OnlineSnapshot *snapshot);
bool online_ui_open(void);
void online_ui_draw(void);

#endif
