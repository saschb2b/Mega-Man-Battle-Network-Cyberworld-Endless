/* One request at a time with the system's own library, for the systems
 * whose statistics go out on a thread of the game's (analytics_net.c),
 * and the updater's look for a new version and its download there:
 * libcurl on Linux and the PortMaster handhelds, loaded where it is there
 * (none: no statistics, never a start refused), linked in on the 3DS with
 * its own sockets and TLS (the 3DS's own SSL module stops at TLS 1.1,
 * which Umami's host refuses), and WinHTTP on Windows, which every
 * Windows has, loaded too (no DLL the .exe asks for at its start). */
#ifndef CW_ANALYTICS_HTTP_H
#define CW_ANALYTICS_HTTP_H

#include <stdbool.h>

#include <SDL.h>

/* (the systems that have it: libcurl's, Linux's and the PortMaster
 * handhelds' loaded, the consoles' linked in (the 3DS's where its build
 * found devkitPro's: the Makefile's CW_3DS_CURL); Windows' WinHTTP) */
#if (defined(__3DS__) && defined(CW_3DS_CURL)) || defined(__SWITCH__) || defined(__vita__) || defined(CW_PS5) || \
	(defined(__linux__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)) || defined(_WIN32)
#define ANALYTICS_HTTP 1
#endif

/* Whether this system can post at all: its library loaded (and, in the
 * Flatpak, the network shared with it). The game's thread, at the start. */
bool analytics_http_ready(void);
/* `body` posted to `url` as JSON, `agent` its User-Agent, waiting for the
 * answer (ten seconds at most, and a second once `stop` is set, where the
 * library can be stopped): true for a 2xx. The statistics' thread. */
bool analytics_http_post(const char *url, const char *agent, const char *body, SDL_atomic_t *stop);
/* A GET of `url`, its redirects followed, the answer's body handed to
 * `sink` as it comes (its false stops it): true for a 2xx read to its end.
 * Given up where nothing comes for twenty seconds, or within a second of
 * `stop`. The updater's (src/update/update.c), on a thread of its own:
 * where the game carries roots, GitHub's with them (analytics_roots.h);
 * on the 3DS the network held for the GET alone. */
typedef bool (*AnalyticsSink)(void *user, const void *data, size_t n);
bool analytics_http_get(const char *url, const char *agent, AnalyticsSink sink, void *user, SDL_atomic_t *stop);
/* The library's own let go, on the thread that posted (the 3DS's sockets) */
void analytics_http_end(void);

#endif
