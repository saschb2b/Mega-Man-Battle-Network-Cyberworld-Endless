/* update_android.h: GameActivity's update* methods, called on the game's
 * thread with SDL's JNIEnv. */
#include "update_android.h"

#ifdef __ANDROID__
#include <stdio.h>

#include "android_call.h"

/* name() returning nothing */
static void call_void(const char *name) {
	AndroidCall c;
	if (!android_call_begin(&c, name, "()V")) return;
	(*c.env)->CallVoidMethod(c.env, c.activity, c.m);
	android_call_end(&c);
}

void update_android_tidy(void) { call_void("updateTidy"); }

void update_android_stop(void) { call_void("updateStop"); }

bool update_android_look(const char *url, const char *agent) {
	AndroidCall c;
	if (!android_call_begin(&c, "updateLook", "(Ljava/lang/String;Ljava/lang/String;)Z")) return false;
	jstring u = (*c.env)->NewStringUTF(c.env, url), a = (*c.env)->NewStringUTF(c.env, agent);
	jboolean v = u && a ? (*c.env)->CallBooleanMethod(c.env, c.activity, c.m, u, a) : JNI_FALSE;
	if (u) (*c.env)->DeleteLocalRef(c.env, u);
	if (a) (*c.env)->DeleteLocalRef(c.env, a);
	return android_call_end(&c) && v;
}

/* updateLooked's "1\nupdate.json" or "0\n", null while under way */
int update_android_looked(char *text, size_t n) {
	AndroidCall c;
	if (!android_call_begin(&c, "updateLooked", "()Ljava/lang/String;")) return 0;
	jstring r = (jstring)(*c.env)->CallObjectMethod(c.env, c.activity, c.m);
	int got = r ? 0 : -1;
	if (r) {
		const char *s = (*c.env)->GetStringUTFChars(c.env, r, NULL);
		if (s && s[0] == '1' && s[1] == '\n') {
			snprintf(text, n, "%s", s + 2);
			got = 1;
		}
		if (s) (*c.env)->ReleaseStringUTFChars(c.env, r, s);
		(*c.env)->DeleteLocalRef(c.env, r);
	}
	return android_call_end(&c) ? got : 0;
}

bool update_android_fetch(const char *url, const char *agent, long long size, const char *sha256) {
	AndroidCall c;
	if (!android_call_begin(&c, "updateFetch", "(Ljava/lang/String;Ljava/lang/String;JLjava/lang/String;)Z")) return false;
	jstring u = (*c.env)->NewStringUTF(c.env, url), a = (*c.env)->NewStringUTF(c.env, agent), h = (*c.env)->NewStringUTF(c.env, sha256);
	jboolean v = u && a && h ? (*c.env)->CallBooleanMethod(c.env, c.activity, c.m, u, a, (jlong)size, h) : JNI_FALSE;
	if (u) (*c.env)->DeleteLocalRef(c.env, u);
	if (a) (*c.env)->DeleteLocalRef(c.env, a);
	if (h) (*c.env)->DeleteLocalRef(c.env, h);
	return android_call_end(&c) && v;
}

int update_android_progress(void) {
	AndroidCall c;
	if (!android_call_begin(&c, "updateProgress", "()I")) return UPDATE_ANDROID_FAILED;
	jint v = (*c.env)->CallIntMethod(c.env, c.activity, c.m);
	return android_call_end(&c) ? (int)v : UPDATE_ANDROID_FAILED;
}
#else
typedef int update_android_unused;
#endif
