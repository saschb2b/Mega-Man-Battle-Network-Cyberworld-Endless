/* A call into GameActivity's Java from the game's thread, with SDL's
 * JNIEnv: the method looked up by its name and signature, then the call,
 * then its end, which clears what Java threw. Android's alone (the ROMs'
 * pickers, launcher/pick_android.c; the game's update,
 * update/update_android.c). */
#ifndef CW_ANDROID_CALL_H
#define CW_ANDROID_CALL_H

#ifdef __ANDROID__
#include <jni.h>
#include <stdbool.h>

typedef struct {
	JNIEnv *env;
	jobject activity;
	jmethodID m;
} AndroidCall;

/* GameActivity's `name` with signature `sig`: false where it has none
 * (nothing is left to end) */
bool android_call_begin(AndroidCall *c, const char *name, const char *sig);
/* the call's end: false where it threw */
bool android_call_end(AndroidCall *c);
#endif

#endif
