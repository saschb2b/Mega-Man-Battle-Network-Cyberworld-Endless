"""A random battle from its start to the map again, won: BN6's RESULT, the
reward it names paid, its viruses in the run's tally, the layer's battles
counted. One variant's reward is zenny, the other's a chip."""


def won(seed, foes, reward, paid):
    return dict(
        START=f'CYBERWORLD_AUTOPILOT=1 CYBERWORLD_STATE_POS=1 --scene emu --seed {seed} --run-depth 2 --dev onehit',
        STEPS=[
            ('expect', 'viruses=0'),
            ('until', 'chat=open', 1200),          # the arrival's words, paged by the autopilot,
            ('until', 'held=none', 1200),          # and MegaMan free
            ('do', 'battle'),
            ('until', 'screen=battle', 1200),
            ('expect', 'battle=random'),
            ('expect', 'rules=plain'),
            ('until', 'phase=results', 3000),
            ('expect', 'result=won'),
            ('expect', f'foes={foes}'),
            ('expect', f'down={foes}'),
            ('expect', f'reward~{reward}'),
            ('until', 'screen=map', 1200),
            ('expect', 'lastbattle=won'),
            ('expect', f'viruses={foes}'),         # counted as he is back on the map
            ('expect', 'battles=1'),
            ('expect', paid),
        ],
    )


MANY = {
    'zenny': won(2, 2, 'zenny 200', 'zenny=200'),
    'chip': won(1, 3, 'chip', 'zenny=0'),
}
