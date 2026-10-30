#include "online_stream.h"

#if defined(__3DS__) && defined(CW_3DS_CURL)
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/services/ps.h>
#include <curl/curl.h>
#include <stdio.h>
#include <string.h>

#include "analytics_roots.h"
#include "network_3ds.h"

static CURL *connection;
static SDL_atomic_t *stopping;
static bool network_held, random_held;

static int cancel_transfer(void *user, curl_off_t a, curl_off_t b, curl_off_t c, curl_off_t d) {
	(void)a; (void)b; (void)c; (void)d;
	return SDL_AtomicGet((SDL_atomic_t *)user);
}

bool online_stream_open(const char *url, SDL_atomic_t *stop, char *error, size_t size) {
	stopping = stop;
	network_held = network_3ds_begin();
	random_held = network_held && R_SUCCEEDED(psInit());
	if (!random_held || !(connection = curl_easy_init())) {
		SDL_strlcpy(error, "Could not initialize 3DS networking", size);
		return false;
	}
	bool secure = !strncmp(url, "wss://", 6);
	char address[512];
	snprintf(address, sizeof address, "%s%s", secure ? "https://" : "http://", url + (secure ? 6 : 5));
	curl_easy_setopt(connection, CURLOPT_URL, address);
	curl_easy_setopt(connection, CURLOPT_PROXY, "");
	curl_easy_setopt(connection, CURLOPT_CONNECT_ONLY, 1L);
	/* Our raw WebSocket Upgrade uses HTTP/1.1, including TLS ALPN. */
	curl_easy_setopt(connection, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(connection, CURLOPT_TCP_NODELAY, 1L);
	curl_easy_setopt(connection, CURLOPT_CONNECTTIMEOUT, 5L);
	curl_easy_setopt(connection, CURLOPT_TIMEOUT, 7L);
	curl_easy_setopt(connection, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(connection, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(connection, CURLOPT_SSL_VERIFYHOST, 2L);
	curl_easy_setopt(connection, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(connection, CURLOPT_XFERINFOFUNCTION, cancel_transfer);
	curl_easy_setopt(connection, CURLOPT_XFERINFODATA, (void *)stop);
	struct curl_blob roots = { analytics_roots, strlen(analytics_roots), CURL_BLOB_NOCOPY };
	curl_easy_setopt(connection, CURLOPT_CAINFO_BLOB, &roots);
	CURLcode result = curl_easy_perform(connection);
	if (result != CURLE_OK) snprintf(error, size, "Connect failed: %s", curl_easy_strerror(result));
	return result == CURLE_OK;
}

bool online_stream_send(const uint8_t *data, size_t size) {
	uint32_t started = SDL_GetTicks();
	while (size && !SDL_AtomicGet(stopping)) {
		size_t sent = 0;
		CURLcode result = curl_easy_send(connection, data, size, &sent);
		if (result != CURLE_OK && result != CURLE_AGAIN) return false;
		data += sent;
		size -= sent;
		if (SDL_GetTicks() - started >= 2000) return false;
		if (!sent) SDL_Delay(2);
	}
	return size == 0;
}

int online_stream_receive(uint8_t *data, size_t size) {
	size_t got = 0;
	CURLcode result = curl_easy_recv(connection, data, size, &got);
	if (result == CURLE_AGAIN) return 0;
	return result != CURLE_OK || !got ? -1 : (int)got;
}

bool online_stream_random(uint8_t *out, size_t size) {
	return random_held && size <= UINT32_MAX && R_SUCCEEDED(PS_GenerateRandomBytes(out, (u32)size));
}

void online_stream_close(void) {
	if (connection) curl_easy_cleanup(connection);
	if (random_held) psExit();
	if (network_held) network_3ds_end();
	connection = NULL;
	network_held = random_held = false;
}
/* (the stream is not one the worker can wait on here: looked at every
 * millisecond, a key sent at the next look; online_socket.h) */
void online_stream_wait(uint32_t ms) { SDL_Delay(ms ? 1 : 0); }
void online_stream_wake(void) {}
#endif
