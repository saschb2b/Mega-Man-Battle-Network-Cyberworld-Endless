/* Network ownership stays off the game thread. Snapshots are copies. */
#ifndef CW_ONLINE_TRANSPORT_H
#define CW_ONLINE_TRANSPORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_types.h"

/* URL must be ws(s)://host[:port]/presence. No fallback. enabled remains
 * true after a refusal, until end, so the online square cannot
 * accidentally become a normal run.
 *
 * A dropped link (issue #153, docs/ONLINE.md, A dropped link): once the
 * server has welcomed this game, a link quiet for ONLINE_QUIET_MS shows as
 * `quiet` in the snapshot; one dead (quiet past its limit, closed or
 * broken) without the server's ERROR is tried again with backoff, a
 * resume's HELLO (protocol v10's hello_resume: the place the server holds
 * taken back), `reconnecting` meanwhile, for ONLINE_RECONNECT_MS from the
 * moment it went quiet or dropped; then the snapshot's error. A resume is
 * done at its first SNAPSHOT after the WELCOME (`resumes` counts them). A
 * refusal (ERROR) is never tried again. end says goodbye (BYE) where the
 * link is up, so the server frees the place at once. */
#define ONLINE_QUIET_MS 2500
/* Thirty seconds: long enough for a phone's Wi-Fi to come back or hand over
 * to its mobile data (a few seconds to tens), short enough that a player
 * who is not coming back is not waited on; the server holds the place ten
 * seconds longer (server/cyberworld_server/holds.py). */
#define ONLINE_RECONNECT_MS 30000
bool online_transport_start(const char *url, const char *name, const uint8_t identity[ONLINE_IDENTITY_BYTES]);
void online_transport_poll(int32_t x, int32_t y, int32_t z, uint8_t facing);
void online_transport_snapshot(OnlineSnapshot *out);
bool online_transport_profile(const OnlineProfile *profile);
bool online_transport_inspect(uint32_t player_id);
/* A phrase (kind 0) or an emote (1), an emote aimed at player `target`
 * (0 none; ONLINE_EMOTE_AIMED's alone) */
bool online_transport_social(uint8_t kind, uint8_t value, uint32_t target);
bool online_transport_channel(uint16_t channel);
/* Trade Central's and the Arena's messages (TRADE, OFFER, ARENA and its relayed
 * LOADOUT, INPUT and HASH; shared/online/README.md): sent within the
 * worker's next pass (about 2 ms), not a frame's queue; false where the
 * outbox is full or the kind is none of them. */
bool online_transport_send(const uint8_t *data, size_t size);
/* ... and theirs come in (TRADE_STATE, BOARD, ARENA_STATE and the opponent's
 * LOADOUT, INPUT and HASH): the next one's bytes into `out`, its size;
 * 0 none. */
size_t online_transport_receive(uint8_t *out, size_t cap);
void online_transport_end(void);
/* The ROMs this game has besides BN6's, as HELLO tells the server
 * (ONLINE_ROMS_BN5 where BN5 Team Colonel's is read): the common ground's
 * one side, each peer's roms the other */
uint8_t online_transport_roms(void);

#endif
