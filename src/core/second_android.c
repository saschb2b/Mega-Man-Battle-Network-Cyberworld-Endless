/* second_android.h. The game draws the second screen into `pixels` and
 * turns them into Android's order; GameActivity.secondFrame then has Java's
 * main thread copy them into a Bitmap (SecondScreen.java). The next picture
 * waits for that copy (nativeCopied), so none is drawn over while Java
 * reads it, and no picture allocates anything. */
#include "second_android.h"

#ifdef __ANDROID__
#include <jni.h>

#include <SDL.h>

#include "platform.h"

/* (second_size's largest: twice the 3DS's bottom screen each way) */
#define MAX_W (2 * SECOND_W)
#define MAX_H (2 * SECOND_H)

static uint32_t pixels[MAX_W * MAX_H];
static SDL_atomic_t display;   /* the display's size from Java's thread, w << 16 | h; 0 none */
static SDL_atomic_t copying;   /* a picture handed to Java, not yet copied */
static int drawn_w, drawn_h;
static bool dark = true;       /* what Java shows (or will, once its queue runs) is black */
static int said;               /* the display last logged */

/* The size drawn for a display of dw x dh: the largest whole scale the
 * 3DS's 320 x 240 still fits at, and the display's size at that scale
 * (the AYN Thor's lower screen, 1240 x 1080: 413 x 360, shown at 3x), so
 * the map is laid out as on the 3DS or roomier and fills the display in
 * whole pixels; on a display smaller than the 3DS's, its 320 x 240, shrunk
 * to fit. */
static void second_size(int dw, int dh, int *w, int *h) {
	int k = 1;
	while (dw / (k + 1) >= SECOND_W && dh / (k + 1) >= SECOND_H) ++k;
	*w = dw / k;
	*h = dh / k;
	if (*w < SECOND_W || *h < SECOND_H) { *w = SECOND_W; *h = SECOND_H; }
	if (*w > MAX_W) *w = MAX_W;
	if (*h > MAX_H) *h = MAX_H;
}

/* GameActivity.secondFrame(w, h) for the picture drawn, or secondDark(),
 * on the game's thread (SDL's, with its own JNIEnv); false once Java lacks
 * them or one threw, after which neither is called again */
static bool tell_java(bool picture) {
	static jmethodID frame, blank;
	static bool missing;
	JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	if (!env || !activity || missing) {
		if (env && activity) (*env)->DeleteLocalRef(env, activity);
		return false;
	}
	if (!frame) {
		jclass cls = (*env)->GetObjectClass(env, activity);
		frame = (*env)->GetMethodID(env, cls, "secondFrame", "(II)V");
		if (frame) blank = (*env)->GetMethodID(env, cls, "secondDark", "()V");
		(*env)->DeleteLocalRef(env, cls);
	}
	if (frame && blank) {
		if (picture) (*env)->CallVoidMethod(env, activity, frame, (jint)drawn_w, (jint)drawn_h);
		else (*env)->CallVoidMethod(env, activity, blank);
	}
	bool ok = frame && blank && !(*env)->ExceptionCheck(env);
	if ((*env)->ExceptionCheck(env)) (*env)->ExceptionClear(env);
	(*env)->DeleteLocalRef(env, activity);
	if (!ok) {
		missing = true;
		SDL_Log("second screen: GameActivity did not take its picture");
	}
	return ok;
}

uint32_t *second_android_begin(int *w, int *h) {
	int d = SDL_AtomicGet(&display);
	if (!d || SDL_AtomicGet(&copying)) return NULL;
	second_size(d >> 16, d & 0xFFFF, w, h);
	if (d != said) {
		said = d;
		SDL_Log("second screen %dx%d: the map drawn at %dx%d", d >> 16, d & 0xFFFF, *w, *h);
	}
	drawn_w = *w;
	drawn_h = *h;
	return pixels;
}

void second_android_end(bool on) {
	if (!on) {
		second_android_dark();
		return;
	}
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	/* (RGBA8888 keeps R in its top byte, the last in memory here; an
	 * Android bitmap's pixels are R, G, B, A in memory) */
	for (int i = 0, n = drawn_w * drawn_h; i < n; ++i) pixels[i] = SDL_Swap32(pixels[i]);
#endif
	SDL_AtomicSet(&copying, 1);
	if (tell_java(true)) dark = false;
	else SDL_AtomicSet(&copying, 0);
}

void second_android_dark(void) {
	if (dark) return;
	dark = true;
	tell_java(false);
}

/* Java's side (SecondScreen's natives), on its main thread. */
JNIEXPORT jobject JNICALL Java_io_github_saschb2b_cyberworldendless_SecondScreen_nativePixels(JNIEnv *env, jclass cls);
JNIEXPORT void JNICALL Java_io_github_saschb2b_cyberworldendless_SecondScreen_nativeDisplay(JNIEnv *env, jclass cls, jint w, jint h);
JNIEXPORT void JNICALL Java_io_github_saschb2b_cyberworldendless_SecondScreen_nativeCopied(JNIEnv *env, jclass cls);

/* the pixels, which Java copies each picture from (the first w * h * 4
 * bytes), once */
JNIEXPORT jobject JNICALL Java_io_github_saschb2b_cyberworldendless_SecondScreen_nativePixels(JNIEnv *env, jclass cls) {
	return (*env)->NewDirectByteBuffer(env, pixels, (jlong)sizeof pixels);
}

/* the size of the display's picture (its view's), 0 x 0 once it is gone */
JNIEXPORT void JNICALL Java_io_github_saschb2b_cyberworldendless_SecondScreen_nativeDisplay(JNIEnv *env, jclass cls, jint w, jint h) {
	SDL_AtomicSet(&display, w > 0 && h > 0 && w < 0x8000 && h < 0x10000 ? (int)(w << 16 | h) : 0);
}

/* the last picture copied: the next may be drawn */
JNIEXPORT void JNICALL Java_io_github_saschb2b_cyberworldendless_SecondScreen_nativeCopied(JNIEnv *env, jclass cls) {
	SDL_AtomicSet(&copying, 0);
}
#endif
