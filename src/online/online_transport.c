#include "online_transport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "online_browser.h"
#include "online_socket.h"
#include "online_wire.h"
#include "rom.h"
#include "square_plan.h"

/* Trade Central's (its board's too) and the Arena's messages (protocol.json): sent as soon as
 * the worker's next pass, and taken in apart from the snapshot, each a u16
 * length and its bytes in a ring */
#define RING 16384
typedef struct { uint8_t b[RING]; unsigned head, tail; } Ring;

static bool ring_put(Ring *r, const uint8_t *data, size_t size) {
	if (!size || size > ONLINE_MAX_MESSAGE || RING - (r->head - r->tail) < size + 2) return false;
	uint8_t len[2] = { (uint8_t)(size >> 8), (uint8_t)size };
	for (size_t i = 0; i < 2; ++i) r->b[r->head++ % RING] = len[i];
	for (size_t i = 0; i < size; ++i) r->b[r->head++ % RING] = data[i];
	return true;
}

static size_t ring_take(Ring *r, uint8_t *out, size_t cap) {
	if (r->head == r->tail) return 0;
	size_t size = (size_t)r->b[r->tail % RING] << 8 | r->b[(r->tail + 1) % RING];
	if (size > cap) { r->tail += 2 + (unsigned)size; return 0; }
	r->tail += 2;
	for (size_t i = 0; i < size; ++i) out[i] = r->b[r->tail++ % RING];
	return size;
}

static bool mail(uint8_t kind) {
	return kind == ONLINE_TRADE_STATE || kind == ONLINE_BOARD || kind == ONLINE_ARENA_STATE || kind == ONLINE_ARENA_LOADOUT ||
		kind == ONLINE_ARENA_INPUT || kind == ONLINE_ARENA_HASH;
}

static struct {
	Ring outbox, inbox;
	bool enabled;
	SDL_Thread *thread;
	SDL_mutex *mutex;
	SDL_atomic_t stop;
	OnlineSnapshot snapshot;
	OnlinePeer local;
	char url[512], name[ONLINE_NAME_BYTES + 1];
	uint8_t hello[ONLINE_HELLO_BYTES + ONLINE_NAME_BYTES];
	struct { uint8_t data[ONLINE_PROFILE_BYTES + 1]; size_t size; } queue[16];
	unsigned queue_read, queue_write;
	size_t hello_size;
	uint32_t last_send, last_ping, last_receive;
} transport;

/* a ROM's 40 hex digits as its 20 bytes */
static void sha1_bytes(const char *hex, uint8_t *out) {
	for (size_t i = 0; i < 20; ++i) {
		char pair[3] = { hex[i * 2], hex[i * 2 + 1], 0 };
		out[i] = (uint8_t)strtoul(pair, NULL, 16);
	}
}

uint8_t online_transport_roms(void) { return XR[XROM_BN5_COLONEL_US].data ? ONLINE_ROMS_BN5 : 0; }

/* HELLO (shared/online/README.md): BN6's identity, and BN5's where its ROM
 * is read (zeros where not), the server's to check and to tell the others */
static bool build_hello(const char *name, const uint8_t identity[ONLINE_IDENTITY_BYTES]) {
	if (!R.layout || !R.layout->code || !R.layout->sha1 || !online_wire_name(name)) return false;
	uint8_t *out = transport.hello;
	memset(out, 0, sizeof transport.hello);
	out[0] = ONLINE_HELLO;
	online_put16(out + 1, ONLINE_PROTOCOL_VERSION);
	online_put16(out + 3, SQUARE_MAKE);
	online_put32(out + 5, SQUARE_HASH);
	memcpy(out + 9, R.layout->code, 4);
	sha1_bytes(R.layout->sha1, out + 13);
	const XRom *bn5 = &XR[XROM_BN5_COLONEL_US];
	if (online_transport_roms() & ONLINE_ROMS_BN5) {
		memcpy(out + 33, bn5->layout->code, 4);
		sha1_bytes(bn5->layout->sha1, out + 37);
	}
	size_t length = strlen(name);
	memcpy(out + 57, identity, ONLINE_IDENTITY_BYTES);
	online_put16(out + 73, 0);
	out[75] = (uint8_t)length;
	memcpy(out + ONLINE_HELLO_BYTES, name, length);
	transport.hello_size = ONLINE_HELLO_BYTES + length;
	return true;
}

static bool send_message(const uint8_t *data, size_t size) {
#ifdef __EMSCRIPTEN__
	return online_browser_send(data, size);
#else
	return online_socket_send(data, size);
#endif
}

#ifndef __EMSCRIPTEN__
static void publish(const OnlineSnapshot *snapshot) {
	SDL_LockMutex(transport.mutex);
	transport.snapshot = *snapshot;
	SDL_UnlockMutex(transport.mutex);
}
#endif

static bool send_position(void) {
	OnlinePeer local;
	if (transport.mutex) SDL_LockMutex(transport.mutex);
	local = transport.local;
	if (transport.mutex) SDL_UnlockMutex(transport.mutex);
	uint8_t data[14];
	data[0] = ONLINE_STATE;
	online_put32(data + 1, (uint32_t)local.x); online_put32(data + 5, (uint32_t)local.y); online_put32(data + 9, (uint32_t)local.z);
	data[13] = local.facing;
	return send_message(data, sizeof data);
}

static bool send_queued(void) {
	uint8_t data[ONLINE_PROFILE_BYTES + 1];
	size_t size = 0;
	if (transport.mutex) SDL_LockMutex(transport.mutex);
	if (transport.queue_read != transport.queue_write) {
		unsigned slot = transport.queue_read++ % 16;
		size = transport.queue[slot].size;
		memcpy(data, transport.queue[slot].data, size);
	}
	if (transport.mutex) SDL_UnlockMutex(transport.mutex);
	return !size || send_message(data, size);
}

#ifndef __EMSCRIPTEN__
/* the outbox, all of it (the browser sends each at once) */
static bool send_mail(void) {
	uint8_t data[ONLINE_MAX_MESSAGE];
	for (;;) {
		if (transport.mutex) SDL_LockMutex(transport.mutex);
		size_t size = ring_take(&transport.outbox, data, sizeof data);
		if (transport.mutex) SDL_UnlockMutex(transport.mutex);
		if (!size) return true;
		if (!send_message(data, size)) return false;
	}
}
#endif

/* a message come in: Trade Central's and the Arena's to the inbox, the
 * rest into the snapshot (false: the connection is to end) */
static bool take_in(const uint8_t *data, size_t size, uint32_t now, OnlineSnapshot *snapshot) {
	if (!size || !mail(data[0])) return online_wire_message(data, size, now, snapshot);
	if (!snapshot->connected) return false;
	if (transport.mutex) SDL_LockMutex(transport.mutex);
	bool room = ring_put(&transport.inbox, data, size);
	if (transport.mutex) SDL_UnlockMutex(transport.mutex);
	return room;
}

static bool enqueue(const uint8_t *data, size_t size) {
	if (!transport.enabled || size > sizeof transport.queue[0].data) return false;
	if (transport.mutex) SDL_LockMutex(transport.mutex);
	bool room = transport.queue_write - transport.queue_read < 16;
	if (room) {
		unsigned slot = transport.queue_write++ % 16;
		transport.queue[slot].size = size;
		memcpy(transport.queue[slot].data, data, size);
	}
	if (transport.mutex) SDL_UnlockMutex(transport.mutex);
	return room;
}


#ifndef __EMSCRIPTEN__
static void connected_loop(OnlineSnapshot *snapshot) {
	uint32_t last_send = 0, last_ping = 0, last_receive = SDL_GetTicks();
	while (!SDL_AtomicGet(&transport.stop)) {
		uint8_t data[ONLINE_MAX_MESSAGE];
		int got = online_socket_receive(data, sizeof data);
		uint32_t now = SDL_GetTicks();
		if (got < 0) break;
		if (got) {
			last_receive = now;
			if (!take_in(data, (size_t)got, now, snapshot)) break;
			publish(snapshot);
		}
		if (now - last_receive > (snapshot->connected ? 15000u : 5000u)) break;
		if (snapshot->connected && !send_mail()) break;
		if (snapshot->connected && now - last_send >= 50) {
			last_send = now;
			if (!send_position() || !send_queued()) break;
		}
		if (snapshot->connected && now - last_ping >= 2000) {
			last_ping = now;
			data[0] = ONLINE_PING;
			online_put32(data + 1, now);
			if (!online_socket_send(data, 5)) break;
		}
		SDL_Delay(2);
	}
}

#else

static void browser_closed(void) {
	transport.snapshot.connected = false;
	transport.snapshot.peer_count = 0;
	if (!transport.snapshot.error[0]) SDL_strlcpy(transport.snapshot.error, "Disconnected from the square", sizeof transport.snapshot.error);
	online_browser_close();
}

static void browser_received(const uint8_t *data, size_t size) {
	transport.last_receive = SDL_GetTicks();
	if (!take_in(data, size, transport.last_receive, &transport.snapshot)) browser_closed();
}

static void browser_poll(void) {
	if (transport.snapshot.error[0]) return;
	uint32_t now = SDL_GetTicks();
	if (now - transport.last_receive > (transport.snapshot.connected ? 15000u : 7000u)) {
		browser_closed();
		return;
	}
	if (!transport.snapshot.connected) return;
	if (now - transport.last_send >= 50) {
		transport.last_send = now;
		if (!send_position() || !send_queued()) browser_closed();
	}
	if (now - transport.last_ping >= 2000) {
		uint8_t data[5] = { ONLINE_PING };
		transport.last_ping = now;
		online_put32(data + 1, now);
		if (!send_message(data, sizeof data)) browser_closed();
	}
}

#endif

#ifndef __EMSCRIPTEN__
static int worker(void *unused) {
	(void)unused;
	OnlineSnapshot snapshot = { 0 };
	if (online_socket_open(transport.url, &transport.stop, snapshot.error, sizeof snapshot.error) &&
	    online_socket_send(transport.hello, transport.hello_size)) connected_loop(&snapshot);
	if (!snapshot.error[0]) SDL_strlcpy(snapshot.error, "Disconnected from the square", sizeof snapshot.error);
	snapshot.connected = false;
	snapshot.peer_count = 0;
	publish(&snapshot);
	if (!SDL_AtomicGet(&transport.stop)) fprintf(stderr, "online: %s\n", snapshot.error);
	online_socket_close();
	return 0;
}
#endif

bool online_transport_start(const char *url, const char *name, const uint8_t identity[ONLINE_IDENTITY_BYTES]) {
	online_transport_end();
	transport.enabled = true;
	if (!url || !name || !identity || strlen(url) >= sizeof transport.url || !build_hello(name, identity)) {
		SDL_strlcpy(transport.snapshot.error, "Online needs BN6 and a 1-16 character name", sizeof transport.snapshot.error);
		return false;
	}
	SDL_strlcpy(transport.url, url, sizeof transport.url);
	SDL_strlcpy(transport.name, name, sizeof transport.name);
#ifdef __EMSCRIPTEN__
	transport.last_receive = SDL_GetTicks();
	if (online_browser_open(url, transport.hello, transport.hello_size, browser_received, browser_closed)) return true;
	SDL_strlcpy(transport.snapshot.error, "Could not open browser WebSocket", sizeof transport.snapshot.error);
	return false;
#else
	transport.mutex = SDL_CreateMutex();
	if (transport.mutex) transport.thread = SDL_CreateThread(worker, "online", NULL);
	if (transport.thread) return true;
	SDL_strlcpy(transport.snapshot.error, "Could not start online worker", sizeof transport.snapshot.error);
	return false;
#endif
}


void online_transport_poll(int32_t x, int32_t y, int32_t z, uint8_t facing) {
	if (!transport.enabled) return;
	if (transport.mutex) SDL_LockMutex(transport.mutex);
	transport.local.x = x; transport.local.y = y; transport.local.z = z; transport.local.facing = facing;
	if (transport.mutex) SDL_UnlockMutex(transport.mutex);
}

void online_transport_snapshot(OnlineSnapshot *out) {
#ifdef __EMSCRIPTEN__
	/* Poll during connection setup too, before the Square exists. */
	if (transport.enabled) browser_poll();
#endif
	if (transport.mutex) SDL_LockMutex(transport.mutex);
	*out = transport.snapshot;
	if (transport.mutex) SDL_UnlockMutex(transport.mutex);
}

bool online_transport_profile(const OnlineProfile *profile) {
	uint8_t data[ONLINE_PROFILE_BYTES + 1];
	return online_wire_profile(data, profile) && enqueue(data, sizeof data);
}

bool online_transport_inspect(uint32_t player_id) {
	uint8_t data[5] = { ONLINE_INSPECT };
	online_put32(data + 1, player_id);
	return player_id && enqueue(data, sizeof data);
}

bool online_transport_social(uint8_t kind, uint8_t value) {
	uint8_t data[3] = { ONLINE_SOCIAL, kind, value };
	return kind <= 1 && value < (kind ? ONLINE_EMOTE_COUNT : ONLINE_PHRASE_COUNT) && enqueue(data, sizeof data);
}

bool online_transport_channel(uint16_t channel) {
	uint8_t data[3] = { ONLINE_CHANNEL };
	online_put16(data + 1, channel);
	return enqueue(data, sizeof data);
}

bool online_transport_send(const uint8_t *data, size_t size) {
	bool ours = size && (data[0] == ONLINE_TRADE || data[0] == ONLINE_OFFER || data[0] == ONLINE_ARENA || data[0] == ONLINE_ARENA_LOADOUT ||
		data[0] == ONLINE_ARENA_INPUT || data[0] == ONLINE_ARENA_HASH);
	if (!transport.enabled || !ours) return false;
#ifdef __EMSCRIPTEN__
	return online_browser_send(data, size);
#else
	SDL_LockMutex(transport.mutex);
	bool room = ring_put(&transport.outbox, data, size);
	SDL_UnlockMutex(transport.mutex);
	return room;
#endif
}

size_t online_transport_receive(uint8_t *out, size_t cap) {
#ifdef __EMSCRIPTEN__
	if (transport.enabled) browser_poll();
#endif
	if (transport.mutex) SDL_LockMutex(transport.mutex);
	size_t size = ring_take(&transport.inbox, out, cap);
	if (transport.mutex) SDL_UnlockMutex(transport.mutex);
	return size;
}

void online_transport_end(void) {
#ifdef __EMSCRIPTEN__
	online_browser_close();
#else
	SDL_AtomicSet(&transport.stop, 1);
	if (transport.thread) SDL_WaitThread(transport.thread, NULL);
	if (transport.mutex) SDL_DestroyMutex(transport.mutex);
#endif
	memset(&transport, 0, sizeof transport);
}
