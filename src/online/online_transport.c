#include "online_transport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "layer_make.h"
#include "online_browser.h"
#include "online_socket.h"
#include "rom.h"

static struct {
	bool enabled;
	SDL_Thread *thread;
	SDL_mutex *mutex;
	SDL_atomic_t stop;
	OnlineSnapshot snapshot;
	OnlinePeer local;
	char url[512], name[ONLINE_NAME_BYTES + 1];
	uint8_t hello[64];
	size_t hello_size;
	uint32_t last_send, last_ping, last_receive;
} transport;

static void put16(uint8_t *out, uint16_t value) {
	out[0] = (uint8_t)(value >> 8); out[1] = (uint8_t)value;
}

static void put32(uint8_t *out, uint32_t value) {
	out[0] = (uint8_t)(value >> 24); out[1] = (uint8_t)(value >> 16);
	out[2] = (uint8_t)(value >> 8); out[3] = (uint8_t)value;
}

static uint32_t get32(const uint8_t *in) {
	return (uint32_t)in[0] << 24 | (uint32_t)in[1] << 16 | (uint32_t)in[2] << 8 | in[3];
}

static int32_t signed32(const uint8_t *in) {
	uint32_t value = get32(in);
	return (int32_t)((int64_t)(value & INT32_MAX) - (int64_t)(value >> 31) * 2147483648LL);
}

static bool valid_name(const char *name) {
	size_t size = strlen(name);
	if (!size || size > ONLINE_NAME_BYTES) return false;
	for (size_t i = 0; i < size; ++i) {
		char c = name[i];
		bool alnum = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
		if (!alnum && (!i || (c != ' ' && c != '_' && c != '-'))) return false;
	}
	return true;
}

static bool build_hello(const char *name) {
	if (!R.layout || !R.layout->code || !R.layout->sha1 || !valid_name(name)) return false;
	uint8_t *out = transport.hello;
	out[0] = ONLINE_HELLO;
	put16(out + 1, ONLINE_PROTOCOL_VERSION);
	put16(out + 3, LAYER_MAKE);
	put32(out + 5, LAYER_MAKE_HASH);
	memcpy(out + 9, R.layout->code, 4);
	for (size_t i = 0; i < 20; ++i) {
		char pair[3] = { R.layout->sha1[i * 2], R.layout->sha1[i * 2 + 1], 0 };
		out[13 + i] = (uint8_t)strtoul(pair, NULL, 16);
	}
	size_t length = strlen(name);
	out[33] = (uint8_t)length;
	memcpy(out + 34, name, length);
	transport.hello_size = 34 + length;
	return true;
}

static bool read_peers(const uint8_t *data, size_t size, OnlineSnapshot *snapshot) {
	if (size < 2 || data[1] >= ONLINE_MAX_PLAYERS || size != 2 + (size_t)data[1] * 33) return false;
	snapshot->peer_count = data[1];
	for (size_t i = 0; i < snapshot->peer_count; ++i) {
		const uint8_t *in = data + 2 + i * 33;
		OnlinePeer *peer = &snapshot->peers[i];
		peer->id = get32(in);
		peer->x = signed32(in + 4); peer->y = signed32(in + 8); peer->z = signed32(in + 12);
		peer->facing = in[16];
		memcpy(peer->name, in + 17, ONLINE_NAME_BYTES);
		peer->name[ONLINE_NAME_BYTES] = '\0';
		if (!peer->id || peer->id == snapshot->player_id || peer->facing > 7 || !valid_name(peer->name)) return false;
		if (peer->x < -ONLINE_COORDINATE_LIMIT || peer->x > ONLINE_COORDINATE_LIMIT ||
		    peer->y < -ONLINE_COORDINATE_LIMIT || peer->y > ONLINE_COORDINATE_LIMIT ||
		    peer->z < -ONLINE_COORDINATE_LIMIT || peer->z > ONLINE_COORDINATE_LIMIT) return false;
		for (size_t j = 0; j < i; ++j) if (snapshot->peers[j].id == peer->id) return false;
	}
	return true;
}

static bool message(const uint8_t *data, size_t size, OnlineSnapshot *snapshot) {
	if (data[0] == ONLINE_WELCOME && !snapshot->connected && size == 12) {
		snapshot->seed = get32(data + 1);
		snapshot->player_id = get32(data + 5);
		if (!snapshot->player_id || !data[11] || data[11] > ONLINE_MAX_PLAYERS) return false;
		snapshot->connected = true;
		fprintf(stderr, "online: connected id=%u seed=%u\n", snapshot->player_id, snapshot->seed);
		return true;
	}
	if (data[0] == ONLINE_ERROR && size >= 3 && size <= 99) {
		size_t length = size - 3;
		for (size_t i = 0; i < length; ++i) snapshot->error[i] = data[i + 3] >= 32 && data[i + 3] < 127 ? (char)data[i + 3] : '?';
		snapshot->error[length] = '\0';
		return false;
	}
	if (!snapshot->connected) return false;
	if (data[0] == ONLINE_SNAPSHOT) return read_peers(data, size, snapshot);
	if (data[0] == ONLINE_PONG && size == 5) {
		snapshot->latency_ms = SDL_GetTicks() - get32(data + 1);
		return snapshot->latency_ms < 15000;
	}
	return false;
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
	put32(data + 1, (uint32_t)local.x); put32(data + 5, (uint32_t)local.y); put32(data + 9, (uint32_t)local.z);
	data[13] = local.facing;
	return send_message(data, sizeof data);
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
			if (!message(data, (size_t)got, snapshot)) break;
			publish(snapshot);
		}
		if (now - last_receive > (snapshot->connected ? 15000u : 5000u)) break;
		if (snapshot->connected && now - last_send >= 50) {
			last_send = now;
			if (!send_position()) break;
		}
		if (snapshot->connected && now - last_ping >= 2000) {
			last_ping = now;
			data[0] = ONLINE_PING;
			put32(data + 1, now);
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
	if (!message(data, size, &transport.snapshot)) browser_closed();
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
		if (!send_position()) browser_closed();
	}
	if (now - transport.last_ping >= 2000) {
		uint8_t data[5] = { ONLINE_PING };
		transport.last_ping = now;
		put32(data + 1, now);
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

bool online_transport_start(const char *url, const char *name) {
	online_transport_end();
	transport.enabled = true;
	if (!url || !name || strlen(url) >= sizeof transport.url || !build_hello(name)) {
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

bool online_transport_enabled(void) { return transport.enabled; }

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
