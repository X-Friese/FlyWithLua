# FlyWithLua for X-Plane 11

[![CMake](https://github.com/X-Friese/FlyWithLua/actions/workflows/cmake.yml/badge.svg)](https://github.com/X-Friese/FlyWithLua/actions/workflows/cmake.yml)

This is the official source code repository for the FlyWithLua plugin project.

FlyWithLua offers Lua scripting to X-Plane since X-Plane 9.

Discussions to general topics should be opened on the [official forum at x-plane.org](http://forums.x-plane.org/index.php?showforum=188).

You will find a binary version ready to use in X-Plane 9 or 10 in the [download area of x-plane.org](http://forums.x-plane.org/index.php?app=downloads&showfile=17468).

The new versions are X-Plane 11 only and the binary is also in the [download area of x-plane.org](https://forums.x-plane.org/index.php?/files/file/38445-flywithlua-ng-next-generation-edition-for-x-plane-11-win-lin-mac/).

If you want to grab a most nightly binary, just download `FlyWithLua_plugin.zip` from the most recent [GitHub Actions build](https://github.com/X-Friese/FlyWithLua/actions/workflows/cmake.yml?query=branch%3Amaster) and unpack it into your X-Plane's plugin folder.

> Copyright (c) 2012 Carsten Lynker
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software
> and associated documentation files (the "Software"), to deal in the Software without restriction,
> including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
> and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
> subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
> TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
> THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
> CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
> DEALINGS IN THE SOFTWARE.

## Building FlyWithLua from source

### On windows

1.  Install required software using [Chocolatey](https://chocolatey.org/) using admin command prompt:

    ```
    choco install git cmake
    choco install mingw --version 8.1.0
    ```

    You can also install the same programs manually if you prefer.

2.  Checkout and configure the project:

    ```
    git clone https://github.com/X-Friese/FlyWithLua.git
    cd FlyWithLua
    cmake -G "MinGW Makefiles" -S .\src -B .\build -DCMAKE_BUILD_TYPE=RelWithDebInfo
    ```

3.  Build the project and copy the plugin DLL into the appropriate directory:

    ```
    cmake --build .\build
    cp .\build\win.xpl .\FlyWithLua\64\
    ```

### On Ubuntu:

1. Install required software:

   ```
   sudo apt-get install -y --no-install-recommends build-essential cmake git freeglut3-dev libudev-dev libopenal-dev

   ```

2. Checkout and configure the project:

   ```
   git clone https://github.com/X-Friese/FlyWithLua.git
   cd FlyWithLua
   cmake -S ./src -B ./build -DCMAKE_BUILD_TYPE=RelWithDebInfo
   ```

3. Build the project and copy the plugin DLL into the appropriate directory:

   ```
   cmake --build ./build
   mkdir ./FlyWithLua/64
   cp ./build/lin.xpl ./FlyWithLua/64/
   ```

### On OS X:

1. Install XCode, Git, CMake (Homebrew can be convenient for this).

2. Checkout and configure the project:

   ```
   git clone https://github.com/X-Friese/FlyWithLua.git
   cd FlyWithLua
   cmake -S ./src -B ./build -DCMAKE_BUILD_TYPE=RelWithDebInfo
   ```

3. Build the project and copy the plugin DLL into the appropriate directory:

   ```
   cmake --build ./build
   mkdir ./FlyWithLua/64
   cp ./build/mac.xpl ./FlyWithLua/64/
   ```

### Additional tips

<details>
<summary>How to cross-compile FlyWithLua on Ubuntu for Windows and OS X</summary>

Note: this is documented just in case, but generally is not recommended. Using native builds and/or GitHub Actions workflow is more convenient and less prone to errors and bugs.

Install cross-compiling toolchains:

```
# Install Windows cross-compiling toolchain (MinGW).
sudo apt-get install -y --no-install-recommends mingw-w64 g++-mingw-w64 && apt-get clean

# Install OS X cross-compiling toolchain (clang).
apt-get install -y --no-install-recommends clang curl && apt-get clean
#Build arguments
OSXCROSS_REPO="tpoechtrager/osxcross"
OSXCROSS_REVISION="f4ba4facae996b3b14d89eb62c0384564f7368b5"
DARWIN_SDK_VERSION="10.11"
DARWIN_SDK_URL="https://github.com/apriorit/osxcross-sdks/raw/master/MacOSX10.11.sdk.tar.xz"
CROSSBUILD=1

mkdir -p "/tmp/osxcross" \
 && cd "/tmp/osxcross" \
 && curl -sSLo osxcross.tar.gz "https://codeload.github.com/${OSXCROSS_REPO}/tar.gz/${OSXCROSS_REVISION}" \
 && tar --strip=1 -xzf osxcross.tar.gz \
 && rm -f osxcross.tar.gz \
 && curl -sLo tarballs/MacOSX${DARWIN_SDK_VERSION}.sdk.tar.xz \
             "${DARWIN_SDK_URL}" \
 && UNATTENDED=1 JOBS=4 SDK_VERSION=${DARWIN_SDK_VERSION} ./build.sh \
 && mv target /usr/osxcross \
 && mv tools /usr/osxcross/ \
 && rm -rf "/usr/osxcross/SDK/MacOSX${DARWIN_SDK_VERSION}.sdk/usr/share/man"

export PATH="$PATH:/usr/osxcross/bin"
```

Build FlyWithLua:

```
git clone https://github.com/X-Friese/FlyWithLua.git
cd FlyWithLua

# For Windows:
cmake -S ./src -B ./build-win -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE=src/Toolchain-mingw-w64-x86-64.cmake
cmake --build ./build-win

# For OS X:
cmake -S ./src -B ./build-mac -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE=src/Toolchain-ubuntu-osxcross-10.11.cmake -DCMAKE_FIND_ROOT_PATH=/usr/osxcross/SDK/MacOSX10.11.sdk/
cmake --build ./build-mac
```

</details>

<details>
<summary>How to create Code Blocks Project From CmakeLists.txt File</summary>

- First you will need to install MinGW-64, Cmake, CodeBlocks and Git.
- Create a CodeBlocks Projects folder like this.
- `mkdir CodeBlocksProjects`
- `cd D:\CodeBlocksProjects`
- `git clone https://github.com/X-Friese/FlyWithLua.git`
- `cd FlyWithLua/src`
- `mkdir build`
- `cd build`
- `cmake -G "CodeBlocks - MinGW Makefiles" ..`
- Open CodeBlocks and use "Open an existing project" browsing to "FlyWithLua/src/build/FlyWithLua.cbp" and click on the Open button.

At that point you should be able to build FlwWithLua.

</details>

<details>
<summary>How To Build Using QT Creator From the CmakeLists.txt File</summary>

- Create a QT Creator Projects folder like this.
  - `mkdir QTCreatorProjects`
  - `cd D:\QTCreatorProjects`
  - `git clone https://github.com/X-Friese/FlyWithLua.git`
- Using QT Creator use "File" > "Open File or Project" and browse to "FlyWithLua/src/CMakeLists.txt" then click on the "Open" button.

At that point you should be able to build FlyWithLua.

</details>
