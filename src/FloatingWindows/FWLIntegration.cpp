/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <sol.hpp>
#include <iostream>

#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>
#include "FWLIntegration.h"
#include "FloatingWindow.h"
#include "FlyWithLuaWindow.h"
#include "../FlyWithLuaImGui.h"
#include "../FlyWithLua.h"
#include "imgui/imgui_internal.h"
#include "FWLContribBindings.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace flwnd {

/**
 * A function, which returns a Lua callback function.
 *
 * This type allows to abstract away method how a callback is passed from lua, be that by name or directly by reference.
 * Using a provider allows to delay callback resolution until invocation time which often is useful when passing
 * callback by name: the callback function may not be defined yet when the callback is set, but it will be by the call
 * time. Prom pedantic perspective this is not a great practice but it does happen in existing scripts, so we support
 * it.
 */
typedef std::function<sol::protected_function()> CallbackProvider;

/**
 * A function, which sets some kind of callback for a floating (or ImGUI) window.
 *
 * This type is used by LuaSetCallbackByName and LuaSetCallbackByRef as a template parameter and is implemented by
 * LuaSetImguiBuilder and LuaSetOn???Callback functions.
 */
typedef void(CallbackSetter)(sol::light<FloatingWindow>, CallbackProvider const&);

std::vector<std::shared_ptr<FloatingWindow>> floatingWindows;
std::vector<ImTextureID> textureIDs;

ImTextureID loadImage(const std::string& fileName) {
    int imgWidth, imgHeight, nComps;
    // Always force 4-channel RGBA
    uint8_t *data = stbi_load(fileName.c_str(), &imgWidth, &imgHeight, &nComps, 4);

    if (!data) {
        throw std::runtime_error(std::string("Couldn't load image: ") + stbi_failure_reason());
    }

    // Static unified API - handles Metal, Vulkan, and OpenGL automatically
    ImTextureID id = ImgWindow::CreateTexture(data, imgWidth, imgHeight);
    stbi_image_free(data);

    if (id == (ImTextureID)0) {
        throw std::runtime_error("ImgWindow::CreateTexture failed");
    }

    textureIDs.push_back(id);
    return id;
}

void deleteImage(ImTextureID textureId) {
    if (textureId != (ImTextureID)0) {
        ImgWindow::DeleteTexture(textureId);
        auto it = std::find(textureIDs.begin(), textureIDs.end(), textureId);
        if (it != textureIDs.end()) {
            textureIDs.erase(it);
        }
    }
}

int LuaCreateFloatingWindow(lua_State *L) {
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isboolean(L, 4)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: wrong arguments given to float_wnd_create.");
        FindAndQuarantine (L);
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
            auto fwlWnd = std::make_shared<FlyWithLuaWindow>(width, height, decoration);
            // Allow optional 5th argument to explicitly override mouse cursor handling
            if (lua_gettop(L) >= 5 && (lua_isboolean(L, 5) || lua_isnumber(L, 5))) {
                bool enable_cursors = lua_isboolean(L, 5) ? (lua_toboolean(L, 5) != 0) : (lua_tointeger(L, 5) != 0);
                fwlWnd->setImguiCursorHandling(enable_cursors);
            }
            wnd = fwlWnd;
        } else {
            wnd = std::make_shared<LegacyFloatingWindow>(width, height, decoration);
        }
        floatingWindows.push_back(wnd);
        lua_pushlightuserdata(L, wnd.get());
        return 1;
    } catch (const std::exception &e) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Couldn't create floating window");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }
}

int LuaDestroyFloatingWindow(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: wrong arguments given to float_wnd_destroy.");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    for (auto wndPtr: floatingWindows) {
        if (wndPtr.get() == wnd) {
            // this will lead to actual destruction in the flightloop callback
            wndPtr->setIsCmdVisible(1);
            wndPtr->setVisible(false);
        }
    }

    return 0;
}

int LuaSetFloatingWindowTitle(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isstring(L, 2)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_title");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    wnd->setTitle(lua_tostring(L, 2));

    return 0;
}


// ImGui font management for FlyWithLua scripts.
// Supports both named fonts (e.g. imgui.PushFont("Roboto-Regular-16") or imgui.PushFont("Roboto-Regular", 16))
// and legacy 1-based indices (1..9) for 100% backward compatibility.
int LuaImguiPushFont(lua_State *L) {
    int num_args = lua_gettop(L);
    if (num_args < 1) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Warning: imgui.PushFont called without arguments, using default font.");
        ImFont* def = flywithlua::GetDefaultFont();
        if (def != nullptr) {
            ImGui::PushFont(def, def->LegacySize);
        } else {
            ImGui::PushFont(nullptr, 0.0f);
        }
        return 0;
    }

    ImFont* target_font = nullptr;
    float font_size = 0.0f;

    if (num_args >= 2 && lua_isnumber(L, 2)) {
        font_size = static_cast<float>(lua_tonumber(L, 2));
    }

    int arg1_type = lua_type(L, 1);
    if (arg1_type == LUA_TNUMBER) {
        int font_number = static_cast<int>(lua_tointeger(L, 1));
        target_font = flywithlua::GetFontByIndex(font_number);
        if (target_font == nullptr) {
            flywithlua::logMsg(logToDevCon, "FlyWithLua Warning: imgui.PushFont: Invalid font index " + std::to_string(font_number) + ", falling back to default font.");
            target_font = flywithlua::GetDefaultFont();
        }
    } else if (arg1_type == LUA_TSTRING) {
        std::string font_name = lua_tostring(L, 1);
        if (font_size > 0.0f) {
            std::string compound_key = font_name + "-" + std::to_string(static_cast<int>(font_size));
            target_font = flywithlua::GetFontByName(compound_key);
        }
        if (target_font == nullptr) {
            target_font = flywithlua::GetFontByName(font_name);
        }
        if (target_font == nullptr) {
            // If no explicit size was given, check if font_name ends with -<size> (e.g. "Roboto-Light-20")
            if (font_size <= 0.0f) {
                size_t dash = font_name.rfind('-');
                if (dash != std::string::npos && dash + 1 < font_name.size()) {
                    try {
                        font_size = std::stof(font_name.substr(dash + 1));
                    } catch (...) {}
                }
            }

            target_font = flywithlua::GetDefaultFont();
            if (font_size > 0.0f) {
                flywithlua::logMsg(logToDevCon, "FlyWithLua Warning: imgui.PushFont: Font '" + font_name + "' not found, falling back to default font @ " + std::to_string(static_cast<int>(font_size)) + "px.");
            } else {
                flywithlua::logMsg(logToDevCon, "FlyWithLua Warning: imgui.PushFont: Font '" + font_name + "' not found, falling back to default font.");
            }
        }
    } else {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Warning: imgui.PushFont expected font name (string) or index (number), falling back to default font.");
        target_font = flywithlua::GetDefaultFont();
    }

    if (target_font != nullptr) {
        if (font_size > 0.0f) {
            ImGui::PushFont(target_font, font_size);
        } else {
            ImGui::PushFont(target_font, target_font->LegacySize);
        }
    } else {
        ImGui::PushFont(nullptr, 0.0f);
    }

    return 0;
}

int LuaImguiPopFont(lua_State *L) {
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (ctx != nullptr && ctx->FontStack.Size > 0) {
        ImGui::PopFont();
    } else {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Warning: imgui.PopFont called with empty font stack.");
    }
    return 0;
}

int LuaImguiGetFontList(lua_State *L) {
    lua_newtable(L);
    int idx = 1;
    for (const auto& name : flywithlua::GetRegisteredFontNames()) {
        lua_pushstring(L, name.c_str());
        lua_rawseti(L, -2, idx++);
    }
    return 1;
}


int LuaSetFloatingWindowPosition(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_position");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->setPosition(lua_tonumber(L, 2), lua_tonumber(L, 3));

    return 0;
}

int LuaLoadFloatinWindowImage(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_load_image");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    std::string name = lua_tostring(L, 1);
    try {
        ImTextureID id = loadImage(name);
        lua_pushinteger(L, (lua_Integer)(intptr_t)id);
        return 1;
    } catch (const std::exception &e) {
        std::string err = std::string("FlyWithLua Error: Couldn't load image: ") + e.what() + " in " + name;
        flywithlua::logMsg(logToDevCon, err);
        lua_pushstring(flywithlua::FWLLua, err.c_str());
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }
}

int LuaDeleteFloatingWindowImage(lua_State *L) {
    if (!lua_isnumber(L, 1)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_delete_image");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    ImTextureID id = (ImTextureID)(intptr_t)lua_tointeger(L, 1);
    deleteImage(id);
    return 0;
}

/**
 * Set floating window drawing function from Lua.
 *
 * See LuaSetImguiBuilder for details about CallbackProvider.
 *
 * @param wnd Pointer to a FloatingWindow instance.
 * @param on_draw_provider A lambda, which returns the actual on_draw handler from Lua.
 */
void LuaSetOnDrawCallback(sol::light<FloatingWindow> wnd, CallbackProvider const& on_draw_provider) {
    static_cast<FloatingWindow*>(wnd)->setDrawCallback([on_draw_provider] (FloatingWindow &fwnd) {
        if (!flywithlua::LuaIsRunning)  {
            return;
        }

        XPLMWindowID window = fwnd.getXWindow();

        int left, top, right, bottom;
        XPLMGetWindowGeometry(window, &left, &top, &right, &bottom);

        XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0);

        flywithlua::WeAreNotInDrawingState = false;
        flywithlua::CopyDataRefsToLua();

        auto on_draw = on_draw_provider();
        if (!on_draw) {
            flywithlua::panic("FlyWithLua Error: invalid or nil window builder passed to float_wnd_set_ondraw");
            // Need to find a way for this to work when we are using sol.
            // FindAndQuarantine (L);
            return;
        }
        auto result = on_draw(sol::light<FloatingWindow>(fwnd), left, bottom);
        if (!result.valid()) {
            sol::error err = result;
            flywithlua::panic(err.what());
        }

        flywithlua::CopyDataRefsToXPlane();
        flywithlua::WeAreNotInDrawingState = true;
    });
}

int LuaGetXPLMWindowHandle(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Can't execute float_wnd_get_xplm_handle.");
        FindAndQuarantine (L);
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
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_get_dimensions");
        FindAndQuarantine (L);
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
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Can't execute float_wnd_get_visible");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    lua_pushboolean(L, wnd->isVisible());

    return 1;
}

int LuaFloatingWindowSetVisible(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || (!lua_isnumber(L, 2) && !lua_isboolean(L, 2))) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_visible");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    bool visible = false;
    int cmdVal = 0;
    if (lua_isboolean(L, 2)) {
        visible = (lua_toboolean(L, 2) != 0);
        cmdVal = visible ? 1 : 0;
    } else {
        cmdVal = static_cast<int>(lua_tointeger(L, 2));
        visible = (cmdVal != 0);
    }

    wnd->setIsCmdVisible(cmdVal);
    wnd->setVisible(visible);

    return 0;
}

int LuaFloatingWindowIsPopped(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Can't execute float_wnd_is_popped");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    lua_pushboolean(L, wnd->isPopped());

    return 1;
}

int LuaFloatingWindowIsFront(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Can't execute float_wnd_is_front");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    lua_pushboolean(L, wnd->isFront());

    return 1;
}

int LuaFloatingWindowBringToFront(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Can't execute float_wnd_bring_to_front");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->bringToFront();

    return 0;
}

int LuaFloatingWindowIsVR(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Can't execute float_wnd_is_vr");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    lua_pushboolean(L, wnd->isVR());

    return 1;
}

int LuaSetFloatingWindowResizingLimits(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_resizing_limits");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->setResizingLimits(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));

    return 0;
}

int LuaSetFloatingWindowPositioningMode(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_positioning_mode");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->setPositioningMode(lua_tonumber(L, 2), lua_tonumber(L, 3));

    return 0;
}

int LuaSetFloatingWindowGravity(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4) || !lua_isnumber(L, 5)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_gravity");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

    wnd->setGravity(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));

    return 0;
}

int LuaGetFloatingWindowGeometry(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Can't execute float_wnd_get_geometry");
        FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    XPLMWindowID window = wnd->getXWindow();

    // For VR: gLeft = Width; gTop = Height
    int gLeft, gTop, gRight,gBottom;

    if (wnd->isVR()) {
        XPLMGetWindowGeometryVR(window, &gLeft, &gTop);
    } else if (wnd->isPopped()) {
        XPLMGetWindowGeometryOS(window, &gLeft, &gTop, &gRight, &gBottom);
    } else {
        XPLMGetWindowGeometry(window, &gLeft, &gTop, &gRight, &gBottom);
    }

    lua_pushnumber(L, gLeft);
    lua_pushnumber(L, gTop);

    if (wnd->isVR()) {
        return 2;
    }

    lua_pushnumber(L, gRight);
    lua_pushnumber(L, gBottom);

    return 4;
}

int LuaSetFloatingWindowGeometry(lua_State *L) {

    if (lua_islightuserdata(L, 1)) {

        FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

        if (wnd->isVR() && lua_isnumber(L, 2) && lua_isnumber(L, 3)) {
            wnd->setWindowGeometryVR(lua_tonumber(L, 2), lua_tonumber(L, 3));
            return 0;
        } else if (lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4) && lua_isnumber(L, 5)) {
            if (wnd->isPopped()) {
                wnd->setWindowGeometryOS(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
            } else {
                wnd->setWindowGeometry(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
            }
            return 0;
        }
    }

    flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_geometry");
    FindAndQuarantine (L);
    flywithlua::LuaIsRunning = false;
    return 0;
}

int LuaFloatingWindowSetImguiCursorHandling(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || (!lua_isboolean(L, 2) && !lua_isnumber(L, 2))) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_imgui_cursor_handling");
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    bool enable = false;
    if (lua_isboolean(L, 2)) {
        enable = (lua_toboolean(L, 2) != 0);
    } else {
        enable = (lua_tointeger(L, 2) != 0);
    }

    auto fwlWnd = dynamic_cast<FlyWithLuaWindow *>(wnd);
    if (fwlWnd) {
        fwlWnd->setImguiCursorHandling(enable);
    }
    return 0;
}

int LuaFloatingWindowGetImguiCursorHandling(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_get_imgui_cursor_handling");
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    auto fwlWnd = dynamic_cast<FlyWithLuaWindow *>(wnd);
    if (fwlWnd) {
        lua_pushboolean(L, fwlWnd->getImguiCursorHandling());
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}

int LuaFloatingWindowSetScrolling(lua_State *L) {
    if (!lua_islightuserdata(L, 1) || (!lua_isboolean(L, 2) && !lua_isnumber(L, 2))) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_scrolling");
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);
    bool enable = false;
    if (lua_isboolean(L, 2)) {
        enable = (lua_toboolean(L, 2) != 0);
    } else {
        enable = (lua_tointeger(L, 2) != 0);
    }

    auto fwlWnd = dynamic_cast<FlyWithLuaWindow *>(wnd);
    if (fwlWnd) {
        fwlWnd->setScrollingAllowed(enable);
    }
    return 0;
}

/**
 * Set floating window mouse handler from Lua.
 *
 * See LuaSetImguiBuilder for details about CallbackProvider.
 *
 * @param fwnd Pointer to a FloatingWindow instance.
 * @param on_click_provider A lambda, which returns the actual on_click handler from Lua.
 */
void LuaSetOnClickCallback(sol::light<FloatingWindow> fwnd, CallbackProvider const& on_click_provider) {
    static_cast<FloatingWindow*>(fwnd)->setClickCallback([on_click_provider] (FloatingWindow &targetWnd, int x, int y, XPLMMouseStatus status) {
        if (!flywithlua::LuaIsRunning)  {
            return;
        }

        XPLMWindowID window = targetWnd.getXWindow();

        auto on_click = on_click_provider();
        if (!on_click) {
            flywithlua::panic("FlyWithLua Error: invalid or nil window builder passed to float_wnd_set_onclick\n"
                              "FlyWithLua Error: check to see if float_wnd_set_onclick function is defined");
            // Need to find a way for this to work when we are using sol.
            // FindAndQuarantine (L);
            return;
        }

        int left, top, right, bottom;
        XPLMGetWindowGeometry(window, &left, &top, &right, &bottom);

        flywithlua::CopyDataRefsToLua();

        auto result = on_click(sol::light<FloatingWindow>(targetWnd), x - left, y - bottom, status);
        if (!result.valid()) {
            sol::error err = result;
            flywithlua::panic(err.what());
        }

        flywithlua::CopyDataRefsToXPlane();
    });
}

/**
 * Set floating window close event handler from Lua.
 *
 * See LuaSetImguiBuilder for details about CallbackProvider.
 *
 * @param fwnd Pointer to a FloatingWindow instance.
 * @param on_close_provider A lambda, which returns the actual on_close handler from Lua.
 */
void LuaSetOnCloseCallback(sol::light<FloatingWindow> fwnd, CallbackProvider const& on_close_provider) {
    static_cast<FloatingWindow*>(fwnd)->setCloseCallback([on_close_provider] (FloatingWindow &targetWnd) {
        if (!flywithlua::LuaIsRunning)  {
            return;
        }

        XPLMWindowID window = targetWnd.getXWindow();

        auto on_close = on_close_provider();
        if (!on_close) {
            flywithlua::panic("FlyWithLua Error: invalid or nil window builder passed to float_wnd_set_onclose\n"
                              "FlyWithLua Error: check to see if float_wnd_set_onclose function is defined");
            // Need to find a way for this to work when we are using sol.
            // FindAndQuarantine (L);
            return;
        }

        flywithlua::CopyDataRefsToLua();

        auto result = on_close(sol::light<FloatingWindow>(targetWnd));
        if (!result.valid()) {
            sol::error err = result;
            flywithlua::panic(err.what());
        }

        flywithlua::CopyDataRefsToXPlane();
    });
}

/**
 * Set ImGUI window builder from lua.
 *
 * This function accepts builder_provider instead of builder directly to allow callback-time resolution of the builder.
 * This is mostly useful when builder function is specified by name and float_wnd_set_imgui_builder() is called before
 * the function is actually defined in Lua. However, by the time builder callback is called, the function is defined and
 * builder_provider will be able to correctly discover it.
 *
 * Note: this builder_provider trick wouldn't be necessary if we only allowed passing builder by reference. If this ever
 * becomes the case, this could should be simplified.
 *
 * @param fwnd Pointer to an instance of ImGUIWindow to set builder for.
 * @param builder_provider A lambda, which returns the actual window builder from Lua.
 */
void LuaSetImguiBuilder(sol::light<FloatingWindow> fwnd, CallbackProvider const& builder_provider) {
    auto* wnd = dynamic_cast<FlyWithLuaWindow*>(static_cast<FloatingWindow*>(fwnd));
    if (!wnd) {
        flywithlua::panic("FlyWithLua Error: Wrong window type passed to float_wnd_set_imgui_builder");
        return;
    }

    wnd->setBuildCallback([builder_provider] (FlyWithLuaWindow& iwnd) {
        if (!flywithlua::LuaIsRunning)  {
            return;
        }

        // Obtain window builder and make sure it is a valid function (otherwise conversion to sol::protected_function
        // will fail).
        auto builder = builder_provider();
        if (!builder) {
            flywithlua::panic("FlyWithLua Error: invalid or nil window builder passed to float_wnd_set_imgui_builder");
            // Need to find a way for this to work when we are using sol.
            // FindAndQuarantine (L);
            return;
        }

        int left, top, right, bottom;
        XPLMGetWindowGeometry(iwnd.getXWindow(), &left, &top, &right, &bottom);

        flywithlua::CopyDataRefsToLua();

        // Attempt to execute passed function and stop Lua if it fails for any reason.
        auto result = builder(sol::light<FloatingWindow>(iwnd), left, bottom);
        if (!result.valid()) {
            sol::error err = result;
            flywithlua::panic(err.what());
        }
        flywithlua::CopyDataRefsToXPlane();
    });

    wnd->setErrorHandler([] (const std::string &errorMsg) {
        flywithlua::logMsg(logToDevCon, "FlyWithLua imgui error: " + errorMsg);
        lua_pushstring(flywithlua::FWLLua, errorMsg.c_str());
        // Need to find a way for this to work when we are using sol.
        // FindAndQuarantine (L);
        flywithlua::LuaIsRunning = false;
    });
}

/**
 * Accepts a function reference and creates an appropriate callback provider for actual callback setter.
 *
 * This function acts as a glue between Lua (sol2) and C++ implementation of callback setter. It allows to abstract away
 * whether the callback was passed by reference or by name when used together with LuaSetCallbackByName.
 *
 * @tparam setter Function which actually implements corresponding callback logic.
 * @param fwnd Pointer to FloatingWindow instance received from Lua as a light userdata.
 * @param callback Reference to the Lua callback function.
 */
template<CallbackSetter setter>
void LuaSetCallbackByRef(::sol::light<FloatingWindow> fwnd, ::sol::protected_function const& callback) {
    if (!callback) {
        flywithlua::panic("FlyWithLua Error: invalid or nil window builder passed to float_wnd_set_imgui_builder");
        // Need to find a way for this to work when we are using sol..
        // FindAndQuarantine (L);
        return;
    }
    setter(fwnd, [callback]() -> auto { return callback; });
}

/**
 * Accepts a function name and creates an appropriate callback provider for actual callback setter.
 *
 * Function with the specified name must exist in the global scope at the time when the callback is invoked. It is
 * responsibility of setter to ensure function validity before calling it.
 *
 * This function acts as a glue between Lua (sol2) and C++ implementation of callback setter. It allows to abstract away
 * whether the callback was passed by reference or by name when used together with LuaSetCallbackByName.
 *
 * @tparam setter Function which actually implements corresponding callback logic.
 * @param fwnd Pointer to FloatingWindow instance received from Lua as a light userdata.
 * @param callback_name Callback function name in the global scope.
 */
template<CallbackSetter setter>
void LuaSetCallbackByName(sol::light<FloatingWindow> fwnd, std::string const& callback_name) {
    setter(fwnd, [callback_name]() {
        sol::state_view lua(flywithlua::FWLLua);
        return lua[callback_name];
    });
}

void initFloatingWindowSupport() {
    // destroy old windows
    deinitFloatingWindowSupport();

    lua_State *L = flywithlua::FWLLua;

    lState = L;
    LoadImguiBindings();

    lua_register(L, "float_wnd_create", LuaCreateFloatingWindow);
    lua_register(L, "float_wnd_set_title", LuaSetFloatingWindowTitle);
    lua_register(L, "imgui_push_font", LuaImguiPushFont);
    lua_register(L, "imgui_pop_font", LuaImguiPopFont);
    lua_register(L, "float_wnd_set_position", LuaSetFloatingWindowPosition);
    lua_register(L, "float_wnd_get_xplm_handle", LuaGetXPLMWindowHandle);
    lua_register(L, "float_wnd_get_dimensions", LuaGetFloatingWindowDimensions);
    lua_register(L, "float_wnd_load_image", LuaLoadFloatinWindowImage);
    lua_register(L, "float_wnd_delete_image", LuaDeleteFloatingWindowImage);
    lua_register(L, "float_wnd_destroy", LuaDestroyFloatingWindow);
    lua_register(L, "float_wnd_get_visible", LuaFloatingWindowGetVisible);
    lua_register(L, "float_wnd_set_visible", LuaFloatingWindowSetVisible);
    lua_register(L, "float_wnd_is_popped", LuaFloatingWindowIsPopped);
    lua_register(L, "float_wnd_is_front", LuaFloatingWindowIsFront);
    lua_register(L, "float_wnd_bring_to_front", LuaFloatingWindowBringToFront);
    lua_register(L, "float_wnd_is_vr", LuaFloatingWindowIsVR);
    lua_register(L, "float_wnd_set_resizing_limits", LuaSetFloatingWindowResizingLimits);
    lua_register(L, "float_wnd_set_positioning_mode", LuaSetFloatingWindowPositioningMode);
    lua_register(L, "float_wnd_set_gravity", LuaSetFloatingWindowGravity);
    lua_register(L, "float_wnd_set_geometry", LuaSetFloatingWindowGeometry);
    lua_register(L, "float_wnd_get_geometry", LuaGetFloatingWindowGeometry);
    lua_register(L, "float_wnd_set_imgui_cursor_handling", LuaFloatingWindowSetImguiCursorHandling);
    lua_register(L, "float_wnd_set_use_imgui_cursors", LuaFloatingWindowSetImguiCursorHandling);
    lua_register(L, "float_wnd_get_imgui_cursor_handling", LuaFloatingWindowGetImguiCursorHandling);
    lua_register(L, "float_wnd_get_use_imgui_cursors", LuaFloatingWindowGetImguiCursorHandling);
    lua_register(L, "float_wnd_set_scrolling", LuaFloatingWindowSetScrolling);
    lua_register(L, "float_wnd_set_scroll_handling", LuaFloatingWindowSetScrolling);

    // Register modern font functions inside the imgui table namespace
    lua_getglobal(L, "imgui");
    if (lua_istable(L, -1)) {
        lua_pushcfunction(L, LuaImguiPushFont);
        lua_setfield(L, -2, "PushFont");
        lua_pushcfunction(L, LuaImguiPushFont);
        lua_setfield(L, -2, "push_font");
        lua_pushcfunction(L, LuaImguiPopFont);
        lua_setfield(L, -2, "PopFont");
        lua_pushcfunction(L, LuaImguiPopFont);
        lua_setfield(L, -2, "pop_font");
        lua_pushcfunction(L, LuaImguiGetFontList);
        lua_setfield(L, -2, "GetFontList");
        lua_pushcfunction(L, LuaImguiGetFontList);
        lua_setfield(L, -2, "get_font_list");
    }
    lua_pop(L, 1);

    // Register modern ImGui contrib extensions (Markdown, TimedTooltip, and Widgets)
    flywithlua::InitContribSupport(L);

	::sol::state_view lua(L);
    lua.set_function("float_wnd_set_imgui_builder", sol::overload(LuaSetCallbackByName<LuaSetImguiBuilder>,
                                                                  LuaSetCallbackByRef<LuaSetImguiBuilder>));
    lua.set_function("float_wnd_set_ondraw", sol::overload(LuaSetCallbackByName<LuaSetOnDrawCallback>,
                                                           LuaSetCallbackByRef<LuaSetOnDrawCallback>));
    lua.set_function("float_wnd_set_onclick", sol::overload(LuaSetCallbackByName<LuaSetOnClickCallback>,
                                                            LuaSetCallbackByRef<LuaSetOnClickCallback>));
    lua.set_function("float_wnd_set_onclose", sol::overload(LuaSetCallbackByName<LuaSetOnCloseCallback>,
                                                            LuaSetCallbackByRef<LuaSetOnCloseCallback>));

    lua_pushnumber(L, 1);
    lua_setglobal(L, "SUPPORTS_FLOATING_WINDOWS");
}

void deinitFloatingWindowSupport() {
    floatingWindows.clear();
    for (auto texId : textureIDs) {
        ImgWindow::DeleteTexture(texId);
    }
    textureIDs.clear();
}

bool FindAndQuarantine (lua_State *L)
{
  if (flywithlua::developer_mode == 1)
  {
    return false;
  }

  int result = 0;
  int wait = 0;
  lua_Debug debug;
  // 1 here means the function which called the current function.
  if (!lua_getstack(L, 1, &debug)) { /* Oops, panic or something... */ }
  if (!lua_getinfo(L, "S", &debug)) { /* Oops, panic again! */ }

  std::ostringstream oss_function_script_path, oss_script_name, oss_script_path_name, oss_quarantine_path_name;
  oss_function_script_path << "FlyWithLua Info: Function Script Path From Stack " << debug.short_src;
  std::string ScriptName = debug.short_src;
  const size_t last_slash_idx = ScriptName.find_last_of("/");
  if (std::string::npos != last_slash_idx)
  {
    ScriptName.erase(0, last_slash_idx + 1);
  }

  oss_script_name << "FlyWithLua Info: Function Script Name From Stack " << ScriptName;

  oss_script_path_name << flywithlua::scriptDir << "/" << ScriptName;
  std::string script_path_name = oss_script_path_name.str();

  oss_quarantine_path_name << flywithlua::quarantineDir << ScriptName;
  std::string quarantine_path_name = oss_quarantine_path_name.str();

  result = rename(script_path_name.c_str(), quarantine_path_name.c_str());
  if (result == 0)
  {
    flywithlua::logMsg(logToDevCon,
                         ("FlyWithLua Info: 3rd Moved Bad Script to " + quarantine_path_name));
  }
  else
  {
    flywithlua::logMsg(logToDevCon,
                         ("FlyWithLua Info: Could not move bad script to " + quarantine_path_name));
  }

  flywithlua::LuaIsRunning = false;
  flywithlua::found_bad_function_script = 1;
  throw std::logic_error(ScriptName.c_str());
  flywithlua::DebugLua();
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
