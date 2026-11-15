/* dive_finds.h */
#include "dive_finds.h"

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "director.h"
#include "dive_link.h"
#include "dive_traces.h"
#include "dive_wire.h"
#include "dive_words.h"
#include "emu.h"
#include "online_transport.h"
#include "run.h"
#include "talk.h"

#define ANSWER_MS 3000   /* the server's first find waited for so long */

static struct {
	uint32_t seed;
	int depth;
	bool known;          /* the opened ones read on this layer */
	uint32_t opened;     /* a bit per Mystery Data index */
	int pending;         /* the one just opened, -1 none */
	bool had_finder;     /* ... someone had opened it first already */
	uint32_t at;
} F = { .pending = -1 };

static uint32_t opened_now(void) {
	uint32_t bits = 0;
	for (int k = 0; k < director_md_count() && k < 32; ++k)
		if (director_md_opened(k)) bits |= 1u << k;
	return bits;
}

static void found(int k) {
	uint8_t m[ONLINE_DIVE_FOUND_BYTES];
	size_t n = dive_wire_found(m, (unsigned)k);
	if (n && online_transport_send(m, n)) fprintf(stderr, "dive: found Mystery Data %d\n", k);
	F.pending = k;
	F.had_finder = *dive_traces_finder((unsigned)k) != 0;
	F.at = SDL_GetTicks();
}

/* once BN6's box has closed: the week's first finder in the band */
static void told(void) {
	if (F.pending < 0 || talk_busy() || emu_read8(BN6_CHATBOX) || !director_on_map()) return;
	const char *name = dive_traces_finder((unsigned)F.pending);
	if (!*name && SDL_GetTicks() - F.at < ANSWER_MS) return;
	if (*name) dive_link_line(dive_find_words(name, !F.had_finder));
	F.pending = -1;
}

void dive_finds_frame(void) {
	if (!director_on_layer()) return;
	if (!F.known || F.seed != run.layer_seed || F.depth != run.depth) {
		F.seed = run.layer_seed;
		F.depth = run.depth;
		F.opened = opened_now();
		F.known = true;
		F.pending = -1;
		for (int k = 0; k < director_md_count(); ++k) {
			int x, y, z;
			director_md_place(k, &x, &y, &z);
			fprintf(stderr, "dive: Mystery Data %d at %d,%d,%d%s\n", k, x, y, z, director_md_opened(k) ? ", opened" : "");
		}
		return;
	}
	uint32_t now = opened_now(), fresh = now & ~F.opened;
	F.opened = now;
	for (int k = 0; fresh; ++k, fresh >>= 1)
		if (fresh & 1) found(k);
	told();
}
