/* transfer.h. One exchange at a time, a small machine both kinds of link
 * drive: each message the server sends handed to took(), the blob's bytes
 * sent by pump() while the link takes them. On a thread of its own where
 * the system's socket waits (online_socket.h); on the browser's main
 * thread, its callbacks and SAVES' frames moving it on. */
#include "transfer.h"

#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "network_prefs.h"
#include "online_browser.h"
#include "online_socket.h"
#include "protocol.h"
#include "transfer_box.h"

/* (the systems with no socket of the Square's: online_stream.c's) */
#if defined(__PSP__) || (defined(__3DS__) && !defined(CW_3DS_CURL))
#define NO_LINK 1
#endif

#ifndef NO_LINK

#define TRIES 3            /* codes made where the server says one is in use */
#define QUIET_MS 15000     /* the server's next word waited for at most (its own limit) */
#define WHOLE_MS 120000    /* an exchange at most */
#define CHUNK (ONLINE_MAX_MESSAGE - 1)

static struct {
	char url[NETWORK_URL];
	bool put;              /* a send's (else a receive's) */
	TransferKeys keys;
	char code[TRANSFER_DIGITS + 1];
	uint8_t *plain;        /* a send's saves (sealed afresh for each code); a receive's once opened */
	size_t plain_n;
	uint8_t *blob;         /* the sealed bytes going or coming */
	size_t blob_n, moved;
	bool sending;          /* READY came: the blob's DATA goes */
	bool receiving;        /* FOUND came: its DATA comes */
	bool again;            /* TAKEN came: another code */
	bool over;             /* this connection's exchange is done */
	bool got;              /* a receive's saves opened (the thread's own till it ends) */
	int tries;
	uint32_t started, heard;
	TransferState st;      /* (the thread's link: under `lock`) */
#ifdef __EMSCRIPTEN__
	bool open;
#else
	SDL_Thread *thread;
	SDL_mutex *lock;
	SDL_atomic_t stop;
#endif
} T;

static void lock(void) {
#ifndef __EMSCRIPTEN__
	SDL_LockMutex(T.lock);
#endif
}

static void unlock(void) {
#ifndef __EMSCRIPTEN__
	SDL_UnlockMutex(T.lock);
#endif
}

/* (the random bytes: the system's own, as the Square's key takes them) */
static bool random16(uint8_t out[16]) { return network_identity(out); }

static void finish(TransferPhase phase, TransferTrouble trouble, uint32_t value) {
	T.over = true;
	lock();
	T.st.phase = phase;
	T.st.trouble = trouble;
	if (phase == TRANSFER_SENT) {
		memcpy(T.st.code, T.code, sizeof T.st.code);
		T.st.until = SDL_GetTicks() + value * 1000u;
	}
	if (trouble == TRANSFER_BUSY) T.st.wait_s = value;
	unlock();
}

static void progress(void) {
	lock();
	T.st.moved = (uint32_t)T.moved;
	T.st.size = (uint32_t)T.blob_n;
	unlock();
}

static bool link_send(const uint8_t *data, size_t n) {
#ifdef __EMSCRIPTEN__
	return T.open && online_browser_send(data, n);
#else
	return online_socket_send(data, n);
#endif
}

/* TRANSFER: put, get or done under the lookup */
static void transfer_message(uint8_t op, uint8_t out[ONLINE_TRANSFER_BYTES]) {
	uint32_t size = op == ONLINE_TRANSFER_PUT ? (uint32_t)T.blob_n : 0;
	out[0] = ONLINE_TRANSFER;
	out[1] = ONLINE_TRANSFER_VERSION;
	out[2] = op;
	memcpy(out + 3, T.keys.lookup, ONLINE_TRANSFER_LOOKUP_BYTES);
	for (int i = 0; i < 4; ++i) out[19 + i] = (uint8_t)(size >> (24 - 8 * i));
}

/* A connection's start: a send's new code, its keys and its saves sealed
 * with them; a receive's keys. False (and finished) where it cannot be. */
static bool prepare(void) {
	T.moved = 0;
	T.sending = T.receiving = T.again = T.over = false;
	T.started = T.heard = SDL_GetTicks();
	if (T.put) {
		free(T.blob);
		T.blob = NULL;
		if (T.plain_n + TRANSFER_HEAD > ONLINE_TRANSFER_MAX) { finish(TRANSFER_FAILED, TRANSFER_TOO_BIG, 0); return false; }
		if (!transfer_code_make(T.code, random16) || !transfer_keys(T.code, &T.keys) ||
		    !(T.blob = transfer_seal(&T.keys, T.plain, T.plain_n, random16, &T.blob_n))) {
			finish(TRANSFER_FAILED, TRANSFER_NO_MEMORY, 0);
			return false;
		}
	} else if (!transfer_keys(T.code, &T.keys)) {
		finish(TRANSFER_FAILED, TRANSFER_NO_MEMORY, 0);
		return false;
	}
	progress();
	return true;
}

/* The blob's next bytes, as many as the link takes now */
static void pump(void) {
	while (T.sending && !T.over && T.moved < T.blob_n) {
		uint8_t m[ONLINE_MAX_MESSAGE];
		size_t k = T.blob_n - T.moved < CHUNK ? T.blob_n - T.moved : CHUNK;
		m[0] = ONLINE_TRANSFER_DATA;
		memcpy(m + 1, T.blob + T.moved, k);
		if (!link_send(m, k + 1)) {
#ifndef __EMSCRIPTEN__
			finish(TRANSFER_FAILED, TRANSFER_UNREACHED, 0);
#endif
			return;   /* (the browser's buffer full: again next frame) */
		}
		T.moved += k;
		T.heard = SDL_GetTicks();
		progress();
	}
	if (T.moved >= T.blob_n) T.sending = false;
}

/* A receive's last byte in: opened, let go on the server (DONE), here */
static void opened(void) {
	size_t n = 0;
	if (!transfer_open(&T.keys, T.blob, T.blob_n, &n)) { finish(TRANSFER_FAILED, TRANSFER_DAMAGED, 0); return; }
	T.plain = T.blob;
	T.plain_n = n;
	T.blob = NULL;
	T.got = true;
	uint8_t m[ONLINE_TRANSFER_BYTES];
	transfer_message(ONLINE_TRANSFER_DONE, m);
	(void)link_send(m, sizeof m);   /* (not let go: gone in its 15 minutes anyway) */
	finish(TRANSFER_RECEIVED, TRANSFER_FINE, 0);
}

/* TRANSFER_STATE's word */
static void took_state(uint8_t status, uint32_t value) {
	switch (status) {
	case ONLINE_TRANSFER_READY:
		if (T.put) T.sending = true;
		else finish(TRANSFER_FAILED, TRANSFER_DAMAGED, 0);
		break;
	case ONLINE_TRANSFER_HELD: finish(T.put ? TRANSFER_SENT : TRANSFER_FAILED, T.put ? TRANSFER_FINE : TRANSFER_DAMAGED, value); break;
	case ONLINE_TRANSFER_FOUND:
		if (T.put || T.receiving || value < TRANSFER_HEAD || value > ONLINE_TRANSFER_MAX) {
			finish(TRANSFER_FAILED, TRANSFER_DAMAGED, 0);
			break;
		}
		if (!(T.blob = malloc(value))) { finish(TRANSFER_FAILED, TRANSFER_NO_MEMORY, 0); break; }
		T.blob_n = value;
		T.receiving = true;
		progress();
		break;
	case ONLINE_TRANSFER_NONE: finish(TRANSFER_FAILED, TRANSFER_NO_SAVES, 0); break;
	case ONLINE_TRANSFER_BUSY: finish(TRANSFER_FAILED, TRANSFER_BUSY, value); break;
	case ONLINE_TRANSFER_TAKEN:
		T.again = T.put;
		T.over = true;
		break;
	case ONLINE_TRANSFER_OFF: finish(TRANSFER_FAILED, TRANSFER_OFF, 0); break;
	case ONLINE_TRANSFER_REMOVED: break;
	default: finish(TRANSFER_FAILED, TRANSFER_DAMAGED, 0); break;
	}
}

/* A message from the server */
static void took(const uint8_t *d, size_t n) {
	T.heard = SDL_GetTicks();
	if (!n || T.over) return;
	if (d[0] == ONLINE_TRANSFER_DATA && T.receiving) {
		size_t k = n - 1;
		if (k > T.blob_n - T.moved) { finish(TRANSFER_FAILED, TRANSFER_DAMAGED, 0); return; }
		memcpy(T.blob + T.moved, d + 1, k);
		T.moved += k;
		progress();
		if (T.moved == T.blob_n) opened();
		return;
	}
	/* (an ERROR: a server from before transfers, which refuses any first
	 * word but HELLO) */
	if (d[0] == ONLINE_ERROR) { finish(TRANSFER_FAILED, TRANSFER_OFF, 0); return; }
	if (d[0] != ONLINE_TRANSFER_STATE || n != ONLINE_TRANSFER_STATE_BYTES) { finish(TRANSFER_FAILED, TRANSFER_DAMAGED, 0); return; }
	took_state(d[1], (uint32_t)d[2] << 24 | (uint32_t)d[3] << 16 | (uint32_t)d[4] << 8 | d[5]);
}

/* (the time an exchange may wait: a quiet server, or one too long) */
static bool too_long(void) {
	uint32_t now = SDL_GetTicks();
	return now - T.heard > QUIET_MS || now - T.started > WHOLE_MS;
}

/* Everything an exchange held let go, its state too (`keep_state` keeps it) */
static void clear(bool keep_state) {
	free(T.plain);
	free(T.blob);
	T.plain = T.blob = NULL;
	T.plain_n = T.blob_n = T.moved = 0;
	T.got = false;
	SDL_memset(&T.keys, 0, sizeof T.keys);
	if (!keep_state) {
		lock();
		SDL_memset(&T.st, 0, sizeof T.st);
		unlock();
	}
}

/* (a send's saves sealed again for another code where TAKEN came: TRIES
 * at most, then the server is as good as full) */
static bool another_try(void) {
	if (!T.again) return false;
	if (++T.tries < TRIES) return true;
	finish(TRANSFER_FAILED, TRANSFER_BUSY, 60);
	return false;
}

#ifdef __EMSCRIPTEN__

static void browser_close(void) {
	if (T.open) online_browser_close();
	T.open = false;
}

static void browser_message(const uint8_t *d, size_t n) {
	took(d, n);
	if (T.over && !T.again) browser_close();
}

static void browser_closed(void) {
	T.open = false;
	online_browser_close();
	if (!T.over) finish(TRANSFER_FAILED, TRANSFER_UNREACHED, 0);
}

static void browser_dial(void) {
	if (!prepare()) return;
	uint8_t m[ONLINE_TRANSFER_BYTES];
	transfer_message(T.put ? ONLINE_TRANSFER_PUT : ONLINE_TRANSFER_GET, m);
	T.open = online_browser_open(T.url, m, sizeof m, browser_message, browser_closed);
	if (!T.open) finish(TRANSFER_FAILED, TRANSFER_UNREACHED, 0);
}

static void begin(void) { browser_dial(); }

/* (a frame's look: the blob's next bytes, the time, another code) */
static void move_on(void) {
	if (T.st.phase != TRANSFER_WORKING) return;
	if (T.over) {
		browser_close();
		if (another_try()) browser_dial();
		return;
	}
	pump();
	if (too_long()) {
		finish(TRANSFER_FAILED, TRANSFER_UNREACHED, 0);
		browser_close();
	}
}

static void join(void) {}

static void stop_all(void) { browser_close(); }

#else

/* One connection's exchange to its end */
static void exchange(void) {
	uint8_t m[ONLINE_MAX_MESSAGE];
	while (!T.over && !SDL_AtomicGet(&T.stop)) {
		pump();
		int got = online_socket_receive(m, sizeof m);
		if (got < 0) { finish(TRANSFER_FAILED, TRANSFER_UNREACHED, 0); break; }
		if (got > 0) { took(m, (size_t)got); continue; }
		if (too_long()) { finish(TRANSFER_FAILED, TRANSFER_UNREACHED, 0); break; }
		online_socket_wait(10);
	}
	/* (a receive's DONE given a moment to go before the socket closes) */
	for (uint32_t since = SDL_GetTicks(); T.got && !SDL_AtomicGet(&T.stop) && SDL_GetTicks() - since < 500; online_socket_wait(10))
		if (online_socket_receive(m, sizeof m) != 0) break;
}

static int worker(void *unused) {
	(void)unused;
	do {
		if (!prepare()) break;
		char why[160];
		uint8_t m[ONLINE_TRANSFER_BYTES];
		transfer_message(T.put ? ONLINE_TRANSFER_PUT : ONLINE_TRANSFER_GET, m);
		if (online_socket_open(T.url, &T.stop, why, sizeof why) && online_socket_send(m, sizeof m)) exchange();
		else if (!SDL_AtomicGet(&T.stop)) finish(TRANSFER_FAILED, TRANSFER_UNREACHED, 0);
		online_socket_close();
	} while (!SDL_AtomicGet(&T.stop) && another_try());
	if (!T.over) finish(TRANSFER_FAILED, TRANSFER_UNREACHED, 0);
	return 0;
}

static void begin(void) {
	SDL_AtomicSet(&T.stop, 0);
	T.thread = SDL_CreateThread(worker, "transfer", NULL);
	if (!T.thread) finish(TRANSFER_FAILED, TRANSFER_NO_MEMORY, 0);
}

static void move_on(void) {}

/* (the thread let finish: at once where it has, else stopped) */
static void join(void) {
	if (!T.thread) return;
	SDL_AtomicSet(&T.stop, 1);
	SDL_WaitThread(T.thread, NULL);
	T.thread = NULL;
}

static void stop_all(void) { join(); }

#endif

static bool under_way(void) {
	lock();
	bool working = T.st.phase == TRANSFER_WORKING;
	unlock();
	return working;
}

/* An exchange begun (the last one's let go first, by its caller) */
static bool start(bool put) {
	T.put = put;
	T.tries = 0;
	lock();
	T.st.phase = TRANSFER_WORKING;
	unlock();
	begin();
	return true;
}

static bool link_send_saves(uint8_t *bytes, size_t n) {
	if (!bytes || under_way()) {
		free(bytes);
		return false;
	}
	join();
	clear(false);
	T.plain = bytes;
	T.plain_n = n;
	return start(true);
}

static bool link_receive(const char *code) {
	if (!transfer_code_valid(code) || under_way()) return false;
	join();
	clear(false);
	memcpy(T.code, code, sizeof T.code);
	return start(false);
}

static void link_state(TransferState *out) {
	move_on();
	lock();
	*out = T.st;
	unlock();
}

static uint8_t *link_take(size_t *n) {
	lock();
	bool received = T.st.phase == TRANSFER_RECEIVED;
	unlock();
	*n = 0;
	if (!received) return NULL;
	/* (the thread done with them: it touches nothing once RECEIVED) */
	join();
	uint8_t *bytes = T.plain;
	*n = T.plain_n;
	T.plain = NULL;
	clear(false);
	return bytes;
}

static void link_cancel(void) {
	stop_all();
	clear(false);
}

static const TransferLink LINK = { link_send_saves, link_receive, link_state, link_take, link_cancel };

const TransferLink *online_transfer_link(const char *url) {
	if (!url || SDL_strlen(url) >= sizeof T.url) return NULL;
	SDL_strlcpy(T.url, url, sizeof T.url);
#ifndef __EMSCRIPTEN__
	if (!T.lock && !(T.lock = SDL_CreateMutex())) return NULL;
#endif
	return &LINK;
}

#else

const TransferLink *online_transfer_link(const char *url) {
	(void)url;
	return NULL;
}

#endif
