/* The stands' words on the watching screen (stands_text.c, issue #128):
 * its heading, its keys, the way in and the catching up, its end. */
#ifndef CW_STANDS_TEXT_H
#define CW_STANDS_TEXT_H

extern const char stands_keys[];
extern const char stands_setting_up[];
extern const char stands_catching_up[];
extern const char stands_stopped[];
extern const char stands_over[];
extern const char stands_back[];
/* "WATCHING ALPHA VS BETA"; "Alpha won the NetBattle." */
const char *stands_heading(const char *first, const char *second);
const char *stands_won(const char *winner);
/* ... a 2v2's (protocol v15): "Alpha's team won the NetBattle.", its lead named */
const char *stands_team_won(const char *lead);
/* a 2v2's heading, the longest that fits first: "WATCHING Alpha & Beta VS
 * Gamma & Delta" (form 0), "Alpha & Beta VS Gamma & Delta" (1), "Alpha's
 * TEAM VS Gamma's TEAM" (2), its names as given (the leads' first) */
const char *stands_team_heading(int form, const char *const names[4]);

#endif
