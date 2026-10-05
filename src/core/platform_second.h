/* The second screen's part of the platform (platform_second.c), for
 * platform.c's frame log. */
#ifndef CW_PLATFORM_SECOND_H
#define CW_PLATFORM_SECOND_H

#include <stdint.h>

/* The second screen's draws since the last call, their time in the
 * performance counter's ticks into `ticks` */
int platform_second_parts(uint64_t *ticks);

#endif
