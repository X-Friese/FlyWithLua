#!/bin/bash
set -e

function build() {
  local src_dir="$1"
  local platform="$2"
  echo "----------------- Building for $platform -----------------"

  local build_dir="$src_dir/build-$platform"

  local flags=()
  local cmake="cmake"
  case "$platform" in
    lin)
      ;;
    win)
      flags+=('-DCMAKE_TOOLCHAIN_FILE=../Toolchain-mingw-w64-x86-64.cmake')
      ;;
    mac)
      flags+=('-DCMAKE_TOOLCHAIN_FILE=../Toolchain-ubuntu-osxcross-10.11.cmake')
      flags+=('-DCMAKE_FIND_ROOT_PATH=/usr/osxcross/SDK/MacOSX10.11.sdk/')
      ;;
    *)
      echo "Platform $platform is not supported, skipping..."
      return
  esac

  (
    export PATH="$PATH:/usr/osxcross/bin"
    mkdir -p "$build_dir" && cd "$build_dir"
    "$cmake" -G Ninja "${flags[@]}" ..
    ninja -v
  )
}

src_dir="$(pwd)"
for platform in $@; do
  build "$src_dir" "$platform"
done
