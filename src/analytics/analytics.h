/* The anonymous statistics (issue #104, README.md's Anonymous statistics):
 * a few events of the game and its runs, sent to the game's own website on
 * the owner's Umami once the player has said yes. The title asks once, at
 * the first start where the build can send (analytics_ask.c); the answer is
 * settings.ini's statistics line, changed on the controls screen (on the
 * 3DS, SELECT on the title). Nothing is sent before a yes, and nothing that
 * tells who plays or on which machine: what was done, never by whom, as the
 * site's own analytics (web/assets/site.js).
 *
 * This part decides and writes; the systems send (analytics_net.c), off the
 * frame loop. Each event is one request to Umami's /api/send, as its own
 * tracker sends them:
 *
 *   (a view)    the game's start: its system as the address's path
 *               ("/linux", "/3ds"), its version as the page's title, the
 *               screen's size
 *   run-start   a new run's setup: net, folder, cross, threat, helpers,
 *               whether BN5 joins (bn5), the runs finished before (runs)
 *   guardian    a guardian's battle over: who, result (won, lost or left),
 *               layer, net, threat, MegaMan's HP left in percent (hp), the
 *               battle's seconds, the how-manyth time MegaMan meets him in
 *               any run (attempt)
 *   run-end     a run over: won, layer, area, by (who deleted MegaMan:
 *               a guardian, ProtoMan or viruses), minutes played, and its
 *               setup as at its start
 *
 * Every event names the same system path and version, so Umami's reports
 * filter by either. */
#ifndef CW_ANALYTICS_H
#define CW_ANALYTICS_H

#include <stdbool.h>
#include <stddef.h>

/* Where they go: the game's own website on the owner's Umami (its reports
 * apart from the site's), the host its events name (it need not resolve),
 * and Umami's own way in */
#define GAME_UMAMI_WEBSITE "18eb1237-ad57-4b04-be8b-1cee21266f5a"
#define GAME_UMAMI_HOSTNAME "cyberworld-endless.saschb2b.com"
#define GAME_UMAMI_SEND "https://umami.saschb2b.com/api/send"

/* The player's answer: none yet, yes, no (settings.ini's statistics line) */
enum { ANALYTICS_UNASKED, ANALYTICS_ON, ANALYTICS_OFF };

/* What a session knows as it starts (analytics_net.c) */
typedef struct {
	const char *platform;   /* the address's path: "linux", "windows", "3ds", ... */
	const char *version;    /* the build's (CW_VERSION) */
	int screen_w, screen_h; /* the display's size, its long side first; 0 unknown */
	const char *settings;   /* settings.ini, where the answer is kept */
	bool plain;             /* a player's start: not headless, no test's or developer's option */
	/* whether the system can send at all (a library, a network), asked
	 * only where the session may send */
	bool (*ready)(void);
	/* a payload handed to the system to send, which may drop it (a full
	 * queue, a network given up on): false then */
	bool (*post)(const char *json);
	/* what waits to be sent dropped (the player said no) */
	void (*drop)(void);
	/* files were written (platform_persist), or NULL */
	void (*persist)(void);
} AnalyticsStart;

/* The session's statistics: the answer read, and the start's view sent
 * where it was yes. */
void analytics_begin(const AnalyticsStart *s);
/* --dev statistics=ask|on|off: a test's session asks, or holds that
 * answer, as a player's would (it sends nothing but to statsurl's);
 * statsurl=URL: payloads go there (a mock server's); statscheck: one
 * event named game-dev-check, at /dev-check, whatever the answer and
 * nothing else, to check a system's way to Umami. True where `option` was
 * one of these. */
bool analytics_dev(const char *option);
/* Where payloads go: Umami's, or statsurl's */
const char *analytics_url(void);
/* ... and whether this is statscheck's session (its outcome printed) */
bool analytics_checking(void);

/* Whether this session can send: the build, the system, and a player's
 * start (or a test's switch); the controls screen's row shows then */
bool analytics_here(void);
int analytics_consent(void);
/* ... and no answer yet: the title asks */
bool analytics_unasked(void);
/* The player's answer, kept in settings.ini: a yes sends the start's view
 * (once a session), a no drops what waits to be sent. */
void analytics_answer(bool yes);

/* The events, where the answer is yes: a new run's start (its setup made,
 * meta_run_begun) ... */
void analytics_run_start(void);
/* ... a guardian's battle over: `navi`, how it ended, MegaMan's HP after
 * and its most, the battle's frames (0 unknown) */
enum { ANALYTICS_WON, ANALYTICS_LOST, ANALYTICS_LEFT };
void analytics_guardian(int navi, int result, int hp, int max_hp, int frames);
/* ... a run over: won, or lost and `by` whom ("viruses", a guardian's
 * name, "ProtoMan") */
void analytics_run_end(bool won, const char *by);

/* A payload's most bytes, and the system's queue of them (its lock held
 * round each call): ANALYTICS_QUEUE at most wait, more are dropped, never
 * sent again */
#define ANALYTICS_PAYLOAD 1024
#define ANALYTICS_QUEUE 8
typedef struct {
	char body[ANALYTICS_QUEUE][ANALYTICS_PAYLOAD];
	int head, count;
} AnalyticsQueue;
bool analytics_queue_put(AnalyticsQueue *q, const char *json);
bool analytics_queue_take(AnalyticsQueue *q, char *out, size_t n);
void analytics_queue_clear(AnalyticsQueue *q);
/* A request tried once, never again; after ANALYTICS_TRIES failures in a
 * row the session sends nothing more (no network, a server gone: no storm
 * of retries). False once it has given up. */
#define ANALYTICS_TRIES 3
typedef struct { int failures; bool gave_up; } AnalyticsTries;
bool analytics_tried(AnalyticsTries *t, bool ok);

#endif
