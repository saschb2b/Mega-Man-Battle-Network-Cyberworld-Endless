/* update_port.h. */
#include "update_port.h"

#if defined(__linux__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* `path` without its last name, into `folder` (n bytes): "/a/b/" gives
 * "/a", "/b" gives "/". False where it holds no '/' or does not fit */
static bool parent_of(const char *path, char *folder, size_t n) {
	size_t len = strlen(path);
	if (len >= n) return false;
	memcpy(folder, path, len + 1);
	while (len > 1 && folder[len - 1] == '/') folder[--len] = 0;
	char *slash = strrchr(folder, '/');
	if (!slash) return false;
	if (slash == folder) slash[1] = 0;
	else *slash = 0;
	return true;
}

bool update_port_dir(const char *game, char *dir, size_t n) {
	char folder[PATH_MAX];
	dir[0] = 0;
	if (!game || game[0] != '/' || !parent_of(game, folder, sizeof folder)) return false;
	/* (the game's own name: what follows the parent and its slash) */
	size_t at = strlen(folder) + (strcmp(folder, "/") ? 1 : 0), len = strlen(game);
	while (len > at && game[len - 1] == '/') --len;
	if (len <= at) return false;
	int w = snprintf(dir, n, "%s%s.%.*s-update", folder, strcmp(folder, "/") ? "/" : "", (int)(len - at), game + at);
	if (w < 0 || (size_t)w >= n) {
		dir[0] = 0;
		return false;
	}
	return true;
}

/* `path`'s folder, its links followed, into `real` (PATH_MAX bytes) */
static bool real_parent(const char *path, char *real) {
	char folder[PATH_MAX];
	return parent_of(path, folder, sizeof folder) && realpath(folder, real) != NULL;
}

/* `s` (`len` bytes) ends in `tail` */
static bool ends_in(const char *s, size_t len, const char *tail) {
	size_t t = strlen(tail);
	return len > t && !memcmp(s + len - t, tail, t);
}

bool update_port_launcher(const char *args, size_t len, const char *game, char *launcher, size_t n) {
	char home[PATH_MAX], there[PATH_MAX];
	launcher[0] = 0;
	if (!game || game[0] != '/' || !real_parent(game, home)) return false;
	for (size_t at = 0; at < len;) {
		const char *a = args + at;
		const char *end = memchr(a, 0, len - at);
		size_t alen = end ? (size_t)(end - a) : len - at;
		struct stat st;
		if (a[0] == '/' && ends_in(a, alen, ".sh") && alen < n && stat(a, &st) == 0 &&
			S_ISREG(st.st_mode) && real_parent(a, there) && !strcmp(home, there))
			snprintf(launcher, n, "%s", a);
		at += alen + 1;
	}
	return launcher[0] != 0;
}

bool update_port_find(const char *path, const char *name, char *found, size_t n) {
	found[0] = 0;
	for (const char *p = path; p && *p;) {
		const char *colon = strchr(p, ':');
		size_t len = colon ? (size_t)(colon - p) : strlen(p);
		struct stat st;
		int w = snprintf(found, n, "%.*s/%s", (int)len, p, name);
		if (p[0] == '/' && w > 0 && (size_t)w < n && stat(found, &st) == 0 && S_ISREG(st.st_mode) && access(found, X_OK) == 0)
			return true;
		p = colon ? colon + 1 : NULL;
	}
	found[0] = 0;
	return false;
}

#endif
