"""A place held through a dropped link (issue #153, protocol v10).

A session that ends without a goodbye (BYE, or a WebSocket close) keeps
its place for HOLD seconds: its slot in its channel, its id, and its
MegaMan standing still in the others' snapshots. A HELLO from the same
identity with the resume bit (hello_resume) takes it back, id and all, so
the others never see him leave; the trade's last word (done or off) is
said again to it, as it may have been lost with the link. What was under
way ends at the drop, as on any leave: a trade not committed is off, a
NetBattle ends without a result, a spectator's watching stops. A goodbye,
a refusal or a moderator's removal frees the place at once; so does a
HELLO without the resume bit (a new visit from the gate), as a new arrival.

docs/ONLINE.md, A dropped link."""

import time

from . import websocket

# The game tries for 30 seconds after its link goes quiet or drops
# (src/online/online_transport.c, RECONNECT_MS); its last try may take a
# handshake's few seconds more, and the server may notice a drop before
# the game does (a reset) or after it (15 seconds of silence). Forty
# seconds from the server's notice covers the game's whole window.
HOLD = 40.0

# A stalled link (a Wi-Fi gap the sockets survive) lets the game's positions
# through at once when it clears, 20 a second of it: positions have a bucket
# this deep of their own (app.py's serve_player), ten seconds' worth, so the
# backlog is no flood, and the game sends fewer while it hears nothing
# (online_transport.c). Each only replaces the last.
BACKLOG = 200


class Holds:
    def __init__(self, server, seconds=HOLD):
        self.server, self.seconds = server, seconds
        self.until = {}        # player id -> monotonic deadline

    def held(self, player):
        return player.id in self.until

    def gone(self, player, goodbye):
        """The session of `player` ended (handle's end): what was under way
        ended, then the place freed, or held where it ended without a word."""
        server = self.server
        server.trades.left(player)
        server.arena.left(player.id)
        server.stands.left(player)
        if goodbye or not self.seconds or player.refused:
            self.free(player)
            return
        self.until[player.id] = time.monotonic() + self.seconds
        server.event("hold", id=player.id, seconds=self.seconds)

    def free(self, player):
        server = self.server
        if server.players.get(player.id) is not player:
            return
        del server.players[player.id]
        self.until.pop(player.id, None)
        server.trades.done.discard(player.id)
        server.trades.last.pop(player.id, None)
        server.event("leave", id=player.id, players=len(server.players))

    def forget(self, old):
        """`old`'s place given up to a new visit of the same identity (a
        HELLO without the resume bit): no hold, a new trade allowed."""
        self.until.pop(old.id, None)
        self.server.trades.done.discard(old.id)
        self.server.trades.last.pop(old.id, None)

    def removed(self, player):
        """A moderator's removal: a place held freed at once."""
        if self.held(player):
            self.free(player)

    def expire(self, now):
        for player_id, deadline in list(self.until.items()):
            player = self.server.players.get(player_id)
            if not player:
                self.until.pop(player_id, None)
            elif now >= deadline:
                self.free(player)

    def take_back(self, old, player):
        """A resume's HELLO: `player` takes `old`'s place, his id, channel,
        position, card and the visit's one trade with it."""
        self.until.pop(old.id, None)
        player.id, player.channel, player.position = old.id, old.channel, old.position
        player.profile, player.profile_time, player.joined = old.profile, old.profile_time, old.joined
        player.offer_time = old.offer_time
        self.server.event("resume", id=player.id, identity=player.identity.hex(), channel=player.channel)

    async def resumed(self, player):
        """After a resume's WELCOME: the trade's last word said again (the
        game's own copy of it was what the link lost, or one it has: it
        sets a repeat aside)."""
        word = self.server.trades.last.get(player.id)
        if word and not player.writer.is_closing():
            await websocket.send(player.writer, word)
