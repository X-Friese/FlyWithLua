# Shared library without any Qt functionality
TEMPLATE = lib
QT -= gui core

CONFIG += warn_on plugin release
CONFIG -= thread exceptions qt rtti debug

VERSION = 1.0.0

INCLUDEPATH += ../src/SDK/CHeaders/XPLM
INCLUDEPATH += ../src/SDK/CHeaders/Wrappers
INCLUDEPATH += ../src/SDK/CHeaders/Widgets
INCLUDEPATH += ../src/hidapi
INCLUDEPATH += ../src/OpenAL
INCLUDEPATH += ../src/FloatingWindows
INCLUDEPATH += ../src
INCLUDEPATH += ../src/imgui
INCLUDEPATH += ../src/third_party/sol2/upstream
INCLUDEPATH += ../src/third_party/luajit
INCLUDEPATH += ../src/Fmod
INCLUDEPATH += ../src/Fmod_api/inc


INCLUDEPATH += ..
VPATH = ..

# Defined to use X-Plane SDK 3.0.0 capabilities - no backward compatibility before 11.10
DEFINES +=  XPLM303 \
            XPLM302 \
            XPLM301 \
            XPLM300 \
            XPLM210 \
            XPLM200

unix:!macx {
    DEFINES += APL=0 IBM=0 LIN=1 LINUX=1
    TARGET = lin.xpl
    # WARNING! This requires the latest version of the X-SDK !!!!
    QMAKE_CXXFLAGS += -fvisibility=hidden -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2 -std=gnu++11 -fpermissive
    QMAKE_CFLAGS += -fvisibility=hidden -O2 -Wall -Wextra -Wshadow -Wfloat-equal -Wformat -Wformat-security \
         --param ssp-buffer-size=4 -fstack-protector -D_FORTIFY_SOURCE=2
    LIBS += -ldl -Wl,--version-script -Wl,../src/exports.txt
    LIBS += -static-libgcc -static-libstdc++ -fPIC
}


HEADERS +=  ../src/FlyWithLua.h \
            ../src/XSBComsDefs.h \
            ../src/FloatingWindows/FloatingWindow.h \
            ../src/FloatingWindows/FLWIntegration.h \
            ../src/FloatingWindows/ImGUIIntegration.h \
            ../src/FloatingWindows/stb_image.h \
            ../src/hidapi/hidapi.h \
            ../src/imgui/imconfig.h \
            ../src/imgui/imgui.h \
            ../src/imgui/imgui_internal.h \
            ../src/third_party/iniReader/inireader.h \
            ../src/third_party/iniReader/inireader.h \
            ../src/third_party/luajit/lauxlib.h \
            ../src/third_party/luajitr/lua.h \
            ../src/third_party/luajit/luaconf.h \
            ../src/third_party/luajit/luajit.h \
            ../src/third_party/luajit/lualib.h \
            ../src/OpenAL/al.h \
            ../src/OpenAL/alc.h \
            ../src/OpenAL/alext.h \
            ../src/OpenAL/efx.h \
            ../src/OpenAL/efx-creative.h \
            ../src/OpenAL/efx-presets.h \
            ../src/OpenAL/EFX-Util.h \
            ../src/OpenAL/xram.h \
            ../src/Fmod/fwl_customdataref.h \
            ../src/Fmod/fwl_dataref.h \
            ../src/Fmod/fmod4xpchannelgroup.h \
            ../src/Fmod/fmod4xpsystem.h \
            ../src/Fmod/fmod4xpdefaultbus.h \
            ../src/Fmod/FmodIntegration.h


SOURCES +=  ../src/FlyWithLua.cpp \
            ../src/FloatingWindows/FLWIntegration.cpp \
            ../src/FloatingWindows/ImGUIIntegration.cpp \
            ../src/FloatingWindows/FloatingWindow.cpp \
            ../src/hidapi/linux/hid.c \
            ../src/imgui/imgui.cpp \
            ../src/imgui/imgui_demo.cpp \
            ../src/imgui/imgui_draw.cpp \
            ../src/imgui/imgui_widgets.cpp \
            ../src/imgui/imgui_lua_bindings.cpp \
            ../src/third_party/iniReader/inireader.cpp \
            ../src/third_party/iniReader/readinifile.cpp \
            ../src/third_party/luajit/lua.hpp \
            ../src/Fmod/FmodIntegration.cpp
