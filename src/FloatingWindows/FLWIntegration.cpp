/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#include <vector>
#include <memory>
#include <stdexcept>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <XPLMDataAccess.h>
#include "FLWIntegration.h"
#include "FloatingWindow.h"
#include "ImGUIIntegration.h"
#include "../FlyWithLua.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace flwnd {

std::vector<std::shared_ptr<FloatingWindow>> floatingWindows;
std::vector<GLuint> textureIDs;

int loadImage(const std::string&fileName) {
    int imgWidth, imgHeight, nComps;
    uint8_t *data = stbi_load(fileName.c_str(), &imgWidth, &imgHeight, &nComps, sizeof(uint32_t));

    if (!data) {
        throw std::runtime_error(std::string("Couldn't load image: ") + stbi_failure_reason());
    }

    int id;
    XPLMGenerateTextureNumbers(&id, 1);
    XPLMBindTexture2d(id, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, imgWidth, imgHeight, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    textureIDs.push_back(id);

    return id;
}

int LuaCreateFloatingWindow(lua_State *L) {
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isboolean(L, 4)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: wrong arguments given to float_wnd_create.");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    int width = lua_tointeger(L, 1);
    int height = lua_tointeger(L, 2);
    int decoration = lua_tointeger(L, 3);
    bool withImgui = lua_toboolean(L, 4);

    std::shared_ptr<FloatingWindow> wnd;

    try {
        if (withImgui) {
            wnd = std::make_shared<ImGUIWindow>(width, height, decoration);
        } else {
            wnd = std::make_shared<FloatingWindow>(width, height, decoration);
        }
        floatingWindows.push_back(wnd);
        lua_pushlightuserdata(L, wnd.get());
        return 1;
    } catch (const std::exception &e) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Couldn't create floating window");
        flywithlua::LuaIsRunning = false;
        return 0;
    }
}

int LuaDestroyFloatingWindow(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: wrong arguments given to float_wnd_destroy.");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    for (auto wndPtr: floatingWindows) {
        if (wndPtr.get() == wnd) {
            // this will lead to actual destruction in the flightloop callback
            wndPtr->setVisible(false);
        }
    }

    return 0;
}

int LuaSetFloatingWindowTitle(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isstring(L, 2)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_title");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    wnd->setTitle(lua_tostring(L, 2));

    return 0;
}

int LuaSetFloatingWindowPosition(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isstring(L, 2)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_position");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    // Not sure what the correct syntxt is needed here.
    wnd->setPosition(lua_tonumber(L, 2), lua_tonumber(L, 3));

    return 0;
}

int LuaLoadFloatinWindowImage(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_load_image");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    std::string name = lua_tostring(L, 1);
    try {
        int id = loadImage(name);
        lua_pushinteger(L, id);
        return 1;
    } catch (const std::exception &e) {
        std::string err = std::string("FlyWithLua Error: Couldn't load image: ") + e.what() + " in " + name;
        flywithlua::logMsg(logToAll, err);
        lua_pushstring(flywithlua::FWLLua, err.c_str());
        flywithlua::LuaIsRunning = false;
        return 0;
    }
}

int LuaSetOnDrawCallback(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isstring(L, 2)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_ondraw");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    std::string cbName = lua_tostring(L, 2);
    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    wnd->setDrawCallback([cbName] (FloatingWindow &fwnd) {
        if (!flywithlua::LuaIsRunning)  {
            return;
        }

        lua_State *L = flywithlua::FWLLua;

        lua_getglobal(L, cbName.c_str());
        if (!lua_isfunction(L, 1)) {
            lua_pop(L, 1);
            return;
        }

        XPLMWindowID window = fwnd.getXWindow();

        int left, top, right, bottom;
        XPLMGetWindowGeometry(window, &left, &top, &right, &bottom);

        lua_pushlightuserdata(L, &fwnd);
        lua_pushnumber(L, left);
        lua_pushnumber(L, bottom);

        XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0);

        flywithlua::WeAreNotInDrawingState = false;
        flywithlua::CopyDataRefsToLua();
        if (lua_pcall(L, 3, 0, 0)) {
            flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute floating window draw callback");
            flywithlua::LuaIsRunning = false;
            return;
        }
        flywithlua::CopyDataRefsToXPlane();
        flywithlua::WeAreNotInDrawingState = true;
    });

    return 0;
}

int LuaGetXPLMWindowHandle(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute float_wnd_get_xplm_handle.");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    XPLMWindowID window = wnd->getXWindow();
    lua_pushlightuserdata(L, window);
    return 1;
}

int LuaGetFloatingWindowDimensions(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_get_dimensions");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    auto xwnd = wnd->getXWindow();

    int left, top, right, bottom;
    XPLMGetWindowGeometry(xwnd, &left, &top, &right, &bottom);

    lua_pushnumber(L, right - left);
    lua_pushnumber(L, top - bottom);

    return 2;
}

int LuaFloatingWindowGetVisible(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute float_wnd_get_visible");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    lua_pushboolean(L, wnd->isVisible());

    return 1;
}

int LuaFloatingWindowSetVisible(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_visible");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

	wnd->setIsCmdVisible(lua_tonumber(L, 2));
    wnd->setVisible(lua_tonumber(L, 2));

    return 0;
}

int LuaFloatingWindowIsPopped(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute float_wnd_is_popped");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    lua_pushboolean(L, wnd->isPopped());

    return 1;
}

int LuaFloatingWindowIsFront(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute float_wnd_is_front");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    lua_pushboolean(L, wnd->isFront());

    return 1;
}

int LuaFloatingWindowBringToFront(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute float_wnd_bring_to_front");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->bringToFront();

    return 0;
}

int LuaSetFloatingWindowResizingLimits(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_resizing_limits");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->setResizingLimits(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));

    return 0;
}

int LuaSetFloatingWindowPositioningMode(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_positioning_mode");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->setPositioningMode(lua_tonumber(L, 2), lua_tonumber(L, 3));

    return 0;
}

int LuaSetFloatingWindowGravity(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_gravity");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->setGravity(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));

    return 0;
}

int LuaSetFloatingWindowGeometry(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_geometry");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->setWindowGeometry(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));

    return 0;
}

int LuaGetFloatingWindowGeometry(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute float_wnd_get_geometry");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    XPLMWindowID window = wnd->getXWindow();

    int sLeft, sTop, sRight, sBottom;
    XPLMGetWindowGeometry(window, &sLeft, &sTop, &sRight, &sBottom);

    lua_pushnumber(L, sLeft);
    lua_pushnumber(L, sTop);
    lua_pushnumber(L, sRight);
    lua_pushnumber(L, sBottom);

    return 4;
}

int LuaSetFloatingWindowGeometryOS(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_geometry_os");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->setWindowGeometryOS(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));

    return 0;
}

int LuaGetFloatingWindowGeometryOS(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute float_wnd_get_geometry_os");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    XPLMWindowID window = wnd->getXWindow();

    int sLeft, sTop, sRight, sBottom;
    XPLMGetWindowGeometryOS(window, &sLeft, &sTop, &sRight, &sBottom);

    lua_pushnumber(L, sLeft);
    lua_pushnumber(L, sTop);
    lua_pushnumber(L, sRight);
    lua_pushnumber(L, sBottom);

    return 4;
}

int LuaSetFloatingWindowGeometryVR(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_geometry_vr");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->setWindowGeometryVR(lua_tonumber(L, 2), lua_tonumber(L, 3));

    return 0;
}

int LuaGetFloatingWindowGeometryVR(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute float_wnd_get_geometry_vr");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    XPLMWindowID window = wnd->getXWindow();

    int gWidth, gHeight;
    XPLMGetWindowGeometryVR(window, &gWidth, &gHeight);

    lua_pushnumber(L, gWidth);
    lua_pushnumber(L, gHeight);

    return 2;
}

int LuaFloatingWindowIsVR(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute float_wnd_is_vr");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    lua_pushboolean(L, wnd->isVR());

    return 1;
}

int LuaSetOnClickCallback(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isstring(L, 2)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_onclick");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    std::string cbName = lua_tostring(L, 2);
    wnd->setClickCallback([cbName] (FloatingWindow &fwnd, int x, int y, XPLMMouseStatus status) {
        if (!flywithlua::LuaIsRunning)  {
            return;
        }

        lua_State *L = flywithlua::FWLLua;

        lua_getglobal(L, cbName.c_str());
        if (!lua_isfunction(L, 1)) {
            lua_pop(L, 1);
            return;
        }

        XPLMWindowID window = fwnd.getXWindow();

        int left, top, right, bottom;
        XPLMGetWindowGeometry(window, &left, &top, &right, &bottom);

        lua_pushlightuserdata(L, &fwnd);
        lua_pushnumber(L, x - left);
        lua_pushnumber(L, y - bottom);
        lua_pushnumber(L, status);

        flywithlua::CopyDataRefsToLua();
        if (lua_pcall(L, 4, 0, 0)) {
            flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute floating window click callback");
            flywithlua::LuaIsRunning = false;
            return;
        }
        flywithlua::CopyDataRefsToXPlane();
    });

    return 0;
}

int LuaSetOnCloseCallback(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isstring(L, 2)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_onclose");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    std::string cbName = lua_tostring(L, 2);
    wnd->setCloseCallback([cbName] (FloatingWindow &fwnd) {
        if (!flywithlua::LuaIsRunning)  {
            return;
        }

        lua_State *L = flywithlua::FWLLua;

        lua_getglobal(L, cbName.c_str());
        if (!lua_isfunction(L, 1)) {
            lua_pop(L, 1);
            return;
        }

        lua_pushlightuserdata(L, &fwnd);

        flywithlua::CopyDataRefsToLua();
        if (lua_pcall(L, 1, 0, 0)) {
            flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute floating window close callback");
            flywithlua::LuaIsRunning = false;
            return;
        }
        flywithlua::CopyDataRefsToXPlane();
    });

    return 0;
}

int LuaSetImguiBuilder(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isstring(L, 2)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_imgui_builder");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    void *ptr = lua_touserdata(L, 1);
    ImGUIWindow *wnd = dynamic_cast<ImGUIWindow *>(reinterpret_cast<FloatingWindow*>(ptr));
    if (!wnd) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong window type passed to float_wnd_set_imgui_builder");
        flywithlua::LuaIsRunning = false;
        return 0;
    }
    std::string cbName = lua_tostring(L, 2);

    wnd->setBuildCallback([cbName] (ImGUIWindow &iwnd) {
        if (!flywithlua::LuaIsRunning)  {
            return;
        }

        lua_State *L = flywithlua::FWLLua;

        lua_getglobal(L, cbName.c_str());
        if (!lua_isfunction(L, 1)) {
            lua_pop(L, 1);
            return;
        }

        int left, top, right, bottom;
        XPLMGetWindowGeometry(iwnd.getXWindow(), &left, &top, &right, &bottom);

        lua_pushlightuserdata(L, &iwnd);
        lua_pushnumber(L, left);
        lua_pushnumber(L, bottom);

        flywithlua::CopyDataRefsToLua();
        if (lua_pcall(L, 3, 0, 0)) {
            flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute imgui window builder callback");
            flywithlua::LuaIsRunning = false;
            return;
        }
        flywithlua::CopyDataRefsToXPlane();
    });

    wnd->setErrorHandler([] (const std::string &errorMsg) {
        flywithlua::logMsg(logToAll, "FlyWithLua imgui error: " + errorMsg);
        lua_pushstring(flywithlua::FWLLua, errorMsg.c_str());
        flywithlua::LuaIsRunning = false;
    });

    return 0;
}

void initFloatingWindowSupport() {
    // destroy old windows
    deinitFloatingWindowSupport();

    lua_State *L = flywithlua::FWLLua;

    lState = L;
    LoadImguiBindings();

    lua_register(L, "float_wnd_create", LuaCreateFloatingWindow);
    lua_register(L, "float_wnd_set_title", LuaSetFloatingWindowTitle);
    lua_register(L, "float_wnd_set_position", LuaSetFloatingWindowPosition);
    lua_register(L, "float_wnd_set_ondraw", LuaSetOnDrawCallback);
    lua_register(L, "float_wnd_set_onclick", LuaSetOnClickCallback);
    lua_register(L, "float_wnd_set_onclose", LuaSetOnCloseCallback);
    lua_register(L, "float_wnd_set_imgui_builder", LuaSetImguiBuilder);
    lua_register(L, "float_wnd_get_xplm_handle", LuaGetXPLMWindowHandle);
    lua_register(L, "float_wnd_get_dimensions", LuaGetFloatingWindowDimensions);
    lua_register(L, "float_wnd_load_image", LuaLoadFloatinWindowImage);
    lua_register(L, "float_wnd_destroy", LuaDestroyFloatingWindow);
	lua_register(L, "float_wnd_get_visible", LuaFloatingWindowGetVisible);
	lua_register(L, "float_wnd_set_visible", LuaFloatingWindowSetVisible);
	lua_register(L, "float_wnd_is_popped", LuaFloatingWindowIsPopped);
	lua_register(L, "float_wnd_is_front", LuaFloatingWindowIsFront);
	lua_register(L, "float_wnd_bring_to_front", LuaFloatingWindowBringToFront);
	lua_register(L, "float_wnd_set_resizing_limits", LuaSetFloatingWindowResizingLimits);
	lua_register(L, "float_wnd_set_positioning_mode", LuaSetFloatingWindowPositioningMode);
	lua_register(L, "float_wnd_set_gravity", LuaSetFloatingWindowGravity);
	lua_register(L, "float_wnd_set_geometry", LuaSetFloatingWindowGeometry);
	lua_register(L, "float_wnd_get_geometry", LuaGetFloatingWindowGeometry);
	lua_register(L, "float_wnd_set_geometry_os", LuaSetFloatingWindowGeometryOS);
	lua_register(L, "float_wnd_get_geometry_os", LuaGetFloatingWindowGeometryOS);
	lua_register(L, "float_wnd_set_geometry_vr", LuaSetFloatingWindowGeometryVR);
	lua_register(L, "float_wnd_get_geometry_vr", LuaGetFloatingWindowGeometryVR);
	lua_register(L, "float_wnd_is_vr", LuaFloatingWindowIsVR);

    lua_pushnumber(L, 1);
    lua_setglobal(L, "SUPPORTS_FLOATING_WINDOWS");
}

void deinitFloatingWindowSupport() {
    floatingWindows.clear();
    if (!textureIDs.empty()) {
        glDeleteTextures(textureIDs.size(), textureIDs.data());
    }
}

void onFlightLoop() {
    for (auto it = floatingWindows.begin(); it != floatingWindows.end(); ) {
        auto wnd = *it;
		if (!wnd->getIsCmdVisible() && !wnd->isVisible()) {
            wnd->reportClose();
            it = floatingWindows.erase(it);
        } else {
            wnd->moveFromOrToVR();
            ++it;
        }
    }
}

} // namespace flwnd
