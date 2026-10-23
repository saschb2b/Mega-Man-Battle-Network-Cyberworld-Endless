/* settings.ini's lines written one at a time (settings_line.h). */
#include "settings_line.h"

#include <stdio.h>
#include <string.h>

#include "compat.h"

bool settings_line_write(const char *path, const char *key, const char *value, const char *note) {
	char tmp[620], line[512], k[32];
	if (!path) return false;
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	FILE *in = fopen(path, "r"), *out = fopen(tmp, "w");
	if (!out) {
		if (in) fclose(in);
		return false;
	}
	bool kept = false;
	while (in && fgets(line, sizeof line, in)) {
		bool ours = line[0] != '#' && sscanf(line, " %31[a-z_]", k) == 1 && !strcmp(k, key);
		if (!ours) fputs(line, out);
		else if (!kept) fprintf(out, "%s = %s\n", key, value);
		kept |= ours;
	}
	/* (what it is, after a line apart from the lines before it, if any) */
	if (!in && *note == '\n') ++note;
	if (in) fclose(in);
	if (!kept) fprintf(out, "%s%s = %s\n", note, key, value);
	return fclose(out) == 0 && cw_rename(tmp, path);
}

bool settings_line_read(const char *path, const char *key, char *value, size_t n) {
	char line[512], k[32], v[64];
	FILE *in = path ? fopen(path, "r") : NULL;
	bool found = false;
	while (in && !found && fgets(line, sizeof line, in))
		found = line[0] != '#' && sscanf(line, " %31[a-z_] = %63s", k, v) == 2 && !strcmp(k, key);
	if (in) fclose(in);
	if (found) snprintf(value, n, "%s", v);
	return found;
}
