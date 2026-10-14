#ifndef CW_ONLINE_LABELS_H
#define CW_ONLINE_LABELS_H

#include "online_types.h"

void online_labels_reset(void);
void online_labels_update(const OnlineSnapshot *shown, uint32_t now);
void online_labels_draw(void);

#endif
