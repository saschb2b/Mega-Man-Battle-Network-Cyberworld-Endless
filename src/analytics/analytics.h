/* The anonymous statistics (issue #104, docs/GUIDE.md's Anonymous statistics):
 * a few events of the game and its runs, sent to the game's own website on
 * the owner's Umami once the player has said yes. The title asks once, at
 * the first start where the build can send (analytics_ask.c); the answer is
 * settings.ini's statistics line, changed on Options' Statistics line.
 * Nothing is sent before a yes, and nothing that
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
#include <stdint.h>

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
/* The run's economy, counted as it goes whatever the answer (docs/
 * docs/game/economy/guard.md; the run log keeps them, runlog.h): the chips
 * found (a battle's reward, a Mystery Data's, a Guardian Data's) and the
 * good among them (3 stars or more, a Mega or a Giga), the Mystery Data
 * opened and the zenny they and the battles paid, in the whole run and in
 * its first act. Numbers only, nothing of the player. */
/* ... and beside it what the run chose (docs/game/core-loop.md, Phase 0:
 * the decisions a review could only argue about): requests taken, kept
 * and vowed, orders at AsterLand, NaviCust drafts left for their BugFrags,
 * the Undernet's and dark layers entered, ProtoMan's duels and netbattles
 * taken and won, folder edits between layers, Virus Battlers' series
 * begun, rounds fought on after a win and pots taken; and the frames on a layer's
 * map (its talks and menus among them) against those in battle. */
enum {
	CHOICE_JOB, CHOICE_JOB_KEPT, CHOICE_VOW, CHOICE_ORDER, CHOICE_DRAFT_SKIP, CHOICE_UNDERNET, CHOICE_DARK_LAYER,
	CHOICE_DUEL, CHOICE_DUEL_WON, CHOICE_NETBATTLE, CHOICE_NETBATTLE_WON, CHOICE_FOLDER_EDIT,
	CHOICE_BATTLER, CHOICE_BATTLER_ON, CHOICE_BATTLER_CASH, CHOICES
};
typedef struct {
	uint32_t seed;
	uint16_t chips, good, md, chips1, good1, md1;
	uint32_t zenny, zenny1;
	uint16_t choice[CHOICES];
	uint32_t map_frames, battle_frames;
	uint32_t folder;    /* (the folder as last noted, a hash: its edits counted against it) */
	uint32_t entered;   /* (the last layer whose kind was counted: a CONTINUE's rebuild counts it once) */
} AnalyticsEconomy;
/* A chip found (`good` one), a Mystery Data opened, zenny earned, on the
 * run's first act (`act1`) or past it */
void analytics_count_chip(AnalyticsEconomy *e, bool good, bool act1);
void analytics_count_md(AnalyticsEconomy *e, bool act1);
void analytics_count_zenny(AnalyticsEconomy *e, int zenny, bool act1);
/* ... a run over: won, or lost and `by` whom ("viruses", a guardian's
 * name, "ProtoMan"), and its economy's counts (`e`, NULL none) */
void analytics_run_end(bool won, const char *by, const AnalyticsEconomy *e);
/* A choice the run made (CHOICE_*) */
void analytics_count_choice(AnalyticsEconomy *e, int choice);
/* ... and what the run chose, as it ends (`trips` back home: the Net's
 * clock), beside run-end: an event of its own, the payload's room */
void analytics_run_choices(bool won, int trips, const AnalyticsEconomy *e);

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
