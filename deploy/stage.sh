#!/usr/bin/env bash
# ==============================================================================
# FlyWithLua Staging & Packaging Tool
# ------------------------------------------------------------------------------
# Assembles the multi-channel SkunkCrafts staging payload for FlyWithLua
# and runs xpdepot.py to compute depot manifests (whitelist, sizeslist, blacklist).
#
# Usage:
#   ./deploy/stage.sh [release|beta|all] [--build-depot]
# ==============================================================================

set -euo pipefail

TARGET="${1:-release}"
BUILD_DEPOT=false

for arg in "$@"; do
    if [[ "$arg" == "--build-depot" ]]; then
        BUILD_DEPOT=true
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

    # Mac: check build/mac_x64/FlyWithLua.xpl first, then legacy build/mac.xpl, fallback to FlyWithLua/mac_x64/FlyWithLua.xpl
    if [[ -f "$REPO_ROOT/build/mac_x64/FlyWithLua.xpl" ]]; then
        cp -a "$REPO_ROOT/build/mac_x64/FlyWithLua.xpl" "$STAGING_DIR/mac_x64/FlyWithLua.xpl"
        echo "    [Mac] Staged local build: build/mac_x64/FlyWithLua.xpl"
    elif [[ -f "$REPO_ROOT/build/mac.xpl" ]]; then
        cp -a "$REPO_ROOT/build/mac.xpl" "$STAGING_DIR/mac_x64/FlyWithLua.xpl"
        echo "    [Mac] Staged legacy build: build/mac.xpl"
    elif [[ -f "$FWL_SRC/mac_x64/FlyWithLua.xpl" ]]; then
        cp -a "$FWL_SRC/mac_x64/FlyWithLua.xpl" "$STAGING_DIR/mac_x64/FlyWithLua.xpl"
        echo "    [Mac] Staged existing binary: FlyWithLua/mac_x64/FlyWithLua.xpl"
    fi

    # Windows: copy win_x64 DLLs and .xpl (check local build first)
    if [[ -d "$FWL_SRC/win_x64" ]]; then
        cp -a "$FWL_SRC/win_x64/"* "$STAGING_DIR/win_x64/"
        echo "    [Windows] Staged win_x64 libraries"
    fi
    if [[ -f "$REPO_ROOT/build/win_x64/FlyWithLua.xpl" ]]; then
        cp -a "$REPO_ROOT/build/win_x64/FlyWithLua.xpl" "$STAGING_DIR/win_x64/FlyWithLua.xpl"
        echo "    [Windows] Staged local build: build/win_x64/FlyWithLua.xpl"
    fi

    # Linux: check build/lin_x64/FlyWithLua.xpl first, fallback to FlyWithLua/lin_x64/FlyWithLua.xpl
    if [[ -f "$REPO_ROOT/build/lin_x64/FlyWithLua.xpl" ]]; then
        cp -a "$REPO_ROOT/build/lin_x64/FlyWithLua.xpl" "$STAGING_DIR/lin_x64/FlyWithLua.xpl"
        echo "    [Linux] Staged local build: build/lin_x64/FlyWithLua.xpl"
    elif [[ -f "$FWL_SRC/lin_x64/FlyWithLua.xpl" ]]; then
        cp -a "$FWL_SRC/lin_x64/FlyWithLua.xpl" "$STAGING_DIR/lin_x64/FlyWithLua.xpl"
        echo "    [Linux] Staged lin_x64 binary"
    fi

    # 4. Stamp resolved build version onto prototype skunkcrafts_updater_config.txt & copy control lists
    local RESOLVED_VER="${FWL_VERSION:-}"
    if [[ -z "$RESOLVED_VER" ]]; then
        RESOLVED_VER=$(grep '#define PLUGIN_VERSION_NO' "$REPO_ROOT/src/FlyWithLua.cpp" | head -n1 | sed -E 's/.*"([^"]+)".*/\1/')
    fi
    if [[ -z "$RESOLVED_VER" ]]; then
        echo "Error: Could not resolve PLUGIN_VERSION_NO from src/FlyWithLua.cpp"
        exit 1
    fi
    sed -E "s/^version\|.*/version|${RESOLVED_VER}/" "$CHANNEL_CFG" > "$STAGING_DIR/skunkcrafts_updater_config.txt"
    echo "    [Version] Stamped version|${RESOLVED_VER} onto prototype skunkcrafts_updater_config.txt"
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
}

if [[ "$TARGET" == "all" ]]; then
    stage_channel "release"
    stage_channel "beta"
else
    stage_channel "$TARGET"
fi

echo "==> Done!"
