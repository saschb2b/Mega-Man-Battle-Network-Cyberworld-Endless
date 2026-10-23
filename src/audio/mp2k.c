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

uint8_t mp2k_u8(uint32_t addr) {
	return rom_is_ptr(addr) && R.data ? R.data[addr - 0x08000000] : 0;
}

uint32_t mp2k_u32(uint32_t addr) {
	return (uint32_t)mp2k_u8(addr) | (uint32_t)mp2k_u8(addr + 1) << 8 | (uint32_t)mp2k_u8(addr + 2) << 16 |
		(uint32_t)mp2k_u8(addr + 3) << 24;
}

/* the players as m4aSoundInit opens them, in BN6_MPLAYS's order (each
 * opened becomes the chain's head, so the first runs first) */
static void players_open(void) {
	int first = 0;
	for (int p = 0; p < MP2K_PLAYERS; ++p) {
		uint32_t row = BN6_MPLAYS + (uint32_t)p * 12;
		Mp2kPlayer *pl = &M.players[p];
		memset(pl, 0, sizeof *pl);
		pl->tracks = mp2k_u8(row + 8);
		pl->check_priority = mp2k_u8(row + 10);
		pl->status = (uint32_t)PLAYER_PAUSE;
		pl->first = first;
		for (int i = 0; i < pl->tracks && first + i < MP2K_TRACKS; ++i) {
			memset(&M.tracks[first + i], 0, sizeof M.tracks[0]);
			M.tracks[first + i].chan = MP2K_NONE;
			M.tracks[first + i].addr = mp2k_u32(row + 4) + (uint32_t)i * 0x50;
		}
		first += pl->tracks;
	}
}

bool mp2k_init(void) {
	if (!R.data || !R.layout) return false;
	memset(&M, 0, sizeof M);
	M.song_table = 0x08000000 + R.layout->song_table;
	M.clock_table = BN6_M4A_CLOCK_TABLE;
	M.scale_table = BN6_M4A_SCALE_TABLE;
	M.freq_table = BN6_M4A_FREQ_TABLE;
	M.cgb_scale_table = BN6_M4A_CGB_SCALE_TABLE;
	M.cgb_freq_table = BN6_M4A_CGB_FREQ_TABLE;
	M.noise_table = BN6_M4A_NOISE_TABLE;
	M.cgb3_vol_table = BN6_M4A_CGB3_VOL_TABLE;
	uint32_t mode = mp2k_u32(BN6_M4A_SOUND_MODE);
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

static Mp2kPlayer *player_of(int song, uint32_t *header) {
	if (!M.ready || song < 0 || song >= MP2K_SONGS) return NULL;
	uint32_t row = M.song_table + (uint32_t)song * 8;
	unsigned p = mp2k_u8(row + 4);
	if (header) *header = mp2k_u32(row);
	return p < MP2K_PLAYERS && rom_is_ptr(mp2k_u32(row)) ? &M.players[p] : NULL;
}

/* MPlayStart: an effects' player keeps a song of higher priority playing */
static void player_start(Mp2kPlayer *pl, uint32_t header) {
	Mp2kTrack *first = &M.tracks[pl->first];
	bool idle = (!pl->song || !(first->flags & TRK_START)) && (!(pl->status & 0xFFFF) || (pl->status & (uint32_t)PLAYER_PAUSE));
	if (pl->check_priority && !idle && pl->priority > mp2k_u8(header + 2)) return;
	pl->status = 0;
	pl->song = header;
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

void mp2k_song_start(int song) {
	uint32_t header = 0;
	Mp2kPlayer *pl = player_of(song, &header);
	if (pl) player_start(pl, header);
}

void mp2k_song_fade(int song, int speed) {
	Mp2kPlayer *pl = player_of(song, NULL);
	if (!pl) return;
	pl->fade_oc = pl->fade_oi = (uint16_t)speed;
	pl->fade_ov = 64 << 2;
}

static bool player_on(const Mp2kPlayer *pl) {
	return pl->song && (pl->status & 0xFFFF) && !(pl->status & (uint32_t)PLAYER_PAUSE);
}

bool mp2k_song_playing(int song) {
	const Mp2kPlayer *pl = player_of(song, NULL);
	return pl && player_on(pl);
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
}

const int8_t *mp2k_mix_right(void) { return &M.ring[0][M.slice * M.per_frame]; }
const int8_t *mp2k_mix_left(void) { return &M.ring[1][M.slice * M.per_frame]; }
int mp2k_per_frame(void) { return M.per_frame; }
