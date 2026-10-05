/* The run on the game's own engine: layers, exits, loot and encounters. */
#ifndef CW_DIRECTOR_H
#define CW_DIRECTOR_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "guest.h"

/* Builds the run's current layer in its area's map and warps MegaMan in. */
bool director_start_layer(void);
/* A new run: Lan in the town, the first layer built for the port's jack-in. */
bool director_start_run(void);
/* The core's ROM copy as the game's own NEW GAME needs it (before its boot):
 * the chip records the All * helper changed for a run before, put back. */
void director_before_boot(void);
/* Lan is still in the town. */
bool director_in_town(void);
/* A run is under way on the layers (it has been saved). */
bool director_on_layer(void);
/* The guardian whose battle runs (his navi index, guardians.h), 0 in any
 * other battle or none. */
int director_guardian_battle(void);
/* The layer's Chip or BugFrag Trader's kind (TraderKind, trader.h), -1
 * for none */
int director_trader_kind(void);
/* The guardian the layer's exit waits on (his navi index), 0 where it
 * opens without one or he is beaten (the second screen's next step) */
int director_guardian_waiting(void);
/* The NaviCust's board as it stands (the second screen, issue #76): why
 * RUN would bug, in MegaMan's words, or NULL; whether program variant `v`
 * (program * 4 + its colour's) fits its free cells as they stand */
const char *director_board_bug(void);
bool director_board_fits(int v);
/* What the drawing reads of the game, taken after each frame, before the
 * update: on a 3DS the next frame runs on another core while this one is
 * drawn, and a read of the game then waits for it. */
void director_see(void);
/* The layer's map over the picture while SELECT is held (drawing only). */
void director_draw_map(void);
/* The layer's Mystery Data counters over the picture, a few seconds after L. */
void director_draw_counts(void);
/* The NaviCust's bug named over the PET as its RUN leaves it, a few seconds. */
void director_draw_bug_note(void);
/* The layer's map in w x h from (x, y), for the second screen (issue #73),
 * whose frame names the layer; nothing off a layer. */
void director_draw_layer_map(int x, int y, int w, int h);
/* The run's folder as the game holds it: BN6_FOLDER_ENTRIES entries,
 * chip | code << 9 (zeros where the game's memory is not there) */
void director_folder_now(uint16_t *folder);
/* The pack's chips as the game holds them: up to `most` of them (chip |
 * code << 9) and their counts, by chip and its record's codes; how many */
int director_pack_now(uint16_t *entry, uint8_t *count, int most);
/* MegaMan's HP as the top screen shows it: the Navi's on the map, his
 * battle object's in a battle, the guest's in an older net's battle (true
 * then). */
bool director_megaman_hp(int *hp, int *max);
/* The rival's duel: its clock against ProtoMan's time, in battle (docs/RIVAL.md). */
void director_draw_duel(void);
/* CircusMan's tent: the panel BN6 lights under MegaMan's feet, where it
 * drops, marked over his sprite while a step can still clear it. */
void director_draw_tent(void);
/* Quitting on a layer's map, free to move: the run is saved there
 * (CONTINUE resumes it where MegaMan stood); false when it could not be. */
bool director_can_suspend(void);
bool director_suspend(void);
/* The PET's Save: a checkpoint where MegaMan stands, once he is free to
 * move on the map ("Run saved"). */
void director_save_here(void);
/* A Navi on the net has named the act's guardian this session. */
bool director_guardian_heard(void);
/* Where the run was last saved on this layer, for the quit prompt: its
 * start, or its Guardian Data. */
const char *director_saved_where(void);
/* Lan (or MegaMan) is on the map the run put him on: the picture can show. */
bool director_arrived(void);
/* What a player sees, in words, one fact a line (remote play). */
void director_describe(FILE *f);
/* Dev: MegaMan put at world (x, y) facing `face` (0-7, else unchanged). */
void director_dev_place(int x, int y, int face);
/* A test's start at home (--scene home, docs/HOME.md): the layer just
 * built behind the town's port, Lan by it, as an act's exit takes him. */
void director_dev_home(void);
/* Dev (the battle step): the layer's next random battle, at the first
 * moment MegaMan is free on its map; the guest's on a layer whose battles
 * are an older net's. */
void director_dev_battle(void);
/* A dev step's: every panel of the layer seen, for the map (--dev mapall). */
void director_dev_reveal(void);
/* The pad's keys on their way to the game: on the map L is MegaMan's
 * word on where they are. */
uint32_t director_keys(uint32_t keys);
/* Rebuilds the saved run's layer and restores the game at its checkpoint. */
bool director_resume(void);
/* A battle on the guest core has ended (guest.h): its result into the run. */
void director_guest_done(const GuestResult *r);
/* The layer's guardian, one of another game's Navis (guardians.h:
 * guardian_older), begins his battle on the guest core at `version`, his
 * HP held to the act's band (docs/BOSSES.md, BN5's Navis); false where it
 * cannot (its end then comes through director_guest_done, to boss.c) */
bool director_guest_guardian(int navi, int version);
/* Once a frame, after the game's frame: exits and encounters. */
void director_update(void);
/* Where the test autopilot heads (grid panel): the guardian, to talk to
 * (*talk), or the exit pad. */
bool director_goal_panel(int *x, int *y, bool *talk);
/* Dev tools: MegaMan walks a layer's map (not a battle, a menu, a warp). */
bool director_on_map(void);
/* ... on to the next layer, or the next guardian's (arriving in the room
 * before its arena), or to grid panel (x, y) of this one. */
bool director_dev_next_layer(void);
bool director_dev_guardian(void);
bool director_dev_warp_cell(int x, int y);
/* The director lets go of the game (the real world's tour). */
void director_stop(void);
/* Test hook (--net-biome): every layer in this biome. */
extern int director_debug_biome;
/* Test hook (--net-biome xN): every layer in another game's net area N
 * (docs/MULTIROM.md), as NET_AREAS + N, where its ROM is beside BN6's. */
extern int director_debug_area;
/* --net-biome's value: a biome, or xN another game's area N. */
void director_net_biome_arg(const char *v);
/* --talk: chats to open at given frames (director_dev.c) */
extern const char *director_dev_talks;

#endif
