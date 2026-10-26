#include "network_prefs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"

static bool letter(unsigned char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
static bool digit(unsigned char c) { return c >= '0' && c <= '9'; }

bool network_name_valid(const char *name) {
	size_t n = strlen(name);
	if (!n || n >= NETWORK_NAME || (!letter((unsigned char)name[0]) && !digit((unsigned char)name[0]))) return false;
	for (size_t i = 1; i < n; ++i) {
		unsigned char c = (unsigned char)name[i];
		if (!letter(c) && !digit(c) && c != ' ' && c != '_' && c != '-') return false;
	}
	return name[n - 1] != ' ';
}

bool network_url_valid(const char *url) {
	const char *host = !strncmp(url, "wss://", 6) ? url + 6 : !strncmp(url, "ws://", 5) ? url + 5 : NULL;
	if (!host || strlen(url) >= NETWORK_URL) return false;
	const char *path = strchr(host, '/');
	if (!path || path == host || strcmp(path, "/presence")) return false;
	for (const char *p = host; p < path; ++p) {
		unsigned char c = (unsigned char)*p;
		if (!letter(c) && !digit(c) && c != '.' && c != '-' && c != ':' && c != '[' && c != ']') return false;
	}
	return true;
}

static int hex(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1;
}

static bool read_identity(const char *value, uint8_t *out) {
	if (strlen(value) != NETWORK_ID_BYTES * 2) return false;
	unsigned any = 0;
	for (unsigned i = 0; i < NETWORK_ID_BYTES; ++i) {
		int a = hex(value[i * 2]), b = hex(value[i * 2 + 1]);
		if (a < 0 || b < 0) return false;
		out[i] = (uint8_t)(a * 16 + b);
		any |= out[i];
	}
	return any != 0;
}

static void read_setting(NetworkPrefs *out, const char *key, const char *value, bool *identity) {
	if (!strcmp(key, "identity")) *identity = read_identity(value, out->identity);
	else if (!strcmp(key, "name") && network_name_valid(value)) snprintf(out->name, sizeof out->name, "%s", value);
	else if (!strcmp(key, "named")) out->named = value[0] == '1';
	else if (!strcmp(key, "quiet")) out->quiet = value[0] == '1';
	else if (!strcmp(key, "hinted")) out->hinted = value[0] == '1';
	else if (!strcmp(key, "favourites")) {
		unsigned f[4];
		if (sscanf(value, "%u,%u,%u,%u", &f[0], &f[1], &f[2], &f[3]) == 4)
			for (int k = 0; k < 4; ++k) out->favourites[k] = (uint8_t)(f[k] < 255u ? f[k] + 1u : 0u);
	}
	else if (!strcmp(key, "selected")) out->selected = (unsigned)strtoul(value, NULL, 10);
	else if (!strcmp(key, "consent")) out->consent = (unsigned)strtoul(value, NULL, 10) & ((1u << NETWORK_SERVERS) - 1u);
	else if (!strcmp(key, "server") && out->count < NETWORK_SERVERS) {
		/* Keep rejected slots until migration so their consent cannot move. */
		if (network_url_valid(value)) snprintf(out->servers[out->count], NETWORK_URL, "%s", value);
		++out->count;
	}
}

static void official_first(NetworkPrefs *out) {
	NetworkPrefs saved = *out;
	memset(out->servers, 0, sizeof out->servers);
	snprintf(out->servers[0], NETWORK_URL, "%s", NETWORK_DEFAULT_URL);
	out->count = 1;
	out->selected = out->consent = 0;
	for (unsigned i = 0; i < saved.count; ++i) {
		const char *url = saved.servers[i];
		if (!url[0]) continue;
		unsigned slot = 0;
		while (slot < out->count && strcmp(out->servers[slot], url)) ++slot;
		if (slot == out->count) {
			if (out->count == NETWORK_SERVERS) continue;
			snprintf(out->servers[out->count++], NETWORK_URL, "%s", url);
		}
		if (saved.selected == i) out->selected = slot;
		if (saved.consent & (1u << i)) out->consent |= 1u << slot;
	}
}

bool network_prefs_read(const char *directory, NetworkPrefs *out) {
	memset(out, 0, sizeof *out);
	snprintf(out->name, sizeof out->name, "%s", "MegaMan");
	char path[1100], line[512];
	snprintf(path, sizeof path, "%s/network.ini", directory);
	FILE *file = fopen(path, "r");
	bool identity = false;
	while (file && fgets(line, sizeof line, file)) {
		char *end = strpbrk(line, "\r\n"), *equals = strchr(line, '=');
		if (end) *end = 0;
		if (!equals || line[0] == '#') continue;
		*equals++ = 0;
		read_setting(out, line, equals, &identity);
	}
	if (file) fclose(file);
	official_first(out);
	if (!identity) memset(out->identity, 0, sizeof out->identity);
	return identity;
}

bool network_prefs_write(const char *directory, const NetworkPrefs *prefs) {
	if (!network_name_valid(prefs->name) || !prefs->count || prefs->count > NETWORK_SERVERS || prefs->selected >= prefs->count) return false;
	if (strcmp(prefs->servers[0], NETWORK_DEFAULT_URL)) return false;
	for (unsigned i = 0; i < prefs->count; ++i) if (!network_url_valid(prefs->servers[i])) return false;
	char path[1100], tmp[1110];
	snprintf(path, sizeof path, "%s/network.ini", directory);
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	FILE *file = fopen(tmp, "w");
	if (!file) return false;
#if !defined(_WIN32) && !defined(__3DS__)
	if (fchmod(fileno(file), 0600)) { fclose(file); remove(tmp); return false; }
#endif
	fputs("# The Net Square's settings (docs/online/). A connection begins only at the gate in Lan's HP.\nidentity=", file);
	for (unsigned i = 0; i < NETWORK_ID_BYTES; ++i) fprintf(file, "%02x", prefs->identity[i]);
	fprintf(file, "\nname=%s\nnamed=%u\nselected=%u\nconsent=%u\n", prefs->name, prefs->named ? 1u : 0u, prefs->selected, prefs->consent);
	for (unsigned i = 0; i < prefs->count; ++i) fprintf(file, "server=%s\n", prefs->servers[i]);
	if (prefs->quiet) fputs("quiet=1\n", file);
	if (prefs->hinted) fputs("hinted=1\n", file);
	if (prefs->favourites[0] && prefs->favourites[1] && prefs->favourites[2] && prefs->favourites[3])
		fprintf(file, "favourites=%u,%u,%u,%u\n", prefs->favourites[0] - 1u, prefs->favourites[1] - 1u, prefs->favourites[2] - 1u, prefs->favourites[3] - 1u);
	bool ok = !ferror(file) && cw_fsync_file(file);
	if (fclose(file)) ok = false;
	if (ok) ok = cw_rename(tmp, path);
	if (!ok) remove(tmp);
	else cw_fsync_dir(directory);
	return ok;
}
