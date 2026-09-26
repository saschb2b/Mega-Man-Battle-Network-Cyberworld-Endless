#!/usr/bin/env python3
"""Play the game from the command line, a batch of input at a time: for
playtests by a person or an agent, and for reproducing what they found.

  tools/play.py start NAME [--seed S] [--fresh] [-- GAME_ARGS...]
      a headless game at the title screen, its files in .build/play/NAME
      (data/ keeps the saves and profile between sessions; --fresh starts
      them over, as a first-time player)
  tools/play.py do NAME "COMMANDS" [--every N]
      runs the commands, then prints what the game shows in words and the
      path of a picture of the screen (with --every N, a sheet of pictures
      taken every N frames)
  tools/play.py stop NAME

Commands, apart by ';':
  press BTN [N]   hold BTN N frames (6), then let go 6 frames
  hold BTN N      hold BTN N frames
  wait N          N frames without input
  mash BTN N      press BTN every 10 frames for N frames
  shot            a picture now (one is always taken at the end)
BTN: A B L R START SELECT UP DOWN LEFT RIGHT, or several with + (UP+RIGHT).
The keyboard's layout is the handheld's buttons: A talks and confirms, B
runs (hold) and cancels, START opens the PET, L and R open the Custom
screen in battle.

Every command is kept in NAME/history.txt; with the seed and NAME/data0
(the data folder as the session began) a session replays exactly.
"""
import os
import shutil
import signal
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BINARY = os.path.join(ROOT, 'build', 'linux', 'cyberworld')
ROM_DIR = os.environ.get('CYBERWORLD_ROM_DIR', os.path.expanduser('~/.cache/mmbn-ref/roms'))
BUTTONS = {'A', 'B', 'L', 'R', 'START', 'SELECT', 'UP', 'DOWN', 'LEFT', 'RIGHT'}
SCALE = 3


def home(name):
    return os.path.join(ROOT, '.build', 'play', name)


def buttons(spec):
    parts = [p.strip().upper() for p in spec.split('+')]
    bad = [p for p in parts if p not in BUTTONS]
    if bad:
        sys.exit(f'unknown button {bad[0]} (buttons: {" ".join(sorted(BUTTONS))})')
    return '+'.join(parts)


def steps(commands):
    """Player commands -> (frames, buttons) steps and 'shot' marks."""
    out = []
    for cmd in commands.split(';'):
        words = cmd.split()
        if not words:
            continue
        op, args = words[0].lower(), words[1:]
        try:
            if op == 'press':
                out += [(int(args[1]) if len(args) > 1 else 6, buttons(args[0])), (6, '')]
            elif op == 'hold':
                out.append((int(args[1]), buttons(args[0])))
            elif op == 'wait':
                out.append((int(args[0]), ''))
            elif op == 'mash':
                for _ in range(max(1, int(args[1]) // 10)):
                    out += [(4, buttons(args[0])), (6, '')]
            elif op == 'shot':
                out.append('shot')
            else:
                sys.exit(f'unknown command {op!r} (press, hold, wait, mash, shot)')
        except (IndexError, ValueError):
            sys.exit(f'bad command {cmd.strip()!r}')
    return out


def split_every(seq, every):
    """Pictures every `every` frames through the steps."""
    out, since = [], 0
    for s in seq:
        if s == 'shot':
            out.append(s)
            since = 0
            continue
        frames, btn = s
        while since + frames >= every:
            take = every - since
            if take:
                out.append((take, btn))
            out.append('shot')
            frames -= take
            since = 0
        if frames:
            out.append((frames, btn))
            since += frames
    return out


def talk(h, line, timeout=300):
    """Sends one batch; waits for its answer."""
    proc = os.path.join(h, 'pid')
    with open(os.path.join(h, 'in'), 'w') as f:
        f.write(line + '\n')
    fd = os.open(os.path.join(h, 'out'), os.O_RDONLY | os.O_NONBLOCK)
    got, start = b'', time.time()
    try:
        while not got.endswith(b'\n'):
            try:
                c = os.read(fd, 1)
            except BlockingIOError:
                c = b''
            if c:
                got += c
                continue
            if time.time() - start > timeout:
                sys.exit('the game did not answer in time')
            pid = int(open(proc).read())
            try:
                os.kill(pid, 0)
            except OSError:
                sys.exit('the game has stopped (see log.txt)')
            time.sleep(0.005)
    finally:
        os.close(fd)
    return got.decode().strip()


def picture(paths, out):
    from PIL import Image, ImageDraw
    frames = [Image.open(p).convert('RGB') for p in paths]
    if len(frames) == 1:
        img = frames[0].resize((frames[0].width * SCALE, frames[0].height * SCALE), Image.NEAREST)
    else:
        cols = min(4, len(frames))
        rows = (len(frames) + cols - 1) // cols
        w, h = frames[0].width * 2, frames[0].height * 2
        img = Image.new('RGB', (w * cols, h * rows))
        draw = ImageDraw.Draw(img)
        for i, fr in enumerate(frames):
            x, y = (i % cols) * w, (i // cols) * h
            img.paste(fr.resize((w, h), Image.NEAREST), (x, y))
            draw.text((x + 4, y + h - 12), str(i + 1), fill=(255, 255, 0))
    img.save(out)
    for p in paths:
        os.remove(p)


def cmd_start(name, rest):
    h = home(name)
    seed, fresh, extra = 0, False, []
    if '--' in rest:
        extra = rest[rest.index('--') + 1:]
        rest = rest[:rest.index('--')]
    if '--seed' in rest:
        seed = int(rest[rest.index('--seed') + 1], 0)
    fresh = '--fresh' in rest
    if os.path.exists(os.path.join(h, 'pid')):
        cmd_stop(name)
    data = os.path.join(h, 'data')
    if fresh and os.path.isdir(data):
        shutil.rmtree(data)
    os.makedirs(data, exist_ok=True)
    os.makedirs(os.path.join(h, 'shots'), exist_ok=True)
    for f in ('in', 'out'):
        if os.path.exists(os.path.join(h, f)):
            os.remove(os.path.join(h, f))
    if os.path.isdir(os.path.join(h, 'data0')):
        shutil.rmtree(os.path.join(h, 'data0'))
    shutil.copytree(data, os.path.join(h, 'data0'))
    if not os.path.exists(BINARY):
        sys.exit('no build/linux/cyberworld: run python3 build.py linux first')
    # the session's own copy, so a rebuild meanwhile leaves it alone
    bin_dir = os.path.join(h, 'bin')
    if os.path.isdir(bin_dir):
        shutil.rmtree(bin_dir)
    os.makedirs(bin_dir)
    shutil.copy2(BINARY, bin_dir)
    shutil.copytree(os.path.join(os.path.dirname(BINARY), 'lib'), os.path.join(bin_dir, 'lib'))
    if not seed:
        seed = int.from_bytes(os.urandom(3), 'little') | 1
    args = [os.path.join(bin_dir, 'cyberworld'), '--headless', '--size', '256x192', '--remote', h, '--rom-dir', ROM_DIR,
            '--data-dir', data, '--seed', str(seed), *extra]
    log = open(os.path.join(h, 'log.txt'), 'w')
    p = subprocess.Popen(args, stdout=log, stderr=subprocess.STDOUT, start_new_session=True, cwd=ROOT)
    open(os.path.join(h, 'pid'), 'w').write(str(p.pid))
    with open(os.path.join(h, 'history.txt'), 'w') as f:
        f.write(f'# seed {seed}\n# args {" ".join(extra)}\n')
    for _ in range(500):
        if os.path.exists(os.path.join(h, 'out')):
            break
        time.sleep(0.01)
    print(f'started {name} (seed {seed})')
    cmd_do(name, ['wait 1'])


def cmd_do(name, rest):
    h = home(name)
    if not os.path.exists(os.path.join(h, 'pid')):
        sys.exit(f'no game {name}: start it first')
    every = 0
    if '--every' in rest:
        every = int(rest[rest.index('--every') + 1])
        rest = rest[:rest.index('--every')] + rest[rest.index('--every') + 2:]
    commands = ' ; '.join(rest)
    seq = steps(commands)
    if every:
        seq = split_every(seq, every)
    if not seq or seq[-1] != 'shot':
        seq.append('shot')
    shots, items = [], []
    n = len(os.listdir(os.path.join(h, 'shots')))
    for s in seq:
        if s == 'shot':
            path = os.path.join(h, 'shots', f'_{n}_{len(shots)}.bmp')
            shots.append(path)
            items.append(f'shot {path}')
        else:
            items.append(f'{s[0]} {s[1]}'.strip())
    state = os.path.join(h, 'state.txt')
    items.append(f'state {state}')
    with open(os.path.join(h, 'history.txt'), 'a') as f:
        f.write(commands + (f'  # every {every}' if every else '') + '\n')
    answer = talk(h, ';'.join(items))
    out = os.path.join(h, 'shots', f'{n:04d}.png')
    picture(shots, out)
    print(open(state).read().strip())
    print(f'picture {out}')
    if not answer.startswith('ok'):
        print('answer:', answer)


def cmd_stop(name):
    h = home(name)
    try:
        pid = int(open(os.path.join(h, 'pid')).read())
        os.killpg(pid, signal.SIGTERM)
    except (OSError, ValueError):
        pass
    if os.path.exists(os.path.join(h, 'pid')):
        os.remove(os.path.join(h, 'pid'))
    print(f'stopped {name}')


def main():
    if len(sys.argv) < 3 or sys.argv[1] not in ('start', 'do', 'stop'):
        sys.exit(__doc__)
    op, name, rest = sys.argv[1], sys.argv[2], sys.argv[3:]
    {'start': cmd_start, 'do': cmd_do, 'stop': lambda n, r: cmd_stop(n)}[op](name, rest)


if __name__ == '__main__':
    main()
