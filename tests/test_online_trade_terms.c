#include "test_online_trade_terms.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "online_trade_terms.h"
#include "sha256.h"

static int failures;
#define CHECK(value) do { if (!(value)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #value); ++failures; } } while (0)

static void hex(const uint8_t *data, size_t n, char *out) {
	for (size_t i = 0; i < n; ++i) snprintf(out + i * 2, 3, "%02x", data[i]);
}

static void hash_vector(const char *message, const char *expected) {
	uint8_t digest[32];
	char text[65];
	cw_sha256(message, strlen(message), digest);
	hex(digest, 32, text);
	CHECK(!strcmp(text, expected));
	CwSha256 hash;
	cw_sha256_init(&hash);
	for (size_t i = 0; message[i]; ++i) cw_sha256_update(&hash, message + i, 1);
	cw_sha256_final(&hash, digest);
	hex(digest, 32, text);
	CHECK(!strcmp(text, expected));
}

static void hash_vectors(void) {
	hash_vector("", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
	hash_vector("abc", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
	hash_vector("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
	CwSha256 hash;
	uint8_t digest[32];
	char thousand[1000], text[65];
	memset(thousand, 'a', sizeof thousand);
	cw_sha256_init(&hash);
	for (int i = 0; i < 1000; ++i) cw_sha256_update(&hash, thousand, sizeof thousand);
	cw_sha256_final(&hash, digest);
	hex(digest, 32, text);
	CHECK(!strcmp(text, "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"));
}

static bool vector_read(uint8_t bytes[ONLINE_TRADE_TERMS_BYTES], char digest[65]) {
	FILE *f = fopen("shared/online/trade-vector.json", "rb");
	if (!f) return false;
	char text[1400] = { 0 };
	size_t got = fread(text, 1, sizeof text - 1, f);
	fclose(f);
	const char *encoded = strstr(text, "\"terms_hex\": \""), *expected = strstr(text, "\"digest\": \"");
	if (!got || !encoded || !expected) return false;
	encoded += strlen("\"terms_hex\": \""); expected += strlen("\"digest\": \"");
	if (strcspn(encoded, "\"") != ONLINE_TRADE_TERMS_BYTES * 2 || strcspn(expected, "\"") != 64) return false;
	for (unsigned i = 0; i < ONLINE_TRADE_TERMS_BYTES; ++i) {
		unsigned byte;
		if (sscanf(encoded + 2 * i, "%2x", &byte) != 1) return false;
		bytes[i] = (uint8_t)byte;
	}
	memcpy(digest, expected, 64); digest[64] = 0;
	return true;
}

static void terms_vector(void) {
	uint8_t bytes[ONLINE_TRADE_TERMS_BYTES], encoded[ONLINE_TRADE_TERMS_BYTES], digest[32];
	char expected[65], actual[65];
	bool got = vector_read(bytes, expected);
	CHECK(got);
	if (!got) return;
	OnlineTradeTerms terms;
	CHECK(online_trade_terms_decode(&terms, bytes, sizeof bytes));
	CHECK(terms.side[0].revision == 0x01020304 && terms.side[1].revision == 0x05060708);
	CHECK(terms.side[0].chip == ((3 << 9) | 1) && terms.side[1].chip == ((26 << 9) | 0x123));
	CHECK(terms.side[0].rules == ONLINE_TRADE_RULE_BN5 && terms.side[1].rules == 0);
	CHECK(online_trade_terms_encode(&terms, encoded) && !memcmp(bytes, encoded, sizeof bytes));
	CHECK(online_trade_terms_digest(&terms, digest)); hex(digest, 32, actual);
	CHECK(!strcmp(expected, actual));
	for (unsigned i = 0; i < sizeof bytes; ++i) {
		uint8_t mutation[ONLINE_TRADE_TERMS_BYTES];
		memcpy(mutation, bytes, sizeof bytes); mutation[i] ^= 1;
		OnlineTradeTerms changed;
		if (online_trade_terms_decode(&changed, mutation, sizeof mutation)) {
			CHECK(online_trade_terms_digest(&changed, digest)); hex(digest, 32, actual);
			CHECK(strcmp(expected, actual));
		}
	}
	for (unsigned i = 0; i < sizeof bytes; ++i) CHECK(!online_trade_terms_decode(&terms, bytes, i));
	CHECK(online_trade_terms_decode(&terms, bytes, sizeof bytes));
	terms.side[0].rules = 128; CHECK(!online_trade_terms_valid(&terms));
	terms.side[0].rules = 2; CHECK(!online_trade_terms_valid(&terms));
	terms.side[0].rules = 1; terms.side[0].revision = UINT32_MAX; CHECK(!online_trade_terms_valid(&terms));
}

int test_online_trade_terms(void) {
	hash_vectors(); terms_vector();
	if (!failures) puts("SHA-256 vectors and immutable trade terms agree with the shared server fixture");
	return failures;
}

#ifdef CW_TRADE_TERMS_TEST_MAIN
int main(void) { return test_online_trade_terms() ? 1 : 0; }
#endif
