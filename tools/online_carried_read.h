#ifndef CW_ONLINE_CARRIED_READ_H
#define CW_ONLINE_CARRIED_READ_H

#include <stdbool.h>
#include <stdio.h>
#include "online_snapshot.h"

struct mCore;
bool online_carried_read(struct mCore *core, OnlineLoadout *out);
bool online_carried_write(FILE *out, const OnlineLoadout *loadout);
void online_carried_blob(FILE *out, const void *data, size_t bytes);

#endif
