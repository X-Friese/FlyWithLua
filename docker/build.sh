#!/bin/bash
set -e

function build() {
  local src_dir="$1"
  local complete_edition="$2"
  local platform="$3"
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

  flags+=("-DCREATECOMPLETEEDITION=$complete_edition")

  (
    export PATH="$PATH:/usr/osxcross/bin"
    mkdir -p "$build_dir" && cd "$build_dir"
    "$cmake" -G Ninja "${flags[@]}" ..
    ninja -v
  )
}

complete_edition=1
if [[ "$1" == "--complete" ]]; then
  complete_edition="$2"
  shift 2
fi

src_dir="$(pwd)"
for platform in $@; do
  build "$src_dir" "$complete_edition" "$platform"
done
