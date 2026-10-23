/* MP2K, BN6's sound engine (mp2k.h): its start, its songs, its frame. */
#include "mp2k.h"

#include <string.h>

#include "bn6.h"
#include "gba_sound.h"
#include "mp2k_state.h"
#include "rom.h"

Mp2k M;

/* SoundInfo as BN6's start leaves it: SoundInit's master volume 15 and the
 * ring of 1584 samples, then m4aSoundMode with BN6's word (BN6_M4A_SOUND_MODE,
 * 0x0093040D): any reverb bits its reverb (13), 4 DirectSound channels, rate
 * 3, which is 176 samples a frame (romlab: docs/ROM_DATA.md, The sound's mix) */
#define BN6_PER_FRAME 176
#define BN6_RING 1584
#define BN6_MASTER_VOLUME 15

/* the ROM of index `rom` (mp2k_state.h), NULL where it is not there */
static const uint8_t *rom_data(int rom) {
	if (rom <= 0) return R.data;
	return rom - 1 < XROM_COUNT && XR[rom - 1].layout ? XR[rom - 1].data : NULL;
}

void mp2k_read(int rom) {
	M.data = rom_data(rom);
	if (!M.data) M.data = R.data;
}

uint8_t mp2k_u8(uint32_t addr) {
	return rom_is_ptr(addr) && M.data ? M.data[addr - 0x08000000] : 0;
}

uint32_t mp2k_u32(uint32_t addr) {
	return (uint32_t)mp2k_u8(addr) | (uint32_t)mp2k_u8(addr + 1) << 8 | (uint32_t)mp2k_u8(addr + 2) << 16 |
		(uint32_t)mp2k_u8(addr + 3) << 24;
}

uint8_t mp2k_lib_u8(uint32_t addr) {
	return rom_is_ptr(addr) && R.data ? R.data[addr - 0x08000000] : 0;
}

uint32_t mp2k_lib_u32(uint32_t addr) {
	return (uint32_t)mp2k_lib_u8(addr) | (uint32_t)mp2k_lib_u8(addr + 1) << 8 | (uint32_t)mp2k_lib_u8(addr + 2) << 16 |
		(uint32_t)mp2k_lib_u8(addr + 3) << 24;
}

/* the players as m4aSoundInit opens them, in BN6_MPLAYS's order (each
 * opened becomes the chain's head, so the first runs first) */
static void players_open(void) {
	int first = 0;
	for (int p = 0; p < MP2K_PLAYERS; ++p) {
		uint32_t row = BN6_MPLAYS + (uint32_t)p * 12;
		Mp2kPlayer *pl = &M.players[p];
		memset(pl, 0, sizeof *pl);
		pl->tracks = mp2k_lib_u8(row + 8);
		pl->check_priority = mp2k_lib_u8(row + 10);
		pl->status = (uint32_t)PLAYER_PAUSE;
		pl->first = first;
		for (int i = 0; i < pl->tracks && first + i < MP2K_TRACKS; ++i) {
			memset(&M.tracks[first + i], 0, sizeof M.tracks[0]);
			M.tracks[first + i].chan = MP2K_NONE;
			M.tracks[first + i].addr = mp2k_lib_u32(row + 4) + (uint32_t)i * 0x50;
		}
		first += pl->tracks;
	}
}

bool mp2k_init(void) {
	if (!R.data || !R.layout) return false;
	memset(&M, 0, sizeof M);
	M.data = R.data;
	M.song_table = 0x08000000 + R.layout->song_table;
	M.clock_table = BN6_M4A_CLOCK_TABLE;
	M.scale_table = BN6_M4A_SCALE_TABLE;
	M.freq_table = BN6_M4A_FREQ_TABLE;
	M.cgb_scale_table = BN6_M4A_CGB_SCALE_TABLE;
	M.cgb_freq_table = BN6_M4A_CGB_FREQ_TABLE;
	M.noise_table = BN6_M4A_NOISE_TABLE;
	M.cgb3_vol_table = BN6_M4A_CGB3_VOL_TABLE;
	uint32_t mode = mp2k_lib_u32(BN6_M4A_SOUND_MODE);
	M.reverb = mode & 0xFF ? mode & 0x7F : 0;
	M.max_chans = (uint8_t)(mode >> 8 & 0xF);
	if (M.max_chans > MP2K_DS_MAX) M.max_chans = MP2K_DS_MAX;
	M.master_volume = mode >> 12 & 0xF ? mode >> 12 & 0xF : BN6_MASTER_VOLUME;
	M.per_frame = BN6_PER_FRAME;
	M.dma_period = BN6_RING / BN6_PER_FRAME;
	M.dma_counter = M.dma_period;
	M.div_freq = (16777216u / 10512u + 1) >> 1;
	for (int i = 0; i < MP2K_CHANS; ++i) M.chans[i].track = M.chans[i].prev = M.chans[i].next = MP2K_NONE;
	/* (MPlayExtender: the PSG's channels' types and their NR51 bits) */
	static const uint8_t masks[MP2K_CGB] = { 0x11, 0x22, 0x44, 0x88 };
	for (int i = 0; i < MP2K_CGB; ++i) {
		CGB_CHAN(i)->type = (uint8_t)(i + 1);
		CGB_CHAN(i)->pan_mask = masks[i];
	}
	players_open();
	gba_sound_reset();
	M.ready = true;
	return true;
}

/* ---- songs ---- */

/* song `song` of ROM `rom`: the player its table names (BN5's 32 players
 * are BN6's, docs/ROM_DATA.md) and its header, the ROM left being read */
static Mp2kPlayer *player_of(int rom, int song, uint32_t *header) {
	if (!M.ready || song < 0 || song >= MP2K_SONGS || !rom_data(rom)) return NULL;
	mp2k_read(rom);
	uint32_t row = (rom ? 0x08000000 + XR[rom - 1].layout->song_table : M.song_table) + (uint32_t)song * 8;
	unsigned p = mp2k_u8(row + 4);
	if (header) *header = mp2k_u32(row);
	return p < MP2K_PLAYERS && rom_is_ptr(mp2k_u32(row)) ? &M.players[p] : NULL;
}

/* MPlayStart: an effects' player keeps a song of higher priority playing */
static void player_start(Mp2kPlayer *pl, int rom, uint32_t header) {
	Mp2kTrack *first = &M.tracks[pl->first];
	bool idle = (!pl->song || !(first->flags & TRK_START)) && (!(pl->status & 0xFFFF) || (pl->status & (uint32_t)PLAYER_PAUSE));
	if (pl->check_priority && !idle && pl->priority > mp2k_u8(header + 2)) return;
	pl->status = 0;
	pl->song = header;
	pl->rom = (uint8_t)rom;
	pl->loops = 0;
	pl->tone = mp2k_u32(header + 4);
	pl->priority = mp2k_u8(header + 2);
	pl->clock = 0;
	pl->tempo_d = pl->tempo_i = 150;
	pl->tempo_u = 0x100;
	pl->tempo_c = 0;
	pl->fade_oi = 0;
	int count = mp2k_u8(header);
	for (int i = 0; i < pl->tracks; ++i) {
		Mp2kTrack *t = &M.tracks[pl->first + i];
		mp2k_track_stop(t);
		t->flags = i < count ? TRK_EXIST | TRK_START : 0;
		t->chan = MP2K_NONE;
		if (i < count) t->cmd = mp2k_u32(header + 8 + 4 * (uint32_t)i);
	}
	/* (m4aSoundMode with the song's reverb, where it sets one) */
	uint8_t reverb = mp2k_u8(header + 3);
	if (reverb & 0x80) M.reverb = reverb & 0x7F;
}

void mp2k_song_start_in(int rom, int song) {
	uint32_t header = 0;
	Mp2kPlayer *pl = player_of(rom, song, &header);
	if (pl) player_start(pl, rom, header);
	mp2k_read(0);
}

void mp2k_song_start(int song) { mp2k_song_start_in(0, song); }

void mp2k_song_fade_in(int rom, int song, int speed) {
	Mp2kPlayer *pl = player_of(rom, song, NULL);
	mp2k_read(0);
	if (!pl) return;
	pl->fade_oc = pl->fade_oi = (uint16_t)speed;
	pl->fade_ov = 64 << 2;
}

void mp2k_song_fade(int song, int speed) { mp2k_song_fade_in(0, song, speed); }

/* m4aMPlayStop: the player paused, its tracks' channels off */
void mp2k_song_stop_in(int rom, int song) {
	Mp2kPlayer *pl = player_of(rom, song, NULL);
	mp2k_read(0);
	if (!pl) return;
	pl->status |= (uint32_t)PLAYER_PAUSE;
	for (int i = 0; i < pl->tracks; ++i) mp2k_track_stop(&M.tracks[pl->first + i]);
}

/* (a song begun and not yet ticked is on too: MPlayStart leaves its status
 * 0 till MPlayMain's first tick, which over the held game runs only while
 * the engine is busy) */
static bool player_on(const Mp2kPlayer *pl) {
	if (!pl->song || (pl->status & (uint32_t)PLAYER_PAUSE)) return false;
	return (pl->status & 0xFFFF) || (pl->tracks && (M.tracks[pl->first].flags & TRK_START));
}

bool mp2k_song_playing(int song) {
	const Mp2kPlayer *pl = player_of(0, song, NULL);
	mp2k_read(0);
	return pl && player_on(pl);
}

/* the player that plays song `song` of ROM `rom`, where that song is its song */
static const Mp2kPlayer *player_playing(int rom, int song) {
	uint32_t header = 0;
	const Mp2kPlayer *pl = player_of(rom, song, &header);
	mp2k_read(0);
	return pl && pl->song == header && pl->rom == rom && player_on(pl) ? pl : NULL;
}

int mp2k_song_loops(int rom, int song) {
	const Mp2kPlayer *pl = player_playing(rom, song);
	return pl ? pl->loops : -1;
}

int mp2k_song_levels(int rom, int song, uint8_t *out, int max) {
	const Mp2kPlayer *pl = player_playing(rom, song);
	if (!pl) return 0;
	int n = pl->tracks < max ? pl->tracks : max;
	for (int i = 0; i < n; ++i) {
		const Mp2kTrack *t = &M.tracks[pl->first + i];
		int level = 0;
		/* (its channels' loudness: the envelope by the louder side's volume) */
		for (int c = t->chan, guard = 0; c != MP2K_NONE && guard < MP2K_CHANS; c = M.chans[c].next, ++guard) {
			const Mp2kChan *ch = &M.chans[c];
			if (!(ch->status & CHN_ON)) continue;
			int side = ch->right > ch->left ? ch->right : ch->left;
			int env = c >= MP2K_DS_MAX ? ch->env * 17 : ch->env;
			int v = env * side / 255;
			if (v > level) level = v;
		}
		out[i] = (uint8_t)(level > 255 ? 255 : level);
	}
	return n;
}

bool mp2k_busy(void) {
	for (int p = 0; p < MP2K_PLAYERS; ++p)
		if (player_on(&M.players[p])) return true;
	for (int i = 0; i < MP2K_CHANS; ++i)
		if (M.chans[i].status & CHN_ON) return true;
	return false;
}

/* ---- a frame ---- */

/* m4aSoundVSync: the ring's DMA counted down, from the top again at 0 */
static void vsync(void) {
	if (--M.dma_counter == 0) M.dma_counter = M.dma_period;
}

void mp2k_frame(void) {
	if (!M.ready) return;
	vsync();
	/* (SoundMain: the slice this frame mixes, the ring's next) */
	M.slice = M.dma_counter > 1 ? M.dma_period - (M.dma_counter - 1) : 0;
	mp2k_players_frame();
	mp2k_cgb_frame();
	mp2k_mix_frame();
	mp2k_read(0);
}

const int8_t *mp2k_mix_right(void) { return &M.ring[0][M.slice * M.per_frame]; }
const int8_t *mp2k_mix_left(void) { return &M.ring[1][M.slice * M.per_frame]; }
int mp2k_per_frame(void) { return M.per_frame; }

void mp2k_quiet_set(uint32_t players) {
	uint32_t changed = players ^ M.quiet;
	M.quiet = players;
	/* (a change: the tracks that play of its players work their volume out again) */
	for (int p = 0; p < MP2K_PLAYERS; ++p) {
		if (!(changed >> p & 1)) continue;
		Mp2kPlayer *pl = &M.players[p];
		for (int i = 0; i < pl->tracks; ++i) {
			Mp2kTrack *t = &M.tracks[pl->first + i];
			if (t->flags & TRK_EXIST) t->flags |= TRK_VOLCHG;
		}
	}
}
