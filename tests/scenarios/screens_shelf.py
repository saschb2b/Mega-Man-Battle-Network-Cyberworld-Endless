"""The JACK-IN SETUP's shelf of starting folders (issue #864,
docs/game/meta/shelf.md): the FOLDER row turns through all six; a locked
one is looked at, never taken (A refused, START and the next row keep the
open one the run takes); a bystander's whisper of how one opens makes it
heard at the next setup; three finished runs from the shelf's first showing,
the BBS's floor makes the closest one heard; and the summary's closest
unlock gives the hint while unheard, the plain sentence once heard."""

SETUP = '--scene setup'
FOLDER_ROW = ('repeat', 'press DOWN; wait 6', 'row=folder', 8)
JACK_IN = [   # START: the run begun with the choice as it stood
    ('repeat', 'press START; wait 60', 'scene=emu'),
    ('until', 'where=town', 600),
]
# (a layer walked by the autopilot; fragile: the first hit deletes MegaMan, and the run is over)
LOST = 'CYBERWORLD_AUTOPILOT=1 --scene emu --seed 3 --run-depth 4 --dev fragile'
OVER = [('until', 'title=summary', 12000), ('repeat', 'press A; wait 30', 'title=start', 12)]
# (a short net won on its last layer, made up: the endless net opens, so the summary's
# closest unlock is the first locked folder, Blade)
WON = '--scene summary --setup short --run-depth 10'

MANY = {
    'locked-is-looked-at-not-taken': dict(
        GUARDS='A locked folder shown with its state, A on it refused, the next row and START keeping the open one.',
        START=SETUP, STEPS=[
            FOLDER_ROW,
            ('expect', 'folders=1'),
            ('expect', 'folderlook=Standard'),
            ('expect', 'folderstate=open'),
            ('expect', 'shelf=Blade:locked,Storm:locked,Z-Canon:locked,Busting:locked,Lean:locked'),
            ('do', 'press RIGHT; wait 6'),
            ('expect', 'folderlook=Blade'),
            ('expect', 'folderstate=locked'),
            ('expect', 'startfolder=Standard'),
            ('do', 'press A; wait 6'),                # refused: the row stays, the run's folder too
            ('expect', 'folderlook=Blade'),
            ('expect', 'startfolder=Standard'),
            ('do', 'press LEFT; wait 6; press LEFT; wait 6'),   # round the shelf: Standard, then Lean
            ('expect', 'folderlook=Lean'),
            ('do', 'press DOWN; wait 6'),             # off the row: it shows the run's folder again
            ('expect', 'folderlook=Standard'),
            ('do', 'press UP; wait 6; press RIGHT; wait 6'),
            ('expect', 'folderlook=Blade'),
            *JACK_IN,                                 # START on a locked one: the open one goes
            ('expect', 'startfolder=Standard'),
        ]),
    'heard-by-a-whisper': dict(
        GUARDS="The second bystander's whisper of how Storm opens (seed 2's first layer), said: Storm heard at the next setup.",
        START=SETUP, STEPS=[
            ('expect', 'shelf~Storm:locked'),          # (the shelf seen: the floor's count and the whispers begin)
            ('restart', '--scene emu --seed 2 --run-depth 1 --dev quiet --talk whisper:60'),
            ('do', 'wait 150; mash A 300; wait 20'),
            ('repeat', 'mash A 60; wait 30', 'chat=closed'),
            ('expect', 'whisperfolder=Storm'),
            ('do', 'wait 30'),
            ('restart', SETUP),
            ('expect', 'shelf=Blade:locked,Storm:heard,Z-Canon:locked,Busting:locked,Lean:locked'),
            FOLDER_ROW,
            ('do', 'press RIGHT; wait 6; press RIGHT; wait 6'),
            ('expect', 'folderlook=Storm'),
            ('expect', 'folderstate=heard'),
        ]),
    'no-whisper-before-the-shelf': dict(
        GUARDS="A profile that has never seen the setup: the same layer's whisper tells of no folder.",
        START='--scene emu --seed 2 --run-depth 1 --dev quiet', STEPS=[
            ('do', 'wait 150; mash A 300; wait 20'),
            ('repeat', 'mash A 60; wait 30', 'chat=closed'),
            ('expect', 'whisperfolder=none'),
        ]),
    'heard-by-the-floor': dict(
        GUARDS='Three runs finished after the shelf was first shown: the BBS makes Blade, the closest, heard; not after two.',
        START=SETUP, STEPS=[
            ('restart', LOST), *OVER,
            ('restart', LOST), *OVER,
            ('restart', SETUP),
            ('expect', 'shelf~Blade:locked'),
            ('restart', LOST), *OVER,
            ('restart', SETUP),
            ('expect', 'runs=3'),
            ('expect', 'shelf=Blade:heard,Storm:locked,Z-Canon:locked,Busting:locked,Lean:locked'),
        ]),
    'summary-gives-the-hint': dict(
        GUARDS="The summary's closest unlock while Blade is locked and unheard: the hint, never the deed.",
        START=WON, STEPS=[
            ('expect', 'title=summary'),
            ('expect', 'nextgoal=The Blade folder opens when something big falls.'),
        ]),
    'summary-gives-the-deed-once-heard': dict(
        GUARDS='... and once Blade is heard, the plain sentence.',
        START=f'{WON} --dev heard=2', STEPS=[
            ('expect', 'title=summary'),
            ('expect', 'nextgoal=Delete any guardian to open the Blade folder.'),
        ]),
}
