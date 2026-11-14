---
type: Feature
title: "The game's own updates"
description: "How the game learns a newer version is out and, where it can, puts it in place itself: the release's update.json, the look at each start (on by default, Options' Updates line switches it off), the title's note and Options' update line, each system's way (the AppImage over itself, the Windows installer run silently, the PortMaster launcher after the game, Android's installer, the download page, the itch app), the checks before a file is placed, and what other systems are to get."
tags: [engine, updates, platforms]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T03:30:00Z }
---

# The game's own updates

Before 0.22.0 a player learned of a new version from the releases page
and downloaded it by hand, but on iOS (SideStore's source), the PS5
(Prospero Store), the AppImage with an updater of its own (Gear Lever,
AppImageUpdate) and a copy installed through the itch app. The owner
asked for updates with no to minimal friction, and chose (14 November
2026) that the game looks for them by default, with a switch to turn it
off. The game now looks at each start and, where the system lets it,
updates itself with one press. The research behind the choices, each
system's channels and stores, is summed up under [What comes
next](#what-comes-next).

## What the player sees

- **The title**, once a newer version is found: `New v0.22.0` in the
  PET's green beside the build's own version, over the copyright line
  (`src/scenes/title_version.c`). Nothing pops up and nothing waits.
- **Options**, opened on the title: the Game page's first line is
  `Update to 0.22.0`, and the first opening after the find puts the
  cursor on it. Its note says what A does, which depends on the copy
  (below). While the file downloads the line's value is its share in
  percent; a failed download says `Failed`, and A tries again.
- **Updates** on the same page, `On` or `Off`, kept in `settings.ini`
  (`updates = off`). Off forgets what was found and stops a download.

The line is shown on the title alone, never in a run's PET: the update
ends the game, and on the title the run waits at its checkpoint. A run
saved by the build before continues in the new one ([Portable
saves](saves.md) and `upgrade_run()` read older formats;
[the old checkpoint test](../devtools/old-checkpoint.md) proves the last
two releases' runs go on before each release).

## The release's update.json

`tools/update_manifest.py` writes it in the release workflow
(`.github/workflows/release.yml`, Update manifest) from the release's own
files, and the release carries it beside them, so

    https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/latest/download/update.json

is always the newest release's. It holds the version and, under a key
per file (`appimage`, `windows-setup`, `macos`, `android`, `switch`,
`3ds-3dsx` and the rest), the file's address at the release's tag, its
size and its SHA-256. A release's address is GitHub's download redirect,
not its API, which limits a machine to 60 calls an hour.
`src/update/update_manifest.c` reads it by its keys' places (the file is
the project's own) and compares versions part by part; the unit tests
read the tool's own output (`tests/test_update.c`).

## The look

`src/update/update.c`. The title's first frame (or Options', on a start
that skips the title) settles what the copy is and starts the look on a
thread of its own: one GET of update.json, through the statistics' own
HTTP client (`src/analytics/analytics_http.c`: libcurl on Linux and the
PortMaster handhelds, WinHTTP on Windows; on Android `HttpURLConnection`
in the app's Java, [below](#android)), its User-Agent
`CyberworldEndless/VERSION` and nothing else sent. A newer version sets
the state the title and Options read. Nothing looks where:

- the build is not a release's (`dev`, `0.21.1-4-gabc123`): only a tag's
  build has a version to compare;
- the game runs headless (captures, playtests, CI);
- `updates = off`;
- the system has none of the ways below yet (macOS, iOS, the
  consoles and the browser, [What comes next](#what-comes-next)).

A developer points it elsewhere with `CYBERWORLD_UPDATE_URL` (another
update.json, any build, headless too) and `CYBERWORLD_UPDATE_FROM` (the
version the build counts as); a local `python3 -m http.server` with a
made-up update.json and file tests the whole way, as this page's first
check did. Android's app takes both as `GameActivity`'s string extras
([below](#android)).

## Each copy's way

`src/update/update_install.c` decides once, at the look:

| The copy | Its way | A on the line |
| --- | --- | --- |
| The AppImage, its folder writable | Installs itself (`appimage`) | Downloads `NAME.AppImage.update` beside it, checks it, renames it over the AppImage (the running one keeps its file), and as the game exits starts the new one in the same process with the game's own arguments: Steam and any launcher that waits go on waiting, its shortcut's path unchanged |
| Windows, the installer's copy (the .exe in the folder the installer wrote under `HKCU\Software\<app id>`) | Installs itself (`windows-setup`) | Downloads the new installer to `%TEMP%`, checks it, and as the game exits runs it with `/S /RUN`: per user, so no administrator's prompt, and a file the game downloads itself carries no Mark of the Web, so no SmartScreen. The installer waits up to ten seconds for the old .exe to be free, installs over it and starts the game (`windows/installer.nsi`) |
| Installed by the itch app (`.itch/` beside it) | The itch app's | Nothing: the itch app updates it on its own |
| The Flatpak, the .deb, the tar.gz's folder, Windows' zip | The download page | Opens the project's download page in the browser |
| A PortMaster handheld whose launcher gives an update folder (`CYBERWORLD_UPDATE_DIR`: the 0.22.0 launcher on, with python3) | Installs itself (`portmaster`) | Downloads the port's zip into `ports/.cyberworld-update/`, checks it and writes `ready.txt` with its version; as the game ends, the launcher shows PortMaster's "keep the device on" message and runs `update.py` (below) |
| A PortMaster handheld with an older launcher, or without python3 | The releases page | Nothing: the note names the releases page, where the port came from |
| Android's app (the release's APK however it came: the browser, Obtainium, `adb`) | Installs itself (`android`) | Downloads the new APK into the app's cache, checks it and hands it to Android's installer while the game waits on the title. From Android 12 on, once the player has let the game install apps, Android puts it in place without a tap; before that, or the first time, Android's own prompt comes up over the game. Android ends the game as it replaces the app, and the player opens the new version from its icon ([below](#android)) |

A download is placed only whole: its size the manifest's and its SHA-256
the manifest's, over TLS from GitHub. A failed one is removed, and the
next start removes what an earlier one left. The game ends the frame
after the file is in place (`update_tick` sets the platform's quit, so
the run, the settings and the statistics end as at any quit), and the
exit handler starts the new version.

## Android

Android has no HTTP client of the game's, so the app's Java does the
work (`android/.../Updater.java`, called from the game's thread through
`src/update/update_android.c` and polled each frame): `HttpURLConnection`
on a thread of its own fetches update.json for the C to read as on every
system, and on A the APK under the `android` key, its size and SHA-256
taken as it comes in. A checked APK is written into a `PackageInstaller`
session and committed with `USER_ACTION_NOT_REQUIRED`, which the app may
ask for as it holds `UPDATE_PACKAGES_WITHOUT_USER_ACTION` and updates
itself; `REQUEST_INSTALL_PACKAGES` lets it install at all.

- **Without a tap**, from Android 12 on (the app targets SDK 35, which
  Android 12 to 16 accept), once the player has allowed installs from
  the game in Android's settings ("Install unknown apps"). Android ends
  the game about a second after the session is committed, so the line
  shows its note (`Android installs it and closes the game. Then open it
  again`) for two and a half seconds first.
- **Android's prompt** where it wants the player's word all the same:
  the first update, which leads through that setting, and every update
  before Android 12. The installer's answer comes to a receiver of the
  game's (`STATUS_PENDING_USER_ACTION`), which shows the prompt over the
  game; a no to it, or any failure, sets the line to `Failed`, and A
  installs the APK already checked again without downloading it anew.
- **The new version**: Android force-stops the game as it replaces the
  app (on the title the run waits at its checkpoint, and the settings
  are written as they change), then tells the new version
  `MY_PACKAGE_REPLACED`. From Android 10 on an app may not open itself
  from the background: on Android 14 in the emulator the system refused
  it (`Background activity launch blocked`, `BAL_BLOCK`), so the player
  opens the game from its icon. Up to Android 9 `UpdateReceiver` opens
  it where the game itself started the update (a mark in the app's
  files, ten minutes at most); that version was not tried, the
  emulator's image being Android 14's.

Every release is signed with the one key ([android/README.md](../../android/README.md),
Signing), so the update keeps the app's files and the saves; an APK
signed otherwise (a build of one's own) is refused by Android and the
line fails. The next start removes the APK and any session an earlier
one left. A developer points the look elsewhere with the extras, on the
`adb` shell's root (the activity is not exported):

    adb root
    adb reverse tcp:8000 tcp:8000
    adb shell am start -n io.github.saschb2b.cyberworldendless/.GameActivity \
      --es CYBERWORLD_UPDATE_URL http://127.0.0.1:8000/update.json

Plain HTTP reaches this device itself alone (`127.0.0.1`, `localhost`:
`res/xml/network_security_config.xml`), which `adb reverse` carries to
the computer's server.

## The PortMaster port

A port's launcher cannot be replaced while it runs, and the game's own
folder holds the player's files too (`--data-dir "$GAMEDIR"`: the saves,
`settings.ini`, `rom/`), so the port updates the way Dungeons of Infinity
and Beyond's port does (the owner's other project, its
`installer/updater.py`): the game only downloads, and the launcher puts
the new version in place once the game has ended
(`port/Cyberworld Endless.sh`, `port/update.py`, shipped in the port's
folder):

1. At its start the launcher undoes an update cut short: where the
   update folder holds `transaction.json`, the copy of `update.py` there
   puts the previous folder and launcher back.
2. Where python3 is there it gives the game `CYBERWORLD_UPDATE_DIR`, the
   update folder beside the game's (`ports/.cyberworld-update`); without
   it the game names the releases page instead.
3. After the game, a `ready.txt` there means a checked download:
   `update.py apply` copies the game's folder to a stage, unpacks the
   zip's files over the copy (the player's files kept where the zip has
   none), refuses a zip that holds anything but the launcher and
   `cyberworld/` (a path out of it, a link, more than 256 MB), checks
   the space first, and only then swaps: a journal written, the old
   folder and launcher aside, the new ones in their names, the journal
   removed, the update folder with them. Exit 10 starts the launcher
   anew, the new version; a failure puts the old one back, says so in
   PortMaster's error box and starts it.

A `ready.txt` the launcher did not act on right away (an older launcher,
a crash) is stale: the next start removes it with its zip.
`tests/test_port_update.py` runs `update.py` on made-up ports folders.

## What comes next

Researched on 14 November 2026 for the owner; each a change of its own.

- **Android**: built ([above](#android)). Obtainium's link on the
  download page for those who use it.
- **macOS**: the app bundle swapped by the game itself (NSURLSession,
  `hdiutil`); a file the app downloads carries no quarantine, so only the
  first install meets Gatekeeper (to be confirmed on a Mac). Sparkle 2
  is the heavier alternative; Homebrew no longer serves unsigned casks.
- **Switch and the 3DS's .3dsx**: the new file staged beside the old one
  and swapped as the game exits (aio-switch-updater's hand-off, never
  over the running file); their curl needs GitHub's roots added to the
  ones the game carries (`analytics_roots.c` has Let's Encrypt's alone).
- **Stores that follow GitHub's releases by themselves**, a listing each
  and no work per release: Universal-DB (the 3DS's Universal-Updater),
  the Homebrew App Store (the Switch), NeoVitaDB (the Vita; VitaDB's own
  list is down). Universal-DB and Flathub ask for AI use to be declared:
  the owner's call. PortMaster-New's GUI marks a listed port's updates;
  a pull request per release. The PS5's Prospero Store already follows
  the releases.
- **The Flatpak**: a repository of the project's own on GitHub Pages,
  the bundle built with its address so Discover updates it, rather than
  Flathub, whose trademark rule would likely mean a new name.
- **The itch app**: one upload a platform, or it asks the player to
  choose at each update; `linux` and `linux-flatpak` may both be tagged
  Linux (to be checked, [the itch.io page](../project/itch.md)).
