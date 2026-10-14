#include "network_prefs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rom.h"

static bool port_number(const char *port, unsigned *number) {
	if (!port) return true;
	if (!*port || strspn(port, "0123456789") != strlen(port)) return false;
	unsigned long parsed = strtoul(port, NULL, 10);
	if (!parsed || parsed > 65535) return false;
	*number = (unsigned)parsed;
	return true;
}

/* The URL's scheme, lower-case host and numeric port identify a server.
 * Keeping ws and wss separate prevents a plaintext host reusing a TLS key. */
static bool origin(const char *url, char out[NETWORK_URL]) {
	if (!network_url_valid(url)) return false;
	bool secure = !strncmp(url, "wss://", 6);
	const char *start = url + (secure ? 6 : 5), *end = strchr(start, '/');
	char host[NETWORK_URL];
	size_t length = (size_t)(end - start);
	memcpy(host, start, length);
	host[length] = 0;
	char *port = NULL;
	if (host[0] == '[') {
		char *bracket = strchr(host, ']');
		if (!bracket || bracket == host + 1 || (bracket[1] && bracket[1] != ':')) return false;
		if (bracket[1]) { bracket[1] = 0; port = bracket + 2; }
	} else {
		port = strchr(host, ':');
		if (port) *port++ = 0;
	}
	unsigned number = secure ? 443u : 80u;
	if (!port_number(port, &number)) return false;
	length = strlen(host);
	if (length && host[length - 1] == '.') host[--length] = 0;
	if (!length) return false;
	for (char *p = host; *p; ++p) if (*p >= 'A' && *p <= 'Z') *p += 'a' - 'A';
	int written = snprintf(out, NETWORK_URL, "%s://%s:%u", secure ? "wss" : "ws", host, number);
	return written > 0 && written < NETWORK_URL;
}

static void digest(const uint8_t *data, size_t length, uint8_t out[20]) {
	char hex[41];
	sha1_hex(data, length, hex);
	for (unsigned i = 0; i < 20; ++i) {
		char pair[3] = { hex[i * 2], hex[i * 2 + 1], 0 };
		out[i] = (uint8_t)strtoul(pair, NULL, 16);
	}
}

bool network_server_identity(const uint8_t secret[NETWORK_ID_BYTES], const char *url, uint8_t out[NETWORK_ID_BYTES]) {
	char address[NETWORK_URL];
	if (!origin(url, address)) return false;
	static const char domain[] = "Cyberworld Network identity v1\n";
	uint8_t inner[64 + sizeof domain + NETWORK_URL], outer[84], hash[20];
	memset(inner, 0x36, 64);
	memset(outer, 0x5c, 64);
	for (unsigned i = 0; i < NETWORK_ID_BYTES; ++i) { inner[i] ^= secret[i]; outer[i] ^= secret[i]; }
	size_t prefix = sizeof domain - 1, size = strlen(address);
	memcpy(inner + 64, domain, prefix);
	memcpy(inner + 64 + prefix, address, size);
	digest(inner, 64 + prefix + size, outer + 64);
	digest(outer, sizeof outer, hash);
	memcpy(out, hash, NETWORK_ID_BYTES);
	return true;
}
