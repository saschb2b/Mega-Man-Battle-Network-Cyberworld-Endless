/* The set pieces' cells (netmap_extra.c), for the map's other files. */
#ifndef CW_NETMAP_EXTRA_H
#define CW_NETMAP_EXTRA_H

#include "netmap_parts.h"

void nm_blocks_place(void);
void nm_gaps_place(void);
void nm_lanes_place(void);
extern CoordExtra nm_extra;
void nm_build_extra(const Learned *L);

#endif
