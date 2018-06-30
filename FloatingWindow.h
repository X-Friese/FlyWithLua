/*
 *   Floating Windows for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#include <XPLMGraphics.h>
#include <XPLMDisplay.h>
#include <cstdint>
#include <vector>
#include <string>
#include <lua.hpp>
#include "FlyWithLua.h"

class FloatingWindow {
public:
    static int xpIsInVR;

    FloatingWindow(int winWidth, int winHeight, int winDecoration);
    void setTitle(char *title);
    void setDrawCallback(char *func);
    void setLeftMouseCallback(char *func);
    void setCloseCallback(char *func);
    bool isVisible() const;
    void onClose();
    ~FloatingWindow();
    
private:

    XPLMWindowID window{};
    int width, height, decoration;
    
    std::string onDrawFunc;
    std::string onLeftMouseFunc;
    std::string onCloseFunc;
    
    void createWindow();

    void onDraw();
    bool onClick(int x, int y, XPLMMouseStatus status);
    bool onRightClick(int x, int y, XPLMMouseStatus status);
    void onKey(char key, XPLMKeyFlags flags, char virtualKey, bool losingFocus);
    XPLMCursorStatus getCursor(int x, int y);
    bool onMouseWheel(int x, int y, int wheel, int clicks);
};

void registerFloatingWindowFunctions(lua_State *L);
void floatingWindowFlightLoopCallback();
