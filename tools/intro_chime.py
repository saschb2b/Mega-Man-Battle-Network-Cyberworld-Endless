#!/usr/bin/env python3
"""The start's chime: "UI Success Chime" by SoundShelfStudio, from Pixabay
(https://pixabay.com/sound-effects/ui-success-chime-513565/, under the
Pixabay Content License: free to use in a work, no attribution asked, not
to be passed on by itself), as the 48 kHz mono samples the engine plays at
the start (src/scenes/intro_logo.c), its silence after cut, with the
samples its two tones begin at: the boot screen shows the name's two
pieces on them.

Writes src/audio/intro_chime.c and src/audio/intro_chime.h, which are
committed; the sound file itself stays out of git. ffmpeg decodes it, in
the image `build.py clips` uses.

    python3 tools/intro_chime.py PATH/TO/ui-success-chime-513565.mp3
"""
import os
import shutil
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IMAGE = 'linuxserver/ffmpeg:9.0-cli-ls82'
RATE = 48000
SOURCE = ('"UI Success Chime" by SoundShelfStudio, from Pixabay\n'
          ' * (pixabay.com/sound-effects/ui-success-chime-513565/, the Pixabay Content\n'
          ' * License)')


def decode(path):
    """The file as 48 kHz mono 16-bit samples (staged in .build: Docker
    Desktop mounts no temporary folder)."""
    work = os.path.join(ROOT, '.build', 'chime')
    os.makedirs(work, exist_ok=True)
    shutil.copyfile(path, os.path.join(work, 'source'))
    subprocess.run(['docker', 'run', '--rm', '-u', f'{os.getuid()}:{os.getgid()}', '--entrypoint', 'ffmpeg',
                    '-v', f'{work}:/work', IMAGE, '-loglevel', 'error', '-y', '-i', '/work/source',
                    '-f', 's16le', '-ar', str(RATE), '-ac', '1', '/work/mono.raw'], check=True)
    with open(os.path.join(work, 'mono.raw'), 'rb') as f:
        raw = f.read()
    return list(struct.unpack(f'<{len(raw) // 2}h', raw))


def tones(s):
    """The samples the two tones begin at: the first loud one, then the
    steepest rise of the loudness (a millisecond's peak) a tenth of a
    second or more later, to the first sample of it."""
    first = next(i for i, v in enumerate(s) if abs(v) > 300)
    w = RATE // 1000
    peak = [max(abs(v) for v in s[k:k + w]) for k in range(0, len(s) - w, w)]
    k = max(range(first // w + 100, len(peak)), key=lambda k: peak[k] - peak[k - 2])
    second = next(i for i in range(k * w - w, k * w + w) if abs(s[i]) > 1.5 * peak[k - 2])
    return first, second


def trim(s):
    """Cut after the last audible sample and a 20 ms tail, faded over its
    last 10 ms."""
    last = max(i for i, v in enumerate(s) if abs(v) > 8)
    s = s[:min(len(s), last + RATE // 50)]
    fade = RATE // 100
    for i in range(fade):
        s[len(s) - fade + i] = int(s[len(s) - fade + i] * (fade - i) / fade)
    return s


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    s = decode(sys.argv[1])
    first, second = tones(s)
    s = trim(s)
    out = os.path.join(ROOT, 'src', 'audio')
    with open(os.path.join(out, 'intro_chime.h'), 'w') as f:
        f.write(f'''/* The start's chime: {SOURCE}, 48 kHz mono; written by tools/intro_chime.py.
 * Its two tones begin INTRO_CHIME_TONE1 and INTRO_CHIME_TONE2 samples in. */
#ifndef CW_INTRO_CHIME_H
#define CW_INTRO_CHIME_H

#include <stdint.h>

#define INTRO_CHIME_RATE {RATE}
#define INTRO_CHIME_LEN {len(s)}
#define INTRO_CHIME_TONE1 {first}
#define INTRO_CHIME_TONE2 {second}

extern const int16_t intro_chime[INTRO_CHIME_LEN];

#endif
''')
    with open(os.path.join(out, 'intro_chime.c'), 'w') as f:
        f.write('/* intro_chime.h: written by tools/intro_chime.py, not by hand */\n')
        f.write('#include "intro_chime.h"\n\nconst int16_t intro_chime[INTRO_CHIME_LEN] = {\n')
        for i in range(0, len(s), 16):
            f.write('\t' + ', '.join(str(v) for v in s[i:i + 16]) + ',\n')
        f.write('};\n')
    print(f'{len(s)} samples ({len(s) / RATE:.3f} s), tones at {first} and {second} '
          f'({first * 1000 / RATE:.1f} and {second * 1000 / RATE:.1f} ms)')


if __name__ == '__main__':
    main()
