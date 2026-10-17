"""Reviewed acceptance for a BN6 routine-level staged chip swap.

The original Take/Give calls supply expected bytes. Candidate code must not
edit this comparator or widen its individual-field boundary to obtain a pass.
"""
import hashlib
import json
from pathlib import Path
import re
import struct

SHA256 = re.compile(r'[0-9a-f]{64}')
TARGET = {
    'rom_sha256': '572e113eeb53bb29cd9ff8acb9db265cfd48c5e509c8d0e6420b58e71e442cf2',
    'state_sha256': '4b921c9cf48c22085605714797373114de206a32b8bba821606dc348f73f95b6',
    'library_sha256': '5b02195c77b74ffa6622952706025bc7e5a77bd5d14d9306e0bbc84ab789f0cd',
}
# offsetof(GBASerializedState, wram/iwram), sizeof, and the pinned Gregar
# state header: measured with mGBA 0.10.5, not inferred from file lengths.
LAYOUT = dict(core_bytes=397312, wram=135168, iwram=102400,
              magic=0x01000007, rom_crc=0x79452182, rom_id=0x45355242)
EWRAM, EWRAM_BYTES, IWRAM, IWRAM_BYTES = 0x02000000, 0x40000, 0x03000000, 0x8000
TOOLKIT, TOOLKIT_PACK, TOOLKIT_MARKS = 0x020093b0, 0x4c, 0x7c
EVENT_FLAGS, LIBRARY_FLAG, ORDER_COUNTER = 0x02001c88, 0x1e20, 0x020065b4
PACK_IDS, PACK_BYTES, PACK_ORDER, MARK_BYTES = 320, 12, 4, 516
RETURN_PC, TAKE, GIVE = 0x080050ec, 0x08021b93, 0x08021aef
CASES = {
    'first-code': dict(give=2, receive=3, slot=0, codes=(11, 17), outgoing=3, incoming=0, counter=0x7fff),
    'second-code': dict(give=3, receive=2, slot=1, codes=(18, 12), outgoing=3, incoming=0, counter=0x7fff),
    'counter-wrap': dict(give=2, receive=3, slot=0, codes=(11, 17), outgoing=1, incoming=0, counter=0),
    'incoming-98': dict(give=3, receive=2, slot=1, codes=(18, 12), outgoing=3, incoming=98, counter=1),
}


# Independent unmodified-source reload controls, reviewed in
# docs/re/trade-stage-reload.md. These are full-state pins, never masks.
RELOAD_SOURCE_SHA256 = 'ed0b8ba29a4ef71bc1cbc19a75b930564b98ded427c313204edcbf28b2ab4377'
RELOAD_BINARY_SHA256 = '7bf8c483defbdaa78a1fd267acd53bc6fb6ba9d7cb2ad62ca73e8021495b0717'
RELOAD_HASHES = {
    'first-code': '64389d911cc93b04d64c95b47506e6a8cbe932007bc8cfaac60531b0a6f6afb2',
    'second-code': 'a734435a8976bb17fe0e8b1daa23405383fba82c9ac7c6fe54a9784aeda36cf8',
    'counter-wrap': '0e32bac01d8ce6fe6fe7ab2fa9c19acd6beec15215c9d4693a35f7bf5cdbf183',
    'incoming-98': 'ba7d6186aa59edd3fe88094295a45ebe4aaa0bf352d1672067cea17144d94b61',
}


class Mismatch(Exception):
    """Reached evidence disagrees."""


class Unreached(Mismatch):
    """Missing evidence or an original path not reached; never a pass."""


def require(condition, message):
    if not condition:
        raise Mismatch(message)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def offset(address, size=1):
    require(EWRAM <= address <= EWRAM + EWRAM_BYTES - size, 'Invalid inventory RAM range')
    return LAYOUT['wram'] + address - EWRAM


def u16(data, at):
    return struct.unpack_from('<H', data, at)[0]


def u32(data, at):
    return struct.unpack_from('<I', data, at)[0]


def state(path):
    try:
        data = Path(path).read_bytes()
    except OSError as error:
        raise Unreached(f'Missing state: {path}') from error
    if len(data) < LAYOUT['core_bytes']:
        raise Unreached('Truncated serialized state')
    require((u32(data, 0), u32(data, 8), u32(data, 28)) ==
            (LAYOUT['magic'], LAYOUT['rom_crc'], LAYOUT['rom_id']), 'Wrong serialized state target')
    return data


def verify_manifest(manifest, reviewed):
    require(type(manifest) is dict and set(manifest) ==
            {'before', 'after', 'layout', 'sources', 'binaries', 'fixtures'}, 'Incomplete target manifest')
    require(manifest['before'] == TARGET == manifest['after'], 'Target changed or wrong target')
    require(manifest['layout'] == LAYOUT, 'Wrong mGBA serialized layout')
    require(type(reviewed) is dict and reviewed and manifest['sources'] == reviewed, 'Unreviewed source pins')
    for value in reviewed.values():
        require(type(value) is str and SHA256.fullmatch(value), 'Invalid source hash')
    require(type(manifest['binaries']) is dict and set(manifest['binaries']) == {'oracle', 'candidate', 'reload'},
            'Missing actual binary pins')
    require(manifest['binaries']['reload'] == RELOAD_BINARY_SHA256 and
            reviewed.get('tools/online_trade_reload.c') == RELOAD_SOURCE_SHA256, 'Unreviewed reload observer')
    for value in manifest['binaries'].values():
        require(type(value) is str and SHA256.fullmatch(value), 'Invalid binary hash')
    require(type(manifest['fixtures']) is dict and set(manifest['fixtures']) == set(CASES), 'Missing source fixture pins')
    for value in manifest['fixtures'].values():
        require(type(value) is str and SHA256.fullmatch(value), 'Invalid fixture hash')


def inventory(before, case):
    pack = u32(before, offset(TOOLKIT + TOOLKIT_PACK, 4))
    marks = u32(before, offset(TOOLKIT + TOOLKIT_MARKS, 4))
    flag = LIBRARY_FLAG + case['receive']
    fields = {
        'outgoing-count': (offset(pack + case['give'] * PACK_BYTES + case['slot']), 1),
        'incoming-count': (offset(pack + case['receive'] * PACK_BYTES + case['slot']), 1),
        'incoming-order': (offset(pack + case['receive'] * PACK_BYTES + PACK_ORDER + 2 * case['slot'], 2), 2),
        'incoming-mark': (offset(marks + case['receive']), 1),
        'incoming-library': (offset(EVENT_FLAGS + flag // 8), 1),
        'order-counter': (offset(ORDER_COUNTER, 2), 2),
    }
    complete = ((offset(pack, PACK_IDS * PACK_BYTES), PACK_IDS * PACK_BYTES),
                (offset(marks, MARK_BYTES), MARK_BYTES),
                (offset(EVENT_FLAGS + LIBRARY_FLAG // 8, 64), 64),
                (offset(ORDER_COUNTER, 2), 2))
    return fields, complete


def differing(a, b):
    require(len(a) == len(b), 'Serialized length changed')
    return [i for i, (x, y) in enumerate(zip(a, b)) if x != y]


def positions(fields):
    return {i for at, size in fields for i in range(at, at + size)}


def compare_regions(a, b, regions, label):
    for at, size in regions:
        require(a[at:at + size] == b[at:at + size], f'{label}: field at {at} differs')


def check_call(call, name, case):
    expected = [case['give'], case['codes'][0], 0 if name == 'noop' else 1] if name != 'give' else \
               [case['receive'], case['codes'][1], 1]
    require(type(call) is dict and set(call) == {'function', 'arguments', 'return_pc', 'steps', 'minimum_sp', 'before', 'after'},
            'Incomplete original call observation')
    require(call['function'] == (GIVE if name == 'give' else TAKE) and call['arguments'] == expected,
            'Wrong original callee or arguments')
    if call['return_pc'] != RETURN_PC or type(call['steps']) is not int or not 0 < call['steps'] < 100000:
        raise Unreached('Original routine did not return through the observed path')
    for regs in (call['before'], call['after']):
        require(type(regs) is list and len(regs) == 16 and
                all(type(v) is int and 0 <= v <= 0xffffffff for v in regs), 'Incomplete register observation')
    require(call['before'][10] == TOOLKIT and call['after'][10] == TOOLKIT, 'Wrong toolkit context')
    require(call['before'][4:12] == call['after'][4:12] and call['before'][13] == call['after'][13],
            'Original callee changed a preserved register')
    require(type(call['minimum_sp']) is int and IWRAM <= call['minimum_sp'] <= call['before'][13] <= IWRAM + IWRAM_BYTES,
            'Invalid observed original stack extent')
    require(call['after'][0] == 0, 'Original chip operation refused')


def oracle(first, second, case_name):
    require(case_name in CASES, 'Unknown routine fixture')
    case = CASES[case_name]
    try:
        calls = json.loads((Path(first) / 'calls.json').read_text())
        repeated = json.loads((Path(second) / 'calls.json').read_text())
    except (OSError, ValueError) as error:
        raise Unreached('Missing original return observations') from error
    require(calls == repeated, 'Original return observations do not repeat')
    require(type(calls) is dict and set(calls) == {'noop', 'take', 'give'}, 'Missing original call observations')
    a, b = {}, {}
    for name in ('before', 'noop', 'taken', 'given'):
        a[name], b[name] = state(Path(first) / f'{name}.state'), state(Path(second) / f'{name}.state')
        require(a[name] == b[name], f'Original controls differ: {name}')
    before, taken, given = a['before'], a['taken'], a['given']
    fields, complete = inventory(before, case)
    for name in calls:
        check_call(calls[name], name, case)
    compare_regions(before, a['noop'], complete, 'No-op original call changed inventory')
    for name, expected in (('outgoing-count', case['outgoing']), ('incoming-count', case['incoming'])):
        require(before[fields[name][0]] == expected, 'Wrong starting count fixture')
    require(u16(before, offset(ORDER_COUNTER, 2)) == case['counter'], 'Wrong starting order fixture')
    # These establish that the requested original path was exercised. Actual
    # expected candidate bytes below always come from the original result.
    if taken[fields['outgoing-count'][0]] != case['outgoing'] - 1 or \
            given[fields['incoming-count'][0]] != case['incoming'] + 1:
        raise Unreached('Original exact-code take/give was not exercised')
    allowed = positions(fields.values())
    ram = range(LAYOUT['wram'], LAYOUT['wram'] + EWRAM_BYTES)
    stack_low = min(call['minimum_sp'] for call in calls.values())
    stack_high = max(call['before'][13] for call in calls.values())
    stack = range(LAYOUT['iwram'] + stack_low - IWRAM, LAYOUT['iwram'] + stack_high - IWRAM)
    iram = range(LAYOUT['iwram'], LAYOUT['iwram'] + IWRAM_BYTES)
    for name, result in a.items():
        unexpected = [i for i in differing(before, result) if i in ram and i not in allowed]
        if unexpected:
            raise Unreached(f'Unclassified original EWRAM effect in {name}: {unexpected[0]}')
        if any(i in iram and i not in stack for i in differing(before, result)):
            raise Unreached(f'Unclassified original IWRAM effect in {name}')
        require(before[LAYOUT['core_bytes']:] == result[LAYOUT['core_bytes']:], 'Original call changed flash/extdata')
    for name in ('incoming-order', 'order-counter'):
        at, size = fields[name]
        if before[at:at + size] == given[at:at + size]:
            raise Unreached(f'Original {name} effect not exercised')
    if not case['incoming']:
        for name in ('incoming-mark', 'incoming-library'):
            at, size = fields[name]
            if before[at:at + size] == given[at:at + size]:
                raise Unreached(f'Original {name} effect not exercised')
    return before, given, fields, complete


def reload_controls(first, second, case_name):
    a, b = state(first), state(second)
    if a != b or digest(a) != RELOAD_HASHES.get(case_name):
        raise Unreached('Unmodified-source reload controls differ or changed from their pin')
    return a


def accept_candidate(before, original, fields, complete, candidate, reloaded, live_before, live_after, control):
    allowed = positions(fields.values())
    changes = differing(before, candidate)
    require(all(i in allowed for i in changes), 'Candidate changed a protected serialized byte')
    compare_regions(original, candidate, complete, 'Candidate differs from original inventory')
    compare_regions(before, control, complete, 'Unmodified-source reload changed inventory')
    require(all(i in allowed for i in differing(control, reloaded)), 'Reload changed a protected serialized byte')
    compare_regions(original, reloaded, complete, 'Reloaded inventory differs from original')
    require(live_before == live_after, 'Live Square sentinel changed')
    return dict(verdict='PASS', oracle='original-routines', changed_bytes=len(changes), masks=[])


def editor(path):
    try:
        data = Path(path).read_bytes()
    except OSError as error:
        raise Unreached('Missing original editor observation') from error
    if len(data) != 512 * 27:
        raise Unreached('Truncated original editor count observation')
    require(all(value <= 99 for value in data), 'Invalid original editor count')
    return data


def accept_case(directory, case_name, manifest, reviewed):
    """Complete acceptance entry; the proof driver must call this for every case."""
    verify_manifest(manifest, reviewed)
    root = Path(directory)
    before, original, fields, complete = oracle(root / 'original-a', root / 'original-b', case_name)
    require(digest(before) == manifest['fixtures'][case_name], 'Pinned source fixture changed')
    require(state(root / 'source-after.state') == before, 'Source checkpoint buffer/file changed')
    control = reload_controls(root / 'reload-a.state', root / 'reload-b.state', case_name)
    result = accept_candidate(before, original, fields, complete, state(root / 'candidate.state'),
                              state(root / 'reloaded.state'), state(root / 'live-before.state'),
                              state(root / 'live-after.state'), control)
    counts = editor(root / 'original-a' / 'editor.counts')
    require(counts == editor(root / 'original-b' / 'editor.counts'), 'Original editor controls differ')
    require(counts == editor(root / 'candidate-editor.counts'), 'Reloaded candidate editor differs')
    result['case'] = case_name
    return result
