/* Another game's town in one of BN6's real-world maps (xtown.c,
 * docs/world/bn5-town.md): its tiles and colours copied in. */
#ifndef CW_XTOWN_H
#define CW_XTOWN_H

#include <stdbool.h>
#include <stdint.h>

/* The BN6 map whose MapBGDescriptor is at ROM offset `desc` drawn with
 * map (group, number) of game `rom` (1 + XRomId): its tile set and 256
 * colours copied into a range of their own (the next layer's may be
 * another game's at the same time), BN6's interface colours 208-255 kept.
 * With `rom` 0, the map's own tile set and colours put back where a copy
 * had taken their place. */
bool xtown_gfx(uint32_t desc, int rom, int group, int number);

#endif
