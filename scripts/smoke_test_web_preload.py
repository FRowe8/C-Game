#!/usr/bin/env python3
"""
Quick smoke test for Emscripten asset preloading.

Reads the generated `.data` bundle and asserts that key asset paths are
present. This does not fully validate integrity but catches packaging
regressions where assets are omitted from the virtual filesystem.
"""
import argparse
import sys
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description="Smoke-test Emscripten asset preload bundle")
    parser.add_argument("data_file", type=Path, help="Path to QuantumIdle.data produced by emscripten")
    parser.add_argument("assets", nargs="+", help="Asset paths expected to be embedded (e.g. assets/fonts/Roboto-Regular.ttf)")
    args = parser.parse_args()

    data_path: Path = args.data_file
    if not data_path.is_file():
        print(f"❌ Data bundle not found: {data_path}", file=sys.stderr)
        return 1

    blob = data_path.read_bytes()
    missing = [asset for asset in args.assets if asset.encode() not in blob]

    if missing:
        print("❌ Asset preload check failed.")
        for asset in missing:
            print(f"   - Missing entry: {asset}")
        return 1

    print(f"✅ Asset preload bundle contains {len(args.assets)} required entries")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
