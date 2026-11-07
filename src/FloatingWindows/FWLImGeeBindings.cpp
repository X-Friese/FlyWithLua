#include "FWLImGeeBindings.h"
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
#include <sys/stat.h>

#include "XPLMUtilities.h"
#include "XPLMPlugin.h"

#if IBM
#include <windows.h>
#include <shellapi.h>
#endif

namespace flywithlua {

#ifdef USE_LEGACY_LOCAL_DOCUMENT_RESOLVER
// [Kept as backup per Steve's instructions: switch back here if Roger's ImGee OpenURL has issues]
// Helper to resolve and open a local document safely
static bool OpenLocalDocument(const std::string& rel_path) {
    auto fileExists = [](const std::string& p) -> bool {
        struct stat st;
        return (stat(p.c_str(), &st) == 0 && S_ISREG(st.st_mode));
    };

    std::string resolved;
    if (fileExists(rel_path)) {
        resolved = rel_path;
    }

    // Try relative to X-Plane system path
    if (resolved.empty()) {
        char sysPath[512] = {0};
        XPLMGetSystemPath(sysPath);
        std::string sPath(sysPath);
        if (!sPath.empty() && sPath.back() != '/' && sPath.back() != '\\') {
            sPath += '/';
        }
        std::string cand1 = sPath + rel_path;
        std::string cand2 = sPath + "Resources/plugins/" + rel_path;
        if (fileExists(cand1)) {
            resolved = cand1;
        } else if (fileExists(cand2)) {
            resolved = cand2;
        }
    }

    // Try relative to calling plugin directory (Resources/plugins/FlyWithLua)
    if (resolved.empty()) {
        XPLMPluginID myId = XPLMGetMyID();
        if (myId != XPLM_NO_PLUGIN_ID) {
            char pluginFilePath[512] = {0};
            XPLMGetPluginInfo(myId, nullptr, pluginFilePath, nullptr, nullptr);
            std::string pPath(pluginFilePath);
            size_t slash = pPath.find_last_of("/\\");
            if (slash != std::string::npos) {
                std::string pluginDir = pPath.substr(0, slash);
                size_t prevSlash = pluginDir.find_last_of("/\\");
                std::string fwlDir = (prevSlash != std::string::npos) ? pluginDir.substr(0, prevSlash) : pluginDir;

                std::string clean = rel_path;
                if (clean.rfind("FlyWithLua/", 0) == 0) {
                    clean = clean.substr(11);
                }
                std::string cand1 = fwlDir + "/" + clean;
                std::string cand2 = fwlDir + "/" + rel_path;
                if (fileExists(cand1)) {
                    resolved = cand1;
                } else if (fileExists(cand2)) {
                    resolved = cand2;
                }
            }
        }
    }

    if (resolved.empty()) {
        return false;
    }

#if IBM
    HINSTANCE res = ShellExecuteA(NULL, "open", resolved.c_str(), NULL, NULL, SW_SHOWNORMAL);
    return (reinterpret_cast<INT_PTR>(res) > 32);
#elif APL
    std::string cmd = "open \"" + resolved + "\" &";
    return (system(cmd.c_str()) == 0);
#elif LIN
    std::string cmd = "xdg-open \"" + resolved + "\" &";
    return (system(cmd.c_str()) == 0);
#endif
}

// Helper to open a URL or local document safely
static void OpenURLOrDocument(const std::string& target) {
    if (target.rfind("local:", 0) == 0) {
        OpenLocalDocument(target.substr(6));
        return;
    }
    if (target.rfind("http://", 0) != 0 && target.rfind("https://", 0) != 0) {
        if (OpenLocalDocument(target)) return;
        return; // Security guard: only open http/https links or valid local files
    }
#if IBM
    ShellExecuteA(NULL, "open", target.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif APL
    std::string cmd = "open \"" + target + "\" &";
    system(cmd.c_str());
#elif LIN
    std::string cmd = "xdg-open \"" + target + "\" &";
    system(cmd.c_str());
#endif
}
#endif

// Universal Web & Document launcher: dispatches directly to ImGee's ImGui::OpenURL
static void OpenBrowserURL(const std::string& url) {
#ifdef USE_LEGACY_LOCAL_DOCUMENT_RESOLVER
    OpenURLOrDocument(url);
#else
    ImGui::OpenURL(url.c_str());
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
#ifdef USE_LEGACY_LOCAL_DOCUMENT_RESOLVER
        OpenURLOrDocument(url);
#else
        ImGui::OpenURL(url.c_str());
#endif
    }
}

static void FWLMarkdownTooltipCallback(ImGui::MarkdownTooltipCallbackData data) {
    if (!data.linkData.link || data.linkData.linkLength <= 0) return;
    std::string link(data.linkData.link, (size_t)data.linkData.linkLength);
    ImGuiID id = ImGui::GetID(link.c_str());
    if (data.linkData.isImage) {
        ImGui::MarkdownDetail::ShowMarkdownTooltip(id, true, link.c_str());
    } else {
        std::string tip;
        if (link.rfind("file:", 0) == 0 || link.rfind("local:", 0) == 0 || link.find(".pdf") != std::string::npos) {
            tip = "**Open Document**\n\n`" + link + "`";
        } else {
            std::string icon = data.linkIcon ? data.linkIcon : "";
            tip = (icon.empty() ? "" : icon + " ") + "**Open in Browser**\n\n`" + link + "`";
        }
        ImGui::MarkdownDetail::ShowMarkdownTooltip(id, true, tip.c_str());
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

void UpdateImGeeMarkdownFonts() {
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

    ImFont* small_font = flywithlua::GetFontByName("Roboto-Regular-13");
    if (!small_font) small_font = flywithlua::GetFontByName("Roboto-Light-13");
    if (!small_font) small_font = flywithlua::GetFontByIndex(4); // customFont4 = primary font 2, size 1 (respects custom imgui_font_size1)
    if (!small_font) small_font = regular;

    ImFont* bold = flywithlua::GetFontByName("Roboto-Bold-16");
    if (!bold) bold = flywithlua::GetFontByName("Roboto-Bold");
    if (!bold) bold = flywithlua::GetFontByName("Roboto-Medium-16");
    if (!bold) bold = flywithlua::GetFontByName("Roboto-Medium");
    if (!bold) bold = flywithlua::GetDefaultFont();

    ImFont* bold_italic = flywithlua::GetFontByName("Roboto-BoldItalic-16");
    if (!bold_italic) bold_italic = flywithlua::GetFontByName("Roboto-BoldItalic");
    if (!bold_italic) bold_italic = bold;

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

    s_FWLMarkdownConfig.h1Font         = h1;
    s_FWLMarkdownConfig.h2Font         = h2;
    s_FWLMarkdownConfig.h3Font         = h3;
    s_FWLMarkdownConfig.boldFont       = bold;
    s_FWLMarkdownConfig.boldItalicFont = bold_italic;
    s_FWLMarkdownConfig.italicFont     = italic;
    s_FWLMarkdownConfig.monoFont       = mono;
    s_FWLMarkdownConfig.regularFont    = regular;

    s_FWLMarkdownConfig.headingFormats[0] = { h1, true };
    s_FWLMarkdownConfig.headingFormats[1] = { h2, true };
    s_FWLMarkdownConfig.headingFormats[2] = { h3, false };
    s_FWLMarkdownConfig.headingFormats[3] = { bold, false };

    s_FWLMarkdownConfig.linkCallback    = DefaultMarkdownLinkCallback;
    s_FWLMarkdownConfig.tooltipCallback = FWLMarkdownTooltipCallback;

    ImGui::SetDefaultMarkdownConfig(&s_FWLMarkdownConfig);

    // Synchronize semantic Font Role registry with active FlyWithLua font atlas
    ImGui::SetFontRole(ImGui::ImFontRole::Regular, regular);
    ImGui::SetFontRole(ImGui::ImFontRole::Title, h2);
    ImGui::SetFontRole(ImGui::ImFontRole::Small, small_font);
    ImGui::SetFontRole(ImGui::ImFontRole::Bold, bold);
    ImGui::SetFontRole(ImGui::ImFontRole::BoldLarge, h1);
    ImGui::SetFontRole(ImGui::ImFontRole::Monospace, mono);
    ImGui::SetFontRole(ImGui::ImFontRole::Italic, italic);
    ImGui::SetFontRole(ImGui::ImFontRole::MonospaceMedium, mono);
    ImGui::SetFontRole(ImGui::ImFontRole::BoldItalic, bold_italic);
    ImGui::SetFontRole(ImGui::ImFontRole::Heading1, h1);
    ImGui::SetFontRole(ImGui::ImFontRole::Heading2, h2);
    ImGui::SetFontRole(ImGui::ImFontRole::Heading3, h3);
    ImGui::SetFontRole(ImGui::ImFontRole::Tiny, small_font);

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
    int top = lua_gettop(L);
    bool clicked = false;

    // Check if second argument is a string (and not purely numeric): tooltip overload
    if (top >= 2 && lua_isstring(L, 2) && !lua_isnumber(L, 2)) {
        const char* tip = lua_tostring(L, 2);
        float wrap_width = (top >= 3 && lua_isnumber(L, 3)) ? static_cast<float>(lua_tonumber(L, 3)) : 400.0f;
        bool allowed = (top >= 4 && lua_isboolean(L, 4)) ? (lua_toboolean(L, 4) != 0) : true;
        float sx = 0.0f, sy = 0.0f;
        if (top >= 5 && lua_isnumber(L, 5)) sx = static_cast<float>(lua_tonumber(L, 5));
        if (top >= 6 && lua_isnumber(L, 6)) sy = static_cast<float>(lua_tonumber(L, 6));
        clicked = ImGui::Markdown::Button(label ? label : "", tip ? tip : "", wrap_width, allowed, ImVec2(sx, sy));
    } else {
        // Standard 1:1 Dear ImGui Button signature: (label, [size_x, size_y])
        float sx = (top >= 2 && lua_isnumber(L, 2)) ? static_cast<float>(lua_tonumber(L, 2)) : 0.0f;
        float sy = (top >= 3 && lua_isnumber(L, 3)) ? static_cast<float>(lua_tonumber(L, 3)) : 0.0f;
        clicked = ImGui::Markdown::Button(label ? label : "", ImVec2(sx, sy));
    }

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
// 3. ImGee / LuiGee Widgets Lua Bindings
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

static int LuaIconButton(lua_State* L) {
    const char* str_id = luaL_checkstring(L, 1);
    const char* icon_name = luaL_checkstring(L, 2);
    float w = 0.0f;
    float h = 0.0f;
    const char* tooltip = nullptr;
    int top = lua_gettop(L);
    if (top >= 3 && lua_isnumber(L, 3)) w = static_cast<float>(lua_tonumber(L, 3));
    if (top >= 4 && lua_isnumber(L, 4)) h = static_cast<float>(lua_tonumber(L, 4));
    if (top >= 5 && !lua_isnil(L, 5)) tooltip = lua_tostring(L, 5);
    bool clicked = ImGui::IconButton(str_id, icon_name, ImVec2(w, h), tooltip);
    lua_pushboolean(L, clicked);
    return 1;
}

static int LuaButtonIcon(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    const char* icon_name = luaL_checkstring(L, 2);
    float w = 0.0f;
    float h = 0.0f;
    bool icon_on_right = false;
    const char* tooltip = nullptr;
    int top = lua_gettop(L);
    if (top >= 3 && lua_isnumber(L, 3)) w = static_cast<float>(lua_tonumber(L, 3));
    if (top >= 4 && lua_isnumber(L, 4)) h = static_cast<float>(lua_tonumber(L, 4));
    if (top >= 5 && !lua_isnil(L, 5)) icon_on_right = (lua_toboolean(L, 5) != 0);
    if (top >= 6 && !lua_isnil(L, 6)) tooltip = lua_tostring(L, 6);
    bool clicked = ImGui::ButtonIcon(label, icon_name, ImVec2(w, h), icon_on_right, tooltip);
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

static int LuaInputTextGetSelection(lua_State* L) {
    ImGuiContext* g = ImGui::GetCurrentContext();
    if (g && g->InputTextState.ID != 0 && g->InputTextState.Stb) {
        int s = g->InputTextState.GetSelectionStart();
        int e = g->InputTextState.GetSelectionEnd();
        if (s > e) std::swap(s, e);
        int cur = g->InputTextState.GetCursorPos();
        lua_pushinteger(L, s);
        lua_pushinteger(L, e);
        lua_pushinteger(L, cur);
        return 3;
    }
    lua_pushinteger(L, -1);
    lua_pushinteger(L, -1);
    lua_pushinteger(L, -1);
    return 3;
}

static int LuaInputTextSetSelection(lua_State* L) {
    ImGuiContext* g = ImGui::GetCurrentContext();
    int s = static_cast<int>(luaL_checkinteger(L, 1));
    int e = static_cast<int>(luaL_checkinteger(L, 2));
    if (g) {
        ImGuiID target_id = (g->InputTextState.ID != 0) ? g->InputTextState.ID : g->InputTextDeactivatedState.ID;
        if (target_id != 0) {
            g->InputTextState.ReloadSelectionStart = s;
            g->InputTextState.ReloadSelectionEnd = e;
            g->InputTextState.WantReloadUserBuf = true;
            g->InputTextReactivateId = target_id;
            g->InputTextDeactivatedState.ID = 0;
        }
    }
    return 0;
}

static int LuaCopyButton(lua_State* L) {
    const char* str_id = luaL_checkstring(L, 1);
    const char* text_to_copy = luaL_checkstring(L, 2);
    float duration = (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) ? (float)lua_tonumber(L, 3) : 1.5f;
    float size_x = (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) ? (float)lua_tonumber(L, 4) : 0.0f;
    float size_y = (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) ? (float)lua_tonumber(L, 5) : 0.0f;

    bool is_copied = false;
    bool clicked = ImGui::CopyButtonEx(str_id, text_to_copy, duration, ImVec2(size_x, size_y), &is_copied);

    lua_pushboolean(L, clicked);
    lua_pushboolean(L, is_copied);
    return 2;
}

static int LuaCopyLastItemButton(lua_State* L) {
    const char* str_id = luaL_checkstring(L, 1);
    float duration = (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) ? (float)lua_tonumber(L, 2) : 1.5f;
    float size_x = (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) ? (float)lua_tonumber(L, 3) : 0.0f;
    float size_y = (lua_gettop(L) >= 4 && !lua_isnil(L, 4)) ? (float)lua_tonumber(L, 4) : 0.0f;

    bool is_copied = false;
    bool clicked = ImGui::CopyLastItemButtonEx(str_id, duration, ImVec2(size_x, size_y), &is_copied);

    lua_pushboolean(L, clicked);
    lua_pushboolean(L, is_copied);
    return 2;
}

// ----------------------------------------------------------------------------
// 9. In-Cockpit File Dialogs & File Picker Suite
// ----------------------------------------------------------------------------

static int LuaFilePickerCombo(lua_State* L) {
    if (!lua_isstring(L, 1) || !lua_isstring(L, 3)) {
        lua_pushboolean(L, false);
        lua_pushstring(L, lua_isstring(L, 2) ? lua_tostring(L, 2) : "");
        return 2;
    }
    const char* label = lua_tostring(L, 1);
    std::string current_file = lua_isstring(L, 2) ? lua_tostring(L, 2) : "";
    const char* directory = lua_tostring(L, 3);
    const char* extensions = (lua_gettop(L) >= 4 && lua_isstring(L, 4)) ? lua_tostring(L, 4) : nullptr;
    ImGuiComboFlags flags = (lua_gettop(L) >= 5 && lua_isnumber(L, 5)) ? (ImGuiComboFlags)lua_tointeger(L, 5) : 0;

    bool changed = ImGui::FilePickerCombo(label, &current_file, directory, extensions, flags);
    lua_pushboolean(L, changed);
    lua_pushstring(L, current_file.c_str());
    return 2;
}

static int LuaOpenFileDialog(lua_State* L) {
    if (!lua_isstring(L, 1)) {
        lua_pushboolean(L, false);
        lua_pushnil(L);
        lua_pushboolean(L, false);
        return 3;
    }
    const char* title = lua_tostring(L, 1);
    bool is_open = lua_toboolean(L, 2);
    const char* starting_dir = (lua_gettop(L) >= 3 && lua_isstring(L, 3)) ? lua_tostring(L, 3) : ".";
    const char* extensions = (lua_gettop(L) >= 4 && lua_isstring(L, 4)) ? lua_tostring(L, 4) : nullptr;

    std::string chosen_path;
    bool confirmed = ImGui::OpenFileDialog(title, &is_open, &chosen_path, starting_dir, extensions);

    lua_pushboolean(L, confirmed);
    if (confirmed) {
        lua_pushstring(L, chosen_path.c_str());
    } else {
        lua_pushnil(L);
    }
    lua_pushboolean(L, is_open);
    return 3;
}

static int LuaSaveFileDialog(lua_State* L) {
    if (!lua_isstring(L, 1)) {
        lua_pushboolean(L, false);
        lua_pushnil(L);
        lua_pushboolean(L, false);
        return 3;
    }
    const char* title = lua_tostring(L, 1);
    bool is_open = lua_toboolean(L, 2);
    const char* starting_dir = (lua_gettop(L) >= 3 && lua_isstring(L, 3)) ? lua_tostring(L, 3) : ".";
    const char* default_filename = (lua_gettop(L) >= 4 && lua_isstring(L, 4)) ? lua_tostring(L, 4) : "";
    const char* extensions = (lua_gettop(L) >= 5 && lua_isstring(L, 5)) ? lua_tostring(L, 5) : nullptr;

    std::string save_path;
    bool confirmed = ImGui::SaveFileDialog(title, &is_open, &save_path, starting_dir, default_filename, extensions);

    lua_pushboolean(L, confirmed);
    if (confirmed) {
        lua_pushstring(L, save_path.c_str());
    } else {
        lua_pushnil(L);
    }
    lua_pushboolean(L, is_open);
    return 3;
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

static int LuaBadgeNamed(lua_State* L) {
    const char* imageName = luaL_checkstring(L, 1);
    if (!imageName) return 0;

    const char* text = nullptr;
    int col_arg = 2;
    if (lua_gettop(L) >= 2 && lua_isstring(L, 2) && !lua_isnumber(L, 2)) {
        text = lua_tostring(L, 2);
        col_arg = 3;
    }

    if (lua_gettop(L) >= col_arg && (lua_istable(L, col_arg) || (lua_isnumber(L, col_arg) && lua_tonumber(L, col_arg) > 10.0))) {
        ImU32 bg_col = ParseLuaColor(L, col_arg);
        ImU32 fg_col = (lua_gettop(L) >= col_arg + 1) ? ParseLuaColor(L, col_arg + 1) : 0;
        ImGui::BadgeNamed(imageName, text, bg_col, fg_col);
    } else {
        ImGui::BadgeNamed(imageName, text);
    }
    return 0;
}

static int LuaPillNamed(lua_State* L) {
    const char* imageName = luaL_checkstring(L, 1);
    if (!imageName) return 0;

    const char* text = nullptr;
    int col_arg = 2;
    if (lua_gettop(L) >= 2 && lua_isstring(L, 2) && !lua_isnumber(L, 2)) {
        text = lua_tostring(L, 2);
        col_arg = 3;
    }

    if (lua_gettop(L) >= col_arg && (lua_istable(L, col_arg) || (lua_isnumber(L, col_arg) && lua_tonumber(L, col_arg) > 10.0))) {
        ImU32 bg_col = ParseLuaColor(L, col_arg);
        ImU32 fg_col = (lua_gettop(L) >= col_arg + 1) ? ParseLuaColor(L, col_arg + 1) : 0;
        ImGui::PillNamed(imageName, text, bg_col, fg_col);
    } else {
        ImGui::PillNamed(imageName, text);
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
// 10. SelectPulldown Widget
// ----------------------------------------------------------------------------

static int LuaSelectPulldown(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    int current_item = (int)luaL_checkinteger(L, 2);
    if (!lua_istable(L, 3)) {
        return luaL_error(L, "SelectPulldown: items must be a table of strings");
    }

    std::vector<std::string> items;
    size_t num_items = lua_objlen(L, 3);
    items.reserve(num_items);
    for (size_t i = 1; i <= num_items; ++i) {
        lua_rawgeti(L, 3, (int)i);
        const char* s = lua_tostring(L, -1);
        items.push_back(s ? s : "");
        lua_pop(L, 1);
    }

    std::vector<std::string> tooltips;
    bool has_tooltips = false;
    float width = 0.0f;
    ImGuiPopupAnchor anchor = ImGuiPopupAnchor_BelowItem;
    ImGuiPopupAlign align_mode = ImGuiPopupAlign_Left;

    int top = lua_gettop(L);
    int arg_idx = 4;
    if (top >= arg_idx && lua_istable(L, arg_idx)) {
        has_tooltips = true;
        size_t num_tips = lua_objlen(L, arg_idx);
        tooltips.reserve(num_tips);
        for (size_t i = 1; i <= num_tips; ++i) {
            lua_rawgeti(L, arg_idx, (int)i);
            const char* t = lua_tostring(L, -1);
            tooltips.push_back(t ? t : "");
            lua_pop(L, 1);
        }
        arg_idx++;
    }
    if (top >= arg_idx && lua_isnumber(L, arg_idx)) {
        width = static_cast<float>(lua_tonumber(L, arg_idx));
        arg_idx++;
    }
    if (top >= arg_idx && !lua_isnil(L, arg_idx)) {
        anchor = static_cast<ImGuiPopupAnchor>(lua_tointeger(L, arg_idx));
        arg_idx++;
    }
    if (top >= arg_idx && !lua_isnil(L, arg_idx)) {
        align_mode = static_cast<ImGuiPopupAlign>(lua_tointeger(L, arg_idx));
        arg_idx++;
    }

    bool changed = ImGui::SelectPulldown(label, &current_item, items, has_tooltips ? &tooltips : nullptr, width, anchor, align_mode);
    lua_pushboolean(L, changed);
    lua_pushinteger(L, current_item);
    return 2;
}

static int LuaSetDefaultPulldownCaret(lua_State* L) {
    const char* down = (lua_gettop(L) >= 1 && !lua_isnil(L, 1)) ? lua_tostring(L, 1) : nullptr;
    const char* up   = (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) ? lua_tostring(L, 2) : nullptr;
    ImGui::SetDefaultPulldownCaret(down, up);
    return 0;
}

// ----------------------------------------------------------------------------
// 11. Directional Popup Anchoring & Menu Placement
// ----------------------------------------------------------------------------

static int LuaOpenPopupAtItem(lua_State* L) {
    const char* str_id = luaL_checkstring(L, 1);
    ImGuiPopupAnchor anchor = ImGuiPopupAnchor_Auto;
    ImGuiPopupAlign align_mode = ImGuiPopupAlign_Auto;
    ImGuiPopupFlags flags = 0;

    int top = lua_gettop(L);
    if (top >= 2 && !lua_isnil(L, 2)) {
        anchor = static_cast<ImGuiPopupAnchor>(lua_tointeger(L, 2));
    }
    if (top >= 3 && !lua_isnil(L, 3)) {
        align_mode = static_cast<ImGuiPopupAlign>(lua_tointeger(L, 3));
    }
    if (top >= 4 && !lua_isnil(L, 4)) {
        flags = static_cast<ImGuiPopupFlags>(lua_tointeger(L, 4));
    }

    bool opened = ImGui::OpenPopupAtItem(str_id, anchor, align_mode, flags);
    lua_pushboolean(L, opened);
    return 1;
}

static int LuaOpenPopupAtPos(lua_State* L) {
    const char* str_id = luaL_checkstring(L, 1);
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));
    ImGuiPopupAnchor anchor = ImGuiPopupAnchor_Auto;
    ImGuiPopupAlign align_mode = ImGuiPopupAlign_Auto;
    ImGuiPopupFlags flags = 0;

    int top = lua_gettop(L);
    if (top >= 4 && !lua_isnil(L, 4)) {
        anchor = static_cast<ImGuiPopupAnchor>(lua_tointeger(L, 4));
    }
    if (top >= 5 && !lua_isnil(L, 5)) {
        align_mode = static_cast<ImGuiPopupAlign>(lua_tointeger(L, 5));
    }
    if (top >= 6 && !lua_isnil(L, 6)) {
        flags = static_cast<ImGuiPopupFlags>(lua_tointeger(L, 6));
    }

    bool opened = ImGui::OpenPopupAtPos(str_id, ImVec2(x, y), anchor, align_mode, flags);
    lua_pushboolean(L, opened);
    return 1;
}

static int LuaBeginAnchoredPopup(lua_State* L) {
    const char* str_id = luaL_checkstring(L, 1);
    ImGuiWindowFlags flags = 0;
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        flags = static_cast<ImGuiWindowFlags>(lua_tointeger(L, 2));
    }
    bool open = ImGui::BeginAnchoredPopup(str_id, flags);
    lua_pushboolean(L, open);
    return 1;
}

static int LuaGetLastPopupHeight(lua_State* L) {
    const char* str_id = luaL_checkstring(L, 1);
    float h = ImGui::GetLastPopupHeight(str_id);
    lua_pushnumber(L, h);
    return 1;
}

static int LuaGetLastPopupSize(lua_State* L) {
    const char* str_id = luaL_checkstring(L, 1);
    ImVec2 sz = ImGui::GetLastPopupSize(str_id);
    lua_pushnumber(L, sz.x);
    lua_pushnumber(L, sz.y);
    return 2;
}

// ----------------------------------------------------------------------------
// 12. StyledText Inline Typography Engine
// ----------------------------------------------------------------------------

static int LuaStyledText(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::StyledTextUnformatted(s.c_str(), s.c_str() + s.size());
    return 0;
}

static int LuaStyledTextWrapped(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::Markdown::TextWrapped(s);
    return 0;
}

static int LuaStyledTextUnformatted(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    if (s) {
        ImGui::StyledTextUnformatted(s);
    }
    return 0;
}

static int LuaPushFontRole(lua_State* L) {
    int role = (int)luaL_checkinteger(L, 1);
    ImGui::PushFontRole(static_cast<ImGui::ImFontRole>(role));
    return 0;
}

static int LuaPopFontRole(lua_State* L) {
    int count = (lua_gettop(L) >= 1 && !lua_isnil(L, 1)) ? (int)lua_tointeger(L, 1) : 1;
    for (int i = 0; i < count; ++i) {
        ImGui::PopFontRole();
    }
    return 0;
}

static int LuaSmallText(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::SmallText("%s", s.c_str());
    return 0;
}

static int LuaSmallTextUnformatted(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    if (s) ImGui::SmallTextUnformatted(s);
    return 0;
}

static int LuaTinyText(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::TinyText("%s", s.c_str());
    return 0;
}

static int LuaTinyTextUnformatted(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    if (s) ImGui::TinyTextUnformatted(s);
    return 0;
}

static int LuaTitleText(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::TitleText("%s", s.c_str());
    return 0;
}

static int LuaTitleTextUnformatted(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    if (s) ImGui::TitleTextUnformatted(s);
    return 0;
}

static int LuaMonoText(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::MonoText("%s", s.c_str());
    return 0;
}

static int LuaMonoTextUnformatted(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    if (s) ImGui::MonoTextUnformatted(s);
    return 0;
}

// ----------------------------------------------------------------------------
// 14. In-Memory Image Service (ImgWindow)
// ----------------------------------------------------------------------------

static int LuaRegisterMemoryImage(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    size_t len = 0;
    const char* data = luaL_checklstring(L, 2, &len);
    if (!name || !data || len == 0) {
        lua_pushboolean(L, false);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        return 3;
    }
    int w = 0, h = 0;
    ImTextureID tex = ImgWindow::RegisterMemoryImage(name, data, len, &w, &h);
    lua_pushboolean(L, tex != (ImTextureID)0);
    lua_pushinteger(L, w);
    lua_pushinteger(L, h);
    return 3;
}

static int LuaUnregisterMemoryImage(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    if (name) {
        ImgWindow::UnregisterMemoryImage(name);
    }
    return 0;
}

static int LuaGetNamedTexture(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    ImVec2 sz(0.0f, 0.0f);
    ImTextureID tex = name ? ImgWindow::GetNamedTexture(name, &sz) : (ImTextureID)0;
    lua_pushboolean(L, tex != (ImTextureID)0);
    lua_pushnumber(L, sz.x);
    lua_pushnumber(L, sz.y);
    return 3;
}

static int LuaImageNamed(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    if (!name) return 0;
    float w = 0.0f, h = 0.0f;
    int top = lua_gettop(L);
    if (top >= 3 && lua_isnumber(L, 2) && lua_isnumber(L, 3)) {
        w = static_cast<float>(lua_tonumber(L, 2));
        h = static_cast<float>(lua_tonumber(L, 3));
    } else if (top >= 2 && lua_istable(L, 2)) {
        lua_rawgeti(L, 2, 1);
        w = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
        lua_rawgeti(L, 2, 2);
        h = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
    }
    ImGui::ImageNamed(name, ImVec2(w, h));
    return 0;
}

// ----------------------------------------------------------------------------
// 15. ImGee Themes & URL / Document Launcher
// ----------------------------------------------------------------------------

static int LuaStyleColorsCharcoal(lua_State* L) {
    (void)L;
    ImGui::StyleColorsCharcoal();
    return 0;
}

static int LuaStyleColorsRusty(lua_State* L) {
    (void)L;
    ImGui::StyleColorsRusty();
    return 0;
}

static int LuaApplyDefaultTheme(lua_State* L) {
    (void)L;
    ImGui::ApplyDefaultTheme();
    return 0;
}

static int LuaStyleColorsDark(lua_State* L) {
    (void)L;
    ImGui::StyleColorsDark();
    return 0;
}

static int LuaStyleColorsLight(lua_State* L) {
    (void)L;
    ImGui::StyleColorsLight();
    return 0;
}

static int LuaStyleColorsClassic(lua_State* L) {
    (void)L;
    ImGui::StyleColorsClassic();
    return 0;
}

static int LuaOpenURL(lua_State* L) {
    const char* target = luaL_checkstring(L, 1);
    bool ok = ImGui::OpenURL(target);
    lua_pushboolean(L, ok);
    return 1;
}

static int LuaOpenDocument(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    bool ok = ImGui::OpenDocument(path);
    lua_pushboolean(L, ok);
    return 1;
}

// Statically embedded 16x16 avionics radio beacon icon (emerald radar pulse rings):
static const unsigned char s_AvionicsBeaconPng[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x00,
    0x4f, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x63, 0x60, 0xa0, 0x36, 0xd0,
    0x3b, 0x53, 0x98, 0x82, 0x0f, 0x13, 0xa5, 0x99, 0x2c, 0x79, 0x64, 0x09,
    0x20, 0xfb, 0x0e, 0x36, 0x8c, 0x4d, 0x2d, 0x86, 0xc9, 0xe8, 0x8a, 0xd1,
    0x0d, 0xc5, 0xea, 0x12, 0x42, 0x9a, 0x71, 0x19, 0x82, 0xd5, 0x00, 0x24,
    0xb1, 0xff, 0xc8, 0x18, 0xd9, 0x90, 0xe1, 0x6e, 0x00, 0xd9, 0x81, 0x48,
    0x51, 0x34, 0xa2, 0x9b, 0x48, 0x72, 0x42, 0xc2, 0xe6, 0x12, 0x72, 0xe4,
    0x51, 0x14, 0x91, 0x95, 0x99, 0xc8, 0x01, 0x00, 0x3a, 0x65, 0xcf, 0x6d,
    0xc5, 0x47, 0x7e, 0x46, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
    0xae, 0x42, 0x60, 0x82
};

// ----------------------------------------------------------------------------
// Registration & Namespace / Alias Hierarchy Setup
// ----------------------------------------------------------------------------

void InitImGeeSupport(lua_State* L) {
    // Auto-register default built-in avionics beacon icon into named texture service:
    ImgWindow::RegisterMemoryImage("avionics_beacon", s_AvionicsBeaconPng, sizeof(s_AvionicsBeaconPng));

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
    lua_pushcfunction(L, LuaMarkdownButtonMD);          lua_setfield(L, -2, "ButtonMarkdown");
    lua_pushcfunction(L, LuaMarkdownButtonMD);          lua_setfield(L, -2, "button_markdown");

    // 6. ImGee Widgets directly on imgui table
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

    lua_pushcfunction(L, LuaIconButton);               lua_setfield(L, -2, "IconButton");
    lua_pushcfunction(L, LuaIconButton);               lua_setfield(L, -2, "icon_button");
    lua_pushcfunction(L, LuaButtonIcon);               lua_setfield(L, -2, "ButtonIcon");
    lua_pushcfunction(L, LuaButtonIcon);               lua_setfield(L, -2, "button_icon");

    lua_pushcfunction(L, LuaButtonLink);                lua_setfield(L, -2, "ButtonLink");
    lua_pushcfunction(L, LuaButtonLink);                lua_setfield(L, -2, "button_link");
    lua_pushcfunction(L, LuaSmallButtonLink);           lua_setfield(L, -2, "SmallButtonLink");
    lua_pushcfunction(L, LuaSmallButtonLink);           lua_setfield(L, -2, "small_button_link");
    lua_pushcfunction(L, LuaTextLink);                  lua_setfield(L, -2, "TextLink");
    lua_pushcfunction(L, LuaTextLink);                  lua_setfield(L, -2, "text_link");
    lua_pushcfunction(L, LuaAttachItemCursor);          lua_setfield(L, -2, "AttachItemCursor");
    lua_pushcfunction(L, LuaAttachItemCursor);          lua_setfield(L, -2, "attach_item_cursor");
    lua_pushcfunction(L, LuaInputTextGetSelection);     lua_setfield(L, -2, "GetInputTextSelection");
    lua_pushcfunction(L, LuaInputTextGetSelection);     lua_setfield(L, -2, "get_input_text_selection");
    lua_pushcfunction(L, LuaInputTextSetSelection);     lua_setfield(L, -2, "SetInputTextSelection");
    lua_pushcfunction(L, LuaInputTextSetSelection);     lua_setfield(L, -2, "set_input_text_selection");
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

    // 8. Universal Standalone & Chaser Copy Buttons
    lua_pushcfunction(L, LuaCopyButton);                lua_setfield(L, -2, "CopyButton");
    lua_pushcfunction(L, LuaCopyButton);                lua_setfield(L, -2, "copy_button");
    lua_pushcfunction(L, LuaCopyButton);                lua_setfield(L, -2, "CopyBufferButton");
    lua_pushcfunction(L, LuaCopyButton);                lua_setfield(L, -2, "copy_buffer_button");
    lua_pushcfunction(L, LuaCopyLastItemButton);        lua_setfield(L, -2, "CopyLastItemButton");
    lua_pushcfunction(L, LuaCopyLastItemButton);        lua_setfield(L, -2, "copy_last_item_button");
    lua_pushcfunction(L, LuaCopyLastItemButton);        lua_setfield(L, -2, "CopyTextFieldButton");
    lua_pushcfunction(L, LuaCopyLastItemButton);        lua_setfield(L, -2, "copy_text_field_button");

    lua_pushcfunction(L, LuaSetCursorHandling);         lua_setfield(L, -2, "SetCursorHandling");
    lua_pushcfunction(L, LuaSetCursorHandling);         lua_setfield(L, -2, "set_cursor_handling");
    lua_pushcfunction(L, LuaSetCursorHandling);         lua_setfield(L, -2, "EnableMouseCursors");
    lua_pushcfunction(L, LuaSetCursorHandling);         lua_setfield(L, -2, "enable_mouse_cursors");
    lua_pushcfunction(L, LuaGetCursorHandling);         lua_setfield(L, -2, "GetCursorHandling");
    lua_pushcfunction(L, LuaGetCursorHandling);         lua_setfield(L, -2, "get_cursor_handling");

    // 9. In-Cockpit File Dialogs & File Picker Suite
    lua_pushcfunction(L, LuaFilePickerCombo);            lua_setfield(L, -2, "FilePickerCombo");
    lua_pushcfunction(L, LuaFilePickerCombo);            lua_setfield(L, -2, "file_picker_combo");
    lua_pushcfunction(L, LuaFilePickerCombo);            lua_setfield(L, -2, "FilePicker");
    lua_pushcfunction(L, LuaFilePickerCombo);            lua_setfield(L, -2, "file_picker");

    lua_pushcfunction(L, LuaOpenFileDialog);             lua_setfield(L, -2, "OpenFileDialog");
    lua_pushcfunction(L, LuaOpenFileDialog);             lua_setfield(L, -2, "open_file_dialog");
    lua_pushcfunction(L, LuaOpenFileDialog);             lua_setfield(L, -2, "OpenDialog");
    lua_pushcfunction(L, LuaOpenFileDialog);             lua_setfield(L, -2, "open_dialog");

    lua_pushcfunction(L, LuaSaveFileDialog);             lua_setfield(L, -2, "SaveFileDialog");
    lua_pushcfunction(L, LuaSaveFileDialog);             lua_setfield(L, -2, "save_file_dialog");
    lua_pushcfunction(L, LuaSaveFileDialog);             lua_setfield(L, -2, "SaveDialog");
    lua_pushcfunction(L, LuaSaveFileDialog);             lua_setfield(L, -2, "save_dialog");

    // 10. SelectPulldown Widget
    lua_pushcfunction(L, LuaSelectPulldown);             lua_setfield(L, -2, "SelectPulldown");
    lua_pushcfunction(L, LuaSelectPulldown);             lua_setfield(L, -2, "select_pulldown");
    lua_pushcfunction(L, LuaSetDefaultPulldownCaret);    lua_setfield(L, -2, "SetDefaultPulldownCaret");
    lua_pushcfunction(L, LuaSetDefaultPulldownCaret);    lua_setfield(L, -2, "set_default_pulldown_caret");

    // 11. Directional Popup Anchoring & Menu Placement
    lua_pushcfunction(L, LuaOpenPopupAtItem);            lua_setfield(L, -2, "OpenPopupAtItem");
    lua_pushcfunction(L, LuaOpenPopupAtItem);            lua_setfield(L, -2, "open_popup_at_item");
    lua_pushcfunction(L, LuaOpenPopupAtPos);             lua_setfield(L, -2, "OpenPopupAtPos");
    lua_pushcfunction(L, LuaOpenPopupAtPos);             lua_setfield(L, -2, "open_popup_at_pos");
    lua_pushcfunction(L, LuaBeginAnchoredPopup);         lua_setfield(L, -2, "BeginAnchoredPopup");
    lua_pushcfunction(L, LuaBeginAnchoredPopup);         lua_setfield(L, -2, "begin_anchored_popup");
    lua_pushcfunction(L, LuaGetLastPopupHeight);         lua_setfield(L, -2, "GetLastPopupHeight");
    lua_pushcfunction(L, LuaGetLastPopupHeight);         lua_setfield(L, -2, "get_last_popup_height");
    lua_pushcfunction(L, LuaGetLastPopupSize);           lua_setfield(L, -2, "GetLastPopupSize");
    lua_pushcfunction(L, LuaGetLastPopupSize);           lua_setfield(L, -2, "get_last_popup_size");

    // PopupAnchor sub-table and constants
    lua_newtable(L);
    lua_pushinteger(L, ImGuiPopupAnchor_BelowItem); lua_setfield(L, -2, "BelowItem");
    lua_pushinteger(L, ImGuiPopupAnchor_AboveItem); lua_setfield(L, -2, "AboveItem");
    lua_pushinteger(L, ImGuiPopupAnchor_Auto);      lua_setfield(L, -2, "Auto");
    lua_setfield(L, -2, "PopupAnchor");

    lua_pushinteger(L, ImGuiPopupAnchor_BelowItem); lua_setfield(L, -2, "PopupAnchor_BelowItem");
    lua_pushinteger(L, ImGuiPopupAnchor_AboveItem); lua_setfield(L, -2, "PopupAnchor_AboveItem");
    lua_pushinteger(L, ImGuiPopupAnchor_Auto);      lua_setfield(L, -2, "PopupAnchor_Auto");
    lua_pushinteger(L, ImGuiPopupAnchor_BelowItem); lua_setfield(L, -2, "ImGuiPopupAnchor_BelowItem");
    lua_pushinteger(L, ImGuiPopupAnchor_AboveItem); lua_setfield(L, -2, "ImGuiPopupAnchor_AboveItem");
    lua_pushinteger(L, ImGuiPopupAnchor_Auto);      lua_setfield(L, -2, "ImGuiPopupAnchor_Auto");

    // PopupAlign sub-table and constants
    lua_newtable(L);
    lua_pushinteger(L, ImGuiPopupAlign_Left);   lua_setfield(L, -2, "Left");
    lua_pushinteger(L, ImGuiPopupAlign_Right);  lua_setfield(L, -2, "Right");
    lua_pushinteger(L, ImGuiPopupAlign_Center); lua_setfield(L, -2, "Center");
    lua_pushinteger(L, ImGuiPopupAlign_Auto);   lua_setfield(L, -2, "Auto");
    lua_setfield(L, -2, "PopupAlign");

    lua_pushinteger(L, ImGuiPopupAlign_Left);   lua_setfield(L, -2, "PopupAlign_Left");
    lua_pushinteger(L, ImGuiPopupAlign_Right);  lua_setfield(L, -2, "PopupAlign_Right");
    lua_pushinteger(L, ImGuiPopupAlign_Center); lua_setfield(L, -2, "PopupAlign_Center");
    lua_pushinteger(L, ImGuiPopupAlign_Auto);   lua_setfield(L, -2, "PopupAlign_Auto");
    lua_pushinteger(L, ImGuiPopupAlign_Left);   lua_setfield(L, -2, "ImGuiPopupAlign_Left");
    lua_pushinteger(L, ImGuiPopupAlign_Right);  lua_setfield(L, -2, "ImGuiPopupAlign_Right");
    lua_pushinteger(L, ImGuiPopupAlign_Center); lua_setfield(L, -2, "ImGuiPopupAlign_Center");
    lua_pushinteger(L, ImGuiPopupAlign_Auto);   lua_setfield(L, -2, "ImGuiPopupAlign_Auto");

    // 12. StyledText Inline Typography Engine
    lua_pushcfunction(L, LuaStyledText);                 lua_setfield(L, -2, "StyledText");
    lua_pushcfunction(L, LuaStyledText);                 lua_setfield(L, -2, "styled_text");
    lua_pushcfunction(L, LuaStyledText);                 lua_setfield(L, -2, "TextStyled");
    lua_pushcfunction(L, LuaStyledText);                 lua_setfield(L, -2, "text_styled");
    lua_pushcfunction(L, LuaStyledTextWrapped);          lua_setfield(L, -2, "StyledTextWrapped");
    lua_pushcfunction(L, LuaStyledTextWrapped);          lua_setfield(L, -2, "styled_text_wrapped");
    lua_pushcfunction(L, LuaStyledTextWrapped);          lua_setfield(L, -2, "TextStyledWrapped");
    lua_pushcfunction(L, LuaStyledTextWrapped);          lua_setfield(L, -2, "text_styled_wrapped");
    lua_pushcfunction(L, LuaStyledTextUnformatted);      lua_setfield(L, -2, "StyledTextUnformatted");
    lua_pushcfunction(L, LuaStyledTextUnformatted);      lua_setfield(L, -2, "styled_text_unformatted");
    lua_pushcfunction(L, LuaStyledTextUnformatted);      lua_setfield(L, -2, "TextStyledUnformatted");
    lua_pushcfunction(L, LuaStyledTextUnformatted);      lua_setfield(L, -2, "text_styled_unformatted");

    lua_pushcfunction(L, LuaPushFontRole);               lua_setfield(L, -2, "PushFontRole");
    lua_pushcfunction(L, LuaPushFontRole);               lua_setfield(L, -2, "push_font_role");
    lua_pushcfunction(L, LuaPopFontRole);                lua_setfield(L, -2, "PopFontRole");
    lua_pushcfunction(L, LuaPopFontRole);                lua_setfield(L, -2, "pop_font_role");

    lua_pushcfunction(L, LuaSmallText);                  lua_setfield(L, -2, "SmallText");
    lua_pushcfunction(L, LuaSmallText);                  lua_setfield(L, -2, "small_text");
    lua_pushcfunction(L, LuaSmallTextUnformatted);       lua_setfield(L, -2, "SmallTextUnformatted");
    lua_pushcfunction(L, LuaSmallTextUnformatted);       lua_setfield(L, -2, "small_text_unformatted");

    lua_pushcfunction(L, LuaTinyText);                   lua_setfield(L, -2, "TinyText");
    lua_pushcfunction(L, LuaTinyText);                   lua_setfield(L, -2, "tiny_text");
    lua_pushcfunction(L, LuaTinyTextUnformatted);        lua_setfield(L, -2, "TinyTextUnformatted");
    lua_pushcfunction(L, LuaTinyTextUnformatted);        lua_setfield(L, -2, "tiny_text_unformatted");

    lua_pushcfunction(L, LuaTitleText);                  lua_setfield(L, -2, "TitleText");
    lua_pushcfunction(L, LuaTitleText);                  lua_setfield(L, -2, "title_text");
    lua_pushcfunction(L, LuaTitleTextUnformatted);       lua_setfield(L, -2, "TitleTextUnformatted");
    lua_pushcfunction(L, LuaTitleTextUnformatted);       lua_setfield(L, -2, "title_text_unformatted");

    lua_pushcfunction(L, LuaMonoText);                   lua_setfield(L, -2, "MonoText");
    lua_pushcfunction(L, LuaMonoText);                   lua_setfield(L, -2, "mono_text");
    lua_pushcfunction(L, LuaMonoTextUnformatted);        lua_setfield(L, -2, "MonoTextUnformatted");
    lua_pushcfunction(L, LuaMonoTextUnformatted);        lua_setfield(L, -2, "mono_text_unformatted");

    // FontRole sub-table and constants
    lua_newtable(L);
    lua_pushinteger(L, (int)ImGui::ImFontRole::Regular);         lua_setfield(L, -2, "Regular");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Title);           lua_setfield(L, -2, "Title");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Small);           lua_setfield(L, -2, "Small");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Bold);            lua_setfield(L, -2, "Bold");
    lua_pushinteger(L, (int)ImGui::ImFontRole::BoldLarge);       lua_setfield(L, -2, "BoldLarge");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Monospace);       lua_setfield(L, -2, "Monospace");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Italic);          lua_setfield(L, -2, "Italic");
    lua_pushinteger(L, (int)ImGui::ImFontRole::MonospaceMedium); lua_setfield(L, -2, "MonospaceMedium");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Heading1);        lua_setfield(L, -2, "Heading1");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Heading2);        lua_setfield(L, -2, "Heading2");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Heading3);        lua_setfield(L, -2, "Heading3");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Tiny);            lua_setfield(L, -2, "Tiny");
    lua_setfield(L, -2, "FontRole");

    lua_pushinteger(L, (int)ImGui::ImFontRole::Regular);         lua_setfield(L, -2, "ImFontRole_Regular");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Title);           lua_setfield(L, -2, "ImFontRole_Title");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Small);           lua_setfield(L, -2, "ImFontRole_Small");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Bold);            lua_setfield(L, -2, "ImFontRole_Bold");
    lua_pushinteger(L, (int)ImGui::ImFontRole::BoldLarge);       lua_setfield(L, -2, "ImFontRole_BoldLarge");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Monospace);       lua_setfield(L, -2, "ImFontRole_Monospace");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Italic);          lua_setfield(L, -2, "ImFontRole_Italic");
    lua_pushinteger(L, (int)ImGui::ImFontRole::MonospaceMedium); lua_setfield(L, -2, "ImFontRole_MonospaceMedium");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Heading1);        lua_setfield(L, -2, "ImFontRole_Heading1");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Heading2);        lua_setfield(L, -2, "ImFontRole_Heading2");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Heading3);        lua_setfield(L, -2, "ImFontRole_Heading3");
    lua_pushinteger(L, (int)ImGui::ImFontRole::Tiny);            lua_setfield(L, -2, "ImFontRole_Tiny");

    // 13. Named Badge & Pill Widgets
    lua_pushcfunction(L, LuaBadgeNamed);                lua_setfield(L, -2, "BadgeNamed");
    lua_pushcfunction(L, LuaBadgeNamed);                lua_setfield(L, -2, "badge_named");
    lua_pushcfunction(L, LuaPillNamed);                 lua_setfield(L, -2, "PillNamed");
    lua_pushcfunction(L, LuaPillNamed);                 lua_setfield(L, -2, "pill_named");

    // 14. In-Memory Image Service (ImgWindow)
    lua_pushcfunction(L, LuaRegisterMemoryImage);       lua_setfield(L, -2, "RegisterMemoryImage");
    lua_pushcfunction(L, LuaRegisterMemoryImage);       lua_setfield(L, -2, "register_memory_image");
    lua_pushcfunction(L, LuaUnregisterMemoryImage);     lua_setfield(L, -2, "UnregisterMemoryImage");
    lua_pushcfunction(L, LuaUnregisterMemoryImage);     lua_setfield(L, -2, "unregister_memory_image");
    lua_pushcfunction(L, LuaUnregisterMemoryImage);     lua_setfield(L, -2, "DeleteMemoryImage");
    lua_pushcfunction(L, LuaUnregisterMemoryImage);     lua_setfield(L, -2, "delete_memory_image");
    lua_pushcfunction(L, LuaGetNamedTexture);           lua_setfield(L, -2, "GetNamedTexture");
    lua_pushcfunction(L, LuaGetNamedTexture);           lua_setfield(L, -2, "get_named_texture");
    lua_pushcfunction(L, LuaGetNamedTexture);           lua_setfield(L, -2, "HasMemoryImage");
    lua_pushcfunction(L, LuaGetNamedTexture);           lua_setfield(L, -2, "has_memory_image");
    lua_pushcfunction(L, LuaImageNamed);                lua_setfield(L, -2, "ImageNamed");
    lua_pushcfunction(L, LuaImageNamed);                lua_setfield(L, -2, "image_named");

    // 15. ImGee Themes & Universal URL / Document Launcher
    lua_pushcfunction(L, LuaStyleColorsCharcoal);        lua_setfield(L, -2, "StyleColorsCharcoal");
    lua_pushcfunction(L, LuaStyleColorsCharcoal);        lua_setfield(L, -2, "style_colors_charcoal");
    lua_pushcfunction(L, LuaStyleColorsRusty);           lua_setfield(L, -2, "StyleColorsRusty");
    lua_pushcfunction(L, LuaStyleColorsRusty);           lua_setfield(L, -2, "style_colors_rusty");
    lua_pushcfunction(L, LuaApplyDefaultTheme);          lua_setfield(L, -2, "ApplyDefaultTheme");
    lua_pushcfunction(L, LuaApplyDefaultTheme);          lua_setfield(L, -2, "apply_default_theme");
    lua_pushcfunction(L, LuaStyleColorsDark);            lua_setfield(L, -2, "StyleColorsDark");
    lua_pushcfunction(L, LuaStyleColorsDark);            lua_setfield(L, -2, "style_colors_dark");
    lua_pushcfunction(L, LuaStyleColorsLight);           lua_setfield(L, -2, "StyleColorsLight");
    lua_pushcfunction(L, LuaStyleColorsLight);           lua_setfield(L, -2, "style_colors_light");
    lua_pushcfunction(L, LuaStyleColorsClassic);         lua_setfield(L, -2, "StyleColorsClassic");
    lua_pushcfunction(L, LuaStyleColorsClassic);         lua_setfield(L, -2, "style_colors_classic");

    lua_pushcfunction(L, LuaOpenURL);                    lua_setfield(L, -2, "OpenURL");
    lua_pushcfunction(L, LuaOpenURL);                    lua_setfield(L, -2, "open_url");
    lua_pushcfunction(L, LuaOpenDocument);               lua_setfield(L, -2, "OpenDocument");
    lua_pushcfunction(L, LuaOpenDocument);               lua_setfield(L, -2, "open_document");

    // 7. Global convenience aliases (matching imgee-global.h in C++)
    lua_getfield(L, -1, "TimedTooltip");
    lua_setglobal(L, "TimedTooltip");

    lua_getfield(L, -1, "Markdown");
    lua_setglobal(L, "Markdown");

    lua_getfield(L, -1, "MD");
    lua_setglobal(L, "MD");

    lua_pop(L, 1); // Pop imgui table
}

} // namespace flywithlua
