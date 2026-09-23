#!/bin/bash
# PortMaster launcher for Mega Man Battle Network: Cyberworld Endless.

XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}
if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  controlfolder="$XDG_DATA_HOME/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

source $controlfolder/control.txt
[ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"
get_controls

GAMEDIR="/$directory/ports/cyberworld"
cd "$GAMEDIR"
> "$GAMEDIR/log.txt" && exec > >(tee "$GAMEDIR/log.txt") 2>&1
$ESUDO chmod +x "$GAMEDIR/cyberworld.aarch64"

# Freedreno drives Qualcomm Adreno GPUs; other GPUs keep Mesa's own choice.
if [ -d /sys/module/msm ]; then
  export MESA_LOADER_DRIVER_OVERRIDE=msm
fi
export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"

$GPTOKEYB "cyberworld.aarch64" &
pm_platform_helper "$GAMEDIR/cyberworld.aarch64"
./cyberworld.aarch64 --rom-dir "$GAMEDIR/rom" --data-dir "$GAMEDIR"
pm_finish
