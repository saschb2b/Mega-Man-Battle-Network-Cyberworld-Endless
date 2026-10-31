#include "network_prefs.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
EM_JS(int, identity_random, (uint8_t *out), {
	if (!globalThis.crypto || !globalThis.crypto.getRandomValues) return 0;
	const bytes = new Uint8Array(16);
	globalThis.crypto.getRandomValues(bytes);
	HEAPU8.set(bytes, out);
	return 1;
});
#elif defined(_WIN32)
#include <windows.h>
#include <string.h>
#elif defined(__3DS__)
#include <3ds.h>
#elif defined(__SWITCH__)
#include <switch/types.h>
#include <switch/kernel/random.h>
#elif defined(__vita__)
#include <psp2/kernel/rng.h>
#elif defined(CW_PS4)
#include <unistd.h>
#else
#include <stdio.h>
#endif

bool network_identity(uint8_t out[NETWORK_ID_BYTES]) {
#if defined(__EMSCRIPTEN__)
	return identity_random(out) != 0;
#elif defined(_WIN32)
	typedef BOOLEAN (WINAPI *RandomBytes)(PVOID, ULONG);
	HMODULE library = LoadLibraryW(L"advapi32.dll");
	if (!library) return false;
	FARPROC address = GetProcAddress(library, "SystemFunction036");
	RandomBytes random_bytes;
	_Static_assert(sizeof random_bytes == sizeof address, "Windows function pointer size");
	memcpy(&random_bytes, &address, sizeof random_bytes);
	bool ok = random_bytes && random_bytes(out, NETWORK_ID_BYTES);
	FreeLibrary(library);
	return ok;
#elif defined(__3DS__)
	if (R_FAILED(psInit())) return false;
	bool ok = R_SUCCEEDED(PS_GenerateRandomBytes(out, NETWORK_ID_BYTES));
	psExit();
	return ok;
#elif defined(__SWITCH__)
	/* (the kernel's random numbers: the Switch has no /dev/urandom) */
	randomGet(out, NETWORK_ID_BYTES);
	return true;
#elif defined(__vita__)
	return sceKernelGetRandomNumber(out, NETWORK_ID_BYTES) >= 0;
#elif defined(CW_PS4)
	/* (the kernel's random bytes: the PS4's sandbox shows no /dev/urandom) */
	return getentropy(out, NETWORK_ID_BYTES) == 0;
#else
	FILE *file = fopen("/dev/urandom", "rb");
	if (!file) return false;
	bool ok = fread(out, NETWORK_ID_BYTES, 1, file) == 1;
	fclose(file);
	return ok;
#endif
}
