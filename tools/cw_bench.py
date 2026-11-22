#!/usr/bin/env python3
"""The agents' tools' benchmark (docs/devtools/agents-tools.md): five
tasks agents do all the time, each the way it was done before the tools
(build.py shot, PIL by hand, play.py and a Read of each picture) and
through the MCP server, as an agent's calls: each call's wall time, the
text it returns and the pictures, with what reading them costs (a
picture about its pixels over 750 in tokens, text about 4 bytes a token,
a shell's output past 30000 characters cut, as Claude Code cuts it).

    python3 tools/cw_bench.py [TASK ...]   (the ROM; Docker for the builds)

It touches src/core/options_text.c (its time only) for the rebuild's task,
and builds REF's commit once in .build/cw/ref for the comparison's.
"""
import base64
import json
import os
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import cw_pictures as pics  # noqa: E402

B = os.path.join(ROOT, '.build', 'bench')
REF = os.environ.get('CW_BENCH_REF', 'd89dc612~1')
CUT = 30000
AUDIO = '60:,4:SELECT,40:,4:DOWN,30:,4:DOWN,30:,4:A,60:'


class Tally:
    def __init__(self, task, way):
        self.task, self.way, self.calls = task, way, []

    def add(self, what, seconds, text=b'', pictures=()):
        text_tokens = min(len(text), CUT) // 4
        pic_tokens = sum(pics.tokens(size) for size in pictures)
        self.calls.append((what, seconds, len(text), list(pictures), text_tokens + pic_tokens))

    def row(self):
        n = len(self.calls)
        secs = sum(c[1] for c in self.calls)
        text = sum(c[2] for c in self.calls)
        pictures = [p for c in self.calls for p in c[3]]
        tokens = sum(c[4] for c in self.calls)
        return dict(task=self.task, way=self.way, calls=n, seconds=round(secs, 1), text=text,
                    pictures=len(pictures), sizes=sorted({f'{w}x{h}' for w, h in pictures}), tokens=tokens,
                    steps=[dict(call=c[0], seconds=round(c[1], 2), text=c[2], pictures=c[3], tokens=c[4]) for c in self.calls])


def shell(t, what, cmd, env=None, cwd=ROOT):
    s = time.time()
    p = subprocess.run(cmd, shell=True, cwd=cwd, capture_output=True, env={**os.environ, **(env or {})})
    t.add(what, time.time() - s, p.stdout + p.stderr)
    return (p.stdout + p.stderr).decode(errors='replace')


def read(t, path):
    """The Read tool on a picture: no time to speak of, its pixels' tokens."""
    with pics.Image.open(path) as im:
        t.add(f'Read {os.path.basename(path)}', 0.0, b'', [im.size])


def convert(t, src, dst, scale=3):
    shell(t, 'convert BMP to PNG', f'python3 -c "from PIL import Image; im=Image.open(\'{src}\'); '
          f'im.resize((im.width*{scale}, im.height*{scale}), Image.NEAREST).save(\'{dst}\')"')


class Server:
    """The MCP server as Claude Code runs it: one process, a call a line."""

    def __init__(self):
        self.p = subprocess.Popen([sys.executable, os.path.join(ROOT, 'tools', 'cw_mcp.py')], cwd=ROOT, text=True,
                                  stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        self.n = 0
        self.ask('initialize', {'protocolVersion': '2025-06-18', 'capabilities': {}, 'clientInfo': {'name': 'bench', 'version': '1'}})

    def ask(self, method, params):
        self.n += 1
        self.p.stdin.write(json.dumps({'jsonrpc': '2.0', 'id': self.n, 'method': method, 'params': params}) + '\n')
        self.p.stdin.flush()
        return json.loads(self.p.stdout.readline())

    def call(self, t, tool, **args):
        s = time.time()
        r = self.ask('tools/call', {'name': tool, 'arguments': args})['result']
        text = ''.join(c['text'] for c in r['content'] if c['type'] == 'text').encode()
        sizes = []
        for c in r['content']:
            if c['type'] == 'image':
                import io
                with pics.Image.open(io.BytesIO(base64.b64decode(c['data']))) as im:
                    sizes.append(im.size)
        t.add(f'{tool} {json.dumps(args)[:60]}', time.time() - s, text, sizes)
        return text.decode()

    def close(self):
        self.p.stdin.close()
        self.p.wait()


def before_audio():
    t = Tally('Options\' Audio from the title', 'before')
    shell(t, 'build.py shot', f'python3 build.py shot --scene title --input "{AUDIO}" --frames 300 --shot "290:/src/.build/bench/a.bmp"')
    convert(t, '.build/bench/a.bmp', '.build/bench/a.png')
    read(t, os.path.join(B, 'a.png'))
    return t


def after_audio(s):
    t = Tally('Options\' Audio from the title', 'after')
    s.call(t, 'shot', start='options/audio')
    return t


JUKEBOX_STEPS = ['mash A 460; wait 30', 'press START; wait 40', 'press DOWN; press DOWN; press DOWN; press DOWN; press DOWN; press A; wait 40',
                 'press DOWN; press RIGHT; wait 20; press A; wait 30', 'press A; wait 330']


def before_jukebox():
    t = Tally('The Jukebox in a new run, a song played', 'before')
    shell(t, 'build.py linux (play.py runs its build)', 'python3 build.py linux')
    out = shell(t, 'play.py start', 'python3 tools/play.py start bjb --fresh -- --scene town --seed 2')
    for cmd in [None] + JUKEBOX_STEPS:
        if cmd:
            out = shell(t, f'play.py do {cmd[:30]}', f'python3 tools/play.py do bjb "{cmd}"')
        read(t, out.split('picture ')[-1].split()[0])
    shell(t, 'play.py stop', 'python3 tools/play.py stop bjb')
    return t


def after_jukebox(s):
    t = Tally('The Jukebox in a new run, a song played', 'after')
    s.call(t, 'play', name='ajb', start='jukebox', fresh=True, do='press A; wait 330')
    s.call(t, 'play', name='ajb', do='stop')
    return t


def before_autopilot():
    t = Tally('The autopilot walks a layer to its exit', 'before')
    # (the frames guessed: the run's log says nothing of the layer's end, so the picture is read to know)
    shell(t, 'build.py shot (autopilot)', 'python3 build.py shot --scene emu --seed 3 --frames 2401 --shot "2400:/src/.build/bench/c.bmp"',
          env={'CYBERWORLD_AUTOPILOT': '1'})
    convert(t, '.build/bench/c.bmp', '.build/bench/c.png')
    read(t, os.path.join(B, 'c.png'))
    return t


def after_autopilot(s):
    t = Tally('The autopilot walks a layer to its exit', 'after')
    s.call(t, 'shot', start='CYBERWORLD_AUTOPILOT=1 layer --seed 3', until='layer>=2')
    return t


def before_compare():
    t = Tally(f'A screen before and after ({REF})', 'before')
    old = os.path.join(B, 'old')
    if os.path.isdir(old):
        subprocess.run(['git', 'worktree', 'remove', '--force', old], cwd=ROOT, capture_output=True)
    shell(t, 'git worktree add', f'git worktree add -q --detach {old} {REF}')
    shell(t, 'build.py host (old)', 'python3 build.py host', cwd=old)
    jukebox = ('40:,' + '4:A,6:,' * 46 + '30:,6:START,40:,' + '6:DOWN,6:,' * 5 + '6:A,40:,6:DOWN,6:,6:RIGHT,20:,6:A,40:')
    for where, d in (('old', old), ('new', ROOT)):
        shell(t, f'build.py shot ({where})', f'python3 build.py shot --scene town --seed 2 --input "{jukebox}" --frames 900 '
              f'--shot "890:/src/.build/{where}.bmp"', cwd=d)
    shell(t, 'stitch with PIL', f'python3 -c "from PIL import Image; a=Image.open(\'{old}/.build/old.bmp\'); b=Image.open(\'.build/new.bmp\'); '
          's=Image.new(\'RGB\',(a.width*2+4,a.height)); s.paste(a,(0,0)); s.paste(b,(a.width+4,0)); '
          's.resize((s.width*2,s.height*2),Image.NEAREST).save(\'.build/bench/d.png\')"')
    read(t, os.path.join(B, 'd.png'))
    subprocess.run(['git', 'worktree', 'remove', '--force', old], cwd=ROOT, capture_output=True)
    return t


def after_compare(s, cold=True):
    t = Tally(f'A screen before and after ({REF})' + ('' if cold else ', its build cached'), 'after')
    if cold:
        subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'cw.py'), 'prune'], cwd=ROOT, capture_output=True)
    s.call(t, 'shot', start='jukebox', ref=REF)
    return t


def touch():
    os.utime(os.path.join(ROOT, 'src', 'core', 'options_text.c'))


def before_rebuild():
    t = Tally('A one-line change built and seen', 'before')
    touch()
    shell(t, 'build.py host', 'python3 build.py host')
    shell(t, 'build.py shot', f'python3 build.py shot --scene title --input "{AUDIO}" --frames 300 --shot "290:/src/.build/bench/a.bmp"')
    convert(t, '.build/bench/a.bmp', '.build/bench/a.png')
    read(t, os.path.join(B, 'a.png'))
    return t


def after_rebuild(s):
    t = Tally('A one-line change built and seen', 'after')
    touch()
    s.call(t, 'shot', start='options/audio')
    return t


TASKS = {'audio': (before_audio, after_audio), 'jukebox': (before_jukebox, after_jukebox),
         'autopilot': (before_autopilot, after_autopilot), 'compare': (before_compare, after_compare),
         'rebuild': (before_rebuild, after_rebuild)}


def main():
    os.makedirs(B, exist_ok=True)
    names = sys.argv[1:] or list(TASKS)
    subprocess.run([sys.executable, 'build.py', 'host'], cwd=ROOT, capture_output=True)
    rows = []
    for name in names:
        before, after = TASKS[name]
        rows.append(before().row())
        s = Server()
        rows.append(after(s).row())
        if name == 'compare':
            rows.append(after_compare(s, cold=False).row())
        s.close()
    print('| Task | Way | Calls | Seconds | Text bytes | Pictures | Tokens read, about |')
    print('| --- | --- | ---: | ---: | ---: | --- | ---: |')
    for r in rows:
        print(f"| {r['task']} | {r['way']} | {r['calls']} | {r['seconds']} | {r['text']} | "
              f"{r['pictures']} ({', '.join(r['sizes'])}) | {r['tokens']} |")
    with open(os.path.join(B, 'bench.json'), 'w') as f:
        json.dump(rows, f, indent=1)


if __name__ == '__main__':
    main()
