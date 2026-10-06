/* analytics_net.h. What a session tells of its system (the address's
 * path, the screen, a User-Agent Umami takes for a player's), and the
 * hand-off to each system's way of sending. */
#include "analytics_net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#ifdef __ANDROID__
#include <jni.h>
#endif
#ifdef __3DS__
#include <3ds/types.h>
#include <3ds/thread.h>
#endif

#include "analytics.h"
#include "analytics_http.h"
#include "game.h"
#include "platform.h"
#include "version.h"

/* (the systems that send from a thread of the game's: analytics_http.c's) */
#if (defined(__3DS__) && defined(CW_3DS_CURL)) || defined(_WIN32) || \
	(defined(__linux__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__))
#define THREADED 1
#endif

static char agent[160];

/* ---- what a session tells ---- */

/* The address's path: the system, and on Linux the package where it says
 * (a Steam Deck's Steam names it, the Flatpak and the AppImage set theirs) */
static const char *platform_name(void) {
#if defined(__EMSCRIPTEN__)
	return "browser";
#elif defined(__ANDROID__)
	return "android";
#elif defined(CW_IOS)
	return "ios";
#elif defined(__3DS__)
	return "3ds";
#elif defined(_WIN32)
	return "windows";
#elif defined(__APPLE__)
	return "macos";
#elif defined(CW_DESKTOP)
	const char *deck = getenv("SteamDeck");
	if (deck && !strcmp(deck, "1")) return "steamdeck";
	if (getenv("FLATPAK_ID")) return "linux/flatpak";
	if (getenv("APPIMAGE")) return "linux/appimage";
	return "linux";
#else
	return "portmaster";
#endif
}

#ifdef __EMSCRIPTEN__
/* (the page's screen, in its CSS pixels, as Umami's own tracker reads it) */
EM_JS(int, cw_stats_screen, (void), { return ((screen.width & 0x7FFF) << 16) | (screen.height & 0xFFFF); });
#endif

/* The display's size, its long side first (a phone held upright says the
 * same as on its side); a desktop's whole display, not the window */
static void screen_size(int *w, int *h) {
#ifdef __EMSCRIPTEN__
	int s = cw_stats_screen();
	*w = s >> 16;
	*h = s & 0xFFFF;
#else
	SDL_DisplayMode m;
	if (SDL_GetDesktopDisplayMode(0, &m) == 0) { *w = m.w; *h = m.h; }
	else { *w = P.screen_w; *h = P.screen_h; }
#endif
	if (*h > *w) { int t = *w; *w = *h; *h = t; }
}

/* The build's version as a User-Agent's word: letters, digits, . + - */
static void version_word(char *v, size_t n) {
	size_t k = 0;
	for (const char *c = CW_VERSION; *c && k + 1 < n; ++c)
		if ((*c >= '0' && *c <= '9') || (*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') || strchr(".+-", *c)) v[k++] = *c;
	v[k] = 0;
	if (!k) snprintf(v, n, "dev");
}

/* The User-Agent: a browser's form (Umami drops requests whose agent looks
 * like a bot's or a tool's, isbot's list), naming the system as browsers
 * do, so its reports' systems are right; the game's own word after it */
static void make_agent(int w, int h) {
	char v[48];
	version_word(v, sizeof v);
	(void)w;
	(void)h;
#if defined(_WIN32)
	snprintf(agent, sizeof agent, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) CyberworldEndless/%s", v);
#elif defined(CW_IOS)
	/* (an iPad's screen is near 4:3, an iPhone's longer than 16:10) */
	bool pad = h > 0 && w * 10 < h * 16;
	snprintf(agent, sizeof agent, "Mozilla/5.0 (%s; CPU %s 16_0 like Mac OS X) CyberworldEndless/%s", pad ? "iPad" : "iPhone",
		pad ? "OS" : "iPhone OS", v);
#elif defined(__APPLE__)
	snprintf(agent, sizeof agent, "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) CyberworldEndless/%s", v);
#elif defined(__ANDROID__)
	snprintf(agent, sizeof agent, "Mozilla/5.0 (Linux; Android) CyberworldEndless/%s", v);
#elif defined(__3DS__)
	snprintf(agent, sizeof agent, "Mozilla/5.0 (Nintendo 3DS) CyberworldEndless/%s", v);
#else
#if defined(__x86_64__)
	const char *arch = "x86_64";
#elif defined(__aarch64__)
	const char *arch = "aarch64";
#elif defined(__arm__)
	const char *arch = "armv7l";
#else
	const char *arch = "unknown";
#endif
	snprintf(agent, sizeof agent, "Mozilla/5.0 (X11; Linux %s) CyberworldEndless/%s", arch, v);
#endif
}

/* ---- a thread of the game's, posting one at a time ---- */

#ifdef THREADED
static struct {
	bool started, broken;
	SDL_mutex *lock;
	SDL_sem *wake, *ended;
	AnalyticsQueue q;       /* (under lock) */
	AnalyticsTries tries;   /* (the thread's own) */
	SDL_atomic_t stop, gave_up;
#ifdef __3DS__
	Thread thread;
#endif
} W;

static int worker(void *arg) {
	(void)arg;
	static char body[ANALYTICS_PAYLOAD];
	while (!SDL_AtomicGet(&W.stop)) {
		SDL_SemWait(W.wake);
		SDL_LockMutex(W.lock);
		bool got = !SDL_AtomicGet(&W.stop) && analytics_queue_take(&W.q, body, sizeof body);
		SDL_UnlockMutex(W.lock);
		if (!got) continue;
		bool ok = analytics_http_post(analytics_url(), agent, body, &W.stop);
		if (analytics_checking()) printf("statistics: %s %s\n", ok ? "taken (2xx) by" : "not taken by", analytics_url());
		if (analytics_tried(&W.tries, ok)) continue;
		/* (given up: what waits is dropped, and nothing more comes) */
		SDL_AtomicSet(&W.gave_up, 1);
		SDL_LockMutex(W.lock);
		analytics_queue_clear(&W.q);
		SDL_UnlockMutex(W.lock);
	}
	analytics_http_end();
	SDL_SemPost(W.ended);
	return 0;
}

#ifdef __3DS__
static void worker_3ds(void *arg) { worker(arg); }
#endif

/* The thread, at the first payload (none for a player who said no) */
static bool thread_start(void) {
	if (W.started || W.broken) return W.started;
	W.broken = true;
	if (!(W.lock = SDL_CreateMutex()) || !(W.wake = SDL_CreateSemaphore(0)) || !(W.ended = SDL_CreateSemaphore(0))) return false;
#ifdef __3DS__
	/* (below the game's own threads, on its core: the requests run in the
	 * frames' waits for the screen; libcurl and mbedTLS want a deep stack) */
	W.thread = threadCreate(worker_3ds, NULL, 0x20000, 0x3F, -2, false);
	W.started = W.thread != NULL;
#else
	SDL_Thread *t = SDL_CreateThreadWithStackSize(worker, "statistics", 256 * 1024, NULL);
	if (t) SDL_DetachThread(t);
	W.started = t != NULL;
#endif
	W.broken = !W.started;
	return W.started;
}

static bool system_post(const char *json) {
	if (SDL_AtomicGet(&W.gave_up) || !thread_start()) return false;
	SDL_LockMutex(W.lock);
	bool put = analytics_queue_put(&W.q, json);
	SDL_UnlockMutex(W.lock);
	if (put) SDL_SemPost(W.wake);
	return put;
}

static void system_drop(void) {
	if (!W.started) return;
	SDL_LockMutex(W.lock);
	analytics_queue_clear(&W.q);
	SDL_UnlockMutex(W.lock);
}

static bool system_ready(void) { return analytics_http_ready(); }

static void thread_quit(void) {
	if (!W.started) return;
	SDL_AtomicSet(&W.stop, 1);
	SDL_SemPost(W.wake);
#ifdef __3DS__
	/* (a 3DS app ending with a thread alive takes the HOME Menu down: a
	 * request under way stops within a second, emu.c) */
	threadJoin(W.thread, U64_MAX);
	threadFree(W.thread);
#else
	/* (a request under way stops within a second; past that, the game
	 * ends without it) */
	SDL_SemWaitTimeout(W.ended, 1500);
#endif
	W.started = false;
}

/* ---- Apple's NSURLSession (analytics_apple.m) ---- */

#elif defined(__APPLE__)
static SDL_atomic_t busy, failures;

static void apple_done(bool ok) {
	SDL_AtomicAdd(&busy, -1);
	if (ok) SDL_AtomicSet(&failures, 0);
	else SDL_AtomicAdd(&failures, 1);
}

static bool system_post(const char *json) {
	if (SDL_AtomicGet(&failures) >= ANALYTICS_TRIES || SDL_AtomicGet(&busy) >= ANALYTICS_QUEUE) return false;
	SDL_AtomicAdd(&busy, 1);
	if (analytics_apple_post(analytics_url(), agent, json, apple_done)) return true;
	SDL_AtomicAdd(&busy, -1);
	return false;
}

/* (a request is under way at once: nothing waits to be dropped) */
static void system_drop(void) {}

static bool system_ready(void) { return true; }

/* ---- Android's HttpURLConnection, on GameActivity's executor ---- */

#elif defined(__ANDROID__)
/* GameActivity's statsSend (url, agent, body) or statsDrop (`body` NULL),
 * on the game's thread (SDL's, with its JNIEnv); false where Java lacks
 * them, refused the payload or threw */
static bool tell_java(const char *body) {
	static jmethodID send, drop;
	static bool missing;
	JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	if (!env || !activity || missing) {
		if (env && activity) (*env)->DeleteLocalRef(env, activity);
		return false;
	}
	if (!send) {
		jclass cls = (*env)->GetObjectClass(env, activity);
		send = (*env)->GetMethodID(env, cls, "statsSend", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z");
		if (send) drop = (*env)->GetMethodID(env, cls, "statsDrop", "()V");
		if ((*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
		missing = !send || !drop;
		(*env)->DeleteLocalRef(env, cls);
	}
	bool ok = false;
	if (!missing && !body) {
		(*env)->CallVoidMethod(env, activity, drop);
		ok = true;
	} else if (!missing) {
		jstring u = (*env)->NewStringUTF(env, analytics_url()), a = (*env)->NewStringUTF(env, agent), b = (*env)->NewStringUTF(env, body);
		ok = u && a && b && (*env)->CallBooleanMethod(env, activity, send, u, a, b) != JNI_FALSE;
		if (u) (*env)->DeleteLocalRef(env, u);
		if (a) (*env)->DeleteLocalRef(env, a);
		if (b) (*env)->DeleteLocalRef(env, b);
	}
	if ((*env)->ExceptionCheck(env)) {
		(*env)->ExceptionClear(env);
		ok = false;
	}
	(*env)->DeleteLocalRef(env, activity);
	return ok;
}

static bool system_post(const char *json) { return tell_java(json); }

static void system_drop(void) { tell_java(NULL); }

/* (the method there: an app built with this C has it) */
static bool system_ready(void) { return tell_java(NULL); }

/* ---- the browser's fetch ---- */

#elif defined(__EMSCRIPTEN__)
EM_JS_DEPS(cw_stats, "$UTF8ToString");
/* (a few under way at most, none after three failures in a row: Umami's
 * host answers the page's CORS as it answers its own tracker's) */
EM_JS(int, cw_stats_fetch, (const char *url, const char *body, int most, int tries), {
	var m = Module;
	if ((m.cwStatsBusy || 0) >= most || (m.cwStatsFailed || 0) >= tries) return 0;
	m.cwStatsBusy = (m.cwStatsBusy || 0) + 1;
	fetch(UTF8ToString(url), { method: 'POST', body: UTF8ToString(body), headers: { 'Content-Type': 'application/json' },
		keepalive: true, credentials: 'omit' })
		.then(function (r) { m.cwStatsFailed = r.ok ? 0 : (m.cwStatsFailed || 0) + 1; })
		.catch(function () { m.cwStatsFailed = (m.cwStatsFailed || 0) + 1; })
		.finally(function () { m.cwStatsBusy--; });
	return 1;
});

static bool system_post(const char *json) { return cw_stats_fetch(analytics_url(), json, ANALYTICS_QUEUE, ANALYTICS_TRIES) != 0; }

static void system_drop(void) {}

static bool system_ready(void) { return true; }

/* ---- a system that cannot send (a 3DS built without libcurl) ---- */

#else
static bool system_post(const char *json) {
	(void)json;
	return false;
}

static void system_drop(void) {}

static bool system_ready(void) { return false; }
#endif

/* ---- the session ---- */

void analytics_net_start(bool headless, bool plain) {
	static char settings[600];
	int w = 0, h = 0;
	snprintf(settings, sizeof settings, "%s/settings.ini", g_data_dir);
	screen_size(&w, &h);
	make_agent(w, h);
	AnalyticsStart s = { platform_name(), CW_VERSION, w, h, settings, plain && !headless, system_ready, system_post, system_drop, platform_persist };
	analytics_begin(&s);
}

void analytics_net_quit(void) {
#ifdef THREADED
	thread_quit();
#endif
}
