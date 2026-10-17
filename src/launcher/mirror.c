/* mirror.h. What the copy has done is kept in the data folder's
 * saves-receipts.txt, device-local and never exported: the hashes of the
 * last files this device wrote out (wrote HASH), and for each place
 * (folder ANSWERED ALLOWS COPIED_AT SCOPE) the file the player answered
 * there, whether that answer lets the copy write over it, and the time of
 * the last copy. A look at the place reads its file into memory and
 * writes nothing. */
#include "mirror.h"
#include "online_trade_gate.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL.h>

#include "game.h"
#include "pick.h"
#include "platform.h"
#include "save_queue.h"

#define REST_MS 3000      /* the saves untouched this long: a checkpoint's files all written */
#define WROTE_KEPT 8
#define PLACES_KEPT 4
#define RECEIPTS "saves-receipts.txt"

typedef struct {
	char scope[1024];
	uint32_t answered;   /* the file answered there (0 none) */
	bool allows;         /* ... and the copy may write over it */
	uint64_t copied_at;
} Place;

static struct {
	char dir[512];       /* the data folder they were read from */
	uint32_t wrote[WROTE_KEPT];
	Place places[PLACES_KEPT];
	int count;
} rec;

/* what the last look found */
enum { LOOK_FREE, LOOK_OURS, LOOK_HELD, LOOK_UNREAD };
static bool looked, found_shown, have_stat, place_taken;
static int verdict;
static char place[1024], scope[1024];
static uint8_t *found;
static size_t found_n;
static uint32_t found_hash;
static uint64_t seen_size;
static int64_t seen_mtime;

static bool dirty, failed;
static uint32_t written_at, put_hash;
static BackupInfo local;
static bool local_read, local_have;

static uint32_t fnv(const uint8_t *b, size_t n) {
	uint32_t h = 2166136261u;
	for (size_t i = 0; i < n; ++i) h = (h ^ b[i]) * 16777619u;
	return h;
}

/* ---- the receipts ---- */

static void receipts_load(void) {
	if (!strcmp(rec.dir, g_data_dir)) return;
	memset(&rec, 0, sizeof rec);
	snprintf(rec.dir, sizeof rec.dir, "%s", g_data_dir);
	char path[600], line[1200];
	snprintf(path, sizeof path, "%s/%s", g_data_dir, RECEIPTS);
	FILE *f = fopen(path, "r");
	if (!f) return;
	int w = 0;
	while (fgets(line, sizeof line, f)) {
		line[strcspn(line, "\r\n")] = 0;
		unsigned hash, allows;
		unsigned long long at;
		int at_scope = 0;
		if (sscanf(line, "wrote %x", &hash) == 1 && w < WROTE_KEPT) rec.wrote[w++] = hash;
		else if (sscanf(line, "folder %x %u %llu %n", &hash, &allows, &at, &at_scope) == 3 && at_scope && rec.count < PLACES_KEPT) {
			Place *p = &rec.places[rec.count++];
			snprintf(p->scope, sizeof p->scope, "%s", line + at_scope);
			p->answered = hash;
			p->allows = allows != 0;
			p->copied_at = at;
		}
	}
	fclose(f);
}

static void receipts_save(void) {
	char path[600], text[6000];
	size_t used = 0;
	for (int i = 0; i < WROTE_KEPT && rec.wrote[i]; ++i)
		used += (size_t)snprintf(text + used, sizeof text - used, "wrote %08x\n", (unsigned)rec.wrote[i]);
	for (int i = 0; i < rec.count && used < sizeof text; ++i) {
		const Place *p = &rec.places[i];
		used += (size_t)snprintf(text + used, sizeof text - used, "folder %08x %d %llu %s\n", (unsigned)p->answered, p->allows ? 1 : 0,
			(unsigned long long)p->copied_at, p->scope);
	}
	if (used >= sizeof text) return;
	snprintf(path, sizeof path, "%s/%s", g_data_dir, RECEIPTS);
	/* (no platform_persist where it would note a change of the saves: the
	 * browser's, which keeps its files only when told) */
	if (backup_write_file(path, (const uint8_t *)text, used) && !(pick_saves_capabilities() & SAVES_CAN_AUTO)) platform_persist();
}

/* the place's own entry, made (first, the oldest gone) where `make` */
static Place *place_entry(bool make) {
	receipts_load();
	if (!scope[0] || strpbrk(scope, "\r\n")) return NULL;
	for (int i = 0; i < rec.count; ++i)
		if (!strcmp(rec.places[i].scope, scope)) return &rec.places[i];
	if (!make) return NULL;
	if (rec.count == PLACES_KEPT) --rec.count;
	memmove(&rec.places[1], &rec.places[0], (size_t)rec.count * sizeof *rec.places);
	++rec.count;
	memset(&rec.places[0], 0, sizeof *rec.places);
	snprintf(rec.places[0].scope, sizeof rec.places[0].scope, "%s", scope);
	return &rec.places[0];
}

static bool wrote_here(uint32_t hash) {
	receipts_load();
	for (int i = 0; i < WROTE_KEPT; ++i) if (rec.wrote[i] == hash) return true;
	return false;
}

void mirror_wrote(const uint8_t *bytes, size_t n) {
	uint32_t hash = fnv(bytes, n);
	if (wrote_here(hash)) return;
	memmove(&rec.wrote[1], &rec.wrote[0], (WROTE_KEPT - 1) * sizeof *rec.wrote);
	rec.wrote[0] = hash;
	receipts_save();
}

/* ---- the local saves ---- */

bool mirror_local(BackupInfo *info) {
	if (!local_read) {
		local_have = backup_local_info(g_data_dir, &local);
		local_read = true;
	}
	*info = local;
	return local_have;
}

/* ---- the look at the place ---- */

static void forget_found(void) {
	free(found);
	found = NULL;
	found_n = 0;
	found_hash = 0;
}

static void take_place(void) {
	if (!pick_saves_place(place, sizeof place)) place[0] = 0;
	if (!pick_saves_scope(scope, sizeof scope)) scope[0] = 0;
	place_taken = true;
}

/* (the file as the last look saw it, by its size and time, where the
 * system can tell without reading it; a phone's provider cannot, and is
 * read once a start) */
static bool unchanged(void) {
	uint64_t size = 0;
	int64_t mtime = 0;
	bool there = pick_saves_stat(&size, &mtime);
	return there == have_stat && (!there || (size == seen_size && mtime == seen_mtime));
}

static void judge(uint8_t *b, size_t n) {
	uint32_t hash = fnv(b, n);
	const Place *p = place_entry(false);
	BackupInfo info, here;
	if (wrote_here(hash) || (p && p->allows && p->answered == hash)) verdict = LOOK_OURS;
	else if (backup_info(b, n, &info) && mirror_local(&here) && info.hash == here.hash) {
		verdict = LOOK_OURS;
		put_hash = here.hash;   /* (the same saves: no copy of them again) */
	} else {
		verdict = LOOK_HELD;
		found = b;
		found_n = n;
		found_hash = hash;
		found_shown = p && p->answered == hash;
		return;
	}
	free(b);
}

static void look(void) {
	if (!online_trade_gate_transfer(g_data_dir)) return;
	if (pick_busy() || pick_saves_busy()) return;   /* (once the picker is done) */
	char was[sizeof scope];
	snprintf(was, sizeof was, "%s", scope);
	take_place();
	if (looked && !strcmp(was, scope) && unchanged()) return;
	looked = true;
	forget_found();
	found_shown = false;
	verdict = LOOK_FREE;
	have_stat = pick_saves_stat(&seen_size, &seen_mtime);
	save_queue_wait();   /* (the saves as the game last wrote them) */
	uint8_t *b = NULL;
	size_t n = 0;
	int r = place[0] ? pick_saves_read(&b, &n) : 0;
	if (r < 0) verdict = LOOK_UNREAD;
	else if (r > 0) judge(b, n);
}

void mirror_rescan(void) { looked = false; }

void mirror_new_folder(void) {
	looked = false;
	failed = false;
	put_hash = 0;
	take_place();
	if (dirty || pick_saves_auto_enabled()) {
		dirty = true;
		written_at = SDL_GetTicks() - REST_MS;
	}
}

void mirror_look(void) { look(); }

bool mirror_incoming(void) {
	/* (the copy off: its place compared only when the player asks, IMPORT) */
	if (!pick_saves_auto_enabled()) return false;
	if (!looked) look();
	if (!found || found_shown) return false;
	found_shown = true;
	return true;
}

bool mirror_held(const uint8_t **bytes, size_t *n) {
	if (!found) return false;
	*bytes = found;
	*n = found_n;
	return true;
}

void mirror_answer(bool allows) {
	if (!found) return;
	Place *p = place_entry(true);
	if (p) {
		p->answered = found_hash;
		p->allows = allows;
		receipts_save();
	}
	found_shown = true;
	if (!allows) return;
	/* (the copy writes over it at this device's next save, not at once) */
	forget_found();
	verdict = LOOK_OURS;
	put_hash = 0;
}

/* ---- the copy ---- */

void mirror_status(MirrorStatus *out) {
	memset(out, 0, sizeof *out);
	if (!place_taken) take_place();
	snprintf(out->place, sizeof out->place, "%s", place);
	if (!pick_saves_auto_enabled()) return;
	const Place *p = place_entry(false);
	if (p) out->copied_at = p->copied_at;
	if (verdict == LOOK_UNREAD) out->state = MIRROR_UNREAD;
	else if (verdict == LOOK_HELD) out->state = MIRROR_HELD;
	else if (failed) out->state = MIRROR_FAILED;
	else out->state = out->copied_at ? MIRROR_COPIED : MIRROR_WAITING;
}

static void put(void) {
	if (!dirty || pick_busy() || pick_saves_busy() || !pick_saves_auto_enabled()) return;
	/* (the place looked at once a start; a desktop's file looked at again
	 * where it changed since) */
	if (!looked || !unchanged()) {
		looked = false;
		look();
	}
	if (!looked || verdict == LOOK_HELD || verdict == LOOK_UNREAD || !place[0]) return;
	dirty = false;
	size_t n = 0;
	uint8_t *b = backup_pack(g_data_dir, 0, &n);
	BackupInfo info;
	if (!b || !backup_info(b, n, &info) || info.hash == put_hash) { free(b); return; }
	bool ok = pick_saves_put(b, n);
	failed = !ok;
	if (ok) {
		put_hash = info.hash;
		mirror_wrote(b, n);
		Place *p = place_entry(true);
		time_t now = time(NULL);
		if (p) p->copied_at = now > 0 ? (uint64_t)now : 1;
		receipts_save();
		verdict = LOOK_OURS;
		have_stat = pick_saves_stat(&seen_size, &seen_mtime);
	} else fprintf(stderr, "saves: the copy in %s could not be written\n", place);
	free(b);
}

void mirror_note(void) {
	local_read = false;
	dirty = true;
	written_at = SDL_GetTicks();
}

/* (no copy while the last look holds it; a look due again, after a resume
 * or a folder chosen, is put()'s to make) */
void mirror_tick(void) {
	if (!online_trade_gate_transfer(g_data_dir)) return;
	bool held = looked && (verdict == LOOK_HELD || verdict == LOOK_UNREAD);
	if (dirty && !held && SDL_GetTicks() - written_at >= REST_MS) put();
}

void mirror_flush(void) { put(); }
