/* Saves sent by a code (docs/engine/saves.md, Sending by code): the ten
 * digits a player reads off one system and types on the other, what they
 * make, and the sealed blob the server holds. The code is never sent:
 * from it come the lookup the server files the blob under and the key
 * that seals it (Argon2id, Monocypher's), so the server can open neither.
 * What a code makes must never change, or a system of one build could not
 * open what another sent: tests/test_transfer_code.c pins it. */
#ifndef CW_TRANSFER_BOX_H
#define CW_TRANSFER_BOX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TRANSFER_DIGITS 10
#define TRANSFER_LOOKUP 16
#define TRANSFER_KEY 32
#define TRANSFER_NONCE 24
/* A sealed blob's head before the sealed bytes: "CWT1", the nonce, the MAC */
#define TRANSFER_HEAD (4 + TRANSFER_NONCE + 16)

typedef struct {
	uint8_t lookup[TRANSFER_LOOKUP];
	uint8_t key[TRANSFER_KEY];
} TransferKeys;

/* Sixteen random bytes (network_identity, network_prefs.h): false where the
 * system gave none */
typedef bool (*TransferRandom)(uint8_t out[16]);

/* A new code: ten digits and a null, from `random`'s bytes */
bool transfer_code_make(char code[TRANSFER_DIGITS + 1], TransferRandom random);
/* Ten digits and nothing else */
bool transfer_code_valid(const char *code);
/* A code as the screens show it: two groups of five, "48291 56203" */
void transfer_code_show(const char *code, char *out, size_t n);
/* The code's lookup and key: a megabyte of work memory for a moment;
 * false where there was none */
bool transfer_keys(const char *code, TransferKeys *out);
/* `plain` sealed: a new buffer of TRANSFER_HEAD + n bytes (`out_n`), its
 * nonce from `random`; NULL where it could not be */
uint8_t *transfer_seal(const TransferKeys *keys, const uint8_t *plain, size_t n, TransferRandom random, size_t *out_n);
/* `sealed` opened in place: its first `plain_n` bytes the saves; false
 * where it is not one of ours, or was sealed with another key or changed */
bool transfer_open(const TransferKeys *keys, uint8_t *sealed, size_t n, size_t *plain_n);

#endif
