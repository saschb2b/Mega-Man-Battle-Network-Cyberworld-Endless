/* BN4's Souls' shots that are a chip, in BN6's engine (docs/multirom/bn4-souls.md,
 * WindSoul in BN6's engine): a power attack of BN4's that sets up a chip of
 * BN4's through its chip route (BN4_CHIP_ROUTE: WindSoul's AirShot and
 * WindRack, ProtSoul's WideSwrd, JunkSoul's Poltrgst) runs as the same routine on BN6's chip of
 * its name through BN6's twin (BN6_CHIP_ROUTE), the stores after it BN4's. */
#ifndef CW_SOULROUTE_H
#define CW_SOULROUTE_H

#include <stdint.h>

/* (after BN4's ROM and the chips' names are read) BN4's power attack `power`
 * as such a shot: its copy in SOULS4_ROUTES, written the first time it is
 * asked, its address with its Thumb bit; 0 where it is no chip route of that
 * shape, its chip is none of BN6's own, or the room is full */
uint32_t soulroute_at(int power);

#endif
