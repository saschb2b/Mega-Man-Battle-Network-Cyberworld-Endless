---
type: Feature
title: "Portable saves"
description: "Portable saves (issue #114): the SAVES screen's views (`src/saves/`), the `.cwsave` carrier and manifest (`src/core/backup*.c`), what travels, import checks, UNDO IMPORT and recovery, the automatic copy (`mirror.c`), and saves sent by a code and kept in step with a cloud slot over the Net Square's server (`saves_code.c`, `saves_cloud.c`, `transfer.c`, `cloud.c`)."
tags: [engine, saves]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T01:30:00Z }
---

# Portable saves

How a player's progress and the run's checkpoint move from one system to
another (issue #114). SAVES on the title writes them to one file,
`cyberworld-endless.cwsave`, and reads one in on any other system, the
browser included. On Android and iOS, where uninstalling deletes the
app's files, the game also keeps that file in a folder outside the app,
and a reinstall finds it there. The player's steps are in
[the guide](../GUIDE.md#moving-saves-between-devices). The screen is
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

## The views

**Home** (`saves_home.c`; reworked on 15 November 2026 when it had grown
to eight buttons, [Save screens elsewhere](saves-screens.md) has why).
Options' kin: the pages CLOUD, CODE and FILE as its tabs, turned by L
and R or a tap (no row where the system has FILE alone: the PSP, a 3DS
build without libcurl); a strip under them, THIS DEVICE with the run
("Layer 4, Act 2", its area where it fits) and the record ("3 runs, best
Layer 9", or "No runs yet", which a first start's device, its `savedata/`
not yet made, reads too: `backup_local_info` takes a missing folder for
no saves, never "Damaged", unless an import cut short left it missing);
the page's lines in the cyan window, Up and
Down round, the line each page was left on kept; KeyItem's information
box under them, what the line does, then where the page has one how it
stands. CLOUD: Turn on (where there are saves) and Link with a code, or
once on Add a device, Unlink this device and, a line apart, Delete cloud
saves; its status "Synced 11-14 23:34", "Syncing...", "Sending 40%" or a
trouble. CODE: Send (where there are saves) and Receive. FILE: Export
(where there are saves), Import, Auto-copy On or Off (a value Left and
Right turn, between Options' orange arrows) and Folder with its last
part, where the system has them; its status the copy's ("Copied to
Downloads 11-14 14:03", "The copy failed. Choose the folder again.",
"New saves in ROMs. Import shows them."). UNDO IMPORT is no line but
START's, `START:UNDO IMPORT` on the band where an import can be undone,
as Options' START:DEFAULT. START, Turn on, Unlink and Delete ask first in
the box ("Bring back your Layer 4 run from before the import?", "Delete
the cloud saves? Each device keeps its own."), BN6's Yes and No under
the words, No chosen. SAVES opens on the page that needs the player (the
cloud's trouble only the player can answer, or new saves in the copy's
place, Import under the cursor), else on the page last shown, CLOUD the
first time. The band names the view, a back arrow before it where B goes
back, the band's left its tap on a phone without a Back key; a view
opened from a line names its line (SEND, RECEIVE, ADD A DEVICE, LINK).
The words are "device" for what holds saves and "sync" for the cloud's
keeping them alike, one name each.

**Compare.** THIS DEVICE and THE FILE (THE CODE, THE CLOUD: whatever
brought them) side by side, each with its run,
area, runs and best, and the system and date it was last played on
("Android, 10-13 09:12": the system's name, never the device's or the
host's). A note says what USE THOSE replaces and that UNDO IMPORT
brings it back; where the file is the automatic copy's own and the copy
is on, it adds that KEEP THESE writes this device's saves over the copy;
where it is the cloud's, that KEEP THESE sends this device's saves to the
cloud and the other device asks at its next start. One that opened by
itself (the copy's place's, the cloud's) takes no press for half a
second, and none answers it but A, so a START mashed through the title
answers nothing. The
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
with CONTINUE where that run can go on, else DONE. The result's UNDO
IMPORT swaps at once (the import's outcome is on the screen); its result
offers UNDO again, which swaps them back. An export's result says where
the file went.

## What travels

The file holds only the progress files under `savedata/`:

| Files | What they hold |
| --- | --- |
| `profile.sav` | Runs, records, unlocks and the Library |
| `rivals.sav`, `battle.sav` | The rival's and the guardians' records |
| `run.sav`, `run.state` | The current run and its emulator checkpoint |
| `run.make`, `run.area`, `run.seen`, `run.folder`, `run.act`, `run.dark`, `run.souls`, `run.board`, `run.lib`, `run.tally`, `run.data` | The checkpoint's generation version, area and the rest of the run's data (`run.lib`: a dark layer's liberation board as it stood, [Liberation Missions](../game/liberation/overview.md); `run.tally`: its economy's counts, [Economy](../game/economy/overview.md); `run.data`: the viruses it deleted by family, which join the profile's battle data as it ends, [the Battle Lab](../game/battle-lab.md)) |
| `weekly.sav`, and each of the above after `weekly-` | The week's net's slot (below): the dive in it and a dive's end waiting for the Board, its own profile, run and checkpoint |
| `transfer.manifest` | Its format, where it was made and a summary of its progress |

The profile's Library holds chip ids to 1023 (`library_w` after the
rest: a profile saved before reads them 0, and a build before it reads
a newer profile's `.cwsave` as made by a newer version), for [ten-bit
chip ids](../multirom/bn4-chips.md). `run.state` holds BN6's RAM and so
its folders' entries, in ten bits since ten-bit chip ids (`chip_entry.h`),
and the cart's save RAM, where the pack counts the chips of ids from 635
([Memory the engine writes](emulation/memory.md), the save RAM's table: a
state from before them is read as holding none):
`RUN_MAGIC` is "CWED" (`save_format.h`; "CWEC" is Oran Isle's run.stop,
nine-bit too); a run saved before ("CWEC", "CWEB", or an older format)
is read as ever, and its folders go through the converter
(`chip_entries_from9`, unit-tested; `tests/test_legacy.c` reads a "CWEB"
and a "CWEC" run as today's) once: `run.folder` as
it loads, the state's three folders (eToolkit `+0x48`) as soon as the
state is in (`own_folder_chips`, `director_folder.c`). Tried with a run
the nine-bit build saved at a guardian's door: the ten-bit build's
CONTINUE the same picture for picture as the nine-bit build's, and
without the converter not, from the battle on. A ten-bit run's `.cwsave`
reads as made by a newer version in a build before it.

ROMs, boot states and any other file are never packed. Controls, keys,
the touch layout, screen settings, the volumes and the statistics answer
stay on each device, as do SAVES' own files (below). The two volumes are
stored inside `Profile`, so an export sets them to fixed values and
recomputes the profile's checksum, and an import or an Undo keeps the
receiving device's: a change of volume is not a change of the saves.

### The weekly slot

A dive of the Net Square's week ([The week's net](../online/weekly-net.md); issue
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

## Sending by code

SEND and RECEIVE move the same `.cwsave` file over the official Net
Square's server, for a player who asked to "load my save file through
internet without copy the file physically" (GitHub discussion 5,
November 2026). A copy by hand or the automatic copy's folder still
work; this is the way with no file to carry.

**Design.** Five agents weighed it in November 2026 (prior art, the
server, the UX through the game-design skill, a devil's advocate): an
account or a Google sign-in costs every player a second screen and
leaves the conflicts to be built anyway, and continuous sync is the most
to build and keep for a demand not yet measured. The owner chose a
one-shot code first (2026-11-14); the server's daily totals count the
codes held and taken (`transfers_held`, `transfers_taken`,
server/README.md), which says whether the next step, sync, is wanted. It
stays a usability problem, as the rest of SAVES: no account, one code,
nothing changed until the comparison's USE THE FILE'S, UNDO IMPORT after
it.

**The views.** Home's CODE page has Send where the system has the link
and saves to send, and Receive where it has the link. Send packs the
saves as Export does, seals them and hands them to the server; its view
(SEND on the band) keeps this device's panel, then the code in letters
twice the size, in two groups of five ("61626 62133"), "Works once, for
15 more minutes." and "On your other device: SAVES, CODE, Receive." While
it sends it says "Sending: N%". Send again with the same saves, while
their code holds, shows that code again rather than another; leaving the
view while it sends gives the sending up (a code shown stays with the
server till its time is up). Receive (RECEIVE on the band) asks for the
code: ten keys in a row (1 to 9, then 0; two rows of five on a narrow
screen), ERASE and OK, the cursor on OK once the tenth digit is in; on
the desktops and the browser a keyboard's digits type it too
(`text_input.h`), Enter is OK and Escape goes back. The saves that come
go to the comparison as THE CODE's, its panel naming the system and the
time they were last played; a cloud's link that comes instead joins this
device to that cloud ([Saves kept in step](#saves-kept-in-step)). Each failure
says one line in gold, a receive's with "Your saves are kept.": the
server not reached, busy (with the minutes to wait), nothing under that
code ("A code works once, for 15 minutes."), a server that takes no saves
yet (one from before them answers TRANSFER with an ERROR), saves that
could not be opened, too big (more than 2 MiB sealed), or too little
memory. Captured in `--scene saves` against a local server
(`server/main.py`, `--dev online=ws://127.0.0.1:PORT/presence`): a send,
its code typed on a fresh data folder, the comparison, the import, and
CONTINUE on the sender's layer, every file the same but `profile.sav`,
whose volumes the receiver keeps ([What travels](#what-travels)).

**The code and the seal** (`src/online/transfer_box.c`). Ten digits from
the system's random bytes (`network_identity`, a byte past 249 thrown
away so each digit is as likely). Argon2id (Monocypher 4.0.2,
[third_party/](../code/third-party.md); a megabyte, eight passes, the
salt `cyberworld.transfer.v1`) makes a 32-byte key and a 16-byte
lookup from them; the saves are sealed with XChaCha20-Poly1305, a random
nonce and `CWT1` as their mark and associated data. The code itself is
never sent: the server files the blob under the lookup and can open
neither. Opening one without the code means trying codes, each an
Argon2 run, up to ten billion of them; as the salt is fixed so that both
systems make the same key, a table could be made once for every code,
which the blob's fifteen minutes in the server's memory alone make worth
little. What a code makes is pinned in `tests/test_transfer_code.c`: a
change to it would break every transfer between builds before it and
after.

**The exchange** (`src/online/transfer.c`). shared/online/README.md
(Saves sent by a code) has the wire: a connection to the Square's
`/presence` whose first message is TRANSFER, over the Square's own
socket through the relay (`relay.c`, below: a thread of its own,
`online_socket.h`; the browser's `online_browser.h` on its main thread,
SAVES' frames moving it on), never
while a visit is on, as SAVES opens only over the title and the ROMs
screen. A put whose lookup the server holds already (TAKEN) is sealed
again under a new code, three times at most. The server's side, its
limits and what it keeps are server/README.md's. SAVES reaches it through
`src/core/transfer_link.h`, which online's transfer sets at the start
(`start_net.c`), as SAVES may not include online
([the layers](../code/layers.md)); a system without the Square's socket
(the PSP, a 3DS build without libcurl) has no link and shows neither
button. The server is the official one, or `--dev online`'s.

## Saves kept in step

CLOUD keeps a system's saves in step with the player's other systems, as
Steam Cloud keeps a game's: set up once on each system, then nothing to
press. It rides the same server and the same sealing as a code's saves
([Sending by code](#sending-by-code)), and the owner asked for it
(2026-11-14) right after the code went in, to give "a Steam cloud save
like experience".

**Design.** The game-design pass is the one the five agents made for it
(November 2026): moving between systems is a frequent errand for the
players who ask for this, not a rare one, so the cloud acts by itself
wherever that is safe and asks only where it is not. Steam's own shape,
measured against what players hold against it: a sync before play, an
upload as the game closes, and one question where both sides moved on,
which here shows each side's run, area, runs and system rather than two
dates, and is undone by UNDO IMPORT where Steam's overwrite is final. No
account: a system joins the slot by a code from one already in it, the
one step Steam's login spares its players. The owner's calls, given as
defaults on 2026-11-14 and his to change: a pull taken in without asking
where this system has not moved on (UNDO IMPORT still brings back what
it replaced); one side's saves whole in a conflict, never a merge; a
slot unused for 180 days deleted; the Net Square's identity still per
system.

**What the player sees.** Home's CLOUD page ([The views](#the-views)):
Turn on where this device has saves, and Link with a code, which is
Receive's code entry (LINK on its band). Turn on asks ("Turn on cloud
saves? These saves go up now.") and makes a slot and sends these saves;
the page then says how it stands ("Sending 40%", "Synced 11-14 23:34",
or a trouble's line) under Add a device, Unlink this device and, a line
apart, Delete cloud saves. Add a device shows a code as Send does (ADD A
DEVICE on its band, "On your other device: SAVES, CLOUD, Link with a
code."), which Link with a code (or Receive) on the other device takes: a
code's bytes are saves or a link, and a link joins this device to the
slot and pulls at once. Unlink asks, then forgets the slot here
("Unlinked. Your saves stay here."); Delete asks ("Delete the cloud
saves? Each device keeps its own."), then deletes it on the server,
forgotten here too ("Cloud saves deleted. Your other devices stop
syncing at their next start."). Neither runs while the cloud moves saves
("Waiting for the sync to end.").

**When it syncs** (`src/saves/saves_cloud.c`). As the game starts and
whenever it comes back from the background, the title pulls; its menu
waits for the pull, six seconds at most, and the title's note box says
"Cloud: syncing...". A pull whose saves are newer than the receipt's
generation is taken in without a word where this system's saves are the
ones it last had in step (an import through `backup_restore`, UNDO
IMPORT after it, the title shown again over them with the note "Cloud:
Layer 7 from Linux, 11-14 23:35"); where this system moved on too, SAVES
opens the comparison at once with THE CLOUD, USE THOSE and KEEP THESE
("KEEP THESE sends this device's saves to the cloud."),
the cursor on the side with more progress as ever; DONE leaves it to
the next start. Three seconds after the saves were last written
(mirror.c's hooks, `MirrorFollow`) they are pushed where they are not
the ones in step, and again as the game closes, waited for four seconds
at most; a New 3DS packs and seals only outside a run (its heap), its
run's saves going up at the title or as it closes. A push the server
answers CONFLICT (another system's came first) stops the pushes until
the title's next pull compares. The slot gone (deleted elsewhere, or
unused 180 days) unlinks this system with "Cloud: the cloud saves are
gone. Turn on to start again."; a server that keeps no slots, or one from
before them, says so. Nothing is lost either way: every system keeps its
own saves, and the server is only how they travel.

**The slot** (`src/online/cloud.c`). A 32-byte secret made on the system
that turned it on, kept in `cloud.ini` in the data folder with the
receipt (the slot's generation these saves are in step with, their
hash, when), never in a `.cwsave`. From it, keyed BLAKE2b makes the
slot's id and auth for the server and the key that seals the saves
(`transfer_slot`, pinned in `tests/test_transfer_code.c`); the link a
code carries is the secret itself, sealed as a code's saves are. The
saves go up as `CWSY`, a flags byte (none yet) and the `.cwsave`,
sealed. shared/online/README.md (Saves kept in step) has the wire, its
generations and their CONFLICT; server/README.md what the server keeps
(the auth's hash, the sealed blob, its days) and its limits.

**One exchange at a time** (`src/online/relay.c`). A code's transfer and
the cloud's pulls and pushes share one engine on the Square's socket: a
connection, a first message, a blob sent or taken; an exchange asked for
while another runs waits for it. Online's transfer and cloud set their
links at the start (`start_net.c`), as SAVES may not include online.

Checked end to end against a local server (`server/main.py`,
`--dev online`): TURN ON from a folder with a run (the slot at
generation 1), a restart pulling SAME ("in step"), ADD A SYSTEM's code
typed on an empty folder (linked, pulled, taken in), a run there saving
and pushing at once (generation 2), the first system's title taking that
in by itself, and both systems' saves changed apart: the first's start
pushing (generation 3), the second's opening the comparison at its
start, USE THE CLOUD'S leaving both at generation 3 with the same saves.
`tests/test_saves.c` drives the decisions with a link of its own.

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
