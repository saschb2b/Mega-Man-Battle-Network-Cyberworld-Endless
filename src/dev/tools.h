/* The dev tools the game's binary runs instead of the game (tools.c):
 * --atlas, --pacing, --render-song, --sheet. */
#ifndef CW_TOOLS_H
#define CW_TOOLS_H

#include <stdbool.h>

/* A tool's option and its value: true where it was one */
bool tools_option(const char *a, const char *v);
/* The tool asked for, run: its exit code; -1 where none was asked for */
int tools_run(void);

#endif
