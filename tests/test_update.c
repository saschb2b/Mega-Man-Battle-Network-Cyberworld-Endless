/* test_update.h. */
#include "test_update.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "update_manifest.h"
#include "update_sha256.h"
#include "update_swap.h"

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

/* (a file holding `text`) */
static void file_put(const char *path, const char *text) {
	FILE *f = fopen(path, "wb");
	if (f) {
		fputs(text, f);
		fclose(f);
	}
}

/* (`path` holds `text` alone; "" where it is not there) */
static bool file_is(const char *path, const char *text) {
	char got[64] = "";
	FILE *f = fopen(path, "rb");
	if (!f) return !text[0];
	size_t n = fread(got, 1, sizeof got - 1, f);
	fclose(f);
	got[n] = 0;
	return text[0] && !strcmp(got, text);
}

/* The Switch's and the 3DS's swap (update_swap.c) on a made-up SD card:
 * which argv[0] names a file the game may replace, a swap that puts the
 * new one in its name and keeps nothing else, one with nothing staged
 * that leaves the old one alone, and a start's tidying */
static void test_swap(void) {
	char dir[600], arg[700], self[700], staged[720], aside[720], other[720];
	if (!getcwd(dir, 400)) return;
	snprintf(dir + strlen(dir), sizeof dir - strlen(dir), "/build/host/test-swap-%d", (int)getpid());
	mkdir("build", 0755);
	mkdir("build/host", 0755);
	mkdir(dir, 0755);
	snprintf(arg, sizeof arg, "%s/cyberworld-endless.nro", dir);
	file_put(arg, "old");
	CHECK(update_swap_self(arg, ".nro", self, sizeof self) && !strcmp(self, arg), "swap: %s is the game's file", arg);
	CHECK(update_swap_self(arg, ".NRO", self, sizeof self), "swap: its extension in any case");
	CHECK(!update_swap_self(arg, ".3dsx", self, sizeof self) && !self[0], "swap: a .nro is no .3dsx");
	CHECK(!update_swap_self("3dslink:/cyberworld-endless.3dsx", ".3dsx", self, sizeof self), "swap: 3dslink's copy stays");
	CHECK(!update_swap_self("cyberworld-endless.nro", ".nro", self, sizeof self), "swap: a path from no root");
	CHECK(!update_swap_self(NULL, ".nro", self, sizeof self), "swap: no argv[0]");
	CHECK(!update_swap_self(arg, ".nro", self, strlen(arg) + 4), "swap: no room for the names beside it");
	snprintf(other, sizeof other, "%s/gone.nro", dir);
	CHECK(!update_swap_self(other, ".nro", self, sizeof self), "swap: a file that is not there");
	CHECK(!update_swap_self(dir, "", self, sizeof self), "swap: a folder is no file");
	update_swap_self(arg, ".nro", self, sizeof self);
	update_swap_staged(self, staged, sizeof staged);
	snprintf(aside, sizeof aside, "%s.old", self);
	/* (nothing staged: the old one stays) */
	CHECK(!update_swap(self) && file_is(self, "old") && file_is(aside, ""), "swap: with nothing staged, the old one stays");
	/* (staged: the new one in its name, nothing beside it) */
	file_put(staged, "new");
	file_put(aside, "left");
	CHECK(update_swap(self), "swap: the new one swapped in");
	CHECK(file_is(self, "new") && file_is(staged, "") && file_is(aside, ""), "swap: the new one in its name, nothing left beside it");
	/* (a start's tidying: a half download and an old one left aside go,
	 * but an old one with none in its name stays for the player) */
	file_put(staged, "half");
	file_put(aside, "old");
	update_swap_tidy(self);
	CHECK(file_is(staged, "") && file_is(aside, "") && file_is(self, "new"), "swap: a start tidies what an earlier one left");
	remove(self);
	file_put(aside, "old");
	update_swap_tidy(self);
	CHECK(file_is(aside, "old"), "swap: the old one stays where none is in its name");
	remove(aside);
	rmdir(dir);
}

int test_update(void) {
	failures = 0;
	test_manifest();
	test_versions();
	test_sha256();
	test_swap();
	return failures;
}
