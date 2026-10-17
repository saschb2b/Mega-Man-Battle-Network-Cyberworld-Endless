/* Receipt history checks around portable save installation. No queued
 * writes, emulator access or changes to the candidate directory. */
#ifndef CW_ONLINE_TRADE_ADMIT_H
#define CW_ONLINE_TRADE_ADMIT_H

#include <stdbool.h>

bool online_trade_admit_check(const char *root, const char *directory);
/* Advance history before allowing the installed candidate to be played. */
bool online_trade_admit_remember(const char *root, const char *directory);
/* Establish the installation namespace using operating-system randomness.
 * Invalid or interrupted existing history is never replaced. */
bool online_trade_admit_namespace(const char *root);

#endif
