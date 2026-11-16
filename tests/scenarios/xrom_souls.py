"""BN5's Souls in a run's battles: with a Soul held, the chip of its kind
picked lights UNITE under OK on BN6's Custom screen, A on it and OK unite
MegaMan with that Soul for three turns, a turn less each Custom screen,
gone as the turn after its last begins, and not offered again that
battle; a Soul not held lights nothing."""

# (a Soul: BN5's number, its bit in the run's Souls, a chip of its kind: docs/devtools/proxy-form.md)
SOULS = {'colonel': (7, 0x01, 143), 'shadow': (8, 0x02, 177), 'number': (9, 0x04, 175), 'tomahawk': (10, 0x08, 116),
         'knight': (11, 0x10, 140), 'toad': (12, 0x20, 181)}


def start(held, chip):
    # (a run's first layer, MegaMan free; a folder of that chip alone; a foe that stays on its panel, and HP to stand it)
    return f'layer --seed 15 --net-biome 0 --dev god,quiet,hp=2000,souls={held:#x},folder={chip}'


def screen(chip):
    """The fight's next Custom screen opened (L, the gauge full)."""
    return [('until', 'custom>=100', 1500), ('do', 'press L'), ('until', f'drawn~{chip}', 600)]


OK = [('do', 'press START; wait 10; press A'), ('until', 'unite=none', 600)]   # OK, nothing more picked; the screen away


def unite(soul, held, chip):
    """A battle's first Custom screen: the chip picked, UNITE lit, taken, OK."""
    return [
        ('expect', f'soulsheld={held:02x}'),
        ('do', 'foes 205@52'),
        ('until', f'drawn~{chip}', 900),
        ('expect', 'unite=grey'),                         # UNITE stands, nothing picked yet
        ('repeat', 'press A; wait 10', 'picked=1'),
        ('expect', 'unite=lit'),
        ('expect', f'unitesoul={soul}'),
        ('do', 'press START; wait 10; press DOWN; wait 10; press A'),   # OK, UNITE under it, A: the chip given up
        ('until', 'unite=chosen', 600),
        ('do', 'press START; wait 10; press A'),          # OK
        ('until', f'united={soul}', 900),
        ('expect', 'unitedturns=3'),
    ]


MANY = {
    f'unite/{name}': dict(
        START=start(0x3F, chip), NEEDS=('bn5',),
        GUARDS=f"UNITE on BN6's Custom screen: {name.capitalize()}Soul lit by a chip of its kind, taken, MegaMan united for three turns",
        STEPS=unite(soul, 0x3F, chip))
    for name, (soul, bit, chip) in SOULS.items()
}
MANY['life'] = dict(
    START=start(0x10, 140), NEEDS=('bn5',),
    GUARDS="A Soul's life: KnightSoul's three turns counted down a Custom screen each, its end, and no second time that battle",
    STEPS=unite(11, 0x10, 140) + screen(140) + [
        ('expect', 'united=11'), ('expect', 'unitedturns=2'),
        ('expect', 'unite=grey'),                         # united: none to take
    ] + OK + screen(140) + [
        ('expect', 'united=11'), ('expect', 'unitedturns=1'),
    ] + OK + screen(140) + [
        ('expect', 'united=11'), ('expect', 'unitedturns=0'),   # its last screen
    ] + OK + [
        ('until', 'united=0', 600),                       # the turn after begins: MegaMan himself again
    ] + screen(140) + [
        ('expect', 'united=0'),
        ('repeat', 'press A; wait 10', 'picked=1'),       # its chip again
        ('expect', 'unite=grey'),                         # ... lights nothing: once a battle
    ])
MANY['not-held'] = dict(
    START=start(0x01, 140), NEEDS=('bn5',),
    GUARDS="A Soul the run does not hold: its chip picked lights nothing (ColonelSoul held, KnightSoul's chip)",
    STEPS=[
        ('expect', 'soulsheld=01'),
        ('do', 'foes 205@52'),
        ('until', 'drawn~140', 900),
        ('repeat', 'press A; wait 10', 'picked=1'),
        ('expect', 'unite=grey'),
        ('expect', 'unitesoul=0'),
    ])
MANY['none-held'] = dict(
    START=start(0, 140), NEEDS=('bn5',),
    GUARDS="A run with no Soul: no UNITE on the Custom screen, BN6's own as it is",
    STEPS=[
        ('expect', 'soulsheld=00'),
        ('do', 'foes 205@52'),
        ('until', 'drawn~140', 900),
        ('repeat', 'press A; wait 10', 'picked=1'),
        ('expect', 'unite=none'),
        ('do', 'press START; wait 10; press A'),
        ('until', 'hand=1', 600),
        ('expect', 'united=0'),
    ])
