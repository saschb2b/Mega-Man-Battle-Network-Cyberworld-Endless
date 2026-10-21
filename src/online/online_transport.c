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

/* Trade Central's (its board's too), the Arena's and the stands' messages (protocol.json): sent
 * as soon as the worker's next pass, and taken in apart from the snapshot, each a u16
 * length and its bytes in a ring, with room for a watched battle's keys as a spectator
 * who came late catches up (the server's 2 KB a tick, the game's frame held up a few
 * ticks by the way in) */
#define RING 65536
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
	return kind == ONLINE_TRADE_STATE || kind == ONLINE_BOARD || kind == ONLINE_WEEK_BOARD || kind == ONLINE_ARENA_STATE || kind == ONLINE_ARENA_LOADOUT ||
		kind == ONLINE_ARENA_INPUT || kind == ONLINE_ARENA_HASH || kind == ONLINE_STANDS || kind == ONLINE_WATCH_STATE ||
		kind == ONLINE_WATCH_RELAY || kind == ONLINE_WATCH_KEYS;
}

/* A dropped link (issue #153, online_transport.h): a link welcomed and then
 * silent this long is given up (snapshots come 20 times a second, PONGs every
 * two), and tried again after a wait that doubles from the first to the
 * most; an unwelcomed one (a try's handshake, the gate's call) waits less */
#define DEAD_MS 8000
#define DEAD_CALLING_MS 5000
#define RETRY_FIRST_MS 500
#define RETRY_MOST_MS 4000
#define REST_MS 10           /* the worker's longest rest between two looks at the link: its position, pings and timers kept to it */

static struct {
	Ring outbox, inbox;
	bool enabled;
	SDL_Thread *thread;
	SDL_mutex *mutex;
	SDL_atomic_t stop, bye, done;
	OnlineSnapshot snapshot;
	OnlinePeer local;
	char url[512], name[ONLINE_NAME_BYTES + 1];
	uint8_t hello[ONLINE_HELLO_BYTES + ONLINE_NAME_BYTES];
	struct { uint8_t data[ONLINE_PROFILE_BYTES + 1]; size_t size; } queue[16];
	unsigned queue_read, queue_write;
	size_t hello_size;
	uint32_t last_send, last_ping, last_receive;
	uint32_t last_poll, hitch;   /* (the browser's: its last poll, and the longest gap between two since the last ping) */
	uint32_t retry_at;           /* (the browser's: its next try, a link dropped) */
	unsigned tries;
	bool dialing;                /* (the browser's: a socket open, a try's or the visit's) */
} transport;

/* how often MegaMan's place is sent: 20 times a second, twice a second
 * once nothing has been heard for 400 ms, so a stalled link's backlog,
 * let through at once as it clears, stays small (the server's own bucket
 * for it: server/cyberworld_server/holds.py, BACKLOG) */
static uint32_t send_every(uint32_t now, uint32_t last_receive) { return now - last_receive > 400 ? 500u : 50u; }

/* the wait before try `tries` (0 the first): 0.5, 1, 2, then 4 seconds */
static uint32_t retry_wait(unsigned tries) {
	uint32_t wait = RETRY_FIRST_MS << (tries < 3 ? tries : 3);
	return wait < RETRY_MOST_MS ? wait : RETRY_MOST_MS;
}

/* HELLO as a resume: the channel MegaMan stood in, the resume bit set, so
 * the server gives back the place it held (protocol v10) */
static void hello_resume(uint16_t channel) {
	online_put16(transport.hello + 73, (uint16_t)(channel | ONLINE_HELLO_RESUME));
}

/* the link ended without a word from the server (no ERROR, nothing
 * unreadable) after it welcomed this game: tried again */
static bool resumable(const OnlineSnapshot *s) { return s->player_id && !s->error[0]; }

/* the link gone quiet or dropped at `lost_at`: the snapshot says it is
 * being tried again */
static void reconnecting(OnlineSnapshot *s, uint32_t lost_at) {
	s->connected = false;
	s->quiet = s->reconnecting = true;
	s->lost_at = lost_at;
}

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
	if (!size || !mail(data[0])) {
		bool ok = online_wire_message(data, size, now, snapshot);
		/* (an unreadable message, not the network: never tried again) */
		if (!ok && !snapshot->error[0]) SDL_strlcpy(snapshot->error, "The server sent an unreadable message", sizeof snapshot->error);
		/* (a resume's WELCOME: what came in from the last session and was not
		 * taken yet is set aside, the server having ended it all at the drop;
		 * the new session's own follows) */
		if (ok && data[0] == ONLINE_WELCOME && snapshot->reconnecting) {
			if (transport.mutex) SDL_LockMutex(transport.mutex);
			transport.inbox.tail = transport.inbox.head;
			if (transport.mutex) SDL_UnlockMutex(transport.mutex);
		}
		/* (a resume done: its WELCOME, then the peers as they stand) */
		if (ok && data[0] == ONLINE_SNAPSHOT && snapshot->reconnecting) {
			snapshot->reconnecting = snapshot->quiet = false;
			++snapshot->resumes;
		}
		return ok;
	}
	if (!snapshot->connected) {
		SDL_strlcpy(snapshot->error, "The server sent a message out of order", sizeof snapshot->error);
		return false;
	}
	if (transport.mutex) SDL_LockMutex(transport.mutex);
	bool room = ring_put(&transport.inbox, data, size);
	if (transport.mutex) SDL_UnlockMutex(transport.mutex);
	if (!room) SDL_strlcpy(snapshot->error, "The server sent more than this game could take in", sizeof snapshot->error);
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
/* the link quiet past ONLINE_QUIET_MS, or heard again: told (not while a
 * resume is under way, which is quiet till it is done) */
static void hush(OnlineSnapshot *snapshot, uint32_t now, uint32_t last_receive) {
	bool quiet = snapshot->connected && now - last_receive > ONLINE_QUIET_MS;
	if (snapshot->reconnecting || quiet == snapshot->quiet) return;
	snapshot->quiet = quiet;
	if (quiet) snapshot->lost_at = last_receive;
	publish(snapshot);
}

/* the session until it ends: the ticks it was last heard from */
static uint32_t connected_loop(OnlineSnapshot *snapshot) {
	uint32_t last_send = 0, last_ping = 0, last_receive = SDL_GetTicks();
	for (;;) {
		uint8_t data[ONLINE_MAX_MESSAGE];
		/* (leaving: the goodbye, so the server frees the place at once;
		 * asked for before the stop, so it is sent even where the stop
		 * came first: a slow system's end waits only so long) */
		if (SDL_AtomicGet(&transport.bye)) {
			/* (sent whether or not the WELCOME is in yet: this socket's
			 * HELLO went first, and a resume the server took before its
			 * WELCOME reached the game would otherwise be held) */
			data[0] = ONLINE_BYE;
			/* (then the server's close, its answer to it, before the
			 * socket goes: a socket closed at once can be reset with the
			 * goodbye still unread, as Windows' are; half a second at
			 * most, as the game is leaving) */
			if (online_socket_send(data, 1))
				for (uint32_t started = SDL_GetTicks(); online_socket_receive(data, sizeof data) >= 0 &&
				                                        SDL_GetTicks() - started < 500;)
					SDL_Delay(2);
			break;
		}
		if (SDL_AtomicGet(&transport.stop)) break;
		/* (every message come in, then the rest: not one a rest, issue #167) */
		int got;
		uint32_t now = SDL_GetTicks();
		bool taken = true;
		while (taken && (got = online_socket_receive(data, sizeof data)) > 0) {
			now = SDL_GetTicks();
			last_receive = now;
			taken = take_in(data, (size_t)got, now, snapshot);
			publish(snapshot);
		}
		if (!taken) break;
		if (got == -2) SDL_strlcpy(snapshot->error, "The server broke the WebSocket protocol", sizeof snapshot->error);
		if (got < 0) break;
		hush(snapshot, now, last_receive);
		if (now - last_receive > (snapshot->connected ? DEAD_MS : DEAD_CALLING_MS)) break;
		if (snapshot->connected && !send_mail()) break;
		if (snapshot->connected && now - last_send >= send_every(now, last_receive)) {
			last_send = now;
			if (!send_position() || !send_queued()) break;
		}
		if (snapshot->connected && now - last_ping >= 2000) {
			last_ping = now;
			data[0] = ONLINE_PING;
			online_put32(data + 1, now);
			if (!online_socket_send(data, 5)) break;
		}
		/* (until a message comes in or one is put to go out: SDL_Delay(2)
		 * before issue #167, a key waiting up to 2 ms at each end) */
		online_socket_wait(REST_MS);
	}
	return last_receive;
}

#else

/* the socket closed, broken or given up: a link welcomed and dropped
 * without a word is tried again (online_transport.h), the rest end */
static void browser_closed(void) {
	online_browser_close();
	transport.dialing = false;
	OnlineSnapshot *s = &transport.snapshot;
	uint32_t now = SDL_GetTicks();
	if (resumable(s) && !s->reconnecting) {
		reconnecting(s, s->quiet ? s->lost_at : now);
		transport.tries = 0;
	}
	if (resumable(s) && now - s->lost_at < ONLINE_RECONNECT_MS) {
		transport.retry_at = now + retry_wait(transport.tries++);
		return;
	}
	s->connected = false;
	s->peer_count = 0;
	if (!s->error[0]) SDL_strlcpy(s->error, "Disconnected from the square", sizeof s->error);
}

static void browser_received(const uint8_t *data, size_t size);

/* the next try, its time come: a resume's HELLO on a new socket */
static void browser_retry(uint32_t now) {
	if (transport.dialing || (int32_t)(now - transport.retry_at) < 0) return;
	hello_resume(transport.snapshot.channel);
	transport.last_receive = now;
	transport.dialing = online_browser_open(transport.url, transport.hello, transport.hello_size, browser_received, browser_closed);
	if (!transport.dialing) browser_closed();
}

/* (a frame of the page longer than this between a ping and its answer:
 * the round trip says the frame, not the network) */
#define BROWSER_HITCH_MS 50

static void browser_hitch(uint32_t now) {
	if (transport.last_poll && now - transport.last_poll > transport.hitch) transport.hitch = now - transport.last_poll;
}

static void browser_received(const uint8_t *data, size_t size) {
	transport.last_receive = SDL_GetTicks();
	uint32_t latency = transport.snapshot.latency_ms;
	if (!take_in(data, size, transport.last_receive, &transport.snapshot)) browser_closed();
	/* (the page's one thread takes the answer in after its frame: a ping
	 * that spanned a long one, the Square built at the arrival among them,
	 * is left out of the round trips a NetBattle's delay is asked by) */
	browser_hitch(transport.last_receive);
	if (size && data[0] == ONLINE_PONG && transport.hitch > BROWSER_HITCH_MS) transport.snapshot.latency_ms = latency;
}

static void browser_poll(void) {
	OnlineSnapshot *s = &transport.snapshot;
	if (s->error[0]) return;
	uint32_t now = SDL_GetTicks();
	/* (the page held up past a second, a tab in the background: the time
	 * it was away is not the link's) */
	if (transport.last_poll && now - transport.last_poll > 1000) transport.last_receive = now;
	browser_hitch(now);
	transport.last_poll = now;
	/* (between a dropped link's tries: the next one, or given up) */
	if (!transport.dialing) {
		if (now - s->lost_at >= ONLINE_RECONNECT_MS) browser_closed();
		else browser_retry(now);
		return;
	}
	bool quiet = s->connected && now - transport.last_receive > ONLINE_QUIET_MS;
	if (!s->reconnecting && quiet != s->quiet) {
		s->quiet = quiet;
		if (quiet) s->lost_at = transport.last_receive;
	}
	if (now - transport.last_receive > (s->connected ? DEAD_MS : 7000u)) {
		browser_closed();
		return;
	}
	if (!s->connected) return;
	if (now - transport.last_send >= send_every(now, transport.last_receive)) {
		transport.last_send = now;
		if (!send_position() || !send_queued()) browser_closed();
	}
	if (now - transport.last_ping >= 2000) {
		uint8_t data[5] = { ONLINE_PING };
		transport.last_ping = now;
		transport.hitch = 0;
		online_put32(data + 1, now);
		if (!send_message(data, sizeof data)) browser_closed();
	}
}

#endif

#ifndef __EMSCRIPTEN__
static bool leaving(void) { return SDL_AtomicGet(&transport.stop) || SDL_AtomicGet(&transport.bye); }

/* the wait before the next try, `wait` and no later than the window's
 * end: false where the game is leaving meanwhile */
static bool rest(uint32_t lost_at, uint32_t wait) {
	uint32_t until = SDL_GetTicks() + wait, last = lost_at + ONLINE_RECONNECT_MS;
	if ((int32_t)(until - last) > 0) until = last;
	while ((int32_t)(SDL_GetTicks() - until) < 0) {
		if (leaving()) return false;
		SDL_Delay(10);
	}
	return true;
}

/* the session, and where its link drops without a word, its tries again
 * (online_transport.h); the snapshot's error once it is over */
static int worker(void *unused) {
	(void)unused;
	OnlineSnapshot snapshot = { 0 };
	char why[128];
	unsigned tries = 0;
	for (;;) {
		why[0] = '\0';
		if (online_socket_open(transport.url, &transport.stop, why, sizeof why) &&
		    online_socket_send(transport.hello, transport.hello_size)) connected_loop(&snapshot);
		online_socket_close();
		if (!snapshot.reconnecting) tries = 0;
		if (leaving() || !resumable(&snapshot)) break;
		uint32_t now = SDL_GetTicks();
		if (!snapshot.reconnecting) reconnecting(&snapshot, snapshot.quiet ? snapshot.lost_at : now);
		/* (what was waiting to go is the last session's: dropped with it) */
		SDL_LockMutex(transport.mutex);
		transport.outbox.tail = transport.outbox.head;
		SDL_UnlockMutex(transport.mutex);
		publish(&snapshot);
		if (now - snapshot.lost_at >= ONLINE_RECONNECT_MS) break;
		fprintf(stderr, "online: the link dropped%s%s; trying again\n", why[0] ? ": " : "", why);
		if (!rest(snapshot.lost_at, retry_wait(tries++))) break;
		hello_resume(snapshot.channel);
	}
	if (!snapshot.error[0]) SDL_strlcpy(snapshot.error, why[0] ? why : "Disconnected from the square", sizeof snapshot.error);
	snapshot.connected = false;
	snapshot.peer_count = 0;
	publish(&snapshot);
	if (!leaving()) fprintf(stderr, "online: %s\n", snapshot.error);
	SDL_AtomicSet(&transport.done, 1);
	return 0;
}

/* the worker's link up, as last published */
static bool linked(void) {
	SDL_LockMutex(transport.mutex);
	bool up = transport.snapshot.connected;
	SDL_UnlockMutex(transport.mutex);
	return up;
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
	transport.dialing = online_browser_open(url, transport.hello, transport.hello_size, browser_received, browser_closed);
	if (transport.dialing) return true;
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
	bool ours = size && (data[0] == ONLINE_TRADE || data[0] == ONLINE_OFFER || data[0] == ONLINE_WEEK_POST || data[0] == ONLINE_ARENA || data[0] == ONLINE_ARENA_LOADOUT ||
		data[0] == ONLINE_ARENA_INPUT || data[0] == ONLINE_ARENA_HASH || data[0] == ONLINE_ARENA_STATUS || data[0] == ONLINE_WATCH);
	if (!transport.enabled || !ours) return false;
	/* (a link down: nothing kept for the next session, which the server
	 * began afresh, what was under way ended at the drop; issue #153) */
#ifdef __EMSCRIPTEN__
	return transport.snapshot.connected && online_browser_send(data, size);
#else
	SDL_LockMutex(transport.mutex);
	bool room = transport.snapshot.connected && ring_put(&transport.outbox, data, size);
	SDL_UnlockMutex(transport.mutex);
	/* (sent now, not at the worker's next look) */
	if (room) online_socket_wake();
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
	/* (leaving with the link up: BYE first, so the server frees the place
	 * at once rather than holding it for a return; protocol v10) */
#ifdef __EMSCRIPTEN__
	uint8_t bye = ONLINE_BYE;
	if (transport.snapshot.connected) online_browser_send(&bye, 1);
	online_browser_close();
#else
	if (transport.thread) {
		SDL_AtomicSet(&transport.bye, 1);
		online_socket_wake();
		uint32_t started = SDL_GetTicks();
		while (!SDL_AtomicGet(&transport.done) && linked() && SDL_GetTicks() - started < 1000) SDL_Delay(1);
	}
	SDL_AtomicSet(&transport.stop, 1);
	if (transport.thread) online_socket_wake();
	if (transport.thread) SDL_WaitThread(transport.thread, NULL);
	if (transport.mutex) SDL_DestroyMutex(transport.mutex);
#endif
	memset(&transport, 0, sizeof transport);
}
