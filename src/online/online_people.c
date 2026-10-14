#include "online_people.h"

#include <stdio.h>
#include <string.h>

#include "compat.h"
#include "game.h"

#define PEOPLE_MAX 64
static struct { uint8_t identity[ONLINE_IDENTITY_BYTES]; unsigned flags; } people[PEOPLE_MAX];
static unsigned count;

void online_people_load(void) {
	count = 0;
	char path[600], line[80];
	snprintf(path, sizeof path, "%s/network-people.ini", g_data_dir);
	FILE *file = fopen(path, "r");
	while (file && count < PEOPLE_MAX && fgets(line, sizeof line, file)) {
		uint8_t identity[ONLINE_IDENTITY_BYTES];
		unsigned flags = 0, any = 0;
		if (strlen(line) < 34 || line[32] != ' ' || sscanf(line + 33, "%u", &flags) != 1 || flags < 1 || flags > 3) continue;
		bool valid = true;
		for (unsigned i = 0; i < ONLINE_IDENTITY_BYTES; ++i) {
			unsigned byte = 0;
			if (sscanf(line + 2 * i, "%2x", &byte) != 1) { valid = false; break; }
			identity[i] = (uint8_t)byte;
			any |= byte;
		}
		if (!valid || !any) continue;
		memcpy(people[count].identity, identity, ONLINE_IDENTITY_BYTES);
		people[count++].flags = flags;
	}
	if (file) fclose(file);
}

unsigned online_people_flags(const uint8_t identity[ONLINE_IDENTITY_BYTES]) {
	for (unsigned i = 0; i < count; ++i)
		if (!memcmp(people[i].identity, identity, ONLINE_IDENTITY_BYTES)) return people[i].flags;
	return 0;
}

static bool write_people(void) {
	char path[600], temp[610];
	snprintf(path, sizeof path, "%s/network-people.ini", g_data_dir);
	snprintf(temp, sizeof temp, "%s.tmp", path);
	FILE *file = fopen(temp, "w");
	if (!file) return false;
	for (unsigned i = 0; i < count; ++i) {
		for (unsigned j = 0; j < ONLINE_IDENTITY_BYTES; ++j) fprintf(file, "%02x", people[i].identity[j]);
		fprintf(file, " %u\n", people[i].flags);
	}
	bool ok = !ferror(file) && cw_fsync_file(file);
	if (fclose(file)) ok = false;
	if (ok) ok = cw_rename(temp, path);
	if (!ok) remove(temp);
	return ok;
}

bool online_people_toggle(const uint8_t identity[ONLINE_IDENTITY_BYTES], unsigned flag) {
	if (flag != ONLINE_MUTED && flag != ONLINE_BLOCKED) return false;
	unsigned i = 0;
	for (; i < count; ++i) if (!memcmp(people[i].identity, identity, ONLINE_IDENTITY_BYTES)) break;
	if (i == count) {
		if (count == PEOPLE_MAX) return false;
		memcpy(people[i].identity, identity, ONLINE_IDENTITY_BYTES);
		people[i].flags = 0;
		++count;
	}
	people[i].flags ^= flag;
	if (!people[i].flags) people[i] = people[--count];
	bool ok = write_people();
	if (!ok) online_people_load();
	return ok;
}
