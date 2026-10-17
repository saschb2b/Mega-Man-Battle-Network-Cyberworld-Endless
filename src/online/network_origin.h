#ifndef CW_NETWORK_ORIGIN_H
#define CW_NETWORK_ORIGIN_H

#include <stdbool.h>
#include <stddef.h>

void network_origin_set(bool checkpoint);
void network_origin_back(char *error, size_t cap);

#endif
