/* update_manifest.h. The file is the project's own (tools/update_manifest.py),
 * an object of strings, numbers and objects: read by its keys' places, a
 * full JSON reader being more than it needs. */
#include "update_manifest.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* (past the blanks) */
static const char *blanks(const char *p, const char *end) {
	while (p < end && isspace((unsigned char)*p)) ++p;
	return p;
}

/* Where key `key`'s value starts within [p, end), past its colon: NULL
 * where it is not there */
static const char *value_of(const char *p, const char *end, const char *key) {
	char quoted[48];
	int n = snprintf(quoted, sizeof quoted, "\"%s\"", key);
	if (n <= 0 || n >= (int)sizeof quoted) return NULL;
	for (const char *at = p; at + n <= end; ++at) {
		if (memcmp(at, quoted, (size_t)n)) continue;
		const char *v = blanks(at + n, end);
		if (v < end && *v == ':') return blanks(v + 1, end);
	}
	return NULL;
}

/* A string value at `v` into `out` (its escapes taken as the letter after
 * them): false where it is not a whole string that fits */
static bool string_at(const char *v, const char *end, char *out, size_t size) {
	if (!v || v >= end || *v != '"') return false;
	size_t n = 0;
	for (++v; v < end && *v != '"'; ++v) {
		if (*v == '\\' && v + 1 < end) ++v;
		if (n + 1 >= size) return false;
		out[n++] = *v;
	}
	out[n] = 0;
	return v < end;
}

/* The object at `v`: its end, past its closing brace (NULL where none),
 * the braces inside it counted */
static const char *object_end(const char *v, const char *end) {
	if (!v || v >= end || *v != '{') return NULL;
	int depth = 0;
	bool in_string = false;
	for (; v < end; ++v) {
		if (in_string) {
			if (*v == '\\') ++v;
			else if (*v == '"') in_string = false;
		} else if (*v == '"') in_string = true;
		else if (*v == '{') ++depth;
		else if (*v == '}' && --depth == 0) return v + 1;
	}
	return NULL;
}

/* the file under `key` in the "files" object within [p, end) */
static bool file_read(const char *p, const char *end, const char *key, UpdateRelease *out) {
	const char *files = value_of(p, end, "files"), *files_end = object_end(files, end);
	if (!files_end) return false;
	const char *entry = value_of(files + 1, files_end, key), *entry_end = object_end(entry, files_end);
	if (!entry_end) return false;
	const char *size = value_of(entry, entry_end, "size");
	if (!size || !isdigit((unsigned char)*size)) return false;
	out->size = strtoll(size, NULL, 10);
	return out->size > 0 && string_at(value_of(entry, entry_end, "url"), entry_end, out->url, sizeof out->url) &&
		string_at(value_of(entry, entry_end, "sha256"), entry_end, out->sha256, sizeof out->sha256) && strlen(out->sha256) == 64;
}

bool update_manifest_read(const char *json, const char *key, UpdateRelease *out) {
	memset(out, 0, sizeof *out);
	if (!json) return false;
	const char *end = json + strlen(json);
	if (!string_at(value_of(json, end, "version"), end, out->version, sizeof out->version) || !update_version_release(out->version))
		return false;
	if (key && !file_read(json, end, key, out)) {
		out->url[0] = out->sha256[0] = 0;
		out->size = 0;
		return false;
	}
	return true;
}

bool update_version_release(const char *v) {
	int dots = 0;
	bool digit = false;
	for (; *v; ++v) {
		if (isdigit((unsigned char)*v)) digit = true;
		else if (*v == '.' && digit && dots < 2) { ++dots; digit = false; }
		else return false;
	}
	return dots == 2 && digit;
}

int update_version_cmp(const char *a, const char *b) {
	for (int part = 0; part < 3; ++part) {
		char *ae, *be;
		long x = strtol(a, &ae, 10), y = strtol(b, &be, 10);
		if (x != y) return x < y ? -1 : 1;
		a = *ae == '.' ? ae + 1 : ae;
		b = *be == '.' ? be + 1 : be;
	}
	return 0;
}
