"""The Square's phrases and emotes (protocol v14, issue #176; docs/ONLINE.md,
Emotes): a burst, then one a second; an emote aimed at a MegaMan, its cap
on one target unanswered (past it only the sender sees his), an answer
within five seconds flagged for every screen; and the server's beat, which
every emote starts on, so that a group's play in step on every screen."""

import math

from . import protocol

SPEC = protocol.SPEC


class Social:
    def __init__(self, server):
        self.server = server
        # (sender, target) -> the times of the sender's aimed emotes at the
        # target he has not answered, newest last (monotonic seconds)
        self.aims = {}

    def allow(self, player, now):
        """The burst (social_burst) and its refill (one each social_refill_ms)."""
        burst, refill = SPEC["social_burst"], SPEC["social_refill_ms"] / 1000
        player.social_tokens = min(burst, player.social_tokens + (now - player.social_time) / refill)
        player.social_time = now
        if player.social_tokens < 1:
            return False
        player.social_tokens -= 1
        return True

    def prune(self, now):
        window = SPEC["aim_window_ms"] / 1000
        for key in list(self.aims):
            times = [t for t in self.aims[key] if now - t < window]
            if times:
                self.aims[key] = times
            else:
                del self.aims[key]

    def aim(self, sender, target, now):
        """An aimed emote from `sender` at `target`: its EVENT's flags (an
        answer to one he aimed at the sender within answer_ms), and whether
        only the sender is shown it (his past aim_cap, unanswered)."""
        self.prune(now)
        theirs = self.aims.get((target, sender), [])
        answered = bool(theirs) and now - theirs[-1] <= SPEC["answer_ms"] / 1000
        if answered:
            # (an answer: both ways start afresh)
            self.aims.pop((target, sender), None)
            self.aims.pop((sender, target), None)
            return SPEC["event_answered"], False
        mine = self.aims.setdefault((sender, target), [])
        shadow = len(mine) >= SPEC["aim_cap"]
        if not shadow:
            mine.append(now)
        return 0, shadow

    def left(self, player_id):
        for key in [k for k in self.aims if player_id in k]:
            del self.aims[key]

    def beat(self, channel, now):
        """Milliseconds from now to the beat an emote sent now starts on: the
        server's clock in beat_ms steps, at least the farthest one-way trip
        of the channel's games (half the round trip their PINGs say) and
        beat_lead_ms ahead, at most beat_lead_most_ms."""
        trips = [p.round_trip for p in self.server.players.values() if p.channel == channel]
        lead = min(SPEC["beat_lead_most_ms"], max(trips, default=0) // 2 + SPEC["beat_lead_ms"])
        clock = now * 1000
        step = SPEC["beat_ms"]
        return int(math.ceil((clock + lead) / step) * step - clock)
