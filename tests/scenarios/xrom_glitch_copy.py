"""0.22.0: the Glitch copy of a Falzar Navi was a flicker of streaks. Gregar
holds Falzar's Navis as battle sprites alone, and the copy at a Glitch
layer's heart stood in animation 4 of one, his warp-out, over and over; it
stands in his body's own now, as he stands in battle (0). A copy in one of
BN6's own overworld sprites faces down-left (5), as it did."""

# (a run dropped onto act 2's Glitch layer; the copy's first body by the seed: 2 SpoutMan, Falzar's, 3 BlastMan)


def copy(seed, navi, stands):
    return dict(
        START=f'--scene emu --seed {seed} --run-depth 4 --dev side=4,quiet',
        GUARDS=f"The Glitch layer's copy in {navi}'s body stands in animation {stands} of his sprite",
        STEPS=[
            ('until', 'copystands>=0', 600),          # the layer's people in place
            ('expect', f'copyform={navi}'),
            ('expect', f'copystands={stands}'),
            ('do', 'wait 300'),
            ('expect', f'copystands={stands}'),       # ... and stays in it
        ])


MANY = {'falzar-spoutman': copy(2, 'SpoutMan', 0), 'gregar-blastman': copy(3, 'BlastMan', 5)}
