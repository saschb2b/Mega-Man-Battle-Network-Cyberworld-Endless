/* How loud the game's sounds are heard (sound_heard.h). */
#include "sound_heard.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* (set by the main thread, read by the device's and the core's) */
static volatile int music = SOUND_LEVELS, effects = SOUND_LEVELS;

static int level_in(int v) { return v < 0 ? 0 : v > SOUND_LEVELS ? SOUND_LEVELS : v; }

void sound_heard_set(int m, int e) {
	music = level_in(m);
	effects = level_in(e);
}

int sound_heard_music(void) { return music; }
int sound_heard_effects(void) { return effects; }

int sound_heard_read(const char *val) {
	if (!strcmp(val, "off") || !strcmp(val, "no")) return 0;
	if (!strcmp(val, "on") || !strcmp(val, "yes")) return SOUND_LEVELS;
	char *end;
	long v = strtol(val, &end, 10);
	return end != val && !*end ? level_in((int)v) : SOUND_LEVELS;
}

double sound_heard_gain(int level) {
	if (level <= 0) return 0.0;
	return level >= SOUND_LEVELS ? 1.0 : pow(10.0, -3.0 * (SOUND_LEVELS - level) / 20.0);
}

int sound_heard_share(int level, int louder) {
	if (level <= 0) return 0;
	return level >= louder ? SOUND_SHARE_ALL : (int)lrint(SOUND_SHARE_ALL * pow(10.0, -3.0 * (louder - level) / 20.0));
}
