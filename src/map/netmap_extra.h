/* The set pieces' cells (netmap_extra.c), for the map's other files. */
#ifndef CW_NETMAP_EXTRA_H
#define CW_NETMAP_EXTRA_H

#include "netmap_parts.h"
#include "stopwatch.h"

void nm_blocks_place(void);
void nm_gaps_place(void);
void nm_lanes_place(void);
extern CoordExtra nm_extra;
void nm_build_extra(const Learned *L);
/* The layer priorities of the side pieces the layer's tiles set (sides.h),
 * each at its panel: after its tiles are picked, before its coordinate
 * data is written. */
void nm_sides_extra(void);

/* `expr` run, its time added to netmap_times' `part` */
#define NM_TIMED(part, expr) do { double t0_ = stopwatch_ms(); expr; netmap_times.part += stopwatch_ms() - t0_; } while (0)

/* The coordinate data written (coords_write), timed */
bool nm_coords_timed(uint32_t slot, const CoordPad *pads, int n);

#endif
