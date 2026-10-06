/* The actors of a guardian's sequence (docs/BOSSES.md): NPC scripts that
 * wait hidden for the director's event flags. */
#ifndef CW_STAGE_NPC_H
#define CW_STAGE_NPC_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	int appear;   /* the guardian logs in */
	int gone;     /* the guardian logs out, deleted */
	int reward;   /* its Guardian Data shows */
	int taken;    /* ... and has been taken */
	int pose;     /* a super boss strikes his pose (npc_super) */
	int seal;     /* Bass's stone shakes (npc_seal) */
} StageFlags;

/* A guardian's body on the net: a sprite (its list and number), the
 * animation it stands in, mirrored or not, the animation it logs in by,
 * the pose it strikes for the title card and the animation it logs out
 * by (-1 none: it shows, or goes, as BN6's beam rises). */
typedef struct {
	int list, index, anim;
	bool mirror;
	int log_in, pose, log_out;
} NpcBody;

/* The guardian: body `b` at world (x, y, z); on `appear` it logs in and
 * strikes its pose; on `gone` it logs out and leaves, as BN6's Navis do on
 * the net, beside the beam (npc_beam). Nobody can talk to it. */
uint32_t npc_guardian(const NpcBody *b, int x, int y, int z, const StageFlags *f);
/* BN6's beam where a Navi logs in or out on the net (map object sprite
 * list 7's 0: its animation 0 in, 1 out, BN6's sound 0x76 with each), at
 * world (x, y, z): in on `appear`, out on `gone`. */
uint32_t npc_beam(int x, int y, int z, const StageFlags *f);
/* The Guardian Data it leaves: a Mystery Data crystal (animation `anim`)
 * that shows on `reward` and runs text `script` of `archive` when checked,
 * which should set `taken`. */
uint32_t npc_guardian_data(int x, int y, int z, int anim, uint32_t archive, int script, const StageFlags *f);
/* A floor pad (sprite list `category`, `index`) that appears once
 * `open_flag` is set. */
uint32_t npc_sealed_pad(int category, int index, int x, int y, int z, int open_flag);

/* A super boss's body (docs/BOSSES.md, Super bosses): his sprite, the
 * animation he stands in and his pose (NpcBody's; no log-in), how long the
 * pose holds and what he stands in after it (Bass his cloak thrown off),
 * and whether he fades slowly (the beast) */
typedef struct {
	NpcBody body;
	int pose_frames, after;
	bool slow;
} SuperBody;
/* A super boss at world (x, y, z): hidden until `appear`, when he stands
 * there at once, out of the white; on `pose` his pose; on `gone` he fades
 * away. Nobody can talk to him. */
uint32_t npc_super(const SuperBody *b, int x, int y, int z, const StageFlags *f);
/* Bass's dormant stone, BN6's own (sprite list 7 0x9B), its screen
 * flickering where he sleeps: on `seal` it shakes with BN6's crack (sound
 * 0xFE), on `appear` it lies in pieces, on `gone` its pieces fade with
 * him. Nothing walks into it. */
#define SEAL_SPRITE 0x9B
uint32_t npc_seal(int x, int y, int z, const StageFlags *f);

#endif
