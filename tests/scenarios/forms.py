"""MegaMan's forms (issue #774; docs/game/meta/forms.md): a run of each form
pays a guardian of another game in its own form. CROSS: BN5's ToadMan
gives SpoutCross by his kind, Aqua, never his Soul. SOUL BN5: BN6's
HeatMan gives NapalmSoul by his kind, Fire (Team ProtoMan's, moved since
#804). SOUL BN4: HeatMan gives BN4's one moved Soul,
ThunSoul, the one the run lacks: the powers line before his data and
after it is taken; CROSS's and the Soul forms' power then taken in the next
battle, after a quit and CONTINUE."""

# (onehit: a short fight; quiet: no random battle on the way to his arena)
BASE = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --dev onehit,quiet,'


def course(form, power):
    return [
        ('expect', f'runform={form}'),
        ('expect', 'powers=none'),
        ('until', 'battle=guardian', 4200),
        ('until', 'data=fresh|data=waiting', 4600),
        ('expect', f'powers={power}'),
        ('until', 'data=taken', 2000),
        ('expect', f'powers={power}'),
    ]


# (his power in the next battle: the game quit once his data is taken, a
# quit that keeps the run as it stands, then CONTINUEd without the
# autopilot, which takes neither a Cross nor UNITE, and a battle begun)
ARRIVE = [
    ('restart', '--scene title --dev onehit,quiet,god'),
    ('repeat', 'press START; wait 40', 'title=menu'),
    ('expect', 'cursor=continue'),
    ('repeat', 'press A; wait 40', 'scene=emu'),
    ('until', 'screen=map', 1200),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('do', 'foes 01@52'),
    ('until', 'phase=custom', 1200),
    ('do', 'wait 60'),
    ('expect', 'form=none'),
]
FIGHT = [
    ('repeat', 'press START; wait 10; press A; wait 60', 'phase=fight'),
    ('repeat', 'wait 10', 'custom>0', 60),
]
CROSS_USED = [*ARRIVE, ('do', 'press UP; wait 30; press A; wait 60'), *FIGHT, ('expect', 'form=SpoutCross')]


def unite(soul, chip, turns):
    """A battle's first Custom screen (xrom_souls.py's): a chip of the Soul's
    kind picked, UNITE lit and taken, OK, MegaMan united for `turns`"""
    return [
        ('until', f'drawn~{chip}', 900),
        ('repeat', 'press A; wait 10', 'handpicks=1'),
        ('expect', 'unite=lit'),
        ('expect', f'unitesoul={soul}'),
        ('do', 'press START; wait 10; press DOWN; wait 10; press A'),
        ('until', 'unite=chosen', 600),
        ('do', 'press START; wait 10; press A'),
        ('until', f'united={soul}', 900),
        ('expect', f'unitedturns={turns}'),
    ]


# (NapalmSoul, BN5's and BN6's Soul 4, Team ProtoMan's, by BN6's chip 107 D, FireHit1, Fire: tests/xdiff/soul_napalm.json's)
SOUL_USED = [*ARRIVE[:6], ('do', 'chip 107 D 30; foes 205@52'), *unite(4, 107, 3)]
# (ThunSoul, BN6's Soul 13 (BN4's 6), handed to UNITE with BN4's bits (soulunite_bn4_bits), by BN6's chip 30, Elec:
# tests/xdiff/soul_thun.json's)
SOUL4_USED = [*ARRIVE[:6], ('do', 'chip 30 B 30; foes 205@52'), *unite(13, 30, 3)]

MANY = {
    'cross-bn5-guardian': dict(START=BASE + 'form=1 --guardian 29', NEEDS=('bn5',), STEPS=course('CROSS', 'SpoutCross') + CROSS_USED),
    'soul-bn5-bn6-guardian': dict(START=BASE + 'form=2 --guardian 1', NEEDS=('bn5',), STEPS=course('SOUL BN5', 'NapalmSoul') + SOUL_USED),
    # (SoulT+1 in a Soul form: the BeastOut helper's turn more, four a unison, where CROSS takes BeastOut)
    'soul-bn5-helper-turn': dict(START='layer --seed 15 --net-biome 0 --setup short,0,0,16,0 --dev god,quiet,hp=2000,form=2,souls=0x20,folder=181',
                                 NEEDS=('bn5',), STEPS=[('expect', 'runform=SOUL BN5'), ('expect', 'runsoulturn=1'),
                                                        ('do', 'foes 205@52'), *unite(12, 181, 4)]),
    # (BN4's Souls unite in the Souls' engine BN5's ROM builds: both ROMs)
    'soul-bn4-bn6-guardian': dict(START=BASE + 'form=3 --guardian 1', NEEDS=('bn5', 'bn4'), STEPS=course('SOUL BN4', 'ThunSoul') + SOUL4_USED),
}
