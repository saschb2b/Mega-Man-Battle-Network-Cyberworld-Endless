"""Issue #865's archetypes on the shelf (docs/game/meta/decisions.md, Issue
865's archetypes): Ember and Tide locked on a new profile, looked at and
never taken; Tide opened by a Fire guardian deleted (BlastMan, by the
autopilot), Ember not by the same win outside HeatCross; and a run
started with either draws from its own 30 (Ember's first chip GrasSeed F,
Tide's TrnArrw1 A, dealt in a battle's hand). Ember's own milestone, a
guardian deleted in HeatCross, is test_folders' (the guardian's record of
the form his last battle was won in)."""

SETUP = '--scene setup'
FOLDER_ROW = ('repeat', 'press DOWN; wait 6', 'row=folder', 8)
# (a run's first layer, MegaMan free on it, a Mettaur's battle at once: the hand dealt)
def first_hand(folder):
    return f'layer --seed 15 --net-biome 0 --setup short,{folder},0,0 --dev god,quiet'

MANY = {
    'locked-on-a-new-profile': dict(
        GUARDS='Ember and Tide locked, their hints shown, A on them refused: the run keeps Standard.',
        START=SETUP, STEPS=[
            FOLDER_ROW,
            ('expect', 'shelf~Ember:locked,Tide:locked'),
            ('do', 'press LEFT; wait 6'),             # round the shelf: the last, Tide
            ('expect', 'folderlook=Tide'),
            ('expect', 'folderstate=locked'),
            ('do', 'press A; wait 6'),
            ('expect', 'startfolder=Standard'),
            ('do', 'press LEFT; wait 6'),
            ('expect', 'folderlook=Ember'),
            ('expect', 'folderstate=locked'),
            ('do', 'press LEFT; wait 6'),             # past any id no folder holds, to the folder before
            ('expect', 'folderlook!=Ember'),
            ('expect', 'folderstate=locked'),
        ]),
    'tide-opens-with-a-fire-guardian': dict(
        GUARDS='BlastMan deleted as the act\'s guardian: Tide open at the next setup and taken; Ember still locked.',
        START='CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 3 --guardian 12 --dev onehit,god', STEPS=[
            ('until', 'guardian=BlastMan beaten', 12000),
            ('restart', SETUP),
            ('expect', 'shelf~Ember:locked,Tide:open'),
            FOLDER_ROW,
            ('do', 'press LEFT; wait 6'),
            ('expect', 'folderlook=Tide'),
            ('expect', 'startfolder=Tide'),
        ]),
    'an-ember-run-draws-from-ember': dict(
        GUARDS="A run started with Ember: its folder is Ember's, and a battle's hand comes from it.",
        START=first_hand(9), STEPS=[
            ('expect', 'startfolder=Ember'),
            ('expect', 'folder~first GrasSeed F'),
            ('do', 'foes 01@52'),
            ('until', 'drawn~68F|drawn~20F|drawn~107|drawn~110H', 900),
        ]),
    'a-tide-run-draws-from-tide': dict(
        GUARDS="A run started with Tide: its folder is Tide's, and a battle's hand comes from it.",
        START=first_hand(10), STEPS=[
            ('expect', 'startfolder=Tide'),
            ('expect', 'folder~first TrnArrw1 A'),
            ('do', 'foes 01@52'),
            ('until', 'drawn~24A|drawn~27T|drawn~69A', 900),
        ]),
}
