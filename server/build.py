#!/usr/bin/env python3
"""Build a standalone, dependency-free Python zipapp without game assets."""
import argparse
from pathlib import Path
import shutil
import tempfile
import zipapp


def main():
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=root / "build/server/cyberworld-server.pyz")
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="cyberworld-server-") as temporary:
        package = Path(temporary) / "cyberworld_server"
        shutil.copytree(root / "server/cyberworld_server", package, ignore=shutil.ignore_patterns("__pycache__"))
        shutil.copy2(root / "shared/online/protocol.json", package / "protocol.json")
        shutil.copy2(root / "LICENSE", Path(temporary) / "LICENSE")
        zipapp.create_archive(temporary, args.output, interpreter="/usr/bin/env python3",
                              main="cyberworld_server.cli:main", compressed=True)
    for source in (root / "LICENSE", root / "server/README.md", root / "server/server.example.ini"):
        destination = args.output.parent / source.name
        if source.resolve() != destination.resolve():
            shutil.copy2(source, destination)
    print(args.output)


if __name__ == "__main__":
    main()
