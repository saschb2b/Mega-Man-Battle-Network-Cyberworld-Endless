/* android_call.h. */
#include "android_call.h"

#ifdef __ANDROID__
#include <SDL.h>

bool android_call_begin(AndroidCall *c, const char *name, const char *sig) {
	c->env = (JNIEnv *)SDL_AndroidGetJNIEnv();
	c->activity = (jobject)SDL_AndroidGetActivity();
	c->m = NULL;
	if (!c->env || !c->activity) return false;
	jclass cls = (*c->env)->GetObjectClass(c->env, c->activity);
	c->m = (*c->env)->GetMethodID(c->env, cls, name, sig);
	(*c->env)->DeleteLocalRef(c->env, cls);
	if (!c->m && (*c->env)->ExceptionCheck(c->env)) (*c->env)->ExceptionClear(c->env);
	if (!c->m) (*c->env)->DeleteLocalRef(c->env, c->activity);
	return c->m != NULL;
}

bool android_call_end(AndroidCall *c) {
	bool ok = !(*c->env)->ExceptionCheck(c->env);
	if (!ok) (*c->env)->ExceptionClear(c->env);
	(*c->env)->DeleteLocalRef(c->env, c->activity);
	return ok;
}
#else
typedef int android_call_unused;
#endif
