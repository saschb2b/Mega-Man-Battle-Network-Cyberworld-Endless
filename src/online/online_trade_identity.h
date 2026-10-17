/* A run receives its identity only beside a verified complete checkpoint.
 * These isolated storage APIs do not queue or authorize ordinary saves. */
#ifndef CW_ONLINE_TRADE_IDENTITY_H
#define CW_ONLINE_TRADE_IDENTITY_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	TRADE_IDENTITY_CLEAR, TRADE_IDENTITY_UNBOUND, TRADE_IDENTITY_BLOCKED, TRADE_IDENTITY_UNSUPPORTED
} OnlineTradeIdentityState;

/* Explicit NEW GAME only: persist random ID + seed before ordinary
 * checkpoint writes. Removes an abandoned receipt only after the intent
 * is durable. False may leave an intent, so recover before any play. */
bool online_trade_identity_intent(const char *root, uint32_t seed, const uint8_t random_id[16]);
/* With writers frozen, queue drained and no new write failures: certify
 * actual checkpoint bytes, bind the intent, then install the receipt. */
bool online_trade_identity_bind(const char *root, const uint8_t expected_content[32]);
/* Enroll an existing complete legacy checkpoint using its stable local
 * seed mapping, never a fresh random identity. Writers must be frozen. */
bool online_trade_identity_migrate(const char *root, uint32_t seed, const uint8_t expected_content[32]);
/* Before all save readers/writers/import recovery. An unbound intent
 * cannot certify a possibly partial first checkpoint after restart. */
OnlineTradeIdentityState online_trade_identity_recover(const char *root);
/* Explicit discard/restart of an unbound NEW GAME only. Removes the
 * incomplete run, preserves profile and history, and deletes intent last. */
bool online_trade_identity_discard(const char *root);
/* Read the current format's checksummed active run seed, without globals. */
bool online_trade_identity_seed(const char *root, uint32_t *seed);

#endif
