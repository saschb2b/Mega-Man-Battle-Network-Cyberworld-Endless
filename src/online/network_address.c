#include "network_address.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "network_prefs.h"

static bool ends_with(const char *name, const char *suffix) {
	size_t n = strlen(name), s = strlen(suffix);
	return n > s && !strcmp(name + n - s, suffix);
}

/* a dotted IPv4 address's four bytes; false where `host` is none */
static bool ipv4(const char *host, unsigned b[4]) {
	for (const char *p = host; *p; ++p) if (!isdigit((unsigned char)*p) && *p != '.') return false;
	char end;
	if (sscanf(host, "%u.%u.%u.%u%c", &b[0], &b[1], &b[2], &b[3], &end) != 4) return false;
	return b[0] < 256 && b[1] < 256 && b[2] < 256 && b[3] < 256;
}

/* the private, shared (a carrier's or a VPN's), loopback and link-local blocks */
static bool ipv4_local(const unsigned b[4]) {
	return b[0] == 10 || b[0] == 127 || (b[0] == 172 && (b[1] & 0xf0) == 16) || (b[0] == 192 && b[1] == 168)
		|| (b[0] == 169 && b[1] == 254) || (b[0] == 100 && (b[1] & 0xc0) == 64);
}

/* an IPv6 address between its brackets, `n` characters: its first group read */
static bool ipv6_local(const char *inner, size_t n) {
	if (n == 3 && !strncmp(inner, "::1", 3)) return true;
	unsigned group = 0;
	size_t i = 0;
	for (; i < n && i < 4 && isxdigit((unsigned char)inner[i]); ++i) {
		int c = tolower((unsigned char)inner[i]);
		group = group * 16 + (unsigned)(isdigit(c) ? c - '0' : c - 'a' + 10);
	}
	if (!i || i >= n || inner[i] != ':') return false;
	return (group & 0xfe00) == 0xfc00 || (group & 0xffc0) == 0xfe80;
}

bool network_address_local(const char *host) {
	size_t n = strlen(host);
	if (n && host[0] == '[') return n > 2 && host[n - 1] == ']' && ipv6_local(host + 1, n - 2);
	char name[NETWORK_URL];
	if (!n || n >= sizeof name) return false;
	for (size_t i = 0; i <= n; ++i) name[i] = (char)tolower((unsigned char)host[i]);
	if (name[n - 1] == '.' && n > 1) name[--n] = 0;   /* (a fully qualified name's root) */
	unsigned b[4];
	if (ipv4(name, b)) return ipv4_local(b);
	if (!strchr(name, '.') || !strcmp(name, "localhost")) return true;
	static const char *const suffixes[] = { ".localhost", ".local", ".lan", ".home", ".home.arpa", ".internal", ".localdomain" };
	for (size_t i = 0; i < sizeof suffixes / sizeof *suffixes; ++i) if (ends_with(name, suffixes[i])) return true;
	return false;
}

bool network_address_url(const char *typed, char *out, size_t cap) {
	char url[NETWORK_URL];
	int n;
	if (!strncmp(typed, "ws://", 5) || !strncmp(typed, "wss://", 6)) {
		/* (a whole address: the path the server answers on, where left out) */
		n = snprintf(url, sizeof url, "%s%s", typed, strchr(strstr(typed, "://") + 3, '/') ? "" : "/presence");
	} else {
		size_t length = strcspn(typed, "/");
		char host[NETWORK_URL];
		if (length >= sizeof host) return false;
		memcpy(host, typed, length);
		host[length] = 0;
		/* where the name ends and a port begins: after an IPv6 address's
		 * bracket, or at a name's one colon */
		size_t name_end = length;
		const char *colon = strchr(host, ':');
		if (host[0] == '[') {
			const char *close = strchr(host, ']');
			if (close) name_end = (size_t)(close - host) + 1;
		} else if (colon && colon == strrchr(host, ':')) name_end = (size_t)(colon - host);
		char name[NETWORK_URL];
		memcpy(name, host, name_end);
		name[name_end] = 0;
		bool local = network_address_local(name);
		n = snprintf(url, sizeof url, "%s%s%s%s", local ? "ws://" : "wss://", host,
			local && name_end == length ? ":" NETWORK_LOCAL_PORT : "", typed[length] ? typed + length : "/presence");
	}
	if (n < 0 || (size_t)n >= sizeof url || (size_t)n >= cap || !network_url_valid(url)) return false;
	snprintf(out, cap, "%s", url);
	return true;
}
