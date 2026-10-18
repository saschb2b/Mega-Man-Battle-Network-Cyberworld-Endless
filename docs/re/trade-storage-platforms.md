# Trade storage: platform contract audit

Audited 18 October 2026. No platform advertises safe trading yet.
The native journal's [transaction contract](../ONLINE_TRADE.md) requires
acknowledged persistence of the source checkpoint, staged checkpoint,
receipt, decision and installation names before each protocol promise.
An API returning success, a read-back from the OS cache, and a successful
restart of one process are different evidence from surviving power loss.

## Current implementation and remaining evidence

| Platform | Current transaction path | Remaining requirement |
| --- | --- | --- |
| Linux / PortMaster | Checked file and directory `fsync`, exclusive process lock, no-follow reads | Native filesystem/device exercise; the Linux suite covers injected errors and process death, not hardware power cuts |
| Android | Same POSIX backend, app-private files | Exercise actual app storage and process restart; document filesystem/device flush behavior |
| macOS / iOS | Same POSIX backend | Add and test the stronger Apple flush contract below before treating this path as a power-loss guarantee |
| Windows | Unsupported; standalone API probe below | Establish persistence of all required namespace operations, or review a different authoritative storage format |
| Browser | Unsupported; ordinary saves use asynchronous IDBFS | Atomic transaction with acknowledged strict persistence, startup error handling and exclusion of other tabs/writers |
| New 3DS | Unsupported | Establish SDMC file and namespace persistence through libctru and verify on actual storage |

Unsupported trading storage retains normal offline saves. Copied pending
transaction artifacts remain blocked. The matrix does not authorize
replacing unsupported operations with successful no-ops.

## Windows: file flush is not the missing directory contract

`FlushFileBuffers` documents flushing the specified file and requires a
write-capable handle. Flushing a whole volume requires administrator
privileges, unsuitable for the game's per-user installation.
[Microsoft: FlushFileBuffers](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-flushfilebuffers).

`MoveFileExW` permits a directory move on the same drive and refuses an
existing directory destination. Its write-through flag begins with broad
wording about moving to disk, but its explicit flush guarantee describes
moves performed as copy-and-delete. The journal forbids that fallback and
needs ordered directory renames plus creation and removal persistence.
This audit therefore does **not** infer a general parent-directory barrier
from the flag.
[Microsoft: MoveFileExW](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw).

`FILE_FLAG_BACKUP_SEMANTICS` opens a directory handle, but Microsoft's list
of operations supported by directory handles does not list
`FlushFileBuffers`. That omission is a missing documented guarantee, not
proof that every Windows filesystem rejects the call.
[Microsoft: Directory Handles](https://learn.microsoft.com/en-us/windows/win32/fileio/obtaining-a-handle-to-a-directory).
File metadata is cached too; the file-caching documentation explains file
flush/write-through behavior without resolving every directory operation
used by this journal.
[Microsoft: File Caching](https://learn.microsoft.com/en-us/windows/win32/fileio/file-caching).

The ROM-free `tools/online_storage_windows_probe.c` uses wide-character
paths in a fresh disposable directory. Seventeen observations cover a
Unicode path, exclusive file handle, complete write, checked file flush,
read-back, file and directory moves, refusal of an occupied directory,
two directory-flush access modes and cleanup. Directory-flush observations
are reported even when unsupported; they never grant capability.
`tests/test_online_storage_windows.py` records every operation and rejects
failed required checks or leftover files. Every report explicitly says
`durability_certified: false`.

```sh
make WERROR=1 build/windows/online-storage-windows-probe.exe
# On Windows, using a MinGW-built binary:
python tests/test_online_storage_windows.py --binary build/windows/online-storage-windows-probe.exe
# Optional compatibility check on Linux; explicitly labelled Wine:
python3 tests/test_online_storage_windows.py --binary build/windows/online-storage-windows-probe.exe --wine
```

The MinGW build passed with warnings as errors. Wine passed all required
API checks, allowed a write-capable directory flush and rejected the
read-only flush. These are compatibility observations only. A native
Windows run and filesystem-specific interruption tests remain outstanding;
neither Wine nor native API success certifies power-loss behavior. The
probe does not edit the production backend or inspect player files.

A possible later design would keep complete authoritative generations
inside an already-established file and treat `savedata` as recoverable
materialization. That is not implemented or accepted here: bootstrap,
torn tails, old-generation refusal, ordinary later progress and cleanup
would all need an independent contract before replacing this journal.

## Apple: a stronger file flush is required

Apple's `fsync(2)` documentation distinguishes sending data to a drive
from draining the drive's buffered writes. It recommends `F_FULLFSYNC`
when an application requires strict write ordering. The current native
backend calls `fsync` only. A follow-up must check the stronger operation,
its failure behavior and directory handling on supported Apple systems;
Linux fault tests cannot certify it.
[Apple: fsync(2)](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/fsync.2.html).

## Browser: completion must reach the protocol

`web/play/app.js` currently schedules ordinary `FS.syncfs`, logs failures
and continues startup after a failed population callback. C's
`platform_persist` receives no result. The Emscripten API explicitly
reports asynchronous completion and errors through its callback.
[Emscripten: FS.syncfs](https://emscripten.org/docs/api_reference/Filesystem-API.html#FS.syncfs).
The project's pinned Emscripten 6.0.10 IDBFS implementation opens its
reconciliation transaction without a strict durability option and signals
completion from the transaction callback.
[Emscripten 6.0.10: IDBFS](https://github.com/emscripten-core/emscripten/blob/6.0.10/src/lib/libidbfs.js).

IndexedDB distinguishes strict, relaxed and default durability. Strict
asks for persistent-medium completion; relaxed permits OS-buffer
completion; default follows the browser's policy. A browser backend must
request and verify the strict mode it depends on, propagate transaction
failure before PREPARED/APPLIED, and test delayed callbacks, aborts, quota,
reload and competing tabs. An ordinary request-success event is not a
transaction commit.
[W3C: transaction durability](https://w3c.github.io/IndexedDB/#transaction-durability-hint).

## New 3DS: SDMC needs its own evidence

The checked libctru source maps file `fsync` to `FSFILE_Flush` and offers
`archiveCommitSaveData`, which calls the archive commit-save-data action.
The game's SDMC archive must not be assumed to share every save-data
archive guarantee. No verified SDMC parent-directory barrier was found
in the archive driver/header review.
[libctru: archive driver](https://github.com/devkitPro/libctru/blob/master/libctru/source/archive_dev.c),
[archive API](https://github.com/devkitPro/libctru/blob/master/libctru/include/3ds/archive.h).

The generic game compatibility helper currently removes a destination
before rename on 3DS and gives directory sync no acknowledged storage
result. A trade backend must avoid that helper, prove absent-destination
moves and archive persistence, and run interruption/recovery cases on the
actual SD card. No device installation or hardware test was performed in
this audit.
