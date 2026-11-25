/* The game's music handed to the engine's MP2K and back (mp2k_carry.h). */
#include "mp2k_carry.h"

#include <string.h>

#include "bytes.h"
#include "mp2k.h"

static Mp2kPlayer *music(void) { return &M.players[MP2K_MUSIC]; }

/* whether track `t` is one of the music player's */
static bool music_track(int t) {
	const Mp2kPlayer *pl = music();
	return t != MP2K_NONE && t >= pl->first && t < pl->first + pl->tracks;
}

static int music_tracks(void) { return music()->tracks < MP2K_CARRY_TRACKS ? music()->tracks : MP2K_CARRY_TRACKS; }

/* the channels in turn, `s` from 0: the DirectSound ones in use, then the PSG's */
static int slots(void) { return M.max_chans + MP2K_CGB; }
static int slot_chan(int s) { return s < M.max_chans ? s : MP2K_DS_MAX + (s - M.max_chans); }
static bool is_cgb(int c) { return c >= MP2K_DS_MAX; }

/* channel `c`'s bytes in `g` */
static const uint8_t *bytes_in(const Mp2kGameMusic *g, int c) { return is_cgb(c) ? g->cgb[c - MP2K_DS_MAX] : g->ds[c]; }
static uint8_t *bytes_of(Mp2kGameMusic *g, int c) { return is_cgb(c) ? g->cgb[c - MP2K_DS_MAX] : g->ds[c]; }

/* the engine's channel `c` silent at once and of no track: no release, as
 * the other side sounds the note on */
static void cut(int c) {
	Mp2kChan *ch = &M.chans[c];
	if (ch->status && is_cgb(c)) mp2k_cgb_off(c - MP2K_DS_MAX + 1);
	ch->status = 0;
	ch->track = ch->prev = ch->next = MP2K_NONE;
}

/* the engine's music player stopped, every note of its tracks cut */
static void music_stop(void) {
	Mp2kPlayer *pl = music();
	for (int c = 0; c < MP2K_CHANS; ++c)
		if (music_track(M.chans[c].track)) cut(c);
	for (int i = 0; i < pl->tracks; ++i) {
		M.tracks[pl->first + i].flags = 0;
		M.tracks[pl->first + i].chan = MP2K_NONE;
	}
	pl->status = (uint32_t)PLAYER_PAUSE;
	pl->song = 0;
	pl->rom = 0;
}

uint32_t mp2k_carry_track_at(int i) { return M.ready && i >= 0 && i < music_tracks() ? mp2k_track_addr(music()->first + i) : 0; }

bool mp2k_carry_plays(const Mp2kGameMusic *g) {
	uint32_t status = get32(g->player + 4);
	return get32(g->player) && !(status & (uint32_t)PLAYER_PAUSE) && (status & 0xFFFF);
}

/* ---- the game's music taken ---- */

/* the channels taken that the game's track lists from `at` on, joined to
 * the engine's track `ti` in that order */
static void link_in(const Mp2kGameMusic *g, int ti, uint32_t at, const bool *taken, bool *linked) {
	Mp2kTrack *t = &M.tracks[ti];
	int last = MP2K_NONE;
	for (int guard = 0; at && guard < MP2K_CHANS; ++guard) {
		int c = mp2k_chan_at(at);
		if (c == MP2K_NONE) break;
		if (taken[c] && !linked[c] && M.chans[c].track == ti) {
			M.chans[c].prev = last;
			if (last == MP2K_NONE) t->chan = c;
			else M.chans[last].next = c;
			last = c;
			linked[c] = true;
		}
		uint32_t next = get32(bytes_in(g, c) + MP2K_CHAN_NEXT);
		if (next == at) break;
		at = next;
	}
}

bool mp2k_carry_in(const Mp2kGameMusic *g) {
	if (!M.ready) return false;
	music_stop();
	Mp2kPlayer *pl = music();
	mp2k_get_player(pl, g->player);
	pl->rom = MP2K_ROM_GAME;
	pl->loops = 0;
	for (int i = 0; i < music_tracks(); ++i) {
		mp2k_get_track(&M.tracks[pl->first + i], g->track[i]);
		M.tracks[pl->first + i].chan = MP2K_NONE;
	}
	M.reverb = g->reverb;
	M.c15 = g->c15;
	/* (each of the song's notes, where no sound of the engine's sounds there) */
	bool taken[MP2K_CHANS] = { false }, linked[MP2K_CHANS] = { false };
	for (int s = 0; s < slots(); ++s) {
		int c = slot_chan(s);
		const uint8_t *b = bytes_in(g, c);
		int t = mp2k_track_at(get32(b + MP2K_CHAN_TRACK));
		Mp2kChan *ch = &M.chans[c];
		if (!music_track(t) || (ch->status & CHN_ON)) continue;
		mp2k_clear_chain(c);
		mp2k_get_chan(ch, b, is_cgb(c));
		ch->track = t;
		ch->prev = ch->next = MP2K_NONE;
		ch->rom = MP2K_ROM_GAME;
		taken[c] = true;
	}
	for (int i = 0; i < music_tracks(); ++i) link_in(g, pl->first + i, get32(g->track[i] + MP2K_TRACK_CHANS), taken, linked);
	for (int c = 0; c < MP2K_CHANS; ++c) {
		if (taken[c] && !linked[c]) cut(c);
		else if (taken[c] && is_cgb(c)) mp2k_cgb_resound(c - MP2K_DS_MAX, NULL);
	}
	return true;
}

void mp2k_carry_hush(Mp2kGameMusic *g, const Mp2kRegs *to) {
	for (int c = MP2K_DS_MAX; c < MP2K_CHANS; ++c) {
		uint8_t *b = bytes_of(g, c);
		if (!(b[0] & CHN_ON) || !music_track(mp2k_track_at(get32(b + MP2K_CHAN_TRACK)))) continue;
		b[0] = 0;
		mp2k_cgb_off_on(c - MP2K_DS_MAX, to);
	}
}

/* ---- the song given back ---- */

/* the game's channel `c` off the list of `owner`, the track holding it
 * there (ClearChain): its neighbours in `g`, or the track's first, which
 * `writes` takes */
static void unlink_out(Mp2kGameMusic *g, int c, int owner, Mp2kWrite *writes, int *n) {
	uint8_t *b = bytes_of(g, c);
	uint32_t prev = get32(b + MP2K_CHAN_PREV), next = get32(b + MP2K_CHAN_NEXT);
	int p = mp2k_chan_at(prev), q = mp2k_chan_at(next);
	if (prev) {
		if (p != MP2K_NONE) put32(bytes_of(g, p) + MP2K_CHAN_NEXT, next);
	} else if (*n < MP2K_CARRY_WRITES) writes[(*n)++] = (Mp2kWrite){ mp2k_track_addr(owner) + MP2K_TRACK_CHANS, next };
	if (q != MP2K_NONE) put32(bytes_of(g, q) + MP2K_CHAN_PREV, prev);
	put32(b + MP2K_CHAN_TRACK, 0);
}

/* the game's channel `c` silent and of no track */
static void clear_out(uint8_t *b) {
	b[0] = 0;
	put32(b + MP2K_CHAN_TRACK, 0);
	put32(b + MP2K_CHAN_PREV, 0);
	put32(b + MP2K_CHAN_NEXT, 0);
}

/* channel `c` in `g` as the engine has it, where the song's: whether it
 * is given (false where the game's own sound keeps it, or neither side's
 * song is there); its PSG's registers on `to` */
static bool give_chan(Mp2kGameMusic *g, int c, const Mp2kRegs *to, Mp2kWrite *writes, int *n) {
	uint8_t *b = bytes_of(g, c);
	int owner = mp2k_track_at(get32(b + MP2K_CHAN_TRACK));
	bool others = owner != MP2K_NONE && !music_track(owner), ours = music_track(M.chans[c].track);
	bool sounded = b[0] & CHN_ON;
	if (others && (sounded || !ours)) return false;
	if (owner == MP2K_NONE && !ours) return false;
	if (others) unlink_out(g, c, owner, writes, n);
	if (ours) mp2k_put_chan(b, &M.chans[c], is_cgb(c));
	else clear_out(b);
	if (ours && is_cgb(c)) mp2k_cgb_resound(c - MP2K_DS_MAX, to);
	else if (sounded && is_cgb(c)) mp2k_cgb_off_on(c - MP2K_DS_MAX, to);
	return ours;
}

/* the music's track `i` in `g`, its list the channels given in the
 * engine's order */
static void track_out(Mp2kGameMusic *g, int i, const bool *given, bool *linked) {
	int ti = music()->first + i;
	uint32_t first = 0, last = 0;
	for (int c = M.tracks[ti].chan, guard = 0; c != MP2K_NONE && guard < MP2K_CHANS; ++guard) {
		int next = M.chans[c].next == c ? MP2K_NONE : M.chans[c].next;
		if (given[c] && !linked[c]) {
			uint32_t at = mp2k_chan_addr(c);
			uint8_t *b = bytes_of(g, c);
			put32(b + MP2K_CHAN_PREV, last);
			put32(b + MP2K_CHAN_NEXT, 0);
			if (last) put32(bytes_of(g, mp2k_chan_at(last)) + MP2K_CHAN_NEXT, at);
			else first = at;
			last = at;
			linked[c] = true;
		}
		c = next;
	}
	mp2k_put_track(g->track[i], &M.tracks[ti]);
	put32(g->track[i] + MP2K_TRACK_CHANS, first);
}

int mp2k_carry_out(Mp2kGameMusic *g, const Mp2kRegs *to, Mp2kWrite *writes) {
	Mp2kPlayer *pl = music();
	if (!M.ready || pl->rom != MP2K_ROM_GAME) return -1;
	int n = 0;
	bool given[MP2K_CHANS] = { false }, linked[MP2K_CHANS] = { false };
	for (int s = 0; s < slots(); ++s) given[slot_chan(s)] = give_chan(g, slot_chan(s), to, writes, &n);
	for (int i = 0; i < music_tracks(); ++i) track_out(g, i, given, linked);
	/* (one naming a track of the song's that its list leaves out: none) */
	for (int c = 0; c < MP2K_CHANS; ++c) {
		if (!given[c] || linked[c]) continue;
		clear_out(bytes_of(g, c));
		if (is_cgb(c)) mp2k_cgb_off_on(c - MP2K_DS_MAX, to);
	}
	mp2k_put_player(g->player, pl);
	music_stop();
	return n;
}

/* ---- the game's music followed ---- */

#define PLAYER_STATUS 0x04
#define PLAYER_CLOCK  0x0C
#define PLAYER_TEMPO_U 0x1E
#define PLAYER_FADE_OI 0x24
#define PLAYER_FADE_OV 0x28
#define FADE_START    (64 << 2)   /* m4aMPlayFadeOut's volume, fading out from all of it */

static bool paused(uint32_t status) { return status & (uint32_t)PLAYER_PAUSE; }

/* m4aMPlayStop */
static void follow_stop(Mp2kPlayer *pl) {
	pl->status |= (uint32_t)PLAYER_PAUSE;
	for (int i = 0; i < pl->tracks; ++i) mp2k_track_stop(&M.tracks[pl->first + i]);
}

void mp2k_carry_follow(const uint8_t *was, const uint8_t *now) {
	if (!M.ready) return;
	Mp2kPlayer *pl = music();
	uint32_t song = get32(now), status = get32(now + PLAYER_STATUS);
	uint32_t was_status = get32(was + PLAYER_STATUS);
	uint16_t oi = get16(now + PLAYER_FADE_OI), ov = get16(now + PLAYER_FADE_OV), u = get16(now + PLAYER_TEMPO_U);
	/* (MPlayStart: another header, or the clock from 0 again; a song begun
	 * shows no pause, whether it has ticked or not) */
	bool begun = song && !paused(status) && (song != get32(was) || get32(now + PLAYER_CLOCK) < get32(was + PLAYER_CLOCK));
	if (begun) {
		mp2k_read(MP2K_ROM_GAME);
		mp2k_player_start(pl, MP2K_ROM_GAME, song);
		mp2k_read(0);
	}
	if (pl->rom != MP2K_ROM_GAME || pl->song != song) return;
	bool fading = pl->fade_oi != 0;
	/* (m4aMPlayFadeOut, FadeOutTemporarily, FadeIn: begun as the game set it) */
	if (oi && (!get16(was + PLAYER_FADE_OI) || begun)) {
		pl->fade_oi = pl->fade_oc = oi;
		pl->fade_ov = (uint16_t)((ov & 2 ? 0 : FADE_START) | (ov & 3));
		fading = true;
	}
	/* (stopped, its tracks kept: a song's own end leaves none) */
	if (paused(status) && (status & 0xFFFF) && !paused(was_status) && !fading && !paused(pl->status)) follow_stop(pl);
	/* (m4aMPlayContinue, the fade in's too) */
	if (!paused(status) && paused(was_status) && !begun && paused(pl->status)) pl->status &= 0xFFFF;
	if (u != get16(was + PLAYER_TEMPO_U) || (begun && u != 0x100)) {
		pl->tempo_u = u;
		pl->tempo_i = (uint16_t)((pl->tempo_d * (uint32_t)u) >> 8);
	}
}

void mp2k_carry_drop(void) {
	if (M.ready && music()->rom == MP2K_ROM_GAME) music_stop();
}
