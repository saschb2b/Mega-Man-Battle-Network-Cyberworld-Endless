/* The director: the run's structure around the game.
 *
 * A layer is generated (net_gen), built into its area's map (netmap), given
 * its exit pad and Mystery Data (mapslot, npc), and entered through the
 * game's warp. The game then runs everything MegaMan does; the director only
 * watches his position to take him to the next layer, and keeps the next
 * random battle's enemies in step with the depth.
 *
 * This file is the frame's course (director_update) and its state (D,
 * director_state.h); its parts are director_*.c, what they say *_words.c. */
#include "director.h"

#include <stdio.h>

#include "bn6_fields.h"
#include "boss.h"
#include "briefing_words.h"
#include "cinema.h"
#include "darkbn6.h"
#include "data.h"
#include "debug.h"
#include "director_board.h"
#include "director_dark.h"
#include "director_dev.h"
#include "director_duel.h"
#include "director_folder.h"
#include "director_guest.h"
#include "director_keys.h"
#include "director_layer.h"
#include "director_save.h"
#include "director_state.h"
#include "director_way.h"
#include "duel_words.h"
#include "encounter.h"
#include "events.h"
#include "game.h"
#include "gamecall.h"
#include "guardians.h"
#include "guest_words.h"
#include "lesson_words.h"
#include "loot.h"
#include "meta.h"
#include "netmap.h"
#include "pet_text.h"
#include "rivals.h"
#include "save.h"
#include "story_words.h"
#include "talk.h"
#include "town.h"

static void win_run(void);
static void gate_and_rush_words(void);

#define REROLL_FRAMES 300  /* the next battle's enemies are re-rolled this often */
DirectorState D;

#define AREA_CARD_AT 45   /* frames on the map after arriving */

/* The run is won where MegaMan stands: the short net's last guardian
 * deleted, only the exit ahead (no reminders then: a playtester was told
 * to place a program on the walk to it). */
bool run_won_here(void) {
	return run.side_kind == LAYER_NORMAL && run.mode == RUN_SHORT && run_short_last(run.depth) && boss_done();
}

#define CHECKPOINT_AFTER  60     /* frames after a layer is entered */
#define ASTRAY_FRAMES     90     /* on another map this long: warp back */

int main_mode(void) { return emu_read8(emu_read32(BN6_TOOLKIT)); }
/* walking the net: the game mode on its map sub-mode (not a battle or menu) */
bool on_map(void) { return main_mode() == BN6_MODE_GAME && emu_read8(BN6_GAMESTATE) == BN6_SUB_MAP; }

int key_item(int id) { return emu_read8(emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS) + (uint32_t)id); }

bool director_in_town(void) { return D.active && D.town; }

bool director_on_layer(void) { return D.active && !D.town; }

/* The guardian's course, and its door's save the frame he steps in (not
 * before the arrival's card: a CONTINUE at the door showed the act's card
 * cut short by the guardian's) */
static void guardian_update(void) {
	if (D.area_card && boss_idle()) return;
	boss_update();
	if (boss_take_door()) arena_door_save();
}

bool director_arrived(void) {
	if (!D.active || !on_map()) return false;
	int group = emu_read8(BN6_MAP_GROUP), number = emu_read8(BN6_MAP_NUMBER);
	return D.town ? group == town_info()->group && number == town_info()->number : group == D.group && number == D.number;
}

/* In the town: nothing to watch but the jack-in, whose arrival on the
 * layer's map starts the run as a layer's warp does. */
static void town_update(void) {
	map_label();
	arrow_update();
	if (on_map()) unwedge();
	int group = emu_read8(BN6_MAP_GROUP), number = emu_read8(BN6_MAP_NUMBER);
	if (group == town_info()->group && number == town_info()->number) D.town_seen = true;
	talk_update();
	/* Lan and MegaMan's words (Dad's call, the first time), once Lan is
	 * out and the map has settled */
	if (D.town_seen && on_map() && !D.intro_said && emu_read8(BN6_WARP_PENDING) == 0 && ++D.town_frames > 40 &&
		talk_script(town_info()->talk_archive, town_info()->intro)) {
		D.intro_said = true;
		if (!profile.seen_intro) { profile.seen_intro = true; profile_save(); }
	}
	bool arrived = D.town_seen && on_map() && emu_read8(BN6_WARP_PENDING) == 0 &&
		emu_read8(BN6_MAP_GROUP) == D.group && emu_read8(BN6_MAP_NUMBER) == D.number;
	if (!arrived) return;
	D.town = false;
	D.frame = 0;
	D.checkpoint = true;
	lock_run();
	mapslot_music_forget_town();
}

/* ---- dev tools (src/dev/devtools.c) ---- */

/* (not through a guest battle, which BN6 waits out on its map: the touch
 * controls' D-pad keeps to four ways in its battle, as in BN6's) */
bool director_on_map(void) { return D.active && on_map() && !guest_active(); }

void director_stop(void) { D.active = false; }

/* Into the Undernet or the Secret Area: MegaMan jacks out as on a warp pad,
 * and into the side layer built meanwhile. */
static void enter_side_layer(void) {
	note_folder_codes();
	if (!new_layer(true)) return;
	D.warping = true;
	D.checkpoint = true;
	emu_warp_out();
}

/* A Yes in a layer's choice: a challenge battle, or into a side layer. */
static bool act_on_choices(void) {
	if (!D.choices_due || emu_read8(BN6_CHATBOX)) return false;   /* once the chat box has closed */
	for (int i = 0; i < D.objs.nchoices; ++i) {
		if (!(D.choices_due & (1u << i))) continue;
		D.choices_due &= ~(1u << i);
		if (D.chosen & (1u << i)) continue;
		D.chosen |= 1u << i;
		switch (D.objs.choice[i].type) {
		case OBJ_CHALLENGE: {
			Encounter e = D.server_rolled ? D.server_enc : make_encounter(run.depth, run.biome, ENC_CHALLENGE);
			set_encounter(&e, true);
			D.challenge = true;
			return true;
		}
		case OBJ_DUEL:
			set_encounter(&D.duel_enc, true);
			/* (its hits, and the netbattle's ProtoMan held to the act's
			 * band as he spawns: docs/RIVAL.md) */
			emu_battle_watch(D.duel_cap);
			D.challenge = true;
			D.duel = true;
			D.duel_hit = false;
			D.duel_time = 0;
			D.duel_call_due = false;
			flag_set(LAYER_DUEL_CALLED_FLAG);
			return true;
		case OBJ_UNDERNET:
		case OBJ_SECRET_GATE:
			run.side_kind = D.objs.choice[i].type == OBJ_UNDERNET ? LAYER_UNDERNET : LAYER_SECRET;
			enter_side_layer();
			return true;
		case OBJ_NPC:
			/* a bystander named the act's guardian (and no more to do) */
			D.heard_act = D.layer_act;
			break;
		case OBJ_NAVI_GATE: {
			/* his SP, as a challenge: the boss theme, and his chip after */
			Encounter e = make_boss(run.depth, run.biome, D.objs.gate_navi);
			e.foes[0].version = 2;
			set_encounter(&e, true);
			D.challenge = true;
			D.gate_fight = true;
			return true;
		}
		default:
			break;
		}
	}
	return false;
}

/* MegaMan stepped on the exit pad: the game plays its warp (jack out, fade,
 * jack in) to warp 1. While it jacks out, the next layer is built and warp 1
 * pointed at its start; nothing else happens until MegaMan has arrived. */
static bool follow_exit_warp(void) {
	int pending = emu_read8(BN6_WARP_PENDING);
	if (D.warping) {
		bool arrived = pending == 0 && on_map() &&
			emu_read8(BN6_MAP_GROUP) == D.group && emu_read8(BN6_MAP_NUMBER) == D.number;
		if (arrived) D.warping = false;
		return !arrived;
	}
	if (pending != 1 || emu_read8(BN6_WARP_INDEX) != 1) return false;
	/* the short net's Nest fallen: the run is won (on threat 10, its first
	 * guardian's exit leads down to the second's, docs/META.md) */
	if (boss_beaten() && run.biome == BIOME_NEST && run_short_last(run.depth)) { win_run(); return true; }
	if (boss_beaten() && run.biome == BIOME_NEST && run_short_nest(run.depth)) run.boss_order[BIOME_NEST] = (uint8_t)run_nest_second();
	if (boss_beaten()) clear_card();
	/* past the endless net's Nest guardian: the net rebuilds (the next
	 * arrival says so; counted with the next checkpoint, which a CONTINUE
	 * cannot undo) */
	if (boss_beaten() && run.biome == BIOME_NEST && !run_short_nest(run.depth)) D.nest_cleared = true;
	/* the Guardian Data's second way on, where it was taken (docs/META.md,
	 * routes): the next act in that area, under its guardian */
	if (run.side_kind == LAYER_NORMAL && is_boss_depth(run.depth) && flag_get(LAYER_ROUTE_FLAG)) {
		int act = (run.depth % CYCLE_LAYERS) / 3, navi = 0, b = run_route_alt(act, &navi);
		if (b >= 0) {
			run.biome_order[act] = (uint8_t)b;
			run.boss_order[b] = (uint8_t)navi;
		}
		flag_clear(LAYER_ROUTE_FLAG);
	}
	/* ... or its dark way, into the Undernet */
	if (run.side_kind == LAYER_NORMAL && is_boss_depth(run.depth) && flag_get(LAYER_ROUTE_DARK_FLAG)) {
		int act = (run.depth % CYCLE_LAYERS) / 3, alt_navi = 0, navi = 0;
		int b = run_route_alt(act, &alt_navi) >= 0 ? run_route_dark(act, alt_navi, &navi) : -1;
		if (b >= 0) {
			run.biome_order[act] = (uint8_t)b;
			run.boss_order[b] = (uint8_t)navi;
		}
		flag_clear(LAYER_ROUTE_DARK_FLAG);
	}
	/* a side layer's exit leads one area deeper too */
	run.depth++;
	run.side_kind = LAYER_NORMAL;
	note_folder_codes();
	if (!new_layer(true)) return false;
	D.warping = true;
	D.checkpoint = true;
	return true;
}

void end_run(void) {
	/* what the summary tells: where, and by whom */
	const char *area = guardian_area_in_text(run.biome, run.side_kind);
	if (D.lost_to) snprintf(title_cause, sizeof title_cause, "by %s in %s", guardian(D.lost_to)->name, area);
	else if (D.lost_duel) snprintf(title_cause, sizeof title_cause, "in ProtoMan's duel");
	else snprintf(title_cause, sizeof title_cause, "in %s", area);
	/* (the guardian's first battle, lost: its battle data is what the run
	 * leaves for the next briefing) */
	const Rival *rv = D.lost_to ? rival(D.lost_to) : NULL;
	snprintf(title_learned, sizeof title_learned, "%s", rv && rv->megaman_won + rv->navi_won == 1 ? guardian(D.lost_to)->name : "");
	/* (a first run is no record to beat) */
	title_new_best = profile.runs > 0 && run.depth > profile.best_depth;
	title_won = false;
	runlog_run_end();
	library_from_game();
	programs_from_game();
	meta_run_over(false);
	profile_record_run();
	save_delete();
	run.active = false;
	D.active = false;
	title_summary = true;
	scene_set(&scene_title);
}

/* The short net won: its Nest's guardian fell and MegaMan stepped on its
 * exit. The run ends on the title's summary of a win, and what it opened
 * for the next (docs/META.md). */
static void win_run(void) {
	snprintf(title_cause, sizeof title_cause, "on layer %d", run.depth);
	title_learned[0] = 0;
	title_new_best = run.depth > profile.best_depth;
	title_won = true;
	runlog_run_end();
	library_from_game();
	programs_from_game();
	meta_run_over(true);   /* (before the clear counts: it names what the win opened) */
	profile.nest_clears++;
	profile_record_run();
	save_delete();
	run.active = false;
	D.active = false;
	title_summary = true;
	scene_set(&scene_title);
}

/* The exit pad shut while a guardian stands: its warp-off flag, written
 * as the guardian's state changes and after the game enters a map (bn6f
 * EnterMap clears the map's flags, 0x1640-0x16FF: EV_MAP_ENTER). */
static void exit_flag(bool entered) {
	bool shut = !boss_exit_open();
	if (!entered && shut == D.exit_shut) return;
	D.exit_shut = shut;
	if (shut) flag_set(BN6_FLAG_WARP_OFF + 1);
	else flag_clear(BN6_FLAG_WARP_OFF + 1);
}

#define DUEL_GRACE 60   /* frames of a duel's fight before a hit counts against its no-hit rung */

/* What the hooks saw in the frames since the last update (events.h). */
static void take_events(void) {
	HookEvent ev[32];
	int n = emu_hook_events(ev, 32);
	bool on_layer = D.active && !D.town;
	for (int i = 0; i < n; ++i)
		switch (ev[i].kind) {
		case EV_BATTLE_START: D.battle_record = ev[i].r[0]; emu_encounter_started(ev[i].r[0]); darkbn6_battle_begins(); break;
		/* (a DarkChip's dark power, or its base chip, in a BN6 battle: its
		 * price after it, docs/META.md) */
		case EV_DARK_RAN: case EV_DARK_BASE: darkbn6_event(&ev[i]); break;
		/* (not in the fight's first second: a Chumpy rammed a playtester at
		 * 0:00.70, as BATTLE START left the screen, and the no-hit duel was
		 * lost before he could act, session 65) */
		case EV_MEGAMAN_HIT: if (emu_read32(BN6_BATTLE_TIMER) >= DUEL_GRACE) D.duel_hit = true; break;
		case EV_MAP_ENTER: if (on_layer) exit_flag(true); break;
		case EV_CHOICE:
			/* (still set: one the layer's setup cleared was a story's) */
			for (int k = 0; on_layer && k < D.objs.nchoices; ++k)
				if (D.objs.choice[k].flag == (int)ev[i].r[0] && flag_get(D.objs.choice[k].flag)) D.choices_due |= 1u << k;
			break;
		case EV_ITEM_GIVEN: if (on_layer) item_given((int)ev[i].r[0]); break;
		case EV_GUEST_BATTLE: if (on_layer) guest_begin(); break;
		}
}

/* The events queued before the run (the boot's), dropped, and a last run's
 * battle's dark power (a GAME OVER never came back to the map to pay it). */
void drop_events(void) {
	HookEvent ev[32];
	while (emu_hook_events(ev, 32) == 32) {}
	darkbn6_battle_begins();
	dark_base_told = false;
}

static void words_due(void) {
	gate_and_rush_words();
	pack_watch();
	pack_words();
	guest_words();
	dark_flame_watch();
	/* (BN6's own DarkChips: in the Pack, their power as the BugFrags held
	 * say, the price of a battle their dark power ran in) */
	dark_pack();
	darkbn6_sync();
	dark6_after_battle();
	if (D.dark_words[0] && !talk_busy() && !emu_read8(BN6_CHATBOX) && !cinema_busy() && on_map() && talk_start(D.dark_words, FACE_MEGAMAN))
		D.dark_words[0] = 0;
}

/* The Navi gate's SP chip, given; Rush's gap, named. */
static void gate_and_rush_words(void) {
	if (D.gate_due && talk_script(D.objs.archive, D.objs.gate_reward)) D.gate_due = false;
	rush_hint();
}

void director_update(void) {
	take_events();
	if (!D.active) return;
	if (D.town) { town_update(); return; }
	++D.frame;
	++D.act_frames;
	/* MegaMan deleted: the game plays its GAME OVER, then the run ends */
	int mode = main_mode();
	if (mode == BN6_MODE_GAME_OVER && !D.gameover) {
		D.gameover = true;
		D.lost_to = boss_fighting() ? D.objs.guardian.navi : 0;
		D.lost_duel = D.duel;
		/* (deleted in a duel: a duel lost, which the verdict, never
		 * reached, would have counted; Chaud's next call knew nothing of
		 * it) */
		if (D.duel) { profile.duel_lost++; profile_save(); }
		boss_lost();
	}
	if (D.gameover) {
		if (mode == BN6_MODE_START_SCREEN) end_run();
		return;
	}
	if (follow_exit_warp()) return;
	map_label();   /* (once MegaMan has arrived: not over the jack-out) */
	arrow_update();
	if (on_map()) {
		/* what MegaMan has come near, for the map */
		int px = bn6_player_x(), py = bn6_player_y(), cx, cy;
		if (netmap_panel(px, py, &cx, &cy)) {
			for (int y = cy - 4; y <= cy + 4; ++y)
				for (int x = cx - 4; x <= cx + 4; ++x)
					if (x >= 0 && y >= 0 && x < MAP_W && y < MAP_H) D.seen[y][x] = 1;
			/* a platform he stands on, all of it */
			for (int i = 0; i < layer.nrooms; ++i) {
				const Room *r = &layer.rooms[i];
				if (cx < r->x || cy < r->y || cx >= r->x + r->w || cy >= r->y + r->h) continue;
				for (int y = r->y - 1; y <= r->y + r->h; ++y)
					for (int x = r->x - 1; x <= r->x + r->w; ++x)
						if (x >= 0 && y >= 0 && x < MAP_W && y < MAP_H) D.seen[y][x] = 1;
			}
			last_stop(cx, cy);
			/* the short net's last guardian fallen and its exit open: the
			 * run's end said on the map, before the pad (the win went from
			 * the pad straight to the title's summary) */
			if (!D.final_told && run.biome == BIOME_NEST && run_short_last(run.depth) && boss_done() && !emu_read8(BN6_CHATBOX) &&
				!talk_busy() && !cinema_busy() && !D.warping &&
				/* (the arrival's growl answered, Dad's voice, and the endless
				 * net's hook: a playtester's first win ended on two lines) */
				talk_start("@M That was the Nest's last guardian,Lan...|@M The whole Net's gone quiet.|"
					"@B Grrrr......|"
					"@M ...Almost. Something deeper down is still awake.|@M The Nest was only its den...|"
					"@D Lan,MegaMan,it's Dad! I watched it all. You did it!|"
					"@D Whatever's growling down there,we'll be ready.|@D Now jack out and come home,you two.|"
					"@L We did it!! The exit's open. Let's jack out!", FACE_MEGAMAN))
				D.final_told = true;
		}
	}
	/* (the PET's first menu is a screen of the game's own mode, its pages
	 * and the shops other modes: either; not the way back to the map after
	 * a battle or a warp, which had MegaMan name a program off the board
	 * after every fight) */
	int screen = emu_read8(BN6_GAMESTATE);
	if (main_mode() != BN6_MODE_GAME ? main_mode() != BN6_MODE_GAME_OVER : screen == BN6_SUB_PET) D.pet_seen = true;
	if (on_map()) { unwedge(); push_arrow(); bug_watch(); spin_watch(); grant_spins(); bugfrag_trade(); code_watch(); reg_watch(); draft_fit_watch(); }
	cinema_on_map(on_map());
	if (!on_map()) {
		int sub = emu_read8(BN6_GAMESTATE);
		if (sub == BN6_SUB_BATTLE_INIT || sub == BN6_SUB_BATTLE) {
			emu_battle_release();   /* the forced battle has begun */
			if (!D.in_battle) {
				bool guardian = boss_fighting();
				D.record_known = guardian;
				if (!guardian && !D.challenge) ++D.battles;
				if (guardian) runlog_battle_start(NULL, "guardian");
			}
			D.in_battle = true;
			/* the duel's time: the results screen's DeleteTime (its hits
			 * come as events: docs/RIVAL.md) */
			if (D.duel && sub == BN6_SUB_BATTLE) {
				int t = (int)emu_read32(BN6_BATTLE_TIMER);
				if (t > D.duel_time) D.duel_time = t;
			}
			/* the battle the game was handed, once StartBattle has named
			 * its record (a re-roll may have come between its roll and
			 * now) */
			if (!D.record_known) {
				int s = D.battle_record ? emu_encounter_record(D.battle_record) : -2;
				if (s >= 0) { D.next = D.rolled[s]; D.foes = D.next.nfoes; }
				if (s != -2) {
					D.record_known = true;
					loot_battle_fought(&D.next);
					runlog_battle_start(&D.next, D.challenge ? "challenge" : "battle");
				}
				if (emu_debug_on() && s != -2) fprintf(stderr, "battle from record %d: field %02x player %02x foes %d\n", s, D.next.field, D.next.player, D.foes);
			}
			/* (where the game put MegaMan, once a battle) */
			if (emu_debug_on() && !D.placed_told)
				for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
					uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
					if ((emu_read8(o) & 1) && emu_read8(o + BN6_T1_ALLIANCE) == 0) {
						fprintf(stderr, "megaman on panel %d %d\n", emu_read8(o + BN6_T1_PANEL_X), emu_read8(o + BN6_T1_PANEL_Y));
						D.placed_told = true;
						break;
					}
				}
		}
		return;
	}
	if (D.in_battle) {
		/* back from a battle: count the deleted viruses (a navi counts below) */
		D.in_battle = false;
		D.placed_told = false;
		D.battle_record = 0;
		bool won = emu_read8(BN6_BATTLE_RESULT) == 1;
		runlog_battle_end(won);
		/* the PET's battle data on the viruses just fought */
		if (!boss_fighting()) {
			for (int i = 0; i < D.next.nfoes; ++i)
				if (D.next.foes[i].kind == FOE_VIRUS) profile_family_note(D.next.foes[i].family);
			profile_save();
			/* (and, the first time, what the Mystery Data on its field
			 * was: said after it was met, kept or broken, not before) */
			for (int i = 0; i < D.next.nobj; ++i) D.gem_due |= D.next.obj[i].kind >> 4 == FIELD_GEM && !profile.gem_taught;
		}
		if (emu_debug_on() && won) {
			int r = emu_read16(BN6_BATTLE_REWARD);
			if (r >> 14 == 0 && r != 0xFFFF) {
				ChipInfo ci;
				chip_info(r & 0x1FF, &ci);
				fprintf(stderr, "battle reward %s %c (folder codes %c%c%c)\n", ci.name, (r >> 9 & 0x1F) >= 26 ? '*' : 'A' + (r >> 9 & 0x1F),
					run.codes[0] ? 'A' + run.codes[0] - 1 : '-', run.codes[1] ? 'A' + run.codes[1] - 1 : '-', run.codes[2] ? 'A' + run.codes[2] - 1 : '-');
			} else fprintf(stderr, "battle reward %04x\n", r);
		}
		if (won && !boss_fighting()) run.viruses_deleted += D.foes;
		if (!D.challenge && !boss_fighting()) roll_encounter();
	}
	/* back from the guardian's battle */
	if (boss_fighting() && !emu_battle_forcing()) {
		boss_battle_over(emu_read8(BN6_BATTLE_RESULT) == 1);
		D.pet_refreshed = false;   /* (Dad's mails made again: the Records, the report) */
	}
	if (D.challenge && !emu_battle_forcing()) {
		/* back from the challenge (the game gave its reward, the signal
		 * gives its own for a win): random battles again */
		D.challenge = false;
		bool won = emu_read8(BN6_BATTLE_RESULT) == 1;
		if (D.duel) { emu_battle_unwatch(); duel_verdict(won); D.pet_refreshed = false; }
		else if (D.gate_fight) D.gate_due = won && D.objs.gate_reward >= 0;
		else D.reward_due = won && D.objs.challenge_reward >= 0;
		D.gate_fight = false;
		roll_encounter();
	}
	/* (as a talk, MegaMan held: run straight off, the A paging its first
	 * box talked to the Server he faced, whose own words took the box, and
	 * the prize was never named) */
	if (D.reward_due && talk_script(D.objs.archive, D.objs.challenge_reward)) D.reward_due = false;
	/* the PET's words that count (the codes, the Library), the profile's
	 * key items and Dad's mail, once a layer is under way (after a
	 * CONTINUE's state, which holds what the run had) */
	if (!D.pet_refreshed && on_map()) {
		D.pet_refreshed = true;
		int mailed = pet_text_refresh();
		if (mailed && D.mail_quiet) D.mail_due = mailed;
		D.mail_quiet = true;
	}
	/* (after the arrival's card and words: said over the jack-in, it was
	 * lost under them) */
	if (D.mail_due && !D.reward_due && !D.gem_due && !D.area_card && !D.beat[0] && !cinema_busy() && !talk_busy() &&
		!emu_read8(BN6_CHATBOX) && !boss_cinematic()) {
		char words[160];
		snprintf(words, sizeof words, "@M Lan,you've got mail from Dad!|@M Our battle data on %s! It's in the PET's E-Mail.",
			guardian(D.mail_due)->name);
		if (talk_start(words, FACE_MEGAMAN)) D.mail_due = 0;
	}
	/* the rival (docs/RIVAL.md): Chaud's verdict after the duel, his call
	 * once the layer's arrival has been said */
	if (D.duel_verdict_due && on_map() && !cinema_busy() && !talk_busy() && !emu_read8(BN6_CHATBOX) &&
		talk_start(D.duel_verdict, FACE_CHAUD))
		D.duel_verdict_due = false;
	if (D.duel_call_due && !D.reward_due && !D.gem_due && !D.mail_due && !D.area_card && !D.beat[0] && !cinema_busy() && !talk_busy() &&
		!emu_read8(BN6_CHATBOX) && !boss_cinematic() && D.frame > 60) {
		const char *call = duel_call_words();
		if (talk_start(call, FACE_CHAUD)) {
			D.duel_call_due = false;
			flag_set(LAYER_DUEL_CALLED_FLAG);
		}
	}
	if (D.gem_due && !D.reward_due && talk_start("@M Lan! Mystery Data on the battlefield!|@M Any hit breaks it,theirs or ours.|"
		"@M If it's still there when we win,it's ours!", FACE_MEGAMAN)) {
		D.gem_due = false;
		profile.gem_taught = 1;
		profile_save();
	}
	words_due();
	if (D.fragment_due && talk_script(D.objs.archive, D.objs.fragment_found)) {
		D.fragment_due = false;
		D.fragments_told = run.fragments;
	}
	exit_flag(false);
	guardian_update();
	/* (after the last card: the area cleared on the way here) */
	if (D.area_card && ++D.arrived >= AREA_CARD_AT && !cinema_busy()) {
		D.area_card = false;
		area_card();
	}
	if (emu_read8(BN6_CHATBOX)) cinema_card_yield();
	/* the arrival's cards and MegaMan's words after them are one beat: he
	 * is held from his arrival until the words begin, as BN6 holds him for
	 * its own scenes, and A ends a card early (issue #13: free under the
	 * card, a player walked to a Mystery Data and opened it, and the words
	 * came after it, out of their moment) */
	bool hold = D.beat[0] && (D.area_card || cinema_busy()) && !talk_busy();
	if (hold && cinema_input_mode() == CINEMA_FREE) cinema_input(CINEMA_HOLD);
	else if (!hold && D.arrival_hold && cinema_input_mode() == CINEMA_HOLD) cinema_input(CINEMA_FREE);
	D.arrival_hold = hold;
	/* the arrival's words once the card has gone; Chaud's call once the
	 * Secret Area's guardian is done */
	talk_update();
	if (!D.area_card && !cinema_busy() && !boss_cinematic() && !boss_fighting() && !talk_busy()) {
		if (D.beat[0] && talk_start(D.beat, FACE_MEGAMAN)) beat_said();
		else if (D.secret_call && boss_done() &&
			talk_start("@C Lan,it's Chaud.|@C That wasn't ProtoMan. He's been in my PET all day.|@C You beat a copy. Watch yourself.|"
				"@M The Nest can even copy ProtoMan...|@L Then we'd better stay on guard!", FACE_MEGAMAN)) {
			D.secret_call = false;
		}
	}
	dev_steps();
	if (act_on_choices()) return;
	/* (never with a chat box open: a state would keep it, and the talk
	 * slot's text is not in a state) */
	/* (nor while the arrival still holds him: the jack-in and the warp pad
	 * keep him for about 90 frames, and the release is not in the state) */
	if (boss_take_checkpoint()) D.checkpoint = D.checkpoint_data = true;
	if (D.checkpoint && D.frame >= CHECKPOINT_AFTER && !talk_busy() && !emu_read8(BN6_CHATBOX) &&
		!emu_read8(BN6_DIALOGUE_LOCK) && flag_get(BN6_FLAG_PLAYER_CAN_MOVE)) {
		D.checkpoint = false;
		save_checkpoint();
		/* (said: a playtester who plays in short sessions asked where it
		 * is safe to stop) */
		D.saved_at = D.checkpoint_here ? "Run saved where you saved it" : D.checkpoint_data ? "Run saved at the Guardian Data" :
			"Run saved at the layer's start";
		D.checkpoint_data = D.checkpoint_here = false;
		cinema_note("Run saved", 150);
		if (D.nest_cleared) { D.nest_cleared = false; profile.nest_clears++; profile_save(); }
	}
	/* on another map (a story warp the run does not use): back to the layer */
	if (emu_read8(BN6_MAP_GROUP) != D.group || emu_read8(BN6_MAP_NUMBER) != D.number) {
		if (++D.astray > ASTRAY_FRAMES) {
			D.astray = 0;
			emu_warp(D.group, D.number, D.start_x, D.start_y, 4);
		}
		return;
	}
	D.astray = 0;
	/* (not over a battle that is about to start) */
	if (D.frame % REROLL_FRAMES == 0 && !emu_battle_forcing()) roll_encounter();
}
