#ifndef CW_TITLE_RECOVERY_H
#define CW_TITLE_RECOVERY_H

#include <stdbool.h>

/* Recovery owns title input while ordinary run access is unavailable. */
void title_recovery_enter(void);
void title_recovery_failed(void);
bool title_recovery_active(void);
bool title_recovery_update(void);
bool title_recovery_draw(void);

#endif
