/* The wait for a dropped link over the game, for a capture (devtools.h). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "devtools.h"
#include "online_transport.h"
#include "platform.h"
#include "reconnect_draw.h"

static struct { bool on, back; uint32_t since; } R;

void reconnect_preview(const char *spec) {
	if (!strcmp(spec, "off")) { R.on = false; return; }
	R.on = true;
	R.back = !strcmp(spec, "back");
	/* (as if the link went `spec` seconds ago) */
	R.since = SDL_GetTicks() - (R.back ? 0u : (uint32_t)atoi(spec) * 1000u);
	printf("reconnect preview: %s\n", spec);
}

void reconnect_preview_draw(void) {
	if (!R.on) return;
	uint32_t now = SDL_GetTicks();
	ReconnectView v = { .back = R.back, .elapsed = now - R.since, .window = ONLINE_RECONNECT_MS, .t = now };
	reconnect_draw(P.core_x, P.core_y, &v);
}
