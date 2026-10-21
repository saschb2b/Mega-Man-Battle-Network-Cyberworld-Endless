#include "online_stream.h"

#ifdef __ANDROID__
#include <jni.h>

static jobject stream;
static jmethodID open_stream, status, send_bytes, receive_bytes, random_bytes, close_stream;

static bool java_ok(JNIEnv *env) {
	if (!(*env)->ExceptionCheck(env)) return true;
	(*env)->ExceptionClear(env);
	return false;
}

static bool create_stream(JNIEnv *env) {
	jobject activity = SDL_AndroidGetActivity();
	if (!activity) return false;
	jclass activity_class = (*env)->GetObjectClass(env, activity);
	jmethodID create = (*env)->GetMethodID(env, activity_class, "onlineStream", "()Lio/github/saschb2b/cyberworldendless/OnlineStream;");
	jobject local = create ? (*env)->CallObjectMethod(env, activity, create) : NULL;
	(*env)->DeleteLocalRef(env, activity_class);
	(*env)->DeleteLocalRef(env, activity);
	if (!java_ok(env) || !local) return false;
	stream = (*env)->NewGlobalRef(env, local);
	jclass type = (*env)->GetObjectClass(env, local);
	open_stream = (*env)->GetMethodID(env, type, "open", "(Ljava/lang/String;)V");
	status = (*env)->GetMethodID(env, type, "status", "()I");
	send_bytes = (*env)->GetMethodID(env, type, "send", "([B)Z");
	receive_bytes = (*env)->GetMethodID(env, type, "receive", "(I)[B");
	random_bytes = (*env)->GetMethodID(env, type, "random", "(I)[B");
	close_stream = (*env)->GetMethodID(env, type, "close", "()V");
	(*env)->DeleteLocalRef(env, type);
	(*env)->DeleteLocalRef(env, local);
	return java_ok(env) && stream && open_stream && status && send_bytes && receive_bytes && random_bytes && close_stream;
}

bool online_stream_open(const char *url, SDL_atomic_t *stop, char *error, size_t size) {
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	if (!env || !create_stream(env)) { SDL_strlcpy(error, "Could not open Android networking", size); return false; }
	jstring address = (*env)->NewStringUTF(env, url);
	if (!address) return false;
	(*env)->CallVoidMethod(env, stream, open_stream, address);
	(*env)->DeleteLocalRef(env, address);
	uint32_t started = SDL_GetTicks();
	while (java_ok(env) && !SDL_AtomicGet(stop) && SDL_GetTicks() - started < 7000) {
		int state = (*env)->CallIntMethod(env, stream, status);
		if (!java_ok(env) || state < 0) break;
		if (state == 1) return true;
		SDL_Delay(10);
	}
	SDL_strlcpy(error, "Connection failed or timed out", size);
	return false;
}

bool online_stream_send(const uint8_t *data, size_t size) {
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	if (!env || !stream || size > 2048) return false;
	jbyteArray bytes = (*env)->NewByteArray(env, (jsize)size);
	if (!bytes) { java_ok(env); return false; }
	(*env)->SetByteArrayRegion(env, bytes, 0, (jsize)size, (const jbyte *)data);
	bool ok = java_ok(env) && (*env)->CallBooleanMethod(env, stream, send_bytes, bytes) != JNI_FALSE;
	(*env)->DeleteLocalRef(env, bytes);
	return java_ok(env) && ok;
}

static int read_bytes(uint8_t *data, size_t size, jmethodID method) {
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	if (!env || !stream) return -1;
	/* A stream may return fewer bytes than the caller's buffer holds. */
	if (size > 2048) size = 2048;
	jbyteArray bytes = (*env)->CallObjectMethod(env, stream, method, (jint)size);
	if (!java_ok(env) || !bytes) return -1;
	jsize count = (*env)->GetArrayLength(env, bytes);
	if (count >= 0 && (size_t)count <= size) (*env)->GetByteArrayRegion(env, bytes, 0, count, (jbyte *)data);
	else count = -1;
	(*env)->DeleteLocalRef(env, bytes);
	return java_ok(env) ? count : -1;
}

int online_stream_receive(uint8_t *data, size_t size) { return read_bytes(data, size, receive_bytes); }
bool online_stream_random(uint8_t *out, size_t size) { return read_bytes(out, size, random_bytes) == (int)size; }

void online_stream_close(void) {
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	if (!env || !stream) return;
	if (close_stream) (*env)->CallVoidMethod(env, stream, close_stream);
	java_ok(env);
	(*env)->DeleteGlobalRef(env, stream);
	stream = NULL;
}
/* (the stream is not one the worker can wait on here: looked at every
 * millisecond, a key sent at the next look; online_socket.h) */
void online_stream_wait(uint32_t ms) { SDL_Delay(ms ? 1 : 0); }
void online_stream_wake(void) {}
#endif
