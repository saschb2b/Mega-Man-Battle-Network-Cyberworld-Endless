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

#include "battle_floor.h"
#include "bn6_fields.h"
#include "boss.h"
#include "briefing_words.h"
#include "cinema.h"
#include "darkbn6.h"
#include "data.h"
#include "debug.h"
#include "director_battler.h"
#include "director_board.h"
#include "director_dark.h"
#include "director_dev.h"
#include "director_duel.h"
#include "director_folder.h"
#include "director_home.h"
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
#include "loot.h"
#include "nets_words.h"
#include "netmap.h"
#include "pet_text.h"
#include "save.h"
#include "story_words.h"
#include "talk.h"

static void gate_words(void);

#define REROLL_FRAMES 300  /* the next battle's enemies are re-rolled this often */
DirectorState D;

#define AREA_CARD_AT 45   /* frames on the map after arriving */

/* The run is won where MegaMan stands: the short net's last guardian
 * deleted, only the exit ahead (no reminders then: a playtester was told
 * to place a program on the walk to it). */
bool run_won_here(void) {
	return run.side_kind == LAYER_NORMAL && run.mode == RUN_SHORT && run_short_last(run.depth) && boss_done();
}

#define ASTRAY_FRAMES     90     /* on another map this long: warp back */

int main_mode(void) { return emu_read8(emu_read32(BN6_TOOLKIT)); }
/* walking the net: the game mode on its map sub-mode (not a battle or menu) */
bool on_map(void) { return main_mode() == BN6_MODE_GAME && emu_read8(BN6_GAMESTATE) == BN6_SUB_MAP; }

int key_item(int id) { return emu_read8(emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS) + (uint32_t)id); }

bool director_in_town(void) { return D.active && D.town && !home_in_hp(); }

bool director_in_hp(void) { return D.active && home_in_hp(); }

const char *director_place_name(void) { return home_place_name(); }

int director_town_hour(void) { return home_hour(); }

int director_town_tint(bool *box) {
	*box = D.tint_box;
	return D.active && D.town ? D.tint : 0;
}

bool director_on_layer(void) { return D.active && !D.town; }

int director_guardian_battle(void) { return D.active && boss_fighting() ? D.objs.guardian.navi : 0; }

int director_trader_kind(void) { return D.active && !D.town ? D.objs.trader_kind : -1; }

int director_guardian_waiting(void) { return D.active && !D.town && D.objs.guardian.navi && !boss_exit_open() ? D.objs.guardian.navi : 0; }

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
	return D.town ? home_map(group, number) : group == D.group && number == D.number;
}

/* ---- dev tools (src/dev/devtools.c) ---- */

bool director_on_map(void) { return D.active && on_map(); }

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
			 * band as he spawns, his deleting blow ending it instead:
			 * docs/game/rival.md) */
			emu_battle_watch(D.duel_cap, layer_objs_duel_rung == 2);
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
		case OBJ_BATTLER:
			/* (his stake paid: BN6's own first round, docs/world/level-design/) */
			battler_begin();
			return true;
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

/* What the run keeps as MegaMan leaves a layer by its exit: past the short
 * net's first Nest guardian the second's (threat 10); past the endless
 * net's, the net rebuilt (the next arrival says so; counted with the next
 * checkpoint, which a CONTINUE cannot undo) and its den the Cybeast's
 * again; a dark layer's special route found, kept for its fork's third way
 * (docs/game/liberation/special-routes.md) */
static void run_left(void) {
	if (boss_beaten() && run.biome == BIOME_NEST && run_short_nest(run.depth)) run.boss_order[BIOME_NEST] = (uint8_t)run_nest_second();
	if (boss_beaten() && run.biome == BIOME_NEST && !run_short_nest(run.depth)) {
		D.nest_cleared = true;
		run_nest_rebuilt();
	}
	if (run.side_kind == LAYER_NORMAL && lib_route_found()) run_route_keep(run.depth);
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
	 * guardian's exit leads down to the second's, docs/game/meta/) */
	if (boss_beaten() && run.biome == BIOME_NEST && run_short_last(run.depth)) { win_run(); return true; }
	run_left();
	runlog_layer_left(D.battles, D.objs.guardian.navi);
	if (boss_beaten()) clear_card();
	/* (an act's last layer: home, docs/world/home/) */
	bool home = home_due();
	const char *beaten = D.act_guardian;
	/* a side layer's exit leads one area deeper too; a trip back's home
	 * (docs/world/home/), the run's own layer made again for it */
	bool back = run.side_kind == LAYER_BACK;
	if (back) run.depth = run.home_depth;
	else run.depth++;
	run.home_depth = 0;
	run.side_kind = LAYER_NORMAL;
	note_folder_codes();
	if (!new_layer(true)) return false;
	if ((home && home_begin(beaten)) || (back && home_return())) return true;
	D.warping = true;
	D.checkpoint = true;
	return true;
}

/* The exit pad shut while a guardian stands: its warp-off flag, written
 * as the guardian's state changes and after the game enters a map (bn6f
 * EnterMap clears the map's flags, 0x1640-0x16FF: EV_MAP_ENTER). */
static void exit_flag(bool entered) {
	bool shut = !boss_exit_open() || lib_exit_shut() || floor_owed();
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
		 * price after it, docs/game/meta/) */
		case EV_DARK_RAN: case EV_DARK_BASE: darkbn6_event(&ev[i]); break;
		/* (not in the fight's first second: a Chumpy rammed a playtester at
		 * 0:00.70, as BATTLE START left the screen, and the no-hit duel was
		 * lost before he could act, session 65) */
		case EV_MEGAMAN_HIT: if (emu_read32(BN6_BATTLE_TIMER) >= DUEL_GRACE) D.duel_hit = true; break;
		case EV_MAP_ENTER: if (on_layer) exit_flag(true); else if (D.active) home_entered(); break;
		case EV_CHOICE:
			/* (still set: one the layer's setup cleared was a story's) */
			for (int k = 0; on_layer && k < D.objs.nchoices; ++k)
				if (D.objs.choice[k].flag == (int)ev[i].r[0] && flag_get(D.objs.choice[k].flag)) D.choices_due |= 1u << k;
			break;
		case EV_ITEM_GIVEN: if (on_layer) item_given((int)ev[i].r[0]); break;
		case EV_JOB: home_jobs_flag((int)ev[i].r[0]); break;
		}
}

/* The events queued before the run (the boot's), dropped, and a last run's
 * battle's dark power (a GAME OVER never came back to the map to pay it). */
void drop_events(void) {
	HookEvent ev[32];
	while (emu_hook_events(ev, 32) == 32) {}
	battler_forget();
	darkbn6_battle_begins();
	dark_forget();
}

static void words_due(void) {
	gate_words();
	nets_words_due();
	dark_flame_watch();
	/* (the run's DarkChips: in the Pack, BN6's power as the BugFrags held
	 * say, the price of a battle a dark power ran in, either net's) */
	dark_pack();
	darkbn6_sync();
	dark_after_battle();
	if (D.dark_words[0] && !talk_busy() && !emu_read8(BN6_CHATBOX) && !cinema_busy() && on_map() && talk_start(D.dark_words, FACE_MEGAMAN))
		D.dark_words[0] = 0;
	home_jobs_words();
}

/* The Navi gate's SP chip, given. */
static void gate_words(void) {
	if (D.gate_due && talk_script(D.objs.archive, D.objs.gate_reward)) D.gate_due = false;
}

/* MegaMan deleted: the game plays its GAME OVER, then the run ends; true
 * while it plays */
static bool gameover_update(void) {
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
		return true;
	}
	return false;
}

/* The short net's last guardian fallen and its exit open: the run's end
 * said on the map, before the pad (the win went from the pad straight to
 * the title's summary) */
static void final_update(void) {
	if (!D.final_told && run.biome == BIOME_NEST && run_short_last(run.depth) && boss_done() && !emu_read8(BN6_CHATBOX) &&
		!talk_busy() && !cinema_busy() && !D.warping && talk_start(final_words(), FACE_MEGAMAN))
		D.final_told = true;
}

/* MegaMan on the map: what he has come near, for the map; the last stop
 * before a guardian's arena; the short net's end */
static void map_update(void) {
	/* what MegaMan has come near, for the map */
	int px = bn6_player_x(), py = bn6_player_y(), cx, cy;
	if (!netmap_panel(px, py, &cx, &cy)) return;
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
	final_update();
}

/* (where the game put MegaMan, once a battle: the debug log's) */
static void placed_tell(void) {
	for (uint32_t i = 0; i < BN6_T1_COUNT; ++i) {
		uint32_t o = BN6_T1_OBJECTS + i * BN6_T1_SIZE;
		if ((emu_read8(o) & 1) && emu_read8(o + BN6_T1_ALLIANCE) == 0) {
			fprintf(stderr, "megaman on panel %d %d\n", emu_read8(o + BN6_T1_PANEL_X), emu_read8(o + BN6_T1_PANEL_Y));
			D.placed_told = true;
			return;
		}
	}
}

/* A battle under way, the game off its map: its start counted, the duel's
 * time, the battle the game was handed */
static void battle_update(void) {
	int sub = emu_read8(BN6_GAMESTATE);
	if (sub != BN6_SUB_BATTLE_INIT && sub != BN6_SUB_BATTLE) return;
	emu_battle_release();   /* the forced battle has begun */
	/* (a Virus Battler's round: BN6's own record, its viruses on MegaMan's
	 * side, none of MegaMan's battles' counts) */
	if (D.battler) {
		D.in_battle = true;
		return;
	}
	if (!D.in_battle) {
		bool guardian = boss_fighting();
		D.record_known = guardian;
		if (!guardian && !D.challenge) ++D.battles;
		/* (the one fought, BN5's by his id moved into BN6's engine) */
		int hp = 0;
		const Encounter *g = guardian ? boss_battle(&hp) : NULL;
		if (guardian) runlog_guardian_start(D.objs.guardian.navi, g, hp);
		D.souls_used = 0;
		home_jobs_battle_start();
	}
	D.in_battle = true;
	if (sub == BN6_SUB_BATTLE) {
		runlog_fought();
		lib_battle_frame();
		home_jobs_battle_frame();
		D.souls_used |= emu_battle_souls();   /* (the run log's unisons) */
	}
	/* the duel's time: the results screen's DeleteTime (its hits
	 * come as events: docs/game/rival.md) */
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
			runlog_battle_start(&D.next, D.duel ? "duel" : D.challenge ? "challenge" : D.floor_fight ? "floor" : "battle");
		}
		if (emu_debug_on() && s != -2) fprintf(stderr, "battle from record %d: field %02x player %02x foes %d\n", s, D.next.field, D.next.player, D.foes);
	}
	if (emu_debug_on() && !D.placed_told) placed_tell();
}

/* Back on the map from a battle: its viruses counted, the PET's battle
 * data, the next battle rolled */
static void after_battle(void) {
	/* back from a battle: count the deleted viruses (a navi counts below) */
	D.in_battle = false;
	D.floor_fight = false;
	D.placed_told = false;
	D.battle_record = 0;
	/* (and a Rare deleted on BN6's BtlrCard, for the profile's card) */
	battler_card_note();
	if (D.battler) return;
	bool won = emu_read8(BN6_BATTLE_RESULT) == 1;
	runlog_battle_souls(D.souls_used);
	runlog_battle_end(won, (int)emu_read32(BN6_BATTLE_TIMER));
	/* (a request's battles: a guardian's counts for none) */
	if (!boss_fighting()) home_jobs_battle_end(won, emu_read16(BN6_NAVI_HP), -1);
	bug_pay_battle(won);
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
	if (!boss_fighting()) nets_battle_note(&D.next);   /* (a visitor of the other net: MegaMan's word, once a profile) */
	if (!D.challenge && !boss_fighting()) roll_encounter();
}

/* Back from the guardian's battle, or a challenge */
static void after_forced(void) {
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
		if (D.duel) { duel_verdict(won, emu_battle_unwatch()); D.pet_refreshed = false; }
		else if (D.battler) battler_round_over(emu_read8(BN6_BATTLE_RESULT));
		else if (D.gate_fight) D.gate_due = won && D.objs.gate_reward >= 0;
		else {
			/* (its pick of three, there at the Server while B leaves it) */
			if (won) flag_set(LAYER_SERVER_WON_FLAG);
			D.reward_due = won && D.objs.challenge_reward >= 0;
		}
		D.gate_fight = false;
		roll_encounter();
	}
}

/* The PET's words, and Dad's mail once the arrival is done */
static void mail_update(void) {
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
	 * lost under them; and after a guardian's data is taken, which it had
	 * stopped MegaMan short of) */
	if (D.mail_due && !D.reward_due && !D.area_card && !D.beat[0] && !cinema_busy() && !talk_busy() &&
		!emu_read8(BN6_CHATBOX) && !boss_cinematic() && !boss_reward_waiting()) {
		if (talk_start(mail_words(D.mail_due), FACE_MEGAMAN)) D.mail_due = 0;
	}
}

/* Chaud's verdict and call */
static void rival_update(void) {
	/* the rival (docs/game/rival.md): Chaud's verdict after the duel, his call
	 * once the layer's arrival has been said */
	if (D.duel_verdict_due && on_map() && !cinema_busy() && !talk_busy() && !emu_read8(BN6_CHATBOX) &&
		talk_start(D.duel_verdict, FACE_CHAUD))
		D.duel_verdict_due = false;
	if (D.duel_call_due && !D.reward_due && !D.mail_due && !D.area_card && !D.beat[0] && !cinema_busy() && !talk_busy() &&
		!emu_read8(BN6_CHATBOX) && !boss_cinematic() && D.frame > 60) {
		const char *call = duel_call_words();
		if (talk_start(call, FACE_CHAUD)) {
			D.duel_call_due = false;
			flag_set(LAYER_DUEL_CALLED_FLAG);
		}
	}
}

/* What waits for its moment on the map: the Server's pick, the PET's words
 * and Dad's mail, Chaud's verdict and call, the lessons, a ScrtData's */
static void talks_due(void) {
	/* (as a talk, MegaMan held: run straight off, the A paging its first
	 * box talked to the Server he faced, whose own words took the box, and
	 * the prize was never named) */
	if (D.reward_due && talk_script(D.objs.archive, D.objs.challenge_reward)) D.reward_due = false;
	mail_update();
	rival_update();
	words_due();
	if (D.fragment_due && talk_script(D.objs.archive, D.objs.fragment_found)) D.fragment_due = false;
}

/* The arrival's cards and MegaMan's words after them, as one beat; Chaud's
 * call once the Secret Area's guardian is done */
static void arrival_update(void) {
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
		else if (D.after_call && boss_done() && talk_start(D.objs.guardian.navi == SUPER_CYBEAST ? den_call_words() : secret_call_words(), FACE_MEGAMAN)) {
			D.after_call = false;
		}
	}
}

/* On another map (a story warp the run does not use): back to the layer;
 * true while astray */
static bool astray_update(void) {
	if (emu_read8(BN6_MAP_GROUP) != D.group || emu_read8(BN6_MAP_NUMBER) != D.number) {
		if (++D.astray > ASTRAY_FRAMES) {
			D.astray = 0;
			emu_warp(D.group, D.number, D.start_x, D.start_y, D.start_face);
		}
		return true;
	}
	D.astray = 0;
	return false;
}

void director_update(void) {
	/* (the run's time, a game frame at a time, so fast forward leaves the
	 * statistics' minutes and the week's Board as they were: save.h) */
	profile_played_frame();
	take_events();
	if (!D.active) return;
	if (D.town) { home_update(); return; }
	++D.frame;
	++D.act_frames;
	if (gameover_update() || follow_exit_warp()) return;
	map_label();   /* (once MegaMan has arrived: not over the jack-out) */
	/* (no way-on arrow over a liberation's board: its way is the board's) */
	if (!lib_on()) arrow_update();
	if (on_map()) map_update();
	/* (the PET's first menu is a screen of the game's own mode, its pages
	 * and the shops other modes: either; not the way back to the map after
	 * a battle or a warp, which had MegaMan name a program off the board
	 * after every fight) */
	int screen = emu_read8(BN6_GAMESTATE);
	if (main_mode() != BN6_MODE_GAME ? main_mode() != BN6_MODE_GAME_OVER : screen == BN6_SUB_PET) D.pet_seen = true;
	if (on_map()) { unwedge(); if (!lib_on()) push_arrow(); exit_pad_step(); floor_walk(); hp_full_watch(); bug_watch(); spin_watch(); grant_spins(); bugfrag_trade(); code_watch(); reg_watch(); draft_skip_watch(); home_jobs_watch(); runlog_walked(); }
	cinema_on_map(on_map());
	if (!on_map()) {
		battle_update();
		return;
	}
	if (D.in_battle) after_battle();
	after_forced();
	talks_due();
	exit_flag(false);
	guardian_update();
	arrival_update();
	lib_update();
	dev_steps();
	battler_update();
	if (act_on_choices()) return;
	checkpoint_update();
	if (astray_update()) return;
	/* (not over a battle that is about to start) */
	if (D.frame % REROLL_FRAMES == 0 && !emu_battle_forcing()) roll_encounter();
}
