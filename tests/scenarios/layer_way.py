"""The way on from a layer: a run's first words on it and MegaMan free
after them, the exit pad shut until the layer's first battle and then down
to the next layer, shut again on a guardian's, and the fork a dark warp
makes: MegaMan's read of where the pad leads, once, the exit's way, and
the warp's way through the Undernet to the same next layer."""

RUN = 'CYBERWORLD_STATE_POS=1 --scene emu --dev quiet'
MIDDLE = f'{RUN} --seed 3 --run-depth 2'      # an act's middle layer
LAST = f'{RUN} --seed 3 --run-depth 3'        # ... its last, the guardian's
FORK = f'{RUN} --seed 4 --run-depth 5'        # a layer with a dark warp beside its exit

PAGED = ('repeat', 'press A; wait 30', 'held=none', 30)         # a chat's boxes to its end, MegaMan free again
# (a layer arrived on: its words, where it has any, an A a box, and MegaMan free on its map)
ARRIVE = [('do', 'wait 240'), PAGED, ('do', 'wait 30'), PAGED, ('expect', 'screen=map')]

MANY = {
    # a run's first words on a layer begin by themselves and end with MegaMan free on its map
    'arrival-words': dict(START=MIDDLE, STEPS=[
        ('expect', 'layer=2'),
        ('until', 'talk=director', 600),
        PAGED,
        ('expect', 'talk=none'),
        ('expect', 'screen=map'),
    ]),
    # the battle floor: a layer's exit pad is shut until its first battle is fought (the autopilot walks and fights)
    'exit-pad-opens-after-a-battle': dict(START='CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 2', STEPS=[
        ('until', 'doing=map', 600),
        ('expect', 'exitpad=shut'),
        ('until', 'screen=battle', 8000),
        ('expect', 'exitpad=shut'),
        ('until', 'exitpad=open', 8000),
        ('expect', 'layer=2'),
    ]),
    # the exit pad, stepped on: the next layer, its own arrival's words, and on a guardian's layer the pad shut
    'exit-pad-leads-down': dict(START=MIDDLE, STEPS=ARRIVE + [
        ('keep', 'area'),
        ('do', 'by exit; wait 2'),
        ('until', 'layer=3', 600),
        *ARRIVE,
        ('same', 'area'),
        ('expect', 'guardian~waiting'),
        ('expect', 'exitpad=shut'),
    ]),
    # ... shut while the guardian stands: stepped on, it leads nowhere
    'exit-pad-shut-before-the-guardian': dict(START=LAST, STEPS=ARRIVE + [
        ('expect', 'exitpad=shut'),
        ('do', 'by exit; wait 240'),
        ('repeat', 'press A; wait 30', 'held=none', 30),
        ('expect', 'layer=3'),
        ('expect', 'doing=map'),
    ]),
    # a fork: near the exit pad MegaMan reads where it leads, once a layer
    'fork-pad-read-once': dict(START=FORK, STEPS=ARRIVE + [
        ('do', 'by exitside; wait 2'),
        ('until', 'talk=director', 300),
        PAGED,
        ('expect', 'layer=5'),
        ('do', 'by exitside; wait 120'),
        ('expect', 'talk=none'),
        ('expect', 'held=none'),
    ]),
    # ... the exit's way: the next layer
    'fork-exit-way': dict(START=FORK, STEPS=ARRIVE + [
        ('keep', 'area'),
        ('do', 'by exit; wait 2'),
        ('until', 'layer=6', 600),
        *ARRIVE,
        ('same', 'area'),
    ]),
    # ... the dark warp asks, on No; No keeps the layer
    'fork-dark-way-declined': dict(START=FORK, STEPS=ARRIVE + [
        ('do', 'by undernet; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=2/2', 30),
        ('do', 'press A; wait 30'),
        PAGED,
        ('expect', 'layer=5'),
        ('expect', 'area!=the Undernet'),
    ]),
    # ... Yes: an Undernet layer, and its exit leads to the same next layer the pad did
    'fork-dark-way-rejoins': dict(START=FORK, STEPS=ARRIVE + [
        ('keep', 'area'),
        ('do', 'by undernet; wait 10'),
        ('repeat', 'press A; wait 30', 'choice=2/2', 30),
        ('repeat', 'press LEFT; wait 10', 'choice=1/2'),
        ('do', 'press A; wait 30'),
        ('until', 'area=the Undernet', 900),
        *ARRIVE,
        ('do', 'by return; wait 2'),                            # a side layer's pad out
        ('until', 'layer=6', 600),
        *ARRIVE,
        ('same', 'area'),
    ]),
}
