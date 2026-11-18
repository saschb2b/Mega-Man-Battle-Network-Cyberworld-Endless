/* Discord's Rich Presence (docs/engine/discord.md): the player's Discord
 * shows their friends that they play, and how far the run has come
 * ("Layer 14, Sky Area", "Fighting ElecMan"), with the time played and a
 * button to the game's page. It talks to the Discord app on the same
 * machine over its local pipe (discord_ipc.c), the way Discord's own RPC
 * libraries do: no SDK, no account, nothing sent anywhere but to that app,
 * which shows it as the player's Discord settings allow.
 *
 * The desktop builds have it (Windows, Linux, macOS; not the handhelds,
 * the consoles, the phones or the browser), at a player's start (never
 * headless, a test's or a developer's). On by default: settings.ini's
 * discord line, turned on Options' Game page. Where no Discord runs, it
 * looks again every DISCORD_RETRY_MS, and nothing shows.
 *
 * This part keeps the setting and the connection and sends what it is
 * handed; discord_game.c reads the game for it. */
#ifndef CW_DISCORD_H
#define CW_DISCORD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* The game's application on Discord's developer portal: its name is what
 * a friend reads ("Playing Cyberworld Endless"). settings.ini's
 * discord_app takes another (a fork's own). Empty: the build has none,
 * and shows nothing (no Options line either). */
#define DISCORD_APP_ID ""
/* The picture beside it (the game's icon on its site: Discord takes an
 * address for the art as it takes a key uploaded to the portal), and the
 * one button a friend can press */
#define DISCORD_IMAGE "https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/assets/icons/512.png"
#define DISCORD_URL "https://saschb2b.github.io/Mega-Man-Battle-Network-Cyberworld-Endless/"
#define DISCORD_BUTTON "Get Cyberworld Endless"

/* Discord's limits: a line's most bytes (and least: 2), a button's label's */
#define DISCORD_TEXT 128
#define DISCORD_LABEL 32
/* The pace: an activity sent at most once in DISCORD_GAP_MS (Discord
 * takes five in 20 seconds), and Discord looked for again after
 * DISCORD_RETRY_MS where it was not there; its READY waited for
 * DISCORD_HELLO_MS */
#define DISCORD_GAP_MS 5000
#define DISCORD_RETRY_MS 20000
#define DISCORD_HELLO_MS 10000

/* What the player's Discord shows: the first line, the second, the
 * picture's tooltip (empty: none), and the time played counted from
 * `start` (Unix seconds, 0 none) */
typedef struct {
	char details[DISCORD_TEXT];
	char state[DISCORD_TEXT];
	char large_text[DISCORD_TEXT];
	int64_t start;
} DiscordActivity;

/* The session: whether it may show (`plain`: a player's start, not
 * headless), and settings.ini's discord and discord_app lines read */
void discord_start(bool plain);
/* Whether this build and start can show it: Options' line shows then */
bool discord_here(void);
/* ... and whether the player has it on */
bool discord_on(void);
/* The player's choice, kept in settings.ini: off clears what Discord
 * shows and lets it go at once */
void discord_set_on(bool on);
/* Whether it is here and on: the game's state is worth reading */
static inline bool discord_live(void) { return discord_here() && discord_on(); }

/* What to show now; sent when it differs from what was, at the pace */
void discord_show(const DiscordActivity *a);
/* A frame: Discord looked for, its answers read, the activity sent */
void discord_frame(void);
/* The game's end: what Discord shows cleared, the pipe closed */
void discord_quit(void);

/* The wire, numbers only (discord_activity.c), ROM-free for the tests:
 * a frame's header (opcode and length, little-endian) and its JSON
 * written into `out` (n bytes), its length, 0 where it does not fit ... */
enum { DISCORD_OP_HANDSHAKE, DISCORD_OP_FRAME, DISCORD_OP_CLOSE, DISCORD_OP_PING, DISCORD_OP_PONG };
#define DISCORD_HEADER 8
#define DISCORD_FRAME_MAX 4096
size_t discord_frame_put(uint8_t *out, size_t n, int op, const char *json, size_t len);
/* ... a whole frame at the start of `in` (len bytes): its opcode, its
 * JSON's place and length; the frame's bytes, 0 where it has not all
 * come, -1 where it cannot be one (too long) */
long discord_frame_take(const uint8_t *in, size_t len, int *op, const char **json, size_t *json_len);
/* ... the handshake's JSON, the SET_ACTIVITY command's (`a` NULL clears
 * it), each into `out`; its length, 0 where it does not fit */
size_t discord_hello_json(char *out, size_t n, const char *app);
size_t discord_activity_json(char *out, size_t n, const DiscordActivity *a, long pid, unsigned nonce);
/* ... and whether two activities show the same */
bool discord_same(const DiscordActivity *a, const DiscordActivity *b);

#endif
