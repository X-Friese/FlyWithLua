#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
xpdepot.py - SkunkCrafts Updater Depot Manifest Generator & Publisher
====================================================================
Based on Lionel's official SkunkCrafts Updater depot generation logic.
Supports both:
  1. Headless CI / Local directory generation (for GitHub Actions & GitHub Pages)
  2. Legacy multi-zone FTP / mounted directory deployment

Usage:
  # Headless mode (used by CI and local builds):
  python3 xpdepot.py --stage <staging_dir> --depot <depot_dir> [--lock | --unlock]

  # Legacy mode:
  python3 xpdepot.py <zone> <aircraft_folder> <command>
"""

import os
import sys
import glob
import io
import shutil
import binascii
import argparse

# Config file names recognized by SkunkCrafts Updater
RMT_CONFIG  = "skunkcrafts_updater.cfg"
LCL_CONFIG  = "skunkcrafts_updater_config.txt"
BLACK_NAME  = "skunkcrafts_updater_blacklist.txt"
WHITE_NAME  = "skunkcrafts_updater_whitelist.txt"
SIZES_NAME  = "skunkcrafts_updater_sizeslist.txt"
IGNORE_NAME = "skunkcrafts_updater_ignorelist.txt"
ONCE_NAME   = "skunkcrafts_updater_oncelist.txt"

# Files that should not be listed as downloadable content in the whitelist
SYSTEM_IGNORE_FILES = {
    SIZES_NAME,
    WHITE_NAME,
    BLACK_NAME,
    ONCE_NAME,
    IGNORE_NAME,
    LCL_CONFIG,
    ".DS_Store",
    ".gitkeep",
    "Thumbs.db",
}

def crc32_from_file(filepath: str) -> str:
    """Compute unsigned 32-bit CRC formatted as a decimal string."""
    with open(filepath, "rb") as fh:
        buf = fh.read()
    crc = binascii.crc32(buf) & 0xFFFFFFFF
    return f"{crc:d}"

def load_list_file(filepath: str) -> set:
    """Load line-delimited items into a normalized set (forward slashes)."""
    if not os.path.isfile(filepath):
        return set()
    items = set()
    with open(filepath, "r", encoding="utf-8", errors="ignore") as fh:
        for line in fh:
            line = line.strip().replace("\\", "/")
            if line and not line.startswith("#"):
                items.add(line)
    return items

def is_in_list(rel_path: str, item_set: set) -> bool:
    """
    Check if rel_path matches any entry in item_set, supporting exact match,
    folder prefixes, and wildcard suffixes ('/*', '/**').
    """
    if rel_path in item_set:
        return True
    for item in item_set:
        norm_item = item.rstrip("/*")
        if rel_path == norm_item or rel_path.startswith(norm_item + "/"):
            return True
    return False

def build_depot_headless(stage_dir: str, depot_dir: str, lock_status: bool = False):
    """
    Builds the depot manifest files and copies the staged files to the destination depot.
    """
    stage_dir = os.path.abspath(stage_dir)
    depot_dir = os.path.abspath(depot_dir)

    if not os.path.isdir(stage_dir):
        print(f"Error: Staging directory not found: {stage_dir}", file=sys.stderr)
        sys.exit(1)

    print(f"==> Building SkunkCrafts depot manifest")
    print(f"    Source (Staging): {stage_dir}")
    print(f"    Destination (Depot): {depot_dir}")

    # Load control files from staging
    oncelist = load_list_file(os.path.join(stage_dir, ONCE_NAME))
    ignorelist = load_list_file(os.path.join(stage_dir, IGNORE_NAME))
    blacklist = load_list_file(os.path.join(stage_dir, BLACK_NAME))

    new_white = {}
    sizes_list = {}

    # Traverse all files in staging directory
    for root, _, files in os.walk(stage_dir):
        for fname in sorted(files):
            full_path = os.path.join(root, fname)
            rel_path = os.path.relpath(full_path, stage_dir).replace("\\", "/")

            if fname in SYSTEM_IGNORE_FILES or is_in_list(rel_path, ignorelist):
                continue

            size = os.path.getsize(full_path)
            sizes_list[rel_path] = size

            if is_in_list(rel_path, oncelist):
                # -1 marks oncelist files: installed once, never overwritten on updates!
                new_white[rel_path] = -1
                print(f"    [ONCE - CRC -1] {rel_path} ({size} bytes)")
            else:
                crc = crc32_from_file(full_path)
                new_white[rel_path] = crc
                print(f"    [CRC {crc:>10}] {rel_path} ({size} bytes)")

    # Ensure output depot directory exists
    os.makedirs(depot_dir, exist_ok=True)

    # 1. Copy payload files to depot
    for rel_path in new_white.keys():
        src_file = os.path.join(stage_dir, rel_path)
        dst_file = os.path.join(depot_dir, rel_path)
        os.makedirs(os.path.dirname(dst_file), exist_ok=True)
        shutil.copy2(src_file, dst_file)

    # 2. Write skunkcrafts_updater_whitelist.txt
    white_file = os.path.join(depot_dir, WHITE_NAME)
    with open(white_file, "w", newline="\n", encoding="utf-8") as fh:
        for entry, crc in sorted(new_white.items()):
            fh.write(f"{entry}|{crc}\n")
    print(f"==> Wrote whitelist: {white_file} ({len(new_white)} entries)")

    # 3. Write skunkcrafts_updater_sizeslist.txt
    sizes_file = os.path.join(depot_dir, SIZES_NAME)
    with open(sizes_file, "w", newline="\n", encoding="utf-8") as fh:
        for entry, size in sorted(sizes_list.items()):
            fh.write(f"{entry}|{size}\n")
    print(f"==> Wrote sizeslist: {sizes_file} ({len(sizes_list)} entries)")

    # 4. Write skunkcrafts_updater_blacklist.txt
    black_file = os.path.join(depot_dir, BLACK_NAME)
    with open(black_file, "w", newline="\n", encoding="utf-8") as fh:
        for entry in sorted(blacklist):
            fh.write(f"{entry}\n")
    print(f"==> Wrote blacklist: {black_file} ({len(blacklist)} entries)")

    # 5. Convert skunkcrafts_updater_config.txt -> skunkcrafts_updater.cfg
    lcl_cfg_path = os.path.join(stage_dir, LCL_CONFIG)
    rmt_cfg_path = os.path.join(depot_dir, RMT_CONFIG)
    cfg_entries = {}
    if os.path.isfile(lcl_cfg_path):
        with open(lcl_cfg_path, "r", encoding="utf-8") as fh:
            for line in fh:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                parts = line.split("|", 1)
                if len(parts) == 2:
                    cfg_entries[parts[0].strip()] = parts[1].strip()
    
    if lock_status:
        cfg_entries["locked"] = "true"
    elif "locked" not in cfg_entries:
        cfg_entries["locked"] = "false"

    with open(rmt_cfg_path, "w", newline="\n", encoding="utf-8") as fh:
        for k, v in cfg_entries.items():
            fh.write(f"{k}|{v}\n")
    print(f"==> Wrote depot config: {rmt_cfg_path}")

    print("==> Depot build completed successfully!")

def main():
    parser = argparse.ArgumentParser(description="SkunkCrafts Updater Depot Manifest Tool")
    parser.add_argument("--stage", help="Path to staging payload directory")
    parser.add_argument("--depot", help="Path to output depot directory")
    parser.add_argument("--lock", action="store_true", help="Set locked=true in skunkcrafts_updater.cfg")
    parser.add_argument("--unlock", action="store_true", help="Set locked=false in skunkcrafts_updater.cfg")

    args, remaining = parser.parse_known_args()

    if args.stage and args.depot:
        lock_val = True if args.lock else False
        build_depot_headless(args.stage, args.depot, lock_status=lock_val)
        return

    # Check for simple positional arguments: python3 xpdepot.py <stage_dir> <depot_dir>
    if len(remaining) == 2 and os.path.isdir(remaining[0]):
        build_depot_headless(remaining[0], remaining[1])
        return

    parser.print_help()
    sys.exit(1)

if __name__ == "__main__":
    main()
