/* analytics_http.h. libcurl as Linux has it (loaded by its file's name,
 * its few names by curl.h's numbers, which no libcurl since 7.32 has
 * changed: no build needs its headers) or as the 3DS links it, and
 * Windows' WinHTTP. */
#include "analytics_http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analytics.h"
#include "analytics_roots.h"

/* (the 3DS's where its build found devkitPro's libcurl: the Makefile's CW_3DS_CURL) */
#if (defined(__3DS__) && defined(CW_3DS_CURL)) || (defined(__linux__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__))
#define HTTP_CURL 1
#endif

#define CONNECT_S 5L   /* seconds to the host */
#define TOTAL_S 10L    /* ... and to its answer */

#ifdef HTTP_CURL

#ifdef __3DS__
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/services/soc.h>
#include <3ds/services/sslc.h>
#include <curl/curl.h>
#include <malloc.h>

#define CURL_DO(f) curl_##f
#else
typedef void CURL;
struct curl_slist;
typedef long long curl_off_t;
struct curl_blob { void *data; size_t len; unsigned int flags; };
enum {
	CURLE_OK = 0, CURLE_PEER_FAILED_VERIFICATION = 60, CURLE_SSL_CACERT_BADFILE = 77,
	CURLOPT_TIMEOUT = 13, CURLOPT_NOPROGRESS = 43, CURLOPT_POSTFIELDSIZE = 60, CURLOPT_CONNECTTIMEOUT = 78, CURLOPT_NOSIGNAL = 99,
	CURLOPT_URL = 10002, CURLOPT_POSTFIELDS = 10015, CURLOPT_USERAGENT = 10018, CURLOPT_HTTPHEADER = 10023, CURLOPT_XFERINFODATA = 10057,
	CURLOPT_WRITEFUNCTION = 20011, CURLOPT_XFERINFOFUNCTION = 20219, CURLOPT_CAINFO_BLOB = 40309,
	CURLINFO_RESPONSE_CODE = 0x200002,
};
#define CURL_GLOBAL_DEFAULT 3L
#define CURL_BLOB_NOCOPY 0

/* libcurl's own, once loaded */
static struct {
	void *lib;
	int (*global_init)(long);
	CURL *(*easy_init)(void);
	int (*easy_setopt)(CURL *, int, ...);
	int (*easy_perform)(CURL *);
	int (*easy_getinfo)(CURL *, int, ...);
	void (*easy_cleanup)(CURL *);
	struct curl_slist *(*slist_append)(struct curl_slist *, const char *);
	void (*slist_free_all)(struct curl_slist *);
} C;
#define CURL_DO(f) C.f

/* (its name as Debian's and most systems' packages give it, then the
 * GnuTLS build's, then the bare one a handheld's firmware may have) */
static bool curl_load(void) {
	static const char *const names[] = { "libcurl.so.4", "libcurl-gnutls.so.4", "libcurl.so" };
	for (size_t i = 0; !C.lib && i < sizeof names / sizeof *names; ++i) C.lib = SDL_LoadObject(names[i]);
	if (!C.lib) return false;
	C.global_init = (int (*)(long))SDL_LoadFunction(C.lib, "curl_global_init");
	C.easy_init = (CURL * (*)(void)) SDL_LoadFunction(C.lib, "curl_easy_init");
	C.easy_setopt = (int (*)(CURL *, int, ...))SDL_LoadFunction(C.lib, "curl_easy_setopt");
	C.easy_perform = (int (*)(CURL *))SDL_LoadFunction(C.lib, "curl_easy_perform");
	C.easy_getinfo = (int (*)(CURL *, int, ...))SDL_LoadFunction(C.lib, "curl_easy_getinfo");
	C.easy_cleanup = (void (*)(CURL *))SDL_LoadFunction(C.lib, "curl_easy_cleanup");
	C.slist_append = (struct curl_slist * (*)(struct curl_slist *, const char *)) SDL_LoadFunction(C.lib, "curl_slist_append");
	C.slist_free_all = (void (*)(struct curl_slist *))SDL_LoadFunction(C.lib, "curl_slist_free_all");
	if (C.global_init && C.easy_init && C.easy_setopt && C.easy_perform && C.easy_getinfo && C.easy_cleanup && C.slist_append && C.slist_free_all)
		return true;
	SDL_UnloadObject(C.lib);
	C.lib = NULL;
	return false;
}

/* The Flatpak's sandbox: whether the network is shared with it (its
 * manifest shares it; a player's override may take it away) */
static bool sandbox_network(void) {
	FILE *f = fopen("/.flatpak-info", "r");
	if (!f) return true;
	char line[512];
	bool shared = false;
	while (fgets(line, sizeof line, f))
		if (!strncmp(line, "shared=", 7) && strstr(line, "network")) shared = true;
	fclose(f);
	return shared;
}
#endif

/* (an answer's body, unread) */
static size_t discard(char *p, size_t size, size_t n, void *user) {
	(void)p;
	(void)user;
	return size * n;
}

/* (a request given up as the game ends: libcurl asks this at least once
 * a second) */
static int stopped(void *stop, curl_off_t a, curl_off_t b, curl_off_t c, curl_off_t d) {
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	return SDL_AtomicGet((SDL_atomic_t *)stop) ? 1 : 0;
}

/* One request: its result (CURLE_*), the answer's status in `code` */
static int curl_once(const char *url, const char *agent, const char *body, SDL_atomic_t *stop, bool roots, long *code) {
	CURL *h = CURL_DO(easy_init)();
	if (!h) return -1;
	struct curl_slist *head = CURL_DO(slist_append)(NULL, "Content-Type: application/json");
	CURL_DO(easy_setopt)(h, CURLOPT_URL, url);
	CURL_DO(easy_setopt)(h, CURLOPT_USERAGENT, agent);
	CURL_DO(easy_setopt)(h, CURLOPT_HTTPHEADER, head);
	CURL_DO(easy_setopt)(h, CURLOPT_POSTFIELDS, body);
	CURL_DO(easy_setopt)(h, CURLOPT_POSTFIELDSIZE, (long)strlen(body));
	CURL_DO(easy_setopt)(h, CURLOPT_CONNECTTIMEOUT, CONNECT_S);
	CURL_DO(easy_setopt)(h, CURLOPT_TIMEOUT, TOTAL_S);
	CURL_DO(easy_setopt)(h, CURLOPT_NOSIGNAL, 1L);
	CURL_DO(easy_setopt)(h, CURLOPT_WRITEFUNCTION, discard);
	CURL_DO(easy_setopt)(h, CURLOPT_NOPROGRESS, 0L);
	CURL_DO(easy_setopt)(h, CURLOPT_XFERINFOFUNCTION, stopped);
	CURL_DO(easy_setopt)(h, CURLOPT_XFERINFODATA, (void *)stop);
	struct curl_blob ca = { analytics_roots, strlen(analytics_roots), CURL_BLOB_NOCOPY };
	if (roots) CURL_DO(easy_setopt)(h, CURLOPT_CAINFO_BLOB, &ca);
	int r = CURL_DO(easy_perform)(h);
	*code = 0;
	if (r == CURLE_OK) CURL_DO(easy_getinfo)(h, CURLINFO_RESPONSE_CODE, code);
	CURL_DO(easy_cleanup)(h);
	CURL_DO(slist_free_all)(head);
	return r;
}

#ifdef __3DS__
/* The 3DS's network for libcurl: its sockets (unless 3dslink's output
 * has them already, start_3ds.c) and its SSL service, whose random bytes
 * mbedTLS draws from; taken at the first request, so a player who said no
 * never has them */
#define SOC_SIZE 0x100000
static bool soc_own, sslc_own, net_tried, net_up;

static bool net_3ds(void) {
	if (net_tried) return net_up;
	net_tried = true;
	u32 *buf = memalign(0x1000, SOC_SIZE);
	Result r = buf ? socInit(buf, SOC_SIZE) : -1;
	soc_own = R_SUCCEEDED(r);
	if (!soc_own) free(buf);
	sslc_own = R_SUCCEEDED(sslcInit(0));
	net_up = (soc_own || (buf && R_DESCRIPTION(r) == RD_ALREADY_INITIALIZED)) && sslc_own;
	return net_up;
}
#endif

bool analytics_http_ready(void) {
#ifdef __3DS__
	return true;
#else
	return curl_load() && sandbox_network();
#endif
}

bool analytics_http_post(const char *url, const char *agent, const char *body, SDL_atomic_t *stop) {
	static bool begun, roots;
#ifdef __3DS__
	/* (no store of its own knows Let's Encrypt's roots) */
	roots = true;
	if (!net_3ds()) return false;
#endif
	if (!begun) {
		begun = true;
		CURL_DO(global_init)(CURL_GLOBAL_DEFAULT);
	}
	long code = 0;
	int r = curl_once(url, agent, body, stop, roots, &code);
	/* (a system without its CA file, or one too old for the host's root:
	 * once more with the roots the game carries, and so from then on) */
	if (!roots && (r == CURLE_PEER_FAILED_VERIFICATION || r == CURLE_SSL_CACERT_BADFILE)) {
		roots = true;
		r = curl_once(url, agent, body, stop, roots, &code);
	}
	return r == CURLE_OK && code >= 200 && code < 300;
}

void analytics_http_end(void) {
#ifdef __3DS__
	if (sslc_own) sslcExit();
	if (soc_own) socExit();
	sslc_own = soc_own = false;
#endif
}

#elif defined(_WIN32)

#include <windows.h>
#include <winhttp.h>

/* WinHTTP's own, once loaded */
static struct {
	void *lib;
	__typeof__(&WinHttpOpen) open;
	__typeof__(&WinHttpSetTimeouts) timeouts;
	__typeof__(&WinHttpConnect) connect;
	__typeof__(&WinHttpOpenRequest) request;
	__typeof__(&WinHttpSendRequest) send;
	__typeof__(&WinHttpReceiveResponse) receive;
	__typeof__(&WinHttpQueryHeaders) query;
	__typeof__(&WinHttpCloseHandle) close;
} H;

bool analytics_http_ready(void) {
	if (H.lib) return true;
	if (!(H.lib = SDL_LoadObject("winhttp.dll"))) return false;
	H.open = (__typeof__(H.open))SDL_LoadFunction(H.lib, "WinHttpOpen");
	H.timeouts = (__typeof__(H.timeouts))SDL_LoadFunction(H.lib, "WinHttpSetTimeouts");
	H.connect = (__typeof__(H.connect))SDL_LoadFunction(H.lib, "WinHttpConnect");
	H.request = (__typeof__(H.request))SDL_LoadFunction(H.lib, "WinHttpOpenRequest");
	H.send = (__typeof__(H.send))SDL_LoadFunction(H.lib, "WinHttpSendRequest");
	H.receive = (__typeof__(H.receive))SDL_LoadFunction(H.lib, "WinHttpReceiveResponse");
	H.query = (__typeof__(H.query))SDL_LoadFunction(H.lib, "WinHttpQueryHeaders");
	H.close = (__typeof__(H.close))SDL_LoadFunction(H.lib, "WinHttpCloseHandle");
	if (H.open && H.timeouts && H.connect && H.request && H.send && H.receive && H.query && H.close) return true;
	SDL_UnloadObject(H.lib);
	H.lib = NULL;
	return false;
}

/* `url`'s parts: http or https, the host, its port, the path */
static bool url_parts(const char *url, bool *secure, wchar_t *host, int hn, int *port, wchar_t *path, int pn) {
	*secure = !strncmp(url, "https://", 8);
	if (!*secure && strncmp(url, "http://", 7)) return false;
	const char *h = url + (*secure ? 8 : 7), *slash = strchr(h, '/'), *colon = strchr(h, ':');
	if (!slash) slash = h + strlen(h);
	if (colon && colon > slash) colon = NULL;
	const char *end = colon ? colon : slash;
	char name[128];
	if (end == h || end - h >= (int)sizeof name) return false;
	snprintf(name, sizeof name, "%.*s", (int)(end - h), h);
	*port = colon ? atoi(colon + 1) : *secure ? 443 : 80;
	return MultiByteToWideChar(CP_UTF8, 0, name, -1, host, hn) > 0 && MultiByteToWideChar(CP_UTF8, 0, *slash ? slash : "/", -1, path, pn) > 0;
}

bool analytics_http_post(const char *url, const char *agent, const char *body, SDL_atomic_t *stop) {
	(void)stop;   /* (a call under way runs out its timeouts: the game ends without waiting) */
	bool secure;
	int port;
	wchar_t host[128], path[256], who[192];
	char data[ANALYTICS_PAYLOAD];
	DWORD n = (DWORD)snprintf(data, sizeof data, "%s", body);
	if (n >= sizeof data || !url_parts(url, &secure, host, 128, &port, path, 256) || MultiByteToWideChar(CP_UTF8, 0, agent, -1, who, 192) <= 0)
		return false;
	HINTERNET s = H.open(who, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!s) return false;
	H.timeouts(s, (int)CONNECT_S * 1000, (int)CONNECT_S * 1000, (int)TOTAL_S * 1000, (int)TOTAL_S * 1000);
	HINTERNET c = H.connect(s, host, (INTERNET_PORT)port, 0);
	HINTERNET r = c ? H.request(c, L"POST", path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0) : NULL;
	DWORD status = 0, size = sizeof status;
	bool ok = r && H.send(r, L"Content-Type: application/json\r\n", (DWORD)-1L, data, n, n, 0) && H.receive(r, NULL) &&
		H.query(r, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX) &&
		status >= 200 && status < 300;
	if (r) H.close(r);
	if (c) H.close(c);
	H.close(s);
	return ok;
}

void analytics_http_end(void) {}

#endif
