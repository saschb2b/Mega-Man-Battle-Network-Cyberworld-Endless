/* The portable saves screen and every way into its comparison (issue #114). */
#ifndef CW_SAVES_H
#define CW_SAVES_H

#include <stdbool.h>

/* Read the system's device name before any portable revision is made. */
void saves_init(void);
/* Open over the title, launcher, or a suspended run. */
void saves_open(void);
/* A picked, dropped, or discovered local file goes through the same preflight. */
void saves_import(const char *path);
/* Look for a file at the title before automatic export can replace it. */
void saves_tick(void);

#endif
