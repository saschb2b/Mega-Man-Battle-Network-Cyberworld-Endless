/* Network settings and keyboard trust boundaries, without a ROM or socket. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "network_address.h"
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
	/* (L's favourites as the strip keeps them, phrases among them (items 12-19), and its hint said: issue #178) */
	before.hinted = 1;
	const uint8_t favourites[4] = { 1 + 12, 1 + 1, 1 + 19, 1 + 0 };
	memcpy(before.favourites, favourites, sizeof favourites);
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
	CHECK(after.count == 2 && after.consent == 0);
	CHECK(!strcmp(after.servers[0], NETWORK_DEFAULT_URL));
	CHECK(!strcmp(after.servers[1], "wss://other.example/presence"));
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

static void saved_servers(const char *path, const char *settings) {
	FILE *file = fopen(path, "w");
	CHECK(file != NULL);
	if (!file) return;
	fputs("identity=0123456789abcdef0123456789abcdef\n", file);
	fputs(settings, file);
	CHECK(!fclose(file));
}

static void server_migration(void) {
	char directory[] = "/tmp/cyberworld-servers-XXXXXX", path[128];
	CHECK(mkdtemp(directory) != NULL);
	snprintf(path, sizeof path, "%s/network.ini", directory);
	NetworkPrefs prefs, roundtrip;
	/* Four custom slots from the old menu survive adding the fixed entry. */
	saved_servers(path, "selected=2\nconsent=9\n"
		"server=wss://one.example/presence\nserver=wss://two.example/presence\n"
		"server=wss://three.example/presence\nserver=wss://four.example/presence\n");
	CHECK(network_prefs_read(directory, &prefs));
	CHECK(prefs.count == 5 && prefs.selected == 3 && prefs.consent == 18);
	CHECK(!strcmp(prefs.servers[0], NETWORK_DEFAULT_URL));
	CHECK(!strcmp(prefs.servers[1], "wss://one.example/presence"));
	CHECK(!strcmp(prefs.servers[2], "wss://two.example/presence"));
	CHECK(!strcmp(prefs.servers[3], "wss://three.example/presence"));
	CHECK(!strcmp(prefs.servers[4], "wss://four.example/presence"));
	CHECK(network_prefs_write(directory, &prefs));
	CHECK(network_prefs_read(directory, &roundtrip));
	CHECK(!memcmp(&prefs, &roundtrip, sizeof prefs));
	/* Consent follows an existing official entry when it moves to first. */
	saved_servers(path, "selected=1\nconsent=2\nserver=wss://one.example/presence\n"
		"server=" NETWORK_DEFAULT_URL "\nserver=wss://two.example/presence\n");
	CHECK(network_prefs_read(directory, &prefs));
	CHECK(prefs.count == 3 && prefs.selected == 0 && prefs.consent == 1);
	CHECK(!strcmp(prefs.servers[1], "wss://one.example/presence"));
	CHECK(!strcmp(prefs.servers[2], "wss://two.example/presence"));
	/* Duplicates collapse; an approved URL stays approved, not its index. */
	saved_servers(path, "selected=2\nconsent=4\nserver=wss://one.example/presence\n"
		"server=" NETWORK_DEFAULT_URL "\nserver=wss://one.example/presence\n");
	CHECK(network_prefs_read(directory, &prefs));
	CHECK(prefs.count == 2 && prefs.selected == 1 && prefs.consent == 2);
	/* Rejecting a damaged slot does not discard another URL's own consent. */
	saved_servers(path, "selected=1\nconsent=3\nserver=invalid\nserver=wss://two.example/presence\n");
	CHECK(network_prefs_read(directory, &prefs));
	CHECK(prefs.count == 2 && prefs.selected == 1 && prefs.consent == 2);
	CHECK(!strcmp(prefs.servers[1], "wss://two.example/presence"));
	/* Neither the editor nor a caller can persist a replacement official URL. */
	snprintf(prefs.servers[0], NETWORK_URL, "wss://replacement.example/presence");
	CHECK(!network_prefs_write(directory, &prefs));
	CHECK(network_prefs_read(directory, &roundtrip));
	CHECK(!strcmp(roundtrip.servers[0], NETWORK_DEFAULT_URL));
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

/* what a player types at the keeper, and the URL it reaches (network_address.h) */
static bool reaches(const char *typed, const char *url) {
	char out[NETWORK_URL];
	bool ok = network_address_url(typed, out, sizeof out) && !strcmp(out, url);
	if (!ok) fprintf(stderr, "  \"%s\" reached \"%s\", not \"%s\"\n", typed, network_address_url(typed, out, sizeof out) ? out : "nothing", url);
	return ok;
}

static void addresses(void) {
	/* a Square started bare on this network: plain WebSocket, the server's port */
	CHECK(reaches("192.168.1.20", "ws://192.168.1.20:8765/presence"));
	CHECK(reaches("10.0.0.5", "ws://10.0.0.5:8765/presence"));
	CHECK(reaches("172.16.4.1", "ws://172.16.4.1:8765/presence"));
	CHECK(reaches("172.31.255.1", "ws://172.31.255.1:8765/presence"));
	CHECK(reaches("100.101.102.103", "ws://100.101.102.103:8765/presence"));
	CHECK(reaches("127.0.0.1", "ws://127.0.0.1:8765/presence"));
	CHECK(reaches("localhost", "ws://localhost:8765/presence"));
	CHECK(reaches("Lans-PC", "ws://Lans-PC:8765/presence"));
	CHECK(reaches("lans-pc.local", "ws://lans-pc.local:8765/presence"));
	CHECK(reaches("NAS.LAN", "ws://NAS.LAN:8765/presence"));
	CHECK(reaches("pc.home.arpa", "ws://pc.home.arpa:8765/presence"));
	CHECK(reaches("[::1]", "ws://[::1]:8765/presence"));
	CHECK(reaches("[fd12:3456::1]", "ws://[fd12:3456::1]:8765/presence"));
	CHECK(reaches("[fe80::1]", "ws://[fe80::1]:8765/presence"));
	/* a port, a path or a scheme typed is kept */
	CHECK(reaches("192.168.1.20:9000", "ws://192.168.1.20:9000/presence"));
	CHECK(reaches("192.168.1.20/presence", "ws://192.168.1.20:8765/presence"));
	CHECK(reaches("[fd12::1]:9000", "ws://[fd12::1]:9000/presence"));
	CHECK(reaches("wss://nas.local", "wss://nas.local/presence"));
	CHECK(reaches("ws://192.168.1.20:8765", "ws://192.168.1.20:8765/presence"));
	CHECK(reaches("ws://example.com:8765/presence", "ws://example.com:8765/presence"));
	/* anywhere else: TLS on its own port, as before */
	CHECK(reaches("square.example.com", "wss://square.example.com/presence"));
	CHECK(reaches("square.example.com:8443", "wss://square.example.com:8443/presence"));
	CHECK(reaches("172.32.0.1", "wss://172.32.0.1/presence"));
	CHECK(reaches("100.128.0.1", "wss://100.128.0.1/presence"));
	CHECK(reaches("8.8.8.8", "wss://8.8.8.8/presence"));
	CHECK(reaches("[2001:db8::1]", "wss://[2001:db8::1]/presence"));
	CHECK(reaches("local.example.com", "wss://local.example.com/presence"));
	CHECK(reaches("cyberworld.saschb2b.com", NETWORK_DEFAULT_URL));
	CHECK(!network_address_local("192.168.1"));
	CHECK(!network_address_local("192.168.1.256"));
	CHECK(!network_address_local("[fc0::1]"));
	/* no address at all */
	char out[NETWORK_URL];
	CHECK(!network_address_url("", out, sizeof out));
	CHECK(!network_address_url("host/other", out, sizeof out));
	CHECK(!network_address_url("user@host", out, sizeof out));
	CHECK(!network_address_url("192.168.1.20", out, 20));
}

int main(void) {
	identity_keys();
	preferences();
	server_migration();
	editor();
	addresses();
	if (!failures) puts("network: identity isolation, consent persistence and text input passed");
	return failures ? 1 : 0;
}
