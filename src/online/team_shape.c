/* team_shape.h */
#include "team_shape.h"

#include "bn6.h"
#include "hook.h"

static struct {
	bool party;
	const unsigned *swapped;
} shape[HOOK_CORES];

static bool in(int slot) { return slot >= 0 && slot < HOOK_CORES; }

void team_shape_party(int slot, bool party, const unsigned *swapped) {
	if (!in(slot)) return;
	shape[slot].party = party;
	shape[slot].swapped = party ? swapped : NULL;
}

bool team_shape_is_party_of(int slot) { return in(slot) && shape[slot].party; }
bool team_shape_is_party(void) { return team_shape_is_party_of(hook_slot()); }

unsigned team_partners_of(int slot) { return team_shape_is_party_of(slot) ? TEAM_PARTNERS : TEAM_ARENA; }
unsigned team_partners(void) { return team_partners_of(hook_slot()); }

unsigned team_partner_side(unsigned p) { return team_shape_is_party() ? 0 : p & 1u; }

static uint32_t slot_of(int slot, unsigned p) {
	if (!team_shape_is_party_of(slot)) return BN6_BATTLE_PLAYERS + (p & 1u) * BN6_BATTLE_PLAYER_STEP + BN6_SIDE_PARTNER;
	/* (side 0's slot p + 1, or its first while the field has traded them: party_field.c's aim) */
	unsigned k = p + 1, traded = shape[slot].swapped ? *shape[slot].swapped : 0;
	return BN6_BATTLE_PLAYERS + (traded == k ? 0 : k) * BN6_SIDE_PARTNER;
}

uint32_t team_partner_slot(unsigned p) { return slot_of(hook_slot(), p); }
