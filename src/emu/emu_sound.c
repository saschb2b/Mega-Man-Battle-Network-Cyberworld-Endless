/* The core's sound on its way to the device (emu_sound.h): a ring of
 * stereo frames that the core's frames write and the device's callback
 * reads (emu_audio_read), its fill held near LAT_TARGET by nudging the
 * resampler's output rate (audio_pace.h) and sound dropped past LAT_MAX,
 * and its place on the sound's timeline for a rollback (emu_audio_mark).
 * Both cores' alike: emu.c's mGBA and emu_gpsp.c's gpSP. */
#include "emu_sound.h"

#include <string.h>

#include <SDL.h>

#include "audio_pace.h"
#include "emu.h"

#define RING 16384 /* stereo frames of buffered sound */
/* The GBA runs at 59.73 Hz and frames are paced at 60, so the core makes a
 * little more sound than is played: keep the buffer near LAT_TARGET frames
 * (about 32 ms) by nudging the output rate, and drop sound past LAT_MAX. */
#ifdef CW_DESKTOP
#define LAT_TARGET 1024   /* (a desktop keeps up: about 21 ms) */
#else
#define LAT_TARGET 1536
#endif
#define LAT_MAX    6144

static int16_t ring[RING * 2];
static int ring_r, ring_w;
static SDL_mutex *ring_lock;
/* the ring's place on the sound's timeline (emu_audio_mark): samples written
 * and played since the start, and those of a rollback's sound still to be
 * skipped, as they were played already (issue #167) */
static uint64_t ring_written, ring_played, ring_debt;
static AudioPace pace;   /* (the ring's averaged fill: the output rate's nudge) */

bool emu_sound_ready(void) { return ring_lock != NULL; }

void emu_sound_put(const int16_t *frames, int n) {
	if (!ring_lock) return;
	SDL_LockMutex(ring_lock);
	for (int i = 0; i < n; ++i) {
		if (ring_debt) { --ring_debt; continue; }   /* (heard already, before a rollback: emu_audio_rewind) */
		int next = (ring_w + 1) % RING;
		if (next == ring_r) break; /* full: drop the rest */
		ring[ring_w * 2] = frames[i * 2];
		ring[ring_w * 2 + 1] = frames[i * 2 + 1];
		ring_w = next;
		++ring_written;
	}
	SDL_UnlockMutex(ring_lock);
}

double emu_sound_settle(void) {
	if (!ring_lock) return 1.0;
	SDL_LockMutex(ring_lock);
	int fill = (ring_w - ring_r + RING) % RING;
	if (fill > LAT_MAX) {
		ring_played += (uint64_t)(fill - LAT_TARGET);
		ring_r = (ring_w - LAT_TARGET + RING) % RING;
		fill = LAT_TARGET;
		audio_pace_reset(&pace, fill);
	}
	SDL_UnlockMutex(ring_lock);
	return audio_pace(&pace, fill, LAT_TARGET);
}

uint64_t emu_audio_mark(void) {
	if (!ring_lock) return 0;
	SDL_LockMutex(ring_lock);
	uint64_t mark = ring_written;
	SDL_UnlockMutex(ring_lock);
	return mark;
}

void emu_audio_rewind(uint64_t mark) {
	if (!ring_lock) return;
	SDL_LockMutex(ring_lock);
	if (mark >= ring_played && mark < ring_written) {
		/* (none of it heard yet: cut, the next written in its place) */
		ring_w = (int)(((uint64_t)ring_w + RING - (ring_written - mark)) % RING);
		ring_written = mark;
	} else if (mark < ring_played) {
		/* (some heard: the rest cut, and as much of the next written skipped) */
		ring_debt += ring_played - mark;
		ring_w = ring_r;
		ring_written = ring_played;
	}
	SDL_UnlockMutex(ring_lock);
}

void emu_audio_open(void) {
	if (!ring_lock) ring_lock = SDL_CreateMutex();
}

int emu_audio_read(int16_t *out, int frames) {
	int n = 0;
	if (ring_lock) SDL_LockMutex(ring_lock);
	while (n < frames && ring_r != ring_w) {
		out[n * 2] = ring[ring_r * 2];
		out[n * 2 + 1] = ring[ring_r * 2 + 1];
		ring_r = (ring_r + 1) % RING;
		++n;
	}
	ring_played += (uint64_t)n;
	if (ring_lock) SDL_UnlockMutex(ring_lock);
	memset(out + n * 2, 0, (size_t)(frames - n) * 4);
	return n;
}
