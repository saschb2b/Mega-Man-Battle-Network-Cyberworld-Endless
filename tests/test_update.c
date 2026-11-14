/* test_update.h. */
#include "test_update.h"

#include <stdio.h>
#include <string.h>

#include "update_manifest.h"
#include "update_sha256.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* (tools/update_manifest.py's own output, for a folder with an AppImage of
 * "abc" and an empty .nro) */
static const char manifest[] =
	"{\n"
	" \"version\": \"0.22.0\",\n"
	" \"notes\": \"https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/tag/v0.22.0\",\n"
	" \"files\": {\n"
	"  \"appimage\": {\n"
	"   \"url\": \"https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/download/v0.22.0/cyberworld-endless-x86_64.AppImage\",\n"
	"   \"size\": 3,\n"
	"   \"sha256\": \"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\"\n"
	"  },\n"
	"  \"switch\": {\n"
	"   \"url\": \"https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/download/v0.22.0/cyberworld-endless.nro\",\n"
	"   \"size\": 0,\n"
	"   \"sha256\": \"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\"\n"
	"  }\n"
	" }\n"
	"}\n";

static void test_manifest(void) {
	UpdateRelease r;
	CHECK(update_manifest_read(manifest, NULL, &r) && !strcmp(r.version, "0.22.0") && !r.url[0], "the version alone: %s", r.version);
	CHECK(update_manifest_read(manifest, "appimage", &r) && r.size == 3 &&
		!strcmp(r.sha256, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") &&
		!strcmp(r.url, "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/download/v0.22.0/cyberworld-endless-x86_64.AppImage"),
		"the AppImage's entry: %s %lld %s", r.url, r.size, r.sha256);
	/* (an empty file is no update; a key the release lacks is none either) */
	CHECK(!update_manifest_read(manifest, "switch", &r) && !r.url[0], "an empty file read as an update");
	CHECK(!update_manifest_read(manifest, "windows-setup", &r) && !r.url[0], "a file the release lacks read");
	/* (a key that is a value's part elsewhere, and no version) */
	CHECK(!update_manifest_read("{\"files\": {}}", NULL, &r), "a manifest without a version read");
	CHECK(!update_manifest_read("{\"version\": \"0.22.0-1-gabc\"}", NULL, &r), "a build between releases read as a release");
	CHECK(!update_manifest_read("{\"version\": \"0.22.0\", \"files\": {\"appimage\": {\"url\": \"x\", \"size\": 3", "appimage", &r),
		"a cut-off entry read");
	CHECK(!update_manifest_read(NULL, NULL, &r), "nothing read as a manifest");
}

static void test_versions(void) {
	CHECK(update_version_release("0.22.0") && update_version_release("10.0.12"), "a release's version refused");
	CHECK(!update_version_release("dev") && !update_version_release("0.21.1-4-gabc123") && !update_version_release("0.22") &&
		!update_version_release("0.22.0.1") && !update_version_release(".1.2") && !update_version_release("1..2") && !update_version_release(""),
		"a build between releases taken as a release");
	CHECK(update_version_cmp("0.22.0", "0.21.1") > 0 && update_version_cmp("0.21.1", "0.22.0") < 0, "0.22.0 against 0.21.1");
	CHECK(update_version_cmp("0.10.0", "0.9.9") > 0, "0.10.0 read as older than 0.9.9");
	CHECK(update_version_cmp("1.0.0", "0.99.99") > 0 && update_version_cmp("0.21.1", "0.21.1") == 0, "majors, or the same");
	CHECK(update_version_cmp("0.21.2", "0.21.1") > 0, "a patch read as no newer");
}

static void test_sha256(void) {
	static const struct { const char *in, *hex; } known[] = {
		{ "", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" },
		{ "abc", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" },
		{ "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1" },
	};
	char hex[65];
	Sha256 s;
	for (size_t i = 0; i < sizeof known / sizeof *known; ++i) {
		sha256_begin(&s);
		sha256_feed(&s, known[i].in, strlen(known[i].in));
		sha256_end(&s, hex);
		CHECK(!strcmp(hex, known[i].hex), "sha256(%s) = %s", known[i].in, hex);
	}
	/* (a million a's, fed in pieces that straddle the blocks) */
	static char a[1001];
	memset(a, 'a', sizeof a);
	sha256_begin(&s);
	for (int i = 0; i < 1000; ++i) sha256_feed(&s, a, i % 2 ? 999 : 1001);
	sha256_end(&s, hex);
	CHECK(!strcmp(hex, "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"), "sha256(a * 1000000) = %s", hex);
}

int test_update(void) {
	failures = 0;
	test_manifest();
	test_versions();
	test_sha256();
	return failures;
}
