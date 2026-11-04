/* romlab: headless libmgba runner driven by a line-based script on stdin.
 *   run N                 advance N frames with the current keys
 *   keys A B UP ...       hold these keys (empty = release all)
 *   tap KEY [N]           hold KEY for N frames (default 2), then release and wait 8
 *   shot FILE.ppm         write the current frame
 *   dump ADDR LEN FILE    write LEN bytes of the bus starting at ADDR (hex)
 *   save FILE / load FILE save states
 *   poke8 ADDR VAL / poke32 ADDR VAL   write memory (hex)
 *   peek32 ADDR           print a word (hex)
 *   fill ADDR LEN VAL     write VAL over LEN bytes (hex; works on ROM, like a patch)
 *   audio N FILE          run N frames recording audio to a 16-bit stereo WAV
 *   rec N PREFIX          run N frames, writing PREFIX%05d.ppm per frame (numbered
 *                         across calls) and appending the IO registers and held
 *                         keys to PREFIX.io (0x60 register bytes + u16 keys each)
 *   pokes ADDR HEX        write a hex byte string starting at ADDR
 *   watch ADDR LEN        rec also appends these LEN bytes per frame to PREFIX.w
 *   mute MASK             channels left out of the sound (hex: 1 DMA A, 2 DMA B,
 *                         4-0x20 the PSG's 1-4)
 *   probe ADDR [MEM LEN]  a Thumb instruction of the ROM made a probe: each time it
 *                         runs, a line `probe ADDR fFRAME r0..r7 sp lr [MEM's LEN
 *                         bytes]` (hex), then the instruction as it was (a BKPT
 *                         patched in, the original fed to the CPU)
 *   writes ADDR LEN       each store by the CPU into these LEN bytes (hex): a line
 *                         `write ADDR=VALUE/SIZE at PC lr LR fFRAME` (a store
 *                         multiple, push or stmia, and DMA not seen)
 *   sound N PREFIX        run N frames: their sound as a 48 kHz WAV (PREFIX.wav),
 *                         MP2K's SoundInfo, its PSG channels, the music player
 *                         and its tracks after each (PREFIX.si) and the sound
 *                         registers (PREFIX.io): the MP2K player's differential
 */
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/core/log.h>
#include <mgba-util/vfs.h>
#include <mgba/core/blip_buf.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/audio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/internal/arm/arm.h>

static struct { uint32_t addr, mem, len; uint16_t op; } probes[256];
static int nprobes;
static struct mCore *probe_core;
static void (*probe_next)(struct ARMCore *, int);

/* a probe's BKPT: its line, then the instruction it stands on run as it was */
static void probe_hit(struct ARMCore *cpu, int imm) {
	uint32_t at = cpu->gprs[ARM_PC] - 4;
	if (imm >= nprobes || probes[imm].addr != at) { if (probe_next) probe_next(cpu, imm); return; }
	printf("probe %08x f%u", at, probe_core->frameCounter(probe_core));
	for (int r = 0; r < 8; ++r) printf(" %08x", cpu->gprs[r]);
	printf(" sp=%08x lr=%08x", cpu->gprs[ARM_SP], cpu->gprs[ARM_LR]);
	if (probes[imm].len) {
		printf(" ");
		for (uint32_t i = 0; i < probes[imm].len; ++i) printf("%02x", probe_core->rawRead8(probe_core, probes[imm].mem + i, -1));
	}
	printf("\n");
	ARMRunFake(cpu, probes[imm].op);
}

static uint32_t wwatch_addr, wwatch_len;
static void (*store8_next)(struct ARMCore *, uint32_t, int8_t, int *);
static void (*store16_next)(struct ARMCore *, uint32_t, int16_t, int *);
static void (*store32_next)(struct ARMCore *, uint32_t, int32_t, int *);

/* a store seen by `writes`: its line where it touches the watched bytes */
static void wwatch_seen(struct ARMCore *cpu, uint32_t a, uint32_t v, int size) {
	if (a + size <= wwatch_addr || a >= wwatch_addr + wwatch_len) return;
	uint32_t pc = cpu->gprs[ARM_PC] - (cpu->executionMode == MODE_THUMB ? 4 : 8);
	printf("write %08x=%x/%d at %08x lr %08x f%u\n", a, v, size, pc, cpu->gprs[ARM_LR], probe_core->frameCounter(probe_core));
}
static void wstore8(struct ARMCore *cpu, uint32_t a, int8_t v, int *c) { wwatch_seen(cpu, a, (uint8_t) v, 1); store8_next(cpu, a, v, c); }
static void wstore16(struct ARMCore *cpu, uint32_t a, int16_t v, int *c) { wwatch_seen(cpu, a & ~1u, (uint16_t) v, 2); store16_next(cpu, a, v, c); }
static void wstore32(struct ARMCore *cpu, uint32_t a, int32_t v, int *c) { wwatch_seen(cpu, a & ~3u, (uint32_t) v, 4); store32_next(cpu, a, v, c); }

static void nolog(struct mLogger *l, int c, enum mLogLevel lv, const char *f, va_list a) {(void)l;(void)c;(void)lv;(void)f;(void)a;}
static struct mLogger quiet = { .log = nolog };

static const char *names[] = {"A","B","SELECT","START","RIGHT","LEFT","UP","DOWN","R","L"};
static int keybit(const char *s) {
	for (int i = 0; i < 10; ++i) if (!strcmp(s, names[i])) return 1 << i;
	return 0;
}

int main(int argc, char **argv) {
	if (argc < 2) { fprintf(stderr, "usage: romlab ROM [SAVE] < script\n"); return 2; }
	mLogSetDefaultLogger(&quiet);
	struct mCore *core = mCoreFind(argv[1]);
	if (!core || !core->init(core)) return 1;
	mCoreInitConfig(core, NULL);
	unsigned w, h; core->desiredVideoDimensions(core, &w, &h);
	color_t *buf = calloc(w * h, sizeof(color_t));
	core->setVideoBuffer(core, buf, w);
	if (!mCoreLoadFile(core, argv[1])) return 1;
	if (argc > 2) {
		struct VFile *sv = VFileOpen(argv[2], O_RDWR);
		if (sv) core->loadSave(core, sv);
	}
	core->reset(core);
	char line[8192]; unsigned keys = 0; int recn = 0; unsigned watch_addr = 0, watch_len = 0;
	while (fgets(line, sizeof line, stdin)) {
		char *cmd = strtok(line, " \t\r\n");
		if (!cmd || cmd[0] == '#') continue;
		if (!strcmp(cmd, "run")) {
			int n = atoi(strtok(NULL, " \t\r\n"));
			core->setKeys(core, keys);
			for (int i = 0; i < n; ++i) core->runFrame(core);
		} else if (!strcmp(cmd, "keys")) {
			keys = 0; char *k;
			while ((k = strtok(NULL, " \t\r\n"))) keys |= keybit(k);
		} else if (!strcmp(cmd, "tap")) {
			int bit = keybit(strtok(NULL, " \t\r\n"));
			char *n = strtok(NULL, " \t\r\n"); int frames = n ? atoi(n) : 2;
			core->setKeys(core, keys | bit);
			for (int i = 0; i < frames; ++i) core->runFrame(core);
			core->setKeys(core, keys);
			for (int i = 0; i < 8; ++i) core->runFrame(core);
		} else if (!strcmp(cmd, "rec")) {
			int n = atoi(strtok(NULL, " \t\r\n"));
			const char *pre = strtok(NULL, " \t\r\n");
			char path[600]; snprintf(path, sizeof path, "%s.io", pre);
			FILE *io = fopen(path, "ab");
			core->setKeys(core, keys);
			for (int i = 0; i < n; ++i, ++recn) {
				core->runFrame(core);
				for (unsigned a = 0; a < 0x60; ++a) fputc(core->rawRead8(core, 0x04000000 + a, -1), io);
				fputc(keys & 0xFF, io); fputc(keys >> 8, io);
				if (watch_len) {
					snprintf(path, sizeof path, "%s.w", pre);
					FILE *wf = fopen(path, "ab");
					for (unsigned a = 0; a < watch_len; ++a) fputc(core->rawRead8(core, watch_addr + a, -1), wf);
					fclose(wf);
				}
				snprintf(path, sizeof path, "%s%05d.ppm", pre, recn);
				FILE *f = fopen(path, "wb");
				fprintf(f, "P6\n%u %u\n255\n", w, h);
				for (unsigned j = 0; j < w * h; ++j) {
					uint32_t c = buf[j];
					unsigned char px[3] = { c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF };
					fwrite(px, 1, 3, f);
				}
				fclose(f);
			}
			fclose(io);
		} else if (!strcmp(cmd, "watch")) {
			watch_addr = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			watch_len = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
		} else if (!strcmp(cmd, "pokes")) {
			unsigned addr = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			const char *hx = strtok(NULL, " \t\r\n");
			for (unsigned i = 0; hx[2 * i] && hx[2 * i + 1]; ++i) {
				unsigned v; sscanf(hx + 2 * i, "%2x", &v);
				core->rawWrite8(core, addr + i, -1, v);
			}
		} else if (!strcmp(cmd, "shot")) {
			FILE *f = fopen(strtok(NULL, " \t\r\n"), "wb");
			fprintf(f, "P6\n%u %u\n255\n", w, h);
			for (unsigned i = 0; i < w * h; ++i) {
				uint32_t c = buf[i];
				unsigned char px[3] = { c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF };
				fwrite(px, 1, 3, f);
			}
			fclose(f);
		} else if (!strcmp(cmd, "dump")) {
			unsigned addr = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			unsigned len = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			FILE *f = fopen(strtok(NULL, " \t\r\n"), "wb");
			for (unsigned i = 0; i < len; ++i) fputc(core->rawRead8(core, addr + i, -1), f);
			fclose(f);
		} else if (!strcmp(cmd, "save") || !strcmp(cmd, "load")) {
			struct VFile *vf = VFileOpen(strtok(NULL, " \t\r\n"), cmd[0] == 's' ? O_CREAT | O_TRUNC | O_RDWR : O_RDONLY);
			if (!vf) { fprintf(stderr, "state file error\n"); continue; }
			if (cmd[0] == 's') mCoreSaveStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
			else mCoreLoadStateNamed(core, vf, SAVESTATE_RTC);
			vf->close(vf);
		}
		else if (!strcmp(cmd, "audio")) {
			int n = atoi(strtok(NULL, " \t\r\n"));
			FILE *f = fopen(strtok(NULL, " \t\r\n"), "wb");
			unsigned rate = 32768;
			core->setAudioBufferSize(core, 4096);
			blip_t *L = core->getAudioChannel(core, 0), *Rr = core->getAudioChannel(core, 1);
			blip_set_rates(L, core->frequency(core), rate);
			blip_set_rates(Rr, core->frequency(core), rate);
			fwrite("RIFF\0\0\0\0WAVEfmt \x10\0\0\0\x01\0\x02\0", 1, 24, f);
			uint32_t v = rate; fwrite(&v, 4, 1, f); v = rate * 4; fwrite(&v, 4, 1, f);
			fwrite("\x04\0\x10\0data\0\0\0\0", 1, 12, f);
			core->setKeys(core, keys);
			uint32_t total = 0;
			for (int i = 0; i < n; ++i) {
				core->runFrame(core);
				int16_t tmp[4096 * 2];
				int got;
				while ((got = blip_samples_avail(L)) > 0) {
					if (got > 4096) got = 4096;
					blip_read_samples(L, tmp, got, 1);
					blip_read_samples(Rr, tmp + 1, got, 1);
					fwrite(tmp, 4, (size_t)got, f);
					total += (uint32_t)got * 4;
				}
			}
			fseek(f, 4, SEEK_SET); v = 36 + total; fwrite(&v, 4, 1, f);
			fseek(f, 40, SEEK_SET); fwrite(&total, 4, 1, f);
			fclose(f);
			printf("audio rate %u bytes %u\n", rate, total);
		}
		else if (!strcmp(cmd, "sound")) {
			/* sound N PREFIX: N frames, their sound as a 48 kHz WAV (PREFIX.wav, the
			 * output stage as the game hears it) and, after each frame, MP2K's
			 * SoundInfo whole (SI bytes from SIADDR: its channels and its mix
			 * ring) in PREFIX.si and the sound registers 0x04000060-0x040000AF in
			 * PREFIX.io, for the player's differential (docs/re) */
			int n = atoi(strtok(NULL, " \t\r\n"));
			const char *pre = strtok(NULL, " \t\r\n");
			const unsigned SIADDR = 0x030045C0, SI = 0xFB0, RATE = 48000;
			char path[600];
			snprintf(path, sizeof path, "%s.wav", pre);
			FILE *f = fopen(path, "wb");
			snprintf(path, sizeof path, "%s.si", pre);
			FILE *si = fopen(path, "wb");
			snprintf(path, sizeof path, "%s.io", pre);
			FILE *io = fopen(path, "wb");
			core->setAudioBufferSize(core, 4096);
			blip_t *L = core->getAudioChannel(core, 0), *Rr = core->getAudioChannel(core, 1);
			blip_set_rates(L, core->frequency(core), RATE);
			blip_set_rates(Rr, core->frequency(core), RATE);
			fwrite("RIFF\0\0\0\0WAVEfmt \x10\0\0\0\x01\0\x02\0", 1, 24, f);
			uint32_t v = RATE; fwrite(&v, 4, 1, f); v = RATE * 4; fwrite(&v, 4, 1, f);
			fwrite("\x04\0\x10\0data\0\0\0\0", 1, 12, f);
			core->setKeys(core, keys);
			uint32_t total = 0;
			for (int i = 0; i < n; ++i) {
				core->runFrame(core);
				int16_t tmp[4096 * 2];
				int got;
				while ((got = blip_samples_avail(L)) > 0) {
					if (got > 4096) got = 4096;
					blip_read_samples(L, tmp, got, 1);
					blip_read_samples(Rr, tmp + 1, got, 1);
					fwrite(tmp, 4, (size_t)got, f);
					total += (uint32_t)got * 4;
				}
				for (unsigned a = 0; a < SI; ++a) fputc(core->rawRead8(core, SIADDR + a, -1), si);
				/* (then the PSG's four channels, the music player and its eight tracks) */
				for (unsigned a = 0; a < 0x100; ++a) fputc(core->rawRead8(core, 0x03005600 + a, -1), si);
				for (unsigned a = 0; a < 0x40; ++a) fputc(core->rawRead8(core, 0x02010890 + a, -1), si);
				for (unsigned a = 0; a < 0x280; ++a) fputc(core->rawRead8(core, 0x02007EE8 + a, -1), si);
				/* (I/O by words: mGBA's byte and halfword reads of it mask what the
				 * registers hold) */
				for (unsigned a = 0x60; a < 0xB0; a += 4) {
					uint32_t w = core->rawRead32(core, 0x04000000 + a, -1);
					for (int k = 0; k < 4; ++k) fputc((w >> (8 * k)) & 0xFF, io);
				}
			}
			fseek(f, 4, SEEK_SET); v = 36 + total; fwrite(&v, 4, 1, f);
			fseek(f, 40, SEEK_SET); fwrite(&total, 4, 1, f);
			fclose(f); fclose(si); fclose(io);
			printf("sound %d frames, %u bytes of 48 kHz\n", n, total);
		}
		else if (!strcmp(cmd, "mute")) {
			/* mute MASK: channels left out of the sound (bit 0 DirectSound A,
			 * 1 B, 2-5 the PSG's 1-4), to hear one part alone */
			unsigned m = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			struct GBA *gba = core->board;
			gba->audio.forceDisableChA = m & 1;
			gba->audio.forceDisableChB = m >> 1 & 1;
			for (int i = 0; i < 4; ++i) gba->audio.psg.forceDisableCh[i] = m >> (2 + i) & 1;
		}
		else if (!strcmp(cmd, "fill")) {
			unsigned addr = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			unsigned len = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			unsigned val = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			for (unsigned i = 0; i < len; ++i) core->rawWrite8(core, addr + i, -1, val);
		}
		else if (!strcmp(cmd, "poke8") || !strcmp(cmd, "poke32")) {
			unsigned addr = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			unsigned val = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			if (cmd[4] == '8') core->rawWrite8(core, addr, -1, val);
			else core->rawWrite32(core, addr, -1, val);
		} else if (!strcmp(cmd, "probe")) {
			struct ARMCore *cpu = core->cpu;
			char *m = NULL, *n = NULL;
			unsigned addr = strtoul(strtok(NULL, " \t\r\n"), NULL, 16) & ~1u;
			if ((m = strtok(NULL, " \t\r\n"))) n = strtok(NULL, " \t\r\n");
			if (nprobes < 256) {
				if (!probe_core) { probe_core = core; probe_next = cpu->irqh.bkpt16; cpu->irqh.bkpt16 = probe_hit; }
				probes[nprobes].addr = addr;
				probes[nprobes].mem = m ? strtoul(m, NULL, 16) : 0;
				probes[nprobes].len = n ? strtoul(n, NULL, 16) : 0;
				probes[nprobes].op = core->rawRead16(core, addr, -1);
				core->rawWrite16(core, addr, -1, 0xBE00 | nprobes);
				++nprobes;
			}
		} else if (!strcmp(cmd, "writes")) {
			struct ARMCore *cpu = core->cpu;
			wwatch_addr = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			wwatch_len = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			probe_core = core;
			if (!store8_next) {
				store8_next = cpu->memory.store8; store16_next = cpu->memory.store16; store32_next = cpu->memory.store32;
				cpu->memory.store8 = wstore8; cpu->memory.store16 = wstore16; cpu->memory.store32 = wstore32;
			}
		} else if (!strcmp(cmd, "peek32")) {
			unsigned addr = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			printf("%08x=%08x\n", addr, core->rawRead32(core, addr, -1));
		}
		printf("ok %s\n", cmd); fflush(stdout);
	}
	core->deinit(core);
	return 0;
}
