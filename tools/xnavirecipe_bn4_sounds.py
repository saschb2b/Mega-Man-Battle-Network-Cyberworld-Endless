"""xnavirecipe_bn4_sounds.py: BN4's Navis' sounds for tools/xnavirecipe_bn4.py (docs/multirom/bn4-navis.md, GutsMan built):
the sounds their code plays, BN6's of the same song, a family's slot or the pool's (src/emu/xnavi_bn4_sounds.h), the
calls past a byte, and the pool's songs laid out in the art's room (tools/xnavirecipe_bn4_art.py) on their players"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import xrecipe_bn4 as X4  # noqa: E402
import xrecipe_thumb as T  # noqa: E402

# A table of its moved objects' kinds whose bytes name a sound their init plays (BN4_PLAY_SOUND with r0 a byte of it, not a
# movs): its address -> its entries, their size, the kinds, the sound's place in an entry, the play it reaches, why; each
# kind's sound given BN6's number as a movs's would be (xrecipe_bn4.sounds_of: BN6's of the same song, else its slot)
SOUND_TABLES = {
    0x080D0A60: dict(entry=6, kinds=(0, 1, 2), sound=4, play=0x080D0B1A,
                     why="the third kind's 0x1E's kinds (6 bytes each: its speed, its +0x64, its animation, its collision's, its sound, "
                         "the status it gives), read by its init (0x080D0AAE, r7 the kind's by +4) to the code at 0x080D0A74"),
}
# Their sounds BN6 has no song of (the owner's call of 5 November 2026, docs/multirom/bn4.md call 11): a shared pool of BN6's
# empty song slots a Navi's take, its songs packed into its art's room at install and the pool's slots pointed at the songs
# of the Navi its battle holds (src/emu/xnavi_bn4_sounds.c). The pool: BN6's empty song slots nothing else takes (docs/rom/
# sound.md: 0x26-0x62 and fourteen past them, the families' 0x34-0x3F, BN5's 0x40-0x62 and the themes' 0x26-0x31 left
# alone), the five a movs or a byte can name (to 0xFF) first. A Navi's k-th sound its k-th slot: the ones its code names by
# a byte the low five
SOUND_POOL = X4.SOUND_POOL   # (the families' generator's list, a late family's sounds taking it too: the owner's call 13)
SONG_STEP = 0x40


def table_sounds(b4, b6, f):
    """its sounds named by a table's bytes (SOUND_TABLES): each kind's BN6's song of its number where the same, else its
    slot of BN6's (xrecipe_bn4.SOUND_SLOTS) as a byte of the table's copy, else the owner's call; the play's 'made otherwise'
    decision answered"""
    for v, t in SOUND_TABLES.items():
        if not any(t['play'] in w.bls for w in f.walks.values()):
            continue
        f.decisions = [d for d in f.decisions if not (d[0] == 'sound' and d[1] == f'a sound played at {t["play"]:08X} made otherwise')]
        for k in t['kinds']:
            at = v + t['entry'] * k + t['sound']
            snd = b4.bytes(at, 1)[0]
            if X4.song_sig(b4, X4.BN4['songs'], snd) == X4.song_sig(b6, X4.SONGS6, snd):
                f.notes.append(f'{at:08X}: its table\'s kind {k} plays {snd:#x}, BN6\'s of the same number the same song')
            elif X4.SOUND_SLOTS.get(snd, 0) > 0xFF:
                f.decide('sound', f'{at:08X}: its table\'s kind {k} plays {snd:#x} at the pool\'s {X4.SOUND_SLOTS[snd]:#x}, past a '
                                  f'byte: its shim (the owner\'s call of 5 November 2026) is built for a movs alone')
            elif snd in X4.SOUND_SLOTS:
                f.bytes.append((at, snd, X4.SOUND_SLOTS[snd], f'its table\'s kind {k}: its sound {snd:#x}, BN4\'s copied into BN6\'s slot '
                                                               f'{X4.SOUND_SLOTS[snd]:#x}'))
            else:
                f.decide('sound', f'{at:08X}: its table\'s kind {k} plays {snd:#x}, no song of BN6\'s and no slot (SOUND_SLOTS): ask')


def wide_sounds(f):
    """its sounds named by a movs whose pool slot is past a byte (the owner's call of 5 November 2026, docs/multirom/bn4.md
    call 11): the movs left as BN4's, its call to BN4_PLAY_SOUND aimed at a shim that makes r0 the slot (XvSoundCall)"""
    f.sound_calls = list(getattr(f, 'wide_calls', []))   # (a movs and an adds past a byte: xnavirecipe_bn4_objects.wide_movs)
    wide = {i['at']: i for i in f.imms if i['form'] == 'THUMB_MOVS' and i['now'] > 0xFF and i['why'].startswith('its sound ')}
    for _, w in sorted(f.walks.items()):
        for site, t in sorted(w.bls.items()):
            if t != X4.BN4['sound']:
                continue
            v, chain = T.back_value(w, site, 0)
            if len(chain) == 1 and chain[0][0] in wide:
                i = wide[chain[0][0]]
                f.sound_calls.append(dict(at=site, slot=i['now'], why=f'its sound {v:#x} at the pool\'s {i["now"]:#x}, past a byte'))
    f.imms = [i for i in f.imms if i['at'] not in wide]


def sound_ids(b4, f):
    """every sound its code plays: a movs's before BN4_PLAY_SOUND, a kind's byte of SOUND_TABLES"""
    ids = set()
    for w in f.walks.values():
        for site, t in w.bls.items():
            if t == X4.BN4['sound']:
                v, _ = T.back_value(w, site, 0)
                if v is not None:
                    ids.add(v)
    if getattr(f, 'summon', None):
        ids.add(f.summon['sound4'])   # (the summon's, played by its shim: tools/xnavirecipe_bn4_summon.py)
    for v, t in SOUND_TABLES.items():
        if any(t['play'] in w.bls for w in f.walks.values()):
            ids |= {b4.bytes(v + t['entry'] * k + t['sound'], 1)[0] for k in t['kinds']}
    return ids


def pool_of(b4, b6, f):
    """its sounds that are no song of BN6's and no family's (xrecipe_bn4.SOUND_SLOTS), each its slot of SOUND_POOL in order;
    one past what a movs can name (0xFF) played through a shim (wide_sounds), past the pool a decision"""
    odd = sorted(v for v in sound_ids(b4, f) if X4.song_sig(b4, X4.BN4['songs'], v) != X4.song_sig(b6, X4.SONGS6, v)
                 and v not in X4.SOUND_SLOTS)
    # (then the sounds of the families it summons whose slot is its pool's: xrecipe_bn4.SUMMONER_POOL, each the next slot)
    extra = sorted(v for v, (_, who) in X4.SUMMONER_POOL.items() if who == f.name and v not in odd)
    if len(odd) + len(extra) > len(SOUND_POOL):
        f.decide('sound', f'{len(odd) + len(extra)} sounds for the pool\'s {len(SOUND_POOL)}')
    pool = dict(zip(odd + extra, SOUND_POOL))
    for v in extra:
        if pool.get(v) != X4.SUMMONER_POOL[v][0]:
            f.decide('sound', f'its summoned family\'s sound {v:#x} at {pool.get(v)}, where its recipe names {X4.SUMMONER_POOL[v][0]:#x}')
    f.notes.append('its sounds in the pool: ' + ', '.join(f'{v:#x} at {slot:#x}' for v, slot in pool.items()))
    return pool


def songs_of(b4, f, pool, alloc):
    """its pool's songs, each packed into its art's room at install (xnavi_bn4_sounds.c), its bytes rounded up to SONG_STEP,
    each on BN4's player but where BN6 keeps that player for one song of its own (song_player)"""
    f.songs = []
    # (the players its pool's songs keep, BN4's numbers, which a song moved off its player must not take: in BN4 none shares
    # one with another, as Roll's 0x131 on its 22 and her summoned Mettaur's 0x8B on its 9)
    kept = {b4.u16(X4.BN4['songs'] + 8 * v + 4) for v in pool}
    for v, slot in pool.items():
        room = X4.rounded(X4.song_bytes(b4, v, X4.BN4['songs']), SONG_STEP)
        f.songs.append(dict(song=v, slot=slot, at=alloc.art, room=room, player=song_player(b4, f, v, kept)))
        alloc.art += room
    if alloc.art > alloc.art_end:
        f.decide('room', f'its songs end at {alloc.art:08X}, past {alloc.room}\'s art ({alloc.art_end:08X})')


# The sound effects' numbers whose players are compared (BN4's and BN6's song tables, 0x60 on: their music below)
SFX_FIRST, SFX_END = 0x60, 0x180


def song_player(b4, f, v, kept=()):
    """BN6's player for its pooled song v, 0 where BN4's own number serves: where BN6 keeps BN4's player number for one song
    alone (its 22, its hit's 0x6B alone, which BN4 plays on its 10: a pooled song of BN4's 22 would be cut by BN6's hit
    sound, as BN4's is not), the BN6 player most of the numbers of BN4's songs on that player take in BN6, past the players
    its pool's other songs keep (`kept`, BN4's numbers: BN4's 22 takes 9 three times in BN6, but Roll's pool holds her
    summoned Mettaur's 0x8B on its 9, which would cut her 0x131 as BN4's never does, roll_v2 at t1880: 10, the first of
    the next)"""
    b6 = f.b6
    p4 = b4.u16(X4.BN4['songs'] + 8 * v + 4)
    on6 = [n for n in range(SFX_FIRST, SFX_END) if b6.u16(X4.SONGS6 + 8 * n + 4) == p4]
    if len(on6) != 1:
        return 0
    took = [b6.u16(X4.SONGS6 + 8 * n + 4) for n in range(SFX_FIRST, SFX_END) if b4.u16(X4.BN4['songs'] + 8 * n + 4) == p4]
    took = [p for p in took if p not in kept] or took
    p6 = max(sorted(set(took)), key=took.count)
    f.notes.append(f'its sound {v:#x} on BN6\'s player {p6}: BN4\'s player {p4} is one BN6 keeps for its song {on6[0]:#x} alone, '
                   f'and most of BN4\'s songs of that player take {p6} in BN6 by their numbers')
    return p6
