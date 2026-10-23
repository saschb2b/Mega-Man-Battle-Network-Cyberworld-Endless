#!/usr/bin/env python3
"""The project's MCP server for agents (docs/DEVTOOLS.md, Agents' tools):
tools/cw.py's shot, play and build as tools, each answering in one call
with a few lines of text and the picture inline. It keeps no state of its
own (the games are tools/play.py's folders in .build/play, the other
commits' builds in .build/cw/ref), so it can restart at any time. Python's
standard library and Pillow only: MCP's stdio transport is JSON-RPC 2.0,
a message a line.

Registered in the repository's .mcp.json as "cyberworld"; to try it by
hand, `python3 tools/cw_mcp.py` and a line such as
{"jsonrpc":"2.0","id":1,"method":"tools/list"}.
"""
import base64
import contextlib
import json
import os
import sys
import traceback

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cw  # noqa: E402
import cw_pictures as pics  # noqa: E402
import cw_routes  # noqa: E402

COMMANDS = ('play.py\'s, apart by ";": press BTN [N] (N frames, 6; then 6 let go), hold BTN N, wait N, '
            'mash BTN N (a press every 10 frames), shot (a picture now), second (the second screen); '
            'BTN A B L R START SELECT UP DOWN LEFT RIGHT, or several: UP+RIGHT; 60 frames a second. '
            'Dev steps: place X Y FACING, flags FROM TO 1|0, battle, foes 200@52/01@43, chip ID CODE [N], '
            'status KIND [FRAMES], xrecord ADDR, sig (docs/DEVTOOLS.md)')
START = ('Where the game starts: a route, the game\'s options, or both ("jukebox --seed 4", '
         '"--scene emu --seed 7 --net-biome 3"); VAR=VALUE first sets an environment variable '
         '("CYBERWORLD_AUTOPILOT=1 layer"). Routes: ' + ', '.join(cw_routes.ROUTES))
UNTIL = ('Run on, 60 frames at a time, until the state says so: WORD OP VALUE with OP = != > >= < <= ~ '
         '("layer>=2", "screen=battle", "doing~battle"); several apart by | are any')
COMMON = {
    'do': {'type': 'string', 'description': 'Input after the start: ' + COMMANDS},
    'until': {'type': 'string', 'description': UNTIL},
    'limit': {'type': 'integer', 'description': 'An until\'s most frames (20000)'},
    'every': {'type': 'integer', 'description': 'A picture every N frames, on one labelled sheet (at most 24)'},
    'picture': {'type': 'boolean', 'description': 'A picture at the end (true); false when the state in words is enough'},
    'scale': {'type': 'integer', 'description': '1-4: one picture 2x, a sheet 1x by default'},
    'full': {'type': 'boolean', 'description': 'The whole 256x192 canvas, not the game\'s 240x160'},
}
TOOLS = [
    {'name': 'shot',
     'description': 'A fresh headless game of one batch (seed 1 unless given): START, then the input; its state in '
                    'words (frame, screen, scene, and in a run where, doing, hp...) and the game\'s picture. '
                    'With ref, the same by that commit\'s build beside this one\'s, what differs framed and counted. '
                    'The host build is rebuilt first where a source is newer.',
     'inputSchema': {'type': 'object', 'properties': {
         'start': {'type': 'string', 'description': START},
         **COMMON,
         'state': {'type': 'string', 'enum': ['full', 'none'], 'description': 'The state in words (full)'},
         'ref': {'type': 'string', 'description': 'A git ref (v0.13.0, dev, HEAD~1) to compare with; its build is cached'},
     }}},
    {'name': 'play',
     'description': 'A game kept between calls (tools/play.py\'s NAME in .build/play): started with start (or at the '
                    'title when none runs), then the input, what changed in its state and a picture. do "stop" '
                    'ends it. The same folders as play.py: its replay and the playtest scripts read them.',
     'inputSchema': {'type': 'object', 'required': ['name'], 'properties': {
         'name': {'type': 'string', 'description': 'The game\'s name'},
         'start': {'type': 'string', 'description': '(Re)start it: ' + START},
         'fresh': {'type': 'boolean', 'description': 'Its data folder emptied first: a first-time player'},
         **COMMON,
         'state': {'type': 'string', 'enum': ['changes', 'full', 'none'], 'description': 'The state: what changed (default), whole, or none'},
     }}},
    {'name': 'build',
     'description': 'build.py host (default), linux, test or lint, answered in a line, or its errors alone.',
     'inputSchema': {'type': 'object', 'properties': {
         'what': {'type': 'string', 'enum': ['host', 'linux', 'test', 'lint']},
     }}},
]
INSTRUCTIONS = ('Cyberworld Endless headless: shot looks from a fresh start, play keeps a game between calls, '
                'build builds or checks quietly. Pictures are the game\'s 240x160 at 2x; ask for none (picture '
                'false) when the state\'s screen and doing lines answer the question.')


def call(name, args):
    """A tool's reply (cw.Reply) for its arguments."""
    pick = {k: args[k] for k in ('do', 'until', 'limit', 'every', 'picture', 'scale', 'full', 'state') if k in args}
    if name == 'shot':
        return cw.shot(args.get('start') or 'title', ref=args.get('ref'), **pick)
    if name == 'play':
        return cw.play_game(args['name'], start=args.get('start'), fresh=bool(args.get('fresh')), **pick)
    if name == 'build':
        return cw.build(args.get('what') or 'host')
    return cw.Reply(f'no tool {name}', error=True)


def content(reply):
    out = [{'type': 'text', 'text': reply.text or 'ok'}]
    for im in reply.images:
        out.append({'type': 'image', 'data': base64.b64encode(pics.png(im)).decode(), 'mimeType': 'image/png'})
    return {'content': out, 'isError': bool(reply.error)}


def answer(msg):
    """The response to one message, None for a notification."""
    method, mid, params = msg.get('method'), msg.get('id'), msg.get('params') or {}
    if mid is None:
        return None
    if method == 'initialize':
        result = {'protocolVersion': params.get('protocolVersion', '2025-06-18'), 'capabilities': {'tools': {}},
                  'serverInfo': {'name': 'cyberworld', 'version': '1'}, 'instructions': INSTRUCTIONS}
    elif method == 'tools/list':
        result = {'tools': TOOLS}
    elif method == 'tools/call':
        try:
            # (stdout is the protocol's: anything printed goes to stderr)
            with contextlib.redirect_stdout(sys.stderr):
                result = content(call(params.get('name'), params.get('arguments') or {}))
        except Exception:
            result = {'content': [{'type': 'text', 'text': traceback.format_exc(limit=4)}], 'isError': True}
    elif method == 'ping':
        result = {}
    else:
        return {'jsonrpc': '2.0', 'id': mid, 'error': {'code': -32601, 'message': f'no method {method}'}}
    return {'jsonrpc': '2.0', 'id': mid, 'result': result}


def main():
    os.chdir(cw.ROOT)
    for line in sys.stdin:
        if not line.strip():
            continue
        try:
            msg = json.loads(line)
        except ValueError:
            continue
        reply = answer(msg)
        if reply is not None:
            sys.stdout.write(json.dumps(reply) + '\n')
            sys.stdout.flush()


if __name__ == '__main__':
    main()
