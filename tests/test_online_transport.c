/* ROM-free runtime exercise of the game's real native transport worker. */
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "network_prefs.h"
#include "online_transport.h"
#include "rom.h"

enum { SEEN_PEER = 1, SEEN_CARD = 2, SEEN_EVENT = 4, SEEN_CHANNEL = 8, SEEN_RETURN = 16 };

static unsigned observe(const OnlineSnapshot *s, unsigned seen) {
	for (size_t i = 0; i < s->peer_count; ++i) {
		const OnlinePeer *p = &s->peers[i];
		if (!strcmp(p->name, "ProbePeer") && p->x == 321 && p->y == -123 && p->facing == 7) seen |= SEEN_PEER;
	}
	if (s->card_serial && s->card_player == s->player_id && s->card.hp == 173 &&
	    s->card.max_hp == 300 && s->card.board[48] == 6) seen |= SEEN_CARD;
	for (size_t i = 0; i < ONLINE_EVENT_HISTORY; ++i)
		if (s->events[i].serial && s->events[i].player_id == s->player_id &&
		    s->events[i].kind == 1 && s->events[i].value == 5) seen |= SEEN_EVENT;
	if (s->channel == 2) seen |= SEEN_CHANNEL;
	if ((seen & SEEN_CHANNEL) && s->channel == 1) seen |= SEEN_RETURN;
	return seen;
}

static bool exercise(const OnlineSnapshot *s, unsigned seen, unsigned *sent, bool peer) {
	if (!*sent) {
		OnlineProfile p = { .hp = 173, .max_hp = 300, .attack = 4, .speed = 3, .charge = 2 };
		p.board[48] = 6;
		if (!online_transport_profile(&p) || !online_transport_inspect(s->player_id) ||
		    !online_transport_social(1, 5)) return false;
		*sent = 1;
	}
	unsigned needed = SEEN_CARD | SEEN_EVENT | (peer ? SEEN_PEER : 0);
	if (*sent == 1 && (seen & needed) == needed) {
		if (!online_transport_channel(2)) return false;
		*sent = 2;
	}
	if (*sent == 2 && (seen & SEEN_CHANNEL)) {
		if (!online_transport_channel(1)) return false;
		*sent = 3;
	}
	return true;
}

static bool session(const char *url, const char *mode, const uint8_t *identity) {
	bool refuse = !strcmp(mode, "refuse"), cancel = !strcmp(mode, "cancel");
	/* (disconnect: the link dropped without a word, tried again, the place taken back; malformed: never tried again) */
	bool disconnect = !strcmp(mode, "disconnect"), malformed = !strcmp(mode, "malformed"), peer = !strcmp(mode, "exchange");
	bool connected = false, passed = false, dropped = false;
	uint32_t first_id = 0;
	unsigned seen = 0, sent = 0;
	OnlineSnapshot snapshot = { 0 };
	uint64_t maximum_poll = 0;
	uint32_t started = SDL_GetTicks();
	if (!online_transport_start(url, "TransportProbe", identity)) return false;
	while (SDL_GetTicks() - started < 12000) {
		uint64_t before = SDL_GetPerformanceCounter();
		online_transport_poll(123, -234, 12, 3);
		online_transport_snapshot(&snapshot);
		uint64_t elapsed = SDL_GetPerformanceCounter() - before;
		if (elapsed > maximum_poll) maximum_poll = elapsed;
		if (snapshot.connected) {
			connected = true;
			if (!first_id) first_id = snapshot.player_id;
			if (refuse || cancel) break;
			seen = observe(&snapshot, seen);
			if (!disconnect && !malformed && !exercise(&snapshot, seen, &sent, peer)) break;
		}
		if (snapshot.reconnecting) dropped = true;
		if (disconnect && dropped && snapshot.resumes && snapshot.connected) { passed = snapshot.player_id == first_id; break; }
		if (cancel && SDL_GetTicks() - started >= 200) { passed = !connected; break; }
		if (snapshot.error[0]) {
			passed = (refuse && !connected) || (malformed && connected && !dropped);
			break;
		}
		unsigned needed = SEEN_CARD | SEEN_EVENT | SEEN_CHANNEL | SEEN_RETURN | (peer ? SEEN_PEER : 0);
		if (!disconnect && !malformed && !refuse && !cancel && (seen & needed) == needed &&
		    SDL_GetTicks() - started >= 2500 && snapshot.position_serial >= 20) { passed = true; break; }
		SDL_Delay(2);
	}
	uint32_t closing = SDL_GetTicks();
	online_transport_end();
	uint32_t close_ms = SDL_GetTicks() - closing;
	if (close_ms > 3000) passed = false;
	double poll_ms = (double)maximum_poll * 1000.0 / (double)SDL_GetPerformanceFrequency();
	printf("{\"passed\":%s,\"connected\":%s,\"seen\":%u,\"snapshots\":%u,"
	       "\"latency_ms\":%u,\"maximum_poll_ms\":%.4f,\"close_ms\":%u,\"resumes\":%u}\n",
	       passed ? "true" : "false", connected ? "true" : "false", seen,
	       snapshot.position_serial, snapshot.latency_ms, poll_ms, close_ms, snapshot.resumes);
	if (!passed || refuse) fprintf(stderr, "transport: %s\n", snapshot.error);
	return passed;
}

int main(int argc, char **argv) {
	if (argc != 5) { fprintf(stderr, "usage: probe URL MODE ROM_CODE ROM_SHA1\n"); return 2; }
	if (strlen(argv[3]) != 4 || strlen(argv[4]) != 40) return 2;
	RomLayout layout = { .code = argv[3], .sha1 = argv[4] };
	R.layout = &layout; /* Compatibility metadata only; no ROM is read. */
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_TIMER)) return 2;
	uint8_t identity[NETWORK_ID_BYTES];
	bool ok = network_identity(identity);
	/* Reopening in the same process catches stale callbacks and handles. */
	for (unsigned i = 0; i < 2 && ok; ++i) {
		ok = session(argv[1], argv[2], identity);
		SDL_Delay(150);
	}
	SDL_Quit();
	return ok ? 0 : 1;
}
