#ifndef CW_TITLE_ACCESS_H
#define CW_TITLE_ACCESS_H

#include <stdbool.h>

bool title_access_keys(void);
void title_access_offer(void);
void title_access_hint(int x, int y, bool menu, bool pressed, bool confirm);
void title_access_network(void);

#endif
