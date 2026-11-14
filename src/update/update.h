/* The game's own updates (docs/engine/updates.md): once a start, on a
 * thread of its own, a look at the newest release's update.json; where it
 * names a newer version, the title says so beside the version, and the
 * title's Options has a line for it. Where this copy can put the new one
 * in place itself (the AppImage, the Windows installer's copy, a
 * PortMaster handheld's, Android's app), A on that line downloads it,
 * checks its SHA-256, and the game ends and starts again as the new
 * version (on Android, Android's installer ends it and puts the new one
 * in place); elsewhere the line says where it comes from.
 * On by default (the owner's call, 2026-11-14), Options' Updates line and
 * settings.ini's `updates = off` switch it off. Nothing is sent but the
 * request for that one file.
 *
 * A developer's look elsewhere: CYBERWORLD_UPDATE_URL (another
 * update.json, any build, headless too) and CYBERWORLD_UPDATE_FROM (the
 * version this build counts as); on Android, the same names as
 * GameActivity's string extras. */
#ifndef CW_UPDATE_H
#define CW_UPDATE_H

#include <stdbool.h>

/* Where it stands: nothing (none found, not looked, or switched off), a
 * newer version found, its download under way, failed, or done and in
 * place (the game ends at the next tick and starts it; on Android, in
 * the hands of Android's installer, which ends the game itself) */
typedef enum { UPDATE_IDLE, UPDATE_FOUND, UPDATE_GETTING, UPDATE_FAILED, UPDATE_READY } UpdateState;

/* How this copy is updated: by the game itself, from the download page
 * in the browser, from the releases page by hand (a PortMaster handheld
 * whose launcher cannot put it in place: no browser), or by the itch app
 * that installed it */
typedef enum { UPDATE_WAY_NONE, UPDATE_WAY_INSTALL, UPDATE_WAY_PAGE, UPDATE_WAY_RELEASES, UPDATE_WAY_ITCH } UpdateWay;

/* A frame of the title's, or of Options' over anything (title_version.c,
 * options.c): the first starts the look (settings.ini read by then); a
 * download done ends the game, which starts the new version as it exits */
void update_tick(void);

/* Whether this build looks at all (Options' Updates line) */
bool update_here(void);
/* ... and whether it may: the switch, kept in settings.ini */
bool update_on(void);
void update_set_on(bool on);

UpdateState update_state(void);
UpdateWay update_way(void);
/* The newer version found ("0.22.0"), "" where none */
const char *update_version(void);
/* The download's share in, 0-100 */
int update_percent(void);
/* Whether Options shows the update's line: one found, and the title the
 * scene under it (a run's PET never ends the game) */
bool update_line_shown(void);
/* ... once a start, when Options opens with it: its cursor goes there */
bool update_line_first(void);
/* A on the line: the download begun, or the download page opened; false
 * where it does nothing (the itch app's, a handheld's releases page) */
bool update_press(void);

#endif
