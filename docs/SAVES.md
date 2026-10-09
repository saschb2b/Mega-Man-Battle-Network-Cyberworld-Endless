# Portable saves

SAVES on the title moves one profile and its checkpoint between the
project's systems (issue #114). The player exports a `.cwsave`, takes it
to another device, and imports it there. The receiving screen compares
both copies before the player chooses one. The progress is replaced as a
whole; separate runs are not merged. [The guide](GUIDE.md#moving-saves-between-devices)
has the player's steps.

The design serves the session and progression loops: earned progress can
follow the player, while replacing a checkpoint remains a deliberate
choice. The comparison shows both copies, focuses Keep this device and
names the run that Import would replace. Undo supplies the recovery path.

The first screen leads with the current run's layer, act and area, then
the profile's runs and record and the transfer folder's copy status.
Export and Import are followed by Undo, Details and Options. Options
holds automatic copying, folder selection and Retry copy. Routine changes
give inline feedback. Controller directions follow the actual button
positions; portrait menus stack the same actions. Back and result buttons
respond to their visible hit areas.

The comparison shows progress and recorded times without ranking divergent
runs by revision or device clock. Keep is the safe first choice. The screen
also says that Keep can refresh the folder copy when Auto-export is on.
Each copy's Details pages preserve its full device name, build and path,
and Back returns to the same comparison. Import results name the installed
run, offer Continue where its checkpoint is available, and retain Undo.

The screen's course, drawing and words live in `src/saves/`. Every entry
into an import uses the same core preflight: a system picker, a dropped
file or a copy found at startup. The browser's menu opens that screen
through `cw_saves_open`; its page does not parse, pack or restore saves.

## What travels

The carrier contains only the known progress files under `savedata/`:

| Files | What they hold |
| --- | --- |
| `profile.sav` | Runs, records, unlocks and the Library |
| `rivals.sav`, `battle.sav` | The rival's and the guardians' records |
| `run.sav`, `run.state` | The current run and its emulator checkpoint |
| `run.make`, `run.area`, `run.seen`, `run.folder`, `run.act`, `run.dark`, `run.souls`, `run.board` | The checkpoint's generation version, area and accompanying run data |
| `transfer.manifest` | Its version, provenance and progress summary |

ROMs, boot states and unknown files are never packed. Keys, controller
bindings, touch layout, display settings and statistics consent stay in
the receiving data folder. The music and sound volumes are stored inside
`Profile`, so export normalizes those two fields and recalculates the
profile checksum; import restores the receiving profile's volumes before
installing it. A volume change therefore does not make a new portable
revision.

## Carrier and manifest

`src/core/backup.c` keeps the original carrier, so `.cwsave` files made
before this screen still read. Every number is little endian:

| Field | Size |
| --- | --- |
| `CWSAVE1\n` | 8 bytes |
| Timestamp, seconds since 1970 (last played in v2; export time in old files) | u64 |
| File count | u32 |
| Each file: name length, name, byte length, bytes | u16, that many bytes, u32, that many bytes |
| FNV-1a of all preceding carrier bytes | u32 |

The complete carrier is at most 16 MiB, with at most 64 entries and each
entry at most 4 MiB. All names and bounds are checked before any callback
can write a file. Duplicate names, embedded nulls, path traversal and
unrecognized names are refused. The four settings files in an old
carrier (`settings.ini`, `keys.ini`, `pad.ini`, `touch.ini`) are accepted
for compatibility and ignored when installing it.

V2 adds the fixed-schema `savedata/transfer.manifest` text file inside
that carrier; the outer magic stays the same. It records:

- Format and full game build, including the development build suffix.
- `RUN_MAGIC` and `LAYER_MAKE` for checkpoint compatibility.
- Last played, source system and device, and the save's identity,
  revision and parent revision.
- The portable payload hash and file count.
- Runs played, best layer, current layer, act, area and whether the
  checkpoint needs BN5.

`backup_manifest.c` updates the timestamp and revision only when the
portable payload changes. Starting the game or exporting the same saves
again keeps their provenance. `backup_info.c` derives the summary from
the checksummed save blobs and verifies it against the manifest, rather
than trusting the manifest's labels alone. Old files without a manifest
still show what their profile and run say; missing provenance is unknown.

## Import and undo

`backup_check` refuses damaged carriers, unsupported formats, a newer
release or development revision, a newer layer-generation version and an
unsupported run structure. A BN5 checkpoint also requires the receiving
build to have BN5's ROM available. The source build is shown with the
refusal so the player knows when to update. No refusal changes the local
progress or checkpoint.
The pre-emulator `CWE1` run format remains readable without a `run.state`;
CONTINUE rebuilds its layer fresh, as the existing legacy loader does.
Modern active runs still require their emulator checkpoint.

`backup_restore` stages all accepted files in `savedata.import/` and
checks every write before renaming anything. It then keeps the previous
complete `savedata/` as `savedata.old/` and installs the staged folder.
Failure rolls back the renames. Startup recovery restores the previous
folder if an interrupted install left `savedata/` absent.

Undo swaps the complete current and previous folders. The next successful
import replaces the previous undo copy. The player can therefore bring
back the progress and checkpoint that the last import replaced, including
a different local run.
Before Restore, `backup_undo_info` reads the actual previous folder without
rewriting its manifest or changing its files. An empty previous folder is
shown as empty progress. The core checks compatibility and reads it again
before swapping; a changed or unreadable copy is refused.
An unreadable existing profile refuses import or Undo before the folders
change; an empty profile is synthesized only when the previous profile
is confirmed absent.

## Platform boundary

`src/launcher/pick_saves.c` and each platform's picker move files between
the data folder and a transfer destination. Native builds can keep a
fresh automatic copy after each save, on by default for phone apps and
optional elsewhere. The browser offers manual downloads and uploads; its
automatic-copy and folder actions are unavailable. The pickers do not
decide which progress wins. `src/launcher/mirror.c` maintains the chosen automatic
copy and finds a copy at startup before writing over it.
Its device-local, checksummed `saves-export.receipt` remembers the exact
carrier fingerprint, successful automatic-copy time and destination;
the older `saves-export.hash` remains a discovery fallback. The receipt is
scoped to the full native path, Android document-tree URI or resolved
iOS folder bookmark, so identically named provider folders do not share
copy status. Failed copies retain the last successful time; Retry checks
for incoming saves before writing again. A folder copy does not confirm
that a cloud provider has synced it. Revision numbers alone do not
identify that output: two devices can independently make different
revision 2s from revision 1. A divergent copy, including one at a local
parent revision, stays held for the comparison screen.
Discovery and automatic writes wait until native ROM and saves pickers
finish, including their background work retaining a new destination.
Android folder-listing failures stage a refusal rather than claiming an
incoming file is absent, and failed listings cannot authorize writes.

In the browser, `Module.savesPick()` opens the hidden file input. The
page stages its bytes as `/tmp/cw-import.cwsave` and calls
`cw_saves_import(path)`; cancellation sends an empty path. The core owns
validation, comparison, installation and the persistence call. Export
hands a core-produced file to `Module.savesDownload(path, name)`, which
downloads the bytes unchanged. Local savedata, its undo folder and
settings continue to live in IndexedDB.

## Validation and limits

| Check | Verified locally |
| --- | --- |
| Carrier and restore tests | ROM-free tests cover the carrier, legacy files, metadata, local settings and the transactional installer. |
| Failure and legacy regressions | Injected unreadable profiles preserve both current and Undo folders; the pre-emulator run format imports without a state while modern runs without one are refused. Busy ROM/saves pickers protect incoming files, and failed exports report failure. |
| SAVES interface regressions | Production scene input, spatial focus, touch hit areas, import and Undo transactions, result actions, full Details paging and copy failure/retry are tested under sanitizers with synthetic progress. |
| Android provider errors | Production `RomLook` runs against JVM provider doubles for absent files, null/error listings, broken cursors, unreadable files and recovery; failed old/stale lookups perform no writes or deletes. This does not establish physical-provider behavior. |
| Linux native import and Undo | A layer 2 checkpoint replaced a local layer 5 run; Undo brought layer 5 back. The local settings file stayed unchanged. |
| Native to browser | An isolated layer 2, seed 7 checkpoint imported through the real Chrome player's file input. CONTINUE showed layer 2 and loaded that map, with its 430,144-byte emulator state present. Settings stayed byte identical and both volume values stayed at 9. |
| Browser persistence and Undo | IndexedDB reload kept the imported layer and ROM. Undo restored the browser's complete pre-import profile, preserving the imported run in the previous folder. Picker cancellation returned to the title without changing the run. A dropped `.cwsave` reached the same core check and left the local run unchanged. |
| Browser export | Export wrote a core-produced `.cwsave`; the page downloaded its bytes unchanged for the return transfer. Both menu shortcuts also opened SAVES before PLAY. |
| Browser to native | The browser-produced file replaced the native test device's layer 5 checkpoint. CONTINUE resumed the same layer 2 map and Lan/Chaud dialogue. |
| Browser to Android to native | In an isolated Android emulator, the native document picker imported the browser export and CONTINUE resumed layer 2. Choosing a separate Saves folder kept the ROMs folder unchanged. The native export picker returned a byte-identical carrier, which imported and resumed on Linux. Android's statistics answer and Linux's settings stayed local. |
| Native file drops | Injected SDL file-drop events at the title and ROMs screen opened the comparison with Keep selected, including a simultaneous START press. Both local layer 5 checkpoints stayed unchanged. |
| Portrait touch | The comparison fit a 360×640 display with full action labels. A scripted tap on Use file installed layer 2 and kept the receiving settings unchanged. |
| Cross builds | The Windows, Android, browser and New 3DS targets built with warnings treated as errors. |

These checks use local Linux builds, headless Chrome, an isolated Android
emulator and test saves. They do not establish hardware behavior on a ROCKNIX handheld, a
physical New 3DS or phone, or the Steam Deck's Gaming Mode. macOS and iOS
builds, signing and iOS Simulator startup passed in CI. Apple picker
behavior has not been exercised. No ROM or
game-derived save, state or transfer file is published; committed test
fixtures contain only synthetic bytes.
