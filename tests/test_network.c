/* Network settings and keyboard trust boundaries, without a ROM or socket. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "network_prefs.h"
#include "text_input.h"

static int failures;
#define CHECK(condition) do { if (!(condition)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); ++failures; } } while (0)

static void identity_keys(void) {
	uint8_t secret[16], a[16], b[16];
	for (unsigned i = 0; i < sizeof secret; ++i) secret[i] = (uint8_t)i;
	/* Independently computed with Python hmac/hashlib, not the game hash. */
	static const uint8_t expected[] = { 0xc1,0x58,0xc8,0xd8,0x34,0xdf,0xd3,0x0c,0x32,0x74,0xe2,0xe4,0xfe,0x5b,0x16,0x9b };
	CHECK(network_server_identity(secret, "wss://example.com/presence", a));
	CHECK(!memcmp(a, expected, sizeof a));
	CHECK(network_server_identity(secret, "wss://EXAMPLE.COM.:00443/presence", b));
	CHECK(!memcmp(a, b, sizeof a));
	CHECK(network_server_identity(secret, "wss://other.example/presence", b));
	CHECK(memcmp(a, b, sizeof a));
	CHECK(network_server_identity(secret, "ws://example.com:443/presence", b));
	CHECK(memcmp(a, b, sizeof a));
	CHECK(network_server_identity(secret, "wss://example.com:444/presence", b));
	CHECK(memcmp(a, b, sizeof a));
	CHECK(!network_server_identity(secret, "wss://example.com:0/presence", b));
	CHECK(!network_server_identity(secret, "wss://example.com:65536/presence", b));
	CHECK(!network_server_identity(secret, "wss://example.com:443x/presence", b));
	CHECK(network_identity(a));
	CHECK(network_identity(b));
	CHECK(memcmp(a, b, sizeof a));
}

static void preferences(void) {
	char directory[] = "/tmp/cyberworld-network-XXXXXX", path[128];
	CHECK(mkdtemp(directory) != NULL);
	NetworkPrefs before, after;
	CHECK(!network_prefs_read(directory, &before));
	CHECK(before.count == 1 && !before.consent && before.selected == 0);
	CHECK(!strcmp(before.servers[0], NETWORK_DEFAULT_URL));
	CHECK(network_identity(before.identity));
	snprintf(before.name, sizeof before.name, "Navi_42");
	snprintf(before.servers[1], NETWORK_URL, "ws://localhost:8777/presence");
	before.count = 2; before.selected = 1; before.consent = 2;
	CHECK(network_prefs_write(directory, &before));
	CHECK(network_prefs_read(directory, &after));
	CHECK(!memcmp(&before, &after, sizeof before));
	snprintf(path, sizeof path, "%s/network.ini", directory);
	struct stat st;
	CHECK(!stat(path, &st) && (st.st_mode & 0777) == 0600);
	/* An invalid slot must not transfer its consent to the next valid URL. */
	FILE *file = fopen(path, "w");
	CHECK(file != NULL);
	if (file) {
		fputs("name=Wrong\nName\nidentity=00000000000000000000000000000000\nconsent=1\nserver=evil\nserver=wss://other.example/presence\n", file);
		fclose(file);
	}
	CHECK(!network_prefs_read(directory, &after));
	CHECK(after.count == 1 && after.consent == 0);
	CHECK(!network_name_valid(" leading") && !network_name_valid("trailing "));
	CHECK(!network_name_valid("<script>") && !network_name_valid("12345678901234567"));
	CHECK(!network_url_valid("wss://user@host/presence"));
	CHECK(!network_url_valid("wss://host/presence?token=secret"));
	CHECK(!network_url_valid("wss://host/presence\nname=Changed"));
	before.selected = before.count;
	CHECK(!network_prefs_write(directory, &before));
	remove(path);
	rmdir(directory);
}

static void key(SDL_Scancode code, SDL_Keymod modifiers) {
	SDL_Event event = { .type = SDL_KEYDOWN };
	event.key.keysym.scancode = code;
	event.key.keysym.mod = modifiers;
	CHECK(text_input_event(&event));
}

static void editor(void) {
	CHECK(!SDL_Init(SDL_INIT_VIDEO));
	char field[8] = "Navi";
	text_input_begin(field, sizeof field);
	/* Mapped game keys are swallowed while editing; text arrives separately. */
	key(SDL_SCANCODE_Z, KMOD_NONE);
	CHECK(!strcmp(field, "Navi") && text_input_result() == TEXT_INPUT_NONE);
	SDL_Event event = { .type = SDL_TEXTINPUT };
	snprintf(event.text.text, sizeof event.text.text, " 123456789");
	CHECK(text_input_event(&event));
	CHECK(!strcmp(field, "Navi 12"));
	key(SDL_SCANCODE_A, KMOD_CTRL);
	text_input_add("A\nB\tC\x01");
	CHECK(!strcmp(field, "ABC"));
	key(SDL_SCANCODE_BACKSPACE, KMOD_NONE);
	CHECK(!strcmp(field, "AB"));
	key(SDL_SCANCODE_RETURN, KMOD_NONE);
	CHECK(text_input_result() == TEXT_INPUT_DONE);
	CHECK(text_input_result() == TEXT_INPUT_NONE);
	key(SDL_SCANCODE_ESCAPE, KMOD_NONE);
	CHECK(text_input_result() == TEXT_INPUT_CANCEL);
	text_input_end();
	CHECK(!text_input_event(&event));
	SDL_Quit();
}

int main(void) {
	identity_keys();
	preferences();
	editor();
	if (!failures) puts("network: identity isolation, consent persistence and text input passed");
	return failures ? 1 : 0;
}
