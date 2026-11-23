"""A dark layer's board with BN5's ROM beside BN6's (docs/game/liberation/building/stage-3-team.md,
cross-navis-team.md): BN5's own board and Darkloid, the team of the Navis
whose Souls or Crosses the run holds, each Navi's turn and his command, a
teammate's battle fought as MegaMan in his Soul or Cross on his HP, his
being struck down, the team picked, a BonusPanel's roulette. (--dev
souls=MASK holds BN5's Souls, a bit each from Colonel's, then Team
ProtoMan's from bit 6 (64 ProtoSoul); --setup's CROSS brings a Cross Navi; a team's board is its own, so a Navi's seed is one
where his command can be taken from the first phase.)"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from liberation_board import BRIEFED, PRIZED, RESULT, Course  # noqa: E402

MANY = {}


def scene(name, guards, start, steps):
    MANY[name] = dict(GUARDS=' '.join(guards.split()), START=start, STEPS=list(steps), NEEDS=('bn5',))


def board(dev='', seed=5, setup=''):
    return f'--scene emu --seed {seed} --run-depth 5 --dev dark{dev}{setup}'


def soul(mask, dev='', seed=5):
    return board(f',souls={mask}{dev}', seed)


def cross(navi, dev='', seed=5):
    return board(dev, seed, f' --setup endless,0,0,0,{navi},0')


def in_play(name):
    """R: play passed to the next Navi yet to act, his word as he first comes into play paged."""
    return [('do', 'press R; wait 10'), ('repeat', 'press A; wait 20', f'inplay={name}', 40), ('repeat', 'press A; wait 20', 'board=move', 40)]


# ---- BN5's board, the team's turns ----

scene('look', """BN5's own board: its map, panels and pieces (none of ours recoloured over them), its Darkloid at the far end,
      MegaMan alone where the run holds no Soul.""", board(), BRIEFED + [
    ('expect', 'boardlook=bn5'), ('expect', 'area=Oran Area'), ('expect', 'darkloid=ShadeMan standing'),
    ('expect', 'inplay=MegaMan'), ('expect', 'mate0=MegaMan 100/100 inplay'), ('expect', 'boardphase=1'), ('expect', 'boardexit=shut'),
    ('do', 'board dark; wait 4'), ('expect', 'faces=dark'), ('expect', 'darkshown=0'),
])

c = Course(('expect', 'mate1=Colonel 200/200 ready'), ('expect', 'inplay=MegaMan'))
c.command('dark').take().win(1)
c.add(RESULT, ('repeat', 'press A; wait 20', 'inplay=Colonel', 40), ('repeat', 'press A; wait 20', 'board=move', 40),
      ('expect', 'boardphase=1'), ('expect', 'side=player'), ('expect', 'mate0~acted'), ('expect', 'mate1~inplay'))
c.command('dark').take().win(1).paged()
c.add(('expect', 'boardphase=2'), ('expect', 'inplay=MegaMan'), ('expect', 'mate0~inplay'), ('expect', 'mate1~ready'))
scene('turns', """The team's turns: Colonel beside MegaMan at BN5's 200 HP; MegaMan's command passes play to him, his to the
      Darkloids' phase, and the next phase begins with MegaMan again.""", soul(1, ',god'), c.steps)

c = Course()
for _ in range(6):
    c.command('next').take().win(1).paged()
c.add(('expect', 'darkholes=0'), ('expect', 'boardphase=7'), ('expect', 'par=9'))
c.command('darkloid').take().add(('expect', 'fights=darkloid'), ('expect', 'foehp=600')).win(1)
c.add(RESULT, ('expect', 'darkloid=ShadeMan deleted'), ('expect', 'prize=under'), PRIZED,
      ('expect', 'boardexit=open'), ('do', 'hold DOWN+RIGHT 60'), ('until', 'layer=6', 1200), ('until', 'board=none', 1200))
scene('cleared', """BN5's board cleared by MegaMan alone in seven phases, two under its par of 9: ShadeMan deleted in his own
      battle from the board's 600 HP, the best prize, the exit open where he stood and the next layer a plain
      one.""", board(',god'), c.steps)


# ---- each Navi's command, an Order Point each ----

def commanded(name, label, kind, faces, fought, *after):
    """Teammate `name` in play first (R), his command `label` taken on the
    nearest `kind`; `fought`: the state's line of the Soul or Cross his
    battle is fought in (None: a command with no battle). Then MegaMan,
    yet to act, is in play in the same phase."""
    c = Course(('expect', f'mate1~{name}'), ('keep', 'orderpoints freed darkpanels items boardphase guardians'), *in_play(name))
    c.add(('do', f'board {kind}; wait 4'), ('expect', f'faces={faces}'), ('repeat', 'press A; wait 10', 'board=menu', 4),
          ('repeat', 'press DOWN; wait 2', f'command={label}', 3))
    if fought:
        # (his battle: MegaMan in his Soul or Cross from the first turn, on his HP)
        c.take().win(2, first=[('until', fought, 900), ('expect', 'hp!=100/100')]).paged()
        c.add(('expect', 'lastfight=won'), ('expect', 'freed>0'))
    else:
        c.add(('expect', 'takes=yes'), ('do', 'press A; wait 10'), ('repeat', 'press A; wait 20', 'board=move', 40), ('expect', 'lastfight=none'))
    return c.add(('expect', 'inplay=MegaMan'), ('expect', 'mate1~acted'), ('expect', 'mate0~MegaMan 100/100 inplay'),
                 ('moved', 'boardphase', 0), *after).steps


scene('command/colonel', """Colonel's ScrenDiv: more panels by one battle, fought as MegaMan in ColonelSoul on Colonel's HP.""",
      soul(1, ',god'), commanded('Colonel', 'ScrenDiv', 'dark', 'dark', 'soul=7'))
scene('command/tomahawkman', """TomahawkMan's TomahawkSwing: more panels by one battle, fought in TomahawkSoul.""",
      soul(8, ',god', 1), commanded('TomahawkMan', 'TomahawkSwing', 'dark', 'dark', 'soul=10'))
scene('command/numberman', """NumberMan's NumberCheck: the items ahead taken with no battle, their panels left dark.""",
      soul(4, ',god', 1), commanded('NumberMan', 'NumberCheck', 'item', 'item', None,
                                    ('moved', 'items', -1), ('moved', 'darkpanels', 0), ('moved', 'freed', 0)))
scene('command/shadowman', """ShadowMan's SneakAttack: a guardian in his reach struck with no battle, its next battle the easier.""",
      soul(2, ',god', 4), commanded('ShadowMan', 'SneakAttack', 'sneak', 'dark', None,
                                    ('expect', 'unit1~weak|unit2~weak'), ('moved', 'guardians', 0), ('moved', 'orderpoints', -1)))
scene('command/toadman', """ToadMan's LifeMelody: sung for MegaMan beside him, yet to act.""",
      soul(32, ',god'), commanded('ToadMan', 'LifeMelody', 'mate', 'mate', None, ('expect', 'mate0~sung'), ('moved', 'orderpoints', -1)))
scene('command/heatman', """HeatMan's Napalm (a Cross Navi's, Team ProtoMan's): by a battle fought as MegaMan in HeatCross.""",
      cross(1, ',god'), commanded('HeatMan', 'Napalm', 'dark', 'dark', 'cross=1'))
scene('command/slashman', """SlashMan's WideSwrd: by a battle fought in SlashCross.""",
      cross(3, ',god'), commanded('SlashMan', 'WideSwrd', 'dark', 'dark', 'cross=3'))
scene('command/eraseman', """EraseMan's PanelSearch: its line's item panel freed by a battle fought in EraseCross.""",
      cross(4, ',god', 2), commanded('EraseMan', 'PanelSearch', 'item', 'item', 'cross=4', ('moved', 'items', -1)))
scene('command/elecman', """ElecMan's MagnetBarrier: no battle, no harm on the board till the next phase.""",
      cross(2, ',god'), commanded('ElecMan', 'MagnetBarrier', 'dark', 'dark', None, ('expect', 'shield=on'), ('moved', 'orderpoints', -1)))
scene('command/chargeman', """ChargeMan's BombLiberation: the dark panel he flies over freed with no battle.""",
      cross(5, ',god', 6), commanded('ChargeMan', 'BombLiberation', 'over', 'dark', None, ('moved', 'freed', 1), ('moved', 'orderpoints', -1)))

# (ProtoSoul held: the real ProtoMan, Chaud's, in BN6's own body, list 6's 0x3B, his own WideSwrd)
c = commanded('ProtoMan', 'WideSwrd', 'dark', 'dark', 'soul=1')
c.insert(c.index(('do', 'board dark; wait 4')), ('expect', 'body=59'))
scene('command/protoman', """A held Team ProtoMan Soul brings its own Navi: ProtoMan on the board in BN6's own body, his own WideSwrd
      by a battle fought as MegaMan in ProtoSoul on his HP.""", soul(64, ',god'), c)

c = Course(('expect', 'mate1~KnightMan'), *in_play('KnightMan'))
c.command('dark').add(('expect', 'command=Liberation'), ('do', 'press DOWN'), ('expect', 'command=Pass'), ('do', 'press DOWN'),
                      ('expect', 'command=Liberation'))
scene('command/knightman', """KnightMan has no command: his window holds Liberation and Pass alone.""", soul(16), c.steps)


# ---- struck down, the pick, the roulette ----

c = Course(('expect', 'mate1=Colonel 1/200 ready'), ('keep', 'freed'), *in_play('Colonel'))
c.command('dark').take().add(('repeat', 'press START; wait 10; press A; wait 50', 'custom>0', 10),
                             ('repeat', 'mash A 60', 'screen=map', 20), ('until', 'lastfight=lost', 600), ('expect', 'foughton=0'),
                             ('expect', 'mate1~out'), ('expect', 'scene=emu'), ('moved', 'freed', 0),
                             ('repeat', 'press A; wait 20', 'board=move', 40),
                             ('expect', 'inplay=MegaMan'), ('expect', 'mate0=MegaMan 100/100 inplay'), ('expect', 'boardphase=1'))
scene('struck-down', """A teammate struck down in his battle (--dev matehp=1: the first blow): LIBERATE FAILED!, he leaves the
      board, and MegaMan and the run go on.""", soul(1, ',matehp=1'), c.steps)

# (Colonel and ShadowMan held, a place for one: seed 5's board takes ShadowMan)
PICKED = [('until', 'board=pick', 1500), ('expect', 'teampick=ShadowMan'), ('expect', 'teamrow=GO')]
scene('pick/kept', """More Navis held than the board has places: the team is picked on arrival. START takes the board's own, and
      is the pick's alone (held on, it opened BN6's PET over the briefing).""", soul(3), PICKED + [
    ('do', 'press START; wait 10'), ('expect', 'screen=map')] + BRIEFED + [('expect', 'screen=map'), ('expect', 'mate1~ShadowMan')])
scene('pick/changed', """The team's pick: A on another Navi takes him in the one place, and the board begins with him.""",
      soul(3), PICKED + [
    ('do', 'press UP; press UP'), ('expect', 'teamrow=Colonel'), ('do', 'press A'), ('expect', 'teampick=Colonel'),
    ('do', 'press START; wait 10')] + BRIEFED + [('expect', 'mate1~Colonel'), ('expect', 'inplay=MegaMan')])

# (ProtoSoul held and SlashCross brought, a place for one: SlashMan keeps the WideSwrd he was lent, and the pick chooses)
PROTO_SLASH = board(',souls=64', 5, ' --setup endless,0,0,0,3,0')
scene('pick/protoman', """ProtoMan beside a Cross Navi of his command: ProtoMan and SlashMan both held for one place, the pick
      lists both, and A on ProtoMan takes him in it.""", PROTO_SLASH, [
    ('until', 'board=pick', 1500), ('expect', 'teampick=SlashMan'), ('expect', 'teamrow=GO'),
    ('do', 'press UP; press UP'), ('expect', 'teamrow=ProtoMan'), ('do', 'press A'), ('expect', 'teampick=ProtoMan'),
    ('do', 'press START; wait 10')] + BRIEFED + [('expect', 'mate1~ProtoMan'), ('expect', 'inplay=MegaMan')])


def roulette(slot, before, *after):
    """A BonusPanel's roulette stopped by A on `slot`, caught as it comes
    up after `before` (a slot shows four frames)."""
    c = Course(('keep', 'bonuspanels orderpoints guardians darkloidhp boardphase'))
    c.command('bonus').add(('expect', 'takes=yes'), ('do', 'press A; wait 10'), ('repeat', 'press A; wait 20', 'board=bonus', 20),
                           ('repeat', 'wait 1', f'roulette={before}', 400), ('repeat', 'wait 1', f'roulette={slot}', 8),
                           ('do', 'press A; wait 4'), ('repeat', 'press A; wait 20', 'board=move', 40),
                           ('expect', f'bonus={slot}'), ('moved', 'bonuspanels', -1), ('moved', 'boardphase', 0), ('expect', 'mate0~inplay'), *after)
    return c.steps


scene('roulette/shield', """A BonusPanel's roulette (BN5's) stopped on its shield: no harm on the board this phase.""",
      board(), roulette('shield', 'hit', ('expect', 'shield=on')))
scene('roulette/hp', """The roulette stopped on HP: MegaMan's HP full.""", board(), roulette('hp', 'shield', ('expect', 'hp=100/100')))
scene('roulette/order-points', """The roulette stopped on Order Points: five more.""",
      board(), roulette('orderpoints', 'hp', ('moved', 'orderpoints', 5)))
scene('roulette/chip', """The roulette stopped on a chip: one of the board's four given.""", board(), roulette('chip', 'orderpoints'))
scene('roulette/hit', """The roulette stopped on Major Hit: the nearest guardian deleted.""",
      board(), roulette('hit', 'chip', ('moved', 'guardians', -1)))
