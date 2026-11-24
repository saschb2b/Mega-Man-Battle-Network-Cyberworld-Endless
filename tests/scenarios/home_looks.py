"""Issue #854's colours screen: the PET menu's Colours opens it. A colour
picked on the armour is the look worn (the state's `look` line), the grid's
cursor recolours him only until A picks; B leaves the screen and the look
stays worn. A locked palette's colour cannot be picked: A buzzes and the
look is as it was. A on the Look row copies the look to another
(docs/game/looks.md)."""
import cw_routes

START = '--scene town --seed 2'
OPEN = [
    ('do', cw_routes.PET),
    ('expect', 'screen=pet'),
    ('do', 'press DOWN; wait 6; ' * 5 + 'wait 10'),
    ('expect', 'petmenu=at Colours'),
    ('do', 'press A; wait 40'),
    ('expect', 'screen=looks'),
]

MANY = {
    'pick': dict(
        GUARDS="The colours screen: a Classic colour picked on the armour is the look worn, on the screen, back in the PET's menu and on the map.",
        START=START,
        STEPS=[
            *OPEN,
            ('expect', 'look=-,-,- slot 0'),
            ('expect', 'looks~at look list'),
            ('do', 'press DOWN; wait 8'),
            ('expect', 'looks~at armour list page Classic'),
            ('do', 'press A; wait 8'),
            ('expect', 'looks~at armour grid page Classic cell plain'),
            ('do', 'press UP; wait 8; press RIGHT; wait 8'),
            ('expect', 'looks~cell 13'),
            ('expect', 'look=-,-,- slot 0'),                     # the cursor alone: nothing picked yet
            ('do', 'press A; wait 8'),
            ('expect', 'looks~at armour list'),
            ('expect', 'look=0.13,-,- slot 0'),
            ('repeat', 'press B; wait 40', 'screen=pet', 4),
            ('expect', 'look=0.13,-,- slot 0'),
            ('repeat', 'press B; wait 40', 'screen=town', 6),
            ('expect', 'look=0.13,-,- slot 0'),
        ]),
    'locked': dict(
        GUARDS="The colours screen: a palette the profile does not own pages in as locked, and A on its colour buzzes, the look as it was.",
        START=START,
        STEPS=[
            *OPEN,
            ('do', 'press DOWN; wait 8; press R; wait 8'),
            ('expect', 'looks~page Undernet locked'),
            ('do', 'press A; wait 8; press UP; wait 8; press A; wait 8'),
            ('expect', 'looks~grid page Undernet locked cell 8'),      # still in the grid: nothing picked
            ('expect', 'look=-,-,- slot 0'),
            ('do', 'press B; wait 8'),
            ('expect', 'looks~at armour list'),
            ('expect', 'look=-,-,- slot 0'),
        ]),
    'copy': dict(
        GUARDS="The colours screen: a look copied with the Look row's pop-up is the other look's and worn; over a look that is not plain it asks first, No chosen.",
        START=START,
        STEPS=[
            *OPEN,
            ('do', 'press DOWN; wait 8; press A; wait 8; press UP; wait 8; press A; wait 8'),   # armour: Classic 12
            ('expect', 'look=0.12,-,- slot 0'),
            ('do', 'press UP; wait 8; press A; wait 8'),
            ('expect', 'looks~popup 0'),
            ('do', 'press A; wait 8'),                                                            # copied to B, B worn
            ('expect', 'look=0.12,-,- slot 1'),
            ('do', 'press A; wait 8; press A; wait 8'),                                           # B over C: C plain, no question
            ('expect', 'look=0.12,-,- slot 2'),
            ('do', 'press A; wait 8; press A; wait 8'),                                           # C over A: A not plain, asked, No chosen
            ('expect', 'looks~popup 0 asking no'),
            ('do', 'press A; wait 8'),                                                            # No: back to the pop-up, nothing lost
            ('expect', 'look=0.12,-,- slot 2'),
            ('do', 'press B; wait 8; press DOWN; wait 8; press DOWN; wait 8; press A; wait 8; press UP; wait 8; press UP; wait 8; press UP; wait 8; press A; wait 8'),   # suit: Classic 4 on C
            ('expect', 'look=0.12,0.4,- slot 2'),
            ('do', 'press UP; wait 8; press UP; wait 8; press A; wait 8; press A; wait 8; press RIGHT; wait 8; press A; wait 8'),     # C over A, Yes
            ('expect', 'look=0.12,0.4,- slot 0'),
        ]),
}
