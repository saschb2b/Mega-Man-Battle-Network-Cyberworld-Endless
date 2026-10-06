/* The director's state (D, director.c), shared by its parts (director_*.c, *_words.c), and
 * director.c's helpers they use. */
#ifndef CW_DIRECTOR_STATE_H
#define CW_DIRECTOR_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "layer_objs.h"
#include "navicust.h"
#include "rom.h"
#include "runlog.h"

typedef struct {
	bool active;
	int group, number;
	int frame;
	bool checkpoint;       /* save once MegaMan has arrived */
	bool gameover;         /* the game's GAME OVER is playing */
	int start_x, start_y;
	int start_face;        /* the way MegaMan faces there: along the way on (layer.arrive_face, issue #106) */
	LayerObjs objs;
	unsigned chosen;       /* choices already acted on (bit per choice) */
	bool challenge;        /* a challenge battle was started */
	bool reward_due;       /* ... and won: its prize is told (and given) once a talk can start */
	bool gate_fight;       /* the challenge is a Navi gate's SP (docs/META.md, gates) */
	/* the rival's duel (docs/RIVAL.md): the layer's squad, the duel under
	 * way (a hit taken, the DeleteTime), and Chaud's words due: his call
	 * as the layer begins, his verdict after */
	Encounter duel_enc;
	Encounter server_enc;  /* the layer's Server's battle, rolled with the layer (its words name a Navi) */
	bool server_rolled;
	bool duel, duel_hit, duel_call_due, duel_verdict_due;
	int duel_time;
	int duel_cap;       /* the netbattle's ProtoMan at most this HP (half the act's guardian band's top), 0 none */
	bool lost_duel;     /* MegaMan deleted in the rival's duel (the summary says so) */
	char duel_verdict[512];
	bool gate_due;         /* ... and won: his SP chip is given once a talk can start */
	bool fragment_due;     /* ... once a talk can start */
	bool spin_due;         /* the run's Spin picked up: MegaMan says what it does once a talk can start */
	int fragments_told;    /* ScrtData L's briefing (or MegaMan at one) last counted */
	bool in_battle;        /* a battle is on */
	bool record_known;     /* the battle's record (D.rolled) is known */
	unsigned choices_due;  /* the layer's choices taken (EV_CHOICE), acted on once the chat box closes */
	bool exit_shut;        /* the exit pad's warp-off flag as last written */
	uint32_t battle_record;   /* the BattleSettings StartBattle was given (EV_BATTLE_START), 0 none since the last battle */
	bool placed_told;      /* (debug) MegaMan's first panel in it was printed */
	int foes;              /* viruses in the battle the game will start next */
	Encounter next;        /* that battle */
	Encounter rolled[2];   /* the battles in the two records the roll hands out (encounter.c) */
	int battles;           /* random battles fought on this layer */
	int guest_xrom, guest_group, guest_number;   /* the layer's battles in another game's engine: its ROM, the map whose records they take */
	const NetAreaDef *guest_area;   /* ... and the area's whole definition (its other maps' records) */
	int guest_foes;        /* the viruses its battle under way set, counted as deleted for a win */
	int guest_guardian;    /* ... or the guardian of the older net it is (guardians.h, guardian_older), 0 none */
	bool battle_due;       /* (dev: the battle step) the next random battle, at the first free moment on the map */
	int layer_tiles;       /* the area the layer draws in (layer_area): another game's where it dresses BN6's */
	bool beat_cross;       /* the arrival's words say the older net had no Crosses */
	bool beat_out;         /* ... or, to a profile told that, the chips that sit out, once a run */
	bool beat_beast;       /* ... and that the Cybeast can't come in, the first time a profile arrives holding BeastOut */
	int astray;            /* frames MegaMan has spent on another map */
	bool warping;          /* the exit pad's warp is under way */
	bool area_card;        /* show the area's title card once MegaMan is in */
	bool arrival_hold;     /* MegaMan held for the arrival's cards and the words after them */
	int arrived;           /* frames on the layer's map since the warp ended */
	int act_viruses;       /* viruses deleted when the act began */
	int act_frames;        /* frames spent in the act */
	const char *act_guardian;  /* the guardian beaten on the way out */
	bool town;             /* home: the town or Lan's HP; the next layer waits behind a portal */
	bool home;             /* ... come home after an act (docs/HOME.md), not the run's start */
	bool hp_said;          /* ... MegaMan has said what Lan's HP is, this run */
	bool portal_taken;     /* ... a portal's way is under way */
	const char *home_beaten;   /* ... the guardian deleted on the way */
	bool home_back;        /* ... home from a trip back (docs/HOME.md, going back), not an act */
	bool home_saved;       /* ... and the run saved there */
	bool home_save_due;    /* the PET's Save at home, waiting for MegaMan (or Lan) free */
	unsigned home_told;    /* ... the portals MegaMan has named this visit, a bit each */
	bool town_seen;        /* ... and has got there */
	int town_frames;       /* frames on the town's map */
	int tint;              /* the town's hour as its picture is tinted (home_hour), 0 none: on its map alone */
	bool tint_box;         /* ... a chat box open over it, which keeps its own colours */
	bool intro_said;       /* Lan and MegaMan have spoken there */
	char beat[900];        /* what they say on arriving, once the card has gone */
	bool after_call;       /* a call after the layer's guardian is due: Chaud's after the Secret Area's, Dad's after the Cybeast */
	bool act_resumed;      /* the act was continued from a checkpoint: no clear stats */
	bool l_held, r_held, a_held;   /* L, R and A were down last frame */
	bool chat_was_open;            /* the game's chat box was open last frame */
	int a_quiet;                   /* frames an A is not passed on after a chat on the map closed */
	int l_kept;                    /* frames an L pressed while busy is kept */
	int counts_t;                  /* frames L's Mystery Data counters stay once its words are gone */
	int walk_to, walk_t;           /* the NPC slot MegaMan walks up to after an A short of it, frames left */
	bool dir_held;         /* a direction is held this frame */
	bool map_shown;        /* SELECT is held on a layer's map: the map shows */
	uint8_t seen[MAP_H][MAP_W];   /* panels MegaMan has come near on this layer */
	bool arrow_pending;    /* the way-on arrow lasts until a little after L's words close */
	int free_x, free_y;    /* MegaMan's last place clear of every NPC */
	int wedged;            /* frames he has pushed, unmoving, against an NPC he stands inside */
	int last_x, last_y;    /* where he stood the frame before */
	bool port_told;        /* MegaMan has said where the way on is at home (the town's port, Lan's HP's pink pad) and how to jack in or out */
	bool errands_told;     /* ... and, this visit, what the town holds besides: requests posted, AsterLand's order */
	bool layer_told;       /* ... where they are on this layer (as LAYER_TOLD_FLAG) */
	bool more_told;        /* ... and, at a second L, what else the layer holds */
	int layer_act;         /* 1 + the act of the layer built last, 0 none (a side layer) */
	int dealer_act;        /* 1 + the act whose Net Dealer has already spoken, 0 none (kept across a CONTINUE: act_note) */
	int heard_act;         /* 1 + the act whose guardian a bystander has named, 0 none (kept across a CONTINUE: act_note) */
	int guest_due;         /* 1 + the chips that sat out of an older net's first battle, its words due (once a profile) */
	char guest_out[96];    /* ... those chips, named (out_names) */
	int recode_due;        /* ... it read a chip's code its own way: 1 going in, 2 the reward coming back (once a profile each) */
	int recode_chip, recode_from, recode_to;   /* the chip, and its codes out here and in there */
	int dark_kind;         /* the DarkChip in the layer's flame of darkness (darkchips.h), -1 none */
	bool dark_given;       /* ... and the run holds it */
	bool dark_pack_due;    /* BN6's chips of the run's DarkChips are looked for in its Pack and folder, given where missing (dark_pack) */
	char dark_words[360];  /* MegaMan's words on a DarkChip's price (and a fall it rose from), due ("" none) */
	int mail_due;          /* a guardian whose battle data Dad has just mailed (the PET's E-Mail), 0 none */
	bool mail_quiet;       /* the session's first mails come without a word (a run's start brings every guardian's) */
	bool pet_refreshed;    /* the layer's PET words, items and mail made (once on the map: a warp's frames go by unseen) */
	bool checkpoint_data;  /* the checkpoint due is the Guardian Data's ... */
	bool checkpoint_here;  /* ... or the PET's Save's, where MegaMan stands */
	bool checkpoint_door;  /* the checkpoint being saved is the arena's door's */
	const char *saved_at;  /* where the run was last saved, for the quit prompt */
	bool beat_guardian;    /* the arrival's words (beat) name the act's guardian ... */
	bool guardian_named;   /* ... and have been said on this layer */
	int lost_to;           /* the guardian MegaMan was deleted by, 0 none */
	bool nest_cleared;     /* the Nest's guardian fell; the profile counts it at the checkpoint */
	uint8_t bugs[NAVICUST_BUGS];   /* the NaviCust's bug counts MegaMan last spoke of */
	uint32_t board;        /* ... the board they were read with (board_hash) */
	int board_size;        /* ... and its ExpMemry */
	bool bugs_known;       /* ... read on this layer */
	int programs_seen;     /* ... with the programs in the PET, counted since */
	bool pet_seen;         /* the PET's menus were open since the map was last quiet */
	bool off_told;         /* ... and MegaMan has said, on this layer, that a program is off the board */
	bool last_stop_told;   /* ... and named the Net Dealer and the heal before the guardian's arena */
	bool final_told;       /* ... and, the short net's last guardian fallen, said the run is won */
	int code_due;          /* a program whose compression code was entered for the first time in any run, until MegaMan says Dad keeps it (issue #50) */
	bool reg_due;          /* a RegUp just found, until MegaMan says what Reg memory MegaMan has now (issue #51) */
} DirectorState;

extern DirectorState D;

bool run_won_here(void);
int main_mode(void);
bool on_map(void);
int key_item(int id);
void end_run(void);
void drop_events(void);

#endif
