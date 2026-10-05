/* jc_uinput: virtual Joy-Cons on Linux's uinput, as Linux's hid-nintendo
 * reports a left and a right one (drivers/hid/hid-nintendo.c: the left's
 * BTN_TL, BTN_TL2, BTN_SELECT, BTN_THUMBL, BTN_DPAD_*, BTN_Z and, out of
 * the grip, BTN_TR/BTN_TR2 for SL/SR, its stick ABS_X/ABS_Y; the right's
 * BTN_EAST (A), BTN_SOUTH (B), BTN_NORTH (X), BTN_WEST (Y), BTN_TR, BTN_TR2,
 * BTN_START, BTN_THUMBR, BTN_MODE, BTN_TL/BTN_TL2 for SL/SR, its stick
 * ABS_RX/ABS_RY; sticks +-32767), for testing the Android app in the
 * emulator without Joy-Cons (issue #37): Android's input stack and SDL see
 * what a phone with them gives them. Built with the Android image's NDK
 * for the emulator's x86-64 (android/README.md):
 *   docker run --rm -u "$(id -u)" -v "$PWD:/src" -w /src cyberworld-android \
 *     /opt/android-sdk/ndk/27.2.12479018/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android26-clang \
 *     -O2 -o .build/jc_uinput tools/jc_uinput.c
 * and run as root (adb root) from /data/local/tmp, its devices there while
 * it runs. Commands, one a line on stdin:
 *   open L|R          make the device (Joy-Con (L) 057e:2006, (R) 057e:2007, Bluetooth)
 *   key L|R CODE 0|1  a button up or down (Linux's code: 314 BTN_SELECT, Minus)
 *   abs L|R AXIS V    an axis to V (0 ABS_X, 1 ABS_Y; the right's 3 and 4)
 *   tap L|R CODE MS   down, MS milliseconds, up
 *   sleep MS
 *   close L|R
 */
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

static int fd[2] = { -1, -1 };

static void nap(int ms) {
	struct timespec t = { ms / 1000, (long)(ms % 1000) * 1000000L };
	nanosleep(&t, NULL);
}

static void emit(int side, int type, int code, int value) {
	struct input_event e;
	memset(&e, 0, sizeof e);
	e.type = (unsigned short)type;
	e.code = (unsigned short)code;
	e.value = value;
	if (write(fd[side], &e, sizeof e) != (ssize_t)sizeof e) perror("write");
}

static void sync_ev(int side) { emit(side, EV_SYN, SYN_REPORT, 0); }

static void axis(int f, int code) {
	struct uinput_abs_setup a;
	memset(&a, 0, sizeof a);
	a.code = (unsigned short)code;
	a.absinfo.minimum = -32767;
	a.absinfo.maximum = 32767;
	a.absinfo.fuzz = 250;
	a.absinfo.flat = 500;
	if (ioctl(f, UI_ABS_SETUP, &a) < 0) perror("UI_ABS_SETUP");
}

static int make(int side) {
	static const int left[] = { BTN_TL, BTN_TL2, BTN_SELECT, BTN_THUMBL, BTN_DPAD_UP, BTN_DPAD_DOWN, BTN_DPAD_LEFT, BTN_DPAD_RIGHT, BTN_Z, BTN_TR, BTN_TR2 };
	static const int right[] = { BTN_EAST, BTN_SOUTH, BTN_NORTH, BTN_WEST, BTN_TR, BTN_TR2, BTN_START, BTN_THUMBR, BTN_MODE, BTN_TL, BTN_TL2 };
	int f = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (f < 0) { perror("/dev/uinput"); return -1; }
	ioctl(f, UI_SET_EVBIT, EV_KEY);
	ioctl(f, UI_SET_EVBIT, EV_ABS);
	const int *keys = side ? right : left;
	for (int i = 0; i < 11; ++i) ioctl(f, UI_SET_KEYBIT, keys[i]);
	ioctl(f, UI_SET_ABSBIT, side ? ABS_RX : ABS_X);
	ioctl(f, UI_SET_ABSBIT, side ? ABS_RY : ABS_Y);
	axis(f, side ? ABS_RX : ABS_X);
	axis(f, side ? ABS_RY : ABS_Y);
	struct uinput_setup u;
	memset(&u, 0, sizeof u);
	u.id.bustype = BUS_BLUETOOTH;
	u.id.vendor = 0x057e;
	u.id.product = side ? 0x2007 : 0x2006;
	u.id.version = 0x8001;
	snprintf(u.name, sizeof u.name, "%s", side ? "Joy-Con (R)" : "Joy-Con (L)");
	if (ioctl(f, UI_DEV_SETUP, &u) < 0) perror("UI_DEV_SETUP");
	if (ioctl(f, UI_DEV_CREATE) < 0) perror("UI_DEV_CREATE");
	return f;
}

int main(void) {
	char line[128];
	setvbuf(stdout, NULL, _IOLBF, 0);
	while (fgets(line, sizeof line, stdin)) {
		char cmd[16], s[4] = "L";
		int a = 0, b = 0;
		int n = sscanf(line, "%15s %3s %d %d", cmd, s, &a, &b);
		if (n < 1) continue;
		int side = s[0] == 'R';
		if (!strcmp(cmd, "sleep")) {
			int ms = 0;
			if (sscanf(line, "%*s %d", &ms) == 1) nap(ms);
			continue;
		}
		if (!strcmp(cmd, "open")) { fd[side] = make(side); printf("opened %c\n", side ? 'R' : 'L'); continue; }
		if (fd[side] < 0) { fprintf(stderr, "%c not open\n", side ? 'R' : 'L'); continue; }
		if (!strcmp(cmd, "key")) { emit(side, EV_KEY, a, b); sync_ev(side); }
		else if (!strcmp(cmd, "abs")) { emit(side, EV_ABS, a, b); sync_ev(side); }
		else if (!strcmp(cmd, "tap")) { emit(side, EV_KEY, a, 1); sync_ev(side); nap(b > 0 ? b : 100); emit(side, EV_KEY, a, 0); sync_ev(side); }
		else if (!strcmp(cmd, "close")) { ioctl(fd[side], UI_DEV_DESTROY); close(fd[side]); fd[side] = -1; }
		printf("%s", line);
	}
	return 0;
}
