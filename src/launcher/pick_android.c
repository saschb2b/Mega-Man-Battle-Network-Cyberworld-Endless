/* pick.h on Android: GameActivity's methods (android/.../GameActivity.java,
 * with RomLook.java), called on the game's thread with SDL's JNIEnv.
 * Android's own pickers, the folder's document tree (its access kept,
 * read and write, for the looks at later starts and the saves' copy) or
 * files, several at once; each .gba there told by its header's game code,
 * BN6's and BN5's checked by their SHA-1 and copied into the app's ROM
 * folder, where the game reads them; a .cwsave in the folder copied to
 * the data folder's found.cwsave, which the launcher offers back. */
#include "pick.h"

#ifdef __ANDROID__
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "backup.h"
#include "pick_saves.h"

/* a call into GameActivity: its environment, the activity and the method */
typedef struct {
	JNIEnv *env;
	jobject activity;
	jmethodID m;
} Call;

static bool call_begin(Call *c, const char *name, const char *sig) {
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

/* the call's end: false where it threw */
static bool call_end(Call *c) {
	bool ok = !(*c->env)->ExceptionCheck(c->env);
	if (!ok) (*c->env)->ExceptionClear(c->env);
	(*c->env)->DeleteLocalRef(c->env, c->activity);
	return ok;
}

/* name() returning a String: into out, false for null */
static bool call_string(const char *name, char *out, size_t n) {
	Call c;
	if (!call_begin(&c, name, "()Ljava/lang/String;")) return false;
	jstring r = (jstring)(*c.env)->CallObjectMethod(c.env, c.activity, c.m);
	if (n) out[0] = 0;
	if (r) {
		const char *u = (*c.env)->GetStringUTFChars(c.env, r, NULL);
		if (u) {
			snprintf(out, n, "%s", u);
			(*c.env)->ReleaseStringUTFChars(c.env, r, u);
		}
		(*c.env)->DeleteLocalRef(c.env, r);
	}
	return call_end(&c) && r != NULL;
}

/* name(int) returning a boolean */
static bool call_bool_int(const char *name, int arg) {
	Call c;
	if (!call_begin(&c, name, "(I)Z")) return false;
	jboolean v = (*c.env)->CallBooleanMethod(c.env, c.activity, c.m, (jint)arg);
	return call_end(&c) && v;
}

/* name(String) returning a boolean */
static bool call_bool_string(const char *name, const char *arg) {
	Call c;
	if (!call_begin(&c, name, "(Ljava/lang/String;)Z")) return false;
	jstring s = (*c.env)->NewStringUTF(c.env, arg);
	jboolean v = s ? (*c.env)->CallBooleanMethod(c.env, c.activity, c.m, s) : JNI_FALSE;
	if (s) (*c.env)->DeleteLocalRef(c.env, s);
	return call_end(&c) && v;
}

static bool busy;

unsigned pick_kinds(void) {
	static bool named;
	char name[128];
	if (!named && call_string("savesDevice", name, sizeof name)) { backup_set_device(name); named = true; }
	return PICK_FOLDER | PICK_FILES;
}

bool pick_open(int kind, int slot) {
	(void)slot;
	if (busy || !call_bool_int("romsPick", kind == PICK_FILES ? 1 : 0)) return false;
	busy = true;
	return true;
}

bool pick_busy(void) { return busy; }

/* romsResult's "status\nsaves\nwhat the look found", once a picker closed */
bool pick_done(PickResult *r) {
	char s[1200];
	if (!busy || !call_string("romsResult", s, sizeof s)) return false;
	busy = false;
	memset(r, 0, sizeof *r);
	char *nl = strchr(s, '\n'), *nl2 = nl ? strchr(nl + 1, '\n') : NULL;
	r->status = atoi(s) > 0 ? 1 : -1;
	r->saves = nl && nl[1] == '1';
	snprintf(r->text, sizeof r->text, "%s", nl2 ? nl2 + 1 : "");
	return true;
}

/* romsLook's "bits\nwhat it found" */
int pick_look(char *msg, size_t n) {
	char s[1200];
	if (!call_string("romsLook", s, sizeof s)) return -1;
	char *nl = strchr(s, '\n');
	if (nl && nl[1]) snprintf(msg, n, "%s", nl + 1);
	return atoi(s);
}

bool pick_folder(char *name, size_t n) { return call_string("romsFolder", name, n) && name[0]; }

bool pick_saves_put(const char *from) { return call_bool_string("savesPut", from); }

bool pick_saves_get(const char *to) { return call_bool_string("savesGet", to); }
bool saves_phone_place(char *out, size_t n) { return call_string("savesFolder", out, n) && out[0]; }
bool saves_phone_scope(char *out, size_t n) { return n > 1 && call_string("savesScope", out, n) && out[0] && strlen(out) + 1 < n; }

static bool saves_busy;
static int saves_kind;

static bool saves_dialog_open(int kind, const char *from) {
	Call c;
	if (busy || saves_busy || !call_begin(&c, "savesPick", "(ILjava/lang/String;)Z")) return false;
	jstring s = from ? (*c.env)->NewStringUTF(c.env, from) : NULL;
	jboolean v = (*c.env)->CallBooleanMethod(c.env, c.activity, c.m, (jint)kind, s);
	if (s) (*c.env)->DeleteLocalRef(c.env, s);
	saves_busy = call_end(&c) && v;
	saves_kind = kind;
	return saves_busy;
}

bool pick_saves_import(void) { return saves_dialog_open(0, NULL); }
bool pick_saves_export(const char *from) { return saves_dialog_open(1, from); }
bool pick_saves_choose_folder(void) { return saves_dialog_open(2, NULL); }
bool pick_saves_busy(void) { return saves_busy; }

bool pick_saves_done(PickResult *r) {
	char s[1200];
	if (!saves_busy || !call_string("savesResult", s, sizeof s)) return false;
	saves_busy = false;
	memset(r, 0, sizeof *r);
	r->status = atoi(s);
	r->path = saves_kind == 0;
	char *nl = strchr(s, '\n');
	snprintf(r->text, sizeof r->text, "%s", nl ? nl + 1 : "");
	return true;
}
#else
typedef int pick_android_unused;
#endif
