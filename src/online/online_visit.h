/* A Square visit borrows the checkpoint and never saves online RAM. */
#ifndef CW_ONLINE_VISIT_H
#define CW_ONLINE_VISIT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_snapshot.h"
#include "online_types.h"

bool online_visit_begin(const char *url, const char *name, const uint8_t identity[16], char *error, size_t cap);
/* Explicit command-line join, using the data directory's local identity. */
bool online_visit_command(const char *url, const char *name, char *error, size_t cap);
bool online_visit_active(void);
/* Also true after return until the player deliberately resumes play. */
bool online_visit_preserves_checkpoint(void);
uint32_t online_visit_run_keys(uint32_t keys);
/* Called by scene_emu after core installation, before its first frame. */
bool online_visit_prepare(char *error, size_t cap);
void online_visit_profile(OnlineProfile *out);
/* Queues return through CONTINUE, which rebuilds the checkpoint's ROM map. */
void online_visit_return(void);
/* No more frames between a queued return and the scene's next enter. */
bool online_visit_transitioning(void);
void online_visit_returned(bool restored);
void online_visit_left(void);
/* Close/background shutdown never turns Square RAM into a checkpoint. */
void online_visit_shutdown(void);

#endif
