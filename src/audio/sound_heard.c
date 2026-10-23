/* The game's sounds heard or not (sound_heard.h). */
#include "sound_heard.h"

/* (set by the main thread, read by the device's and the core's) */
static volatile bool music = true, effects = true;

void sound_heard_set(bool m, bool e) {
	music = m;
	effects = e;
}

bool sound_heard_music(void) { return music; }
bool sound_heard_effects(void) { return effects; }
