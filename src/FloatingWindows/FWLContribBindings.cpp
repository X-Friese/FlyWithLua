#include "FWLContribBindings.h"
#include "FlyWithLuaWindow.h"
#include "../FlyWithLuaImGui.h"
#include "../FlyWithLua.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "contrib.h"

#include <string>
#include <vector>
#include <cstdlib>
#include <cctype>

#if IBM
#include <windows.h>
#include <shellapi.h>
#endif

namespace flywithlua {

// Helper to open a URL in the user's default browser safely
static void OpenBrowserURL(const std::string& url) {
    if (url.rfind("http://", 0) != 0 && url.rfind("https://", 0) != 0) {
        return; // Security guard: only open http/https links
    }
#if IBM
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif APL
    std::string cmd = "open \"" + url + "\" &";
    system(cmd.c_str());
#elif LIN
    std::string cmd = "xdg-open \"" + url + "\" &";
    system(cmd.c_str());
#endif
}

// Format Lua string: handles single string literal, or variadic format string + arguments via Lua string.format
static std::string FormatLuaString(lua_State* L) {
    int num_args = lua_gettop(L);
    if (num_args <= 0) {
        return "";
    }
    if (num_args == 1) {
        const char* s = lua_tostring(L, 1);
        return s ? s : "";
    }

    // Call Lua's string.format with all arguments on stack
    lua_getglobal(L, "string");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        const char* s = lua_tostring(L, 1);
        return s ? s : "";
    }
    lua_getfield(L, -1, "format");
    lua_remove(L, -2); // Remove "string" table

    for (int i = 1; i <= num_args; ++i) {
        lua_pushvalue(L, i);
    }

    if (lua_pcall(L, num_args, 1, 0) != 0) {
        // Formatting error: pop error message and return first raw argument
        lua_pop(L, 1);
        const char* raw = lua_tostring(L, 1);
        return raw ? raw : "";
    }

    const char* formatted = lua_tostring(L, -1);
    std::string res = formatted ? formatted : "";
    lua_pop(L, 1);
    return res;
}

// ----------------------------------------------------------------------------
// Markdown Extension Callbacks & Config
// ----------------------------------------------------------------------------

static void DefaultMarkdownLinkCallback(ImGui::MarkdownLinkCallbackData data) {
    if (!data.isImage && data.link && data.linkLength > 0) {
        std::string url(data.link, data.linkLength);
        OpenBrowserURL(url);
    }
}

// ----------------------------------------------------------------------------
// Tooltip & Markdown Persistent State
// ----------------------------------------------------------------------------

static ImGui::TimedTooltip::TooltipConfig s_FWLTooltipConfig;
static bool s_FWLTooltipConfigInitialized = false;

static ImGui::TimedTooltip::TooltipConfig& GetFWLTooltipConfig() {
    if (!s_FWLTooltipConfigInitialized) {
        s_FWLTooltipConfig = ImGui::TimedTooltip::GetCurrentTooltipConfig();
        s_FWLTooltipConfigInitialized = true;
        ImGui::TimedTooltip::SetDefaultTooltipConfig(&s_FWLTooltipConfig);
    }
    return s_FWLTooltipConfig;
}

void UpdateContribMarkdownFonts() {
    static ImGui::MarkdownConfig s_FWLMarkdownConfig;

    ImFont* h1 = flywithlua::GetFontByName("Roboto-Medium-20");
    if (!h1) h1 = flywithlua::GetFontByName("Roboto-Medium");
    if (!h1) h1 = flywithlua::GetDefaultFont();

    ImFont* h2 = flywithlua::GetFontByName("Roboto-Medium-16");
    if (!h2) h2 = flywithlua::GetFontByName("Roboto-Medium");
    if (!h2) h2 = flywithlua::GetDefaultFont();

    ImFont* h3 = flywithlua::GetFontByName("Roboto-Regular-16");
    if (!h3) h3 = flywithlua::GetFontByName("Roboto-Regular");
    if (!h3) h3 = flywithlua::GetDefaultFont();

    ImFont* regular = flywithlua::GetFontByName("Roboto-Regular-16");
    if (!regular) regular = flywithlua::GetFontByName("Roboto-Regular");
    if (!regular) regular = flywithlua::GetDefaultFont();

    ImFont* bold = flywithlua::GetFontByName("Roboto-Medium-16");
    if (!bold) bold = flywithlua::GetFontByName("Roboto-Medium");
    if (!bold) bold = flywithlua::GetDefaultFont();

    ImFont* italic = flywithlua::GetFontByName("Roboto-Italic-16");
    if (!italic) italic = flywithlua::GetFontByName("Roboto-Italic");
    if (!italic) italic = flywithlua::GetFontByName("Roboto-Light-16");
    if (!italic) italic = flywithlua::GetFontByName("Roboto-Light");
    if (!italic) italic = flywithlua::GetDefaultFont();

    ImFont* mono = flywithlua::GetFontByName("RobotoMono-Regular-16");
    if (!mono) mono = flywithlua::GetFontByName("RobotoMono-Regular");
    if (!mono) mono = flywithlua::GetFontByName("ProggyForever");
    if (!mono) mono = flywithlua::GetFontByName("ProggyClean");
    if (!mono) mono = flywithlua::GetDefaultFont();

    s_FWLMarkdownConfig.h1Font      = h1;
    s_FWLMarkdownConfig.h2Font      = h2;
    s_FWLMarkdownConfig.h3Font      = h3;
    s_FWLMarkdownConfig.boldFont    = bold;
    s_FWLMarkdownConfig.italicFont  = italic;
    s_FWLMarkdownConfig.monoFont    = mono;
    s_FWLMarkdownConfig.regularFont = regular;

    s_FWLMarkdownConfig.headingFormats[0] = { h1, true };
    s_FWLMarkdownConfig.headingFormats[1] = { h2, true };
    s_FWLMarkdownConfig.headingFormats[2] = { h3, false };
    s_FWLMarkdownConfig.headingFormats[3] = { bold, false };

    s_FWLMarkdownConfig.linkCallback = DefaultMarkdownLinkCallback;

    ImGui::SetDefaultMarkdownConfig(&s_FWLMarkdownConfig);

    auto& tt_cfg = GetFWLTooltipConfig();
    tt_cfg.font = regular;
}

// ----------------------------------------------------------------------------
// 1. Markdown Extension Lua Bindings
// ----------------------------------------------------------------------------

static int LuaMarkdownText(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::Markdown::Text(s);
    return 0;
}

static int LuaMarkdownTextWrapped(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::Markdown::TextWrapped(s);
    return 0;
}

static int LuaMarkdownTextUnformatted(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    ImGui::Markdown::TextUnformatted(s ? s : "");
    return 0;
}

static int LuaMarkdownButtonMD(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    std::string tip = (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) ? luaL_checkstring(L, 2) : "";
    float wrap_width = (lua_gettop(L) >= 3 && lua_isnumber(L, 3)) ? static_cast<float>(lua_tonumber(L, 3)) : 400.0f;
    bool clicked = ImGui::Markdown::ButtonMD(label ? label : "", tip, wrap_width);
    lua_pushboolean(L, clicked);
    return 1;
}

static int LuaMarkdownCalcTextSize(lua_State* L) {
    std::string s = FormatLuaString(L);
    float wrap_width = (lua_gettop(L) >= 2 && lua_isnumber(L, 2)) ? static_cast<float>(lua_tonumber(L, 2)) : 0.0f;
    ImGuiID hash_id = ImGui::GetID("##FWL_MD_CALC");
    ImVec2 sz = ImGui::Markdown::CalcTextSize(hash_id, 0, s, wrap_width);
    lua_pushnumber(L, sz.x);
    lua_pushnumber(L, sz.y);
    return 2;
}

// ----------------------------------------------------------------------------
// 2. TimedTooltip Extension Lua Bindings
// ----------------------------------------------------------------------------

static int LuaTimedTooltipText(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::TimedTooltip::Text("%s", s.c_str());
    return 0;
}

static int LuaTimedTooltipTextMD(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::TimedTooltip::TextMD(s);
    return 0;
}

static int LuaTimedTooltipTextUnformatted(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    ImGui::TimedTooltip::TextUnformatted(s ? s : "");
    return 0;
}

static int LuaTimedTooltipTextMDUnformatted(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    ImGui::TimedTooltip::TextMDUnformatted(s ? s : "");
    return 0;
}

static int LuaTimedTooltipSetDelay(lua_State* L) {
    int cycles = static_cast<int>(luaL_checkinteger(L, 1));
    auto& cfg = GetFWLTooltipConfig();
    cfg.hover_delay_cycles = cycles;
    return 0;
}

static int LuaTimedTooltipSetMaxDisplay(lua_State* L) {
    int cycles = static_cast<int>(luaL_checkinteger(L, 1));
    auto& cfg = GetFWLTooltipConfig();
    cfg.max_display_cycles = cycles;
    return 0;
}

static int LuaTimedTooltipSetMaxWidth(lua_State* L) {
    float w = static_cast<float>(luaL_checknumber(L, 1));
    auto& cfg = GetFWLTooltipConfig();
    cfg.max_wrap_width = w;
    return 0;
}

static int LuaTimedTooltipSetMinWidth(lua_State* L) {
    float w = static_cast<float>(luaL_checknumber(L, 1));
    auto& cfg = GetFWLTooltipConfig();
    cfg.min_wrap_width = w;
    return 0;
}

static int LuaTimedTooltipClear(lua_State* L) {
    (void)L;
    ImGui::TimedTooltip::ClearActiveTooltip();
    return 0;
}

// ----------------------------------------------------------------------------
// 3. Contrib Widgets Lua Bindings
// ----------------------------------------------------------------------------

static int LuaToggleButton(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    bool val = lua_toboolean(L, 2) != 0;
    bool changed = ImGui::ToggleButton(label, &val);
    lua_pushboolean(L, changed);
    lua_pushboolean(L, val);
    return 2;
}

static int LuaToggleButtons(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    int state = static_cast<int>(luaL_checkinteger(L, 2));
    const char* off_label = luaL_checkstring(L, 3);
    const char* on_label  = luaL_checkstring(L, 4);
    bool changed = ImGui::ToggleButtons(label, &state, off_label, on_label);
    lua_pushboolean(L, changed);
    lua_pushinteger(L, state);
    return 2;
}

static int LuaAdjustItemOnMouseWheel(lua_State* L) {
    float val   = static_cast<float>(luaL_checknumber(L, 1));
    float v_min = static_cast<float>(luaL_checknumber(L, 2));
    float v_max = static_cast<float>(luaL_checknumber(L, 3));
    float step  = (lua_gettop(L) >= 4 && lua_isnumber(L, 4))
                      ? static_cast<float>(lua_tonumber(L, 4)) : 0.01f;
    bool changed = ImGui::AdjustItemOnMouseWheel(&val, v_min, v_max, step);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaCycleItemOnMouseWheel(lua_State* L) {
    int idx   = static_cast<int>(luaL_checkinteger(L, 1));
    int count = static_cast<int>(luaL_checkinteger(L, 2));
    bool wrap = (lua_gettop(L) >= 3) ? (lua_toboolean(L, 3) != 0) : false;
    
    // Support 1-based indexing if caller passes 1..count
    bool one_based = (idx >= 1 && idx <= count);
    int zero_idx = one_based ? (idx - 1) : idx;
    bool changed = ImGui::CycleItemOnMouseWheel(&zero_idx, count, wrap);
    int result_idx = one_based ? (zero_idx + 1) : zero_idx;

    lua_pushboolean(L, changed);
    lua_pushinteger(L, result_idx);
    return 2;
}

static int LuaPercentSlider(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    float val = static_cast<float>(luaL_checknumber(L, 2));
    bool changed = ImGui::PercentSlider(label, &val);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaSliderWithReset(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    float val     = static_cast<float>(luaL_checknumber(L, 2));
    float v_min   = static_cast<float>(luaL_checknumber(L, 3));
    float v_max   = static_cast<float>(luaL_checknumber(L, 4));
    float def_val = static_cast<float>(luaL_checknumber(L, 5));
    bool changed = ImGui::SliderWithReset(label, &val, def_val, v_min, v_max);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaPercentSliderWithReset(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    float val     = static_cast<float>(luaL_checknumber(L, 2));
    float def_val = static_cast<float>(luaL_checknumber(L, 3));
    bool changed = ImGui::PercentSliderWithReset(label, &val, def_val);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaResetButton(lua_State* L) {
    const char* str_id = luaL_checkstring(L, 1);
    bool is_mod   = lua_toboolean(L, 2) != 0;
    bool abutting = (lua_gettop(L) >= 3) ? (lua_toboolean(L, 3) != 0) : true;
    bool clicked  = ImGui::ResetButton(str_id, is_mod, 0.0f, abutting);
    lua_pushboolean(L, clicked);
    return 1;
}

static int LuaButtonLink(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    bool clicked = ImGui::ButtonLink(label);
    if (clicked && lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        std::string url = luaL_checkstring(L, 2);
        OpenBrowserURL(url);
    }
    lua_pushboolean(L, clicked);
    return 1;
}

static int LuaTextLink(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    bool clicked = ImGui::TextLink(label);
    if (clicked && lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        std::string url = luaL_checkstring(L, 2);
        OpenBrowserURL(url);
    }
    lua_pushboolean(L, clicked);
    return 1;
}

static int LuaSmallButtonLink(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    bool clicked = ImGui::SmallButtonLink(label);
    if (clicked && lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        std::string url = luaL_checkstring(L, 2);
        OpenBrowserURL(url);
    }
    lua_pushboolean(L, clicked);
    return 1;
}

static int LuaAttachItemCursor(lua_State* L) {
    ImGuiMouseCursor cursor = (lua_gettop(L) >= 1 && lua_isnumber(L, 1))
                                  ? static_cast<ImGuiMouseCursor>(lua_tointeger(L, 1))
                                  : ImGuiMouseCursor_Hand;
    ImGui::AttachItemCursor(cursor);
    return 0;
}

// Helper to parse ImU32 color from Lua (supports integer, table {r, g, b, [a]}, or nil/0)
static ImU32 ParseLuaColor(lua_State* L, int idx) {
    if (lua_gettop(L) < idx || lua_isnil(L, idx)) return 0;
    if (lua_isnumber(L, idx)) {
        return (ImU32)lua_tonumber(L, idx);
    }
    if (lua_istable(L, idx)) {
        float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
        lua_rawgeti(L, idx, 1); if (lua_isnumber(L, -1)) r = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, idx, 2); if (lua_isnumber(L, -1)) g = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, idx, 3); if (lua_isnumber(L, -1)) b = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, idx, 4); if (lua_isnumber(L, -1)) a = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));
    }
    return 0;
}

static ImGui::ImGuiBadgeVariant ParseLuaBadgeVariant(lua_State* L, int idx) {
    if (lua_isnumber(L, idx)) {
        return static_cast<ImGui::ImGuiBadgeVariant>(lua_tointeger(L, idx));
    }
    if (lua_isstring(L, idx)) {
        std::string s = lua_tostring(L, idx);
        for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (s == "primary" || s == "blue") return ImGui::ImGuiBadgeVariant_Primary;
        if (s == "success" || s == "green") return ImGui::ImGuiBadgeVariant_Success;
        if (s == "warning" || s == "yellow" || s == "caution") return ImGui::ImGuiBadgeVariant_Warning;
        if (s == "danger" || s == "red" || s == "error") return ImGui::ImGuiBadgeVariant_Danger;
        if (s == "info") return ImGui::ImGuiBadgeVariant_Info;
        return ImGui::ImGuiBadgeVariant_Default;
    }
    return ImGui::ImGuiBadgeVariant_Default;
}

static int LuaBadge(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    if (!text) return 0;

    if (lua_gettop(L) >= 2 && (lua_istable(L, 2) || (lua_isnumber(L, 2) && lua_tonumber(L, 2) > 10.0))) {
        ImU32 bg_col = ParseLuaColor(L, 2);
        ImU32 fg_col = (lua_gettop(L) >= 3) ? ParseLuaColor(L, 3) : 0;
        float rounding = (lua_gettop(L) >= 4 && lua_isnumber(L, 4)) ? static_cast<float>(lua_tonumber(L, 4)) : -1.0f;
        ImGui::Badge(text, bg_col, fg_col, rounding);
    } else {
        ImGui::ImGuiBadgeVariant variant = (lua_gettop(L) >= 2 && !lua_isnil(L, 2))
                                               ? ParseLuaBadgeVariant(L, 2)
                                               : ImGui::ImGuiBadgeVariant_Default;
        float rounding = (lua_gettop(L) >= 3 && lua_isnumber(L, 3)) ? static_cast<float>(lua_tonumber(L, 3)) : -1.0f;
        ImGui::Badge(text, variant, rounding);
    }
    return 0;
}

static int LuaPill(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    if (!text) return 0;

    if (lua_gettop(L) >= 2 && (lua_istable(L, 2) || (lua_isnumber(L, 2) && lua_tonumber(L, 2) > 10.0))) {
        ImU32 bg_col = ParseLuaColor(L, 2);
        ImU32 fg_col = (lua_gettop(L) >= 3) ? ParseLuaColor(L, 3) : 0;
        ImGui::Pill(text, bg_col, fg_col);
    } else {
        ImGui::ImGuiBadgeVariant variant = (lua_gettop(L) >= 2 && !lua_isnil(L, 2))
                                               ? ParseLuaBadgeVariant(L, 2)
                                               : ImGui::ImGuiBadgeVariant_Default;
        ImGui::Pill(text, variant);
    }
    return 0;
}

static int LuaKbd(lua_State* L) {
    const char* key_label = luaL_checkstring(L, 1);
    if (key_label) {
        ImGui::Kbd(key_label);
    }
    return 0;
}

static int LuaSpinner(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    float radius      = (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) ? (float)lua_tonumber(L, 2) : 0.0f;
    float thickness   = (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) ? (float)lua_tonumber(L, 3) : -1.0f;
    ImU32 color       = ParseLuaColor(L, 4);
    float speed       = (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) ? (float)lua_tonumber(L, 5) : 1.0f;
    bool res = ImGui::Spinner(label, radius, thickness, color, speed);
    lua_pushboolean(L, res);
    return 1;
}

static int LuaRadialTimer(lua_State* L) {
    const char* label       = luaL_checkstring(L, 1);
    float frac              = (float)luaL_checknumber(L, 2);
    float radius            = (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) ? (float)lua_tonumber(L, 3) : 0.0f;
    float thickness         = (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) ? (float)lua_tonumber(L, 4) : -1.0f;
    ImU32 color             = ParseLuaColor(L, 5);
    ImU32 bg_color          = ParseLuaColor(L, 6);
    bool clockwise          = (lua_gettop(L) >= 7 && !lua_isnil(L, 7)) ? (lua_toboolean(L, 7) != 0) : true;
    bool res = ImGui::RadialTimer(label, frac, radius, thickness, color, bg_color, clockwise);
    lua_pushboolean(L, res);
    return 1;
}

static int LuaCountdownTimer(lua_State* L) {
    const char* label       = luaL_checkstring(L, 1);
    float current_sec       = (float)luaL_checknumber(L, 2);
    float max_sec           = (float)luaL_checknumber(L, 3);
    float radius            = (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) ? (float)lua_tonumber(L, 4) : 0.0f;
    float thickness         = (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) ? (float)lua_tonumber(L, 5) : -1.0f;
    ImU32 color             = ParseLuaColor(L, 6);
    ImU32 bg_color          = ParseLuaColor(L, 7);
    bool clockwise          = (lua_gettop(L) >= 8 && !lua_isnil(L, 8)) ? (lua_toboolean(L, 8) != 0) : true;
    bool res = ImGui::CountdownTimer(label, current_sec, max_sec, radius, thickness, color, bg_color, clockwise);
    lua_pushboolean(L, res);
    return 1;
}

static int LuaGetMouseWheel(lua_State* L) {
    bool consume = (lua_gettop(L) >= 1) ? (lua_toboolean(L, 1) != 0) : true;
    float delta = ImGui::GetMouseWheel(consume);
    lua_pushnumber(L, delta);
    return 1;
}

static int LuaSetCursorHandling(lua_State* L) {
    if (lua_gettop(L) >= 1) {
        if (lua_islightuserdata(L, 1) && lua_gettop(L) >= 2) {
            auto wnd = reinterpret_cast<flwnd::FloatingWindow*>(lua_touserdata(L, 1));
            bool enable = lua_toboolean(L, 2) != 0;
            auto fwlWnd = dynamic_cast<flwnd::FlyWithLuaWindow*>(wnd);
            if (fwlWnd) {
                fwlWnd->setImguiCursorHandling(enable);
            }
        } else if (lua_isboolean(L, 1) || lua_isnumber(L, 1)) {
            bool enable = lua_isboolean(L, 1) ? (lua_toboolean(L, 1) != 0) : (lua_tointeger(L, 1) != 0);
            flywithlua::imgui_mouse_cursors = enable ? 1 : 0;
        }
    }
    lua_pushboolean(L, flywithlua::imgui_mouse_cursors != 0);
    return 1;
}

static int LuaGetCursorHandling(lua_State* L) {
    if (lua_gettop(L) >= 1 && lua_islightuserdata(L, 1)) {
        auto wnd = reinterpret_cast<flwnd::FloatingWindow*>(lua_touserdata(L, 1));
        auto fwlWnd = dynamic_cast<flwnd::FlyWithLuaWindow*>(wnd);
        if (fwlWnd) {
            lua_pushboolean(L, fwlWnd->getImguiCursorHandling());
            return 1;
        }
    }
    lua_pushboolean(L, flywithlua::imgui_mouse_cursors != 0);
    return 1;
}

// ----------------------------------------------------------------------------
// Registration & Namespace / Alias Hierarchy Setup
// ----------------------------------------------------------------------------

void InitContribSupport(lua_State* L) {
    // 1. Get or create global imgui table
    lua_getglobal(L, "imgui");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "imgui");
    }

    // 2. Build imgui.Markdown table
    lua_newtable(L);
    lua_pushcfunction(L, LuaMarkdownText);              lua_setfield(L, -2, "Text");
    lua_pushcfunction(L, LuaMarkdownTextWrapped);       lua_setfield(L, -2, "TextWrapped");
    lua_pushcfunction(L, LuaMarkdownTextUnformatted);   lua_setfield(L, -2, "TextUnformatted");
    lua_pushcfunction(L, LuaMarkdownButtonMD);          lua_setfield(L, -2, "ButtonMD");
    lua_pushcfunction(L, LuaMarkdownButtonMD);          lua_setfield(L, -2, "Button");
    lua_pushcfunction(L, LuaMarkdownCalcTextSize);      lua_setfield(L, -2, "CalcTextSize");
    lua_pushcfunction(L, LuaMarkdownCalcTextSize);      lua_setfield(L, -2, "CalcSize");

    lua_pushcfunction(L, LuaMarkdownText);              lua_setfield(L, -2, "text");
    lua_pushcfunction(L, LuaMarkdownTextWrapped);       lua_setfield(L, -2, "text_wrapped");
    lua_pushcfunction(L, LuaMarkdownTextUnformatted);   lua_setfield(L, -2, "text_unformatted");
    lua_pushcfunction(L, LuaMarkdownButtonMD);          lua_setfield(L, -2, "button");
    lua_pushcfunction(L, LuaMarkdownCalcTextSize);      lua_setfield(L, -2, "calc_text_size");

    // Assign to imgui.Markdown and imgui.MD
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "Markdown");
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "MD");
    lua_pop(L, 1); // Pop Markdown table

    // 3. Build imgui.TimedTooltip table
    lua_newtable(L);
    lua_pushcfunction(L, LuaTimedTooltipText);                lua_setfield(L, -2, "Text");
    lua_pushcfunction(L, LuaTimedTooltipTextMD);              lua_setfield(L, -2, "TextMD");
    lua_pushcfunction(L, LuaTimedTooltipTextUnformatted);     lua_setfield(L, -2, "TextUnformatted");
    lua_pushcfunction(L, LuaTimedTooltipTextMDUnformatted);   lua_setfield(L, -2, "TextMDUnformatted");
    lua_pushcfunction(L, LuaTimedTooltipTextMDUnformatted);   lua_setfield(L, -2, "TextUnformattedMD");
    lua_pushcfunction(L, LuaTimedTooltipSetDelay);            lua_setfield(L, -2, "SetDelay");
    lua_pushcfunction(L, LuaTimedTooltipSetDelay);            lua_setfield(L, -2, "SetHoverDelay");
    lua_pushcfunction(L, LuaTimedTooltipSetMaxDisplay);       lua_setfield(L, -2, "SetMaxDisplay");
    lua_pushcfunction(L, LuaTimedTooltipSetMaxWidth);         lua_setfield(L, -2, "SetMaxWidth");
    lua_pushcfunction(L, LuaTimedTooltipSetMinWidth);         lua_setfield(L, -2, "SetMinWidth");
    lua_pushcfunction(L, LuaTimedTooltipClear);               lua_setfield(L, -2, "ClearActiveTooltip");

    lua_pushcfunction(L, LuaTimedTooltipText);                lua_setfield(L, -2, "text");
    lua_pushcfunction(L, LuaTimedTooltipTextMD);              lua_setfield(L, -2, "text_md");
    lua_pushcfunction(L, LuaTimedTooltipTextUnformatted);     lua_setfield(L, -2, "text_unformatted");
    lua_pushcfunction(L, LuaTimedTooltipTextMDUnformatted);   lua_setfield(L, -2, "text_md_unformatted");
    lua_pushcfunction(L, LuaTimedTooltipSetDelay);            lua_setfield(L, -2, "set_delay");
    lua_pushcfunction(L, LuaTimedTooltipSetDelay);            lua_setfield(L, -2, "set_hover_delay");
    lua_pushcfunction(L, LuaTimedTooltipSetMaxDisplay);       lua_setfield(L, -2, "set_max_display");
    lua_pushcfunction(L, LuaTimedTooltipSetMaxWidth);         lua_setfield(L, -2, "set_max_width");
    lua_pushcfunction(L, LuaTimedTooltipSetMinWidth);         lua_setfield(L, -2, "set_min_width");
    lua_pushcfunction(L, LuaTimedTooltipClear);               lua_setfield(L, -2, "clear");

    // Sub-namespaces: imgui.TimedTooltip.MD and imgui.TimedTooltip.Markdown
    lua_newtable(L);
    lua_pushcfunction(L, LuaTimedTooltipTextMD);              lua_setfield(L, -2, "Text");
    lua_pushcfunction(L, LuaTimedTooltipTextMDUnformatted);   lua_setfield(L, -2, "TextUnformatted");
    lua_pushcfunction(L, LuaTimedTooltipTextMD);              lua_setfield(L, -2, "text");
    lua_pushcfunction(L, LuaTimedTooltipTextMDUnformatted);   lua_setfield(L, -2, "text_unformatted");

    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "MD");
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "Markdown");
    lua_pop(L, 1); // Pop MD sub-table

    // Assign to imgui.TimedTooltip
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "TimedTooltip");
    lua_pop(L, 1); // Pop TimedTooltip table

    // 4. Cross-bridge aliases: imgui.Markdown.TimedTooltip and imgui.MD.TimedTooltip
    lua_getfield(L, -1, "TimedTooltip");
    lua_getfield(L, -2, "Markdown");
    lua_pushvalue(L, -2); // Push TimedTooltip table
    lua_setfield(L, -2, "TimedTooltip");
    lua_pop(L, 1); // Pop Markdown

    lua_getfield(L, -2, "MD");
    lua_pushvalue(L, -2); // Push TimedTooltip table
    lua_setfield(L, -2, "TimedTooltip");
    lua_pop(L, 1); // Pop MD
    lua_pop(L, 1); // Pop TimedTooltip

    // 5. Root convenience functions directly on imgui table
    lua_pushcfunction(L, LuaMarkdownText);              lua_setfield(L, -2, "TextMD");
    lua_pushcfunction(L, LuaMarkdownText);              lua_setfield(L, -2, "text_md");
    lua_pushcfunction(L, LuaMarkdownTextWrapped);       lua_setfield(L, -2, "TextWrappedMD");
    lua_pushcfunction(L, LuaMarkdownTextWrapped);       lua_setfield(L, -2, "text_wrapped_md");
    lua_pushcfunction(L, LuaMarkdownTextUnformatted);   lua_setfield(L, -2, "TextUnformattedMD");
    lua_pushcfunction(L, LuaMarkdownTextUnformatted);   lua_setfield(L, -2, "text_unformatted_md");
    lua_pushcfunction(L, LuaMarkdownButtonMD);          lua_setfield(L, -2, "ButtonMD");
    lua_pushcfunction(L, LuaMarkdownButtonMD);          lua_setfield(L, -2, "button_md");

    // 6. Contrib Widgets directly on imgui table
    lua_pushcfunction(L, LuaToggleButton);              lua_setfield(L, -2, "ToggleButton");
    lua_pushcfunction(L, LuaToggleButton);              lua_setfield(L, -2, "toggle_button");
    lua_pushcfunction(L, LuaToggleButtons);             lua_setfield(L, -2, "ToggleButtons");
    lua_pushcfunction(L, LuaToggleButtons);             lua_setfield(L, -2, "toggle_buttons");

    lua_pushcfunction(L, LuaAdjustItemOnMouseWheel);     lua_setfield(L, -2, "AdjustItemOnMouseWheel");
    lua_pushcfunction(L, LuaAdjustItemOnMouseWheel);     lua_setfield(L, -2, "adjust_item_on_mouse_wheel");
    lua_pushcfunction(L, LuaCycleItemOnMouseWheel);      lua_setfield(L, -2, "CycleItemOnMouseWheel");
    lua_pushcfunction(L, LuaCycleItemOnMouseWheel);      lua_setfield(L, -2, "cycle_item_on_mouse_wheel");

    lua_pushcfunction(L, LuaPercentSlider);             lua_setfield(L, -2, "PercentSlider");
    lua_pushcfunction(L, LuaPercentSlider);             lua_setfield(L, -2, "percent_slider");
    lua_pushcfunction(L, LuaPercentSlider);             lua_setfield(L, -2, "SliderPercent");
    lua_pushcfunction(L, LuaPercentSlider);             lua_setfield(L, -2, "slider_percent");

    lua_pushcfunction(L, LuaSliderWithReset);           lua_setfield(L, -2, "SliderWithReset");
    lua_pushcfunction(L, LuaSliderWithReset);           lua_setfield(L, -2, "slider_with_reset");
    lua_pushcfunction(L, LuaSliderWithReset);           lua_setfield(L, -2, "ResettableSlider");
    lua_pushcfunction(L, LuaSliderWithReset);           lua_setfield(L, -2, "resettable_slider");

    lua_pushcfunction(L, LuaPercentSliderWithReset);    lua_setfield(L, -2, "PercentSliderWithReset");
    lua_pushcfunction(L, LuaPercentSliderWithReset);    lua_setfield(L, -2, "percent_slider_with_reset");
    lua_pushcfunction(L, LuaPercentSliderWithReset);    lua_setfield(L, -2, "ResettablePercentSlider");
    lua_pushcfunction(L, LuaPercentSliderWithReset);    lua_setfield(L, -2, "resettable_percent_slider");
    lua_pushcfunction(L, LuaPercentSliderWithReset);    lua_setfield(L, -2, "ResettableSliderPercent");
    lua_pushcfunction(L, LuaPercentSliderWithReset);    lua_setfield(L, -2, "resettable_slider_percent");

    lua_pushcfunction(L, LuaResetButton);               lua_setfield(L, -2, "ResetButton");
    lua_pushcfunction(L, LuaResetButton);               lua_setfield(L, -2, "reset_button");

    lua_pushcfunction(L, LuaSpinner);                   lua_setfield(L, -2, "Spinner");
    lua_pushcfunction(L, LuaSpinner);                   lua_setfield(L, -2, "spinner");
    lua_pushcfunction(L, LuaSpinner);                   lua_setfield(L, -2, "ActivitySpinner");
    lua_pushcfunction(L, LuaSpinner);                   lua_setfield(L, -2, "activity_spinner");
    lua_pushcfunction(L, LuaRadialTimer);               lua_setfield(L, -2, "RadialTimer");
    lua_pushcfunction(L, LuaRadialTimer);               lua_setfield(L, -2, "radial_timer");
    lua_pushcfunction(L, LuaRadialTimer);               lua_setfield(L, -2, "ProgressDial");
    lua_pushcfunction(L, LuaRadialTimer);               lua_setfield(L, -2, "progress_dial");
    lua_pushcfunction(L, LuaCountdownTimer);            lua_setfield(L, -2, "CountdownTimer");
    lua_pushcfunction(L, LuaCountdownTimer);            lua_setfield(L, -2, "countdown_timer");
    lua_pushcfunction(L, LuaCountdownTimer);            lua_setfield(L, -2, "CountdownDial");
    lua_pushcfunction(L, LuaCountdownTimer);            lua_setfield(L, -2, "countdown_dial");

    lua_pushcfunction(L, LuaButtonLink);                lua_setfield(L, -2, "ButtonLink");
    lua_pushcfunction(L, LuaButtonLink);                lua_setfield(L, -2, "button_link");
    lua_pushcfunction(L, LuaSmallButtonLink);           lua_setfield(L, -2, "SmallButtonLink");
    lua_pushcfunction(L, LuaSmallButtonLink);           lua_setfield(L, -2, "small_button_link");
    lua_pushcfunction(L, LuaTextLink);                  lua_setfield(L, -2, "TextLink");
    lua_pushcfunction(L, LuaTextLink);                  lua_setfield(L, -2, "text_link");
    lua_pushcfunction(L, LuaAttachItemCursor);          lua_setfield(L, -2, "AttachItemCursor");
    lua_pushcfunction(L, LuaAttachItemCursor);          lua_setfield(L, -2, "attach_item_cursor");
    lua_pushcfunction(L, LuaGetMouseWheel);             lua_setfield(L, -2, "GetMouseWheel");
    lua_pushcfunction(L, LuaGetMouseWheel);             lua_setfield(L, -2, "get_mouse_wheel");
    lua_pushcfunction(L, LuaGetMouseWheel);             lua_setfield(L, -2, "GetMouseWheelEither");
    lua_pushcfunction(L, LuaGetMouseWheel);             lua_setfield(L, -2, "get_mouse_wheel_either");
    lua_pushcfunction(L, LuaGetMouseWheel);             lua_setfield(L, -2, "GetMouseWheelAny");
    lua_pushcfunction(L, LuaGetMouseWheel);             lua_setfield(L, -2, "get_mouse_wheel_any");

    // 7. Badges, Pills & Keycaps
    lua_pushcfunction(L, LuaBadge);                     lua_setfield(L, -2, "Badge");
    lua_pushcfunction(L, LuaBadge);                     lua_setfield(L, -2, "badge");
    lua_pushcfunction(L, LuaBadge);                     lua_setfield(L, -2, "Tag");
    lua_pushcfunction(L, LuaBadge);                     lua_setfield(L, -2, "tag");
    lua_pushcfunction(L, LuaPill);                      lua_setfield(L, -2, "Pill");
    lua_pushcfunction(L, LuaPill);                      lua_setfield(L, -2, "pill");
    lua_pushcfunction(L, LuaKbd);                       lua_setfield(L, -2, "Kbd");
    lua_pushcfunction(L, LuaKbd);                       lua_setfield(L, -2, "kbd");
    lua_pushcfunction(L, LuaKbd);                       lua_setfield(L, -2, "Key");
    lua_pushcfunction(L, LuaKbd);                       lua_setfield(L, -2, "key");
    lua_pushcfunction(L, LuaKbd);                       lua_setfield(L, -2, "KeyCap");
    lua_pushcfunction(L, LuaKbd);                       lua_setfield(L, -2, "key_cap");
    lua_pushcfunction(L, LuaKbd);                       lua_setfield(L, -2, "Keycap");
    lua_pushcfunction(L, LuaKbd);                       lua_setfield(L, -2, "keycap");

    // Badge variant enums
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Default);      lua_setfield(L, -2, "ImGuiBadgeVariant_Default");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Primary);      lua_setfield(L, -2, "ImGuiBadgeVariant_Primary");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Success);      lua_setfield(L, -2, "ImGuiBadgeVariant_Success");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Warning);      lua_setfield(L, -2, "ImGuiBadgeVariant_Warning");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Danger);       lua_setfield(L, -2, "ImGuiBadgeVariant_Danger");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Info);         lua_setfield(L, -2, "ImGuiBadgeVariant_Info");

    lua_pushcfunction(L, LuaSetCursorHandling);         lua_setfield(L, -2, "SetCursorHandling");
    lua_pushcfunction(L, LuaSetCursorHandling);         lua_setfield(L, -2, "set_cursor_handling");
    lua_pushcfunction(L, LuaSetCursorHandling);         lua_setfield(L, -2, "EnableMouseCursors");
    lua_pushcfunction(L, LuaSetCursorHandling);         lua_setfield(L, -2, "enable_mouse_cursors");
    lua_pushcfunction(L, LuaGetCursorHandling);         lua_setfield(L, -2, "GetCursorHandling");
    lua_pushcfunction(L, LuaGetCursorHandling);         lua_setfield(L, -2, "get_cursor_handling");

    // 7. Global convenience aliases (matching contrib-global.h in C++)
    lua_getfield(L, -1, "TimedTooltip");
    lua_setglobal(L, "TimedTooltip");

    lua_getfield(L, -1, "Markdown");
    lua_setglobal(L, "Markdown");

    lua_getfield(L, -1, "MD");
    lua_setglobal(L, "MD");

    lua_pop(L, 1); // Pop imgui table
}

} // namespace flywithlua
