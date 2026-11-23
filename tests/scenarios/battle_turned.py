"""A chip used by MegaMan turned round on a Surrounded field (issue #838,
reported on Discord by Agergo100, 23 November 2026: "GolmHit on
Liberation Mission battle is always attack on the right, even if you
facing to the left"). A liberation's battle from level 3 sets him between
the enemy's columns 1-2 and 5-6, and L or R turns him round; BN6 never
does, and GolmHit's fist searched its row from the field's edge behind
him (BN6_ROW_SEARCH), which there is the enemy's: it came down on column 6
turned left, on column 1 facing right. Now it strikes the first of the
enemy's columns ahead of him either way, and in BN6's own battles as it
did (docs/rom/chips-code.md, A chip used turned round;
src/emu/turned_aim.h)."""

# (a run's first layer, MegaMan free on it; no battle but the step's, and HP enough to stand the foes while the chip is played)
START = 'layer --seed 15 --net-biome 0 --dev god,quiet,hp=2000'

# (BN6's RareMttr2, 180 HP: GolmHit1's 140 leaves it standing, so the state says which was struck; on a Surrounded field
# one on every column of the enemy's, so the state names the column the fist came down on)
FOE = '06'
GOLMHIT1 = (120, 'I')


def golmhit(where, turn, struck, about):
    chip, code = GOLMHIT1
    steps = [
        ('do', f'chip {chip} {code} 29; foes {where}'),
        ('until', f'drawn~{chip}{code}', 900),        # the Custom screen: the hand dealt
        ('repeat', 'press A; wait 10', 'handpicks=1'),
        ('do', 'press START; wait 10; press A'),      # to OK, and OK
        ('until', 'hand=1', 600),
        ('expect', f'nextchip={chip}'),
        ('until', 'phase=fight', 600),
        ('do', 'status paralysis 900'),               # each foe held on its panel, in MegaMan's row, before its shockwave
        ('until', 'foestatus=paralysed', 120),
    ]
    if turn:
        steps.append(('repeat', 'hold L 4; wait 10', 'megamanfaces=left', 6))   # L turns him round (no Custom screen asked)
    steps += [
        ('expect', 'megamanfaces=' + ('left' if turn else 'right')),
        ('expect', 'foeshurt=none'),
        ('repeat', 'press A; wait 30', 'hand=0'),     # the fist called
        ('repeat', 'wait 20', 'foeshurt!=none', 20),  # come down
        ('do', 'wait 60'),
        ('expect', f'foeshurt={struck}'),             # on the foe ahead of him, the one behind untouched
    ]
    return dict(START=START, GUARDS=f'GolmHit1 {about}', STEPS=steps)


SURROUNDED = f'{FOE}@12 {FOE}@22 {FOE}@52 {FOE}@62 sides@33 mm@32'
MANY = {
    # turned left: the fist on the foe at column 2 (before: column 6, this report)
    'golmhit/turned': golmhit(SURROUNDED, True, '2', 'turned left on a Surrounded field: the foe on his left struck'),
    # facing right between the two sides: the foe at 5 (before: column 1, behind him, empty)
    'golmhit/surrounded': golmhit(SURROUNDED, False, '5', 'facing right on a Surrounded field: the foe ahead struck, not his back'),
    # BN6's own field: the first of the enemy's columns, 4, as it always came down
    'golmhit/plain': golmhit(f'{FOE}@42 {FOE}@62', False, '4', "on BN6's own field: column 4, as BN6 strikes it"),
}
