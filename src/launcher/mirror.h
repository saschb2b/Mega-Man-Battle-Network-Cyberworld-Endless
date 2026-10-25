/* The automatic copy (docs/engine/saves.md, The automatic copy): after the saves
 * rest a few seconds, one .cwsave in the transfer place (pick.h: a
 * phone's ROM folder by default, a folder chosen in SAVES, a desktop's
 * Downloads, the data folder where there is no other). It writes only
 * where the place's file is absent, one this device wrote, or one the
 * player answered in SAVES' comparison (USE THE FILE'S or KEEP THESE);
 * any other file holds it, and SAVES compares that file first. The place
 * is looked at once a start or resume, and as SAVES opens, never every
 * frame. The browser has no copy: it exports by download. */
#ifndef CW_MIRROR_H
#define CW_MIRROR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "backup.h"

typedef enum {
	MIRROR_OFF,       /* no automatic copy: none on this system, or turned off */
	MIRROR_WAITING,   /* none made there yet */
	MIRROR_COPIED,    /* made, at copied_at */
	MIRROR_FAILED,    /* the last could not be written */
	MIRROR_HELD,      /* the place holds another device's saves: none over them */
	MIRROR_UNREAD     /* the place's file could not be read: none over it */
} MirrorState;

typedef struct {
	MirrorState state;
	uint64_t copied_at;   /* the last copy there, seconds since 1970; 0 none */
	char place[1024];     /* the transfer place, "" none */
} MirrorStatus;

/* For the screens' lines: kept since the last look, no disk read */
void mirror_status(MirrorStatus *out);

/* Files were written (platform_persist): the local saves read again when
 * next asked, a copy due once they rest */
void mirror_note(void);
/* Each frame: the copy, once the saves have rested */
void mirror_tick(void);
/* The app going away (quit, sent to the background): the copy made now,
 * where it is due */
void mirror_flush(void);
/* A start or a resume: the place looked at again when next needed */
void mirror_rescan(void);
/* A folder newly chosen: looked at again, its last failure forgotten */
void mirror_new_folder(void);
/* SAVES opened: the place looked at now (the player asked; a desktop's
 * Downloads only then, or with the copy turned on) */
void mirror_look(void);

/* Once a look: true where the place holds saves not yet answered (the
 * title opens their comparison) */
bool mirror_incoming(void);
/* The place's file that holds the copy: its bytes (the mirror's, until
 * answered), false for none */
bool mirror_held(const uint8_t **bytes, size_t *n);
/* The player's answer about that file: `allows` (USE THE FILE'S, KEEP
 * THESE) lets the copy write over it at the next save; either way it does
 * not open by itself again (kept on disk, per folder) */
void mirror_answer(bool allows);

/* Bytes this device wrote out (an export, a copy): never taken for another
 * device's where they turn up again */
void mirror_wrote(const uint8_t *bytes, size_t n);
/* The local saves' summary (backup_local_info), read again only after
 * mirror_note: false for none */
bool mirror_local(BackupInfo *info);

#endif
