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
INCLUDEPATH += ../src/Fmod_api/core/inc
INCLUDEPATH += ../src/Fmod_api/studio/inc


INCLUDEPATH += ..
VPATH = ..

# Defined to use X-Plane SDK 4.0.0 capabilities - no backward compatibility before 12.00
DEFINES +=  XPLM400 \
            XPLM303 \
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


HEADERS +=  ../src/SDK/CHeaders/XPLM/XPLMCamera.h \
            ../src/SDK/CHeaders/XPLM/XPLMDataAccess.h \
            ../src/SDK/CHeaders/XPLM/XPLMDefs.h \
            ../src/SDK/CHeaders/XPLM/XPLMDisplay.h \
            ../src/SDK/CHeaders/XPLM/XPLMGraphics.h \
            ../src/SDK/CHeaders/XPLM/XPLMInstance.h \
            ../src/SDK/CHeaders/XPLM/XPLMMap.h \
            ../src/SDK/CHeaders/XPLM/XPLMMenus.h \
            ../src/SDK/CHeaders/XPLM/XPLMNavigation.h \
            ../src/SDK/CHeaders/XPLM/XPLMPlanes.h \
            ../src/SDK/CHeaders/XPLM/XPLMPlugin.h \
            ../src/SDK/CHeaders/XPLM/XPLMProcessing.h \
            ../src/SDK/CHeaders/XPLM/XPLMScenery.h \
            ../src/SDK/CHeaders/XPLM/XPLMSound.h \
            ../src/SDK/CHeaders/XPLM/XPLMUtilities.h \
            ../src/SDK/CHeaders/XPLM/XPLMWeather.h \
            ../src/FlyWithLua.h \
            ../src/XSBComsDefs.h \
            ../src/FloatingWindows/FloatingWindow.h \
            ../src/FloatingWindows/FLWIntegration.h \
            ../src/FloatingWindows/ImGUIIntegration.h \
            ../src/FloatingWindows/stb_image.h \
            ../src/hidapi/hidapi.h \
            ../src/imgui/imconfig.h \
            ../src/imgui/imgui.h \
            ../src/imgui/imgui_internal.h \
            ../src/imgui/imstb_rectpack.h \
            ../src/imgui/imstb_textedit.h \
            ../src/imgui/imstb_truetype.h \
            ../src/imgui/imgui_iterator.inl \
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
            ../src/Fmod/FmodIntegration.h \
            ../src/Fmod_api/core/inc/fmod.h \
            ../src/Fmod_api/core/inc/fmod_codec.h \
            ../src/Fmod_api/core/inc/fmod_common.h \
            ../src/Fmod_api/core/inc/fmod_dsp.h \
            ../src/Fmod_api/core/inc/fmod_dsp_effects.h \
            ../src/Fmod_api/core/inc/fmod_errors.h \
            ../src/Fmod_api/core/inc/fmod_output.h \
            ../src/Fmod_api/studio/inc/fmod_studio.h \
            ../src/Fmod_api/studio/inc/fmod_studio_common.h


SOURCES +=  ../src/FlyWithLua.cpp \
            ../src/FloatingWindows/FLWIntegration.cpp \
            ../src/FloatingWindows/ImGUIIntegration.cpp \
            ../src/FloatingWindows/FloatingWindow.cpp \
            ../src/hidapi/linux/hid.c \
            ../src/imgui/imgui.cpp \
            ../src/imgui/imgui_demo.cpp \
            ../src/imgui/imgui_draw.cpp \
            ../src/imgui/imgui_lua_bindings.cpp \
            ../src/imgui/imgui_widgets.cpp \
            ../src/imgui/parse_blocks.pl \
            ../src/third_party/iniReader/inireader.cpp \
            ../src/third_party/iniReader/readinifile.cpp \
            ../src/third_party/luajit/lua.hpp \
            ../src/Fmod/FmodIntegration.cpp
