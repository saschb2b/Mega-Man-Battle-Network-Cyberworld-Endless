/* The opt-in Phase 0 room never starts or saves a run. */
#ifndef CW_ONLINE_SCENE_H
#define CW_ONLINE_SCENE_H

#include <stdbool.h>
#include <stdint.h>

void online_scene_start(void);
void online_scene_end(void);
void online_scene_update(void);
void online_scene_draw(void);
bool online_scene_enabled(void);
bool online_scene_ready(void);
bool online_scene_arrived(void);
uint32_t online_scene_keys(uint32_t keys);

#endif
