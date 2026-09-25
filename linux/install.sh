#!/bin/sh
# Adds Cyberworld Endless to the desktop's application menu, with its icon,
# pointing at this folder. Run it again after moving the folder;
# ./install.sh --remove takes the entry and the icons away again.
set -e
id=io.github.saschb2b.CyberworldEndless
here=$(cd "$(dirname "$0")" && pwd)
share="${XDG_DATA_HOME:-$HOME/.local/share}"
apps="$share/applications"
if [ "$1" = --remove ]; then
	rm -f "$apps/$id.desktop" "$apps/cyberworld-endless.desktop"
	for s in 32 64 128 256 512; do rm -f "$share/icons/hicolor/${s}x$s/apps/$id.png"; done
	echo "Removed Cyberworld Endless from the application menu."
	exit 0
fi
mkdir -p "$apps"
for s in 32 64 128 256 512; do
	mkdir -p "$share/icons/hicolor/${s}x$s/apps"
	cp "$here/icons/$s.png" "$share/icons/hicolor/${s}x$s/apps/$id.png"
done
# the name older versions used
rm -f "$apps/cyberworld-endless.desktop"
cat > "$apps/$id.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Cyberworld Endless
GenericName=Roguelike
Comment=A roguelike on Mega Man Battle Network 6 (bring your own ROM)
Exec="$here/cyberworld-endless"
Path=$here
Icon=$id
Terminal=false
Categories=Game;ActionGame;RolePlaying;
Keywords=Mega Man;Battle Network;MMBN;roguelike;GBA;
StartupWMClass=$id
DESKTOP
update-desktop-database "$apps" >/dev/null 2>&1 || true
echo "Added Cyberworld Endless to the application menu ($apps/$id.desktop)."
