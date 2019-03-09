cmake_minimum_required (VERSION 3.0)
project(PPL VERSION 2.0.0 DESCRIPTION "Plugin Patterns Library for X-Plane 11")

# set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PPL_BINARY_DIR}/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PPL_BINARY_DIR}/)

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PPL_BINARY_DIR}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PPL_BINARY_DIR}/lib)

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

if(UNIX AND NOT APPLE)
set(LINUX TRUE)
endif()

if(UNIX)
    set(PLATFORM_COMPILE_OPTIONS -Wall -Wextra -Wfloat-equal -pedantic)
else()
    set(PLATFORM_COMPILE_OPTIONS -Wall -std=c++0x)
#    set(PLATFORM_COMPILE_OPTIONS /W3 /wd4996)
endif()

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.11)
#    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
    set(PLATFORM_CORE_DEFINITIONS
        APL=1
        LIN=0
        IBM=0)
    set(PLATFORM_INCLUDE_DIRECTORIES
        /usr/X11/include/freetype2)
else()
if(LINUX)
    set(PLATFORM_CORE_DEFINITIONS
        APL=0
        LIN=1
        IBM=0)
    set(PLATFORM_INCLUDE_DIRECTORIES
        /usr/include/freetype2)
else()
    set(PLATFORM_CORE_DEFINITIONS
        APL=0
        LIN=0
        IBM=1
        FREETYPE2_STATIC
#        _USE_MATH_DEFINES
#        NOMINMAX
#        WIN32_LEAN_AND_MEAN
        GLEW_STATIC=1)
    set(PLATFORM_INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/upstream/include/glew/include
        ${CMAKE_CURRENT_SOURCE_DIR}/upstream/include/openal-soft/include
        ${CMAKE_CURRENT_SOURCE_DIR}/upstream/include/freetype2/include)
#    set(PLATFORM_DEBUG_OPTIONS "-MTd")
#    set(PLATFORM_RELEASE_OPTIONS "-MT")
endif()
endif()

set(DEFINITIONS
    XPLM200=1
    XPLM210=1
    XPLM300=1
    XPLM301=1
    ${PLATFORM_CORE_DEFINITIONS})

include_directories(${CMAKE_SOURCE_DIR}/XPSDK301/CHeaders/XPLM
                    ${CMAKE_SOURCE_DIR}/XPSDK301/CHeaders/Widgets
                    ${CMAKE_CURRENT_SOURCE_DIR}/upstream/include/simpleini
                    ${CMAKE_CURRENT_SOURCE_DIR}/upstream/include/freetype2/include
                    ${PLATFORM_INCLUDE_DIRECTORIES})

set(SOURCES
    upstream/src/pluginpath.cpp
    upstream/src/settings.cpp
#    upstream/src/dataref.cpp
#    upstream/src/messagewindow.cpp
#    upstream/src/onscreendisplay.cpp
#    upstream/src/owneddata.cpp
#    upstream/src/logichandler.cpp
#    upstream/src/texture.cpp # Not building on Windows
#    upstream/src/overlaygauge.cpp # Not building on Linux
    upstream/src/log.cpp
    upstream/src/logwriter.cpp)
#    upstream/src/menuitem.cpp
#    upstream/src/processor.cpp
#    upstream/src/vertexbuffer.cpp) # Not building on Linux
#    upstream/src/alsoundbuffer.cpp
#    upstream/src/alcontextmanager.cpp) # Not building on Mac
#    upstream/src/alcontextchanger.cpp
#    upstream/src/fontmgr.cpp)

add_library(PPL STATIC ${SOURCES})
target_compile_options(PPL PUBLIC ${PLATFORM_COMPILE_OPTIONS})
target_compile_options(PPL PUBLIC "$<$<CONFIG:DEBUG>:${PLATFORM_DEBUG_OPTIONS}>")
target_compile_options(PPL PUBLIC "$<$<CONFIG:RELEASE>:${PLATFORM_RELEASE_OPTIONS}>")
target_compile_definitions(PPL PRIVATE ${DEFINITIONS})
