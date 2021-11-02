# FlyWithLua build script.
#
# Useful command line flags:
#   * -DCMAKE_BUILD_TYPE=XXX, where XXX is one of Debug, Release,
#     RelWithDebInfo and MinSizeRel. Default is Release.
#   * -DCREATECOMPLETEEDITION=XXX, where XXX is true or false. Default is true.

cmake_minimum_required(VERSION 3.9)
project(FlyWithLua VERSION 2.7.5 DESCRIPTION "FlyWithLua X-Plane plugin")

# X-Plane supports OS X 10.10+, so this should ensure FlyWithLua can run on
# all supported versions.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10")

# By default, use optimized release configuration.
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

set(CMAKE_CXX_STANDARD 14)

# Set include directories used by our code and dependencies.
include_directories("${CMAKE_SOURCE_DIR}/XPSDK301/CHeaders/Widgets")
include_directories("${CMAKE_SOURCE_DIR}/XPSDK301/CHeaders/Wrappers")
include_directories("${CMAKE_SOURCE_DIR}/XPSDK301/CHeaders/XPLM")
include_directories("${CMAKE_SOURCE_DIR}/include64")
include_directories("${CMAKE_SOURCE_DIR}/hidapi")
include_directories("${CMAKE_SOURCE_DIR}/OpenAL")
include_directories("${CMAKE_SOURCE_DIR}/.")
include_directories("${CMAKE_SOURCE_DIR}/FloatingWindows")
include_directories("${CMAKE_SOURCE_DIR}/imgui")

if (WIN32)
    include_directories("${CMAKE_SOURCE_DIR}/GLUT_for_Windows")
endif (WIN32)


# Specify library search locations.
if (WIN32)
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/include64")
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/GLUT_for_Windows/GL")
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/XPSDK301/Libraries/Win")
elseif (APPLE)
    list(APPEND CMAKE_FRAMEWORK_PATH "${CMAKE_SOURCE_DIR}/XPSDK301/Libraries/Mac")
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/mac_libs")
elseif (UNIX)
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/linux_libs/OpenAL")
endif ()


# Enable all X-Plane SDK APIs up to the newest version.
add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1)
# Define platform macros.
add_definitions(-DAPL=$<BOOL:${APPLE}> -DIBM=$<BOOL:${WIN32}> -DLIN=$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>)

# Allow toggling between complete and standard editions via CMake flag, e.g.:
# $ cmake -DCREATECOMPLETEEDITION=true ..
set(CREATECOMPLETEEDITION true CACHE BOOL "Enable Complete Edition extensions.")
if (CREATECOMPLETEEDITION)
    add_definitions(-DCREATECOMPLETEEDITION)
endif ()

# Force-enable exception support. This is most likely redundant, although for C
# code the default is the opposite. Since we are mixing C++ and C libraries,
# safer to set it on?
add_compile_options(-fexceptions)
# On UNIX systems this makes symbols non-exported by default. On Windows this
# option is simply ignored, since symbol visibility works differently there.
add_compile_options(-fvisibility=hidden)

# Enable stricter warnings and then disable some we are not interested in.
add_compile_options(-Wall -Wpedantic -Wshadow -Wfloat-equal -Wextra)
add_compile_options(-Wno-unused)

# Always use position-independent code and highest optimization level (FPS!).
add_compile_options(-O3 -fPIC)

# Turn on Debug mode
# add_compile_options(-g)

# X-Plane plugin
# FIXME: Split this into individual targets.
list(APPEND FLYWITHLUA_SRCS FlyWithLua.cpp third_party/iniReader/inireader.cpp third_party/iniReader/readinifile.cpp)
list(APPEND FLYWITHLUA_SRCS FloatingWindows/FLWIntegration.cpp FloatingWindows/ImGUIIntegration.cpp FloatingWindows/FloatingWindow.cpp)
list(APPEND FLYWITHLUA_SRCS imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp imgui/imgui_lua_bindings.cpp)
if (WIN32)
    list(APPEND FLYWITHLUA_SRCS hidapi/windows/hid.c)
elseif (APPLE)
    list(APPEND FLYWITHLUA_SRCS hidapi/mac/hid.c)
elseif (UNIX)
    list(APPEND FLYWITHLUA_SRCS hidapi/linux/hid.c)
endif ()

add_library(FlyWithLua SHARED ${FLYWITHLUA_SRCS})

target_compile_features(FlyWithLua PUBLIC cxx_std_11)

# Link OpenGL and OpenAL related libraries.
find_package(OpenGL REQUIRED)  # apt install freeglut3-dev
find_library(OPENAL_LIBRARY NAMES openal OpenAL OpenAL32)  # apt install libopenal-dev
find_library(GLUT_LIBRARY NAMES glut64 glut)  # apt install freeglut3-dev
target_link_libraries(FlyWithLua ${OPENGL_LIBRARIES} ${OPENAL_LIBRARY} ${GLUT_LIBRARY})

# Link Lua related libraries.
add_subdirectory(third_party/luajit)
add_subdirectory(third_party/luaxml)
add_subdirectory(third_party/luasocket)
target_link_libraries(FlyWithLua LuaXML_lib LuaSocket::Mime LuaSocket::Socket LuaSocket::Unix LuaSocket::Serial luajit::LuaJIT)

# Add Sol2 library for C++ <-> Lua bindings
add_subdirectory(third_party/sol2)
target_link_libraries(FlyWithLua sol2::sol2)

# Link X-Plane plugin system libraries. They are only provided for OS X and Windows.
if (WIN32 OR APPLE)
    find_library(XPLM_LIBRARY NAMES XPLM XPLM_64.lib)
    find_library(XPWIDGETS_LIBRARY NAMES XPWidgets XPWidgets_64.lib)
    target_link_libraries(FlyWithLua ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY})
endif ()

# Link OS-specific libraries HidApi depends on. Only supported for Windows and Linux.
if ((WIN32 OR UNIX) AND NOT APPLE)
    # HINTS is required to force CMake to search among the toolchain-provided libraries.
    # This is important to make sure that the correct library is picked up on a system with
    # several toolchains installed.
    find_library(HIDAPIDEPS_LIBRARY NAMES setupapi udev HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})
    target_link_libraries(FlyWithLua ${HIDAPIDEPS_LIBRARY})
endif ()

# Link library for dynamic loading of shared objects on UNIX systems.
if (UNIX)
    find_library(DL_LIBRARY dl)
    target_link_libraries(FlyWithLua ${DL_LIBRARY})
endif ()

# Link OS X core system libraries.
if (APPLE)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(IOKIT_LIBRARY IOKit)
    target_link_libraries(FlyWithLua ${IOKIT_LIBRARY} ${CORE_FOUNDATION_LIBRARY})
endif ()

if (WIN32)
    # Unlike OS X and Linux we build standard libraries statically since X-Plane does not provide them.
    target_link_libraries(FlyWithLua -static-libgcc -static-libstdc++)
    # MSYS2 for some reason always links against winpthread, so we have to work around it by statically linking the
    # entire plugin. This allows building the plugin nativaly on Windows.
    target_link_libraries(FlyWithLua -static)
elseif (APPLE)
    # Restrict set of symbols exported from the plugin. This reduces changes of
    # conflict with other plugins, in particular ones with Lua interpreter
    # embedded.
    target_link_libraries(FlyWithLua "-exported_symbols_list ${CMAKE_SOURCE_DIR}/flywithlua.sym_mac")
elseif (UNIX)
    # Specify additional runtime search laths for dynamically-linked libraries.
    target_link_libraries(FlyWithLua -Wl,-rpath=./Resources/plugins/FlyWithLua/64)
    target_link_libraries(FlyWithLua -Wl,-rpath=./Resources/plugins/FlyWithLua/Modules)
    # Restrict set of symbols exported from the plugin. This reduces chances of
    # conflict with other plugins, in particular ones with Lua interpreter
    # embedded.
    target_link_libraries(FlyWithLua -Wl,--version-script -Wl,${CMAKE_SOURCE_DIR}/flywithlua.sym)
endif ()


set_target_properties(FlyWithLua PROPERTIES PREFIX "")
if (WIN32)
    set_target_properties(FlyWithLua PROPERTIES OUTPUT_NAME "win")
elseif (APPLE)
    set_target_properties(FlyWithLua PROPERTIES OUTPUT_NAME "mac")
elseif (UNIX)
    set_target_properties(FlyWithLua PROPERTIES OUTPUT_NAME "lin")
endif ()
set_target_properties(FlyWithLua PROPERTIES SUFFIX ".xpl")
