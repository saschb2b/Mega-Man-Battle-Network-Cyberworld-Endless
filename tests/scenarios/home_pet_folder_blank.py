"""The Chip Folder's screens drawn after the PET's menu of ours (#793): BN6
fills their BG0 with tile 0x2FF, its blank, and the menu's Forms label
loaded ten tiles of its word past Options' up to that tile, so ChipFolder
and Folder Edit showed one orange striped page with only the folder's
sprites over it. Forms' word stays out of the video memory now, and the
blank tile is zero on both screens (docs/rom/pet-and-second-screen.md, the
PET menu's labels)."""
import cw_routes

START = '--scene town --seed 2'

STEPS = [
    ('do', cw_routes.ROOM),
    ('repeat', 'press START; wait 40', 'screen=pet', 6),
    ('expect', 'petmenu=at ChipFolder'),
    ('do', 'press A; wait 60'),
    ('until', 'screen=folders', 300),
    ('do', 'wait 30'),
    ('expect', 'blanktile=zero'),              # the folders' list drawn, not covered by the blank
    ('repeat', 'press A; wait 40', 'screen=folder-edit', 6),
    ('do', 'wait 60'),
    ('expect', 'blanktile=zero'),              # Folder Edit's cards drawn too
]
