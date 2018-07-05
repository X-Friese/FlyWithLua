/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#include <vector>
#include <memory>
#include <stdexcept>
#include <XPLMDataAccess.h>
#include "FLWIntegration.h"
#include "FloatingWindow.h"
#include "ImGUIIntegration.h"
#include "../FlyWithLua.h"

namespace flwnd {

std::vector<std::shared_ptr<FloatingWindow>> floatingWindows;

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
            return true;
        }

        lua_State *L = flywithlua::FWLLua;

        lua_getglobal(L, cbName.c_str());
        if (!lua_isfunction(L, 1)) {
            lua_pop(L, 1);
            return true;
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
            return true;
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
    floatingWindows.clear();

    lua_State *L = flywithlua::FWLLua;

    lState = L;
    LoadImguiBindings();

    setupImguiIntegration(L);

    lua_register(L, "float_wnd_create", LuaCreateFloatingWindow);
    lua_register(L, "float_wnd_set_title", LuaSetFloatingWindowTitle);
    lua_register(L, "float_wnd_set_ondraw", LuaSetOnDrawCallback);
    lua_register(L, "float_wnd_set_onclick", LuaSetOnClickCallback);
    lua_register(L, "float_wnd_set_onclose", LuaSetOnCloseCallback);
    lua_register(L, "float_wnd_set_imgui_builder", LuaSetImguiBuilder);
    lua_register(L, "float_wnd_get_dimensions", LuaGetFloatingWindowDimensions);
    lua_register(L, "float_wnd_destroy", LuaDestroyFloatingWindow);
}

void deinitFloatingWindowSupport() {
    floatingWindows.clear();
    destroyImguiIntegration();
}

void onFlightLoop() {
    for (auto it = floatingWindows.begin(); it != floatingWindows.end(); ) {
        auto wnd = *it;
        if (!wnd->isVisible()) {
            wnd->reportClose();
            it = floatingWindows.erase(it);
        } else {
            wnd->moveFromOrToVR();
            ++it;
        }
    }
}

} // namespace flwnd
