/* The shared dive's band over the map's foot (issue #589): the feed's
 * lines a few seconds each, else the crew's wait at a portal, else the
 * party forming at a guardian's door; never over BN6's chat box. Lines go
 * in with dive_link_line (dive_link.h). */
#ifndef CW_DIVE_BAND_H
#define CW_DIVE_BAND_H

#include <stdint.h>

void dive_band_reset(void);
/* Each frame: the line shown long enough gives way to the next. */
void dive_band_frame(uint32_t now);

#endif
