/* saves_cloud.h. The link carries (src/online/cloud.c); here is when, and
 * what a pull's saves become: taken in silently where this system has not
 * moved on since it was last in step (an import as IMPORT's, UNDO IMPORT
 * after it), compared where both moved on. Nothing here writes the saves
 * but through backup_restore. */
#include "saves_cloud.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "backup.h"
#include "cloud_link.h"
#include "game.h"
#include "mirror.h"
#include "rom.h"
#include "saves.h"
#include "saves_actions.h"
#include "saves_code.h"
#include "saves_state.h"
#include "saves_text.h"

#define REST_MS 3000      /* a push this long after the saves were last written (mirror.c's rest) */
#define SETTLE_MS 6000    /* the start's pull waited for at most */
#define CLOSE_MS 4000     /* a push as the game closes waited for at most */
#define NOTE_FRAMES 330   /* the title's note shown */

static struct {
	bool pull_due;        /* a pull as soon as the title or SAVES is up */
	bool settling;        /* the start's pull under way */
	uint32_t settle_since;
	bool dirty;           /* the saves written since the last push looked */
	uint32_t written_at;
	uint32_t ends_seen;   /* the link's exchanges ended, as handled */
	uint32_t compare_gen; /* the slot's generation the comparison shows */
	char note[160];
	int note_frame;       /* frames the note has been up; -1 none */
} K = { .note_frame = -1 };

/* (the link as the last frame's update found it: the picture's and the
 * buttons') */
static CloudState shown;

static bool bn5(void) { return XR[XROM_BN5_COLONEL_US].data != NULL; }

/* (the CLOUD view's cursor on its first button, as the link stands now) */
static void focus_first(void) {
	if (cloud_link()) cloud_link()->state(&shown);
	int actions[SAVES_ACTIONS];
	int n = saves_actions(actions);
	SV.focus = n ? actions[0] : SAVES_DONE;
}

static void note(const char *text) {
	snprintf(K.note, sizeof K.note, "%s", text);
	K.note_frame = 0;
}

/* (the 3DS's heap: no pack and seal in a run, only at the title and as it closes) */
static bool may_push_now(void) {
#ifdef __3DS__
	return scene_current() != &scene_emu;
#else
	return true;
#endif
}

/* These saves pushed where they are not the ones the slot has */
static void push_now(const CloudState *s) {
	const CloudLink *c = cloud_link();
	BackupInfo info;
	if (!c || !s->linked || !mirror_local(&info)) return;
	if (s->generation && info.hash == s->hash) return;
	size_t n = 0;
	uint8_t *bytes = backup_pack(g_data_dir, 0, &n);
	if (bytes) c->push(bytes, n, s->generation, info.hash);
}

/* ---- the saves' writes (mirror.h's hooks: every scene) ---- */

static void written(void) {
	K.dirty = true;
	K.written_at = SDL_GetTicks();
}

static void each_frame(void) {
	const CloudLink *c = cloud_link();
	if (!c) return;
	CloudState s;
	c->state(&s);
	/* (a push after another system's waits for the pull at the title) */
	if (!s.linked || !K.dirty || s.trouble == CLOUD_CONFLICT || SDL_GetTicks() - K.written_at < REST_MS || !may_push_now()) return;
	K.dirty = false;
	push_now(&s);
}

static void closing(void) {
	const CloudLink *c = cloud_link();
	if (!c) return;
	CloudState s;
	c->state(&s);
	if (!s.linked || s.trouble == CLOUD_CONFLICT) return;
	K.dirty = false;
	push_now(&s);
	c->wait(CLOSE_MS);
}

static void came_back(void) { K.pull_due = true; }

static const MirrorFollow FOLLOW = { written, each_frame, closing, came_back };

void saves_cloud_start(void) {
	if (!cloud_link()) return;
	mirror_follow(&FOLLOW);
	K.pull_due = true;
}

/* ---- a pull's saves ---- */

/* Taken in where this system has not moved on: an import, the receipt the
 * slot's generation, the title shown again with them */
static void take_in(uint8_t *bytes, size_t n, uint32_t generation, const BackupInfo *cloud) {
	const CloudLink *c = cloud_link();
	BackupStatus status = backup_check(cloud, bn5());
	if (status == BACKUP_OK) status = backup_restore(g_data_dir, bytes, n, bn5());
	free(bytes);
	char text[160];
	if (status != BACKUP_OK) {
		saves_cloud_refused(status, cloud, text, sizeof text);
		note(text);
		return;
	}
	saves_reload();
	BackupInfo here;
	c->synced(generation, mirror_local(&here) ? here.hash : 0);
	saves_cloud_from(cloud, text, sizeof text);
	note(text);
	if (scene_current() == &scene_title) scene_set(&scene_title);
}

static void pulled(const CloudState *s) {
	const CloudLink *c = cloud_link();
	size_t n = 0;
	uint32_t generation = 0;
	uint8_t *bytes = c->take_pulled(&n, &generation);
	BackupInfo cloud, here;
	if (!bytes || !backup_info(bytes, n, &cloud)) {
		free(bytes);
		note(saves_cloud_trouble(CLOUD_DAMAGED));
		return;
	}
	bool have = mirror_local(&here);
	if (have && here.hash == cloud.hash) {
		free(bytes);
		c->synced(generation, here.hash);
		return;
	}
	/* (this system moved on since it was last in step: both did) */
	bool moved = have && !(s->generation && here.hash == s->hash);
	if (!moved) { take_in(bytes, n, generation, &cloud); return; }
	K.compare_gen = generation;
	saves_compare_cloud(bytes, n);
}

/* An exchange ended: what it left to do */
static void ended(const CloudState *s) {
	const CloudLink *c = cloud_link();
	switch (s->trouble) {
	case CLOUD_FINE: return;
	case CLOUD_CONFLICT: c->pull(); return;
	case CLOUD_GONE:
	case CLOUD_DENIED: c->unlink(); break;
	default: break;
	}
	note(saves_cloud_trouble(s->trouble));
}

void saves_cloud_tick(void) {
	const CloudLink *c = cloud_link();
	if (K.note_frame >= 0 && ++K.note_frame > NOTE_FRAMES) K.note_frame = -1;
	if (!c) return;
	CloudState s;
	c->state(&s);
	shown = s;
	if (!s.linked) {
		K.settling = false;
		return;
	}
	uint32_t now = SDL_GetTicks();
	if (K.pull_due && s.doing == CLOUD_IDLE) {
		K.pull_due = false;
		K.settling = c->pull();
		K.settle_since = now;
		return;
	}
	if (K.settling && (s.doing == CLOUD_IDLE || now - K.settle_since > SETTLE_MS)) K.settling = false;
	if (s.ends != K.ends_seen && s.doing == CLOUD_IDLE) {
		K.ends_seen = s.ends;
		ended(&s);
	}
	if (s.pulled && !SV.busy && SV.view != SAVES_COMPARE) pulled(&s);
}

bool saves_cloud_settling(void) { return K.settling; }

bool saves_cloud_note(char *out, size_t n, int *t, int *len) {
	*len = NOTE_FRAMES;
	if (K.settling) {
		snprintf(out, n, "%s", saves_cloud_syncing());
		*t = NOTE_FRAMES / 2;
		return true;
	}
	if (K.note_frame < 0) return false;
	snprintf(out, n, "%s", K.note);
	*t = K.note_frame;
	return true;
}

/* ---- SAVES' buttons ---- */

bool saves_cloud_here(void) { return cloud_link() != NULL; }

bool saves_cloud_on(void) { return shown.linked; }

int saves_cloud_actions(int *out) {
	int n = 0;
	if (shown.linked) {
		out[n++] = SAVES_ADD;
		out[n++] = SAVES_UNLINK;
		out[n++] = SAVES_DELETE;
	} else {
		if (SV.have) out[n++] = SAVES_TURN_ON;
		out[n++] = SAVES_LINK;
	}
	return n;
}

void saves_cloud_line(char *out, size_t n) {
	if (SV.said[0]) snprintf(out, n, "%s", SV.said);
	else saves_cloud_status(&shown, out, n);
}

static void turn_on(void) {
	const CloudLink *c = cloud_link();
	if (!c->create()) return;
	CloudState s;
	c->state(&s);
	K.dirty = false;
	push_now(&s);
	SV.said[0] = 0;   /* (the line says the sending, then that they are in step) */
}

static void add_system(void) {
	size_t n = 0;
	uint8_t *bytes = cloud_link()->link_bytes(&n);
	saves_code_send_link(bytes, n);
}

static void delete_slot(void) {
	if (!SV.delete_asked) {
		SV.delete_asked = true;
		return;
	}
	SV.delete_asked = false;
	cloud_link()->drop();
	snprintf(SV.said, sizeof SV.said, "%s", saves_cloud_deleted());
}

bool saves_cloud_press(int action) {
	const CloudLink *c = cloud_link();
	if (!c) return false;
	if (action != SAVES_DELETE) SV.delete_asked = false;
	switch (action) {
	case SAVES_CLOUD:
		SV.view = SAVES_CLOUD_VIEW;
		SV.said[0] = 0;
		focus_first();
		return true;
	case SAVES_TURN_ON: turn_on(); break;
	case SAVES_LINK: saves_code_press(SAVES_RECEIVE); return true;
	case SAVES_ADD: add_system(); return true;
	case SAVES_UNLINK:
		c->unlink();
		snprintf(SV.said, sizeof SV.said, "%s", saves_cloud_unlinked());
		break;
	case SAVES_DELETE: delete_slot(); break;
	default: return false;
	}
	focus_first();
	return true;
}

bool saves_cloud_join(const uint8_t *bytes, size_t n) {
	const CloudLink *c = cloud_link();
	if (!c || !c->join(bytes, n)) return false;
	K.pull_due = true;
	SV.view = SAVES_CLOUD_VIEW;
	SV.said[0] = 0;   /* (the line says the syncing) */
	focus_first();
	return true;
}

void saves_cloud_answered(bool took) {
	const CloudLink *c = cloud_link();
	if (!c) return;
	BackupInfo here;
	bool have = mirror_local(&here);
	if (took) {
		c->synced(K.compare_gen, have ? here.hash : 0);
		return;
	}
	/* (KEEP THESE: this system's saves over the slot's, made from the
	 * generation the comparison showed) */
	size_t n = 0;
	uint8_t *bytes = have ? backup_pack(g_data_dir, 0, &n) : NULL;
	if (bytes) c->push(bytes, n, K.compare_gen, here.hash);
}
