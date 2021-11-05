cmake_minimum_required(VERSION 3.9)
project(luajit DESCRIPTION "LuaJIT is a Just-In-Time (JIT) compiler for the Lua programming language.")

include(ExternalProject)

set (luajit_version $ENV{LUAJIT_VERSION})
if ("${lua_version}" STREQUAL "")
  set(luajit_version 2.0.5)
endif()

set(build_options "")
if (NOT CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
  # We are cross-compiling: set appropriate toolchain options for LuaJIT.
  if (WIN32)
    set(build_options "CC=gcc" "CROSS=${TOOLCHAIN_PREFIX}-" "TARGET_SYS=Windows")
  elseif (APPLE)
    set(build_options "CC=clang" "CROSS=${TOOLCHAIN_PREFIX}-" "TARGET_SYS=Darwin")
  endif ()
endif()

set(luajit_prefix ${CMAKE_CURRENT_BINARY_DIR}/luajit-${luajit_version})

find_package(Patch REQUIRED)
find_program(Make_EXECUTABLE NAMES make mingw32-make REQUIRED)

ExternalProject_Add(project_luajit
  URL http://luajit.org/download/LuaJIT-${luajit_version}.tar.gz
  PREFIX ${luajit_prefix}
  CONFIGURE_COMMAND ""
  PATCH_COMMAND ${Patch_EXECUTABLE} -p1 -t -N < ${CMAKE_CURRENT_SOURCE_DIR}/x-plane-luajit.patch
  BUILD_COMMAND ${CMAKE_COMMAND} -E env MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} ${Make_EXECUTABLE} -j ${build_options}
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND ""
  BUILD_BYPRODUCTS ${luajit_prefix}/lib/libluajit-5.1.a
)

ExternalProject_Get_Property(project_luajit install_dir source_dir)

add_custom_command(
  TARGET project_luajit POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
          ${luajit_prefix}/src/project_luajit/src/libluajit.a
          ${install_dir}/lib/libluajit-5.1.a)

add_library(LuaJIT STATIC IMPORTED GLOBAL)
add_library(luajit::LuaJIT ALIAS LuaJIT)
add_dependencies(LuaJIT project_luajit)
set_property(TARGET LuaJIT PROPERTY IMPORTED_LOCATION "${install_dir}/lib/libluajit-5.1.a")
target_include_directories(LuaJIT INTERFACE "${source_dir}/src")
file(MAKE_DIRECTORY "${source_dir}/src")  # CMake doesn't like if include dir doesn't exist at configure time.