# Sample toolchain file for building for Mac OS X from an Ubuntu Linux system.
#
# Typical usage:
#    *) install cross compiler from https://github.com/tpoechtrager/osxcross
#    *) mkdir build
#    *) cd build
#    *) cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/ubuntu-osxcross-10.7.cmake ..

set(CMAKE_SYSTEM_NAME Darwin)
set(TOOLCHAIN_PREFIX x86_64-apple-darwin15)

# cross compilers to use for C and C++
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-clang)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-clang++)


# target environment on the build host system
#   set 1st to dir with the cross compiler's C/C++ headers/libs
if (NOT DEFINED CMAKE_FIND_ROOT_PATH)
  set(CMAKE_FIND_ROOT_PATH "~/X-Plane_OsxCrossBuild/osxcross/target/SDK/MacOSX10.11.sdk/usr/")
endif()


# modify default behavior of FIND_XXX() commands to
# search for headers/libs in the target environment and
# search for programs in the build host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
