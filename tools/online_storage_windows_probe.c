/* Win32 storage API observations, not a power-loss durability certificate.
 * Run only in a fresh disposable directory. No game files or ROM required. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>

#define PATH_ROOM 2048
static unsigned failed;

static void observed(const char *name, bool ok, DWORD error, bool required) {
	printf("{\"operation\":\"%s\",\"ok\":%s,\"error\":%lu,\"required\":%s}\n",
		name, ok ? "true" : "false", (unsigned long)(ok ? 0 : error), required ? "true" : "false");
	if (required && !ok) ++failed;
}

static void checked(const char *name, BOOL ok) { observed(name, ok != 0, GetLastError(), true); }

static bool path_at(wchar_t out[PATH_ROOM], const wchar_t *root, const wchar_t *name) {
	int n = swprintf(out, PATH_ROOM, L"%ls\\%ls", root, name);
	return n > 0 && n < PATH_ROOM;
}

static bool same_bytes(const wchar_t *path, const uint8_t *expected, DWORD n) {
	HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, NULL);
	if (file == INVALID_HANDLE_VALUE) return false;
	uint8_t bytes[32];
	DWORD got = 0;
	bool ok = n <= sizeof bytes && ReadFile(file, bytes, sizeof bytes, &got, NULL) && got == n && !memcmp(bytes, expected, n);
	return CloseHandle(file) && ok;
}

static void file_operations(const wchar_t *root) {
	wchar_t temporary[PATH_ROOM], target[PATH_ROOM];
	if (!path_at(temporary, root, L"receipt.tmp") || !path_at(target, root, L"receipt.bin")) { ++failed; return; }
	const uint8_t bytes[] = { 0x43, 0x57, 0x01, 0x02, 0x03, 0x04 };
	HANDLE file = CreateFileW(temporary, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW,
		FILE_FLAG_WRITE_THROUGH | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
	observed("create-new-file", file != INVALID_HANDLE_VALUE, GetLastError(), true);
	if (file == INVALID_HANDLE_VALUE) return;
	HANDLE competing = CreateFileW(temporary, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, NULL);
	DWORD error = GetLastError();
	observed("exclusive-handle-refuses-second-open", competing == INVALID_HANDLE_VALUE && error == ERROR_SHARING_VIOLATION, error, true);
	if (competing != INVALID_HANDLE_VALUE) CloseHandle(competing);
	DWORD written = 0;
	bool wrote = WriteFile(file, bytes, sizeof bytes, &written, NULL) && written == sizeof bytes;
	observed("write-complete-file", wrote, GetLastError(), true);
	bool flushed = FlushFileBuffers(file) != 0;
	observed("flush-file", flushed, GetLastError(), true);
	checked("close-file", CloseHandle(file));
	bool moved = MoveFileExW(temporary, target, MOVEFILE_WRITE_THROUGH) != 0;
	observed("move-file-write-through", moved, GetLastError(), true);
	checked("readback-file", same_bytes(target, bytes, sizeof bytes));
	checked("delete-file", DeleteFileW(target));
}

static void directory_flush(const wchar_t *directory, DWORD rights, const char *name) {
	HANDLE file = CreateFileW(directory, rights, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
	DWORD error = GetLastError();
	bool ok = file != INVALID_HANDLE_VALUE;
	if (ok) { ok = FlushFileBuffers(file) != 0; error = GetLastError(); }
	observed(name, ok, error, false);
	if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
}

static void directory_operations(const wchar_t *root) {
	wchar_t stage[PATH_ROOM], target[PATH_ROOM], occupied[PATH_ROOM];
	if (!path_at(stage, root, L"stage") || !path_at(target, root, L"installed") || !path_at(occupied, root, L"occupied")) { ++failed; return; }
	checked("create-stage-directory", CreateDirectoryW(stage, NULL));
	checked("create-occupied-directory", CreateDirectoryW(occupied, NULL));
	bool refused = !MoveFileExW(stage, occupied, MOVEFILE_WRITE_THROUGH);
	observed("refuse-occupied-directory", refused, GetLastError(), true);
	bool moved = MoveFileExW(stage, target, MOVEFILE_WRITE_THROUGH) != 0;
	observed("move-directory-write-through", moved, GetLastError(), true);
	directory_flush(root, GENERIC_READ, "flush-directory-read-handle");
	directory_flush(root, GENERIC_READ | GENERIC_WRITE, "flush-directory-write-handle");
	checked("remove-installed-directory", RemoveDirectoryW(target));
	checked("remove-occupied-directory", RemoveDirectoryW(occupied));
}

int wmain(int argc, wchar_t **argv);
int wmain(int argc, wchar_t **argv) {
	if (argc != 2) { fprintf(stderr, "usage: online-storage-windows-probe FRESH-DIRECTORY\n"); return 2; }
	const wchar_t *root = argv[1];
	if (!CreateDirectoryW(root, NULL)) { fprintf(stderr, "probe directory must not exist (%lu)\n", (unsigned long)GetLastError()); return 2; }
	file_operations(root);
	directory_operations(root);
	checked("remove-probe-directory", RemoveDirectoryW(root));
	printf("{\"summary\":true,\"failed_checks\":%u,\"durability_certified\":false}\n", failed);
	return failed ? 1 : 0;
}
#else
int main(void) { fprintf(stderr, "this probe requires Win32\n"); return 2; }
#endif
