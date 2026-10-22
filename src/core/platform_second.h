/* The second screen's part of the platform (platform_second.c), for
 * platform.c's frame log. */
#ifndef CW_PLATFORM_SECOND_H
#define CW_PLATFORM_SECOND_H

#include <stdint.h>

/* The second screen's draws since the last call, their time in the
 * performance counter's ticks into `ticks` */
int platform_second_parts(uint64_t *ticks);
/* Once a frame in platform_poll: a finger let go on the second screen
 * (the 3DS's bottom one, read from its touch panel; Android's display's,
 * handed over by Java) kept for platform_second_tap */
void platform_second_poll(void);

#endif
