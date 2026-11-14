/* test_transfer_code.h. */
#include "test_transfer_code.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "transfer_box.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* (a source of "random" bytes the test controls: a counter, or bytes past
 * 249 first, which a code throws away) */
static uint8_t next_byte;
static bool counting(uint8_t out[16]) {
	for (int i = 0; i < 16; ++i) out[i] = next_byte++;
	return true;
}
static bool high_first(uint8_t out[16]) {
	static int calls;
	for (int i = 0; i < 16; ++i) out[i] = calls ? (uint8_t)(i * 7) : (uint8_t)(250 + i % 6);
	++calls;
	return true;
}
static bool none(uint8_t out[16]) {
	(void)out;
	return false;
}

static void test_codes(void) {
	char code[TRANSFER_DIGITS + 1], shown[16];
	next_byte = 0;
	CHECK(transfer_code_make(code, counting) && !strcmp(code, "0123456789"), "a code from the bytes 0..9: %s", code);
	CHECK(transfer_code_make(code, high_first) && !strcmp(code, "0741852963"), "bytes past 249 thrown away: %s", code);
	CHECK(!transfer_code_make(code, none), "no random bytes, no code");
	CHECK(transfer_code_valid("0123456789"), "ten digits");
	CHECK(!transfer_code_valid("012345678") && !transfer_code_valid("01234567890") && !transfer_code_valid("01234a6789") &&
		!transfer_code_valid(NULL), "nine, eleven, a letter, none");
	transfer_code_show("4829156203", shown, sizeof shown);
	CHECK(!strcmp(shown, "48291 56203"), "two groups of five: %s", shown);
	transfer_code_show("48291", shown, sizeof shown);
	CHECK(!shown[0], "a short code shows nothing: %s", shown);
}

/* What "0123456789" makes, pinned: a change here breaks every transfer
 * between builds before it and after it */
static void test_keys_pinned(void) {
	static const uint8_t lookup[TRANSFER_LOOKUP] = {
		0x5b, 0x0e, 0x55, 0xa3, 0x27, 0xbf, 0xaf, 0xc0, 0x88, 0x2c, 0xcd, 0x38, 0x9c, 0xb7, 0xb3, 0x6f,
	};
	static const uint8_t key[TRANSFER_KEY] = {
		0x89, 0x8e, 0xda, 0x57, 0x09, 0x22, 0xb5, 0xa1, 0xb7, 0xad, 0x1f, 0xb7, 0x1c, 0x32, 0xa0, 0xf9,
		0x3e, 0xbc, 0x4e, 0x6e, 0xc4, 0x70, 0xfa, 0x6c, 0x40, 0x61, 0x68, 0xa4, 0x4d, 0xb7, 0x25, 0xf1,
	};
	TransferKeys k, other;
	CHECK(transfer_keys("0123456789", &k), "keys made");
	CHECK(!memcmp(k.lookup, lookup, sizeof lookup), "the lookup as pinned");
	CHECK(!memcmp(k.key, key, sizeof key), "the key as pinned");
	CHECK(transfer_keys("0123456788", &other) && memcmp(other.lookup, k.lookup, TRANSFER_LOOKUP) &&
		memcmp(other.key, k.key, TRANSFER_KEY), "a digit apart, other keys");
	CHECK(!transfer_keys("012345678", &other), "no keys from a short code");
}

static void test_seal(void) {
	TransferKeys k, wrong;
	CHECK(transfer_keys("4829156203", &k) && transfer_keys("4829156204", &wrong), "keys made");
	uint8_t plain[1000];
	for (size_t i = 0; i < sizeof plain; ++i) plain[i] = (uint8_t)(i * 31 + 7);
	size_t n = 0, got = 0;
	next_byte = 100;
	uint8_t *sealed = transfer_seal(&k, plain, sizeof plain, counting, &n);
	CHECK(sealed && n == TRANSFER_HEAD + sizeof plain && !memcmp(sealed, "CWT1", 4), "sealed: %zu bytes", n);
	if (!sealed) return;
	CHECK(memcmp(sealed + TRANSFER_HEAD, plain, 64), "the saves unreadable in it");
	uint8_t *copy = malloc(n);
	memcpy(copy, sealed, n);
	CHECK(!transfer_open(&wrong, copy, n, &got), "another code's key opens nothing");
	CHECK(!memcmp(copy, sealed, n), "... and changes nothing");
	copy[TRANSFER_HEAD + 500] ^= 1;
	CHECK(!transfer_open(&k, copy, n, &got), "a changed byte refused");
	memcpy(copy, sealed, n);
	copy[0] = 'X';
	CHECK(!transfer_open(&k, copy, n, &got), "another mark refused");
	CHECK(!transfer_open(&k, sealed, TRANSFER_HEAD - 1, &got), "too short refused");
	CHECK(!transfer_open(&k, sealed, n - 1, &got), "cut short refused");
	memcpy(copy, sealed, n);
	CHECK(transfer_open(&k, copy, n, &got) && got == sizeof plain && !memcmp(copy, plain, got), "opened: %zu bytes", got);
	free(copy);
	size_t m = 0;
	next_byte = 200;
	uint8_t *again = transfer_seal(&k, plain, sizeof plain, counting, &m);
	CHECK(again && m == n && memcmp(again + 4, sealed + 4, TRANSFER_NONCE), "a new nonce, another blob");
	free(again);
	free(sealed);
	uint8_t *empty = transfer_seal(&k, plain, 0, counting, &m);
	CHECK(empty && m == TRANSFER_HEAD && transfer_open(&k, empty, m, &got) && got == 0, "an empty blob round trips");
	free(empty);
	CHECK(!transfer_seal(&k, plain, sizeof plain, none, &m), "no random bytes, no seal");
}

int test_transfer_code(void) {
	failures = 0;
	test_codes();
	test_keys_pinned();
	test_seal();
	printf("transfer code: %s\n", failures ? "FAILED" : "ok");
	return failures;
}
