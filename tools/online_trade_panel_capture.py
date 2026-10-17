"""Capture a static trade panel in the actual game with a local presence server.

Used by build.py screenshots. The panel is visibly labelled as a preview;
this fixture cannot trade or modify a Pack. All runtime files stay in .build.
"""
import asyncio
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'server'))
from cyberworld_server.app import Server  # noqa: E402


async def capture(phase, args):
    server = Server()
    port = await server.start('127.0.0.1', 0)
    process = None
    try:
        process = await asyncio.create_subprocess_exec(
            str(ROOT / 'build/host/cyberworld'), *args,
            '--online', f'ws://127.0.0.1:{port}/presence', '--online-name', 'PanelReview',
            env=dict(os.environ, CYBERWORLD_TRADE_PREVIEW=phase))
        return await asyncio.wait_for(process.wait(), 45)
    finally:
        if process and process.returncode is None:
            process.kill()
            await process.wait()
        await server.close()


if __name__ == '__main__':
    if len(sys.argv) < 3 or sys.argv[1] not in tuple(map(str, range(8))):
        raise SystemExit('usage: online_trade_panel_capture.py PHASE GAME_ARGUMENTS...')
    raise SystemExit(asyncio.run(capture(sys.argv[1], sys.argv[2:])))
