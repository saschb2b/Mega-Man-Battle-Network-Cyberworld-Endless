/* Saves kept in step (docs/engine/saves.md, Saves kept in step): the
 * game's side of shared/online/README.md's SYNC exchange, as a job on the
 * relay (relay.h), and the slot this system shares with the player's
 * others, kept in cloud.ini in the data folder: its secret and the receipt
 * (the generation these saves are in step with, their hash, when). */
#ifndef CW_CLOUD_H
#define CW_CLOUD_H

#include "cloud_link.h"

/* The link for SAVES and the title (cloud_link_set), its server `url` (the
 * official Square's, or --dev online's); NULL where the system has no
 * network for it */
const CloudLink *online_cloud_link(const char *url);

#endif
