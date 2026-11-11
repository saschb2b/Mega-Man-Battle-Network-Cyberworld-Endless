/* Another game's songs in BN6: an MP2K song is a header (its tracks, its
 * voice group), the tracks' command streams, and the voice group's voices
 * with what they point at: samples, programmable waves, the sub-voices
 * and key tables of key splits, the 128 sub-voices of a drum kit. A song
 * takes along only the voices it selects (and, from a game whose layout
 * says so, only the kit's keys its tracks play), and a sample two songs
 * share is copied once. */
#include "xsong.h"

#include <stdio.h>
#include <string.h>

#include "bn5_liberation.h"
#include "bn5_music.h"
#include "bn6.h"
#include "bytes.h"
#include "debug.h"
#include "emu.h"
#include "rom.h"
#include "song_table.h"

#define XSONG_AT    FREE_SONGS              /* (docs/engine/emulation/) */
#define XSONG_END   FREE_XNAVIS
#define SLOT_FIRST  BN6_SONGS_EMPTY_FIRST   /* BN6's empty song slots: a shared header of no tracks (docs/rom/), */
#define SLOT_LAST   0x31    /* ... the per-run themes' (issue #82's shared allocations: the tracks' copied sounds above them, each
                               sound at a fixed slot whichever track copies it, XSOUNDS; a moved family's packed into its own
                               room, xvirus.c's XvSong, the Navis' copied here; 0x34-0x3F BN4's families', 0x32 and 0x33 the pool
                               of BN4's Navis' sounds, xnavi_bn4_sounds.h; every theme copied today, 12 with BN5's and BN4's ROMs,
                               fits below them) */
#define SOUND_FIRST 0x34                     /* the tracks' sounds' fixed slots: from, */
#define SOUND_LAST  BN6_SONGS_EMPTY_LAST     /* ... to */
#define XSONG_GROWN 128                      /* the sounds copied into the grown table's slots (song_table.h) kept track of, at most */
#define BGM_PLAYER  31      /* BN6's music player, 8 tracks */
#define TRACKS_MAX  8
#define SEQ_MAX     0x8000  /* a song's sequence: its tracks and header */
#define PTRS_MAX    512
#define TODO_MAX    1024    /* the ways a walk has still to go: a track's start or a branch's each (BN4's 0x1C holds 227) */
#define BUS         0x08000000u

bool xsong_walk(const uint8_t *seq, uint32_t n, uint32_t base, const uint32_t *starts, int nstarts, uint32_t *ptrs, int *nptrs, int maxptrs,
                uint8_t programs[16]) {
	static uint8_t seen[SEQ_MAX / 8];
	if (n > SEQ_MAX) return false;
	memset(seen, 0, (n + 7) / 8);
	static uint32_t todo[TODO_MAX];
	int ntodo = 0;
	for (int t = 0; t < nstarts && ntodo < TODO_MAX; ++t) todo[ntodo++] = starts[t];
	while (ntodo) {
		uint32_t o = todo[--ntodo];
		while (o < n && !(seen[o >> 3] >> (o & 7) & 1)) {
			seen[o >> 3] |= (uint8_t)(1 << (o & 7));
			uint8_t c = seq[o];
			if (c == 0xB1) break;   /* FINE */
			if (c == 0xB9) return false;   /* (MEMACC, whose branches carry pointers too: none in the songs read so far) */
			/* GOTO, PATT, REPT (a count before its pointer): the pointer to
			 * move, and the way on there (a PATT returns, as the pattern
			 * written in place the first time runs on past its PEND) */
			if (c == 0xB2 || c == 0xB3 || c == 0xB5) {
				uint32_t at = o + 1 + (c == 0xB5);
				if (at + 4 > n || *nptrs >= maxptrs || ntodo >= TODO_MAX) return false;
				uint32_t to = get32(seq + at) - base;
				if (to >= n) return false;
				ptrs[(*nptrs)++] = at;
				todo[ntodo++] = to;
				if (c == 0xB2) break;
				o = at + 4;
				continue;
			}
			/* VOICE, and any more given it by running status (every other
			 * command's arguments are under 0x80 too, and skip so) */
			if (c == 0xBD) {
				for (++o; o < n && seq[o] < 0x80; ++o) programs[seq[o] >> 3] |= (uint8_t)(1 << (seq[o] & 7));
				continue;
			}
			++o;
		}
	}
	return true;
}

/* (a byte of the sequence, 0xFF past its end: no argument, no command run on) */
static uint8_t seq_at(const uint8_t *seq, uint32_t n, uint32_t o) { return o < n ? seq[o] : 0xFF; }

/* Where a track's command `cmd` (its arguments from `a`) leaves off: the
 * next command's offset, `n` where it ends the track (FINE, the commands
 * that end it as FINE does); its VOICE's program and its note's or EOT's
 * key noted (a note's velocity and extra gate after its key) */
static uint32_t track_step(const uint8_t *seq, uint32_t n, uint8_t cmd, uint32_t a, uint8_t programs[16], uint8_t keys[16]) {
	if (cmd <= 0xB0 || cmd == 0xB4) return a;   /* a wait, PEND (the pattern's run past, as xsong_walk takes it) */
	if (cmd == 0xBD) {
		uint8_t p = seq_at(seq, n, a);
		if (p < 0x80) programs[p >> 3] |= (uint8_t)(1 << (p & 7));
		return a + 1;
	}
	if ((cmd >= 0xBA && cmd <= 0xC5) || cmd == 0xC8) return a + 1;   /* one argument each */
	if (cmd == 0xCC) return a + 2;   /* PORT: a register and its byte */
	if (cmd == 0xCD) {   /* XCMD: its sub-command, then a pointer's four bytes or one */
		uint8_t x = seq_at(seq, n, a);
		return x == 1 ? a + 5 : x == 2 || (x >= 4 && x <= 11) ? a + 2 : n;
	}
	if (cmd < 0xCE) return n;
	uint8_t k = seq_at(seq, n, a);
	if (k >= 0x80) return a;
	keys[k >> 3] |= (uint8_t)(1 << (k & 7));
	++a;
	for (int i = 0; cmd >= 0xCF && i < 2 && seq_at(seq, n, a) < 0x80; ++i) ++a;
	return a;
}

bool xsong_track_keys(const uint8_t *seq, uint32_t n, uint32_t base, uint32_t start, uint8_t programs[16], uint8_t keys[16]) {
	static uint8_t seen[SEQ_MAX / 8];
	if (n > SEQ_MAX) return false;
	memset(seen, 0, (n + 7) / 8);
	static struct { uint32_t o; uint8_t running; } todo[TODO_MAX];
	int ntodo = 0;
	todo[ntodo++] = (__typeof__(todo[0])){ start, 0 };
	while (ntodo) {
		uint32_t o = todo[--ntodo].o;
		uint8_t running = todo[ntodo].running;
		while (o < n && !(seen[o >> 3] >> (o & 7) & 1)) {
			seen[o >> 3] |= (uint8_t)(1 << (o & 7));
			/* (a byte under 0x80 runs the last command from VOICE on again, its arguments from there) */
			uint8_t cmd = seq[o];
			uint32_t a = o + 1;
			if (cmd < 0x80) {
				if (!running) break;
				cmd = running;
				a = o;
			} else if (cmd >= 0xBD) running = cmd;
			/* GOTO, PATT, REPT: the way on there too */
			if (cmd == 0xB2 || cmd == 0xB3 || cmd == 0xB5) {
				uint32_t at = a + (cmd == 0xB5);
				if (at + 4 > n || ntodo >= TODO_MAX || get32(seq + at) - base >= n) return false;
				todo[ntodo++] = (__typeof__(todo[0])){ get32(seq + at) - base, running };
				o = cmd == 0xB2 ? n : at + 4;
				continue;
			}
			o = track_step(seq, n, cmd, a, programs, keys);
		}
	}
	return true;
}

/* What is copied into the core's ROM copy: whether it is, its end, the
 * slots and songs, and the pieces songs share, by their address in their
 * own ROM. */
static struct {
	bool ready;
	uint32_t next;
	uint8_t *buf;          /* (xsong_pack: what is put goes here, as it would stand from buf_at, room bytes, not into the core) */
	uint32_t buf_at, room;
	bool full;
	int nsongs;
	struct { int rom, song, slot; uint32_t header; } song[SOUND_LAST - SLOT_FIRST + 1 + XSONG_GROWN];   /* (and the grown table's) */
	int nmoved;
	struct { int rom; uint32_t from, to; } moved[PTRS_MAX];
} X;

static const uint8_t *xr;   /* the ROM the song comes from */
static int xr_id;
/* The song's drum kits' keys, a bit each per program, where its game's are
 * copied for those alone (XRomLayout.kit_keys_played): the keys of every
 * track whose VOICE selects the program */
static bool by_keys;
static uint8_t kit_keys[128][16];

/* `len` bytes into the free space, at a word; 0 where it is full. */
static uint32_t put(const uint8_t *src, uint32_t len) {
	uint32_t at = X.next;
	if (X.buf) {
		if (!len || at < X.buf_at || at - X.buf_at > X.room || len > X.room - (at - X.buf_at)) { X.full = true; return 0; }
		memcpy(X.buf + (at - X.buf_at), src, len);
		X.next = (at + len + 3) & ~3u;
		return at;
	}
	if (!len || at + len > XSONG_END) return 0;
	emu_write(at, src, len);
	X.next = (at + len + 3) & ~3u;
	return at;
}

static uint32_t moved(uint32_t from) {
	for (int i = 0; i < X.nmoved; ++i)
		if (X.moved[i].rom == xr_id && X.moved[i].from == from) return X.moved[i].to;
	return 0;
}

static uint32_t keep(uint32_t from, uint32_t to) {
	if (to && X.nmoved < PTRS_MAX) X.moved[X.nmoved++] = (__typeof__(X.moved[0])){ xr_id, from, to };
	return to;
}

/* `len` bytes at `ptr` of the song's ROM, copied once; 0 where they are
 * not in it or there is no room. */
static uint32_t copy_once(uint32_t ptr, uint32_t len) {
	uint32_t off = ptr - BUS, to = moved(ptr);
	if (to) return to;
	if (ptr < BUS || off >= ROM_SIZE || len > ROM_SIZE - off) return 0;
	return keep(ptr, put(xr + off, len));
}

/* A sample: its 16-byte header (the length at +12), then the length and
 * one more byte, as the mixer reads one past the end. */
static uint32_t copy_wave(uint32_t ptr) {
	uint32_t off = ptr - BUS;
	if (ptr < BUS || off > ROM_SIZE - 16) return 0;
	uint32_t len = get32(xr + off + 12);
	return len < 0x100000 ? copy_once(ptr, 16 + len + 1) : 0;
}

static bool copy_voice(const uint8_t *v, uint8_t out[12], bool sub, const uint8_t *keys);

/* A key split's or drum kit's sub-voices (`n`), copied once; 0 where they
 * cannot be. */
static uint32_t copy_subs(uint32_t ptr, int n) {
	uint32_t off = ptr - BUS, to = moved(ptr);
	if (to) return to;
	if (ptr < BUS || off >= ROM_SIZE || (uint32_t)n * 12 > ROM_SIZE - off) return 0;
	uint8_t subs[128 * 12];
	for (int i = 0; i < n; ++i)
		if (!copy_voice(xr + off + (uint32_t)i * 12, subs + i * 12, true, NULL)) return 0;
	return keep(ptr, put(subs, (uint32_t)n * 12));
}

/* A drum kit's sub-voices for the keys `keys` names alone, the others a
 * kit's type with no pointers (the sequencer plays no kit inside a kit:
 * nothing), copied for its song alone; 0 where it cannot be. */
static uint32_t copy_kit(uint32_t ptr, const uint8_t keys[16]) {
	uint32_t off = ptr - BUS;
	if (ptr < BUS || off >= ROM_SIZE || 128 * 12 > ROM_SIZE - off) return 0;
	uint8_t subs[128 * 12];
	memset(subs, 0, sizeof subs);
	for (int k = 0; k < 128; ++k)
		if (!(keys[k >> 3] >> (k & 7) & 1)) subs[k * 12] = 0x80;
		else if (!copy_voice(xr + off + (uint32_t)k * 12, subs + k * 12, true, NULL)) return 0;
	return put(subs, sizeof subs);
}

/* A voice (12 bytes: its type first, a pointer at +4 and, for a key
 * split, its key table at +8) with what it points at copied and its
 * pointers moved there; a key split or drum kit inside another, which the
 * sequencer never follows, with its pointers cleared; a drum kit for the
 * keys `keys` names alone where it is given (none: the voice left empty, as
 * no track plays it). */
static bool copy_voice(const uint8_t *v, uint8_t out[12], bool sub, const uint8_t *keys) {
	memcpy(out, v, 12);
	uint8_t type = v[0];
	uint32_t p = get32(v + 4), to = 0;
	if (type & 0xC0) {
		if (sub) { memset(out + 4, 0, 8); return true; }
		if (type & 0x80) {
			uint8_t any = 0;
			for (int k = 0; keys && k < 16; ++k) any |= keys[k];
			if (keys && !any) { memset(out, 0, 12); return true; }
			to = keys ? copy_kit(p, keys) : copy_subs(p, 128);
			put32(out + 4, to);
			return to != 0;
		}
		/* (a key split: the table, read at the note's key, names the
		 * sub-voices it needs) */
		uint32_t q = get32(v + 8), qo = q - BUS;
		if (q < BUS || qo > ROM_SIZE - 128) return false;
		int n = 0;
		for (int k = 0; k < 128; ++k) if (xr[qo + (uint32_t)k] >= n) n = xr[qo + (uint32_t)k] + 1;
		uint32_t table = copy_once(q, 128);
		if (n > 128 || !table || !(to = copy_subs(p, n))) return false;
		put32(out + 4, to);
		put32(out + 8, table);
		return true;
	}
	if ((type & 7) == 0) to = copy_wave(p);            /* a sample */
	else if ((type & 7) == 3) to = copy_once(p, 16);   /* a programmable wave */
	else return true;                                  /* square and noise: no pointer */
	put32(out + 4, to);
	return to != 0;
}

/* The song's voice group, the voices it selects copied (the rest left
 * empty); its address, 0 where it cannot be. */
static uint32_t copy_voices(uint32_t group, const uint8_t programs[16]) {
	uint32_t off = group - BUS;
	int last = -1;
	for (int p = 0; p < 128; ++p) if (programs[p >> 3] >> (p & 7) & 1) last = p;
	if (last < 0 || group < BUS || off >= ROM_SIZE || (uint32_t)(last + 1) * 12 > ROM_SIZE - off) return 0;
	uint8_t voices[128 * 12];
	memset(voices, 0, sizeof voices);
	for (int p = 0; p <= last; ++p)
		if (programs[p >> 3] >> (p & 7) & 1 && !copy_voice(xr + off + (uint32_t)p * 12, voices + p * 12, false, by_keys ? kit_keys[p] : NULL))
			return 0;
	return put(voices, (uint32_t)(last + 1) * 12);
}

/* The song copied: its header's address here, 0 where it cannot be. */
static uint32_t copy_song(uint32_t header) {
	uint32_t ho = header - BUS;
	if (header < BUS || ho > ROM_SIZE - 8) return 0;
	int nt = xr[ho];
	if (nt < 1 || nt > TRACKS_MAX || ho + 8 + 4 * (uint32_t)nt > ROM_SIZE) return 0;
	uint32_t lo = ho, hi = ho + 8 + 4 * (uint32_t)nt, starts[TRACKS_MAX];
	for (int t = 0; t < nt; ++t) {
		uint32_t tp = get32(xr + ho + 8 + 4 * (uint32_t)t);
		if (tp < BUS || tp - BUS >= ho) return 0;
		if (tp - BUS < lo) lo = tp - BUS;
	}
	if (hi - lo > SEQ_MAX) return 0;
	for (int t = 0; t < nt; ++t) starts[t] = get32(xr + ho + 8 + 4 * (uint32_t)t) - BUS - lo;
	/* (program 0: a track's voice before any VOICE) */
	uint8_t programs[16] = { 1 };
	static uint32_t ptrs[PTRS_MAX];
	int nptrs = 0;
	if (!xsong_walk(xr + lo, hi - lo, BUS + lo, starts, nt, ptrs, &nptrs, PTRS_MAX, programs)) return 0;
	/* (each track's keys to the kits its VOICEs select, and key 0, which a
	 * note before any key's plays: a track begins with it, Clear64byte's) */
	by_keys = XR[xr_id].layout->kit_keys_played;
	memset(kit_keys, 0, sizeof kit_keys);
	for (int t = 0; by_keys && t < nt; ++t) {
		uint8_t selects[16] = { 0 }, keys[16] = { 1 };
		if (!xsong_track_keys(xr + lo, hi - lo, BUS + lo, starts[t], selects, keys)) return 0;
		for (int p = 0; p < 128; ++p)
			for (int k = 0; selects[p >> 3] >> (p & 7) & 1 && k < 16; ++k) kit_keys[p][k] |= keys[k];
	}
	uint32_t voices = copy_voices(get32(xr + ho + 4), programs);
	if (!voices) return 0;
	/* the sequence where it will be: its pointers moved by as much */
	static uint8_t seq[SEQ_MAX];
	memcpy(seq, xr + lo, hi - lo);
	uint32_t at = X.next, shift = at - (BUS + lo);
	for (int i = 0; i < nptrs; ++i) put32(seq + ptrs[i], get32(seq + ptrs[i]) + shift);
	put32(seq + (ho - lo) + 4, voices);
	for (int t = 0; t < nt; ++t) put32(seq + (ho - lo) + 8 + 4 * (uint32_t)t, at + starts[t]);
	return put(seq, hi - lo) == at ? at + (ho - lo) : 0;
}

/* a sound's slot: one of the tracks' fixed ones among BN6's empty songs, or one of the grown table's */
static bool sound_slot(int slot) {
	return (slot >= SOUND_FIRST && slot <= SOUND_LAST) || (slot >= (int)SONG_GROWN_FIRST && song_slot_ok(slot));
}

/* whether a slot still holds what nothing copied into: BN6's entry of the player's ROM, a grown slot's the empty song */
static bool slot_empty(int slot) {
	uint32_t e[2], at = song_entry_bus(slot);
	song_entry_empty(slot, e);
	return at && emu_read32(at) == e[0] && emu_read32(at + 4) == e[1];
}

/* Song `song` of extra ROM `xrom` copied in after those before it, and
 * entered in slot `slot` (a theme's, SLOT_FIRST to SLOT_LAST), played by
 * BN6's music player, or (`own`: a sound's, SOUND_FIRST to SOUND_LAST) by
 * the players its game's entry names (as BN6's and BN5's players are the
 * same 32); false where it cannot be (the slot then stays BN6's empty
 * one). */
static bool copy_in(int xrom, int song, bool own, int slot) {
	if ((own ? !sound_slot(slot) : slot < SLOT_FIRST || slot > SLOT_LAST) || X.nsongs >= (int)(sizeof X.song / sizeof *X.song) ||
		xrom < 0 || xrom >= XROM_COUNT || !XR[xrom].data || !XR[xrom].layout->song_table || song < 0 ||
		(uint32_t)song * 8 + 8 > ROM_SIZE - XR[xrom].layout->song_table)
		return false;
	xr = XR[xrom].data;
	xr_id = xrom;
	uint32_t header = copy_song(get32(xr + XR[xrom].layout->song_table + (uint32_t)song * 8));
	if (!header) return false;
	uint8_t entry[8];
	const uint8_t *was = xr + XR[xrom].layout->song_table + (uint32_t)song * 8;
	put32(entry, header);
	put16(entry + 4, own ? get16(was + 4) : BGM_PLAYER);
	put16(entry + 6, own ? get16(was + 6) : BGM_PLAYER);
	emu_write(song_entry_bus(slot), entry, sizeof entry);
	X.song[X.nsongs++] = (__typeof__(X.song[0])){ xrom, song, slot, header };
	if (emu_debug_on())
		fprintf(stderr, "xsong: song %#x of %s in slot %#x, %u bytes of songs so far\n", song, XR[xrom].layout->name, slot, X.next - XSONG_AT);
	return true;
}

static int slot_of(int xrom, int song) {
	for (int i = 0; i < X.nsongs; ++i)
		if (X.song[i].rom == xrom && X.song[i].song == song) return X.song[i].slot;
	return -1;
}

int xsong_install(int xrom, int song, int fallback) {
	if (!R.layout || !R.layout->song_table) return fallback;
	/* (the core's ROM copy made anew: the songs went with the old) */
	if (X.nsongs && emu_read32(song_entry_bus(X.song[0].slot)) != X.song[0].header) memset(&X, 0, sizeof X);
	/* every song the other games' areas play, the first time, in their
	 * order: each lands where it did in the session a checkpoint's state
	 * was saved in, whose music player points into it, whichever area the
	 * run has come to first */
	if (!X.ready) {
		X.ready = true;
		X.next = XSONG_AT;
		for (int k = 0; k < XAREAS_MAX; ++k) {
			const NetAreaDef *x = net_area_def(NET_AREAS + k);
			if (x && x->xrom > 0 && x->xsong && slot_of(x->xrom - 1, x->xsong) < 0) copy_in(x->xrom - 1, x->xsong, false, SLOT_FIRST + X.nsongs);
		}
		/* then the battle themes of the games whose areas are there (after
		 * the areas' own, which a state saved before them points into) */
		for (int i = 0; i < XROM_COUNT; ++i)
			for (int b = 0; b < 2; ++b) {
				int theme = XR[i].data && XR[i].layout->nareas ? XR[i].layout->battle_songs[b] : 0;
				if (theme && slot_of(i, theme) < 0) copy_in(i, theme, false, SLOT_FIRST + X.nsongs);
			}
		/* then BN5's Liberation Missions' songs, a dark layer's board's
		 * (docs/game/liberation/), after the rest, which keep their places:
		 * its own, its opening (BN5's before a fight, a guardian's prelude
		 * on its areas too), its win and a battle's won in one turn
		 * (docs/multirom/music.md) */
		static const uint8_t board_songs[] = { BN5_SONG_LIBERATION, BN5_SONG_MISSION_OPEN, BN5_SONG_LIB_WON, BN5_SONG_ONE_TURN_WIN };
		for (size_t k = 0; XR[XROM_BN5_COLONEL_US].data && k < sizeof board_songs; ++k)
			if (slot_of(XROM_BN5_COLONEL_US, board_songs[k]) < 0) copy_in(XROM_BN5_COLONEL_US, board_songs[k], false, SLOT_FIRST + X.nsongs);
	}
	int slot = slot_of(xrom, song);
	return slot < 0 ? fallback : slot;
}

uint32_t xsong_pack(int xrom, int song, uint32_t at, uint8_t *out, uint32_t room) {
	if (xrom < 0 || xrom >= XROM_COUNT || !XR[xrom].data || !XR[xrom].layout || !XR[xrom].layout->song_table || song < 0 || (at & 3) ||
		(uint32_t)song * 8 + 8 > ROM_SIZE - XR[xrom].layout->song_table)
		return 0;
	/* (the music's copies and what they share kept aside: a pack shares nothing with them) */
	static __typeof__(X) kept;
	const uint8_t *kept_xr = xr;
	int kept_id = xr_id;
	kept = X;
	memset(&X, 0, sizeof X);
	X.next = at;
	X.buf = out;
	X.buf_at = at;
	X.room = room;
	memset(out, 0, room);
	xr = XR[xrom].data;
	xr_id = xrom;
	uint32_t header = copy_song(get32(xr + XR[xrom].layout->song_table + (uint32_t)song * 8));
	bool fit = !X.full;
	X = kept;
	xr = kept_xr;
	xr_id = kept_id;
	return fit ? header : 0;
}

int xsong_slot_holding(uint32_t header) {
	for (int n = 0; song_slot_ok(n); ++n)
		if (emu_read32(song_entry_bus(n)) == header) return n;
	return -1;
}

int xsong_copy_of(int slot, int *xrom) {
	for (int i = 0; i < X.nsongs; ++i)
		if (X.song[i].slot == slot) {
			if (xrom) *xrom = X.song[i].rom;
			return X.song[i].song;
		}
	return -1;
}

/* Another game's sounds that are no BN6 song, each at a slot of its own
 * whichever track's code plays it (issue #82's plan for its tracks and its
 * amendments; docs/rom/sound.md, BN6's empty song slots): copied by the
 * first install that needs it, found there by the others */
static const struct { uint8_t xrom; uint16_t song; uint16_t slot; } XSOUNDS[] = {
	{ XROM_BN5_COLONEL_US, 0xC0, 0x62 }, { XROM_BN5_COLONEL_US, 0xB7, 0x61 }, { XROM_BN5_COLONEL_US, 0xBA, 0x60 },
	{ XROM_BN5_COLONEL_US, 0x70, 0x5F }, { XROM_BN5_COLONEL_US, 0x117, 0x5E }, { XROM_BN5_COLONEL_US, 0x129, 0x5D },
	{ XROM_BN5_COLONEL_US, 0x16B, 0x5C }, { XROM_BN5_COLONEL_US, 0xE1, 0x5B }, { XROM_BN5_COLONEL_US, 0x115, 0x5A },
	{ XROM_BN5_COLONEL_US, 0x105, 0x59 }, { XROM_BN5_COLONEL_US, 0x10C, 0x58 }, { XROM_BN5_COLONEL_US, 0x10D, 0x57 },
	{ XROM_BN5_COLONEL_US, 0xCF, 0x56 }, { XROM_BN5_COLONEL_US, 0xBC, 0x55 }, { XROM_BN5_COLONEL_US, 0x6F, 0x54 },
	{ XROM_BN5_COLONEL_US, 0xE4, 0x53 }, { XROM_BN5_COLONEL_US, 0x12E, 0x52 }, { XROM_BN5_COLONEL_US, 0xA6, 0x51 },
	{ XROM_BN5_COLONEL_US, 0xC6, 0x50 }, { XROM_BN5_COLONEL_US, 0xBD, 0x4F }, { XROM_BN5_COLONEL_US, 0x10E, 0x4E },
	{ XROM_BN5_COLONEL_US, 0x119, 0x4D }, { XROM_BN5_COLONEL_US, 0x8E, 0x4C }, { XROM_BN5_COLONEL_US, 0x148, 0x4B },
	/* (the Darkloids', from the first slot up: the chips' own go down, PileDrvr's 0x148 at 0x4B, from 0x4A) */
	{ XROM_BN5_COLONEL_US, 0xE3, 0x40 },
	/* (Team ProtoMan's from 0x43 up, the Darkloids needing none past 0xE3: GyroMan's three, Meddy's; docs/game/team-protoman.md) */
	{ XROM_BN5_COLONEL_US, 0x103, 0x43 }, { XROM_BN5_COLONEL_US, 0x104, 0x44 }, { XROM_BN5_COLONEL_US, 0x14C, 0x45 },
	{ XROM_BN5_COLONEL_US, 0xE0, 0x46 },
	/* (BN5's battles' win songs, its Victory and Fanfare, on its areas' layers: src/director/win_song.c; the Darkloids' next from 0x43) */
	{ XROM_BN5_COLONEL_US, BN5_SONG_VICTORY, 0x41 }, { XROM_BN5_COLONEL_US, BN5_SONG_FANFARE, 0x42 },
	/* BN4's families' (tools/xrecipe_bn4.py's SOUND_SLOTS, by install order from the themes' top down: Spikey's, Shrimpy's,
	 * Walla's, Elmperor's, CirKill's; docs/multirom/bn4.md, Its sounds) */
	{ XROM_BN4_RED_SUN_US, 0xBC, 0x3F }, { XROM_BN4_RED_SUN_US, 0xC4, 0x3E }, { XROM_BN4_RED_SUN_US, 0x9F, 0x3D },
	{ XROM_BN4_RED_SUN_US, 0xC9, 0x3C }, { XROM_BN4_RED_SUN_US, 0xCA, 0x3B }, { XROM_BN4_RED_SUN_US, 0x70, 0x3A },
	{ XROM_BN4_RED_SUN_US, 0x9A, 0x39 }, { XROM_BN4_RED_SUN_US, 0xA6, 0x38 }, { XROM_BN4_RED_SUN_US, 0xB7, 0x37 },
	{ XROM_BN4_RED_SUN_US, 0x9D, 0x36 }, { XROM_BN4_RED_SUN_US, 0xFC, 0x35 }, { XROM_BN4_RED_SUN_US, 0xF9, 0x34 },
	/* BN4's chips' (tools/xchiprecipe_bn4.py's SOUND_SLOTS), in the grown table's slots (song_table.h, from SONG_GROWN_FIRST up,
	 * the owner's call 6 of docs/multirom/bn4-chips.md): PanlSht's shot's */
	{ XROM_BN4_RED_SUN_US, 0x1C6, SONG_GROWN_FIRST + 0 }, { XROM_BN4_RED_SUN_US, 0xBF, SONG_GROWN_FIRST + 1 },
	/* ... ShotStar's star's fall (its burst, 0x70, the families' slot 0x3A above) */
	{ XROM_BN4_RED_SUN_US, 0x146, SONG_GROWN_FIRST + 2 },
	/* ... Counter's hit (Repair's 0x9A the families' slot 0x39 above) */
	{ XROM_BN4_RED_SUN_US, 0xBE, SONG_GROWN_FIRST + 3 },
	/* ... WhitWeb's web */
	{ XROM_BN4_RED_SUN_US, 0xF7, SONG_GROWN_FIRST + 4 },
	/* ... Boomer's boomerang */
	{ XROM_BN4_RED_SUN_US, 0xAB, SONG_GROWN_FIRST + 5 },
	/* ... MokoRus's Moko */
	{ XROM_BN4_RED_SUN_US, 0xC7, SONG_GROWN_FIRST + 6 },
	/* ... SidBmbo's bamboo */
	{ XROM_BN4_RED_SUN_US, 0x8E, SONG_GROWN_FIRST + 7 },
};

int xsong_sound_slot(int xrom, int song) {
	for (size_t i = 0; i < sizeof XSOUNDS / sizeof *XSOUNDS; ++i)
		if (XSOUNDS[i].xrom == xrom && XSOUNDS[i].song == song) return XSOUNDS[i].slot;
	return -1;
}

int xsong_install_sound(int xrom, int song, int fallback) {
	/* (the areas' songs and themes first, so each keeps its place) */
	xsong_install(xrom, -1, fallback);
	if (!R.layout || !R.layout->song_table || !R.data || !X.ready) return fallback;
	int slot = xsong_sound_slot(xrom, song);
	if (slot < 0 || slot_of(xrom, song) == slot) return slot < 0 ? fallback : slot;
	/* (copied only where the slot is still BN6's empty one, the player's ROM's entry; else another's install put it there) */
	if (!slot_empty(slot)) return slot;
	return copy_in(xrom, song, true, slot) ? slot : fallback;
}

bool xsong_found(int xrom, int song, int slot) {
	/* (the areas' songs and themes first, so each keeps its place) */
	xsong_install(xrom, -1, -1);
	if (!R.layout || !R.layout->song_table || !X.ready || slot < SOUND_FIRST || slot > SOUND_LAST || xsong_sound_slot(xrom, song) != slot)
		return false;
	if (slot_of(xrom, song) == slot) return true;
	if (X.nsongs >= (int)(sizeof X.song / sizeof *X.song)) return false;
	X.song[X.nsongs++] = (__typeof__(X.song[0])){ xrom, song, slot, emu_read32(song_entry_bus(slot)) };
	return true;
}

static bool slot_taken(int slot) {
	for (int i = 0; i < X.nsongs; ++i)
		if (X.song[i].slot == slot) return true;
	return false;
}

int xsong_install_at(int xrom, int song, int slot) {
	xsong_install(xrom, -1, -1);
	if (!R.layout || !R.layout->song_table || !R.data || !X.ready || !sound_slot(slot) || xsong_sound_slot(xrom, song) != slot)
		return -1;
	/* (copied before, or noted where another track's install copied it: at its slot alone, one BN5 sound one slot) */
	int at = slot_of(xrom, song);
	if (at >= 0) return at == slot ? slot : -1;
	/* (the slot still BN6's empty one, as the player's ROM has it: no copy of anything there yet) */
	if (slot_taken(slot) || !slot_empty(slot)) return -1;
	return copy_in(xrom, song, true, slot) ? slot : -1;
}
