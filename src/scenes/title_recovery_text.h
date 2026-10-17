#ifndef CW_TITLE_RECOVERY_TEXT_H
#define CW_TITLE_RECOVERY_TEXT_H

enum { TITLE_RECOVERY_START, TITLE_RECOVERY_TRADE, TITLE_RECOVERY_BLOCKED, TITLE_RECOVERY_FAILED, TITLE_RECOVERY_DISCARD, TITLE_RECOVERY_PAGES };
extern const char title_recovery_heading[], title_recovery_help[];
extern const char *const title_recovery_lines[TITLE_RECOVERY_PAGES][3];
extern const char *const title_recovery_choices[TITLE_RECOVERY_PAGES][2];

#endif
