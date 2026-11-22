/* The shared dive's party core (issue #589; docs/online/shared-dive.md, As
 * built): one BN6 core of its own on which every game in a party runs the
 * same guardian's battle, frame for frame, as the Arena's games run their
 * pair. It is made alike on every game: the Arena's boot to the net
 * (core_boot.h, the same state everywhere, kept for the next), the run's
 * ROM copy with its hooks' originals put back (emu_rom_clean: the same
 * layer's on every game of the party), the battle's record at
 * FREE_RECORDS, the lead's MegaMan (seat 0's loadout) in BN6's own RAM, the
 * installs' hooks (xinstall_log.h) and the party's field (party_field.h:
 * seats 1..n-1 MegaMen beside him on side 0, each one's keys and HP his
 * own). The encounter check forced into StartBattle with the record, the
 * core runs to the battle's first Custom screen with no keys: the ready.
 * From there each frame takes every seat's keys. */
#ifndef CW_PARTY_CORE_H
#define CW_PARTY_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_snapshot.h"
#include "party_field.h"

#define PARTY_RECORD_BYTES 0x80   /* a battle's record: BattleSettings, its list at +0x20 (encounter.c's) */
#define PARTY_W 240
#define PARTY_H 160

typedef struct {
	uint8_t record[PARTY_RECORD_BYTES];   /* the battle's record as at FREE_RECORDS (its list's pointer there) */
	int foe_from, foe_to;                 /* the guardian's own HP, and the party's (the spawn gives him that) */
	unsigned n;                           /* its MegaMen, 1 to PARTY_MOST */
	OnlineLoadout seat[PARTY_MOST];       /* each seat's MegaMan, as it came */
	bool quiet;                           /* no field log */
	bool scripted;                        /* the partners' keys the spike's script (--dev partycore alone) */
	bool spread;                          /* the enemies' aim passed over the party in turn (party_field_aim) */
	bool own;                             /* each partner his own NaviStats, hand and Custom screen (team_shape.h): his
	                                         seat's keys his battle keys or his screen's picks (arena_team_custom.h) */
	int end;                              /* the enemies set to 1 HP at this iteration (the dev's partyend), 0 never */
	int del;                              /* ... and the MegaMen of the seats of this mask deleted then (the dev's partydel), 0 none */
} PartyBattle;

/* FNV-1a over bytes: a field's inputs told apart in the log (party_play.c) */
uint64_t party_core_bytes_hash(const void *data, size_t size);

struct mCore;
typedef struct PartyCore PartyCore;

/* The boot to the net, kept for every party battle after: run in slices
 * of `frames` (a progress bar's), 1 while it runs on, 0 once kept, -1 where
 * it failed (the reason on stderr) */
int party_core_boot(const uint8_t *rom, size_t size, unsigned frames);

/* The battle's core from the kept boot: `run_rom` the run's ROM copy
 * (emu_rom_clean's, EMU_ROM_SIZE bytes), run to the battle's first Custom
 * screen; NULL, the reason in `why`, where it could not be */
PartyCore *party_core_open(const uint8_t *rom, size_t size, const uint8_t *run_rom, const PartyBattle *b, char *why, size_t wn);
void party_core_close(PartyCore *pc);

/* One frame with each seat's keys (keys[0] the lead's pad) */
void party_core_frame(PartyCore *pc, const uint16_t keys[PARTY_MOST]);
/* Its frames since the ready */
uint32_t party_core_frames(const PartyCore *pc);
/* The core's hash (the sound's fields left out, the field's state in) */
uint64_t party_core_hash(PartyCore *pc);

/* A snapshot for a rollback: the core and the field's state */
typedef struct PartyCoreSnap PartyCoreSnap;
PartyCoreSnap *party_core_snap_new(PartyCore *pc);
void party_core_snap_free(PartyCoreSnap *s);
bool party_core_save(PartyCore *pc, PartyCoreSnap *s);
bool party_core_load(PartyCore *pc, const PartyCoreSnap *s);

/* The battle over (the core back on the map, or its results shown):
 * whether it is, and then whether side 0 won */
bool party_core_over(PartyCore *pc, bool *won);
/* ... and back on the map, its results gone */
bool party_core_back(PartyCore *pc);
/* A seat's MegaMan off the field (his game gone): his HP 0, so BN6
 * deletes him as it deletes a partner, and the battle goes on */
void party_core_leave(PartyCore *pc, unsigned seat);
/* The battle's phase (BN6_BATTLE_PHASE: BN6_PHASE_CUSTOM the Custom screen) */
unsigned party_core_phase(PartyCore *pc);
/* Each seat's MegaMan's HP as the field last saw it (0 deleted) */
unsigned party_core_hp(const PartyCore *pc, unsigned seat);
/* The battle over, BN6's results to come on this game's own pad (the
 * hashes stopped): seat's HP and max HP where they show the lead's, his
 * battle object and the Navi's HP, so that BN6's results show a
 * partner his own (issue #757); seat 0 nothing */
void party_core_results_as(PartyCore *pc, unsigned seat);
/* A seat's HP and max HP as the field means them: his going in before
 * the battle's first iteration (his object holds his lead's till then),
 * else his object's; 0 0 none */
void party_core_seat_hp(PartyCore *pc, unsigned seat, unsigned *hp, unsigned *max);
/* ... the Navi's HP there, as BN6's results show it */
unsigned party_core_navi_hp(PartyCore *pc);
/* The guardian's HP and max (side 1's first actor), 0 none */
unsigned party_core_foe_hp(PartyCore *pc);
/* The reward BN6's results picked as the battle ended (BN6_BATTLE_REWARD:
 * a row's entry, chip_entry.h's REWARD_*) */
uint16_t party_core_reward(PartyCore *pc);

/* The picture (PARTY_W x PARTY_H, mGBA's colour order), and the core's
 * sound into the game's (true) or dropped */
const uint32_t *party_core_video(const PartyCore *pc);
void party_core_sound(PartyCore *pc, bool heard);

/* The field's own GBA (between frames: a partner's screen is copied from it) */
struct mCore *party_core_gba(PartyCore *pc);
/* A partner's (seat 1-3) deck as the field dealt it, the order drawn
 * (0 before it is dealt, or no partners' own); whether his turn is open */
unsigned party_core_deck(PartyCore *pc, unsigned seat, uint16_t out[BN6_DECK_SIZE]);
bool party_core_picking(PartyCore *pc, unsigned seat);
/* The turn's clock: frames since its first OK (0 none) */
unsigned party_core_clock(PartyCore *pc);


#endif
