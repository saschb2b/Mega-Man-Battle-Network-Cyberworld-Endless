/* Another game's songs in BN6: an MP2K song is a header (its tracks, its
 * voice group), the tracks' command streams, and the voice group's voices
 * with what they point at: samples, programmable waves, the sub-voices
 * and key tables of key splits, the 128 sub-voices of a drum kit. A song
 * takes along only the voices it selects, and a sample two songs share is
 * copied once. */
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

#define XSONG_AT    FREE_SONGS              /* (docs/EMULATION.md) */
#define XSONG_END   FREE_XNAVIS
#define SLOT_FIRST  BN6_SONGS_EMPTY_FIRST   /* BN6's empty song slots: a shared header of no tracks (docs/ROM_DATA.md), */
#define SLOT_LAST   0x3F    /* ... the per-run themes' (issue #82's shared allocations: the tracks' copied sounds above them, each BN5
                               sound at a fixed slot whichever track copies it, BN5_SOUNDS; a moved family's packed into its own
                               room, xvirus.c's XvSong, the Navis' copied here) */
#define SOUND_FIRST (SLOT_LAST + 1)          /* the tracks' sounds' fixed slots: from, */
#define SOUND_LAST  BN6_SONGS_EMPTY_LAST     /* ... to */
#define BGM_PLAYER  31      /* BN6's music player, 8 tracks */
#define TRACKS_MAX  8
#define SEQ_MAX     0x8000  /* a song's sequence: its tracks and header */
#define PTRS_MAX    512
#define BUS         0x08000000u

bool xsong_walk(const uint8_t *seq, uint32_t n, uint32_t base, const uint32_t *starts, int nstarts, uint32_t *ptrs, int *nptrs, int maxptrs,
                uint8_t programs[16]) {
	static uint8_t seen[SEQ_MAX / 8];
	if (n > SEQ_MAX) return false;
	memset(seen, 0, (n + 7) / 8);
	uint32_t todo[64];
	int ntodo = 0;
	for (int t = 0; t < nstarts && ntodo < 64; ++t) todo[ntodo++] = starts[t];
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
				if (at + 4 > n || *nptrs >= maxptrs || ntodo >= 64) return false;
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
	struct { int rom, song, slot; uint32_t header; } song[SOUND_LAST - SLOT_FIRST + 1];
	int nmoved;
	struct { int rom; uint32_t from, to; } moved[PTRS_MAX];
} X;

static const uint8_t *xr;   /* the ROM the song comes from */
static int xr_id;

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

static bool copy_voice(const uint8_t *v, uint8_t out[12], bool sub);

/* A key split's or drum kit's sub-voices (`n`), copied once; 0 where they
 * cannot be. */
static uint32_t copy_subs(uint32_t ptr, int n) {
	uint32_t off = ptr - BUS, to = moved(ptr);
	if (to) return to;
	if (ptr < BUS || off >= ROM_SIZE || (uint32_t)n * 12 > ROM_SIZE - off) return 0;
	uint8_t subs[128 * 12];
	for (int i = 0; i < n; ++i)
		if (!copy_voice(xr + off + (uint32_t)i * 12, subs + i * 12, true)) return 0;
	return keep(ptr, put(subs, (uint32_t)n * 12));
}

/* A voice (12 bytes: its type first, a pointer at +4 and, for a key
 * split, its key table at +8) with what it points at copied and its
 * pointers moved there; a key split or drum kit inside another, which the
 * sequencer never follows, with its pointers cleared. */
static bool copy_voice(const uint8_t *v, uint8_t out[12], bool sub) {
	memcpy(out, v, 12);
	uint8_t type = v[0];
	uint32_t p = get32(v + 4), to = 0;
	if (type & 0xC0) {
		if (sub) { memset(out + 4, 0, 8); return true; }
		if (type & 0x80) {
			to = copy_subs(p, 128);
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
		if (programs[p >> 3] >> (p & 7) & 1 && !copy_voice(xr + off + (uint32_t)p * 12, voices + p * 12, false)) return 0;
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

/* Song `song` of extra ROM `xrom` copied in after those before it, and
 * entered in slot `slot` (a theme's, SLOT_FIRST to SLOT_LAST), played by
 * BN6's music player, or (`own`: a sound's, SOUND_FIRST to SOUND_LAST) by
 * the players its game's entry names (as BN6's and BN5's players are the
 * same 32); false where it cannot be (the slot then stays BN6's empty
 * one). */
static bool copy_in(int xrom, int song, bool own, int slot) {
	if (slot < (own ? SOUND_FIRST : SLOT_FIRST) || slot > (own ? SOUND_LAST : SLOT_LAST) || X.nsongs >= (int)(sizeof X.song / sizeof *X.song) ||
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
	emu_write(BUS + R.layout->song_table + (uint32_t)slot * 8, entry, sizeof entry);
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
	if (X.nsongs && emu_read32(BUS + R.layout->song_table + (uint32_t)X.song[0].slot * 8) != X.song[0].header) memset(&X, 0, sizeof X);
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
		 * (docs/LIBERATION.md), after the rest, which keep their places:
		 * its own, its opening (BN5's before a fight, a guardian's prelude
		 * on its areas too), its win and a battle's won in one turn
		 * (docs/MULTIROM.md, BN5's music in a run) */
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

int xsong_copy_of(int slot, int *xrom) {
	for (int i = 0; i < X.nsongs; ++i)
		if (X.song[i].slot == slot) {
			if (xrom) *xrom = X.song[i].rom;
			return X.song[i].song;
		}
	return -1;
}

/* BN5's sounds that are no BN6 song, each at a slot of its own whichever
 * track's code plays it (issue #82's plan for its tracks and its
 * amendments; docs/ROM_DATA.md, BN6's empty song slots): copied by the
 * first install that needs it, found there by the others */
static const struct { uint16_t song; uint8_t slot; } BN5_SOUNDS[] = {
	{ 0xC0, 0x62 }, { 0xB7, 0x61 }, { 0xBA, 0x60 }, { 0x70, 0x5F }, { 0x117, 0x5E }, { 0x129, 0x5D }, { 0x16B, 0x5C }, { 0xE1, 0x5B },
	{ 0x115, 0x5A }, { 0x105, 0x59 }, { 0x10C, 0x58 }, { 0x10D, 0x57 }, { 0xCF, 0x56 }, { 0xBC, 0x55 }, { 0x6F, 0x54 }, { 0xE4, 0x53 },
	{ 0x12E, 0x52 }, { 0xA6, 0x51 }, { 0xC6, 0x50 }, { 0xBD, 0x4F }, { 0x10E, 0x4E }, { 0x119, 0x4D }, { 0x8E, 0x4C },
	/* (the Darkloids', from the first slot up: the chips' own go down from 0x4B) */
	{ 0xE3, 0x40 },
};

int xsong_sound_slot(int xrom, int song) {
	for (size_t i = 0; xrom == XROM_BN5_COLONEL_US && i < sizeof BN5_SOUNDS / sizeof *BN5_SOUNDS; ++i)
		if (BN5_SOUNDS[i].song == song) return BN5_SOUNDS[i].slot;
	return -1;
}

int xsong_install_sound(int xrom, int song, int fallback) {
	/* (the areas' songs and themes first, so each keeps its place) */
	xsong_install(xrom, -1, fallback);
	if (!R.layout || !R.layout->song_table || !R.data || !X.ready) return fallback;
	int slot = xsong_sound_slot(xrom, song);
	if (slot < 0 || slot_of(xrom, song) == slot) return slot < 0 ? fallback : slot;
	/* (copied only where the slot is still BN6's empty one, the player's ROM's entry; else another's install put it there) */
	uint32_t entry = R.layout->song_table + (uint32_t)slot * 8;
	if (emu_read32(BUS + entry) != get32(R.data + entry) || emu_read32(BUS + entry + 4) != get32(R.data + entry + 4)) return slot;
	return copy_in(xrom, song, true, slot) ? slot : fallback;
}

bool xsong_found(int xrom, int song, int slot) {
	/* (the areas' songs and themes first, so each keeps its place) */
	xsong_install(xrom, -1, -1);
	if (!R.layout || !R.layout->song_table || !X.ready || slot < SOUND_FIRST || slot > SOUND_LAST || xsong_sound_slot(xrom, song) != slot)
		return false;
	if (slot_of(xrom, song) == slot) return true;
	if (X.nsongs >= (int)(sizeof X.song / sizeof *X.song)) return false;
	X.song[X.nsongs++] = (__typeof__(X.song[0])){ xrom, song, slot, emu_read32(BUS + R.layout->song_table + (uint32_t)slot * 8) };
	return true;
}

static bool slot_taken(int slot) {
	for (int i = 0; i < X.nsongs; ++i)
		if (X.song[i].slot == slot) return true;
	return false;
}

int xsong_install_at(int xrom, int song, int slot) {
	xsong_install(xrom, -1, -1);
	if (!R.layout || !R.layout->song_table || !R.data || !X.ready || slot < SOUND_FIRST || slot > SOUND_LAST || xsong_sound_slot(xrom, song) != slot)
		return -1;
	/* (copied before, or noted where another track's install copied it: at its slot alone, one BN5 sound one slot) */
	int at = slot_of(xrom, song);
	if (at >= 0) return at == slot ? slot : -1;
	/* (the slot still BN6's empty one, as the player's ROM has it: no copy of anything there yet) */
	uint32_t entry = BUS + R.layout->song_table + (uint32_t)slot * 8;
	if (slot_taken(slot) || emu_read32(entry) != get32(R.data + (entry - BUS)) || emu_read32(entry + 4) != get32(R.data + (entry - BUS) + 4)) return -1;
	return copy_in(xrom, song, true, slot) ? slot : -1;
}
