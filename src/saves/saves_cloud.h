/* SAVES' saves kept in step (docs/engine/saves.md, Saves kept in step), as
 * Steam Cloud keeps a game's: a pull as the game starts or comes back
 * (the title's CONTINUE waits for it a moment), a push a few seconds after
 * each save and as the game closes, and a choice only where this system
 * and the slot both moved on since they were last in step. Through
 * cloud_link.h, none of it where the system has no link. */
#ifndef CW_SAVES_CLOUD_H
#define CW_SAVES_CLOUD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* The game's start (start_net.c): the saves' writes followed (mirror.h's
 * hooks), a pull due */
void saves_cloud_start(void);
/* Each frame of the title, the ROMs screen and SAVES: the pull due begun,
 * a pull's saves taken in or compared */
void saves_cloud_tick(void);
/* The start's pull under way (a few seconds at most): CONTINUE waits */
bool saves_cloud_settling(void);
/* The title's note now, its frame and length for its box (cinema's):
 * false for none */
bool saves_cloud_note(char *out, size_t n, int *t, int *len);
/* Whether this system can keep its saves in step, and whether it does */
bool saves_cloud_here(void);
bool saves_cloud_on(void);
/* The CLOUD view's buttons now, in reading order: their count */
int saves_cloud_actions(int *out);
/* The CLOUD view's line, for the picture (the link as this frame's update
 * found it) */
void saves_cloud_line(char *out, size_t n);
/* A button of the cloud's chosen (home's CLOUD, the CLOUD view's): false
 * for any other */
bool saves_cloud_press(int action);
/* A link's bytes received by code: this system linked, its first pull
 * begun */
bool saves_cloud_join(const uint8_t *bytes, size_t n);
/* The comparison of the slot's saves answered: USE THE CLOUD'S taken
 * (`took`, its import done) or KEEP THESE (this system's pushed over the
 * slot's) */
void saves_cloud_answered(bool took);

#endif
