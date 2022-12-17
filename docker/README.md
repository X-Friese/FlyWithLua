# Xbtn2cmd cross-compiling environment

Scripts in this directory provide an easy way to build Xbtn2cmd for Windows,
Linux and OS X in a reproducible way. This is useful when you want to verify
that your changes work (or at least compile) for all supported platforms
without manually setting up three independent build environments.

The long-term goal is to integrate this seamlessly with host-native CMake and
IDEs. For now it just simplifies setting things up and keeps your host system
free of all the extra compilers and libraries for all platforms.

## Prerequisites

  1. [Docker](https://docs.docker.com/install/) — provides containers for
     a reproducible build environment independent from the host system.
  2. GNU Make — provides an easy way to build container image and launch build
     processes.

## Usage

Let's assume we're in a root directory of Git repository:

    $ cd $Xbtn2cmd_REPO

Build Xbtn2cmd binaries for all platforms:

    $ (cd docker; make)
    $ ls src/build-*/*.xpl
    src/build-lin/lin.xpl src/build-mac/mac.xpl src/build-win/win.xpl 

Build for a specific platform (`lin`, `mac` or `win`):

    $ (cd docker; make lin)

Start bash shell inside container, useful for various debugging needs or to
access platform-specific tools:

    $ (cd docker; make bash)
