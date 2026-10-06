# The Android app

The same game as every other build, for Android phones, tablets and
handhelds (Android 5 and later; arm64, 32-bit ARM and x86-64): SDL's
activity (`org.libsdl.app.SDLActivity`, from SDL2's own source) runs the
game's C, built by the NDK as `libmain.so` with SDL2 and the GBA core.

- `RomActivity` is the icon's activity: it starts `GameActivity`, asking
  for the ROMs screen when it comes from the icon's **ROMs** shortcut
  (`res/xml/shortcuts.xml`). Up to issue #97 it was a page of its own.
- `GameActivity` is SDL's activity with the ROM and data folders as its
  arguments; it keeps the screen on. Where a display stands beside the
  game's, such as the AYN Thor's lower screen, `SecondScreen` shows the
  PET beside the game on it ([below](#the-second-screen)).
- The ROMs screen (`src/launcher/`, which every desktop and phone build
  shows at its first start: README.md, The ROMs screen) calls into
  `GameActivity` from the game's thread (`src/launcher/pick_android.c`):
  `romsPick` opens Android's own folder picker
  (`ACTION_OPEN_DOCUMENT_TREE`, no storage permission) or its file picker,
  several at once (`EXTRA_ALLOW_MULTIPLE`, for Download itself, which
  Android 11 and later keep from the folder picker), and `romsResult`
  hands back what `RomLook` found there. It opens only `.gba` files, tells
  each by its header's game code, checks BN6 Cybeast Gregar's and BN5 Team
  Colonel's by their SHA-1 and copies those two alone into the app's files
  (`files/rom/bn6g.gba`, `bn5c.gba`), where the game finds BN5 beside BN6;
  every other `.gba` is named with why it was refused. The folder is kept
  (a persisted grant, read and write) and looked in again at each start
  while BN5 is missing (`romsLook`); a file seen before (its document, size
  and date) is not opened again. Nothing of them leaves the device.
- The anonymous play statistics (README.md, Anonymous statistics), once
  the player said yes, are posted by `GameActivity.statsSend` with
  `HttpURLConnection` on an executor of their own (one at a time, eight
  waiting at most, none after three failures in a row; `statsDrop` drops
  what waits at a no), called from the game's thread
  (`src/analytics/analytics_net.c`). They are the one reason the app asks
  for the `INTERNET` permission.
- The saves and settings are in `files/data`, which uninstalling deletes.
  So the game keeps a copy in the folder chosen (`src/launcher/mirror.c`):
  `cyberworld-endless.cwsave` (`src/core/backup.h`), packed a few seconds
  after the game last wrote a file and as the app goes to the background,
  and written into the folder by `savesPut` (written whole as a new file,
  then put in the old one's place where the folder's provider renames).
  A folder chosen that holds one has it copied to `files/data/found.cwsave`,
  and the ROMs screen offers it back. Android's Auto Backup takes the saves
  and settings, never the ROMs (`res/xml/backup_rules.xml`,
  `data_extraction_rules.xml`); with no rules it took everything, ROMs
  too, and a reinstall could bring BN6 back with older saves and never
  show the page that adds BN5 (issue #97).
- A controller (a handheld's own controls, Bluetooth or USB) works as on the
  desktop builds. Without one the game draws its touch controls from the
  start; a controller's button puts them away, a touch brings them back.
  Back asks before it quits, as Escape does elsewhere.

## The second screen

On a handheld with a second display, above all the AYN Thor (1240 x 1080
below its 1920 x 1080 screen), the second one is the PET beside the game,
as the 3DS's bottom screen is: framed like BN6's PET screens, with HP,
Zenny and BugFrags, on the net the layer's map, always open (the floor
MegaMan has seen, the way on, what he has come near or senses), in the
folder editor the whole folder, in a battle the Custom screen's chip and
the fight, in the NaviCustomizer the program under the cursor and what
RUN would bring, in a shop the entry under the cursor, and in the town
and on the title the PET at home with MegaMan's face. A phone without a second display plays as before.

- **The display:** one of Android's presentation displays, in the order
  the system lists them (wireless, cabled, overlay, virtual, then
  built-in), but not the game's own, not one off or dozing, and from
  Android 12 not one that tells its place on an HDMI chain (its EDID's
  HDMI block, a TV or monitor on a cable), which goes on mirroring the
  game; a DisplayPort monitor tells none and gets the map. One named as
  the phone's own screen comes last: some devices keep a virtual display
  under that name (Azahar's pull request #1667), and some name their real
  second screen so. A display added, removed or changed is chosen again,
  the map moving with it; with none left the game plays on alone.
- **The window** is a `Presentation`, never focusable: the controller,
  Back and the touch controls stay with the game, and a touch on the map
  does nothing (the map has no use for one, on the 3DS either). It hides
  the system's bars on its display (the Thor's lower screen otherwise
  keeps a band for its navigation bar, tmc-android found) and comes and
  goes with the game's thread, as SDL runs and pauses it: at `onStart` and
  `onStop` (before Android 7, `onResume` and `onPause`), so a dialog over
  the game leaves the map up, and the lower screen has its own launcher
  back while the game is away. After the screen was off it is made again
  0.7 seconds after the game returns: one shown as the screen woke could
  stay black for good (chrono-duo found it on a Thor Lite).
- **The picture:** every fifth frame where it changed (the map's always
  may; a battle's or the folder's panel only with the game's state, and
  once a second all the same), and at once when its panel changes, the
  game draws the second screen (`src/second/`, the 3DS's own) into
  memory at the display's size at the largest whole scale that still fits
  the 3DS's 320 x 240 (the Thor's lower screen: 413 x 360, shown at 3x),
  turns it into Android's byte order and hands it over in a direct
  `ByteBuffer` (`src/core/second_android.c`). Java's main thread copies it
  into a `Bitmap` and shows it with sharp pixels, black round it; the next
  picture waits for that copy, and none allocates anything. In the
  emulator, on a desktop's core, drawing took 0.15 ms, the hand-over 0.1
  ms and Java's copy 0.1 ms.

To try it in the emulator, add a display the size of the Thor's lower
screen beside the phone's with `adb emu multidisplay add 1 1240 1080 240
0`, and take it away with `adb emu multidisplay del 1`. `adb emu
screenrecord screenshot --display 1 DIR` saves its picture (`adb shell
screencap` takes the phone's own only), `adb shell input -d ID tap X Y`
touches it, and `adb logcat -s Cyberworld SDL/APP` names the display chosen
(`second screen: the map on display 2 (Emulator 2D Display)`) and the size
drawn (`second screen 1240x1080: the map drawn at 413x360`).

## Building

```sh
python3 build.py android    # build/release/cyberworld-endless.apk
```

It builds in `docker/Dockerfile.android` (the SDK, NDK 27, Gradle 8.10, SDL2's
source and the GBA core for each ABI). Gradle keeps its downloads in
`.build/gradle-home`; the build and its downloads need the network the first
time.

## Signing

Android installs an update over an app only when both carry the same
signature, and uninstalling deletes the app's saves (all but their copy in
the ROM folder), so every release has to be signed with one key. `build.py android` signs with it when these are set:

| Variable | |
| --- | --- |
| `ANDROID_KEYSTORE` | the keystore, a path inside the repository (the build runs in Docker with the repository at `/src`) |
| `ANDROID_KEYSTORE_PASSWORD` | its password |
| `ANDROID_KEY_ALIAS` | the key's alias (default `cyberworld`) |
| `ANDROID_KEY_PASSWORD` | the key's password (default: the keystore's) |

Without them it signs with a debug key kept in `.build/android-home`, which
is fine for trying a build on your own device.

The release workflow takes the key from the repository's secrets
`ANDROID_KEYSTORE_BASE64` (the keystore file, base64) and
`ANDROID_KEYSTORE_PASSWORD`; without them a release has no APK rather than
one signed with a key the next could not match. A key is made once:
`tools/android_key.sh` runs the JDK's keytool in the Android build image
(it asks you for the password), writes `~/cyberworld-release.jks` and sets
both secrets with `gh`. By hand:

```sh
keytool -genkeypair -v -keystore release.jks -alias cyberworld -keyalg RSA -keysize 4096 -validity 36500
base64 -w0 release.jks   # the value of ANDROID_KEYSTORE_BASE64
```

Keep the keystore and its password safe outside the repository: a lost key
means players reinstall to update, their saves coming back only from the
copy in their ROM folder.

## Installing

Copy the APK to the device and open it (Android asks once to allow installs
from the file manager or browser), or with a cable: `adb install -r
cyberworld-endless.apk`. The first start opens the ROMs screen, whose BN6
slot asks for the folder the ROMs are in: Mega Man Battle Network 6:
Cybeast Gregar (USA), unmodified, the same file every other build takes,
and optionally Mega Man Battle Network 5: Team Colonel (USA).

To try it in the emulator, push copies of the ROMs into its storage, for
instance `adb push bn6g.gba /sdcard/Download/ROMs/`, then choose
Download/ROMs; `adb logcat -s Cyberworld SDL/APP` shows each look (`looked in
ROMs: 3 .gba, kept 3, refused [Falzar.gba: BN6 Cybeast Falzar, not Gregar],
saves found`) and the game's own line, `BN5 found: ...`. A reinstall's
saves: play until the game saves, check that
`/sdcard/Download/ROMs/cyberworld-endless.cwsave` is there, `adb uninstall
io.github.saschb2b.cyberworldendless`, install the APK again and choose the
folder: the ROMs screen asks **SAVES FOUND**, and **BRING BACK** puts back
`files/data/savedata` as it was (the emulator, with `adb root`, can hash
both).

Joy-Cons can be tried there without the hardware: `tools/jc_uinput.c`
(built with this image's NDK, the command in its first lines) makes a left
and a right one on the emulator's uinput as Linux's hid-nintendo reports
them, so Android's input stack (its generic key layout) and SDL's Android
driver see what a phone paired with Joy-Cons gives them. On a Google APIs
image, `adb root`, push it to `/data/local/tmp` and feed it lines such as
`open L`, `open R`, `tap L 314 200` (Minus), `abs L 0 30000` (the left
stick right) or `tap L 546 500` (the left arrow); logcat then says
`controller: Joy-Con (L), a Nintendo pad` as the game opens each.
