/* Shared ownership of the 3DS socket pool and TLS random service. */
#ifndef CW_NETWORK_3DS_H
#define CW_NETWORK_3DS_H

#include <stdbool.h>

bool network_3ds_begin(void);
void network_3ds_end(void);

#endif
