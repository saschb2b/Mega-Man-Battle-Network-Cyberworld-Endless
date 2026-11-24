"""0.24.0's trailer footage, the single-game scenes: each a scripted headless run (tools/trailer.py's capture()).

Every scene here was run as capture() runs it (the whole ROM folder mounted, a fresh data folder) and its frames
looked at, 24 November 2026. A sample is every second game frame from `first`: sample = (frame - first) // 2.

MegaMan wears a look (docs/game/looks.md) by --dev look=, a palette.colour a channel: LOOK, the release's (Glitch's
magenta armour, Nebula's violet suit, Glitch's cyan accent), on the layer, in battle, in the chat box and on the
Custom screen; the forms' takes each another of LOOKS, worn until the Cross, the Soul or the beast takes over.

Three scenes go on from a run another run made (BN4's Souls come only from a guardian, and the setup's FORM row turns
only to forms a profile has opened): prep(work) plays those runs first, one after another, and copies their data
folders (PREP and SAVED below). capture_all alone does not: run prep(WORK) before it, then capture those three.

    python3 -c "import sys; sys.path.insert(0,'tools'); import trailer; trailer.WORK='.build/trailer-0.24.0'
    from trailers import v0_24_0_footage as f; f.prep(trailer.WORK); trailer.capture_all(f.SCENES, False)"
"""

import os

AUTO = {'CYBERWORLD_AUTOPILOT': '1'}
WEAK = {'CYBERWORLD_AUTOPILOT': 'weak'}


def frames(script):
    """the frames an --input script takes (its steps are durations, held in sequence)"""
    return sum(int(step.split(':')[0]) for step in script.split(',') if step)


# MegaMan's looks (docs/game/looks.md): armour, suit, accent, each palette.colour (src/core/looks.c)
LOOK = '11.5,4.4,11.7'          # Glitch's magenta armour, Nebula's violet suit, Glitch's cyan accent: the release's look
LOOKS = {
    'glitch': LOOK,
    'proto': '3.2,4.4,5.1',      # ProtoMan's red, Nebula's violet, Tournament's gold
    'hazard': '6.5,6.7,6.1',     # Hazard's lime, its deep green, its yellow
    'mystery': '10.9,10.6,10.10',  # Mystery's purple, its dark blue, its yellow
    'mono': '11.0,11.4,11.12',     # Glitch's white, its black, its red
}

# a layer's arrival paged (MegaMan free at 470)
ARRIVE = '150:,' + '6:A,4:,' * 30 + '20:,'
# ... paged slowly, each box read (the mugshot's look)
CHAT = '600:,' + '6:A,70:,' * 4 + '50:'
# Lan's HP after act 3 (seed 7): MegaMan's words as he comes home, his mugshot in his look
HOME = ['--scene', 'home', '--seed', '7', '--run-depth', '10']
# a battle's arrival paged for good (780) and the foes step
ARRIVED = '150:,' + '6:A,4:,' * 60 + '30:,'


def net(seed, depth, dev, biome=None, setup=None):
    b = ['--net-biome', biome] if biome is not None else []
    s = ['--setup', setup] if setup else []
    return ['--scene', 'emu', '--run-depth', str(depth), '--seed', str(seed), *b, *s, '--dev', dev]


def fight(look, foes, first, last, biome='2', seed=4, dev='quiet,god'):
    """the foes step's battle on a layer, the autopilot fighting in god mode, MegaMan in `look`"""
    return (net(seed, 2, f'{dev},look={look}', biome), AUTO, ARRIVE + f'0:foes {foes}', first, last)


# the Custom screen held open: the foes step, the screen, a look at it, Right through the chips, OK
CUSTOM = ARRIVED + '0:foes 01@42/01@53/01@63,320:,' + '6:RIGHT,30:,' * 4 + '6:A,40:,6:A,60:,6:START,30:,6:A,200:'

# a form taken: three Mettaur3 (120 HP; they come to MegaMan's row) to strike, MegaMan a column forward (a Navi, BN6's
# HeatMan or BN5's KnightMan and TomahawkMan, dodged the scripted shots: 3 HP at most in a whole take); CROSSSELECT (UP, A), OK; then charged shots and chips
FOE = '03@42/03@51/03@63/mm@32'
PICK = '6:A,10:,6:RIGHT,10:,6:A,10:,6:RIGHT,10:,6:A,20:,'
SHOTS = '200:,' + '110:B,40:,6:A,70:,' * 3 + '110:B,80:'
CROSS = ARRIVED + f'0:foes {FOE},260:,' + '6:UP,30:,6:A,60:,' + PICK + '6:START,10:,6:A,' + SHOTS


def cross(n, look):
    """the setup's Cross n (battle_forms.py: 1-5 Gregar's, 6-10 Falzar's) taken on the first Custom screen"""
    return (net(1, 2, f'quiet,god,look={look}', setup=f'short,0,0,0,{n}'), {}, CROSS, 1000, frames(CROSS))


# a Soul of BN5's united (xrom_souls.py's): held by the souls mask (bits 6-11 Team ProtoMan's: 0x40 ProtoSoul,
# 0x80 GyroSoul, 0x100 SearchSoul, 0x200 NapalmSoul, 0x400 MagnetSoul, 0x800 MeddySoul), a folder of one chip of its
# kind; the chip picked (A), UNITE under OK (START, DOWN, A), OK (START, A); then charged shots
# (a Soul's charge is longer than a Cross's: B held 170 frames)
SOUL_SHOTS = '200:,' + '170:B,40:,6:A,70:,' * 3 + '170:B,80:'
UNITE = ARRIVED + f'0:foes {FOE},260:,' + '6:A,10:,6:A,10:,6:A,20:,6:START,10:,6:DOWN,10:,6:A,60:,6:START,10:,6:A,' + SOUL_SHOTS


def soul5(mask, chip, look):
    return (net(15, 2, f'quiet,god,hp=2000,souls={mask:#x},folder={chip},look={look}', '0'), {}, UNITE, 1000, frames(UNITE))


# Beast Out (--dev powers; 0.22.0's cold open): before BN5's Draggin, Powie and Marina on a layer of BN5's SciLab, on the
# Custom screen five Right and Down to the star, A, START to OK, A; then the buster held and a chip now and then
BEAST_RUN = ['--scene', 'emu', '--run-depth', '4', '--seed', '4', '--net-biome', 'x1', '--dev']
BEAST = ('150:,' + '6:A,4:,' * 60 + '30:,0:foes 254@52/20C@41/260@63,260:,' + '6:RIGHT,6:,' * 5
         + '6:DOWN,6:,10:,6:A,6:,90:,6:START,6:,20:,6:A,6:,240:,'
         + '60:B,6:,4:A,30:,6:UP,10:,60:B,6:,4:A,30:,6:DOWN,10:,' * 4)

# the setup's FOLDER row (screens_shelf.py): Down to it, Right through all six and round
SHELF = '90:,6:DOWN,20:,6:DOWN,40:,' + '6:RIGHT,90:,' * 6 + '6:RIGHT,60:'
# the layer's map (layer_map.py): a press of SELECT, MegaMan walks under it, the next press closes it
MAP = ARRIVE + '60:DOWN,20:,6:SELECT,40:,150:DOWN,150:LEFT,150:DOWN,150:RIGHT,20:,6:SELECT,90:DOWN,30:'
# fast forward (FAST: its button, fast_forward.h), the weak autopilot walking: off, pressed on, off again
FAST = '700:,6:FAST,400:,6:FAST,200:'

# Lan's HP (home_requests.py, home_bring_choice.py): seed 7 after act 3 (depth 10), the arrival's words paged, the
# courier spoken to from below him (place -84 54 7)
HOME_IN = '600:,' + '6:A,60:,' * 8 + '0:place -84 54 7,10:,'
REWARD = HOME_IN + '6:A,60:,' + '6:A,70:,' * 4 + '80:'
CHOICE = HOME_IN + '6:A,60:,' + '6:A,50:,' * 5 + '100:,6:DOWN,40:,6:UP,40:,6:DOWN,40:,6:A,80:,6:A,80:,' + '6:A,60:,' * 4

SCENES = {
    # name: (argv, env, script, first, last)   # as tools/trailer.py's capture() takes them
    'look-field': (net(4, 2, f'quiet,look={LOOK}', '2'), WEAK, None, 460, 1100),
    'look-battle': fight(LOOK, '01@42/01@53/01@63', 560, 1300),
    'look-chat': (HOME + ['--dev', f'quiet,look={LOOK}'], {}, CHAT, 560, frames(CHAT)),
    'look-custom': (net(4, 2, f'quiet,god,look={LOOK}', '2'), {}, CUSTOM, 980, frames(CUSTOM)),
    **{f'look-many-{k}': fight(v, '01@42/01@53/01@63', 560, 900) for k, v in LOOKS.items()},
    'form-cross-tengu': cross(8, LOOKS['proto']),
    'form-cross-dust': cross(10, LOOKS['hazard']),
    'form-cross-ground': cross(9, LOOKS['mystery']),
    'form-cross-elec': cross(2, LOOKS['mono']),
    'form-soul5-napalm': soul5(0x200, 107, LOOKS['hazard']),
    'form-soul5-proto': soul5(0x40, 71, LOOKS['glitch']),
    'form-soul5-magnet': soul5(0x400, 30, LOOKS['mystery']),
    'form-soul5-gyro': soul5(0x80, 4, LOOKS['proto']),
    'form-beast': (BEAST_RUN + [f'quiet,god,powers,look={LOOKS["mono"]}'], {}, BEAST, 1000, frames(BEAST)),
    'folders-shelf': (['--scene', 'setup'], {}, SHELF, 60, frames(SHELF)),
    'map-open': (net(3, 2, f'quiet,look={LOOK}'), {}, MAP, 480, frames(MAP)),
    'fast-forward': (net(4, 2, f'quiet,look={LOOK}', '2'), WEAK, FAST, 480, frames(FAST)),
    'reward-home': (HOME + ['--dev', f'job=3,jobstate=2,look={LOOK}'], {}, REWARD, frames(HOME_IN) - 40, frames(REWARD)),
    'chip-choice': (HOME + ['--dev', f'job=4,jobstate=2,pack=38,look={LOOK}'], {}, CHOICE, frames(HOME_IN) - 40, frames(CHOICE)),
    'party-solo': (['--scene', 'emu', '--run-depth', '3', '--seed', '3', '--dev', f'quiet,partycore=3,partybot=1,look={LOOK}'], AUTO, None, 1700, 2600),
}

# ---- BN4's Souls and the FORM row: runs that need a profile or a run another run made (PREP) ----
#
# BN4's Souls come only from a guardian (no dev switch holds one) and the setup's FORM row turns only to forms a
# profile has opened (a BN5 and a BN4 Navi fallen as guardians) with --dev forms. So prep(work) first plays the runs
# that make them, one after another, each a capture() of its last frame alone (the autopilot to the guardian, his
# data taken: forms.py's courses), and copies each data folder for the scenes that go on from it (SAVED), which name
# it with a --data-dir of their own after capture()'s (the last one given counts). A BN4 guardian in a SOUL BN4 run
# (--dev forms,form=3) pays the Soul of his kind: ThunderMan (39) ThunSoul, AquaMan (44) AquaSoul.
def _guardian(navi, form, data=None):
    more = ['--data-dir', data] if data else []
    return ['--scene', 'emu', '--seed', '3', '--run-depth', '3', '--net-biome', 'x15' if navi >= 34 else 'x0', '--guardian', str(navi),
            '--dev', f'onehit,god,quiet,forms,form={form}', *more]


PREP = {   # name: (argv, last frame), run in this order
    'prep-thun': (_guardian(39, 3), 4200),
    'prep-aqua': (_guardian(44, 3), 6000),   # (at 4200 still in AquaMan's arena, where the foes step starts no battle)
    'prep-row': (_guardian(29, 1), 4200),                                                  # BN5's ToadMan: SOUL BN5 opens
    'prep-row-bn4': (_guardian(39, 3, '/src/.build/trailer-0.24.0/data-prep-row'), 4200),   # ... then BN4's ThunderMan: SOUL BN4
}
SAVED = {'form-soul4-thun': 'prep-thun', 'form-soul4-aqua': 'prep-aqua', 'form-row': 'prep-row'}


def prep(work, keep=False):
    """PREP's runs in order, then each scene of SAVED its own copy of its run's data folder (work/saved-SCENE)"""
    import shutil
    import sys
    trailer = sys.modules.get('trailer') or __import__('trailer')
    for name, (argv, last) in PREP.items():
        if keep and os.path.isdir(os.path.join(work, 'data-' + name)):
            continue
        trailer.capture(name, argv, AUTO, None, last, last)
    for scene, name in SAVED.items():
        shutil.rmtree(os.path.join(work, 'saved-' + scene), ignore_errors=True)
        shutil.copytree(os.path.join(work, 'data-' + name), os.path.join(work, 'saved-' + scene))


def saved(scene):
    return ['--data-dir', f'/src/.build/trailer-0.24.0/saved-{scene}']


# CONTINUE from the title on the run a guardian paid, its arrival paged; the Soul's chip (30 Elec, 591 BN4's Bubbler
# moved, Aqua) as the whole folder, then UNITE as UNITE above (forms.py's SOUL4_USED)
def soul4(scene, chip, code, look, pages=12):
    script = ('60:,6:START,60:,6:A,400:,' + '6:A,30:,' * pages + f'0:chip {chip} {code} 30,0:foes {FOE},260:,'
              + '6:A,10:,6:A,10:,6:A,20:,6:START,10:,6:DOWN,10:,6:A,60:,6:START,10:,6:A,' + SHOTS)
    return (['--scene', 'title', '--dev', f'quiet,god,look={look}', *saved(scene)], {}, script, 900, frames(script))


# the setup over a profile with both Soul forms open (--dev forms shows the row), its cursor on FOLDER: Down to FORM,
# Right to SOUL BN5 and SOUL BN4, Down to the Soul brought and Right (ThunSoul, won), Up and Right round to CROSS
ROW = '90:,6:DOWN,60:,6:RIGHT,100:,6:RIGHT,100:,6:DOWN,60:,6:RIGHT,100:,6:UP,40:,6:RIGHT,100:'
SCENES.update({
    'form-soul4-thun': soul4('form-soul4-thun', 30, 'B', LOOKS['proto']),
    # (AquaSoul's run goes on at layer 4 with Chaud's words: more pages before the foes)
    'form-soul4-aqua': soul4('form-soul4-aqua', 591, '*', LOOKS['mystery'], 30),
    'form-row': (['--scene', 'setup', '--dev', 'forms', *saved('form-row')], {}, ROW, 60, frames(ROW)),
})
SCENES['form-soul4-aqua'] = (*SCENES['form-soul4-aqua'][:3], 1780, SCENES['form-soul4-aqua'][4])   # (from the Custom screen on)

# name: {what a sample shows: sample index}
MARKS = {
    'look-field': {"MegaMan in Glitch magenta, Nebula violet and cyan on Sky's floor, a Navi by him": 0, 'walking on, a Navi below': 48,
                   'along the edge, the blue void': 100, 'the green warp pad ahead': 150, 'on the pad': 184, 'the warp (dark to 210)': 200,
                   'layer 3, its arrival pad': 224, 'walking on (to the end)': 260},
    'look-battle': {'the field opens, MegaMan in his look before three Mettaurs (Sky)': 8, 'the Custom screen: AirShot': 64,
                    'MiniBomb': 72, 'Cannon': 80, 'CHIP DATA TRANSMISSION': 96, 'the field, BATTLE START! (136 to 160)': 144,
                    'AirShot': 166, 'Cannon': 182, "MiniBomb's blast": 192, 'charging (the glints)': 214, 'the buster': 246,
                    "a Mettaur's shockwave, MegaMan steps": 280, 'the fight goes on': 330},
    'look-chat': {"Lan's HP: \"Home again,Lan!\", MegaMan's mugshot in his look": 4, "Lan: Alright! Let's get set for the next dive!": 70,
                  'MegaMan again: Two ways open this time! The pink pad and the link up top! (to the end)': 140},
    'look-custom': {'the field opens': 0, "the Custom screen, Cannon; his face in the emotion window in his look (by the 100)": 16,
                    'the cursor across: Recov10': 64, 'WideSwrd': 80, 'MiniBomb': 104, 'WhiCapsl picked': 120,
                    'CHIP DATA TRANSMISSION': 192, 'the field': 216, 'BATTLE START!': 248},
    'form-beast': {'the Custom screen, Beast Out marked': 100, 'CHIP DATA TRANSMISSION': 120, 'the field, MegaMan in white, black and red': 140,
                   'the dark, MegaMan alone': 170, 'the purple flare': 180, 'the flare wide': 200, 'the beast': 210,
                   'BATTLE START! (230 to 255)': 240, 'his shots': 270, 'fire rings rise': 320, 'rings, a hit': 400,
                   "a chip, the beast's gauge full": 520, 'the fight goes on': 560},
    'folders-shelf': {'the setup, NEXT': 0, 'the FOLDER row: Standard, 1/6': 32, 'Blade LOCKED: Swords, up close / Opens when something big falls': 60,
                      'Storm: Elec chips for Aqua foes / Opens where lightning strikes hardest': 110,
                      'Z-Canon: Cannons that make GigaCan1 / Opens when chips work as one, in order': 156,
                      'Busting: Heavy hitters, no Recovery / Opens for an act with no safety net': 204,
                      'Lean: Any five chips make a hand / Opens deep down in the Net': 252, 'Standard again': 300, 'Blade again': 346},
    'map-open': {'layer 2 (seed 3), MegaMan on his pad': 0, 'SELECT: the map open': 40, 'walking under it: the You dot moves': 160,
                 'the map drawn on as he walks': 250, 'more of it': 300, 'SELECT again: closed, MegaMan where he walked to': 368},
    'fast-forward': {"Sky, at the game's pace": 0, 'fast forward on: the double arrow top right': 112, 'the warp, fast': 136,
                     'layer 3, still fast': 150, 'running across, stairs': 220, 'on': 270, "MegaMan's words, still fast": 296,
                     "off again, the game's pace": 330},
    'reward-home': {"Lan's HP": 0, 'beside the courier Mr.Prog': 18, 'DELIVERY,DELIVERY!': 48, 'THE MAN FROM THE LAB SENT THIS FOR YOU!': 100,
                    'MegaMan got: "1000 Zenny"!!': 158, 'the box closed': 180},
    'chip-choice': {"Lan's HP, the courier": 20, 'THE NETBATTLER WANTS A WOOD CHIP!': 60, 'YOU HAVE A FEW!': 100, 'WHICH ONE SHOULD I TAKE HIM?': 150,
                    "BN6's Chip Trader screen: RskyHny1 B, RskyHny2 C": 200, 'the cursor on RskyHny2': 250, 'up and down again': 300,
                    'RskyHny2 put in': 320, "Lan,trade this one? Yes No (MegaMan's mugshot in his look)": 350,
                    'Lan handed over: "RskyHny2 C".': 410, "I'LL RUN IT OVER TO ASTERLAND": 460},
    **{f'look-many-{k}': {'the field opens, MegaMan in this look': 8, 'three Mettaurs arrive': 40, 'the Custom screen (64 to 96)': 70,
                          'the field': 120, 'BATTLE START! (136 to 160)': 144, 'AirShot (the last samples)': 166} for k in LOOKS},
    'form-row': {'the setup over a profile with both Soul forms open: FORM, CROSS (Crosses and BeastOut. Guardians give Crosses)': 20,
                 "SOUL BN5 (BN5's Souls. Guardians give Souls)": 50, "SOUL BN4 (BN4's Souls. Guardians give Souls)": 110,
                 'the row under it: Soul, None': 160, 'ThunSoul (From the first battle. Guardians give the rest)': 190,
                 'back on FORM': 240, 'CROSS again': 270},
    'party-solo': {'three MegaMen in the look, BlastMan (400) on the red field': 0, 'BATTLE START! (30 to 55)': 40,
                   'all three firing, CrakShot': 60, 'charging together': 80, 'BlastMan in the air': 100, 'a white blast on the front panel': 200,
                   'its smoke, a MegaMan struck (90)': 220, 'his flame on the field': 280, 'a blast on the MegaMen': 360, 'the fight goes on': 400},
}


def _cross(look, hits):
    """a Cross's take: CROSSSELECT and OK, the field in his look, the change, then his hits"""
    return {'the Custom screen, the Cross taken in CROSSSELECT': 100, 'CHIP DATA TRANSMISSION': 124,
            f'the field, three Mettaur3, MegaMan in {look}': 136, 'the dark': 164, 'the white flash': 172,
            'the Cross Navi beside him': 184, 'the Cross whole': 208, 'the field again': 220, 'BATTLE START! (228 to 250)': 236,
            'Cannon, a hit': 316, **hits}


def _soul(look, name, hits):
    """a Soul's take (BN5's, in BN6's engine): UNITE and OK, the field in his look, the Soul taken, then the fight"""
    return {'CHIP DATA TRANSMISSION after UNITE': 100, f'the field, three Mettaur3, MegaMan in {look}': 112, 'the dark': 148,
            f'{name} beside him': 160, 'the Soul taken': 172, 'MegaMan in the Soul': 184, 'BATTLE START! (208 to 235)': 220,
            'charging (the glints)': 410, 'the screen dims: Invisibl, MegaMan unseen (560 to 615)': 580, **hits}


MARKS.update({
    'form-cross-tengu': _cross("ProtoMan's red", {"TenguCross's charged swing, the crescent across the row": 404, 'again': 516}),
    'form-cross-dust': _cross("Hazard's green", {"DustCross's charged scrap crushing a Mettaur3 (79 to 19)": 400, 'again': 520, 'and again (111, 60)': 628}),
    'form-cross-ground': _cross("Mystery's purple", {"GroundCross's charged drill (to 34)": 424, 'the drill, its smoke': 436, 'the drill again (80 to 40)': 532,
                                                    'the drill at the last': 652}),
    'form-cross-elec': _cross("white, black and red", {"ElecCross's Cannon, an Elec hit": 316, 'his charged bolt down the row (to 64)': 520}),
    'form-soul5-napalm': _soul("Hazard's green", 'NapalmMan', {"his charged shot's target rings": 452, 'a Fire hit (to 90)': 740}),
    'form-soul5-proto': _soul("Glitch's magenta", 'ProtoMan', {'charging again (the pink spark)': 708, 'a hit': 740}),
    'form-soul5-magnet': _soul("Mystery's purple", 'MagnetMan', {'a hit': 740}),
    'form-soul5-gyro': _soul("ProtoMan's red", 'GyroMan', {"his charged shot's target ring": 440, 'a whirlwind, two hit (to 102 and 80)': 730}),
    'form-soul4-thun': {"CONTINUE: layer 4, MegaMan in ProtoMan's red": 0, "the Custom screen, Thunder (B) the folder's": 140,
                        'UNITE under OK': 200, 'the field, MegaMan in his look': 280, 'the dark': 288, 'ThunderMan in gold sparks': 296,
                        'the Soul taken': 320, 'MegaMan in ThunSoul': 328, 'BATTLE START! (352 to 376)': 360, 'a hit (119)': 408,
                        'his charged punch': 528, 'the fight goes on': 600},
    'form-soul4-aqua': {"the field, Chaud's words just over": 0, "the Custom screen, BN4's Bubbler (moved) the folder's": 28,
                        'UNITE under OK': 84, 'CHIP DATA TRANSMISSION': 132, "the field, MegaMan in Mystery's purple": 140, 'the dark': 172,
                        'AquaMan beside him': 180, 'the Soul taken': 204, 'MegaMan in AquaSoul': 212, 'BATTLE START! (230 to 255)': 240,
                        'bubbles burst on a Mettaur3 (120 to 100)': 300, 'a bubble shot on two': 420, 'bubbles again': 528,
                        'a bubble spread (96, then 80)': 636},
})

BN6_ONLY = ()   # every scene was taken with the whole ROM folder (the forms need BN5's and BN4's ROMs)

# what could not be taken as asked
MISSING = {
    'big hits on a Navi': "the forms first struck BN5's KnightMan (1D4) and TomahawkMan (1D0), then BN6's HeatMan (101): over a "
                          "whole take KnightMan lost 1 HP, TomahawkMan and HeatMan none (a cw.py check: HeatMan 697 of 700 after "
                          "two Cannons and the buster; the scripted shots, blind, miss a Navi that moves). Three Mettaur3 (120 HP, "
                          "they come to MegaMan's row) take the hits instead, MegaMan a column forward (mm@32). The Crosses' charged "
                          "attacks land (Tengu's crescent, Dust's scrap, Ground's drill, Elec's bolt), and AquaSoul's bubbles; Team "
                          "ProtoMan's Souls' charged shots mostly miss or are not loosed (B held 110 frames, then 170: the same), "
                          "so their takes are the change itself, then a hit or two late on. `onehit` was never used.",
    'map-open (MegaMan under the map)': "the map is drawn over the whole 240x160 picture, so while it stays open MegaMan is seen as "
                                        "the map's You dot moving and the map drawn on, not walking beneath it",
    'form-soul5 (SearchSoul, MeddySoul, NumSoul)': "not taken: four of Team ProtoMan's (Napalm, Proto, Magnet, Gyro) cover the "
                                                   "ask; SearchSoul (190, Cursor) and MeddySoul (154, Recovery) unite the same way "
                                                   "(soul5(0x100, 190, ...), soul5(0x800, 154, ...)). NumSoul is BN4's, as GutsSoul.",
    'form-soul4 (GutsSoul)': "not taken: BN4's GutsMan (guardian 35) would pay it in a SOUL BN4 run as ThunderMan and AquaMan pay "
                             "theirs, but the chip kind that unites it was not looked up",
}
