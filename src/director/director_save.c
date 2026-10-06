/* Checkpoints and CONTINUE: the run's state saved beside the emulator's,
 * what the director knows of the act kept with it, the quit's save, the
 * arena door's, home's, and the layer rebuilt and the game restored. Each
 * waits for MegaMan free as BN6 has him (director_hold.c, issue #23): a
 * state written while BN6 held him came back held after a CONTINUE. */
#include "director_save.h"

#include <stdio.h>
#include <string.h>

#include "bn6_fields.h"
#include "boss.h"
#include "cinema.h"
#include "darkchips.h"
#include "debug.h"
#include "director.h"
#include "director_board.h"
#include "director_duel.h"
#include "director_folder.h"
#include "director_hold.h"
#include "director_home.h"
#include "director_layer.h"
#include "director_state.h"
#include "gamecall.h"
#include "layer_make.h"
#include "netmap.h"
#include "save.h"
#include "save_blob.h"
#include "souls.h"

/* (LAYER_MAKE: layer_make.h, beside its hash) */
#define LAYER_MAKE_MAGIC 0x434D4B31u   /* "CMK1" */
#define LAYER_AREA_MAGIC 0x43415231u   /* "CAR1" */
#define LAYER_SEEN_MAGIC 0x43534E31u   /* "CSN1" */

/* What the director knows of the act a checkpoint is in, saved beside its
 * state: the viruses deleted before the act began, its frames, the act
 * whose Net Dealer has spoken, and the act whose guardian a bystander has
 * named. A CONTINUE takes it back, where an act continued from a save had
 * no count on its AREA CLEAR card and its next Net Dealer greeted MegaMan
 * as new (a playtester's, both), and the guardian a rumor had named on
 * the act's first layer was "one we've never faced" again (session 63).
 * (A note from before `heard` reads it as 0.) */
#define ACT_NOTE_MAGIC 0x41435432u   /* "ACT2" */
typedef struct { uint32_t seed; int32_t act, viruses, frames, dealer, unknown, heard, where; } ActNote;
enum { SAVED_START = 1, SAVED_HERE, SAVED_DATA, SAVED_LEFT, SAVED_DOOR, SAVED_HOME, SAVED_HOME_HERE };   /* the checkpoint's place (ActNote's where; 0 a note from before) */
static bool suspending;   /* the checkpoint being saved is a quit's, where MegaMan stands */
static ActNote act_note;
static bool act_note_ok;

static void act_note_save(void) {
	/* (an act already continued without one has no whole count to keep) */
	ActNote an = { run.seed, (run.depth - 1) / 3, D.act_viruses, D.act_frames, D.dealer_act, D.act_resumed, D.heard_act,
		suspending ? SAVED_LEFT : D.town ? (D.checkpoint_here ? SAVED_HOME_HERE : SAVED_HOME) : D.checkpoint_door ? SAVED_DOOR : D.checkpoint_here ? SAVED_HERE :
		D.checkpoint_data ? SAVED_DATA : SAVED_START };
	save_write_blob("run.act", ACT_NOTE_MAGIC, &an, sizeof an);
}

/* on CONTINUE, before the layer is built: its dealer's greeting reads it */
static void act_note_read(void) {
	act_note_ok = save_read_blob_upto("run.act", ACT_NOTE_MAGIC, &act_note, sizeof act_note) && act_note.seed == run.seed &&
		act_note.act == (run.depth - 1) / 3;
	if (act_note_ok) {
		D.dealer_act = act_note.dealer;
		D.heard_act = act_note.heard;
	}
}

/* CONTINUE's word on where the run goes on from: a playtester's two chips,
 * bought after the layer's checkpoint, were gone with no word why
 * (session 63) */
static void resume_note(bool restarted) {
	static const char *const from[] = { NULL, "From the layer's start", "From where you saved", "From the Guardian Data", "From where you left off",
		"From the arena's door", "From Lan's HP", "From where you saved" };
	/* (a layer made otherwise, by this build or without the ROM that drew
	 * it, starts again: "From where you left off" over the layer's start
	 * misled a playtester, session 67) */
	if (restarted) cinema_note(from[SAVED_START], 240);
	else if (act_note_ok && act_note.where >= SAVED_START && act_note.where <= SAVED_HOME_HERE) cinema_note(from[act_note.where], 240);
}

/* ... and after it, where building it began the act afresh */
static void act_note_apply(void) {
	D.act_resumed = !act_note_ok || act_note.unknown;
	if (act_note_ok) { D.act_viruses = act_note.viruses; D.act_frames = act_note.frames; }
	if (emu_debug_on()) fprintf(stderr, "act note: %s, act %d, viruses from %d, %d frames, dealer %d\n", act_note_ok ? "taken back" : "none",
		act_note.act, act_note.viruses, act_note.frames, act_note.dealer);
}

void save_checkpoint(void) {
	char path[600];
	save_state_path(path, sizeof path);
	library_from_game();
	programs_from_game();
	save_run();
	emu_save_state(path);
	int make = LAYER_MAKE;
	save_write_blob("run.make", LAYER_MAKE_MAGIC, &make, sizeof make);
	save_write_blob("run.area", LAYER_AREA_MAGIC, &D.layer_tiles, sizeof D.layer_tiles);
	/* the map's panels seen so far, beside the state they go with */
	save_write_blob("run.seen", LAYER_SEEN_MAGIC, D.seen, sizeof D.seen);
	folder_made_save();
	act_note_save();
	dark_save();
	souls_save();
	off_board_save();
}

bool director_can_suspend(void) {
	/* (not through a guest battle: BN6 stands on the map while it runs, and
	 * a quit there skipped the battle) */
	return D.active && !D.town && !D.gameover && !guest_active() && !D.warping && boss_idle() && !D.challenge && hold_save_ok();
}

const char *director_saved_where(void) { return D.saved_at ? D.saved_at : "Run saved at the layer's start"; }

void director_save_here(void) {
	/* (the PET's Save: in the town, before the run's first layer, there is
	 * no run to save yet; home after an act saves where Lan stands, as the
	 * HP's arrival saved: a playtester's purchase and request in town were
	 * not saved, and "Saves begin on layer 1" told him so wrongly, session
	 * 70) */
	if (!D.active) return;
	if (D.town && D.home) { D.home_save_due = true; return; }
	if (D.town) { cinema_note("Saves begin on layer 1", 150); return; }
	D.checkpoint = true;
	D.checkpoint_here = true;
}

/* The arena's door (the owner's: bigger RPGs save before a boss): the run
 * saved on the last frame MegaMan is free before the guardian's staging,
 * so a quit in the fight goes on from the door with the HP he walked in
 * with, where it went back to the layer's start before (a playtester's
 * free retry, session 64) */
void arena_door_save(void) {
	if (!hold_save_ok() || D.warping) return;
	D.checkpoint_door = true;
	save_checkpoint();
	D.checkpoint_door = false;
	D.saved_at = "Run saved at the arena's door";
	cinema_note("Run saved", 150);
}

static void home_save(void) {
	save_checkpoint();
	D.saved_at = "Run saved at home";
	cinema_note("Run saved", 150);
}

/* Home's saves once MegaMan (or Lan) is free: its checkpoint in Lan's HP,
 * once its words are said, and the PET's Save */
void home_saves(void) {
	if (!hold_save_ok()) return;
	if (D.home_save_due) {
		D.home_save_due = false;
		D.checkpoint_here = true;
		home_save();
		D.checkpoint_here = false;
	}
	if (D.home && home_in_hp() && D.intro_said && !D.home_saved) {
		D.home_saved = true;
		home_save();
	}
}

#define CHECKPOINT_AFTER 60   /* frames after a layer is entered */

/* The checkpoint due, saved once MegaMan is free (never with a chat box
 * open: a state would keep it, and the talk slot's text is not in a state;
 * nor while the arrival still holds him: the jack-in and the warp pad keep
 * him for about 90 frames, and the release is not in the state) */
void checkpoint_update(void) {
	if (boss_take_checkpoint()) D.checkpoint = D.checkpoint_data = true;
	if (!D.checkpoint || D.frame < CHECKPOINT_AFTER || !hold_save_ok()) return;
	D.checkpoint = false;
	save_checkpoint();
	/* (said: a playtester who plays in short sessions asked where it is
	 * safe to stop) */
	D.saved_at = D.checkpoint_here ? "Run saved where you saved it" : D.checkpoint_data ? "Run saved at the Guardian Data" :
		"Run saved at the layer's start";
	D.checkpoint_data = D.checkpoint_here = false;
	cinema_note("Run saved", 150);
	if (D.nest_cleared) { D.nest_cleared = false; profile.nest_clears++; profile_save(); }
}

bool director_suspend(void) {
	if (!director_can_suspend()) return false;
	suspending = true;
	save_checkpoint();
	suspending = false;
	return true;
}

/* Whether the layer just rebuilt is the one the checkpoint saved: made by
 * this build's generation, and drawn in the same area. A layer another
 * game's area drew, continued without that game's ROM, is laid out as
 * BN6's own, and the state's place and picks would land on another layer:
 * it starts again (owner's call, 4 October 2026). */
static bool same_layer(void) {
	int make = 0, drawn = -1;
	bool same = save_read_blob("run.make", LAYER_MAKE_MAGIC, &make, sizeof make) && make == LAYER_MAKE;
	if (save_read_blob("run.area", LAYER_AREA_MAGIC, &drawn, sizeof drawn) && drawn != D.layer_tiles) same = false;
	if (!same && emu_debug_on()) fprintf(stderr, "resume: the layer was made by another build or drawn in area %d, now %d: from its start\n",
		drawn, D.layer_tiles);
	return same;
}

/* MegaMan into the layer's map again, the game reloading its NPCs and
 * tiles from this build's tables, which a state does not hold: where he
 * stands (a build that lays the layer out otherwise may have no floor
 * there any more: then its arrival), or at its start */
void layer_reenter(bool at_start) {
	int x = bn6_player_x(), y = bn6_player_y(), cx, cy;
	if (at_start || !netmap_panel(x, y, &cx, &cy) || cx < 0 || cy < 0 || cx >= MAP_W || cy >= MAP_H ||
	    (layer.cell[cy][cx] != C_PATH && layer.cell[cy][cx] != C_PROPPED))
		x = D.start_x, y = D.start_y;
	emu_warp(D.group, D.number, x, y, 4);
}

/* choices made before the checkpoint stay made */
static void resume_choices(void) {
	for (int i = 0; i < D.objs.nchoices; ++i)
		if (flag_get(D.objs.choice[i].flag)) {
			D.chosen |= 1u << i;
			if (D.objs.choice[i].type == OBJ_NPC) D.heard_act = D.layer_act;
		}
}

/* CONTINUE at home: the town entered where Lan stood, its words said */
static bool resume_home(void) {
	spins_sync();
	run.fragments = key_item(SCRIPTS_SECRET_DATA);
	own_folder_chips();
	star_folder_pack();
	official_sync(true);
	home_resume();
	hold_watch_arm();
	act_note_apply();
	D.beat[0] = 0;
	resume_note(false);
	return true;
}

bool director_resume(void) {
	drop_events();
	off_board_load();
	forget_heard();
	D.saved_at = "Run saved where you continued";
	folder_made_load();
	act_note_read();
	dark_load(run.seed);
	souls_load(run.seed);
	/* the layer's tables live in the ROM copy, which a state does not hold */
	if (!new_layer(false)) return false;
	/* (saved at home: the town's too, the layer behind its port) */
	bool home = act_note_ok && (act_note.where == SAVED_HOME || act_note.where == SAVED_HOME_HERE) && home_rebuild();
	char path[600];
	save_state_path(path, sizeof path);
	bool same = same_layer();
	if (emu_load_state(path)) {
		lock_run();
		if (home) return resume_home();
		spins_sync();
		/* (the ScrtData the state holds, as the game counts them) */
		run.fragments = key_item(SCRIPTS_SECRET_DATA);
		/* the shops' data in RAM is the saved one: this layer's again,
		 * what was bought before the save still bought (a CONTINUE had
		 * restocked both shops); another build's layer, afresh */
		layer_objs_shops(&D.objs, same);
		own_folder_chips();   /* (a run saved with the folder's chips unmarked) */
		star_folder_pack();
		official_sync(true);
		/* L starts over, its first words and then the rest at a second L:
		 * the state kept that L had spoken, and a playtester's first L
		 * after a CONTINUE said the rest at once, a dozen boxes with no
		 * word of where they were (session 64) */
		flag_clear(LAYER_TOLD_FLAG);
		if (!same) {
			/* another build's layer: its flags and Mystery Data picks
			 * forgotten, and in from the start */
			/* (a gift taken stays taken, and a guardian beaten stays
			 * beaten, his Guardian Data shown or taken, his exit open:
			 * they are the run's, not the layer's; a playtester's run
			 * saved beside ElementMan's open exit would have met him
			 * again, his Guardian Data twice) */
			for (int f = MAPSLOT_MD_FLAG; f <= LAYER_HEAL_TOLD_FLAG; ++f)
				if (f != LAYER_GIFT_FLAG && (f < LAYER_BOSS_GONE_FLAG || f > LAYER_EXIT_OPEN_FLAG)) flag_clear(f);
			flag_clear(LAYER_VAULT_FLAG);
			flag_clear(LAYER_OFFICIAL_FLAG);
			flag_clear(LAYER_DUEL_CALLED_FLAG);
			flag_clear(LAYER_RUSH_TOLD_FLAG);
			flag_clear(LAYER_PCODE_FLAG);
			flag_clear(LAYER_NUMBER_SEALED_FLAG);
			for (int i = 0; i <= LAYER_GIFT_FLAG - MAPSLOT_MD_FLAG; ++i) { uint8_t z[2] = { 0, 0 }; emu_write(BN6_MYSTERY_PICKS + 2 * (uint32_t)i, z, 2); }
			emu_write32(BN6_PLAYER_X, (uint32_t)D.start_x << 16);
			emu_write32(BN6_PLAYER_Y, (uint32_t)D.start_y << 16);
			/* (and the music stopped: the state's song can be a copy of
			 * another game's in the free space this build's layer fills
			 * with something else, and the driver ran its bytes as tracks
			 * every frame, the screen black: a run BN5's ACDC Area drew,
			 * continued without BN5's ROM; the map plays its own) */
			emu_write32(BN6_MUSIC_STATUS, emu_read32(BN6_MUSIC_STATUS) | BN6_MUSIC_STOPPED);
		}
		/* a state saved while the jack-in still held MegaMan (runs from
		 * before the checkpoint waited for him): the game's own release
		 * would never come, so it is done here */
		if (emu_read8(BN6_DIALOGUE_LOCK) || !flag_get(BN6_FLAG_PLAYER_CAN_MOVE)) {
			emu_write8(BN6_DIALOGUE_LOCK, 0);
			flag_set(BN6_FLAG_PLAYER_CAN_MOVE);
			flag_clear(BN6_FLAG_DIALOGUE_1719);
		}
		boss_resume();
		/* the map as far as it was seen (none for another build's layer) */
		if (same && !save_read_blob("run.seen", LAYER_SEEN_MAGIC, D.seen, sizeof D.seen)) memset(D.seen, 0, sizeof D.seen);
		resume_duel();
		resume_choices();
		layer_reenter(false);
		/* (and watched: a state saved while BN6 held him, issue #23) */
		hold_watch_arm();
		/* where they are, again; the arrival's words were said before */
		begin_area(false);
		act_note_apply();
		D.beat[0] = 0;
		resume_note(!same);
		return true;
	}
	/* no state (a run from before the game engine): enter the layer fresh */
	emu_warp(D.group, D.number, D.start_x, D.start_y, 4);
	D.checkpoint = true;
	return true;
}
