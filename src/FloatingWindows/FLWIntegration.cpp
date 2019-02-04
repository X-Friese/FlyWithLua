/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#include <vector>
#include <memory>
#include <stdexcept>
#include <sol.hpp>
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
            XPLMWindowID window = wnd->getXWindow();
            std::ostringstream oss_window_id;
            oss_window_id << "FlyWithLua Info: This XPLMWindowID " << window << " is for LuaCreateFloatingWindow";
            flywithlua::logMsg(logToAll, oss_window_id.str());
        } else {
            wnd = std::make_shared<FloatingWindow>(width, height, decoration);
            XPLMWindowID window = wnd->getXWindow();
            std::ostringstream oss_window_id;
            oss_window_id << "FlyWithLua Info: This XPLMWindowID " << window << " is for LuaCreateFloatingWindow";
            flywithlua::logMsg(logToAll, oss_window_id.str());
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
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_position");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    FloatingWindow *wnd = (FloatingWindow *) lua_touserdata(L, 1);

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
            std::ostringstream oss_window_id;
            oss_window_id << "FlyWithLua Info: This XPLMWindowID " << window << " is for LuaCreateFloatingWindow";
            flywithlua::logMsg(logToAll, oss_window_id.str());
            flywithlua::panic("FlyWithLua Error: invalid or nil window builder passed to float_wnd_set_ondraw");
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

int LuaGetFloatingWindowGeometry(lua_State *L) {
    if (!lua_islightuserdata(L, 1)) {
        flywithlua::logMsg(logToAll, "FlyWithLua Error: Can't execute float_wnd_get_geometry");
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

    flywithlua::logMsg(logToAll, "FlyWithLua Error: Wrong parameters passed to float_wnd_set_geometry");
    flywithlua::LuaIsRunning = false;
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
    static_cast<FloatingWindow*>(fwnd)->setClickCallback([on_click_provider] (FloatingWindow &fwnd, int x, int y, XPLMMouseStatus status) {
        if (!flywithlua::LuaIsRunning)  {
            return;
        }

        XPLMWindowID window = fwnd.getXWindow();

        auto on_click = on_click_provider();
        if (!on_click) {
            std::ostringstream oss_window_id;
            oss_window_id << "FlyWithLua Info: This XPLMWindowID " << window << " is for LuaCreateFloatingWindow";
            flywithlua::logMsg(logToAll, oss_window_id.str());
            flywithlua::panic("FlyWithLua Error: invalid or nil window builder passed to float_wnd_set_onclick\n"
                              "FlyWithLua Error: check to see if float_wnd_set_onclick function is defined");
            return;
        }

        int left, top, right, bottom;
        XPLMGetWindowGeometry(window, &left, &top, &right, &bottom);

        flywithlua::CopyDataRefsToLua();

        auto result = on_click(sol::light<FloatingWindow>(fwnd), x - left, y - bottom, status);
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
    static_cast<FloatingWindow*>(fwnd)->setCloseCallback([on_close_provider] (FloatingWindow &fwnd) {
        if (!flywithlua::LuaIsRunning)  {
            return;
        }

        XPLMWindowID window = fwnd.getXWindow();

        auto on_close = on_close_provider();
        if (!on_close) {
            std::ostringstream oss_window_id;
            oss_window_id << "FlyWithLua Info: This XPLMWindowID " << window << " is for LuaCreateFloatingWindow";
            flywithlua::logMsg(logToAll, oss_window_id.str());
            flywithlua::panic("FlyWithLua Error: invalid or nil window builder passed to float_wnd_set_onclose\n"
                              "FlyWithLua Error: check to see if float_wnd_set_onclose function is defined");
            return;
        }

        flywithlua::CopyDataRefsToLua();

        auto result = on_close(sol::light<FloatingWindow>(fwnd));
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
    auto* wnd = dynamic_cast<ImGUIWindow*>(static_cast<FloatingWindow*>(fwnd));
    if (!wnd) {
        flywithlua::panic("FlyWithLua Error: Wrong window type passed to float_wnd_set_imgui_builder");
        return;
    }

    wnd->setBuildCallback([builder_provider] (ImGUIWindow& iwnd) {
        if (!flywithlua::LuaIsRunning)  {
            return;
        }

        // Obtain window builder and make sure it is a valid function (otherwise conversion to sol::protected_function
        // will fail).
        auto builder = builder_provider();
        if (!builder) {
            flywithlua::panic("FlyWithLua Error: invalid or nil window builder passed to float_wnd_set_imgui_builder");
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
        flywithlua::logMsg(logToAll, "FlyWithLua imgui error: " + errorMsg);
        lua_pushstring(flywithlua::FWLLua, errorMsg.c_str());
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
    lua_register(L, "float_wnd_set_position", LuaSetFloatingWindowPosition);
    lua_register(L, "float_wnd_get_xplm_handle", LuaGetXPLMWindowHandle);
    lua_register(L, "float_wnd_get_dimensions", LuaGetFloatingWindowDimensions);
    lua_register(L, "float_wnd_load_image", LuaLoadFloatinWindowImage);
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
