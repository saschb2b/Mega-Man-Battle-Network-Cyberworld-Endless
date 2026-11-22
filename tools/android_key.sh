#!/bin/sh
# tools/android_key.sh [KEYSTORE]: the Android app's release key, made once
# (android/README.md, Signing). keytool runs in the Android build image (the
# JDK), asks you for a password and writes the keystore outside the
# repository (default ~/cyberworld-release.jks); then the repository's two
# secrets are set with gh, the password typed again at gh's own prompt.
# Nothing is printed or kept here. Back up the keystore and the password
# (a password manager): a lost key means players reinstall, losing their
# saves, to update.
set -eu
KEY=${1:-$HOME/cyberworld-release.jks}
REPO=saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless
IMAGE=cyberworld-android
case "$KEY" in /*) ;; *) KEY=$PWD/$KEY ;; esac
if [ -e "$KEY" ]; then
	echo "$KEY exists already: it is the key, keep it (delete it only to start over)."
else
	dir=$(dirname "$KEY")
	docker image inspect "$IMAGE" >/dev/null 2>&1 || { echo "No $IMAGE image: run python3 build.py android once first."; exit 1; }
	echo "Making the key: keytool asks for a password, twice."
	docker run --rm -it --user "$(id -u):$(id -g)" -e HOME=/tmp -v "$dir:/keys" "$IMAGE" keytool -genkeypair -keystore "/keys/$(basename "$KEY")" \
		-alias cyberworld -keyalg RSA -keysize 4096 -validity 36500 -dname "CN=Cyberworld Endless"
	echo "The key is in $KEY."
fi
echo "Setting the repository's secrets (gh asks for the password once more)."
base64 -w0 "$KEY" | gh secret set ANDROID_KEYSTORE_BASE64 -R "$REPO"
gh secret set ANDROID_KEYSTORE_PASSWORD -R "$REPO"
gh secret list -R "$REPO"
echo "Done. Back up $KEY and its password somewhere safe."
