# Sample toolchain file for building for Windows from an Ubuntu Linux system.
#
# Typical usage:
#    *) install cross compiler: `sudo apt-get install mingw-w64 g++-mingw-w64`
#    *) cd build
#    *) cmake -DCMAKE_TOOLCHAIN_FILE=~/Toolchain-Ubuntu-mingw32.cmake ..

set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

# Cross compilers to use for C and C++
# Prefer POSIX threading model (winpthreads) required for C++11 std::thread and std::mutex
find_program(MINGW_CXX_POSIX ${TOOLCHAIN_PREFIX}-g++-posix)
if (MINGW_CXX_POSIX)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc-posix)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++-posix)
else ()
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
endif ()
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# target environment on the build host system
#   set 1st to dir with the cross compiler's C/C++ headers/libs
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# Modify default behavior of FIND_XXX() commands to search for headers/libs in
# the target environment and search for programs in the build host environment.
# Note that using "ONLY" causes CMake to fail finding libraries bundled in the
# git repository, since it forcefully prepends CMAKE_FIND_ROOT_PATH to any
# CMAKE_LIBRARY_PATH.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
