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
import scenarios  # noqa: E402
import scenarios_fuzz as fuzz  # noqa: E402

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
    check(cw.changes('gameframe 5\nlayer 1\n', 'gameframe 9\nlayer 1\n') == [] and cw.changes('gameframe 5\n', 'layer 1\n') == ['layer 1'],
          "the game's frame count left out of what changed, as the frame is")
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
    check(s.size == (6 * 240 + 5 * 2, 2 * (160 + pics.LABEL_H) + 2), 'a sheet in rows of six')
    check(s.getpixel((2, 159)) == (200, 0, 0), 'a sheet\'s label off the frame (BN6 names the chip at its foot)')
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


def old_checkpoints():
    """tools/old_checkpoint.py's reading of states: the words that point into the free space, the regions they lie
    in, and what a release's state still holds where this build's own run has another (no game, no ROM)."""
    import struct
    import tempfile
    import old_checkpoint as oc
    table = oc.regions('#define EMU_FREE 0x08800000u\n#define FREE_A (EMU_FREE + 0x000100u)\n#define FREE_A_END (EMU_FREE + 0x000200u)\n'
                       '#define FREE_B (EMU_FREE + 0x480000u)  /* b */\n#define RAM_X 0x02010000u\n')
    check(table == [(0x08800100, 'FREE_A'), (0x08C80000, 'FREE_B')], "the header's regions, their ends and the RAM's left out")
    check(oc.region(table, 0x08C80071) == ('FREE_B', 0x08C80000) and oc.region(table, 0x08800000) is None, 'an address in its region, none before the first')
    check(oc.layer_of('frame 3\nlayer 12\n') == 12 and oc.layer_of('frame 3\n') == 0 and oc.words('doing battle\ndoing map\n')['doing'] == 'battle',
          "a state's layer and first words")

    def state(iwram=(), ewram=()):
        b = bytearray(oc.STATE_SIZE)
        for base, at, words_ in ((oc.STATE_IWRAM, oc.IWRAM, iwram), (oc.STATE_EWRAM, oc.EWRAM, ewram)):
            for addr, v in words_:
                struct.pack_into('<I', b, base + addr - at, v)
        return bytes(b)
    with tempfile.TemporaryDirectory() as d:
        files = {}
        # the release's state: a call into its code in IWRAM, a song's place and a sprite's in EWRAM
        files['saved'] = state([(0x03007254, 0x08D90071)], [(0x02007F10, 0x089C0000), (0x0200B820, 0x08C82D00), (0x02001000, 0x08800280)])
        # ... continued by this build: the call still the release's, the song this build's since, the rest as it was
        files['old'] = state([(0x03007254, 0x08D90071)], [(0x02007F10, 0x089C5000), (0x0200B820, 0x08C82D00), (0x02001000, 0x08800280)])
        # this build's own run: its own call, another song, the sprite elsewhere
        files['mine'] = state([(0x03007254, 0x08D900B1)], [(0x02007F10, 0x089C7000), (0x0200B820, 0x08C82D60), (0x02001000, 0x08800280)])
        for name, b in files.items():
            with open(os.path.join(d, name), 'wb') as f:
                f.write(b)
        check(oc.free_pointers(oc.ram(os.path.join(d, 'saved'))[0], oc.IWRAM) == {0x03007254: 0x08D90071}, "a state's IWRAM words into the free space")
        got = oc.carried('HEAD', None, os.path.join(d, 'saved'), os.path.join(d, 'old'), os.path.join(d, 'mine'), True)
        check('1 bytes differ' in got[0] and got[1].strip().startswith('03007254: 71'), "IWRAM's routines: the release's byte beside this build's")
        check("as this build's own run has them 1, written by this build since 1" in got[2] and got[2].endswith('none: 2'),
              'the words counted: the same, written since, the release\'s own still')
        check(sum('03007254 -> 08d90071 (this build 08d900b1)' in line or '0200b820 -> 08c82d00 (this build 08c82d60)' in line for line in got) == 2
              and not any('02007f10' in line for line in got), "the release's own words listed, this build's since left out")
        with open(os.path.join(d, 'short'), 'wb') as f:
            f.write(b'\0' * 64)
        try:
            oc.ram(os.path.join(d, 'short'))
            check(False, 'a file too short for a state is refused')
        except oc.Failed:
            pass


def scenario_files():
    """tools/scenarios.py without the game: every scenario of tests/scenarios
    reads, one written wrong is refused with why, a name finds its own, and a
    step's test of the state holds or fails as the state says."""
    every = scenarios.load()
    check(len(every) >= 4 and 'liberation-rules-end' in every, 'the scenarios are found')
    for name, s in every.items():
        for line in scenarios.problems(name, s):
            check(False, line)
    check(all(f'routes/{name}' in every for name in cw_routes.ROUTES), 'a scenario a route')
    check(every['routes/darkloid/shademan']['needs'] == ('bn5',), "BN5's routes need its ROM")
    good = dict(guards='issue #1: a thing', start='--scene emu', needs=(), steps=[('keep', 'hp'), ('same', 'hp')])
    good['known'] = ''
    check(not scenarios.problems('good', good), 'a scenario that reads')
    check(scenarios.problems('bad', {**good, 'known': 669}), 'refused: a KNOWN that is no line of words')
    lost, won = dict(ok=False, lines=['     step 2: not so']), dict(ok=True, lines=[])
    check(scenarios.verdict(won, '') == ('PASS', False, []) and scenarios.verdict(lost, '')[:2] == ('FAIL', True), 'a scenario passes or fails')
    check(scenarios.verdict(lost, 'issue #1: wrong')[:2] == ('KNOWN', False), 'a known bug shown is no failure of the run')
    check(scenarios.verdict(won, 'issue #1: wrong')[:2] == ('FAIL', True), 'a known bug gone fails until its KNOWN goes')
    for what, change in (('no word on what it guards', dict(guards='')), ('a ROM nobody has', dict(needs=('bn9',))),
                         ('no steps', dict(steps=[])), ('a step unknown', dict(steps=[('hope', 'hp=1')])),
                         ('a test that does not read', dict(steps=[('expect', 'hp is 1')])),
                         ('a command play.py has none of', dict(steps=[('do', 'jump 3')])),
                         ('a limit that is no number', dict(steps=[('until', 'layer>=2', 'soon')])),
                         ('a restart on no route', dict(steps=[('restart', 'nowhere')])),
                         ('same of a word never kept', dict(steps=[('keep', 'hp'), ('same', 'zenny')]))):
        check(scenarios.problems('bad', {**good, **change}), f'refused: {what}')
    check(set(scenarios.chosen(every, ['routes/pet'])) == {n for n in every if n.startswith('routes/pet')}, 'a name finds those it starts')
    check(list(scenarios.chosen(every, ['liberation-rules-end'])) == ['liberation-rules-end'], 'a name finds its own')
    try:
        scenarios.chosen(every, ['nothing-of-the-kind'])
        check(False, 'an unknown scenario is refused')
    except SystemExit as e:
        check('liberation-rules-end' in str(e.code), 'an unknown scenario names the scenarios')
    state = 'frame 9\nscreen battle\nhp 80/100\nzenny 150\nrules plain'
    kept = {}
    check(scenarios.step_run(None, ('expect', 'rules=plain'), state, kept) == (state, 0), 'an expect that holds')
    scenarios.step_run(None, ('keep', 'hp zenny'), state, kept)
    check(kept == {'hp': 'hp 80/100', 'zenny': 'zenny 150'}, "a keep's lines")
    scenarios.step_run(None, ('same', 'hp zenny'), state, kept)
    for step, later in ((('expect', 'rules=liberation'), state), (('same', 'zenny'), state.replace('150', '0')),
                        (('same', 'hp'), 'frame 10\nscreen title'), (('keep', 'layer'), state)):
        try:
            scenarios.step_run(None, step, later, kept)
            check(False, f'{step} fails where the state says otherwise')
        except scenarios.Failed as e:
            check(e.args[0], f'{step} says what it found')


def quit_fuzz():
    """tools/scenarios_fuzz.py without the game: a game's plan comes from the
    fuzz's seed alone, a state's situation is the player's, the checkpoint
    and the map before a box are followed, and a CONTINUE is judged by its
    quit's situation."""
    plan = fuzz.plan(7, 3, 4)
    check(plan == fuzz.plan(7, 3, 4) and plan != fuzz.plan(7, 4, 4) and plan != fuzz.plan(8, 3, 4), "a game's plan is its seed's and its number's")
    check(len(plan['quits']) == 4 and plan['pilot'] in ('1', 'weak') and fuzz.DEPTHS[0] <= plan['depth'] <= fuzz.DEPTHS[1] and
          all(fuzz.GAP[0] <= gap <= fuzz.GAP[1] and 0 <= extra <= fuzz.EDGE_AFTER for gap, _, extra in plan['quits']), "a plan's numbers in their bounds")
    check(fuzz.plan(7, 3, 2)['quits'] == plan['quits'][:2], 'fewer quits are the first of more')

    def state(**lines):
        base = dict(scene='emu', screen='map', where='layer', doing='map', chat='closed', talk='none', gameframe='100', hp='80/100',
                    zenny='150', bugfrags='2', quit='here', saves='1', layer='2', area='Seaside Area', scrtdata='0')
        return {k: v for k, v in {**base, **lines}.items() if v is not None}
    town = state(where='town', screen='town', quit='checkpoint', layer=None, area=None, scrtdata=None)
    for want, s, won in (('map', state(), False), ('box', state(chat='open'), False), ('box', state(talk='director'), False),
                         ('box', state(screen='reward-pick'), False), ('box', state(doing='menu'), False),
                         ('battle', state(screen='battle', doing='battle', battle='fight'), False),
                         ('won', state(screen='battle', doing='battle', battle='won'), True), ('won', state(doing='other'), True),
                         ('guardian', state(guardian='BlastMan fighting', doing='battle'), False),
                         ('guardian', state(guardian='BlastMan beaten'), True), ('map', state(guardian='??? waiting'), False),
                         ('home', town, False), (None, dict(scene='title', screen='title'), False),
                         (None, state(doing='gameover'), False), (None, dict(scene='emu', where='none'), False)):
        check(fuzz.situation(s, won) == want, f'a state of {want}: {fuzz.situation(s, won)}')
    check(fuzz.read('frame 9\nscreen map\nhp 80/100\nguardian ??? waiting\nmap one\nmap two') ==
          dict(frame='9', screen='map', hp='80/100', guardian='??? waiting', map='one'), "a state's lines by their first word")

    # a game followed: its checkpoint, a box, a battle won
    first = state(saves='0', quit='checkpoint')
    t = fuzz.track()
    check(fuzz.follow(t, first) == 'map' and not t['saved'], 'no checkpoint yet')
    saved = state()
    fuzz.follow(t, saved, 30)
    check(t['saved'] == [first, saved], 'a checkpoint saved between two states: either')
    walked = state(gameframe='130')
    fuzz.follow(t, walked, 30)
    box = state(chat='open', quit='talk', hp='100/100', gameframe='160')
    check(fuzz.follow(t, box, 30) == 'box' and t['before'] == [walked, box] and t['saved'] == [first, saved], 'a box: the states since the map')
    fight = state(screen='battle', doing='battle', battle='fight', quit='checkpoint', gameframe='190')
    check(fuzz.follow(t, fight, 30) == 'battle' and not t['won'], 'a battle under way')
    result = state(screen='battle', doing='battle', battle='won', quit='after', gameframe='220')
    check(fuzz.follow(t, result, 30) == 'won' and t['won'], "a battle's RESULT")
    fade = state(doing='other', quit='after', zenny='300', gameframe='250')
    check(fuzz.follow(t, fade, 30) == 'won', '... won until MegaMan is free')
    check(fuzz.follow(t, state(zenny='300', gameframe='280'), 30) == 'map' and not t['won'] and t['free'], '... and the map after it')
    for _ in range(3):
        fuzz.follow(t, state(zenny='300', gameframe='280'), 30)
    check(t['still'] == 90, "the frames the game's count stands")
    fuzz.follow(t, state(zenny='300', gameframe='281'), 30)
    check(t['still'] == 0 and t['frames'] == 330, '... and goes on')
    after = fuzz.track(saved)
    check(after['saved'] == [saved] and not after['free'], "a CONTINUE's state is its checkpoint")

    # a CONTINUE judged
    kept = state(zenny='300', hp='60/100')
    check(not fuzz.judge('map', kept, dict(kept), [saved], [kept]), 'the map: all of it back')
    check(any('zenny 300 kept, 150 found' in line for line in fuzz.judge('map', kept, state(hp='60/100'), [saved], [kept])), 'the map: zenny gone back')
    check(any(line.startswith('layer') for line in fuzz.judge('map', state(layer='3'), state(), [saved], [])), 'the map: the layer gone back')
    check(not fuzz.judge('won', result, state(zenny='300', hp='90/100'), [saved], []), "a won battle: its reward come, the RESULT's HP another")
    check(fuzz.judge('won', fade, state(), [saved], []) == ['zenny 300 kept, 150 found'], 'a won battle lost')
    onward = state(layer='3', area='Sky Area', zenny='300', guardian='??? waiting')
    check(not fuzz.judge('won', fade, onward, [saved], []), 'a won battle on the exit: the game run on to the next layer')
    check(fuzz.judge('won', fade, state(layer='4', zenny='300'), [saved], []), '... and no further')
    check(fuzz.judge('map', state(zenny='300'), onward, [saved], []), 'the map, the run kept here: the next layer is another')
    check(not fuzz.judge('battle', fight, dict(saved), [first, saved], []), 'a battle under way: the last checkpoint')
    check(fuzz.judge('battle', state(zenny='300', doing='battle'), state(zenny='0'), [saved], []) ==
          ['zenny 300 kept, 0 found (150 at the last checkpoint)'], 'a battle under way: further back than the checkpoint')
    check(any(line.startswith('layer') for line in fuzz.judge('battle', state(layer='3'), state(), [saved], [])), 'a battle: another layer')
    check(not fuzz.judge('box', box, dict(walked), [saved], [walked, box]), 'a box: the run as it stood before it')
    check(not fuzz.judge('box', box, dict(box), [saved], [walked, box]), '... or as it stands')
    check(fuzz.judge('box', state(chat='open', zenny='300'), state(zenny='0'), [saved], [state(zenny='300')]) ==
          ['zenny 300 kept, 0 found (300 since the map)'], 'a box: further back than the map before it')
    door = state(guardian='BlastMan waiting')
    check(not fuzz.judge('guardian', state(guardian='BlastMan fighting', hp='10/100'), dict(door), [door], []), "a guardian's fight: the door")
    unmet = state(guardian='??? waiting')
    check(not fuzz.judge('guardian', state(guardian='BlastMan fighting'), dict(door), [unmet], []), '... his name known since: the profile has met him')
    check(fuzz.judge('guardian', state(guardian='BlastMan fighting'), state(guardian='BlastMan done'), [unmet], []), '... but not his end')
    check(fuzz.judge('map', door, dict(unmet), [], []) == ['guardian BlastMan waiting kept, ??? waiting found'], 'a guardian heard of, forgotten')
    check(not fuzz.judge('home', town, dict(saved), [saved], []), 'the town before its save: the layer left')
    check(fuzz.judge('home', town, dict(scene='emu', where='none'), [town], [])[0] == 'where town kept, none found', 'the town: no run found')
    check(fuzz.judge('map', kept, dict(scene='emu', where='layer'), [saved], [])[0] == 'layer 2 kept, None found', 'a line gone is told')

    # the bugs known
    for k in fuzz.KNOWN:
        check(k['kind'] in ('map', 'won', 'box', 'battle', 'guardian', 'home') and k['what'] and k['kept'], f'a known bug reads: {k["what"][:40]}')
        check(fuzz.known(k['kind'], state(**k['kept'])) == k['what'], 'a known bug is told from its quit')
    check(fuzz.known('map', state()) is None, 'a quit on the map with the run kept here is no known bug')


def main():
    for part in (routes, starts, states, pictures, builds, server, old_checkpoints, scenario_files, quit_fuzz):
        if part is pictures and not pics.Image:
            print('agents\' tools: the pictures skipped (no Pillow here)')
            continue
        part()
    print(f'agents\' tools: {failed} failed')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
