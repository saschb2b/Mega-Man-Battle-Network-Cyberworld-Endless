#ifndef CW_ONLINE_CARRIED_OBSERVE_H
#define CW_ONLINE_CARRIED_OBSERVE_H

#include <stdbool.h>
#include "online_frame_pair.h"

typedef struct OnlineCarriedObserver OnlineCarriedObserver;
OnlineCarriedObserver *online_carried_observer_open(OnlineFramePair *pair, const char *prefix);
bool online_carried_observer_flush(OnlineCarriedObserver *observer);
void online_carried_observer_close(OnlineCarriedObserver *observer);

#endif
