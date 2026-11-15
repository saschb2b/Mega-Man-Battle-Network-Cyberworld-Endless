/* Saves kept in step (docs/engine/saves.md, Saves kept in step): the link
 * SAVES and the title call to keep this system's saves in step with a slot
 * on the server that the player's other systems share, as Steam Cloud
 * keeps a game's. Online's cloud (src/online/cloud.c), which SAVES may not
 * include, sets it at the start; none where the system has no network for
 * it. What the link does is the transport and the receipt (which
 * generation of the slot these saves are in step with); what to do with a
 * pull (take it, or compare) is SAVES' (saves_cloud.c). */
#ifndef CW_CLOUD_LINK_H
#define CW_CLOUD_LINK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum { CLOUD_IDLE, CLOUD_PULLING, CLOUD_PUSHING, CLOUD_DROPPING } CloudDoing;

typedef enum {
	CLOUD_FINE,
	CLOUD_UNREACHED,     /* no connection to the server */
	CLOUD_BUSY,          /* the server is full or busy */
	CLOUD_GONE,          /* the slot is not there (deleted, or unused 180 days) */
	CLOUD_DENIED,        /* the slot's, but not our auth */
	CLOUD_OFF,           /* that server keeps no slots (or is older than this) */
	CLOUD_CONFLICT,      /* a push came after another system's: pull first */
	CLOUD_DAMAGED,       /* what came could not be opened */
	CLOUD_TOO_BIG,
	CLOUD_NO_MEMORY
} CloudTrouble;

typedef struct {
	bool linked;          /* this system has a slot */
	CloudDoing doing;
	CloudTrouble trouble; /* the last exchange's (FINE: it went well) */
	bool pulled;          /* a pull's newer saves wait (take_pulled) */
	uint32_t generation;  /* the receipt: the slot's generation these saves are in step with (0 none yet) */
	uint32_t hash;        /* ... these saves' hash (BackupInfo.hash) then */
	uint64_t synced_at;   /* the last exchange that went well, seconds since 1970 (0 none) */
	uint32_t moved, size; /* bytes so far, of all */
	uint32_t ends;        /* exchanges ended so far (a new one: its trouble to act on) */
} CloudState;

typedef struct {
	/* Where it stands; a frame's call on the browser moves it on */
	void (*state)(CloudState *out);
	/* A new slot's secret made here (linked, nothing sent yet: the first
	 * push makes the slot) */
	bool (*create)(void);
	/* A link's bytes (from another system's ADD A SYSTEM): linked to its
	 * slot, the receipt empty; false where they are not a link */
	bool (*join)(const uint8_t *bytes, size_t n);
	/* This slot's link, for another system (the caller's to free) */
	uint8_t *(*link_bytes)(size_t *n);
	/* The slot forgotten here (the server's copy stays for the others) */
	void (*unlink)(void);
	/* The slot's saves, where newer than the receipt's generation */
	bool (*pull)(void);
	/* These saves (`saves`, a .cwsave, taken) made from generation `from`,
	 * `hash` theirs (the receipt's once kept) */
	bool (*push)(uint8_t *saves, size_t n, uint32_t from, uint32_t hash);
	/* The slot deleted on the server, and forgotten here */
	bool (*drop)(void);
	/* A pull's saves (a .cwsave, the caller's to free) and their
	 * generation */
	uint8_t *(*take_pulled)(size_t *n, uint32_t *generation);
	/* The receipt set: these saves, `hash`, in step with `generation` (a
	 * pull taken in) */
	void (*synced)(uint32_t generation, uint32_t hash);
	/* What is under way waited for, `ms` at most (the game closing) */
	void (*wait)(uint32_t ms);
} CloudLink;

void cloud_link_set(const CloudLink *link);
/* NULL where the system has none */
const CloudLink *cloud_link(void);

#endif
