/* Dev steps on a layer (director_dev.c), for the director's other parts. */
#ifndef CW_DIRECTOR_DEV_H
#define CW_DIRECTOR_DEV_H

#include "foes.h"

void dev_steps(void);
/* A dev step's battle (the foes step's, the xrecord step's, director.h),
 * forced at the first moment MegaMan stands free on the layer's map */
void director_dev_due(const Encounter *e);

#endif
