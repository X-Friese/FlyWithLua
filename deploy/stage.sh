#!/usr/bin/env bash
# ==============================================================================
# FlyWithLua Staging & Packaging Tool
# ------------------------------------------------------------------------------
# Assembles the multi-channel SkunkCrafts staging payload for FlyWithLua,
# runs xpdepot.py to compute manifests (whitelist, sizeslist, oncelist),
# and optionally packages standalone zip distributions.
#
# Usage:
#   ./deploy/stage.sh [release|beta|testing|all] [--build-depot] [--package-zip]
# ==============================================================================

set -euo pipefail

TARGET="${1:-release}"
BUILD_DEPOT=false
PACKAGE_ZIP=false

for arg in "$@"; do
    if [[ "$arg" == "--build-depot" ]]; then
        BUILD_DEPOT=true
    fi
    if [[ "$arg" == "--package-zip" ]]; then
        PACKAGE_ZIP=true
    fi
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
FWL_SRC="$REPO_ROOT/FlyWithLua"

stage_channel() {
    local CHANNEL="$1"
    local CHANNEL_CFG="$REPO_ROOT/deploy/channels/${CHANNEL}_config.txt"
    if [[ ! -f "$CHANNEL_CFG" ]]; then
        echo "Error: Unknown channel configuration: $CHANNEL_CFG"
        echo "Available channels: release, beta, testing, all"
        exit 1
    fi

    local STAGING_DIR="$REPO_ROOT/build/staging/$CHANNEL/FlyWithLua"
    echo "==> Staging FlyWithLua for channel: $CHANNEL"
    rm -rf "$STAGING_DIR"
    mkdir -p "$STAGING_DIR"

    # 1. Copy core directories and static assets
    echo "    Copying core assets..."
    cp -a "$FWL_SRC/Custom_Fonts" "$STAGING_DIR/"
    cp -a "$FWL_SRC/Documentation" "$STAGING_DIR/"
    cp -a "$FWL_SRC/Internals" "$STAGING_DIR/"
    cp -a "$FWL_SRC/Modules" "$STAGING_DIR/"
    cp -a "$FWL_SRC/README.txt" "$STAGING_DIR/"
    cp -a "$FWL_SRC/fwl_prefs.ini" "$STAGING_DIR/"
    cp -a "$FWL_SRC/user.ini" "$STAGING_DIR/"
    cp -a "$FWL_SRC/user.exit" "$STAGING_DIR/"
    cp -a "$FWL_SRC/Scripts (disabled)" "$STAGING_DIR/"

    # 2. Assemble Scripts directory (Whitelisted demo + Oncelist starter + Add-ons)
    echo "    Assembling Scripts directory..."
    mkdir -p "$STAGING_DIR/Scripts"
    if [[ -f "$FWL_SRC/Scripts/please read the manual.lua" ]]; then
        cp -a "$FWL_SRC/Scripts/please read the manual.lua" "$STAGING_DIR/Scripts/"
    fi
    if [[ -f "$FWL_SRC/Scripts/ImGui_LuiGee_Demo.lua" ]]; then
        cp -a "$FWL_SRC/Scripts/ImGui_LuiGee_Demo.lua" "$STAGING_DIR/Scripts/"
    fi
    if [[ -d "$FWL_SRC/Scripts/CockpitNotes" ]]; then
        cp -a "$FWL_SRC/Scripts/CockpitNotes" "$STAGING_DIR/Scripts/"
    fi

    # 3. Binaries
    echo "    Assembling multiplatform binaries..."
    mkdir -p "$STAGING_DIR/mac_x64" "$STAGING_DIR/win_x64" "$STAGING_DIR/lin_x64"

    # Mac: check build/mac.xpl first, fallback to FlyWithLua/mac_x64/FlyWithLua.xpl
    if [[ -f "$REPO_ROOT/build/mac.xpl" ]]; then
        cp -a "$REPO_ROOT/build/mac.xpl" "$STAGING_DIR/mac_x64/FlyWithLua.xpl"
        echo "    [Mac] Staged local build: build/mac.xpl"
    elif [[ -f "$FWL_SRC/mac_x64/FlyWithLua.xpl" ]]; then
        cp -a "$FWL_SRC/mac_x64/FlyWithLua.xpl" "$STAGING_DIR/mac_x64/FlyWithLua.xpl"
        echo "    [Mac] Staged existing binary: FlyWithLua/mac_x64/FlyWithLua.xpl"
    fi

    # Windows: copy win_x64 DLLs and .xpl if present
    if [[ -d "$FWL_SRC/win_x64" ]]; then
        cp -a "$FWL_SRC/win_x64/"* "$STAGING_DIR/win_x64/"
        echo "    [Windows] Staged win_x64 libraries"
    fi

    # Linux: copy lin_x64 if present
    if [[ -f "$FWL_SRC/lin_x64/FlyWithLua.xpl" ]]; then
        cp -a "$FWL_SRC/lin_x64/FlyWithLua.xpl" "$STAGING_DIR/lin_x64/FlyWithLua.xpl"
        echo "    [Linux] Staged lin_x64 binary"
    fi

    # 4. Copy SkunkCrafts configuration & control lists
    cp -a "$CHANNEL_CFG" "$STAGING_DIR/skunkcrafts_updater_config.txt"
    cp -a "$REPO_ROOT/deploy/skunkcrafts_updater_oncelist.txt" "$STAGING_DIR/"
    cp -a "$REPO_ROOT/deploy/skunkcrafts_updater_blacklist.txt" "$STAGING_DIR/"
    cp -a "$REPO_ROOT/deploy/skunkcrafts_updater_ignorelist.txt" "$STAGING_DIR/"

    echo "==> Staging complete at: $STAGING_DIR"

    # 5. Build depot manifests via xpdepot.py
    if [[ "$BUILD_DEPOT" == true ]]; then
        local DEPOT_DIR="$REPO_ROOT/build/depot/depot/$CHANNEL/FlyWithLua"
        echo "==> Generating depot manifests with xpdepot.py for channel: $CHANNEL..."
        python3 "$SCRIPT_DIR/xpdepot.py" --stage "$STAGING_DIR" --depot "$DEPOT_DIR"
    fi

    # 6. Package distribution zip
    if [[ "$PACKAGE_ZIP" == true ]]; then
        local DIST_DIR="$REPO_ROOT/build/dist"
        mkdir -p "$DIST_DIR"

        local VER="2.8.22"
        if [[ -f "$STAGING_DIR/skunkcrafts_updater.cfg" ]]; then
            VER=$(grep "^version|" "$STAGING_DIR/skunkcrafts_updater.cfg" | cut -d'|' -f2 || echo "2.8.22")
        fi

        local ZIP_NAME="FlyWithLua_${VER}_${CHANNEL}.zip"
        echo "==> Packaging standalone zip: $DIST_DIR/$ZIP_NAME..."
        (cd "$REPO_ROOT/build/staging/$CHANNEL" && zip -rq "$DIST_DIR/$ZIP_NAME" "FlyWithLua")
        echo "    Created: $DIST_DIR/$ZIP_NAME ($(du -h "$DIST_DIR/$ZIP_NAME" | cut -f1))"
    fi
}

if [[ "$TARGET" == "all" ]]; then
    stage_channel "release"
    stage_channel "beta"
else
    stage_channel "$TARGET"
fi

echo "==> Done!"
