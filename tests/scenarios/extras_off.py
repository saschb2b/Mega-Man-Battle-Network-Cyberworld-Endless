"""The extras a run's seed places, with no dev switch, and the JACK-IN
SETUP's EXTRAS page (docs/game/meta/extras.md): each met where the seed
puts it with its switch on, gone with it off and nothing else moved; and
an act walked with all of them off, never meeting one."""

ARRIVE = [('do', 'wait 150; mash A 300; wait 20'), ('repeat', 'mash A 60; wait 30', 'chat=closed')]


def layer(seed, depth, off, *expects):
    """A layer of a run by its seed alone, the extras of `off` switched off (--setup's sixth)."""
    return dict(START=f'--scene emu --seed {seed} --run-depth {depth} --dev quiet --setup endless,0,0,0,0,{off}',
                STEPS=[*ARRIVE, ('expect', f'layer={depth}'), *[('expect', e) for e in expects]])


# (the switches' bits: 2 Tournament, 4 Glitch, 8 Virus Battler, 16 DarkChips)
MANY = {
    'glitch-warp-by-the-seed': dict(layer(3, 4, 0, 'glitchwarp=here'),
        GUARDS="A run whose seed holds a Glitch layer (one in three): its warp on act 2's first layer."),
    'glitch-warp-off': dict(layer(3, 4, 4, 'glitchwarp=none', 'sidelayer=none'),
        GUARDS='... and none there with Glitch off.'),
    'battler-by-the-seed': dict(layer(4, 4, 0, 'battler=1', 'stake=500'),
        GUARDS="A Virus Battler where the run's seed puts one: BN6's first on act 2's first layer."),
    'battler-off': dict(layer(4, 4, 8, 'battler=none'),
        GUARDS='... and none there with Virus Battler off.'),
    'flame-by-the-seed': dict(layer(4, 5, 0, 'flame=burning', 'flamenet=bn6', 'darkwarp=here'),
        GUARDS="A flame of darkness on an act's middle layer, here beside a dark warp."),
    'flame-off': dict(layer(4, 5, 16, 'flame=none', 'darkwarp=here'),
        GUARDS='... and no flame with DarkChips off, the dark warp (no extra) still there.'),
    'two-on-a-layer': dict(layer(12, 4, 0, 'glitchwarp=here', 'battler=1'),
        GUARDS="A layer holding the Glitch warp and a Virus Battler by its seed."),
    'two-off': dict(layer(12, 4, 12, 'glitchwarp=none', 'battler=none'),
        GUARDS='... and neither with both off.'),
    'never-met': dict(
        GUARDS="Glitch, the Virus Battler and DarkChips off, each forced by its dev switch: an act walked by the autopilot "
               'meets none on any of its layers, and goes home.',
        START='CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 4 --setup endless,0,0,0,0,28 --dev quiet,god,onehit,glitch,battler=1',
        STEPS=[
            ('expect', 'layer=4'),
            ('expect', 'glitchwarp=none'),
            ('expect', 'battler=none'),
            ('until', 'layer=5', 20000),
            ('expect', 'flame=none'),
            ('expect', 'battler=none'),
            ('expect', 'sidelayer=none'),
            ('until', 'layer=6', 20000),
            ('expect', 'glitchwarp=none'),
            ('expect', 'battler=none'),
            ('expect', 'sidelayer=none'),
            ('until', 'where=town', 20000),
        ]),
}
