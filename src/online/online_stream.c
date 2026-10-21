#include "online_stream.h"

#include <stdio.h>
#include <string.h>

#if (defined(__linux__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)) || (defined(__APPLE__) && !defined(CW_IOS))

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

typedef void CURL;
enum {
	OPT_URL = 10002, OPT_PROXY = 10004, OPT_CONNECTTIMEOUT = 78, OPT_TIMEOUT = 13,
	OPT_NOSIGNAL = 99, OPT_CONNECT_ONLY = 141, OPT_NOPROGRESS = 43,
	OPT_XFERINFODATA = 10057, OPT_XFERINFOFUNCTION = 20219,
	OPT_SSL_VERIFYPEER = 64, OPT_SSL_VERIFYHOST = 81, OPT_HTTP_VERSION = 84, OPT_TCP_NODELAY = 121,
	INFO_ACTIVESOCKET = 0x500000 + 44,
	CURL_AGAIN = 81
};
static struct {
	void *lib;
	CURL *handle;
	FILE *random;
	SDL_atomic_t *stop;
	int (*global_init)(long);
	CURL *(*easy_init)(void);
	int (*easy_setopt)(CURL *, int, ...);
	int (*easy_perform)(CURL *);
	void (*easy_cleanup)(CURL *);
	int (*easy_send)(CURL *, const void *, size_t, size_t *);
	int (*easy_recv)(CURL *, void *, size_t, size_t *);
	int (*easy_getinfo)(CURL *, int, ...);
	const char *(*easy_strerror)(int);
	int fd;   /* the connection's socket, -1 where curl does not say it */
} stream;

/* the worker's wake (online_stream_wake): a pipe polled beside the socket,
 * made once and kept for the process, as the game's thread writes to it
 * whenever it likes */
static SDL_atomic_t wake_made;
static int wake_pipe[2] = { -1, -1 };

static bool load_curl(void) {
#ifdef __APPLE__
	static const char *const names[] = { "/usr/lib/libcurl.dylib", "libcurl.dylib" };
#else
	static const char *const names[] = { "libcurl.so.4", "libcurl-gnutls.so.4", "libcurl.so" };
#endif
	for (size_t i = 0; !stream.lib && i < sizeof names / sizeof *names; ++i) stream.lib = SDL_LoadObject(names[i]);
	if (!stream.lib) return false;
	stream.global_init = (int (*)(long))SDL_LoadFunction(stream.lib, "curl_global_init");
	stream.easy_init = (CURL * (*)(void))SDL_LoadFunction(stream.lib, "curl_easy_init");
	stream.easy_setopt = (int (*)(CURL *, int, ...))SDL_LoadFunction(stream.lib, "curl_easy_setopt");
	stream.easy_perform = (int (*)(CURL *))SDL_LoadFunction(stream.lib, "curl_easy_perform");
	stream.easy_cleanup = (void (*)(CURL *))SDL_LoadFunction(stream.lib, "curl_easy_cleanup");
	stream.easy_send = (int (*)(CURL *, const void *, size_t, size_t *))SDL_LoadFunction(stream.lib, "curl_easy_send");
	stream.easy_recv = (int (*)(CURL *, void *, size_t, size_t *))SDL_LoadFunction(stream.lib, "curl_easy_recv");
	stream.easy_strerror = (const char *(*)(int))SDL_LoadFunction(stream.lib, "curl_easy_strerror");
	stream.easy_getinfo = (int (*)(CURL *, int, ...))SDL_LoadFunction(stream.lib, "curl_easy_getinfo");
	return stream.global_init && stream.easy_init && stream.easy_setopt && stream.easy_perform &&
	       stream.easy_cleanup && stream.easy_send && stream.easy_recv && stream.easy_strerror;
}

static int cancelled(void *user, long long a, long long b, long long c, long long d) {
	(void)a; (void)b; (void)c; (void)d;
	return SDL_AtomicGet((SDL_atomic_t *)user);
}

bool online_stream_open(const char *url, SDL_atomic_t *stop, char *error, size_t size) {
	stream.stop = stop;
	if (!load_curl()) {
		SDL_strlcpy(error, "Online needs the system's libcurl", size);
		return false;
	}
	stream.random = fopen("/dev/urandom", "rb");
	if (!stream.random || stream.global_init(3L) || !(stream.handle = stream.easy_init())) {
		SDL_strlcpy(error, "Could not initialize online transport", size);
		return false;
	}
	char address[512];
	bool secure = !strncmp(url, "wss://", 6);
	snprintf(address, sizeof address, "%s%s", secure ? "https://" : "http://", url + (secure ? 6 : 5));
	stream.easy_setopt(stream.handle, OPT_URL, address);
	stream.easy_setopt(stream.handle, OPT_PROXY, "");
	stream.easy_setopt(stream.handle, OPT_CONNECT_ONLY, 1L);
	/* Our raw WebSocket Upgrade uses HTTP/1.1, including TLS ALPN. */
	stream.easy_setopt(stream.handle, OPT_HTTP_VERSION, 2L);
	stream.easy_setopt(stream.handle, OPT_CONNECTTIMEOUT, 5L);
	stream.easy_setopt(stream.handle, OPT_TIMEOUT, 7L);
	stream.easy_setopt(stream.handle, OPT_NOSIGNAL, 1L);
	stream.easy_setopt(stream.handle, OPT_SSL_VERIFYPEER, 1L);
	stream.easy_setopt(stream.handle, OPT_SSL_VERIFYHOST, 2L);
	stream.easy_setopt(stream.handle, OPT_NOPROGRESS, 0L);
	stream.easy_setopt(stream.handle, OPT_XFERINFOFUNCTION, cancelled);
	stream.easy_setopt(stream.handle, OPT_XFERINFODATA, (void *)stop);
	/* (no small send held back for the last one's acknowledgement: libcurl's
	 * default since 7.50.2, said for older ones; issue #167) */
	stream.easy_setopt(stream.handle, OPT_TCP_NODELAY, 1L);
	int result = stream.easy_perform(stream.handle);
	if (result) snprintf(error, size, "Connect failed: %s", stream.easy_strerror(result));
	/* (the socket, for the worker's rest: curl 7.45 and later say it) */
	int fd = -1;   /* (curl_socket_t, an int here) */
	stream.fd = -1;
	if (!result && stream.easy_getinfo && !stream.easy_getinfo(stream.handle, INFO_ACTIVESOCKET, &fd) && fd >= 0) stream.fd = fd;
	if (!SDL_AtomicGet(&wake_made) && !pipe(wake_pipe)) {
		fcntl(wake_pipe[0], F_SETFL, O_NONBLOCK);
		fcntl(wake_pipe[1], F_SETFL, O_NONBLOCK);
		SDL_AtomicSet(&wake_made, 1);
	}
	return result == 0;
}

bool online_stream_send(const uint8_t *data, size_t size) {
	uint32_t started = SDL_GetTicks();
	while (size && !SDL_AtomicGet(stream.stop)) {
		size_t sent = 0;
		int result = stream.easy_send(stream.handle, data, size, &sent);
		if (result != 0 && result != CURL_AGAIN) return false;
		data += sent;
		size -= sent;
		if (SDL_GetTicks() - started >= 2000) return false;
		if (!sent) SDL_Delay(2);
	}
	return size == 0;
}

int online_stream_receive(uint8_t *data, size_t size) {
	size_t got = 0;
	int result = stream.easy_recv(stream.handle, data, size, &got);
	if (result == CURL_AGAIN) return 0;
	if (result || !got) return -1;
	return (int)got;
}

bool online_stream_random(uint8_t *out, size_t size) {
	return stream.random && fread(out, 1, size, stream.random) == size;
}

void online_stream_close(void) {
	if (stream.handle && stream.easy_cleanup) stream.easy_cleanup(stream.handle);
	if (stream.random) fclose(stream.random);
	/* Other modules also load libcurl. Let the process keep its global state. */
	if (stream.lib) SDL_UnloadObject(stream.lib);
	memset(&stream, 0, sizeof stream);
	stream.fd = -1;
}

void online_stream_wait(uint32_t ms) {
	struct pollfd look[2] = { { .fd = stream.fd, .events = POLLIN }, { .fd = -1, .events = POLLIN } };
	if (SDL_AtomicGet(&wake_made)) look[1].fd = wake_pipe[0];
	/* (curl without its socket said: looked at every millisecond) */
	if (stream.fd < 0 && ms > 1) ms = 1;
	if (poll(look, 2, (int)ms) > 0 && look[1].revents) {
		char drained[64];
		while (read(wake_pipe[0], drained, sizeof drained) > 0) {}
	}
}

void online_stream_wake(void) {
	if (!SDL_AtomicGet(&wake_made)) return;
	char one = 1;
	/* (a full pipe already wakes the worker) */
	if (write(wake_pipe[1], &one, 1) < 0) return;
}

#elif !defined(__ANDROID__) && (!defined(__3DS__) || !defined(CW_3DS_CURL))

bool online_stream_open(const char *url, SDL_atomic_t *stop, char *error, size_t size) {
	(void)url; (void)stop;
	SDL_strlcpy(error, "Online Phase 0 is unavailable on this platform", size);
	return false;
}
bool online_stream_send(const uint8_t *data, size_t size) { (void)data; (void)size; return false; }
int online_stream_receive(uint8_t *data, size_t size) { (void)data; (void)size; return -1; }
bool online_stream_random(uint8_t *out, size_t size) { (void)out; (void)size; return false; }
void online_stream_close(void) {}
void online_stream_wait(uint32_t ms) { SDL_Delay(ms ? 1 : 0); }
void online_stream_wake(void) {}

#endif
