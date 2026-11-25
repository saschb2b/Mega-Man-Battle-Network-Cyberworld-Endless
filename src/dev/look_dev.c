/* devtools.h's dev_look. */
#include "devtools.h"

#include <stdio.h>
#include <string.h>

#include "looks.h"

void dev_look_step(const char *spec) {
	Look l = look_plain();
	unsigned v[6];
	if (strcmp(spec, "plain") && sscanf(spec, "%u,%u,%u,%u,%u,%u", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]) == 6)
		for (int c = 0; c < LOOK_CHANNELS; ++c) { l.palette[c] = (uint8_t)v[c]; l.colour[c] = (uint8_t)v[3 + c]; }
	if (look_valid(&l)) looks_set(looks_slot(), &l);
	else fprintf(stderr, "look: %s is no look\n", spec);
}
