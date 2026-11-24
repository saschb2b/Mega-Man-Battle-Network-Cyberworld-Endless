"""MegaMan's palettes earned (issue #854; docs/game/looks.md, Earning): a
milestone the profile holds grants its palette once, MegaMan's word and
BN6's "MegaMan got:" box said once; the run's rare find, a ColrData in a
Mystery Data, grants its palette, and a run whose profile owns it holds
none (the shop's: looks_shop.py)."""

FREE = ('until', 'held=none', 600)
PAGED = ('repeat', 'press A; wait 30', 'held=none', 40)
ARRIVE = [('do', 'wait 150; mash A 300; wait 20'), ('repeat', 'mash A 60; wait 30', 'chat=closed'), ('until', 'screen=map', 600)]
# (seed 7's run: its Mystery find on layer 4, a blue data; no Glitch layer)
MYSTERY = '--scene emu --seed 7 --run-depth 4 --setup short,0,0,0,0 --dev quiet'
# (a run begun on a Glitch layer: its find the Glitch colors)
GLITCH = '--scene emu --seed 3 --run-depth 4 --dev glitch,quiet,side=4'
PROTOMAN, MYST, GLT = 1 << 3, 1 << 10, 1 << 11


def found(start, palette):
    return dict(
        GUARDS=f"The run's rare find: A on its ColrData grants palette {palette.bit_length() - 1}, BN6's box and MegaMan's word, once.",
        START=start,
        STEPS=ARRIVE + [
            ('expect', f'lookfind={palette.bit_length() - 1}'),
            ('do', 'by colrdata; wait 10; press A; wait 40'),
            ('repeat', 'press A; wait 40', f'looksowned={palette | 1}', 8),
            ('repeat', 'press A; wait 40', 'talk=director', 8),   # BN6's box paged, MegaMan names it
            PAGED, FREE,
            ('expect', f'lookstold={palette}'),
            ('do', 'wait 120'),
            ('expect', 'talk=none'),
        ])


MANY = {
    'mystery-found': found(MYSTERY, MYST),
    'glitch-found': found(GLITCH, GLT),
    'mystery-owned': dict(
        GUARDS="A profile that owns the Mystery colors and runs without a Glitch layer: no find this run.",
        START=MYSTERY.replace('--dev quiet', f'--dev quiet,looks={MYST}'),
        STEPS=ARRIVE + [('expect', 'lookfind=-1'), ('expect', f'looksowned={MYST | 1}')],
    ),
    'milestone-once': dict(
        GUARDS="Chaud's second clearance held (three duels won): the ProtoMan colors granted at the start, said in the room's first moments (the route pages it), and not again.",
        START='room --dev duels=3',
        STEPS=[
            FREE,
            ('expect', f'looksowned={PROTOMAN | 1}'),
            ('expect', f'lookstold={PROTOMAN}'),
            ('do', 'wait 200'),
            ('expect', 'talk=none'),
            ('expect', f'looksowned={PROTOMAN | 1}'),
        ]),
}
