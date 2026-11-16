"""Trade Central (docs/online/trade-central.md): a spare chip offered to
another player's MegaMan at the clerk's desk, his chip offered back, both
players' Yes on the chip trade, and each one's spares after, in the Square
and at home; an offer refused, a deal declined, one trade a visit; a trade
cut off by a game closed or the server gone before it is committed (no
chip moves on either side) and by a game closed after (the swap is the
run's at CONTINUE, once); an offer on the trade board, read and answered
by another; and a game without the Trade stamp, which the clerk offers
nothing."""
import scenarios_online as online
from scenarios_online import soon

NEEDS = ('bn5', 'bn4')   # (a trade takes the NetPass's full set on both games)
CAST = dict(START=online.HOME, NEEDS=NEEDS, SERVER=True, GAMES={'beta': online.HOME}, FILES={'netpass.ini': online.SEEN_FULL})
DESK = 'place 20 -372 7; wait 10'      # before Trade Central's clerk, facing him (seed 120's Square)
BEFORE = 'spares=2: 1Ax2 20Fx2'        # Alpha's spares as he comes: two Cannon A, two FireBrn1 F
BEFORE_BETA = 'spares=2: 1Ax2 411Mx2'  # Beta's: two Cannon A, two MrkCan1 M
AFTER = 'spares=3: 1Ax2 20F 411M'      # either's after the trade: one of his own left, one of the other's


def page(game=''):
    """A chat box's words paged to its end."""
    return online.at(game, ('repeat', 'press A; wait 6', 'chat=closed', 30))


# (each MegaMan brings spares: the chip step writes its chip over the folder's first entry, the one before left a spare)
ARRIVE = online.course(
    online.paged(),
    ('do', 'chip 20 F 1; chip 1 A 1; wait 10'),
    ('expect', BEFORE),
    online.paged('beta'),
    ('@beta', 'do', 'chip 411 M 1; chip 1 A 1; wait 10'),
    ('@beta', 'expect', BEFORE_BETA),
    online.into_square(),
    online.into_square('beta'),
    soon('peers~Beta'),
)
# (Alpha at the desk: the clerk's hello paged to his options, "Offer a chip", then Beta, then FireBrn1 F on the spares list)
OFFER = online.course(
    ('do', DESK),
    ('repeat', 'press A; wait 10', 'choice=1/3', 20),
    ('repeat', 'press A; wait 10', 'tradescreen=list', 20),
    ('expect', 'listrow=1/2'),
    ('repeat', 'press DOWN; wait 4', 'listchip=20F', 4),
    ('do', 'press A; wait 10'),
    soon('trade=waiting'),
    ('expect', 'gives=20F'),
    ('expect', 'tradewith=Beta'),
    page(),                                # the clerk's word on it
    soon('trade=offered', 'beta'),         # MegaMan's word on the offer, wherever he stands
    ('@beta', 'expect', 'gets=20F'),
    ('@beta', 'expect', 'tradewith=Alpha'),
    page('beta'),
)
# (Beta to the desk: the clerk tells the offer, "Offer one back", MrkCan1 M on the list; the chip trade at once)
ANSWER = online.course(
    ('@beta', 'do', DESK),
    ('@beta', 'repeat', 'press A; wait 10', 'choice=1/2', 20),
    ('@beta', 'repeat', 'press A; wait 10', 'tradescreen=list', 20),
    ('@beta', 'repeat', 'press DOWN; wait 4', 'listchip=411M', 4),
    ('@beta', 'do', 'press A; wait 10'),
    soon('tradescreen=deal', 'beta'),
    ('@beta', 'expect', 'deal=ask'),
    ('@beta', 'expect', 'dealcursor=no'),
    ('@beta', 'expect', 'gives=411M'),
    soon('trade=answered'),                # Alpha: MegaMan's word on the answer, then the chip trade
    ('repeat', 'press A; wait 10', 'tradescreen=deal', 20),
    ('expect', 'gets=411M'),
    ('expect', 'dealcursor=no'),
)


def deal(game=''):
    """The chip trade's Yes: the cursor off No, A."""
    return online.at(game, ('repeat', 'press RIGHT; wait 4', 'dealcursor=yes', 4), ('do', 'press A; wait 10'))


def unchanged():
    """Neither Pack changed, and nothing kept for home."""
    return [('expect', BEFORE), ('expect', 'pending=none'), ('@beta', 'expect', BEFORE_BETA), ('@beta', 'expect', 'pending=none')]


MANY = {
    'chip-for-chip': dict(CAST, STEPS=online.course(
        ARRIVE, OFFER, ANSWER,
        deal('beta'),
        ('@beta', 'expect', 'deal=wait'),          # "Beta's still thinking": nothing swapped on one Yes
        ('@beta', 'expect', BEFORE_BETA),
        ('expect', BEFORE),
        deal(),
        soon('trade=done'),
        soon('deal=swapped'),
        ('expect', AFTER),                         # the swap made in the Square's own game, and kept for home
        ('expect', 'pending=20F for 411M'),
        soon('trade=done', 'beta'),
        soon('deal=swapped', 'beta'),
        ('@beta', 'expect', AFTER),
        ('@beta', 'expect', 'pending=411M for 20F'),
        ('repeat', 'press A; wait 10', 'tradescreen=none', 6),
        page(),                                    # "Deal! ... is ours once we're home!"
        online.home_again(),
        ('expect', AFTER),                         # home has it, and nothing is left to settle
        ('expect', 'pending=none'),
    )),
    'offer-refused': dict(CAST, STEPS=online.course(
        ARRIVE, OFFER,
        ('@beta', 'do', DESK),
        ('@beta', 'repeat', 'press A; wait 10', 'choice=1/2', 20),
        ('@beta', 'repeat', 'press DOWN; wait 4', 'choice=2/2', 4),   # "No thanks"
        ('@beta', 'do', 'press A; wait 10'),
        soon('trade=cancelled'),
        page(),                                    # "the trade's off"
        soon('trade=cancelled', 'beta'),
        page('beta'),
        unchanged(),
    )),
    'no-deal': dict(CAST, STEPS=online.course(
        ARRIVE, OFFER, ANSWER,
        deal('beta'),
        ('do', 'press A; wait 10'),                # Alpha's A with the cursor on No
        soon('trade=cancelled'),
        ('expect', 'tradescreen=none'),
        soon('trade=cancelled', 'beta'),
        soon('tradescreen=none', 'beta'),          # Beta's waiting screen closes with it
        unchanged(),
    )),
    'after-the-deal': dict(CAST, STEPS=online.course(
        ARRIVE, OFFER, ANSWER,
        deal('beta'),
        deal(),
        soon('deal=swapped', 'beta'),
        ('@beta', 'expect', AFTER),
        ('@beta', 'repeat', 'press A; wait 10', 'tradescreen=none', 6),
        page('beta'),
        # (one trade a visit: the clerk says so, and lists nothing)
        ('@beta', 'do', 'wait 20; press A; wait 20'),
        soon('chat=open', 'beta'),
        page('beta'),
        ('@beta', 'expect', 'tradescreen=none'),
        ('@beta', 'expect', 'trade=done'),
        # (Beta's game closed in the Square, the swap kept on disk: CONTINUE at the gate has it, and nothing is left over)
        ('@beta', 'expect', 'pending=411M for 20F'),
        ('@beta', 'restart', online.HOME.replace('--scene home', '--scene title')),
        ('@beta', 'repeat', 'press START; wait 40', 'title=menu'),
        ('@beta', 'expect', 'cursor=continue'),
        ('@beta', 'repeat', 'press A; wait 40', 'scene=emu'),
        ('@beta', 'until', 'where=town', 1200),
        ('@beta', 'repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('@beta', 'expect', AFTER),
        ('@beta', 'expect', 'pending=none'),
    )),
    'closed-before-the-deal': dict(CAST, STEPS=online.course(
        ARRIVE, OFFER, ANSWER,
        deal('beta'),
        ('@beta', 'expect', 'deal=wait'),
        # (Alpha's game closed with Beta's Yes said and his own not: no trade)
        ('restart', online.HOME.replace('--scene home', '--scene title')),
        soon('trade=cancelled', 'beta', 900),
        soon('tradescreen=none', 'beta'),
        page('beta'),
        ('@beta', 'expect', BEFORE_BETA),
        ('@beta', 'expect', 'pending=none'),
        ('repeat', 'press START; wait 40', 'title=menu'),
        ('repeat', 'press A; wait 40', 'scene=emu'),
        ('until', 'where=town', 1200),
        ('repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('expect', BEFORE),
        ('expect', 'pending=none'),
    )),
    'server-gone-mid-trade': dict(CAST, STEPS=online.course(
        ARRIVE, OFFER, ANSWER,
        deal('beta'),
        ('server', 'stop'),
        soon('visit=waiting', '', 600),
        ('repeat', 'press B; wait 10', 'visit=home', 6),   # B gives up: home, the keeper's word
        ('expect', 'visitended=lost'),
        ('repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('expect', BEFORE),
        ('expect', 'pending=none'),
        soon('visit=waiting', 'beta', 600),
        ('@beta', 'repeat', 'press B; wait 10', 'visit=home', 6),
        ('@beta', 'repeat', 'mash A 60; wait 30', 'chat=closed'),
        ('@beta', 'expect', BEFORE_BETA),
        ('@beta', 'expect', 'pending=none'),
    )),
    'board-offer': dict(CAST, STEPS=online.course(
        ARRIVE,
        # (Alpha at the clerk: "Trade board", no offers up, "Put mine up": FireBrn1 F for anything)
        ('do', DESK),
        ('repeat', 'press A; wait 10', 'choice=1/3', 20),
        ('do', 'wait 20'),
        ('repeat', 'press DOWN; wait 6', 'choice=2/3', 4),
        ('do', 'press A; wait 20'),
        ('repeat', 'press A; wait 10', 'choice=1/2', 20),
        ('repeat', 'press A; wait 20', 'tradescreen=list', 8),
        ('repeat', 'press DOWN; wait 4', 'listchip=20F', 4),
        ('do', 'press A; wait 20'),
        ('repeat', 'press A; wait 10', 'choice~/3', 20),     # what he wants for it: anything the first
        online.pick('tradeboard=1'),
        ('expect', 'myoffer~has 20F wants 0'),
        page(),
        soon('tradeboard=1', 'beta'),                             # read by the other
        ('@beta', 'expect', 'tradeoffer=Alpha has 20F wants 0'),
        # (Beta at the clerk: "Trade board", Alpha's offer read out, "Answer it" with MrkCan1 M)
        ('@beta', 'do', DESK),
        ('@beta', 'repeat', 'press A; wait 10', 'choice=1/3', 20),
        ('@beta', 'do', 'wait 20'),
        ('@beta', 'repeat', 'press DOWN; wait 6', 'choice=2/3', 4),
        ('@beta', 'do', 'press A; wait 20'),
        ('@beta', 'repeat', 'press A; wait 10', 'choice=1/2', 20),
        ('@beta', 'repeat', 'press A; wait 20', 'tradescreen=list', 8),
        ('@beta', 'repeat', 'press DOWN; wait 4', 'listchip=411M', 4),
        ('@beta', 'do', 'press A; wait 10'),
        soon('tradescreen=deal', 'beta'),                    # both chips named at once: the chip trade
        ('@beta', 'expect', 'gives=411M'),
        ('@beta', 'expect', 'gets=20F'),
        soon('trade=answered'),                              # "Lan! Beta took our offer!", then the chip trade
        ('repeat', 'press A; wait 10', 'tradescreen=deal', 20),
        ('expect', 'gives=20F'),
        ('expect', 'gets=411M'),
    )),
    'no-stamp': dict(START=online.HOME, NEEDS=(), SERVER=True, FILES={'netpass.ini': online.SEEN_SQUARE}, STEPS=online.course(
        online.paged(),
        ('do', 'chip 20 F 1; chip 1 A 1; wait 10'),
        ('expect', BEFORE),
        online.into_square(),
        ('expect', 'netpass=square'),
        ('do', DESK),
        ('repeat', 'press A; wait 10', 'chat=open', 6),
        ('repeat', 'press A; wait 10', 'chat=closed', 20),   # the clerk's word on the stamp, and no options
        ('expect', 'tradescreen=none'),
        ('expect', 'trade=none'),
    )),
}
