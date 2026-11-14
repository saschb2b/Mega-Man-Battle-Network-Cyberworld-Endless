/* Saves sent by a code (docs/engine/saves.md, Sending by code): the link
 * SAVES calls to send its saves to the server under a code, or to receive
 * a code's. Online's transfer (src/online/transfer.c), which SAVES may not
 * include (tests/lint/layers.txt), sets it at the start; none where the
 * system has no network for it, and SAVES then shows no SEND or RECEIVE. */
#ifndef CW_TRANSFER_LINK_H
#define CW_TRANSFER_LINK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
	TRANSFER_IDLE,       /* nothing asked */
	TRANSFER_WORKING,    /* under way */
	TRANSFER_SENT,       /* the server holds the saves under `code` */
	TRANSFER_RECEIVED,   /* a code's saves are here: take them */
	TRANSFER_FAILED      /* ended: `trouble` says why */
} TransferPhase;

typedef enum {
	TRANSFER_FINE,
	TRANSFER_UNREACHED,  /* no connection to the server */
	TRANSFER_BUSY,       /* the server is full, or asked too often: again in `wait_s` */
	TRANSFER_NO_SAVES,   /* nothing held under that code (mistyped, taken, or its time up) */
	TRANSFER_OFF,        /* that server takes no saves (or is older than this) */
	TRANSFER_DAMAGED,    /* what came could not be opened */
	TRANSFER_TOO_BIG,    /* the saves are more than the server takes */
	TRANSFER_NO_MEMORY
} TransferTrouble;

typedef struct {
	TransferPhase phase;
	TransferTrouble trouble;
	char code[11];       /* a send's ten digits */
	uint32_t until;      /* ... SDL_GetTicks when the server lets them go */
	uint32_t wait_s;     /* BUSY's wait */
	uint32_t moved, size;   /* bytes so far, of all */
} TransferState;

typedef struct {
	/* The saves (`bytes`, taken: the link's to free) sent: false where a
	 * transfer is already under way */
	bool (*send)(uint8_t *bytes, size_t n);
	/* The ten digits' saves asked for */
	bool (*receive)(const char *code);
	/* Where it stands; a frame's call on the browser moves it on */
	void (*state)(TransferState *out);
	/* RECEIVED's saves, the caller's to free (NULL for none), and the link
	 * idle again */
	uint8_t *(*take)(size_t *n);
	/* Whatever is under way given up, and idle (a sent code stays with the
	 * server till its time is up) */
	void (*cancel)(void);
} TransferLink;

void transfer_link_set(const TransferLink *link);
/* NULL where the system has none */
const TransferLink *transfer_link(void);

#endif
