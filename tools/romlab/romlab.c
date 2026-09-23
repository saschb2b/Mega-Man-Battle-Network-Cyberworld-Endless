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
 */
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/core/log.h>
#include <mgba-util/vfs.h>
#include <mgba/core/blip_buf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		} else if (!strcmp(cmd, "peek32")) {
			unsigned addr = strtoul(strtok(NULL, " \t\r\n"), NULL, 16);
			printf("%08x=%08x\n", addr, core->rawRead32(core, addr, -1));
		}
		printf("ok %s\n", cmd); fflush(stdout);
	}
	core->deinit(core);
	return 0;
}
