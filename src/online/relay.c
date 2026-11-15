/* relay.h. Each message the server sends handed to took() (the blob's
 * DATA kept here, the rest the job's word), the blob going sent by pump()
 * while the link takes it. */
#include "relay.h"

#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "network_prefs.h"
#include "online_browser.h"
#include "online_socket.h"
#include "protocol.h"

/* (the systems with no socket of the Square's: online_stream.c's) */
#if defined(__PSP__) || (defined(__3DS__) && !defined(CW_3DS_CURL))
#define NO_RELAY 1
#endif

#define QUIET_MS 15000     /* the server's next word waited for at most (its own limit) */
#define WHOLE_MS 120000    /* an exchange at most */
#define CHUNK (ONLINE_MAX_MESSAGE - 1)

static struct {
	char url[NETWORK_URL];
	const RelayJob *job;
	const uint8_t *out;    /* the blob going (the job's) */
	size_t out_n;
	uint8_t *in;           /* the blob coming */
	size_t in_n, moved;
	bool sending, receiving;
	bool ended, again;     /* this connection's exchange done; another wanted */
	bool replied;          /* a message sent back: given a moment before the close */
	uint32_t started, heard;
	uint32_t shown_moved, shown_size;   /* (under the lock) */
#ifdef __EMSCRIPTEN__
	bool running, open;
#else
	SDL_Thread *thread;
	SDL_mutex *lock;
	SDL_atomic_t stop, done;
#endif
} R;

bool relay_here(void) {
#ifdef NO_RELAY
	return false;
#else
	return true;
#endif
}

void relay_use(const char *url) {
	SDL_strlcpy(R.url, url ? url : "", sizeof R.url);
#ifndef __EMSCRIPTEN__
	if (!R.lock) R.lock = SDL_CreateMutex();
#endif
}

void relay_lock(void) {
#ifndef __EMSCRIPTEN__
	if (R.lock) SDL_LockMutex(R.lock);
#endif
}

void relay_unlock(void) {
#ifndef __EMSCRIPTEN__
	if (R.lock) SDL_UnlockMutex(R.lock);
#endif
}

static void progress(void) {
	relay_lock();
	R.shown_moved = (uint32_t)R.moved;
	R.shown_size = (uint32_t)(R.receiving || R.in ? R.in_n : R.out_n);
	relay_unlock();
}

void relay_progress(uint32_t *moved, uint32_t *size) {
	relay_lock();
	*moved = R.shown_moved;
	*size = R.shown_size;
	relay_unlock();
}

static bool link_send(const uint8_t *data, size_t n) {
#ifdef __EMSCRIPTEN__
	return R.open && online_browser_send(data, n);
#elif defined(NO_RELAY)
	(void)data;
	(void)n;
	return false;
#else
	return online_socket_send(data, n);
#endif
}

void relay_send_blob(void) { R.sending = true; }

bool relay_take_blob(uint32_t size) {
	free(R.in);
	R.in = malloc(size ? size : 1);
	R.in_n = size;
	R.moved = 0;
	if (!R.in) return false;
	R.receiving = true;
	progress();
	if (!size) {
		R.receiving = false;
		uint8_t *bytes = R.in;
		R.in = NULL;
		R.job->taken(bytes, 0);
	}
	return true;
}

void relay_reply(const uint8_t *d, size_t n) {
	if (link_send(d, n)) R.replied = true;
}

void relay_end(bool again) {
	R.ended = true;
	R.again = again;
}

#ifndef NO_RELAY

/* (the connection's end before the job said: why, to the job, once) */
static void lose(RelayLoss why) {
	if (R.ended) return;
	R.ended = true;
	R.again = false;
	R.job->lost(why);
}

/* A connection's start: everything of the last one let go */
static void reset(void) {
	free(R.in);
	R.in = NULL;
	R.in_n = R.moved = 0;
	R.out = NULL;
	R.out_n = 0;
	R.sending = R.receiving = R.ended = R.again = R.replied = false;
	R.started = R.heard = SDL_GetTicks();
	progress();
}

/* The blob's next bytes, as many as the link takes now */
static void pump(void) {
	while (R.sending && !R.ended && R.moved < R.out_n) {
		uint8_t m[ONLINE_MAX_MESSAGE];
		size_t k = R.out_n - R.moved < CHUNK ? R.out_n - R.moved : CHUNK;
		m[0] = ONLINE_TRANSFER_DATA;
		memcpy(m + 1, R.out + R.moved, k);
		if (!link_send(m, k + 1)) {
#ifndef __EMSCRIPTEN__
			lose(RELAY_UNREACHED);
#endif
			return;   /* (the browser's buffer full: again next frame) */
		}
		R.moved += k;
		R.heard = SDL_GetTicks();
		progress();
	}
	if (R.sending && R.moved >= R.out_n) R.sending = false;
}

/* A message from the server */
static void took(const uint8_t *d, size_t n) {
	R.heard = SDL_GetTicks();
	if (!n || R.ended) return;
	if (d[0] == ONLINE_TRANSFER_DATA) {
		size_t k = n - 1;
		if (!R.receiving || k > R.in_n - R.moved) { lose(RELAY_DAMAGED); return; }
		memcpy(R.in + R.moved, d + 1, k);
		R.moved += k;
		progress();
		if (R.moved < R.in_n) return;
		R.receiving = false;
		uint8_t *bytes = R.in;
		R.in = NULL;
		R.job->taken(bytes, R.in_n);
		return;
	}
	/* (an ERROR: a server from before the job's kind, which refuses any
	 * first word but HELLO, or one the job broke) */
	if (d[0] == ONLINE_ERROR) { lose(RELAY_REFUSED); return; }
	R.job->word(d, n);
}

static bool too_long(void) {
	uint32_t now = SDL_GetTicks();
	return now - R.heard > QUIET_MS || now - R.started > WHOLE_MS;
}

/* (the job's prepare for the next connection: false where it ended) */
static bool prepare(uint8_t first[RELAY_FIRST], size_t *first_n) {
	reset();
	if (R.job->prepare(first, first_n, &R.out, &R.out_n)) return true;
	R.ended = true;
	return false;
}

#endif

#ifdef NO_RELAY

bool relay_start(const RelayJob *job) {
	(void)job;
	return false;
}
bool relay_busy(void) { return false; }
void relay_poll(void) {}
void relay_stop(void) {}
void relay_wait(uint32_t ms) { (void)ms; }

#elif defined(__EMSCRIPTEN__)

static void browser_close(void) {
	if (R.open) online_browser_close();
	R.open = false;
}

static void browser_message(const uint8_t *d, size_t n) {
	took(d, n);
	if (R.ended) browser_close();
}

static void browser_closed(void) {
	R.open = false;
	online_browser_close();
	lose(RELAY_UNREACHED);
}

static void browser_dial(void) {
	uint8_t first[RELAY_FIRST];
	size_t n = 0;
	if (!prepare(first, &n)) return;
	R.open = online_browser_open(R.url, first, n, browser_message, browser_closed);
	if (!R.open) lose(RELAY_UNREACHED);
}

bool relay_start(const RelayJob *job) {
	if (R.running) return false;
	R.job = job;
	R.running = true;
	browser_dial();
	return true;
}

bool relay_busy(void) { return R.running; }

void relay_poll(void) {
	if (!R.running) return;
	if (R.ended) {
		browser_close();
		if (R.again) browser_dial();
		else {
			R.running = false;
			free(R.in);
			R.in = NULL;
		}
		return;
	}
	pump();
	if (too_long()) {
		lose(RELAY_UNREACHED);
		browser_close();
	}
}

void relay_stop(void) {
	browser_close();
	R.ended = true;
	R.running = false;
	free(R.in);
	R.in = NULL;
}

void relay_wait(uint32_t ms) { (void)ms; }

#else

static bool stopped(void) { return SDL_AtomicGet(&R.stop) != 0; }

/* One connection's exchange to its end */
static void exchange(void) {
	uint8_t m[ONLINE_MAX_MESSAGE];
	while (!R.ended && !stopped()) {
		pump();
		int got = online_socket_receive(m, sizeof m);
		if (got < 0) { lose(RELAY_UNREACHED); break; }
		if (got > 0) { took(m, (size_t)got); continue; }
		if (too_long()) { lose(RELAY_UNREACHED); break; }
		online_socket_wait(10);
	}
	/* (a reply given a moment to go before the socket closes) */
	for (uint32_t since = SDL_GetTicks(); R.replied && !stopped() && SDL_GetTicks() - since < 500; online_socket_wait(10))
		if (online_socket_receive(m, sizeof m) != 0) break;
}

static int worker(void *unused) {
	(void)unused;
	do {
		uint8_t first[RELAY_FIRST];
		size_t n = 0;
		if (!prepare(first, &n)) break;
		char why[160];
		if (online_socket_open(R.url, &R.stop, why, sizeof why) && online_socket_send(first, n)) exchange();
		else lose(RELAY_UNREACHED);
		online_socket_close();
	} while (!stopped() && R.ended && R.again);
	if (!R.ended) lose(RELAY_UNREACHED);
	free(R.in);
	R.in = NULL;
	SDL_AtomicSet(&R.done, 1);
	return 0;
}

bool relay_busy(void) { return R.thread && !SDL_AtomicGet(&R.done); }

bool relay_start(const RelayJob *job) {
	if (!R.lock || relay_busy()) return false;
	if (R.thread) SDL_WaitThread(R.thread, NULL);
	R.thread = NULL;
	R.job = job;
	SDL_AtomicSet(&R.stop, 0);
	SDL_AtomicSet(&R.done, 0);
	R.thread = SDL_CreateThread(worker, "relay", NULL);
	return R.thread != NULL;
}

void relay_poll(void) {}

void relay_stop(void) {
	if (!R.thread) return;
	SDL_AtomicSet(&R.stop, 1);
	SDL_WaitThread(R.thread, NULL);
	R.thread = NULL;
}

void relay_wait(uint32_t ms) {
	for (uint32_t since = SDL_GetTicks(); relay_busy() && SDL_GetTicks() - since < ms;) SDL_Delay(10);
}

#endif
