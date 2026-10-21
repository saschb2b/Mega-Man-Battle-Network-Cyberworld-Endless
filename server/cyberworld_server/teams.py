"""Team NetBattles (epic #167, protocol v12): teams of two, and a relay
between two teams.

A player asks another of his channel to team up (TEAM propose); the other
answers. Two players of one team stay a team until one splits it, leaves
the channel or the server. A Battle (ARENA's challenge) between players of
two teams is a team challenge (arena.py): its Yes begins a team match here
instead of a battle. Each team's captain (the one who asked to team up)
picks who leads, blind: neither order is shown until both are in, and a
captain who has not picked in ORDER_WAIT seconds leads himself. Then the
bouts: each is the Arena's NetBattle as ever (a Match of arena.py, whose
two fighters both say they are ready with ARENA accept before START), the
leads first; the bout's winner (the cable's first's report, stands.py)
stays with the HP he has left, the loser's partner comes in; a team is out
when both its MegaMen are deleted. A bout with no winner puts both out.
The two waiting players are told each bout as it starts, so their games
watch it from the stands. A bout broken off (a leave, a drop, a decline,
a bout never begun) ends the match broken off: no result for either team.
Nothing is won or lost: the server keeps no record of a team match but
its events."""

import time

from . import protocol, websocket

PROPOSE, ANSWER, SPLIT, ORDER = range(1, 5)                                    # TEAM's actions
ASKED, WAITING, FORMED, SPLIT_UP, MATCH, ORDERED, BOUT, FIGHT, OVER = range(1, 10)  # TEAM_STATE's stages
WON, LOST, BROKE_OFF, DRAWN = 1, 2, 3, 4                                      # OVER's result, the recipient's team's
ASK_WAIT = 30         # seconds a proposal waits for its answer
ORDER_WAIT = 30       # ... a captain's order, before he leads himself
MEMBERS = 2


class Team:
    def __init__(self, captain, partner):
        self.captain, self.partner = captain, partner

    @property
    def members(self):
        return (self.captain, self.partner)

    def other(self, player_id):
        return self.partner if player_id == self.captain else self.captain


class TeamMatch:
    def __init__(self, match_id, teams):
        self.id = match_id
        self.teams = teams                    # (challenger's, challenged's)
        self.order = [None, None]             # each team's [lead, second], once its captain picked
        self.time = time.monotonic()
        self.next = [0, 0]                    # each team's next MegaMan in its order
        self.hp = 0                           # the staying MegaMan's HP, as the last bout's report left it (0 whole)
        self.number = 0                       # bouts called
        self.bout = None                      # the Arena's Match under way

    def players(self):
        return [p for team in self.teams for p in team.members]

    def side_of(self, player_id):
        return 0 if player_id in self.teams[0].members else 1

    def left(self, side):
        return MEMBERS - self.next[side]


class Teams:
    def __init__(self, server):
        self.server = server
        self.of = {}           # player id -> Team
        self.asked = {}        # proposer id -> (target id, time)
        self.matches = {}      # team match id -> TeamMatch
        self.in_match = {}     # player id -> team match id
        self.next = 1

    # -- the wire --

    def send(self, player_id, stage, partner=0, match=None, result=0):
        player = self.server.players.get(player_id)
        if not player or player.writer.is_closing():
            return
        ids, left, number, bout, hp, match_id = [0, 0, 0, 0], (0, 0), 0, 0, 0, 0
        if match:
            side = match.side_of(player_id)
            for k, s in enumerate((side, side ^ 1)):
                # (each team's order once both are in, its MegaMan up now first; before, captain and partner)
                order = match.order[s] if match.order[0] and match.order[1] else list(match.teams[s].members)
                n = min(match.next[s], MEMBERS - 1)
                ids[2 * k:2 * k + 2] = order[n:] + order[:n]
            left = (match.left(side), match.left(side ^ 1))
            number, hp, match_id = match.number, match.hp, match.id
            bout = match.bout.id if match.bout else 0
        player.writer.write(websocket.frame(protocol.team_state(stage, partner, match_id, bout, ids, left, number, result, hp)))

    def busy(self, player_id):
        """In a team match (a bout's or waiting for one): a challenge, a trade or a watch of his own refused."""
        return player_id in self.in_match

    def team_of(self, player_id):
        return self.of.get(player_id)

    # -- teams --

    async def handle(self, player, data):
        action, ref, value = protocol.read_team(data)
        if action == SPLIT:
            self.split(player.id)
            return
        if not self.server.arena_on:
            await self.server.notice(player, 18, "The Arena is closed on this server")
            return
        if await self.server.without_pass(player):
            return
        if action == PROPOSE:
            await self.propose(player, ref)
        elif action == ANSWER:
            await self.answer(player, ref, value)
        elif action == ORDER:
            self.pick(player, ref)

    def free(self, player_id):
        server = self.server
        return not (player_id in server.arena.of or player_id in server.trades.of or server.stands.busy(player_id)
                    or player_id in self.in_match)

    async def propose(self, player, target_id):
        target = self.server.players.get(target_id)
        if not target or target.id == player.id or target.channel != player.channel:
            await self.server.notice(player, 17, "That Navi is no longer here")
            return
        if await self.server.without_pass(player, target):
            return
        if player.id in self.of or target.id in self.of:
            await self.server.notice(player, protocol.SPEC["notice_team"], "One of you is on a team already")
            return
        if not self.free(player.id) or not self.free(target.id) or self.server.holds.held(target) or player.id in self.asked \
                or any(t == target.id for t, _ in self.asked.values()):
            await self.server.notice(player, 16, "That Navi is busy")
            return
        self.asked[player.id] = (target.id, time.monotonic())
        self.send(player.id, WAITING, target.id)
        self.send(target.id, ASKED, player.id)

    async def answer(self, player, proposer_id, yes):
        asked = self.asked.get(proposer_id)
        if not asked or asked[0] != player.id:
            await self.server.notice(player, 17, "That Navi is no longer here")
            return
        del self.asked[proposer_id]
        if not yes or proposer_id not in self.server.players or proposer_id in self.of or player.id in self.of:
            self.send(proposer_id, SPLIT_UP, player.id)
            return
        team = Team(proposer_id, player.id)
        self.of[proposer_id] = self.of[player.id] = team
        self.send(proposer_id, FORMED, player.id)
        self.send(player.id, FORMED, proposer_id)
        self.server.event("team", captain=proposer_id, partner=player.id)

    def split(self, player_id, gone=False):
        """A team split (or a proposal taken back, a question turned down): both told."""
        asked = self.asked.pop(player_id, None)
        if asked:
            self.send(asked[0], SPLIT_UP, player_id)
        for proposer, (target, _) in list(self.asked.items()):
            if target == player_id:
                del self.asked[proposer]
                self.send(proposer, SPLIT_UP, player_id)
        team = self.of.get(player_id)
        if not team:
            return
        match = self.matches.get(self.in_match.get(player_id, 0))
        if match:
            self.broken(match, gone=(player_id,) if gone else ())
        for member in team.members:
            self.of.pop(member, None)
        self.send(team.other(player_id), SPLIT_UP, player_id)
        if not gone:
            self.send(player_id, SPLIT_UP, team.other(player_id))

    def left(self, player_id):
        self.split(player_id, gone=True)

    # -- a team match --

    def challenge(self, a_id, b_id):
        """A challenge from a to b: True a team challenge (two teams, all four free), False a lone one; None refused (NOTICE notice_team)."""
        ta, tb = self.of.get(a_id), self.of.get(b_id)
        if not ta and not tb:
            return False
        if not ta or not tb or ta is tb:
            return None
        return True

    def partners_free(self, a_id, b_id):
        """Both still on two teams (a team split since the challenge: False), and both partners free."""
        if self.challenge(a_id, b_id) is not True:
            return False
        return all(self.free(p) and p in self.server.players for p in (self.of[a_id].other(a_id), self.of[b_id].other(b_id)))

    def begin(self, a_id, b_id):
        """A team challenge's Yes: the match begins, each captain asked the order."""
        match = TeamMatch(self.next, (self.of[a_id], self.of[b_id]))
        self.next = self.next % 0xffffffff + 1
        self.matches[match.id] = match
        for p in match.players():
            self.in_match[p] = match.id
            self.server.stands.stop(p)
        for p in match.players():
            self.send(p, MATCH, self.of[p].other(p), match)
        self.server.event("team_match", match=match.id, players=match.players())

    def pick(self, player, lead):
        match = self.matches.get(self.in_match.get(player.id, 0))
        team = self.of.get(player.id)
        if not match or not team or team.captain != player.id or match.number or lead not in team.members:
            return
        side = match.side_of(player.id)
        if match.order[side] is None:
            match.order[side] = [lead, team.other(lead)]
        if match.order[0] and match.order[1]:
            self.ordered(match)

    def ordered(self, match):
        for p in match.players():
            self.send(p, ORDERED, self.of[p].other(p), match)
        self.call(match)

    def fighter(self, match, side):
        return match.order[side][match.next[side]]

    def call(self, match):
        """The next bout called: its two fighters say they are ready (ARENA accept), then START."""
        match.number += 1
        a, b = self.fighter(match, 0), self.fighter(match, 1)
        match.bout = self.server.arena.bout(a, b, match)
        for p in match.players():
            self.send(p, BOUT, self.of[p].other(p), match)

    def fight(self, bout):
        """A bout started: the two waiting are told, so that their games watch it."""
        match = self.matches.get(getattr(bout, "team", 0))
        if match:
            for p in match.players():
                self.send(p, FIGHT, self.of[p].other(p), match)

    def bout_over(self, bout, finished):
        """A bout's end from arena.py: the relay goes on, or the match is over (or broken off)."""
        match = self.matches.get(getattr(bout, "team", 0))
        if not match or match.bout is not bout:
            return
        match.bout = None
        book = self.server.stands.books.get(bout.id)
        status = book.status if book else None
        if not finished or not status:
            self.broken(match)
            return
        hp0, _, hp1, _, _, _, _, _, winner = status
        side_a = match.side_of(bout.a)
        # (winner 1 the cable's first, bout.a; 2 bout.b; 0 none: both out)
        if winner != 1:
            match.next[side_a] += 1
        if winner != 2:
            match.next[side_a ^ 1] += 1
        match.hp = hp0 if winner == 1 else hp1 if winner == 2 else 0
        self.server.event("team_bout", match=match.id, bout=bout.id, winner=winner, hp=match.hp)
        out = [match.left(s) == 0 for s in (0, 1)]
        if out[0] and out[1]:
            self.over(match, None, drawn=True)
        elif out[0] or out[1]:
            self.over(match, 1 if out[0] else 0)
        else:
            self.call(match)

    def over(self, match, winner_side, drawn=False, gone=()):
        """The match over: each told his team's result (won, lost, drawn: both teams out in one bout; else broken off)."""
        self.matches.pop(match.id, None)
        for p in match.players():
            self.in_match.pop(p, None)
        for p in match.players():
            if p in gone:
                continue
            side = match.side_of(p)
            result = DRAWN if drawn else BROKE_OFF if winner_side is None else WON if winner_side == side else LOST
            team = self.of.get(p)
            self.send(p, OVER, team.other(p) if team else 0, match, result)
        self.server.event("team_over", match=match.id, winner=-1 if winner_side is None else winner_side)

    def broken(self, match, gone=()):
        """A team match broken off: no result for either team; a bout under way ended for both."""
        bout, match.bout = match.bout, None
        if bout:
            self.server.arena.end(bout, told=gone)
        self.over(match, None, gone=gone)

    def expire(self, now):
        for proposer, (target, since) in list(self.asked.items()):
            if now - since > ASK_WAIT:
                del self.asked[proposer]
                self.send(proposer, SPLIT_UP, target)
                self.send(target, SPLIT_UP, proposer)
        for match in list(self.matches.values()):
            if not match.number and now - match.time > ORDER_WAIT:
                for side in (0, 1):
                    if match.order[side] is None:
                        team = match.teams[side]
                        match.order[side] = [team.captain, team.partner]
                self.ordered(match)
