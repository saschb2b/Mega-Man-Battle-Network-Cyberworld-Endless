"""Issue #731: Falzar's five Gigas (BigHook, BassAnly, CrossDiv, HubBatc,
BgDthThd), which a run hands out, drew another chip's picture on their
cards: Gregar's own records give each the picture and icon of Gregar's
Giga of its library number (HubBatc ColForce's). Now each draws BN5's own
where BN5's ROM holds the chip, and BN6's blank where no ROM read does,
never another Giga's (docs/rom/chips.md, The other version's Gigas' art)."""

# (Lan's room at a new run: the chip step puts each in the folder's first entry, whose card the state names)
START = 'room'

GIGAS = (('BigHook', '302 H'), ('BassAnly', '306 F'), ('CrossDiv', '308 D'), ('HubBatc', '309 J'), ('BgDthThd', '310 V'))
# (the three BN5's ROM draws: its records 308, 306 and 309, the registry's names for them)
BN5_DRAWS = ('BigHook', 'BassAnly', 'CrossDiv')


def course(bn5):
    steps = []
    for name, chip in GIGAS:
        steps += [
            ('do', f'chip {chip} 1'),
            ('expect', f'folder~first {name}'),
            ('expect', f"foldercard={'own' if bn5 and name in BN5_DRAWS else 'blank'}"),
        ]
    return steps


MANY = {
    'bn6': dict(START=START, NEEDS=(), GUARDS="Falzar's Gigas' cards on BN6 alone: BN6's blank, not Gregar's Gigas' pictures",
                STEPS=course(False)),
    'bn5': dict(START=START, NEEDS=('bn5',), GUARDS="... with BN5's ROM: BN5's own pictures for BigHook, BassAnly and CrossDiv",
                STEPS=course(True)),
}
