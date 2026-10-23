#!/usr/bin/env python3
"""The agents' tools without the ROM (tools/cw.py, cw_mcp.py, cw_routes.py,
cw_pictures.py): every route's input reads as play.py's commands, a start's
words, a state's changes and an until's tests, the pictures' cut, sheet and
comparison, a build's complaints told from its command lines, and the MCP
server's answers. Standard library and Pillow; build.py test runs it."""
import json
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import cw  # noqa: E402
import cw_mcp  # noqa: E402
import cw_pictures as pics  # noqa: E402
import cw_routes  # noqa: E402
import play  # noqa: E402

failed = 0


def check(ok, what):
    global failed
    if not ok:
        failed += 1
        print('FAIL', what)


def routes():
    for name, (start, commands, screen) in cw_routes.ROUTES.items():
        seq = play.steps(commands)
        check(seq and screen and all(isinstance(a, str) for a in start), f'route {name} reads')
        check('--scene' in start, f'route {name} names its scene')
    try:
        cw_routes.route('nowhere')
        check(False, 'an unknown route is refused')
    except SystemExit as e:
        check('jukebox' in str(e.code), 'an unknown route names the routes')


def starts():
    commands, args, env, screen = cw.parse_start('CYBERWORLD_AUTOPILOT=1 layer --seed 3')
    check(env == {'CYBERWORLD_AUTOPILOT': '1'}, 'a start\'s variables')
    check(args[:2] == ['--scene', 'emu'] and args[-2:] == ['--seed', '3'], 'a route\'s options, then the given ones')
    check(screen == 'map' and 'mash A' in commands, 'a route\'s input and screen')
    commands, args, env, screen = cw.parse_start('--scene emu --net-biome 3')
    check(not commands and screen is None and args == ['--scene', 'emu', '--net-biome', '3'], 'options alone')
    seed, rest = cw.take_seed(['--scene', 'town', '--seed', '2', '--seed', '0x10'], 1)
    check(seed == 16 and rest == ['--scene', 'town'], 'the last --seed taken out')
    check(cw.take_seed(['--scene', 'title'], 1) == (1, ['--scene', 'title']), 'the default seed')


def states():
    old = 'frame 10\nscreen map\ndoing walking\nhp 100/100\nlayer 1\nnear object 3 4\nnear folk 5 6\n'
    new = 'frame 90\nscreen battle\ndoing battle\nhp 80/100\nlayer 1\ncustom gauge 50%\n'
    got = cw.changes(old, new)
    check('doing battle (was walking)' in got and 'hp 80/100 (was 100/100)' in got, 'a changed line says what it was')
    check('custom gauge 50%' in got and not any(g.startswith(('layer', 'frame', 'screen')) for g in got), 'new lines; the same left out')
    check('near gone' in got, 'a word gone')
    check(cw.changes('', new)[0] == 'doing battle', 'a first state whole')
    check(cw.headline(new, 80) == 'frame 90 · screen battle · 80 frames run', 'the headline')
    t = cw.condition('layer>=2|screen=battle')
    check(t(new) and not t(old), 'an until\'s any')
    check(cw.condition('hp<90')(new) and not cw.condition('hp<90')(old), 'a number\'s first figures')
    check(cw.condition('doing~bat')(new) and cw.condition('doing!=walking')(new), 'contains, differs')
    check(not cw.condition('guardian=done')(new), 'a word missing never passes')
    try:
        cw.condition('layer two')
        check(False, 'a bad until is refused')
    except SystemExit:
        pass


def pictures():
    Image = pics.Image
    canvas = Image.new('RGB', (256, 192), (0, 0, 0))
    canvas.paste((200, 0, 0), (8, 16, 248, 176))
    g = pics.game(canvas)
    check(g.size == (240, 160) and g.getpixel((0, 0)) == (200, 0, 0), 'the game cut from the canvas')
    check(pics.game(canvas, full=True).size == (256, 192), 'the canvas whole')
    check(pics.game(Image.new('RGB', (400, 240))).size == (400, 240), 'another canvas kept whole')
    check(pics.scaled(g, 2).size == (480, 320) and pics.scaled(g, 9).size == (960, 640), 'scales 1-4')
    s = pics.sheet([g] * 7, list(range(7)), 1)
    check(s.size == (6 * 240 + 5 * 2, 2 * 160 + 2), 'a sheet in rows of six')
    b = g.copy()
    b.putpixel((10, 20), (0, 0, 0))
    b.putpixel((30, 25), (0, 0, 0))
    check(pics.difference(g, b) == (2, (10, 20, 31, 26)), 'what differs and where')
    check(pics.difference(g, g.copy()) == (0, None), 'the same')
    check(pics.side_by_side([(g, b)], [5], 1).size == (484, 160), 'old beside new')
    check(pics.tokens((480, 320)) == 204 and pics.tokens((768, 576)) == 589, 'a picture\'s tokens')
    check(pics.tokens((3000, 3000)) < 1600, 'a big picture scaled down first')
    check(pics.png(g)[:4] == b'\x89PNG', 'PNG bytes')


def builds():
    werror = 'gcc -Werror -c -o build/host/obj/scenes/scene_error.o src/scenes/scene_error.c'
    check(cw.COMPILED.match(werror) and not cw.DIAG.search(werror), 'a command line is no complaint')
    check(cw.DIAG.search('src/core/options.c:58:9: error: expected \';\''), 'an error')
    check(cw.DIAG.search('src/x.c:3:1: warning: unused variable'), 'a warning')
    check(cw.DIAG.search('make: *** [Makefile:12: build/host/x.o] Error 1'), 'make\'s')
    check(cw.build('nothing').error, 'an unknown build refused')


def server():
    init = cw_mcp.answer({'jsonrpc': '2.0', 'id': 1, 'method': 'initialize', 'params': {'protocolVersion': '2025-03-26'}})
    check(init['result']['protocolVersion'] == '2025-03-26' and 'tools' in init['result']['capabilities'], 'initialize')
    tools = cw_mcp.answer({'jsonrpc': '2.0', 'id': 2, 'method': 'tools/list'})['result']['tools']
    check([t['name'] for t in tools] == ['shot', 'play', 'build'], 'the tools')
    check(len(json.dumps(tools)) < 6000, 'the tools\' descriptions stay short (every session reads them)')
    check(cw_mcp.answer({'jsonrpc': '2.0', 'method': 'notifications/initialized'}) is None, 'no answer to a notification')
    check(cw_mcp.answer({'jsonrpc': '2.0', 'id': 3, 'method': 'nothing'})['error']['code'] == -32601, 'an unknown method')
    bad = cw_mcp.answer({'jsonrpc': '2.0', 'id': 4, 'method': 'tools/call', 'params': {'name': 'build', 'arguments': {'what': 'nothing'}}})
    check(bad['result']['isError'], 'a tool\'s refusal is its error')
    if pics.Image:
        reply = cw.Reply('frame 1')
        reply.images.append(pics.Image.new('RGB', (4, 4)))
        c = cw_mcp.content(reply)['content']
        check(c[0] == {'type': 'text', 'text': 'frame 1'} and c[1]['type'] == 'image' and c[1]['mimeType'] == 'image/png', 'text and a picture')
    # (the server over its pipe: a message a line, nothing else on stdout)
    lines = '\n'.join(json.dumps(m) for m in ({'jsonrpc': '2.0', 'id': 1, 'method': 'initialize', 'params': {}},
                                                {'jsonrpc': '2.0', 'method': 'notifications/initialized'},
                                                {'jsonrpc': '2.0', 'id': 2, 'method': 'ping'})) + '\n'
    out = subprocess.run([sys.executable, os.path.join(ROOT, 'tools', 'cw_mcp.py')], input=lines, capture_output=True, text=True,
                         timeout=60).stdout.splitlines()
    check([json.loads(line)['id'] for line in out] == [1, 2], 'the pipe answers each request once')


def main():
    for part in (routes, starts, states, pictures, builds, server):
        if part is pictures and not pics.Image:
            print('agents\' tools: the pictures skipped (no Pillow here)')
            continue
        part()
    print(f'agents\' tools: {failed} failed')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
