"""xnavirecipe_bn4_art.py: BN4's Navis' art's room for tools/xnavirecipe_bn4.py (docs/multirom/bn4-navis.md, Art and songs by
battle): one room the battles share, as their battle sprites' (src/emu/xnavi_bn4_room.h), filled at a battle's start from
the player's BN4 ROM with what the coming battle's Navi draws and plays: his attacks' and effects' sprites and his pool's
songs. Each Navi's are laid out in it from its start, the eighteen over one another, so the room holds the largest Navi's
alone. A sprite another Navi copied first (his list number, xnavirecipe_bn4_objects.navi_sprite and navi_effect_sprites)
is listed by each at a place of his own layout: the fill points the list's entry at the battle's Navi's place. A sprite a
family copied stays in the families' art, resident (xrecipe_bn4.move_effect's 'installed with it')."""


def begin(f, alloc, room):
    """his layout from the room's start (room: its first address)"""
    alloc.art = room
    f.art_room = room


def own_rows(f, alloc):
    """his rows in the room another Navi listed first (the same list and number: a copy of that Navi's row, its place in
    that Navi's layout) each given a place of his own, past what he laid out; the rows' first lister kept by (list, number)"""
    if not hasattr(alloc, 'art_rows'):
        alloc.art_rows = {}
    placed = {}
    for s in f.attack_sprites:
        if not f.art_room <= s['at'] < alloc.art_end:
            continue
        key = (s['to_list'], s['to_index'])
        first = alloc.art_rows.setdefault(key, f.name)
        if first == f.name:
            continue
        if key not in placed:
            placed[key] = alloc.art
            alloc.art += s['room']
            f.notes.append(f'its sprite {s["list"]}:{s["index"]:#x} ({first}\'s list {s["to_list"]} {s["to_index"]:#x}) at a place of '
                           f'his own in the room, {placed[key]:08X}: the fill points the entry at it in his battles')
        s['at'] = placed[key]
    if alloc.art > alloc.art_end:
        f.decide('room', f'its sprites end at {alloc.art:08X}, past {alloc.room}\'s art ({alloc.art_end:08X})')


def end(f, alloc):
    """his art's and songs' bytes in the room, measured; the room's largest kept (alloc.art_most: the Navi and its bytes)"""
    songs = sum(s['room'] for s in getattr(f, 'songs', []))
    f.art_bytes = (alloc.art - f.art_room - songs, songs)
    most = getattr(alloc, 'art_most', None)
    if most is None or sum(f.art_bytes) > sum(most[1]):
        alloc.art_most = (f.name, f.art_bytes)


def report(navis, alloc):
    """the report's lines: each Navi's sprites' and songs' bytes in the room, the largest, and the room's size"""
    L = ['## The art\'s room, by battle (tools/xnavirecipe_bn4_art.py)', '']
    for f in navis:
        sp, so = getattr(f, 'art_bytes', (0, 0))
        L.append(f'- {f.name}: sprites {sp:#x}, songs {so:#x}, together {sp + so:#x} ({(sp + so) / 1024:.1f} KB)')
    most = getattr(alloc, 'art_most', None)
    if most:
        L.append(f'- the largest, {most[0]}\'s, {sum(most[1]):#x} of the room\'s {alloc.art_end - navis[0].art_room:#x}')
    return L + ['']
