/*
 *   Floating Windows for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#include <XPLMGraphics.h>
#include <XPLMDisplay.h>
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <memory>

#include "FloatingWindow.h"

int FloatingWindow::xpIsInVR = false;

FloatingWindow::FloatingWindow(int winWidth, int winHeight, int winDecoration):
    width(winWidth),
    height(winHeight),
    decoration(winDecoration)
{
    createWindow();
}

void FloatingWindow::createWindow() {
    int winLeft, winTop, winRight, winBot;
    XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);

    XPLMCreateWindow_t params;
    params.structSize = sizeof(params);
    params.left = winLeft + 100 ;
    params.right = winLeft + 100 + width;
    params.top = winTop - 100 ;
    params.bottom = winTop - 100 - height;
    params.visible = 1;
    params.refcon = this;
    params.drawWindowFunc = [] (XPLMWindowID id, void *ref) {
        reinterpret_cast<FloatingWindow *>(ref)->onDraw();
    };
    params.handleMouseClickFunc = [] (XPLMWindowID id, int x, int y, XPLMMouseStatus status, void *ref) -> int {
        return reinterpret_cast<FloatingWindow *>(ref)->onClick(x, y, status);
    };
    params.handleRightClickFunc = [] (XPLMWindowID id, int x, int y, XPLMMouseStatus status, void *ref) -> int {
        return reinterpret_cast<FloatingWindow *>(ref)->onRightClick(x, y, status);
    };
    params.handleKeyFunc = [] (XPLMWindowID id, char key, XPLMKeyFlags flags, char vKey, void *ref, int losingFocus) {
        reinterpret_cast<FloatingWindow *>(ref)->onKey(key, flags, vKey, losingFocus);
    };
    params.handleCursorFunc = [] (XPLMWindowID id, int x, int y, void *ref) -> XPLMCursorStatus {
        return reinterpret_cast<FloatingWindow *>(ref)->getCursor(x, y);
    };
    params.handleMouseWheelFunc =  [] (XPLMWindowID id, int x, int y, int wheel, int clicks, void *ref) -> int {
        return reinterpret_cast<FloatingWindow *>(ref)->onMouseWheel(x, y, wheel, clicks);
    };
    params.layer = xplm_WindowLayerFloatingWindows;

    params.decorateAsFloatingWindow = decoration;

    window = XPLMCreateWindowEx(&params);

    if (!window) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Couldn't create floating window");
        flywithlua::LuaIsRunning = false;
        return;
    }

    isInVR = xpIsInVR;
    
    if (xpIsInVR) {
        XPLMSetWindowPositioningMode(window, xplm_WindowVR, -1);
    } else {
        XPLMSetWindowPositioningMode(window, xplm_WindowPositionFree, -1);
        XPLMSetWindowGravity(window, 0, 1, 0, 0);
    }
}

void FloatingWindow::setTitle(char *title) {
    XPLMSetWindowTitle(window, title);
}

void FloatingWindow::setDrawCallback(char *func) {
    onDrawFunc = func;
}

void FloatingWindow::setLeftMouseCallback(char *func) {
    onLeftMouseFunc = func;
}

void FloatingWindow::setCloseCallback(char *func) {
    onCloseFunc = func;
}

void FloatingWindow::moveFromOrToVR() {
    if (xpIsInVR && !isInVR) {
        // X-Plane switched to VR but our window isn't in VR
        XPLMSetWindowPositioningMode(window, xplm_WindowVR, -1);
        isInVR = true;
    } else if (!xpIsInVR && isInVR) {
        // Our window is still in VR but X-Plane switched to 2D
        XPLMSetWindowPositioningMode(window, xplm_WindowPositionFree, -1);
        isInVR = false;
        
        int winLeft, winTop, winRight, winBot;
        XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);
        XPLMSetWindowGeometry(window, winLeft + 100, winTop - 100, winLeft + 100 + width, winTop - 100 - height);
    }
}

void FloatingWindow::onDraw() {
    if (!flywithlua::LuaIsRunning)  {
        return;
    }
    
    lua_State *L = flywithlua::FWLLua;

    lua_getglobal(L, onDrawFunc.c_str());
    if (!lua_isfunction(L, 1)) {
        lua_pop(L, 1);
        return;
    }

    int left, top, right, bottom;
    XPLMGetWindowGeometry(window, &left, &top, &right, &bottom);
    
    lua_pushlightuserdata(L, this);
    lua_pushnumber(L, left);
    lua_pushnumber(L, bottom);
    
    XPLMSetGraphicsState(0, 0, 0, 0, 1, 1, 0);

    flywithlua::WeAreNotInDrawingState = false;
    flywithlua::CopyDataRefsToLua();
    if (lua_pcall(L, 3, 0, 0)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Can't execute floating window draw callback");
        flywithlua::LuaIsRunning = false;
        return;
    }
    flywithlua::CopyDataRefsToXPlane();
    flywithlua::WeAreNotInDrawingState = true;
}

bool FloatingWindow::onClick(int x, int y, XPLMMouseStatus status) {
    if (!flywithlua::LuaIsRunning)  {
        return;
    }
    
    lua_State *L = flywithlua::FWLLua;

    lua_getglobal(L, onLeftMouseFunc.c_str());
    if (!lua_isfunction(L, 1)) {
        lua_pop(L, 1);
        return;
    }

    int left, top, right, bottom;
    XPLMGetWindowGeometry(window, &left, &top, &right, &bottom);

    lua_pushlightuserdata(L, this);
    lua_pushnumber(L, x - left);
    lua_pushnumber(L, y - bottom);
    lua_pushnumber(L, status);

    flywithlua::CopyDataRefsToLua();
    if (lua_pcall(L, 4, 0, 0)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Can't execute floating window click callback");
        flywithlua::LuaIsRunning = false;
        return;
    }
    flywithlua::CopyDataRefsToXPlane();
    
    return true;
}

void FloatingWindow::onClose() {
    if (!flywithlua::LuaIsRunning)  {
        return;
    }
    
    lua_State *L = flywithlua::FWLLua;

    lua_getglobal(L, onCloseFunc.c_str());
    if (!lua_isfunction(L, 1)) {
        lua_pop(L, 1);
        return;
    }

    lua_pushlightuserdata(L, this);

    flywithlua::CopyDataRefsToLua();
    if (lua_pcall(L, 1, 0, 0)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Can't execute floating window close callback");
        flywithlua::LuaIsRunning = false;
        return;
    }
    flywithlua::CopyDataRefsToXPlane();
    
    return true;
}

bool FloatingWindow::isVisible() const {
    return XPLMGetWindowIsVisible(window);
}

bool FloatingWindow::onRightClick(int x, int y, XPLMMouseStatus status) {
    return true;
}

void FloatingWindow::onKey(char key, XPLMKeyFlags flags, char virtualKey, bool losingFocus) {
    return true;
}

XPLMCursorStatus FloatingWindow::getCursor(int x, int y) {
    return xplm_CursorDefault;
}

bool FloatingWindow::onMouseWheel(int x, int y, int wheel, int clicks) {
    return true;
}

FloatingWindow::~FloatingWindow() {
    XPLMDebugString("~FloatingWindow\n");
    XPLMDestroyWindow(window);
}

namespace {
std::vector<std::shared_ptr<FloatingWindow>> floatingWindows;

int LuaCreateFloatingWindow(lua_State *L) {
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: wrong arguments given to create_floating_window.");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    int width = lua_tointeger(L, 1);
    int height = lua_tointeger(L, 2);
    int decoration = lua_tointeger(L, 3);
    
    auto wnd = std::make_shared<FloatingWindow>(width, height, decoration);
    floatingWindows.push_back(wnd);
    
    lua_pushlightuserdata(L, wnd.get());
    
    return 1;
}

int LuaDestroyFloatingWindow(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: wrong arguments given to create_floating_window.");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    for (auto it = floatingWindows.begin(); it != floatingWindows.end(); ) {
        auto &wndPtr = *it;
        if (wndPtr.get() == wnd) {
            it = floatingWindows.erase(it);
            break;
        } else {
            ++it;
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

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    wnd->setDrawCallback(lua_tostring(L, 2));

    return 0;
}

int LuaSetOnClickCallback(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isstring(L, 2)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_onclick");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    wnd->setLeftMouseCallback(lua_tostring(L, 2));

    return 0;
}

int LuaSetOnCloseCallback(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isstring(L, 2)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_onclose");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    wnd->setCloseCallback(lua_tostring(L, 2));

    return 0;
}

}

void registerFloatingWindowFunctions(lua_State *L) {
    lua_register(L, "float_wnd_create", LuaCreateFloatingWindow);
    lua_register(L, "float_wnd_set_title", LuaSetFloatingWindowTitle);
    lua_register(L, "float_wnd_set_ondraw", LuaSetOnDrawCallback);
    lua_register(L, "float_wnd_set_onclick", LuaSetOnClickCallback);
    lua_register(L, "float_wnd_set_onclose", LuaSetOnCloseCallback);
    lua_register(L, "float_wnd_destroy", LuaDestroyFloatingWindow);
}

void floatingWindowFlightLoopCallback() {
    static XPLMDataRef vrEnabledRef = XPLMFindDataRef("sim/graphics/VR/enabled");
    if (vrEnabledRef) {
        FloatingWindow::xpIsInVR = XPLMGetDatai(vrEnabledRef);
    }
    
    for (auto it = floatingWindows.begin(); it != floatingWindows.end(); ) {
        auto wnd = *it;
        if (!wnd->isVisible()) {
            wnd->onClose();
            it = floatingWindows.erase(it);
        } else {
            wnd->moveFromOrToVR();
            ++it;
        }
    }
}

void resetFloatingWindows() {
    floatingWindows.clear();
}
