"""0.24.0's online footage: four games in the Net Square in their own looks, the week's dive together, a crew's
battle joined unasked and a party's guardian fought on one field (tools/trailers/v0_22_0_footage.py's machinery).

Each take is a Net Square server of its own (from source, its key and clock pinned as 0.22.0's) and two to four
host builds in the build image at once, every game held to 60 frames a second by CYBERWORLD_AUDIO_DUMP, so a
script's frame F is F / 60 seconds after its game's start on every game and the same frame of two games is the
same moment. A game's frames are kept in ranges, each a folder of .build/trailer-0.24.0 (NNNN.png, 240 x 160,
every second game frame from the range's first: sample = (frame - first) // 2), as online_footage(work) leaves
them. The comment over ONLINE_SCENES says each take's clock.
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from trailers.v0_22_0_footage import (  # noqa: E402
    HOME, HOST, SERVER_INI, SERVER_KEY, SERVER_TIME, STAMPED, frames)

WORK = '.build/trailer-0.24.0'


def online_game(work, scene, g, url, binary=HOST, logs=False):
    """one game of an online scene, in the build image; its kept frames as capture() leaves a scene's
    (v0_22_0_footage's, the container named for this cut, a look of its own by --dev look=)"""
    import shutil
    import subprocess
    sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
    import build
    from PIL import Image
    data = os.path.join(work, f'data-{scene}-{g["name"]}')
    shutil.rmtree(data, ignore_errors=True)
    os.makedirs(data)
    for fname, text in g.get('files', STAMPED).items():
        with open(os.path.join(data, fname), 'w', encoding='utf-8') as f:
            f.write(text)
    rel = os.path.relpath(work, build.ROOT)
    look = [f'look={g["look"]}'] if g.get('look') else []
    dev = ','.join((['quiet'] if g.get('quiet', True) else []) + [f'online={url}', f'onlinename={g["operator"]}'] + look + list(g.get('dev', [])))
    last = frames(g['script'])
    cmd = ['docker'] + (['--context', build.CONTEXT] if build.CONTEXT else []) + [
        'run', '--name', f'cw-trailer24-{os.getpid()}-{scene}-{g["name"]}', '--rm', '-u', f'{os.getuid()}:{os.getgid()}',
        '-v', f'{build.ROOT}:/src', '-w', '/src', '-v', f'{build.default_rom_dir()}:/rom:ro',
        '-e', 'CYBERWORLD_AUDIO_DUMP=/dev/null']
    for k, v in sorted(g.get('env', {}).items()):
        cmd += ['-e', f'{k}={v}']
    if logs:
        cmd += ['-e', 'CYBERWORLD_EMU_DEBUG=1']
    cmd += [build.IMAGE, binary, '--headless', '--rom-dir', '/rom', '--data-dir', f'/src/{rel}/data-{scene}-{g["name"]}',
            *g.get('args', HOME), '--dev', dev, '--input', g['script'], '--frames', str(last + 1)]
    keeps = g.get('keeps', {})
    lo = min((a for a, _ in keeps.values()), default=0)
    hi = max((b for _, b in keeps.values()), default=0)
    raw = os.path.join(work, f'raw-{scene}-{g["name"]}')
    if keeps:
        shutil.rmtree(raw, ignore_errors=True)
        os.makedirs(raw)
        cmd += ['--shot-range', f'{lo}:{hi}:/src/{rel}/raw-{scene}-{g["name"]}/f']
    if logs:
        with open(os.path.join(work, f'log-{scene}-{g["name"]}.txt'), 'w', encoding='utf-8') as log:
            code = subprocess.call(cmd, stdout=log, stderr=subprocess.STDOUT)
    else:
        code = subprocess.call(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if code:
        print(f'{scene}: game {g["name"]} ended with {code}')
    for keep, (first, end) in keeps.items():
        out = os.path.join(work, keep)
        shutil.rmtree(out, ignore_errors=True)
        os.makedirs(out)
        for n, f in enumerate(range(first, end + 1, 2)):
            p = os.path.join(raw, f'f{f:05d}.bmp')
            if not os.path.exists(p):
                break
            im = Image.open(p).convert('RGB')
            w, h = im.size
            im.crop(((w - 240) // 2, (h - 160) // 2, (w + 240) // 2, (h + 160) // 2)).save(os.path.join(out, f'{n:04d}.png'))
    shutil.rmtree(raw, ignore_errors=True)
    if not logs:
        shutil.rmtree(data, ignore_errors=True)


def online_scene(work, scene, spec, binary=HOST, logs=False):
    """a scene: its server (from source, its key and clock pinned, its state fresh), then its games at once"""
    import shutil
    import socket
    import subprocess
    import threading
    import time
    root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    state = os.path.join(work, f'data-server-{scene}')
    shutil.rmtree(state, ignore_errors=True)
    os.makedirs(state)
    with socket.socket() as s:
        s.bind(('0.0.0.0', 0))
        port = s.getsockname()[1]
    ini = os.path.join(state, 'server.ini')
    with open(ini, 'w', encoding='utf-8') as f:
        f.write(SERVER_INI.format(port=port, state=state))
    with open(os.path.join(state, 'identity.key'), 'wb') as f:
        f.write(SERVER_KEY)
    pinned = ('import runpy, sys, time\nreal, t0 = time.time, time.time()\n'
              f'time.time = lambda: {SERVER_TIME} + (real() - t0)\n'
              f'sys.argv = [{os.path.join(root, "server", "main.py")!r}, "--config", {ini!r}]\n'
              'sys.path.insert(0, sys.argv[0].rsplit("/", 1)[0])\n'
              'runpy.run_path(sys.argv[0], run_name="__main__")\n')
    with open(os.path.join(state, 'server.log'), 'w', encoding='utf-8') as log:
        server = subprocess.Popen([sys.executable, '-c', pinned], stdout=log, stderr=subprocess.STDOUT)
        try:
            time.sleep(2)
            if server.poll() is not None:
                sys.exit(f'the server did not start: {state}/server.log')
            url = f'ws://172.17.0.1:{port}/presence'
            threads, t0 = [], time.time()
            for g in spec['games']:
                wait = g.get('delay', 0) - (time.time() - t0)
                if wait > 0:
                    time.sleep(wait)
                t = threading.Thread(target=online_game, args=(work, scene, g, url, binary, logs))
                t.start()
                threads.append(t)
            for t in threads:
                t.join()
        finally:
            server.terminate()
            server.wait()
    if not logs:
        shutil.rmtree(state, ignore_errors=True)
    print('captured', scene)


def online_footage(work=WORK, keep=False):
    """every scene of ONLINE_SCENES, one at a time, each kept game's frames in work/KEEP/NNNN.png (240 x 160,
    every second game frame from its `first`, numbered from 0000: as capture() leaves a scene); with `keep`
    the scenes whose folders are there already are left"""
    for scene, spec in ONLINE_SCENES.items():
        kept = [k for g in spec['games'] for k in g.get('keeps', {})]
        if keep and all(os.path.isdir(os.path.join(work, k)) and os.listdir(os.path.join(work, k)) for k in kept):
            continue
        online_scene(work, scene, spec)


class Clock:
    """a game's script by its frames: at(F, steps) waits until frame F, then plays the steps"""

    def __init__(self, script=''):
        self.s, self.t = script, frames(script)

    def at(self, frame, steps=''):
        if frame > self.t:
            self.s += f'{frame - self.t}:,'
            self.t = frame
        if steps:
            self.s += steps.rstrip(',') + ','
            self.t += frames(steps)
        return self


# Lan's HP after act 2 (seed 3), the arrival's words paged; the gate at `gate`: in the Square some 30 frames on
GATE = '600:,' + '6:A,94:,' * 6


def gate(frame=1200):
    return Clock(GATE).at(frame, '0:place 16 72 3,')


# the four's looks (palette.colour for armour, suit and accent, src/core/looks.c's palettes): Sol in Gregar's
# orange and red, Kai in ProtoMan's white and red, Nova in Nebula's violet with a cyan accent, Rin in Mystery's
# greens with a yellow accent
LOOKS = {'sol': '2.2,2.9,2.12', 'kai': '3.6,3.2,3.11', 'nova': '4.2,4.5,4.9', 'rin': '10.1,10.3,10.10'}
OPERATOR = {'sol': 'Sol', 'kai': 'Kai', 'nova': 'Nova', 'rin': 'Rin'}

# ---- the takes ----
# the portal's four sides (it stands at -44,172 in seed 120's Square), each facing it: two MegaMen on one
# panel would speak to each other instead
SIDES = {'sol': '-18 172 5', 'kai': '-44 146 3', 'nova': '-70 172 1', 'rin': '-44 198 7'}
# two panels out on the same line, for the walk in
OUT = {'sol': '30 172 5', 'kai': '-44 98 3', 'nova': '-118 172 1', 'rin': '-44 246 7'}
_PAGE = '6:B,24:,'


def _portal(c, who, t):
    """the portal from its side at T: the week's page (A), "N NAVIS DIVING! DIVE IN?" on No, LEFT, Yes; layer 1
    some 90 frames on, its card, then the first layer's words paged with B (free some 550 frames after the Yes, the paging to T + 780)"""
    return (c.at(t - 10, f'0:place {SIDES[who]},').at(t, '6:A,').at(t + 110, '6:A,').at(t + 210, '6:LEFT,')
            .at(t + 232, '6:A,').at(t + 330, _PAGE * 15))


def game(who, script, keeps=None, dev=(), quiet=True, look=True):
    g = dict(name=who, operator=OPERATOR[who], look=LOOKS[who] if look else None, script=script, dev=list(dev))
    if keeps:
        g['keeps'] = keeps
    if not quiet:
        g['quiet'] = False
    return g


# `meet` (four games): Sol through the gate first (in the Square at 1232), walking the plaza alone; the other
# three wait at home and come through at 1830. At 1870 all four are put two panels out from the portal, one on
# each side, walk in (1890) and wave (emotes from 1990): the Square's four looks side by side (1880 to 2180).
# Sol and Kai speak to the portal at 2200 and dive: layer 1 (the pad at -52,384) at about 2470, its card, the
# first layer's words paged, free by 2950; at 3000 they walk up the layer (UP+LEFT is -y) together, a crew of
# two. Nova and Rin dive at 2900 (layer 1 at about 3170, free by 3680) and follow them at 3690: the crew of four
# forms as they come near; then all four walk on together.
def _meet(who):
    c = gate(1200 if who == 'sol' else 1800)
    if who == 'sol':
        c.at(1300, '50:LEFT,10:,40:UP,10:,60:RIGHT,10:,40:DOWN+RIGHT,10:,50:DOWN,20:,40:LEFT,')
    emote = {'sol': 0, 'kai': 1, 'nova': 2, 'rin': 3}[who]
    c.at(1870, f'0:place {OUT[who]},').at(1890, '40:AHEAD,').at(1990 + 30 * emote, f'0:emote {emote},')
    if who in ('sol', 'kai'):
        _portal(c, who, 2200)
        c.at(3000, '130:UP+LEFT,' if who == 'sol' else '100:UP+LEFT,24:UP+RIGHT,')
    else:
        _portal(c, who, 2900)
        c.at(3690, '110:UP+LEFT,20:DOWN+LEFT,' if who == 'nova' else '90:UP+LEFT,')
    return c.at(3900, '90:UP+LEFT,').at(4100).s


# `crew-battle` (three games, BN6's roll on: not quiet; the Arena's bot each one's pad, partybot=1): through the
# gate and straight to the portal (1300), the three on layer 1 together and a crew (free at about 2090). Sol
# takes the `battle` step at 2200 and runs a small square round the pad until BN6's roll gives it; Kai and Nova
# stand beside him and are taken in unasked: three MegaMen on one field against the viruses.
_LOOP = '16:DOWN+LEFT+B,16:UP+LEFT+B,16:UP+RIGHT+B,16:DOWN+RIGHT+B,'


def _crew(who):
    c = _portal(gate(1200), who, 1300)
    if who == 'sol':
        c.at(2200, '0:battle,' + _LOOP * 6)
    elif who == 'kai':
        c.at(2120, '12:UP+LEFT,')
    else:
        c.at(2120, '12:DOWN+RIGHT,')
    return c.at(3600).s


# `party` (three games, each in his own look, the shared field's fix for issue 879 on dev):
# on layer 1 together as above, the dev menu's Next guardian (2200: layer 3, before DiveMan's arena), its arrival
# paged with B, each put by the arena's way in (`by arena`, 2760 + 60 a game), his words there paged with A, walked
# in; Sol, first, leads, and takes the party in with A at 3300: DiveMan's card (about 3400), his words (paged with
# A from 3500), the field from about 4200. Every MegaMan at the build's HP and DiveMan at his times the party (1500
# for three), as a player meets him: an honest fight, which the three lose (about sample 2000 of party-a); DELETED!
# is the solo win (guardian-win). Kai and Nova play as the Arena's bot (partybot=1:
# the first Custom screen, charged shots, a chip now and then, but it never opens the Custom screen again); Sol is
# played by a loop that opens it (L) when it fills, takes chips (A, RIGHT) to OK, and uses them (A)
_NEXT_GUARDIAN = '4:SELECT,6:SELECT+R,20:,' + '6:DOWN,6:,' * 8 + '6:A,'
PARTY_DEV = []   # (no switch that changes what the battle is: the guardian at the build's HP times the party, each MegaMan his own HP)
_FIGHT = '6:L,20:,' + '4:A,16:,4:RIGHT,16:,' * 6 + '4:A,30:,' * 4
PARTY_END = 9000


def _party(who):
    k = {'sol': 0, 'kai': 1, 'nova': 2}[who]
    c = _portal(gate(1200), who, 1300).at(2200, _NEXT_GUARDIAN).at(2400, _PAGE * 6)
    c.at(2760 + 60 * k, '0:by arena,40:,' + '6:A,30:,' * 5 + '0:by arena,5:,90:AHEAD,')
    if who == 'sol':
        c.at(3300, '6:A,')
    c.at(3500, '6:A,60:,' * 10)
    if who == 'sol':
        c.at(4200, _FIGHT * 14)
    return c.at(PARTY_END).s


def _party_dev(who):
    # (Sol played by the take's script, Kai and Nova by the Arena's bot as their pads)
    return PARTY_DEV + ([] if who == 'sol' else ['partybot=1'])

ONLINE_SCENES = {
    'meet': {'games': [
        game('sol', _meet('sol'), {'own-sol': (1240, 1820), 'square-a': (1880, 2180), 'dive-portal': (2190, 2990),
                                   'dive-a': (2980, 4100)}),
        game('kai', _meet('kai'), {'square-b': (1880, 2180), 'dive-b': (2980, 4100)}),
        game('nova', _meet('nova'), {'square-c': (1880, 2180), 'dive-c': (2980, 4100)}),
        game('rin', _meet('rin'), {'square-d': (1880, 2180), 'dive-d': (2980, 4100)}),
    ]},
    'crew-battle': {'games': [
        game('sol', _crew('sol'), keeps={'crew-battle-a': (2080, 3600)}, dev=['partybot=1'], quiet=False),
        game('kai', _crew('kai'), keeps={'crew-battle-b': (2080, 3600)}, dev=['partybot=1'], quiet=False),
        game('nova', _crew('nova'), keeps={'crew-battle-c': (2080, 3600)}, dev=['partybot=1'], quiet=False),
    ]},
    'party': {'games': [
        game('sol', _party('sol'), {'party-a': (2600, PARTY_END)}, dev=_party_dev('sol')),
        game('kai', _party('kai'), {'party-b': (4100, PARTY_END)}, dev=_party_dev('kai')),
        game('nova', _party('nova'), dev=_party_dev('nova')),
    ]},
}

# name: {what a sample shows: sample index} (from the take of 24 November 2026, looked at; a take's samples fall
# a few apart from these, the games' starts a second apart at most)
MARKS = {
    'own-sol': {'Sol (Gregar orange) walks the plaza alone, the portal beside him': 0, 'past the fire, under the Trade Central rock': 40,
                'on to the Arena side': 100, 'Square folk about him': 120, 'by the WELCOME booth': 160, 'back by the portal': 220,
                'KAI LOGGED IN!': 280},
    # the 2x2 mosaic: the same frames of the four games (frame 1880 + 2 x sample), each MegaMan at his own picture's
    # middle, the other three drawn in their looks (the Square's v30)
    'square-a': {'the four walk in to the portal, a side each, NOVA LOGGED IN! over them': 0, 'the four round the portal': 40,
                 "Sol's wave": 55, "Kai's emote (!!)": 80, "Nova's Haha!": 100, "Rin's sparkle": 110, 'the four together': 140},
    'square-b': {'the four walk in': 0, 'round the portal': 40, 'the emotes': 55, "Nova's Haha!": 100, 'the four together': 140},
    'square-c': {'the four walk in': 0, 'round the portal': 40, 'the emotes': 55, "Nova's Haha!": 100, 'the four together': 140},
    'square-d': {'the four walk in': 0, 'round the portal': 40, 'the emotes': 55, "Nova's Haha!": 100, "Rin's sparkle": 110,
                 'the four together': 140},
    'dive-portal': {'Sol at the portal, the other three round it': 0, 'NOV 9-NOV 15. (the week\'s first dive)': 20,
                    'EVERY NAVI DIVES THE SAME NET!': 60, 'NO NAVI DIVING YET. DIVE IN? Yes / No, on No': 100, 'Yes': 120,
                    "layer 1: Sol and Kai on the pad, a dotted ring": 140, "the act's card: Act 1 - Mettaur signal, RoboDog Comp": 160,
                    'the card to': 245, "Whoa., Kai's plate under the HP box": 260, "But it's all copied data,Lan.": 300,
                    'Dad was right.': 320, 'free: Kai beside him, his ring and plate': 360},
    # the four on layer 1 (frame 2980 + 2 x sample, the same frames in all four): on each game every MegaMan wears that
    # game's own look (Sol's orange, Kai's white, Nova's violet, Rin's green), not his own
    'dive-a': {'Sol and Kai on the pad, a crew of two (ring, plate)': 0, 'they walk up the layer together': 15, 'side by side': 90,
               'Nova and Rin come up from the lower right, white names': 375, 'dotted rings: about to be a crew': 395,
               'the crew of four: three plates, green names, a ring under each': 420, 'the four walk on together': 480, 'the last': 555},
    'dive-b': {'Kai and Sol on the pad': 0, 'up the layer': 15, 'Nova and Rin come up': 375, 'the crew of four': 420, 'on together': 480},
    'dive-c': {"Nova's own portal: 2 NAVIS DIVING! DIVE IN?": 30, 'Yes': 75, 'layer 1, Rin beside him': 90, 'the card': 120,
               "Whoa... It looks just like RoboDog Comp!": 225, 'free, Rin his crew': 300, 'up the layer': 375,
               'Sol and Kai ahead, dotted rings': 390, 'the crew of four': 420, 'on together': 480},
    'dive-d': {"Rin's own portal": 30, 'layer 1': 90, 'the card': 120, 'free, Nova his crew': 300, 'up the layer': 375,
               'the crew of four': 420, 'on together': 480},
    # three MegaMen in BN6's blue (frame 2080 + 2 x sample, the same frames in the three)
    'crew-battle-a': {'Sol on the pad, Kai and Nova his crew (plates)': 0, 'Kai and Nova step aside': 40, 'his Custom screen: CrakShot, the Gunner': 80,
                      'MiniBomb': 100, 'the field: three MegaMen in one column': 120, 'BATTLE START!, the three marked Nova, Kai, Sol': 160,
                      "the Gunner's sight": 200, 'all three firing, the Gunner 59': 240, "the Gunner's shot": 300, 'a MegaMan struck': 320,
                      'the fight goes on (the Gunner 43 at the end, no win in the take)': 600},
    'crew-battle-b': {"Kai beside Sol, Sol's and Nova's plates": 40, 'his own Custom screen: WideSwrd': 70, 'AirShot, Vulcan1, MiniBomb': 90,
                      'the field slides in': 120, "BATTLE START!, the band: SOL'S BATTLE!": 150, 'the fight, the band still': 190,
                      'the fight goes on': 400},
    'crew-battle-c': {"Nova beside Sol, Sol's and Kai's plates": 0, "the band: SOL'S BATTLE!": 60, 'his own Custom screen: Atk+10': 80,
                      'the field': 120, "BATTLE START!, SOL'S BATTLE!": 160, 'firing': 220, 'Nova struck (his HP 90)': 240,
                      'the fight goes on': 400},
    # the party, in BN6's blue (party-a: frame 2600 + 2 x sample; party-b: frame 4100 + 2 x sample). No win: see MISSING
    'party-a': {'layer 3: Sol put by the arena, Kai and Nova on their way': 140, 'Kai and Nova come': 200,
                'in at the door: WAITING FOR CREW 10 HOLD A:GO IN (counting down)': 220, 'the crew walks in': 250,
                'PARTY OF 3! A:GO IN B:ALONE': 280, "Sol's A: the screen's bars": 360, 'DiveMan beams in': 380,
                'his card: Guardian of RoboDog Comp, DiveMan, Terror of the Deep': 400, "That's DiveMan! (plates: BUSY)": 500,
                'Or a copy the Nest made of him...': 560, 'Awooga! Awooga! Intruder!': 600, 'Dive! Dive! Torpedoes ready!': 640,
                'Battle routine,set!': 680, 'Execute!!': 700, 'the Custom screen: Cannon, DiveMan 500': 740, 'CrakShot, MiniBomb': 860,
                'the field: three MegaMen and DiveMan': 940, 'BATTLE START!, the three marked Nova, Kai, Sol': 980,
                'all three firing': 1060, 'DiveMan hit, 496': 1100, 'his torpedoes': 1220, 'the Custom screen again (Sol\'s L): Sword': 1420,
                'Recov10, Atk+10': 1460, 'DiveMan leaps': 1620, 'his fire on the lead\'s row, Sol 490': 1900,
                'the field stands still from here to the end: use samples before': 1980},
    'party-b': {"Kai's own Custom screen: MiniBomb": 0, 'Cannon': 40, "Lan,this is no time to run away! (the bot's B)": 80, 'Sword, Atk+10': 120,
                'the field': 200, 'BATTLE START!': 240, 'firing, DiveMan 500': 280, 'a hit on DiveMan, 495': 400,
                'his Custom screen again: Cannon, Vulcan1': 680, 'CHIP DATA TRANSMISSION': 760, 'BATTLE START! again': 800,
                "DiveMan's wave over the lead's row": 880, 'the fight goes on, DiveMan 482': 1000},
    # an earlier take of the same fight (folder=2/4: blank purple chips on its Custom screens, at 740 to 780 and 2070), kept
    # because it runs on 105 seconds without standing still: DiveMan 500 down to 416, the MegaMen 600 down to 210..390
    'party-a-take4': {'the Custom screen, a blank chip': 740, 'BATTLE START!': 990, 'firing, DiveMan 495': 1110, 'his torpedoes': 1290,
                      'the Custom screen again: Atk+10': 1470, 'MiniBomb on him': 1650, 'DiveMan leaps, 471': 1890,
                      "his wave over the field": 2970, 'DiveMan 437 to 416, the fight going on (to the end)': 3630},
}

# what could not be taken as asked
MISSING = {
    'looks on a layer': "on a dive's layer every MegaMan on a game's screen wears that game's own look (Sol's game draws all four "
                        "orange, Nova's all violet): the looks travel in the Square alone (protocol v30), so dive-a..d show each "
                        "game's colour, not each diver's",
    'looks on one field': "crewmates in different looks never share a field: the first takes (each game its own look) logged "
                          "\"party: the fields went apart before the first frame\" on every game, the joiners went back to the map and "
                          "the guardian was fought alone on each game; the same three with no look agree (\"every seat ready with "
                          "...; the fight begins\"). So crew-battle and party are taken in BN6's own blue. Likely cause (read, not "
                          "fixed): look_apply.c writes the look into the core's ROM copy, which the party's field is built from "
                          "(emu_rom_clean, party_play.c's ready hash)",
    'the party\'s win': "not got in the box: the bots (partybot=1) never reopen the Custom screen and their shots seldom land "
                         "(DiveMan 1500 lost 29 HP in 32 s); with partyhpper=0 (his own 500) and Sol played by a loop that reopens "
                         "it, 500 fell to 416 in 105 s (party-a-take4), so a win needs some ten minutes and about 2500 HP a MegaMan. "
                         "folder=ID draws blank chips on the party's field. In the last take the field stood still from sample 1980 "
                         "(frame 6560; cause not looked into, the log has the partners gone only later)",
    'four on one field': 'not tried again: the party is three (the brief: four only if four no longer stand still)',
    'map dots': "not taken (the map on a dive is the engine's own; no time left in the box)",
    'NetBattle in looks': 'not taken (optional; no time left in the box)',
}
