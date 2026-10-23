/* The display's settings (Options' Display section, settings.ini): how
 * the picture is scaled (screen = auto, whole, fill or stretch), the
 * window or the whole screen (fullscreen, the desktops) and smooth motion
 * (smooth_motion), each where the system has it. A change applies at
 * once and is kept in settings.ini; the browser player's menu is told
 * (its Smooth motion button), as it tells the game. */
#ifndef CW_DISPLAY_H
#define CW_DISPLAY_H

#include <stdbool.h>

/* settings.ini's screen. Auto: whole scales, filling where one leaves the
 * picture a quarter smaller or more; whole: the largest whole scale; fill:
 * the largest that fits, its shape kept; stretch: the whole screen, the
 * picture out of its 3:2 shape (the player's choice alone, never a
 * default: AGENTS.md's whole-scale rule) */
typedef enum { SCALE_AUTO, SCALE_WHOLE, SCALE_FILL, SCALE_STRETCH, SCALE_COUNT } DisplayScale;

DisplayScale display_scale(void);
/* settings.ini's word read as a scale (integer for whole too; anything
 * else auto) */
DisplayScale display_scale_read(const char *word);
/* settings.ini's display lines as the start reads them (screen,
 * smooth_motion): true where `key` was one */
bool display_setting(const char *key, const char *val);

/* What this system offers: the scale (all but the browser's page, which
 * sizes the picture itself), the window (the desktops) and smooth motion
 * (all but the 3DS, whose screen keeps the GBA's pace) */
bool display_has_scale(void);
bool display_has_window(void);
bool display_has_smooth(void);

/* The scale `dir` (+1 or -1) from the one in use, in the order this
 * system offers them (the 3DS's auto is its fill: it has no auto of its own) */
DisplayScale display_scale_next(int dir);
/* Set at once (--screen, over settings.ini) */
void display_scale_use(DisplayScale s);
/* ... and kept in settings.ini */
void display_scale_keep(DisplayScale s);
void display_fullscreen_keep(bool on);
/* ... `tell_page`: the browser player's Smooth motion button follows */
void display_smooth_keep(bool on, bool tell_page);

/* settings.ini's fullscreen in the data folder, for the window the start
 * opens: 1 on, 0 off, -1 none kept */
int display_fullscreen_kept(const char *data_dir);

#endif
