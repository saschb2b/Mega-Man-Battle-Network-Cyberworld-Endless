# Portable saves

How a player's progress and the run's checkpoint move from one system to
another (issue #114). SAVES on the title writes them to one file,
`cyberworld-endless.cwsave`, and reads one in on any other system, the
browser included. On Android and iOS, where uninstalling deletes the
app's files, the game also keeps that file in a folder outside the app,
and a reinstall finds it there. The player's steps are in
[the guide](GUIDE.md#moving-saves-between-devices). The screen is
`src/saves/`, the file `src/core/backup*.c`, the automatic copy
`src/launcher/mirror.c`.

## Design

Moving a run between systems takes one decision and never costs
progress. It is a rare errand, done a few times in a player's life with
the game, so it is a usability problem, not an engageability one: the
screen has to be read at a glance and be hard to get wrong, and need not
be fun.

Three patterns shape it. An honest telegraph: before anything changes,
both saves stand side by side, and a line says what the choice replaces
and how it comes back. Error prevention: a file that cannot be used is
refused before anything changes, and nothing is destroyed silently; what
an import replaces stays for UNDO IMPORT, and what Undo held before that
stays as a file. Respect for the player's time: three views (home,
compare, result), only the buttons that can be used, the cursor on the
likelier answer, and a found copy asked about once, not at every start.

SAVES opens with L on the title (Q on a keyboard; the line of keys under
the menu names it), and in the browser also from the page's menu. It
opens over the title or the ROMs screen, never in a run, and stays off
the title's menu, whose rows are drawn from the ROM.

## The three views

**Home.** THIS DEVICE's panel says the run ("Layer 4, Act 2" and its
area) and the profile ("3 runs, best Layer 9", or "No runs yet"). One
line under it says where the copy goes and how the last one went
("Copied to ROMs 10-12 14:03", "No copy in ROMs: choose it again", "New
saves in ROMs: IMPORT shows them", ROMs being the folder). The buttons:
EXPORT where there are saves, IMPORT, UNDO IMPORT where an import can be
undone, AUTO-COPY: ON or OFF where the system has the automatic copy,
FOLDER where it has a folder chooser.

**Compare.** THIS DEVICE and THE FILE side by side, each with its run,
area, runs and best, and the system and date it was last played on
("Android, 10-13 09:12": the system's name, never the device's or the
host's). A note says what USE THE FILE'S replaces and that UNDO IMPORT
brings it back; where the file is the automatic copy's own and the copy
is on, it adds that KEEP THESE makes the folder's copy this device's. The
cursor starts on the side with more progress (more runs, then the deeper
layer, a best or the run on now; this device's at a tie), so after a
reinstall it is on the file's. On a device with no saves the buttons are
DONE and BRING BACK; saves here that cannot be read are said so (their
column "Damaged"), and an import keeps them for UNDO IMPORT like any
other. A refused
file shows one reason and "Your saves are kept.", with DONE alone: a
newer build's file, a damaged one (its column says only "Damaged"), or a
run that needs BN5 where BN5's ROM is missing. A file that holds the very
saves already here goes straight to the result, which says so.

**Result.** What the import put in and what Undo brings back ("Saves
imported: a run on Layer 2. UNDO IMPORT brings back your Layer 5 run."),
with CONTINUE where that run can go on, else DONE. UNDO IMPORT is a
direct swap with no preview; its result offers UNDO again, which swaps
them back. An export's result says where the file went.

## What travels

The file holds only the progress files under `savedata/`:

| Files | What they hold |
| --- | --- |
| `profile.sav` | Runs, records, unlocks and the Library |
| `rivals.sav`, `battle.sav` | The rival's and the guardians' records |
| `run.sav`, `run.state` | The current run and its emulator checkpoint |
| `run.make`, `run.area`, `run.seen`, `run.folder`, `run.act`, `run.dark`, `run.souls`, `run.board` | The checkpoint's generation version, area and the rest of the run's data |
| `weekly.sav`, and each of the above after `weekly-` | The week's net's slot (below): the dive in it and a dive's end waiting for the Board, its own profile, run and checkpoint |
| `transfer.manifest` | Its format, where it was made and a summary of its progress |

ROMs, boot states and any other file are never packed. Controls, keys,
the touch layout, screen settings, the volumes and the statistics answer
stay on each device, as do SAVES' own files (below). The two volumes are
stored inside `Profile`, so an export sets them to fixed values and
recomputes the profile's checksum, and an import or an Undo keeps the
receiving device's: a change of volume is not a change of the saves.

### The weekly slot

A dive of the Net Square's week (docs/ONLINE.md, The week's net; issue
#127) is a run of its own beside the normal one, and the owner's rule is
that it never touches the run in progress. It is a second save slot in
the same folder: while a dive is played, `save_path` (`src/core/save.c`)
puts every progress file after `weekly-` (`weekly-profile.sav`,
`weekly-run.sav`, `weekly-run.state` and the rest of the list above),
and `weekly.sav` (`src/core/weekly.h`, magic `CWW1`, 28 bytes,
`save_layout.h` holding its layout) records the dive in the slot (its
week and seed, unfinished or not), a dive's end waiting to be posted
and the deepest layer of the week's dives (the summary's "This week").
The normal slot's files are not written while a dive is played: a
playtest hashed all twelve before a dive began and after it ended, the
game closed mid-dive and continued in between, and each came back the
same. The title always goes back to the normal slot (its CONTINUE is the
normal run's); a dive goes on from the Square's greeter.

A flat prefix rather than a folder `savedata/weekly/`: the carrier, its
staged install, Undo and recovery all move one flat `savedata/` whole,
so a prefix keeps every one of them as it was. `RUN_MAGIC` and the
profile's size are the same in both slots; the slot's profile is a
`Profile` like the other.

The carrier takes the slot's files by their names (`backup_progress_name`:
`weekly.sav`, and `weekly-` before one of the normal slot's names, no
other), so an export holds both slots and an import puts both back, Undo
and recovery as before. The summary and the manifest are the normal
slot's alone: runs, best and the run's layer are the normal run's, and
the dive's checkpoint is no part of `has_state`. `tests/test_backup.c`
(`test_weekly_slot`) packs a folder with both slots, unpacks it into a
fresh one and finds every file the same, and refuses a name like
`weekly-evil.sav`.

## Carrier and manifest

`src/core/backup.c` keeps the first carrier, so every `.cwsave` made
before SAVES still reads. Every number is little endian:

| Field | Size |
| --- | --- |
| `CWSAVE1\n` | 8 bytes |
| Timestamp, seconds since 1970 (the last progress write; an export's time in old files) | u64 |
| File count | u32 |
| Each file: name length, name, byte length, bytes | u16, that many bytes, u32, that many bytes |
| FNV-1a of all the bytes before it | u32 |

A file is at most 16 MiB, with at most 64 entries of at most 4 MiB each.
Every name and bound is checked before anything is written: duplicate
names, embedded nulls, paths that climb out and unknown names are
refused. The four settings files an old carrier held (`settings.ini`,
`keys.ini`, `pad.ini`, `touch.ini`) are accepted and left out of the
install.

Format 2 adds `savedata/transfer.manifest` among the files; the magic
stays the same. It is text with a fixed schema and order: `format`,
`version` (the full build, a development build's suffix too),
`run_magic` and `layer_make` (the checkpoint's compatibility), `stamp`,
`system`, `device`, `save_id`, `revision`, `parent_id`,
`parent_revision`, `payload_hash`, `files`, then the summary: `runs`,
`best`, `run_depth`, `run_area`, `run_act` and `needs_bn5`. `system` and
`device` both hold the platform's name (Linux, Windows, macOS, Android,
iOS, PortMaster, New 3DS, browser), never a device or host name; the
comparison shows it with the date.

`backup_manifest.c` writes a new stamp and revision only when the
progress changed, so starting the game or exporting the same saves twice
makes no new version. `backup_info.c` reads the summary from the
checksummed save files and refuses a manifest that disagrees with them.
A file without a manifest shows what its profile and run say.

## Import, undo and recovery

`backup_check` refuses a damaged file, a newer format, a newer build or
layer-generation version (`LAYER_MAKE`), a newer run format
(`RUN_MAGIC`), and a run that needs BN5 where the receiving build has no
BN5 ROM. A refusal changes nothing. The pre-emulator `CWE1` run format
still imports without a `run.state` (CONTINUE builds its layer afresh);
a later run without its state is refused. An existing profile that
cannot be read refuses an import or an Undo before any folder moves.

`backup_restore` (`backup_restore.c`) installs a file in steps that a
power cut cannot leave half done:

1. Every file is staged in `savedata.import/`, each written whole and
   flushed (fsync where the system has it), and the staged folder is
   read back and hashed.
2. Where `savedata.old/` holds an earlier import's Undo, it is kept as
   `saves-replaced-1.cwsave` in the data folder; the older ones move to
   `-2` and `-3`, and a fourth is dropped.
3. `savedata.pending` is written with the staged saves' hash, then the
   renames: the old Undo aside as `savedata.undo/`, `savedata/` to
   `savedata.old/`, the staged folder to `savedata/`. A rename that fails
   rolls back the ones before it.
4. `savedata/` is read back; where it holds the staged hash,
   `savedata.undo/` and `savedata.pending` are removed.

`backup_recover` runs at every start before anything reads the saves (the
ROMs screen first), and before each import. Where `savedata.pending`
names saves that `savedata/` does not hold, or `savedata/` is missing,
`savedata.old/` comes back as `savedata/` (and `savedata.undo/` as
`savedata.old/`), and the leftovers are removed. If `savedata/` is still
missing while saves wait in a folder beside it, the game writes no saves
over them (`backup_blocked`), and SAVES says "Not saving: savedata could
not be put back."

UNDO IMPORT reads `savedata.old/` without changing it, checks it as an
import would, writes this device's volumes into its profile, and swaps
the two folders through `savedata.swap/`. Undo again swaps them back.

## The automatic copy

`src/launcher/mirror.c` keeps `cyberworld-endless.cwsave` in a transfer
folder: on Android and iOS the ROM folder, or a folder chosen with
FOLDER; on a desktop its Downloads or a chosen folder; on a PortMaster
handheld and the 3DS the data folder. It is on by default on Android and
iOS and off elsewhere (`saves.ini`); the browser has none and exports by
download.

Three seconds after the game last wrote its saves, so that a
checkpoint's files are all written, and as the app goes to the
background or quits, it packs the saves and writes the file whole. It
writes only where the folder's file is absent, is one this device wrote,
holds the saves already here, or is one the player answered with USE THE
FILE'S or KEEP THESE. Any other file holds the copy, and the game opens
its comparison once, over the title or the ROMs screen. The answer is
kept per folder, so the comparison does not reopen at every start; a
file left with DONE or B keeps holding the copy, and with the copy on
IMPORT shows it first. A file that cannot be read holds the copy too (on
iOS one in iCloud and not yet on the phone, which is sent for). A copy
that fails is tried again at the next save, and the line on SAVES' home
says so.

The folder is read once a start or resume and as SAVES opens (a
desktop's Downloads only then, or with the copy on): its file read into
memory and hashed, nothing written. Where the system can tell a file's
size and time without reading it, a changed file is read again before a
copy; a phone's document provider cannot, and is read once a start.
Nothing looks at the folder every few seconds.

`saves-receipts.txt` in the data folder is what the copy remembers,
device-local and never exported. `wrote HASH` lines hold the last eight
files this device wrote out (copies and exports alike), so its own file
is never taken for another device's. `folder ANSWERED ALLOWS COPIED_AT
SCOPE` lines, one for each of the last four folders, hold the file the
player answered there, whether that answer lets the copy write over it,
and the time of the last copy. SCOPE is the folder's identity: a full
path, an Android document tree's URI, or an iOS folder's resolved URL.

`saves.ini` holds the copy's switch and the chosen folder. With
`saves-receipts.txt`, `saves-replaced-1.cwsave` to `-3` and
`savedata.old/`, that is all SAVES keeps in the data folder; its
temporary files (an export's, a picked file's copy) are removed once
written or read.

## Platform boundary

Each system's pickers (`src/launcher/pick.h`) move a file between the
data folder and the world outside, and never decide which saves win:
every file, whatever brought it, goes through the same check and
comparison. The desktops use their file dialogs (`pick_desktop.c`; no
folder chooser in Steam's big screen); Android its document pickers and
the kept folder tree (`pick_android.c` through `GameActivity` and
`RomLook`: `savesGet` answers 1 read, 0 none, -1 unreadable); iOS its
Files pickers and a bookmark (`pick_ios.c`, `src/core/ios_saves.m`); a
PortMaster handheld and the 3DS the data folder's own file
(`pick_none.c`). A `.cwsave` dropped on a desktop's window opens the same
comparison.

The browser's page (`web/play/app.js`) opens SAVES through
`cw_saves_open`, which refuses in a run (the page then says to go to the
title first). Its menu's Save backup opens SAVES; Load backup opens it
and the page's file chooser at once, as a browser opens a chooser only
in the player's own click, so SAVES' IMPORT in the browser points to
that button and waits for it (B, or a minute, lets it go). The page
hands a chosen or dropped file to `cw_saves_import` as
`/tmp/cw-import.cwsave` (an empty path for a cancel) and downloads the
bytes the core made through `Module.savesDownload`; it never reads or
writes the format itself. The saves, their Undo and the settings stay in
IndexedDB.
