/* ROM-free checks of Discord's presence (src/discord/): the pipe's frames
 * written and read back, split across reads and too long; the handshake's
 * and SET_ACTIVITY's JSON, its text escaped, a clear, a line under two
 * bytes left out, a payload that does not fit refused; and what friends
 * read at each place of the game; their failure count. */
#ifndef CW_TEST_DISCORD_H
#define CW_TEST_DISCORD_H

int test_discord(void);

#endif
