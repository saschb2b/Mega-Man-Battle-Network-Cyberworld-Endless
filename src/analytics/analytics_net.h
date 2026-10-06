/* The statistics sent on each system (analytics.h), never in a frame's
 * time: on a thread of the game's where the system's library waits for
 * an answer (Linux, the PortMaster handhelds, Windows, the 3DS:
 * analytics_http.c), else by the system's own asynchronous call (Apple's
 * NSURLSession, analytics_apple.m; Android's HttpURLConnection on an
 * executor of GameActivity's; the browser's fetch). A request is tried
 * once; a full queue or a network given up on drops what comes. Where the
 * system cannot send at all, the game never asks. */
#ifndef CW_ANALYTICS_NET_H
#define CW_ANALYTICS_NET_H

#include <stdbool.h>

/* The session's statistics, once the window and settings.ini are there
 * (startup.c): `headless` and `plain` (no test's or developer's option)
 * as main.c started it */
void analytics_net_start(bool headless, bool plain);
/* The game's end (platform_shutdown): a request under way given up, the
 * thread waited for a moment (on the 3DS to its end, as a 3DS app must) */
void analytics_net_quit(void);

#ifdef __APPLE__
/* (analytics_apple.m) `body` posted to `url`, `agent` its User-Agent;
 * `done` is called on the session's own queue once it is through */
bool analytics_apple_post(const char *url, const char *agent, const char *body, void (*done)(bool ok));
#endif

#endif
