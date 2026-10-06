/* One request at a time with the system's own library, for the systems
 * whose statistics go out on a thread of the game's (analytics_net.c):
 * libcurl on Linux and the PortMaster handhelds, loaded where it is there
 * (none: no statistics, never a start refused), linked in on the 3DS with
 * its own sockets and TLS (the 3DS's own SSL module stops at TLS 1.1,
 * which Umami's host refuses), and WinHTTP on Windows, which every
 * Windows has, loaded too (no DLL the .exe asks for at its start). */
#ifndef CW_ANALYTICS_HTTP_H
#define CW_ANALYTICS_HTTP_H

#include <stdbool.h>

#include <SDL.h>

/* Whether this system can post at all: its library loaded (and, in the
 * Flatpak, the network shared with it). The game's thread, at the start. */
bool analytics_http_ready(void);
/* `body` posted to `url` as JSON, `agent` its User-Agent, waiting for the
 * answer (ten seconds at most, and a second once `stop` is set, where the
 * library can be stopped): true for a 2xx. The statistics' thread. */
bool analytics_http_post(const char *url, const char *agent, const char *body, SDL_atomic_t *stop);
/* The library's own let go, on the thread that posted (the 3DS's sockets) */
void analytics_http_end(void);

#endif
