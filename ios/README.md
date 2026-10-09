# Cyberworld Endless on iPhone and iPad

The game as an iOS app (iOS 14 and newer, arm64: every iPhone and iPad
since the iPhone 6s and the fifth iPad), with touch controls round the
picture, controllers through Apple's GameController (MFi, Xbox,
PlayStation) and the same saves as every other build. It is built on CI's
Mac (`build.py ios`) and has been started in the iOS Simulator there, not
yet on a device.

## Installing

Apple's App Store takes no app that runs on a ROM, so it comes through a
sideloading store that installs apps with the player's own Apple ID:
[SideStore](https://sidestore.io), set up once from a computer by
[iloader](https://iloader.app) (Windows, macOS or Linux, over a USB cable;
then the LocalDevVPN app on the phone lets SideStore install and renew
apps by itself), or [AltStore Classic](https://altstore.io), with AltServer
on a Windows PC or a Mac. On iOS 16 and newer, Developer Mode too.
AltStore PAL, the EU's App Marketplace edition, installs only apps Apple
has notarized: it refuses the source with "missing a marketplaceID"
(a player's test in Germany, 2026-10-02).

1. Add the source, in the app's **Sources** tab, **+**:
   `https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/latest/download/altstore-source.json`.
   On the phone, the download page's buttons open SideStore
   (`sidestore://source?url=...`) or AltStore Classic
   (`altstore://source?url=...`) with it.
2. Install **Cyberworld Endless** from the source. A free Apple ID's apps
   last seven days; SideStore and AltStore renew them, and offer each new
   release the source lists.
3. Start it: the ROMs screen opens, its two cartridge slots open spots
   until their ROMs are in (the guide's The ROMs screen, docs/GUIDE.md). BN6's slot (a
   tap, or A) opens Files' picker for the folder the ROMs are in. The app
   opens only its `.gba` files (and those of the folders directly in
   it), copies Mega Man Battle Network 6: Cybeast Gregar (USA) and, if
   it is there, Mega Man Battle Network 5: Team Colonel (USA) in, each checked by
   its SHA-1, and names every other `.gba` with why it was refused. The
   folder is kept (a bookmark) and looked in again at each start, so BN5
   put there later comes in by itself; R on the title opens the ROMs
   screen again. **FILES INSTEAD** picks the files instead, more than one
   at once. Or put the ROMs in Files, **On My iPhone › Cyberworld** (the
   app's Documents folder), any file name; the app looks there, and in
   the folder chosen, every three seconds.

`cyberworld-endless.ipa` on the releases page is the app alone, for
another installer (it is signed ad hoc: the installer signs it again).

## What it keeps where

The app's Documents folder, which Files shows as **On My iPhone ›
Cyberworld**, holds the ROMs' copies (`rom/bn6g.gba`, `rom/bn5c.gba`),
the saves and `settings.ini`,
`keys.ini` and `touch.ini`, as a desktop's data folder does; a player can
copy the saves off there. Uninstalling the app deletes it. With
**Auto-export** on, the app keeps a copy in the transfer folder shown in
**SAVES**, using the ROM folder by default; **Choose folder** can select
another destination. The copy is
`cyberworld-endless.cwsave` (`src/core/backup.h`, the format the Android
app and the browser's backups share), written a few seconds after the
game last saved and as the app goes to the background, through the
folder's bookmark as a coordinated write that replaces the old copy
whole. After a reinstall, select the ROMs again and use **SAVES → Import**
to choose that file. A ROM folder chosen that holds one also has it
copied to `found.cwsave`, and the ROMs screen opens SAVES to compare it
with this device. SAVES on the title also exports and imports that portable file
and controls its automatic copy. Imported progress keeps the receiving
device's controls, display, volumes and statistics answer; Undo last import
brings back what was replaced. See
[Moving saves between devices](../docs/GUIDE.md#moving-saves-between-devices).

## Building (on a Mac)

```sh
python3 build.py ios               # build/release/cyberworld-endless.ipa
python3 build.py ios --simulator   # build/ios-iphonesimulator/Payload/CyberworldEndless.app
```

Both need Xcode and CMake. The first build runs `ios/deps.sh`: SDL2 and
the GBA core as static arm64 libraries for the SDK (`iphoneos`, or
`iphonesimulator` for an Apple silicon Mac's Simulator) into
`.build/ios-deps/`. `make TARGET=ios` builds the game's C and
`src/core/ios.m` (the Files pickers and the ROM folder's looks, the
saves' copy in that folder, the haptics, the safe area, and the
`main` that hands SDL's to UIKit); `build.py` makes the bundle from
`ios/Info.plist` and the icons in `ios/icons/` (`tools/app_icon.py`), signs
it ad hoc and zips `Payload/` into the IPA.

To try the Simulator's app:

```sh
xcrun simctl boot "iPhone 15"
xcrun simctl install booted build/ios-iphonesimulator/Payload/CyberworldEndless.app
xcrun simctl launch --console-pty booted io.github.saschb2b.cyberworldendless
```

and drag the ROMs onto the Simulator's Files app (a folder of BN6's and
BN5's, one with only a wrong `.gba`, and BN5 added to the folder after the
first start), or into the app's Documents
(`xcrun simctl get_app_container booted io.github.saschb2b.cyberworldendless data`).
The console (`--console-pty`) shows each look: `ROM look in ROMs: 2 .gba,
kept 3, ...` (bit 1 BN6's, bit 2 BN5's) and the game's `BN5 found: ...`.

CI's `ios` job builds both, checks the IPA (arm64, iOS 14, only iOS's own
frameworks, the bundle signed) and starts the Simulator's app, which must
reach its ROMs screen; its screenshot is in the job's artifact.

## The AltStore source

`tools/altstore_source.py` writes `altstore-source.json` for a release
(the release workflow attaches it beside the IPA): the app's bundle
identifier, version and build, which AltStore checks against the IPA it
downloads, its size, its date and no entitlements or privacy prompts,
which AltStore checks too.
