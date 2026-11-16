"""SAVES (docs/engine/saves.md, issue #114): a run exported to a file,
imported on a device with no saves and continued on the same layer;
imported over another run, which UNDO IMPORT brings back byte for byte
(and UNDO again the import); KEEP THESE, B and a refused file changing
nothing; the same saves said to be the same; a .cwsave dropped on the
title; and the automatic copy's switch kept."""

# (a device of its own: HOME the game's folder, so its Downloads is its
# own, and a big screen's way, Steam's, where no dialog can be answered:
# the file is written to Downloads and read from there at once)
ENV = 'GAMESCOPE_WAYLAND_DISPLAY=none HOME={dir} '
OTHER = ' --data-dir {dir}/other'                 # a second device's data folder, with the same Downloads
RUN_A = ENV + '--scene emu --seed 3 --run-depth 2 --dev quiet'
RUN_B = ENV + '--scene emu --seed 5 --run-depth 4 --dev quiet' + OTHER
SAVES = ENV + '--scene saves'
TITLE = ENV + '--scene title'
WHAT = 'layer area hp zenny'
ARRIVE = [   # the arrival's words paged: MegaMan free on the layer, which a quit saves
    ('do', 'wait 150; mash A 300; wait 20'),
    ('repeat', 'mash A 60; wait 30', 'chat=closed'),
    ('until', 'screen=map', 600),
]


def file_line(name):
    """SAVES' home on its FILE page, the cursor on a line."""
    return [
        ('expect', 'saves=home'),
        ('repeat', 'press R; wait 10', 'savespage=file', 3),
        ('repeat', 'press DOWN; wait 6', f'savesfocus={name}', 4),
    ]


EXPORT = [   # run A saved on its layer, then exported from SAVES: its file in Downloads
    *ARRIVE,
    ('keep', WHAT),
    ('restart', SAVES),
    ('expect', 'herelayer=2'),
    *file_line('export'),
    ('do', 'press A; wait 10'),
    ('until', 'saves=result', 300),
    ('expect', 'savesnote~Exported to'),
    ('expect', 'savesnote~Downloads/cyberworld-endless.cwsave'),
    ('do', 'press A; wait 10'),                   # DONE
    ('expect', 'saves=home'),
]
IMPORT = [   # FILE's Import: the file compared with this device's saves before anything changes
    *file_line('import'),
    ('do', 'press A; wait 10'),
    ('until', 'saves=compare', 300),
    ('expect', 'theirstatus=ok'),
    ('expect', 'theirlayer=2'),
]
CONTINUE_A = [   # the result's CONTINUE: run A, as it was saved
    ('expect', 'savesfocus=continue'),
    ('do', 'press A; wait 10'),
    ('until', 'scene=emu', 600),
    ('until', 'screen=map', 1200),
    ('same', WHAT),
]
OVER_B = [   # run B saved on the other device, then A's file imported over it
    *EXPORT,
    ('restart', RUN_B),
    *ARRIVE,
    ('restart', SAVES + OTHER),
    ('expect', 'herelayer=4'),
    ('expect', 'undo=no'),
    ('keep', 'here'),
    *IMPORT,
    ('expect', 'savesfocus=keep'),                # the cursor on the side with more progress: this device's, deeper
]

MANY = {
    'export-import-continue': dict(START=RUN_A, STEPS=[
        *EXPORT,
        ('restart', SAVES + OTHER),               # a device with no saves
        ('expect', 'here=none'),
        *IMPORT,
        ('expect', 'savesfocus=take'),            # BRING BACK under the cursor
        ('do', 'press A; wait 10'),
        ('until', 'saves=result', 300),
        ('expect', 'herelayer=2'),
        ('expect', 'savesnote~Layer 2'),
        *CONTINUE_A,
        ('restart', TITLE + OTHER),               # and it is this device's run now
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('expect', 'cursor=continue'),
        ('expect', 'saved=2'),
    ]),
    'import-over-a-run-and-undo': dict(START=RUN_A, STEPS=[
        *OVER_B,
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'savesfocus=take'),
        ('do', 'press A; wait 10'),
        ('until', 'saves=result', 300),
        ('expect', 'herelayer=2'),
        ('expect', 'undo=yes'),
        ('expect', 'savesnote~UNDO IMPORT'),
        ('keep', 'herelayer'),
        ('repeat', 'press RIGHT; wait 6', 'savesfocus=undo', 3),
        ('do', 'press A; wait 10'),               # UNDO IMPORT: run B back, byte for byte (its files' hash)
        ('until', 'herelayer=4', 300),
        ('same', 'here'),
        ('repeat', 'press RIGHT; wait 6', 'savesfocus=undo', 3),
        ('do', 'press A; wait 10'),               # UNDO again: the import back
        ('until', 'herelayer=2', 300),
        ('restart', SAVES + OTHER),               # from home it is START's, which asks first, No under the cursor
        ('expect', 'undo=yes'),
        ('do', 'press START; wait 6'),
        ('expect', 'savesasks=no'),
        ('do', 'press A; wait 10'),
        ('expect', 'herelayer=2'),
        ('do', 'press START; wait 6; press LEFT; wait 6'),
        ('expect', 'savesasks=yes'),
        ('do', 'press A; wait 10'),
        ('until', 'herelayer=4', 300),
        ('same', 'here'),
        ('restart', TITLE + OTHER),               # CONTINUE is run B's again
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('expect', 'saved=4'),
    ]),
    # the comparison left by KEEP THESE, or by B: this device's saves as they were, nothing to undo
    'keep-these-changes-nothing': dict(START=RUN_A, STEPS=[
        *OVER_B,
        ('do', 'press A; wait 10'),               # KEEP THESE
        ('expect', 'saves=home'),
        ('same', 'here'),
        ('expect', 'undo=no'),
        *IMPORT,
        ('do', 'press RIGHT; wait 6; press B; wait 10'),      # B, even with USE THOSE under the cursor
        ('expect', 'saves=home'),
        ('same', 'here'),
        ('expect', 'undo=no'),
    ]),
    # the file of the very saves here: no comparison, the result says so
    'the-same-saves-are-said-so': dict(START=RUN_A, STEPS=[
        *EXPORT,
        ('keep', 'here'),
        *file_line('import'),
        ('do', 'press A; wait 10'),
        ('until', 'saves=result', 300),
        ('same', 'here'),
        ('expect', 'undo=no'),
    ]),
    # a file dropped on the title: compared as an imported one; a damaged one refused, the saves kept
    'a-file-dropped-on-the-title': dict(START=RUN_A, STEPS=[
        *EXPORT,
        ('restart', TITLE + OTHER),
        ('do', 'wait 30; drop {dir}/Downloads/cyberworld-endless.cwsave; wait 20'),
        ('expect', 'screen=saves'),
        ('expect', 'saves=compare'),
        ('expect', 'theirstatus=ok'),
        ('expect', 'theirlayer=2'),
        ('expect', 'here=none'),
        ('repeat', 'press RIGHT; wait 6', 'savesfocus=take', 2),
        ('do', 'press A; wait 10'),
        ('until', 'saves=result', 300),
        *CONTINUE_A,
    ]),
    'a-damaged-file-is-refused': dict(START=RUN_A, STEPS=[
        *ARRIVE,
        ('restart', TITLE),
        ('do', 'wait 30; drop {damaged}; wait 20'),
        ('expect', 'saves=compare'),
        ('expect', 'theirstatus=refused'),
        ('expect', 'savesnote~Your saves are kept.'),
        ('expect', 'savesfocus=done'),            # DONE alone
        ('expect', 'herelayer=2'),
        ('keep', 'here'),
        ('do', 'press RIGHT; wait 6; press A; wait 10'),
        ('expect', 'saves=home'),
        ('same', 'here'),
        ('expect', 'undo=no'),
    ]),
    # L on the title opens SAVES, B goes back to the title's menu; a device with no saves has no Export
    'l-on-the-title-and-b-back': dict(START=TITLE, STEPS=[
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('do', 'press L; wait 20'),
        ('expect', 'screen=saves'),
        ('expect', 'here=none'),
        ('repeat', 'press R; wait 10', 'savespage=file', 3),
        ('expect', 'savesfocus=import'),          # the page's first line: no saves to export
        ('do', 'press UP; wait 6; press UP; wait 6; press UP; wait 6'),
        ('expect', 'savesfocus!=export'),
        ('do', 'press B; wait 20'),
        ('expect', 'screen=title'),
        ('expect', 'title=menu'),
    ]),
    # the automatic copy's switch: Left and Right turn it, kept by the next start (saves.ini)
    'auto-copy-kept': dict(START=SAVES, STEPS=[
        *file_line('auto'),
        ('expect', 'autocopy=off'),
        ('do', 'press RIGHT; wait 6'),
        ('expect', 'autocopy=on'),
        ('restart', SAVES),
        ('expect', 'autocopy=on'),
        *file_line('auto'),
        ('do', 'press A; wait 6'),
        ('expect', 'autocopy=off'),
    ]),
}
