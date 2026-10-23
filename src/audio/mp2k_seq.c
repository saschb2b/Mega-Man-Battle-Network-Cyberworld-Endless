/* MP2K's sequencer (mp2k_state.h): a frame of each music player, its
 * tracks' commands, its fade, as BN6's MPlayMain and its jump table run them
 * (bn6f asm/libs.s; Gregar's addresses in docs/ROM_DATA.md, BN6's sound
 * engine). */
#include "mp2k_state.h"

#include <string.h>

#include "gba_sound.h"

/* ---- the channels' lists ---- */

void mp2k_clear_chain(int chan) {
	Mp2kChan *c = &M.chans[chan];
	if (c->track == MP2K_NONE) return;
	if (c->prev != MP2K_NONE) M.chans[c->prev].next = c->next;
	else M.tracks[c->track].chan = c->next;
	if (c->next != MP2K_NONE) M.chans[c->next].prev = c->prev;
	c->track = MP2K_NONE;
}

/* the next on a list (a channel that names itself ends it, as BN6's loops
 * take it) */
static int next_of(int chan) {
	int n = M.chans[chan].next;
	if (n == chan) M.chans[chan].next = n = MP2K_NONE;
	return n;
}

static int track_index(const Mp2kTrack *t) { return (int)(t - M.tracks); }

void mp2k_track_stop(Mp2kTrack *t) {
	if (!(t->flags & TRK_EXIST)) return;
	for (int c = t->chan; c != MP2K_NONE;) {
		Mp2kChan *ch = &M.chans[c];
		if (ch->status) {
			if (ch->type & 7) mp2k_cgb_off(ch->type & 7);
			ch->status = 0;
		}
		ch->track = MP2K_NONE;
		c = next_of(c);
	}
	t->chan = MP2K_NONE;
}

/* ply_fine: the track's notes let go, the track ended */
static void cmd_fine(Mp2kTrack *t) {
	for (int c = t->chan; c != MP2K_NONE;) {
		Mp2kChan *ch = &M.chans[c];
		if (ch->status & CHN_ON) ch->status |= CHN_STOP;
		mp2k_clear_chain(c);
		c = next_of(c);
	}
	t->flags = 0;
}

/* ---- the commands' arguments ---- */

static uint8_t arg(Mp2kTrack *t) { return mp2k_u8(t->cmd++); }

static void cmd_goto(Mp2kTrack *t) { t->cmd = mp2k_u32(t->cmd); }

/* clear_modM: the LFO's swing back to 0 */
static void clear_mod(Mp2kTrack *t) {
	t->mod_m = 0;
	t->lfo_speed_c = 0;
	t->flags |= t->mod_t == 0 ? TRK_PITCHG : TRK_VOLCHG;
}

static void cmd_rept(Mp2kTrack *t) {
	uint8_t count = mp2k_u8(t->cmd);
	if (count == 0) {
		++t->cmd;
		cmd_goto(t);
		return;
	}
	++t->repeat;
	++t->cmd;
	if (t->repeat < count) cmd_goto(t);
	else {
		t->repeat = 0;
		t->cmd += 4;
	}
}

static void cmd_voice(Mp2kPlayer *pl, Mp2kTrack *t) {
	uint32_t tone = pl->tone + (uint32_t)arg(t) * 12;
	for (int i = 0; i < 12; ++i) t->tone[i] = mp2k_u8(tone + (uint32_t)i);
}

static void cmd_tempo(Mp2kPlayer *pl, Mp2kTrack *t) {
	pl->tempo_d = (uint16_t)(arg(t) * 2);
	pl->tempo_i = (uint16_t)((pl->tempo_d * pl->tempo_u) >> 8);
}

/* XCMD: a note's tone changed in place, its pseudo-echo set */
static void cmd_xcmd(Mp2kTrack *t) {
	uint8_t x = arg(t);
	switch (x) {
	case 1:
		for (int i = 0; i < 4; ++i) t->tone[4 + i] = arg(t);
		break;
	case 2: t->tone[0] = arg(t); break;
	case 4: case 5: case 6: case 7: t->tone[8 + x - 4] = arg(t); break;
	case 8: t->echo_volume = arg(t); break;
	case 9: t->echo_length = arg(t); break;
	case 10: t->tone[2] = arg(t); break;
	case 11: t->tone[3] = arg(t); break;
	default: cmd_fine(t); break;
	}
}

/* MEMACC acts on the game's RAM, which a screen of the engine's has none
 * of: its operands read past, its conditions false (no song of BN6's the
 * engine plays uses it) */
static void cmd_memacc(Mp2kTrack *t) {
	uint8_t op = arg(t);
	t->cmd += 2;
	if (op >= 6 && op <= 17) t->cmd += 4;
}

static void cmd_simple(Mp2kTrack *t, uint8_t cmd) {
	uint8_t v = arg(t);
	switch (cmd) {
	case 0xBA: t->priority = v; break;
	case 0xBC: t->key_shift = (int8_t)v; t->flags |= TRK_PITCHG; break;
	case 0xBE: t->vol = v; t->flags |= TRK_VOLCHG; break;
	case 0xBF: t->pan = (int8_t)(v - 0x40); t->flags |= TRK_VOLCHG; break;
	case 0xC0: t->bend = (int8_t)(v - 0x40); t->flags |= TRK_PITCHG; break;
	case 0xC1: t->bend_range = v; t->flags |= TRK_PITCHG; break;
	case 0xC2: t->lfo_speed = v; if (!v) clear_mod(t); break;
	case 0xC3: t->lfo_delay = v; break;
	case 0xC4: t->mod = v; if (!v) clear_mod(t); break;
	case 0xC5: if (t->mod_t != v) { t->mod_t = v; t->flags |= 0x0F; } break;
	case 0xC8: t->tune = (int8_t)(v - 0x40); t->flags |= TRK_PITCHG; break;
	default: break;
	}
}

/* one command of BN6's jump table (0xB1-0xCE) */
static void command(Mp2kPlayer *pl, Mp2kTrack *t, uint8_t cmd) {
	switch (cmd) {
	case 0xB2: cmd_goto(t); break;
	case 0xB3:
		if (t->pattern_level >= 3) { cmd_fine(t); break; }
		t->stack[t->pattern_level++] = t->cmd + 4;
		cmd_goto(t);
		break;
	case 0xB4: if (t->pattern_level) t->cmd = t->stack[--t->pattern_level]; break;
	case 0xB5: cmd_rept(t); break;
	case 0xB9: cmd_memacc(t); break;
	case 0xBB: cmd_tempo(pl, t); break;
	case 0xBD: cmd_voice(pl, t); break;
	case 0xCC: {
		/* PORT: a byte straight into a sound register */
		uint8_t reg = arg(t);
		gba_sound_write8(0x60 + reg, arg(t));
		break;
	}
	case 0xCD: cmd_xcmd(t); break;
	case 0xCE: mp2k_end_tie(t); break;
	case 0xBA: case 0xBC: case 0xBE: case 0xBF: case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC8:
		cmd_simple(t, cmd);
		break;
	default: cmd_fine(t); break;
	}
}

/* ---- a track's tick ---- */

/* its channels' gates counted down, a channel let go taken off */
static void gates(Mp2kTrack *t) {
	for (int c = t->chan; c != MP2K_NONE;) {
		Mp2kChan *ch = &M.chans[c];
		if (ch->status & CHN_ON) {
			if (ch->gate && --ch->gate == 0) ch->status |= CHN_STOP;
		} else mp2k_clear_chain(c);
		c = next_of(c);
	}
}

/* a track begun: its first 64 bytes cleared (Clear64byte), the defaults set */
static void track_begin(Mp2kTrack *t) {
	uint32_t cmd = t->cmd, stack[3], addr = t->addr;
	memcpy(stack, t->stack, sizeof stack);
	memset(t, 0, sizeof *t);
	t->cmd = cmd;
	memcpy(t->stack, stack, sizeof stack);
	t->addr = addr;
	t->chan = MP2K_NONE;
	t->flags = TRK_EXIST;
	t->bend_range = 2;
	t->vol_x = 64;
	t->lfo_speed = 22;
	t->tone[0] = 1;
}

/* the LFO's step: its triangle scaled by the depth, a change flagged */
static void lfo(Mp2kTrack *t) {
	if (!t->lfo_speed || !t->mod) return;
	if (t->lfo_delay_c) { --t->lfo_delay_c; return; }
	t->lfo_speed_c = (uint8_t)(t->lfo_speed_c + t->lfo_speed);
	int r = t->lfo_speed_c, m = (int8_t)(r - 64) < 0 ? (int8_t)r : 128 - r;
	m = (t->mod * m) >> 6;
	if ((uint8_t)(m ^ (uint8_t)t->mod_m) == 0) return;
	t->mod_m = (int8_t)m;
	t->flags |= t->mod_t == 0 ? TRK_PITCHG : TRK_VOLCHG;
}

/* a tick of a track: its commands until it waits; false where it ended */
static bool track_tick(int p, Mp2kTrack *t) {
	Mp2kPlayer *pl = &M.players[p];
	while (t->wait == 0) {
		uint8_t cmd = mp2k_u8(t->cmd);
		if (cmd < 0x80) cmd = t->running;
		else {
			++t->cmd;
			if (cmd >= 0xBD) t->running = cmd;
		}
		if (cmd >= 0xCF) mp2k_note(p, track_index(t), cmd - 0xCF);
		else if (cmd > 0xB0) {
			command(pl, t, cmd);
			if (t->flags == 0) return false;
		} else t->wait = mp2k_u8(M.clock_table + cmd - 0x80);
	}
	--t->wait;
	lfo(t);
	return true;
}

/* the frame's changes of volume and pitch into the track's channels */
static void track_apply(int p, Mp2kTrack *t) {
	mp2k_track_vol_pit(p, t);
	for (int c = t->chan; c != MP2K_NONE;) {
		Mp2kChan *ch = &M.chans[c];
		int next = next_of(c);
		if (!(ch->status & CHN_ON)) {
			mp2k_clear_chain(c);
			c = next;
			continue;
		}
		int type = ch->type & 7;
		if (t->flags & TRK_VOLCHG) {
			mp2k_chan_volume(ch, t);
			if (type) ch->modify |= 1;
		}
		if (t->flags & TRK_PITCHG) {
			int key = ch->key + t->key_m;
			if (key < 0) key = 0;
			if (type) {
				ch->frequency = mp2k_cgb_key_to_freq(type, key, t->pit_m);
				ch->modify |= 2;
			} else ch->frequency = mp2k_key_to_freq(ch->wav, key, t->pit_m);
		}
		c = next;
	}
	t->flags &= 0xF0;
}

/* ---- the fade ---- */

static void fade_end(Mp2kPlayer *pl) {
	for (int i = 0; i < pl->tracks; ++i) {
		Mp2kTrack *t = &M.tracks[pl->first + i];
		mp2k_track_stop(t);
		if (!(pl->fade_ov & 1)) t->flags = 0;
	}
	pl->status = pl->fade_ov & 1 ? pl->status | (uint32_t)PLAYER_PAUSE : (uint32_t)PLAYER_PAUSE;
	pl->fade_oi = 0;
}

/* FadeOutBody: every fade_oi frames the volume 16 of 256 lower (or higher) */
static void fade(Mp2kPlayer *pl) {
	if (!pl->fade_oi || --pl->fade_oc) return;
	pl->fade_oc = pl->fade_oi;
	if (pl->fade_ov & 2) {
		pl->fade_ov = (uint16_t)(pl->fade_ov + 16);
		if (pl->fade_ov > 0xFF) { pl->fade_ov = 0x100; pl->fade_oi = 0; }
	} else {
		pl->fade_ov = (uint16_t)(pl->fade_ov - 16);
		if ((int16_t)pl->fade_ov <= 0) { fade_end(pl); return; }
	}
	for (int i = 0; i < pl->tracks; ++i) {
		Mp2kTrack *t = &M.tracks[pl->first + i];
		if (!(t->flags & TRK_EXIST)) continue;
		t->vol_x = (uint8_t)(pl->fade_ov >> 2);
		t->flags |= TRK_VOLCHG;
	}
}

/* ---- a player's frame: MPlayMain ---- */

static void player_ticks(int p) {
	Mp2kPlayer *pl = &M.players[p];
	pl->tempo_c = (uint16_t)(pl->tempo_c + pl->tempo_i);
	while (pl->tempo_c >= 150) {
		uint32_t bits = 0;
		for (int i = 0; i < pl->tracks; ++i) {
			Mp2kTrack *t = &M.tracks[pl->first + i];
			if (!(t->flags & TRK_EXIST)) continue;
			bits |= 1u << i;
			gates(t);
			if (t->flags & TRK_START) track_begin(t);
			track_tick(p, t);
		}
		++pl->clock;
		if (!bits) { pl->status = (uint32_t)PLAYER_PAUSE; return; }
		pl->status = bits;
		pl->tempo_c = (uint16_t)(pl->tempo_c - 150);
	}
	for (int i = 0; i < pl->tracks; ++i) {
		Mp2kTrack *t = &M.tracks[pl->first + i];
		if ((t->flags & TRK_EXIST) && (t->flags & 0x0F)) track_apply(p, t);
	}
}

void mp2k_players_frame(void) {
	for (int p = 0; p < MP2K_PLAYERS; ++p) {
		Mp2kPlayer *pl = &M.players[p];
		if (pl->status & (uint32_t)PLAYER_PAUSE) continue;
		fade(pl);
		if (pl->status & (uint32_t)PLAYER_PAUSE) continue;
		player_ticks(p);
	}
}
