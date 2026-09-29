# The Android app

The same game as every other build, for Android phones, tablets and
handhelds (Android 5 and later; arm64, 32-bit ARM and x86-64): SDL's
activity (`org.libsdl.app.SDLActivity`, from SDL2's own source) runs the
game's C, built by the NDK as `libmain.so` with SDL2 and the GBA core.

- `RomActivity` starts the game when the player's ROM is kept, else asks
  for it once with Android's own file picker (no storage permission), checks
  its size and SHA-1 and copies it into the app's files. It never leaves
  the device; saves are kept beside it (`files/data`).
- `GameActivity` is SDL's activity with the ROM and data folders as its
  arguments; it keeps the screen on.
- A controller (a handheld's own controls, Bluetooth or USB) works as on the
  desktop builds. Without one the game draws its touch controls from the
  start; a controller's button puts them away, a touch brings them back.
  Back asks before it quits, as Escape does elsewhere.

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
signature, and uninstalling deletes the app's saves, so every release has to
be signed with one key. `build.py android` signs with it when these are set:

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
means players reinstall (and lose their saves) to update.

## Installing

Copy the APK to the device and open it (Android asks once to allow installs
from the file manager or browser), or with a cable: `adb install -r
cyberworld-endless.apk`. The first start asks for the ROM: Mega Man Battle
Network 6: Cybeast Gregar (USA), unmodified, the same file every other build
takes.
