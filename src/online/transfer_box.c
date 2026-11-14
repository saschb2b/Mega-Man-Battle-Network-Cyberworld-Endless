/* transfer_box.h. Monocypher (third_party/monocypher, its 4.0.2 as
 * released): Argon2id for the code, XChaCha20-Poly1305 for the blob. */
#include "transfer_box.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "monocypher.h"

/* (the blob's mark, also its associated data: a blob of another kind or
 * version is refused before it is opened) */
static const uint8_t MARK[4] = { 'C', 'W', 'T', '1' };
/* (Argon2id's salt: fixed, as both systems must make the same key from the
 * code alone, and named for this one use) */
static const char SALT[] = "cyberworld.transfer.v1";
/* (a megabyte of blocks, eight passes: a few milliseconds of a computer's
 * work, a fraction of a second of a New 3DS's slower 32-bit core; each
 * code guessed against a blob costs the same) */
#define KDF_BLOCKS 1024
#define KDF_PASSES 8

bool transfer_code_make(char code[TRANSFER_DIGITS + 1], TransferRandom random) {
	uint8_t bytes[16];
	int made = 0;
	while (made < TRANSFER_DIGITS) {
		if (!random(bytes)) return false;
		/* (bytes past 249 thrown away, so every digit is as likely) */
		for (size_t i = 0; i < sizeof bytes && made < TRANSFER_DIGITS; ++i)
			if (bytes[i] < 250) code[made++] = (char)('0' + bytes[i] % 10);
	}
	code[TRANSFER_DIGITS] = 0;
	crypto_wipe(bytes, sizeof bytes);
	return true;
}

bool transfer_code_valid(const char *code) {
	if (!code) return false;
	for (int i = 0; i < TRANSFER_DIGITS; ++i)
		if (code[i] < '0' || code[i] > '9') return false;
	return code[TRANSFER_DIGITS] == 0;
}

void transfer_code_show(const char *code, char *out, size_t n) {
	if (!transfer_code_valid(code)) { snprintf(out, n, "%s", ""); return; }
	snprintf(out, n, "%.5s %.5s", code, code + 5);
}

bool transfer_keys(const char *code, TransferKeys *out) {
	if (!transfer_code_valid(code)) return false;
	void *work = malloc((size_t)KDF_BLOCKS * 1024);
	if (!work) return false;
	uint8_t made[TRANSFER_KEY + TRANSFER_LOOKUP];
	crypto_argon2_config config = { CRYPTO_ARGON2_ID, KDF_BLOCKS, KDF_PASSES, 1 };
	crypto_argon2_inputs inputs = { (const uint8_t *)code, (const uint8_t *)SALT, TRANSFER_DIGITS, (uint32_t)(sizeof SALT - 1) };
	crypto_argon2(made, sizeof made, work, config, inputs, crypto_argon2_no_extras);
	free(work);
	memcpy(out->key, made, TRANSFER_KEY);
	memcpy(out->lookup, made + TRANSFER_KEY, TRANSFER_LOOKUP);
	crypto_wipe(made, sizeof made);
	return true;
}

uint8_t *transfer_seal(const TransferKeys *keys, const uint8_t *plain, size_t n, TransferRandom random, size_t *out_n) {
	uint8_t nonce[32];
	if (!random(nonce) || !random(nonce + 16)) return NULL;
	uint8_t *sealed = malloc(TRANSFER_HEAD + n);
	if (!sealed) return NULL;
	memcpy(sealed, MARK, sizeof MARK);
	memcpy(sealed + sizeof MARK, nonce, TRANSFER_NONCE);
	uint8_t *mac = sealed + sizeof MARK + TRANSFER_NONCE;
	crypto_aead_lock(sealed + TRANSFER_HEAD, mac, keys->key, nonce, MARK, sizeof MARK, plain, n);
	*out_n = TRANSFER_HEAD + n;
	return sealed;
}

bool transfer_open(const TransferKeys *keys, uint8_t *sealed, size_t n, size_t *plain_n) {
	if (n < TRANSFER_HEAD || memcmp(sealed, MARK, sizeof MARK)) return false;
	const uint8_t *nonce = sealed + sizeof MARK, *mac = nonce + TRANSFER_NONCE;
	uint8_t *body = sealed + TRANSFER_HEAD;
	size_t size = n - TRANSFER_HEAD;
	/* (opened where it lies, then moved to the buffer's start) */
	if (crypto_aead_unlock(body, mac, keys->key, nonce, MARK, sizeof MARK, body, size)) return false;
	memmove(sealed, body, size);
	*plain_n = size;
	return true;
}
