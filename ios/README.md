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
[AltStore Classic](https://altstore.io) or
[SideStore](https://sidestore.io) (each needs a computer once; on iOS 16
and newer, Developer Mode too). AltStore PAL in the EU installs only
notarized apps, which this is not.

1. Add the source, in the app's **Sources** tab, **+**:
   `https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/latest/download/altstore-source.json`.
   On the phone, the download page's buttons open AltStore
   (`altstore://source?url=...`) or SideStore (`sidestore://source?url=...`)
   with it.
2. Install **Cyberworld Endless** from the source. A free Apple ID's apps
   last seven days; AltStore renews them in the background, and offers each
   new release the source lists.
3. Start it. Without a ROM it asks for one: **A** opens Files' picker,
   which copies the file picked into the app. Or put the ROM in Files,
   **On My iPhone › Cyberworld** (the app's Documents folder), any file
   name; the app looks again every three seconds.

`cyberworld-endless.ipa` on the releases page is the app alone, for
another installer (it is signed ad hoc: the installer signs it again).

## What it keeps where

The app's Documents folder, which Files shows as **On My iPhone ›
Cyberworld**, holds the ROM (`rom/`), the saves and `settings.ini`,
`keys.ini` and `touch.ini`, as a desktop's data folder does; a player can
copy the saves off there. Uninstalling the app deletes it.

## Building (on a Mac)

```sh
python3 build.py ios               # build/release/cyberworld-endless.ipa
python3 build.py ios --simulator   # build/ios-iphonesimulator/Payload/CyberworldEndless.app
```

Both need Xcode and CMake. The first build runs `ios/deps.sh`: SDL2 and
the GBA core as static arm64 libraries for the SDK (`iphoneos`, or
`iphonesimulator` for an Apple silicon Mac's Simulator) into
`.build/ios-deps/`. `make TARGET=ios` builds the game's C and
`src/core/ios.m` (the Files picker, the haptics, the safe area, and the
`main` that hands SDL's to UIKit); `build.py` makes the bundle from
`ios/Info.plist` and the icons in `ios/icons/` (`tools/app_icon.py`), signs
it ad hoc and zips `Payload/` into the IPA.

To try the Simulator's app:

```sh
xcrun simctl boot "iPhone 15"
xcrun simctl install booted build/ios-iphonesimulator/Payload/CyberworldEndless.app
xcrun simctl launch --console-pty booted io.github.saschb2b.cyberworldendless
```

and drag a ROM onto the Simulator's Files app, or into the app's Documents
(`xcrun simctl get_app_container booted io.github.saschb2b.cyberworldendless data`).

CI's `ios` job builds both, checks the IPA (arm64, iOS 14, only iOS's own
frameworks, the bundle signed) and starts the Simulator's app, which must
reach its no-ROM screen; its screenshot is in the job's artifact.

## The AltStore source

`tools/altstore_source.py` writes `altstore-source.json` for a release
(the release workflow attaches it beside the IPA): the app's bundle
identifier, version and build, which AltStore checks against the IPA it
downloads, its size, its date and no entitlements or privacy prompts,
which AltStore checks too.
