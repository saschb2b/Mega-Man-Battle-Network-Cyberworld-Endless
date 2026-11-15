/* What a release says of itself to the updater: update.json, a file of
 * every release (tools/update_manifest.py writes it, the release workflow
 * publishes it beside the game's files), read at the game's start
 * (docs/engine/updates.md). Its version, and for each way the game can
 * update itself (a key such as "appimage" or "windows-setup") the file's
 * address, size and SHA-256. Reading it and comparing versions touches
 * nothing else, so the unit tests hold it. */
#ifndef CW_UPDATE_MANIFEST_H
#define CW_UPDATE_MANIFEST_H

#include <stdbool.h>

typedef struct {
	char version[24];   /* "0.22.0" */
	char url[400];      /* the file's address, "" where the release has none for the key asked */
	long long size;     /* its bytes */
	char sha256[65];    /* its digest, lower-case hex */
} UpdateRelease;

/* `json`'s version, and the file under `key` where it has one (NULL: the
 * version alone). False where it has no version or the file's entry is
 * not whole. */
bool update_manifest_read(const char *json, const char *key, UpdateRelease *out);
/* Whether `v` is a release's version: numbers and two dots ("0.22.0"),
 * never a build between releases ("0.21.1-4-gabc123") or "dev" */
bool update_version_release(const char *v);
/* `a` against `b`, part by part: below 0 older, 0 the same, above 0 newer */
int update_version_cmp(const char *a, const char *b);

#endif
