"""A dark layer's board with BN6's ROM alone (docs/game/liberation/): its
opening, each command and what it leaves on the board, the Darkloids'
phase, the board cleared and its prize, a CONTINUE on it, its special
route. MegaMan is stood beside a panel by the `board` dev step (the
nearest of a kind his walk reaches) and takes his command by the pad; a
battle is ended from the dev menu's "Win this battle", so BN6's own loop
ends it in the turn the scenario names (docs/devtools/scenarios.md, A dark
layer's board)."""

# (seed 5's act 2 board: 2 DarkHoles, a BigBrute on each, a BonusPanel on
# the way, par 10)
DARK = '--scene emu --seed 5 --run-depth 5 --dev dark'
# (seed 7's act 2 holds a dark layer behind its own way, no dev switch)
OWN = '--scene emu --seed 7 --run-depth 5'
# (seed 1's act 5 board: trap panels, from act 5)
ACT5 = '--scene emu --seed 1 --run-depth 14 --dev dark'
# (seed 2's act 3 board: a sealed region, its key and its route.
# CYBERWORLD_AUTOPILOT_ROUTE: `board next` goes for the key, then the route)
ACT3 = 'CYBERWORLD_AUTOPILOT_ROUTE=1 --scene emu --seed 2 --run-depth 8 --dev dark,god'

# The briefing paged, to MegaMan's first move
BRIEFED = [
    ('until', 'board=talk', 1200),
    ('repeat', 'press A; wait 20', 'board=move', 40),
]
# The Custom screen left by OK with no chip chosen (START goes to OK; in
# the fight START pauses and the next one goes on, so it is pressed until
# the gauge runs)
OK = ('repeat', 'press START; wait 10; press A; wait 50', 'custom>0', 10)
# A battle's end paged (its banner, RESULT), to the board
RESULT = ('repeat', 'mash A 60', 'screen=map', 40)
# ... then the board's words and the Darkloids' phase, to the next move
PAGED = [RESULT, ('repeat', 'press A; wait 20', 'board=move|board=done', 60)]
# The Darkloids' phase paged, to the next move
PHASED = ('repeat', 'press A; wait 20', 'board=move', 60)
# The prize given and the exit's word said
PRIZED = ('repeat', 'press A; wait 20', 'board=done', 60)
# What a CONTINUE must bring back of a board
KEPT = 'layer area boardphase par orderpoints darkpanels freed items bonuspanels darkholes guardians unit1 unit2 darkloidhp hp zenny'
CONTINUED = [
    ('restart', '--scene title'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'board=move', 2400),
    ('same', KEPT),
]


class Course:
    """A scenario's steps, built: the dev menu's cursor stays on the row it
    last took, so the first win walks it there and the next only press A."""

    def __init__(self, *steps):
        self.steps = list(BRIEFED) + list(steps)
        self.won = False

    def add(self, *steps):
        self.steps += steps
        return self

    def command(self, kind, faces=None, down=0):
        """The Navi in play beside the nearest panel of `kind` (facing
        `faces`, where the kind's name is not what he faces), his window
        opened on it, the cursor `down` rows down."""
        faced = f'faces={faces or kind}' if faces or kind != 'next' else 'faces!=clean'
        self.add(('do', f'board {kind}; wait 4'), ('expect', faced), ('repeat', 'press A; wait 10', 'board=menu', 4))
        if down:
            self.add(('do', '; '.join(['press DOWN'] * down)))
        return self

    def bonus(self):
        """The nearest BonusPanel freed (no battle, no action spent), its gift paged."""
        return self.command('bonus').add(('expect', 'takes=yes'), ('do', 'press A; wait 10'), ('repeat', 'press A; wait 20', 'board=move', 40))

    def take(self):
        """The command under the cursor taken, to its battle's first Custom screen."""
        return self.add(('expect', 'takes=yes'), ('do', 'press A; wait 10'), ('until', 'turn>=1', 900), ('expect', 'rules=liberation'))

    def win(self, turn=1, first=()):
        """The battle won in turn `turn`: the Custom screens before it left
        by OK (`first`: steps once the first turn's fight runs), then the
        enemies deleted."""
        for t in range(1, turn + 1):
            self.add(('until', f'turn={t}', 1500), OK, *(first if t == 1 else ()))
        walk = '' if self.won else 'press DOWN; press DOWN; press DOWN; press DOWN; '
        self.won = True
        return self.add(('do', f'hold SELECT 4; hold SELECT+R 4; wait 6; {walk}press A; wait 6'), ('expect', 'screen=battle'))

    def time_up(self, each=()):
        """The battle left to its third turn's end (`each`: steps in each turn's fight)."""
        for t in (1, 2, 3):
            self.add(('until', f'turn={t}', 1500), OK, *each)
        return self

    def paged(self):
        return self.add(*PAGED)

    def passed(self, kind):
        """Pass taken beside the nearest `kind` (the window opens where a
        command can be taken), to the Darkloids' phase."""
        return self.command(kind, down=2).add(('expect', 'command=Pass'), ('expect', 'takes=yes'), ('do', 'press A; wait 10'),
                                              ('until', 'side=darkloids', 600))


MANY = {}


def scene(name, guards, start, steps):
    MANY[name] = dict(GUARDS=' '.join(guards.split()), START=start, STEPS=list(steps))


# ---- the board begun ----

scene('opens', """A dark layer entered: Lan's briefing, then the board under way: MegaMan's phase 1, three Order Points,
      every DarkHole, guardian and the Darkloid standing, the exit shut, and no random battle on its floor.""", DARK, [
    ('expect', 'layer=5'),
    ('expect', 'board=opening'),
    ('until', 'board=talk', 1200),
    ('expect', 'chat=open'),
    ('expect', 'boardexit=shut'),
    ('repeat', 'press A; wait 20', 'board=move', 40),
    ('expect', 'side=player'), ('expect', 'boardphase=1'), ('expect', 'orderpoints=3'),
    ('expect', 'freed=0'), ('expect', 'darkholes=2'), ('expect', 'guardians=2'),
    ('expect', 'darkloid~standing'), ('expect', 'prize=pending'), ('expect', 'boardexit=shut'),
    ('expect', 'inplay=MegaMan'), ('expect', 'lastfight=none'),
    # (a walk up and down its first way)
    ('do', 'hold DOWN+RIGHT 120; hold UP+LEFT 120; hold DOWN+RIGHT 120; hold UP+LEFT 120'),
    ('expect', 'screen=map'), ('expect', 'board=move'), ('expect', 'boardphase=1'),
])

scene('own-seed', """A dark layer by the run's own seed, no dev switch: seed 7's act 2 holds one behind its own way,
      and its board begins.""", OWN, BRIEFED + [('expect', 'layer=5'), ('expect', 'boardphase=1'), ('expect', 'darkholes>0')])

scene('switched-off', """Liberation switched off in the run's setup (EXTRAS; --setup's OFF bit 1): the layer that would be
      dark is a plain one.""", OWN + ' --setup endless,0,0,0,0,1', [('expect', 'layer=5'), ('expect', 'board=none')])
scene('switched-off/dev-dark', """Liberation switched off in the run's setup: --dev dark makes no dark layer either.""",
      DARK + ' --setup endless,0,0,0,0,1', [('expect', 'layer=5'), ('expect', 'board=none')])

scene('fence', """The dark panels are fenced: MegaMan beside one, the pad held each way in turn, gets no further than
      its rim.""", DARK, BRIEFED + [
    step for way in ('DOWN+RIGHT', 'DOWN+LEFT', 'UP+LEFT', 'UP+RIGHT')
    for step in (('do', f'board dark; wait 4; hold {way} 60'), ('expect', 'under=clean|under=bonus|under=rim'), ('expect', 'board=move'))
])

scene('shown', """With BN6's ROM alone the board shows: its dark panels the floor recoloured, the panel MegaMan faces lit
      (the recolouring read BN6's scroll as wrapping at 512, and drew eight layers of nine 256 pixels off or not at
      all).""", DARK, BRIEFED + [
    ('do', 'board dark; wait 4'), ('expect', 'faces=dark'), ('expect', 'darkshown>1000'), ('expect', 'facedshown>500'),
])


# ---- Liberation ----

def liberate(turn, *after, start=DARK):
    c = Course(('keep', 'freed boardphase darkpanels zenny'))
    c.command('dark').add(('expect', 'command=Liberation')).take().add(('expect', 'fights=panel'))
    if turn:
        c.win(turn)
    else:
        c.time_up()
    return start, c.paged().add(*after).steps


scene('liberate/won', """Liberation on a dark panel, its battle won in the second turn: that panel freed and no other, the
      Darkloids' phase, then phase 2.""", *liberate(
    2, ('expect', 'lastfight=won'), ('moved', 'freed', 1), ('moved', 'darkpanels', -1), ('moved', 'boardphase', 1), ('expect', 'side=player')))
scene('liberate/one-turn', """Liberation won in its first turn (1 TURN LIBERATE!): the dark panels round it are swept
      too.""", *liberate(1, ('expect', 'lastfight=oneturn'), ('expect', 'freed>1'), ('moved', 'boardphase', 1)))
scene('liberate/time-up', """Liberation not won by its third turn's end (TIME UP!, LIBERATE FAILED!): the panel dark still,
      no reward, the phase spent.""", *liberate(
    0, ('expect', 'lastfight=timeup'), ('moved', 'freed', 0), ('moved', 'darkpanels', 0), ('moved', 'zenny', 0), ('moved', 'boardphase', 1),
    start=DARK + ',god'))

c = Course(('keep', 'guardians deleted freed'))
c.command('guardian').take().add(('expect', 'fights=guardian')).win(2)
c.add(RESULT, ('until', 'side=darkloids', 600), ('moved', 'guardians', -1), ('moved', 'deleted', 1), ('moved', 'freed', 1),
      PHASED, ('moved', 'guardians', 0), ('expect', 'darkholes=2'))
scene('guardian', """A DarkHole's guardian fought and deleted: his panel freed; his DarkHole standing, he is back beside it at
      the Darkloids' phase's end.""", DARK + ',god', c.steps)

c = Course()
c.command('dark').take().win(1).paged()
c.add(('keep', 'darkholes closed guardians freed'))
c.command('darkhole').take().add(('expect', 'fights=darkhole')).win(2).paged()
c.add(('moved', 'darkholes', -1), ('moved', 'closed', 1), ('moved', 'guardians', -1), ('expect', 'lastfight=won'))
scene('darkhole', """A DarkHole closed: the way to it opened by a one-turn win, then its own battle: the hole gone and its
      guardian with it.""", DARK + ',god', c.steps)

c = Course(('keep', 'bonuspanels boardphase'))
c.bonus().add(('expect', 'screen=map'), ('moved', 'bonuspanels', -1), ('moved', 'boardphase', 0),
              ('expect', 'bonus!=none'), ('expect', 'mate0~inplay'), ('expect', 'lastfight=none'))
c.command('dark').add(('expect', 'takes=yes'))
scene('bonus', """A BonusPanel: Liberation on it frees it with no battle and gives its gift, and MegaMan's command is still his
      to take this phase.""", DARK, c.steps)

c = Course(('keep', 'orderpoints items'))
c.command('orderpoints', 'item').take().win(2).paged()
c.add(('expect', 'orderpoints>3'), ('expect', 'orderpoints<=8'), ('moved', 'items', -1))
scene('item/order-points', """An item panel of Order Points liberated: the points are his, 8 at most.""", DARK + ',god', c.steps)

c = Course()
c.passed('guardian').add(PHASED, ('expect', 'hp=80/100'))
c.command('heart', 'item').take().win(1)
c.add(RESULT, ('until', 'side=darkloids', 600), ('expect', 'hp=100/100'), ('expect', 'freed>0'))
scene('item/heart', """An item panel's heart: three tenths of his HP back (the 20 a BigBrute took in the phase before, all of
      it).""", DARK, c.steps)

c = Course(('expect', 'traps=4'), ('keep', 'hp traps'))
c.command('trap').take().win(1)
c.add(RESULT, ('until', 'side=darkloids', 600), ('moved', 'traps', -1), ('moved', 'hp', -50), ('expect', 'lastfight=oneturn'))
scene('trap', """A trap panel (from act 5), shown as one: liberated, its toll of 50 is off his HP.""", ACT5, c.steps)


# ---- the other commands, the phases ----

c = Course(('keep', 'orderpoints freed'))
c.command('pair', 'dark', down=1).add(('expect', 'command=LongSwrd')).take().win(2).paged()
c.add(('moved', 'orderpoints', -1), ('moved', 'freed', 2))
scene('longswrd', """LongSwrd: an Order Point pays for two panels in a line freed by one battle.""", DARK + ',god', c.steps)

c = Course()
c.passed('guardian').add(('expect', 'hp=100/100'), PHASED, ('expect', 'boardphase=2'), ('expect', 'hp<100'), ('expect', 'hp>0'), ('keep', 'hp'))
c.passed('guardian').add(('moved', 'hp', 20), PHASED, ('expect', 'boardphase=3'))
scene('pass', """The Darkloids' phase: a guardian strikes MegaMan beside it. Then Pass: a fifth of his HP back.""", DARK, c.steps)

c = Course(('expect', 'darkloidhp=100/600'))
c.passed('dark').add(PHASED, ('expect', 'boardphase=2'), ('expect', 'darkloidhp=580/600'))
c.passed('dark').add(PHASED, ('expect', 'darkloidhp=600/600'))
scene('darkloid/heals', """The Darkloid's HP is the board's (--dev libhp=100 begins him at 100 of 600): in his phase he heals a
      tenth of his max for each dark panel round him, eight of them, then to his max.""", DARK + ',libhp=100', c.steps)


# ---- the board cleared ----

def to_darkloid():
    """Seed 5's board played to the Darkloid in five phases, each battle
    won in its first turn (the autopilot's choice of panel, `board next`;
    the two BonusPanels it would take on the way first): both DarkHoles, a
    panel of his ring, then his battle's first Custom screen."""
    c = Course()
    c.bonus()
    for phase in range(1, 5):
        c.command('next').take().win(1).paged().add(('expect', f'boardphase={phase + 1}'))
    c.add(('expect', 'darkholes=0'), ('expect', 'closed=2'), ('expect', 'prize=pending'), ('expect', 'boardexit=shut'))
    c.bonus()
    c.command('next', 'ring').take().win(1).paged()
    return c.command('darkloid').take().add(('expect', 'fights=darkloid'), ('expect', 'foehp=600'))


def cleared(*after):
    c = to_darkloid().win(1)
    return c.add(RESULT, ('expect', 'darkloid~deleted'), ('expect', 'boardphase=6'), ('keep', 'zenny'), *after).steps


scene('cleared/under-par', """The board cleared two phases under its par or better (phase 6, par 10): the best prize, a chip; the
      exit open where the Darkloid stood, and the next layer a plain one.""", DARK + ',god', cleared(
    ('expect', 'par=10'), ('expect', 'prize=under'), PRIZED, ('expect', 'boardexit=open'), ('moved', 'zenny', 0),
    # (MegaMan stands beside the exit)
    ('do', 'hold DOWN+RIGHT 60'), ('until', 'layer=6', 1200), ('until', 'board=none', 1200)))
scene('cleared/at-par', """The board cleared at its par or a phase under (--dev libpar=7): the second prize, a chip.""",
      DARK + ',god,libpar=7', cleared(('expect', 'par=7'), ('expect', 'prize=par'), PRIZED, ('expect', 'boardexit=open'), ('moved', 'zenny', 0)))
scene('cleared/over-par', """The board cleared over its par (--dev libpar=5): zenny, BN5's 1000 at the second act by the economy's
      zenny curve (653: docs/game/liberation/building/par-prize.md).""",
      DARK + ',god,libpar=5', cleared(('expect', 'par=5'), ('expect', 'prize=over'), PRIZED, ('expect', 'boardexit=open'), ('moved', 'zenny', 653)))

c = to_darkloid().time_up(each=[('do', 'mash B 240')])
c.add(RESULT, ('until', 'side=darkloids', 600), ('expect', 'lastfight=timeup'), ('expect', 'darkloid~standing'),
      ('expect', 'darkloidhp<600'), ('expect', 'darkloidhp>0'), ('expect', 'prize=pending'), ('expect', 'boardexit=shut'))
scene('darkloid/not-won', """The Darkloid fought from the board's 600 HP and not deleted by his battle's third turn: the HP the
      battle left him is his on the board, he stands, and the exit stays shut.""", DARK + ',god', c.steps)


# ---- a quit, then CONTINUE ----

c = Course()
c.command('dark').take().win(1).paged()
c.command('dark', down=1).add(('expect', 'command=LongSwrd')).take().win(1).paged()
c.add(('expect', 'boardphase=3'), ('expect', 'orderpoints=2'), ('expect', 'freed>2'), ('keep', KEPT))
scene('continue', """A quit on the board in its third phase, then CONTINUE: the same board, its panels freed, its phase, Order
      Points, units and the Darkloid's HP as they were.""", OWN, c.steps + CONTINUED)

c = Course()
c.command('dark').take().win(1).paged()
c.command('dark').add(('expect', 'board=menu'), ('keep', KEPT))
scene('continue/window-open', """A quit with MegaMan's command window open, then CONTINUE: the board as it was, the window
      closed and his command still his to take.""", OWN, c.steps + CONTINUED + [('expect', 'mate0~inplay'), ('expect', 'boardphase=2')])

c = Course()
c.command('dark').take().win(1)
# (RESULT paged a press at a time, to his words of the one-turn win and no further)
c.add(('repeat', 'press A; wait 8', 'talk=director', 300),
      ('expect', 'board=talk'), ('expect', 'lastfight=oneturn'), ('expect', 'freed>1'), ('keep', 'layer area freed darkpanels zenny'),
      ('restart', '--scene title'),
      ('repeat', 'press START; wait 40', 'title=menu'),
      ('expect', 'cursor=continue'),
      ('repeat', 'press A; wait 40', 'scene=emu'),
      ('repeat', 'press A; wait 20', 'board=move', 80),
      ('same', 'layer area freed darkpanels zenny'), ('expect', 'boardphase=2'))
scene('continue/under-the-words', """A quit under MegaMan's words after a won battle, then CONTINUE: the battle stays won, its panel
      freed and its zenny kept (the words began while BN6 still held him, with no save before them, and the run went
      back to its last checkpoint; found by the fuzz of quits, --fuzz 3/12).""", OWN, c.steps)

# ---- the special route (docs/game/liberation/special-routes.md) ----

c = Course(('expect', 'sealed=shut'), ('expect', 'sealedroute=hidden'), ('expect', 'barriers=1'))
c.bonus()
c.command('next', 'dark').take().win(1).paged()
c.command('next', 'guardian').take().win(1).paged()
c.command('key', 'item').take().win(1).paged().add(('expect', 'barriers=0'), ('expect', 'sealed=open'), ('expect', 'sealedroute=hidden'))
c.command('next').take().win(1).paged()
c.command('next').take().win(1).paged().add(('expect', 'sealedroute=found'), ('expect', 'route~found'))
scene('route/found', """From act 3 a board holds a sealed region: its key liberated opens its barrier, and the route behind it,
      liberated, is found.""", ACT3, c.steps)

for _ in range(4):
    c.command('next').take().win(1).paged()
c.command('darkloid').take().win(1).add(RESULT, PRIZED, ('expect', 'boardexit=open'))
c.add(('do', 'hold DOWN+RIGHT 60'), ('until', 'layer=9', 1200), ('until', 'board=none', 1200), ('expect', 'route~kept from act 3'))
scene('route/kept', """A route found, the board then cleared and left by its exit: the route is kept for the run's next
      fork.""", ACT3, c.steps)
