#!/usr/bin/env python3
"""Drive the game on a device through a virtual uinput keyboard.

Runs on the device (standard library only). Commands, separated by ';':
  wait SECONDS        tap KEY [SECONDS]        hold KEY SECONDS
  shot PATH           (runs `grim PATH`)
Keys: UP DOWN LEFT RIGHT A B L R START SELECT (mapped to the game's keyboard
layout) or raw evdev names such as KEY_X.

  python3 uinput_keys.py "wait 2; tap START; wait 1; hold RIGHT 0.8; shot /tmp/a.png"
"""
import fcntl
import os
import struct
import subprocess
import sys
import time

KEYS = {
    'KEY_ENTER': 28, 'KEY_Z': 44, 'KEY_X': 45, 'KEY_A': 30, 'KEY_S': 31,
    'KEY_RIGHTSHIFT': 54, 'KEY_UP': 103, 'KEY_LEFT': 105, 'KEY_RIGHT': 106, 'KEY_DOWN': 108,
}
GAME = {
    'UP': 'KEY_UP', 'DOWN': 'KEY_DOWN', 'LEFT': 'KEY_LEFT', 'RIGHT': 'KEY_RIGHT',
    'A': 'KEY_X', 'B': 'KEY_Z', 'L': 'KEY_A', 'R': 'KEY_S', 'START': 'KEY_ENTER', 'SELECT': 'KEY_RIGHTSHIFT',
}
EV_SYN, EV_KEY = 0, 1
UI_SET_EVBIT, UI_SET_KEYBIT = 0x40045564, 0x40045565
UI_DEV_CREATE, UI_DEV_DESTROY = 0x5501, 0x5502


def open_device():
    fd = os.open('/dev/uinput', os.O_WRONLY | os.O_NONBLOCK)
    fcntl.ioctl(fd, UI_SET_EVBIT, EV_KEY)
    for code in KEYS.values():
        fcntl.ioctl(fd, UI_SET_KEYBIT, code)
    name = b'cyberworld-test-keyboard'
    # struct uinput_user_dev: name[80], input_id (4 x u16), ff_effects_max, abs arrays
    dev = struct.pack('80sHHHHi', name, 0x03, 0x1234, 0x5678, 1, 0) + b'\0' * (4 * 64 * 4)
    os.write(fd, dev)
    fcntl.ioctl(fd, UI_DEV_CREATE)
    time.sleep(0.8)  # let the compositor pick the device up
    return fd


def emit(fd, etype, code, value):
    now = time.time()
    os.write(fd, struct.pack('llHHi', int(now), int((now % 1) * 1e6), etype, code, value))


def key(fd, name, down):
    code = KEYS[GAME.get(name, name)]
    emit(fd, EV_KEY, code, 1 if down else 0)
    emit(fd, EV_SYN, 0, 0)


def main():
    fd = open_device()
    try:
        for step in sys.argv[1].split(';'):
            parts = step.split()
            if not parts:
                continue
            op = parts[0]
            if op == 'wait':
                time.sleep(float(parts[1]))
            elif op == 'tap':
                key(fd, parts[1], True)
                time.sleep(float(parts[2]) if len(parts) > 2 else 0.08)
                key(fd, parts[1], False)
                time.sleep(0.05)
            elif op == 'hold':
                key(fd, parts[1], True)
                time.sleep(float(parts[2]))
                key(fd, parts[1], False)
            elif op == 'shot':
                subprocess.run(['grim', parts[1]], check=False)
    finally:
        fcntl.ioctl(fd, UI_DEV_DESTROY)
        os.close(fd)


if __name__ == '__main__':
    main()
