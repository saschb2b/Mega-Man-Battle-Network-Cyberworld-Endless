---
type: Tool
title: "The controller's feel: what was sent"
description: "`--dev feel` prints each beat, rumble and DualSense trigger effect the controller's feel sends, and `--pad ps5` attaches a virtual DualSense that says what reached SDL, since nobody can feel a controller headless."
tags: [devtools]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-25T00:00:00Z }
---

# The controller's feel: what was sent

```bash
CYBERWORLD_AUTOPILOT=1 python3 build.py shot --scene emu --run-depth 1 --pad ps5 --dev feel --frames 6000 --input "0:battle"
python3 tools/cw.py shot "options/audio --pad ps5"   # Options' Vibration and Trigger effects
```

Nobody can feel a controller headless, so `--dev feel` says what the
game's rumble and a DualSense's triggers were sent ([The player's guide](../GUIDE.md#rumble-and-triggers); `src/core/feel_out.c`), a line each on stdout: a beat as
BN6's state raised it, its strength 0-255 (`feel: frame 2340: delete
208`), each change of the motors, the heavy one first (`feel: frame 2341:
rumble 27756 15934`), and each trigger effect (`feel: frame 2792:
triggers L2 gauge, R2 off`, "not taken" where SDL refused it: an SDL
before 2.0.16, or Steam Input's pad in a DualSense's place). The virtual
pads (`--pad`) say what reached SDL as well: `virtual pad: rumble 27756
15934`, and a DualSense's 47 bytes of effects, the right trigger's mode
and numbers (byte 10 on) and the left's (21 on), `25 24 00 05` the
gauge's click (mode 0x25, zones 2 and 5, strength 6). The rules (which
change raises which beat, each beat's shape, the strengths, the
triggers' bytes) are `test_feel` in `tests/test_core.c`. A device is the
owner's: `build.py run` with a DualSense on USB or Bluetooth, Steam Input
off for it ([The player's guide](../GUIDE.md#rumble-and-triggers)).
