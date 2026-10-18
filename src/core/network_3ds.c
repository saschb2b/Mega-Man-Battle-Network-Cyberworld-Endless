#include "network_3ds.h"

#if defined(__3DS__) && defined(CW_3DS_CURL)
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/services/soc.h>
#include <3ds/services/sslc.h>
#include <curl/curl.h>
#include <malloc.h>
#include <stdlib.h>
#include <SDL.h>

#define NETWORK_SOCKET_BYTES 0x100000
static SDL_SpinLock guard;
static unsigned references;
static u32 *socket_buffer;
static bool socket_owned, ssl_owned, curl_ready;

static void release_services(void) {
	if (ssl_owned) sslcExit();
	if (socket_owned) socExit();
	free(socket_buffer);
	socket_buffer = NULL;
	socket_owned = ssl_owned = false;
}

bool network_3ds_begin(void) {
	SDL_AtomicLock(&guard);
	if (references) {
		++references;
		SDL_AtomicUnlock(&guard);
		return true;
	}
	socket_buffer = memalign(0x1000, NETWORK_SOCKET_BYTES);
	Result result = socket_buffer ? socInit(socket_buffer, NETWORK_SOCKET_BYTES) : -1;
	socket_owned = R_SUCCEEDED(result);
	bool sockets = socket_owned || (socket_buffer && R_DESCRIPTION(result) == RD_ALREADY_INITIALIZED);
	if (!socket_owned) { free(socket_buffer); socket_buffer = NULL; }
	ssl_owned = sockets && R_SUCCEEDED(sslcInit(0));
	if (ssl_owned && !curl_ready) curl_ready = curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK;
	if (ssl_owned && curl_ready) references = 1;
	else release_services();
	bool ready = references != 0;
	SDL_AtomicUnlock(&guard);
	return ready;
}

void network_3ds_end(void) {
	SDL_AtomicLock(&guard);
	if (references && !--references) release_services();
	SDL_AtomicUnlock(&guard);
}

#endif
