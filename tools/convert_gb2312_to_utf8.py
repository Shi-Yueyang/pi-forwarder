#!/usr/bin/env python3
"""
Recursively convert text files from GB2312/GBK to UTF-8.

Usage:
    python convert_gb2312_to_utf8.py <input_folder> <output_folder>

The script preserves the directory structure of <input_folder> under
<output_folder>. Files that cannot be decoded as GBK are copied as-is
(so binary files are not corrupted).
"""

import argparse
import os
import shutil
import sys
from pathlib import Path


def is_likely_text_file(file_path: Path, sample_size: int = 8192) -> bool:
    """Heuristic: check whether a file looks like a text file."""
    try:
        with file_path.open("rb") as f:
            chunk = f.read(sample_size)
    except OSError:
        return False

    if not chunk:
        return True

    # If there is a null byte, treat it as binary.
    if b"\x00" in chunk:
        return False

    # Check for common binary signatures.
    binary_signatures = (
        b"\x89PNG",
        b"\xff\xd8\xff",
        b"PK\x03\x04",
        b"Rar!",
        b"7z\xbc\xaf",
        b"\x1f\x8b",
        b"GIF8",
        b"BM",
        b"%PDF",
    )
    if chunk.startswith(binary_signatures):
        return False

    return True


def convert_file(src: Path, dst: Path, src_encoding: str = "gbk") -> bool:
    """
    Try to decode `src` with `src_encoding` and write it as UTF-8 to `dst`.
    Returns True if conversion succeeded, False if the file was copied as-is.
    """
    try:
        text = src.read_text(encoding=src_encoding)
        dst.write_text(text, encoding="utf-8")
        return True
    except (UnicodeDecodeError, LookupError):
        # Not valid GBK / decoding not supported: copy raw bytes.
        shutil.copy2(src, dst)
        return False
    except OSError as exc:
        print(f"[ERROR] {src}: {exc}", file=sys.stderr)
        return False


def convert_tree(input_dir: Path, output_dir: Path, encoding: str = "gbk") -> None:
    """Recursively convert all files from `input_dir` to `output_dir`."""
    converted = 0
    copied = 0
    errors = 0

    for root, _, files in os.walk(input_dir):
        rel_root = Path(root).relative_to(input_dir)
        out_root = output_dir / rel_root

        for name in files:
            src_file = Path(root) / name
            dst_file = out_root / name

            try:
                dst_file.parent.mkdir(parents=True, exist_ok=True)

                if not is_likely_text_file(src_file):
                    shutil.copy2(src_file, dst_file)
                    copied += 1
                    print(f"[COPY  BINARY] {src_file.relative_to(input_dir)}")
                    continue

                if convert_file(src_file, dst_file, encoding):
                    converted += 1
                    print(f"[CONVERT] {src_file.relative_to(input_dir)}")
                else:
                    copied += 1
                    print(f"[COPY  FAIL] {src_file.relative_to(input_dir)}")
            except Exception as exc:  # noqa: BLE001
                errors += 1
                print(f"[ERROR] {src_file}: {exc}", file=sys.stderr)

    print("\nDone.")
    print(f"  Converted: {converted}")
    print(f"  Copied as-is: {copied}")
    print(f"  Errors: {errors}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Recursively convert files from GB2312/GBK to UTF-8."
    )
    parser.add_argument("input_folder", type=Path, help="Source folder to read from.")
    parser.add_argument(
        "output_folder", type=Path, help="Destination folder to write to."
    )
    parser.add_argument(
        "--encoding",
        default="gbk",
        help="Source encoding to use (default: gbk, a superset of gb2312).",
    )
    args = parser.parse_args()

    input_dir = args.input_folder.resolve()
    output_dir = args.output_folder.resolve()

    if not input_dir.is_dir():
        print(f"Input folder does not exist: {input_dir}", file=sys.stderr)
        return 1

    if output_dir.exists() and not output_dir.is_dir():
        print(f"Output path exists but is not a directory: {output_dir}", file=sys.stderr)
        return 1

    convert_tree(input_dir, output_dir, args.encoding)
    return 0


if __name__ == "__main__":
    sys.exit(main())
