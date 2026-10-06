/* launcher_roms.h. A file is told by its header's game code before it is
 * read whole (rom.c checks BN6's SHA-1, xrom_load BN5's); a ROM read once
 * is never read again while the game holds pointers into it. */
#include "launcher_roms.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "game.h"
#include "launcher_text.h"
#include "platform.h"
#include "rom.h"
#include "startup.h"

#define HEAD 0xB0         /* a GBA ROM's header, its game code at 0xAC */
#define BN6_CODE "BR5E"   /* (rom.c's layout's) */

bool roms_have(int slot) { return slot == SLOT_BN5 ? XR[XROM_BN5_COLONEL_US].data != NULL : R.data != NULL; }

static const char *base_name(const char *path) {
	const char *a = strrchr(path, '/'), *b = strrchr(path, '\\');
	if (b > a) a = b;
	return a ? a + 1 : path;
}

static bool exists(const char *path) {
	FILE *f = fopen(path, "rb");
	if (f) fclose(f);
	return f != NULL;
}

static bool copy_file(const char *from, const char *to) {
	FILE *in = fopen(from, "rb");
	if (!in) return false;
	FILE *out = fopen(to, "wb");
	if (!out) { fclose(in); return false; }
	static char buf[65536];
	size_t n;
	bool ok = true;
	while ((n = fread(buf, 1, sizeof buf, in)) > 0) ok = ok && fwrite(buf, 1, n, out) == n;
	fclose(in);
	ok = fclose(out) == 0 && ok;
	if (!ok) remove(to);
	return ok;
}

/* one ROM's copy in rom_dir, where it is not already */
static void keep_copy(const char *path, const char *rom_dir) {
	char to[1400];
	snprintf(to, sizeof to, "%s/%s", rom_dir, base_name(path));
	if (strcmp(to, path) && !exists(to) && !copy_file(path, to))
		fprintf(stderr, "could not copy the ROM to %s; it is used from %s\n", to, path);
}

void roms_keep_copies(const char *rom_dir) {
	if (R.data) keep_copy(R.path, rom_dir);
	for (int i = 0; i < XROM_COUNT; ++i)
		if (XR[i].data) keep_copy(XR[i].path, rom_dir);
}

static bool zipped(const char *name) {
	size_t n = strlen(name);
	return (n > 4 && !strcasecmp(name + n - 4, ".zip")) || (n > 3 && !strcasecmp(name + n - 3, ".7z")) ||
		(n > 4 && !strcasecmp(name + n - 4, ".rar"));
}

/* the file's header: how much of it there was (-1: it cannot be read) */
static int header(const char *path, uint8_t head[HEAD]) {
	FILE *f = fopen(path, "rb");
	if (!f) return -1;
	int got = (int)fread(head, 1, HEAD, f);
	fclose(f);
	return got;
}

int roms_take(const char *path, const char *rom_dir, char *note, size_t n, bool *beside) {
	const char *file = base_name(path);
	uint8_t head[HEAD];
	int got = zipped(file) ? 0 : header(path, head);
	*beside = false;
	if (zipped(file) || got < HEAD) {
		note_refused(file, got < 0 ? "could not be read" : refuse_not_rom(zipped(file)), note, n);
		return -1;
	}
	const char *code = (const char *)head + 0xAC;
	int slot = !memcmp(code, BN6_CODE, 4) ? SLOT_BN6 : xrom_of_code(code) == XROM_BN5_COLONEL_US ? SLOT_BN5 : -1;
	if (slot < 0) {
		note_refused(file, refuse_why(code, -1, false), note, n);
		return -1;
	}
	/* (one read already stays: the game draws from its bytes) */
	if (roms_have(slot)) {
		note_slot_ready(slot, NULL, note, n);
		return slot;
	}
	bool five = roms_have(SLOT_BN5), ok;
	if (slot == SLOT_BN6) {
		char msg[512];
		ok = rom_load_file(path, msg, sizeof msg);
	} else ok = xrom_load(path);
	if (!ok) {
		note_refused(file, refuse_why(code, slot, true), note, n);
		return -1;
	}
	if (rom_dir) roms_keep_copies(rom_dir);
	*beside = slot == SLOT_BN6 && !five && roms_have(SLOT_BN5);
	note_took(slot, file, *beside, note, n);
	return slot;
}

void roms_reload(const char *rom_dir) {
	char msg[512];
#ifdef CW_DESKTOP
	/* (the game's own rom/, beside the program, ./rom, then front ends' and
	 * Downloads, as at the start) */
	if (!R.data) desktop_rom_anywhere(msg, sizeof msg);
#else
	if (!R.data && rom_dir) rom_find(rom_dir, msg, sizeof msg);
#ifdef CW_IOS
	/* (Files puts a file dropped on the app's folder at its top) */
	if (!R.data) rom_find(g_data_dir, msg, sizeof msg);
	if (R.data) xrom_find(g_data_dir);
#endif
#endif
	if (R.data && rom_dir) xrom_find(rom_dir);
	if (R.data) xrom_find_beside();
}

/* ---- the last start ---- */

static void record_path(char *out, size_t n) { snprintf(out, n, "%s/launcher.ini", g_data_dir); }

RomRecord roms_record_read(void) {
	RomRecord r = { 0 };
	char path[600], line[128];
	record_path(path, sizeof path);
	FILE *f = fopen(path, "r");
	if (!f) return r;
	r.known = true;
	while (fgets(line, sizeof line, f)) {
		char key[16], val[16];
		if (line[0] == '#' || sscanf(line, " %15[a-z0-9] = %15s", key, val) != 2) continue;
		bool yes = !strcmp(val, "yes");
		if (!strcmp(key, "bn6")) r.bn6 = yes;
		if (!strcmp(key, "bn5")) r.bn5 = yes;
	}
	fclose(f);
	return r;
}

void roms_record_write(void) {
	char path[600];
	record_path(path, sizeof path);
	FILE *f = fopen(path, "w");
	if (!f) return;
	fprintf(f, "# Cyberworld Endless: the ROMs the last start played with. The ROMs\n"
		"# screen shows again at a start where one of them is gone; delete this\n"
		"# file to see it at the next start.\n\nbn6 = %s\nbn5 = %s\n",
		roms_have(SLOT_BN6) ? "yes" : "no", roms_have(SLOT_BN5) ? "yes" : "no");
	fclose(f);
	platform_persist();
}
