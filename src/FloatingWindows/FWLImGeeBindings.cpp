#include "FWLImGeeBindings.h"
#include "FWLIntegration.h"
#include "FlyWithLuaWindow.h"
#include "FWLDocWindow.h"
#include "../FlyWithLuaImGui.h"
#include "../FlyWithLua.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "contrib.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <cctype>
#include <cstdint>
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

// Format Lua string starting at stack index start_idx: handles single string literal,
// or variadic format string + arguments via Lua string.format
static std::string FormatLuaStringFrom(lua_State* L, int start_idx) {
    int top = lua_gettop(L);
    int num_args = top - start_idx + 1;
    if (num_args <= 0) {
        return "";
    }
    if (num_args == 1) {
        const char* s = lua_tostring(L, start_idx);
        return s ? s : "";
    }

    // Call Lua's string.format with arguments [start_idx .. top]
    lua_getglobal(L, "string");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        const char* s = lua_tostring(L, start_idx);
        return s ? s : "";
    }
    lua_getfield(L, -1, "format");
    lua_remove(L, -2); // Remove "string" table

    for (int i = start_idx; i <= top; ++i) {
        lua_pushvalue(L, i);
    }

    if (lua_pcall(L, num_args, 1, 0) != 0) {
        // Formatting error: pop error message and return raw format argument
        lua_pop(L, 1);
        const char* raw = lua_tostring(L, start_idx);
        return raw ? raw : "";
    }

    const char* formatted = lua_tostring(L, -1);
    std::string res = formatted ? formatted : "";
    lua_pop(L, 1);
    return res;
}

static std::string FormatLuaString(lua_State* L) {
    return FormatLuaStringFrom(L, 1);
}

// Shared Lua color parser: accepts RGBA table {r, g, b, [a]}, packed ImU32 number, or CSS hex/named string
static bool ParseLuaColorVec4(lua_State* L, int idx, ImVec4& out_col) {
    if (lua_gettop(L) < idx || lua_isnil(L, idx)) return false;
    int t = lua_type(L, idx);
    if (t == LUA_TTABLE) {
        float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
        lua_rawgeti(L, idx, 1); if (lua_isnumber(L, -1)) r = static_cast<float>(lua_tonumber(L, -1)); lua_pop(L, 1);
        lua_rawgeti(L, idx, 2); if (lua_isnumber(L, -1)) g = static_cast<float>(lua_tonumber(L, -1)); lua_pop(L, 1);
        lua_rawgeti(L, idx, 3); if (lua_isnumber(L, -1)) b = static_cast<float>(lua_tonumber(L, -1)); lua_pop(L, 1);
        lua_rawgeti(L, idx, 4); if (lua_isnumber(L, -1)) a = static_cast<float>(lua_tonumber(L, -1)); lua_pop(L, 1);
        out_col = ImVec4(r, g, b, a);
        return true;
    }
    if (t == LUA_TNUMBER) {
        out_col = ImGui::ColorConvertU32ToFloat4(static_cast<ImU32>(lua_tonumber(L, idx)));
        return true;
    }
    if (t == LUA_TSTRING) {
        const char* s = lua_tostring(L, idx);
        if (s && s[0] != '\0' && ImGui::ParseMarkdownColor(s, out_col)) {
            return true;
        }
    }
    return false;
}

static ImU32 ParseLuaColor(lua_State* L, int idx) {
    if (lua_gettop(L) < idx || lua_isnil(L, idx)) return 0;
    if (lua_type(L, idx) == LUA_TNUMBER) {
        return static_cast<ImU32>(lua_tonumber(L, idx));
    }
    ImVec4 v(0.0f, 0.0f, 0.0f, 0.0f);
    if (ParseLuaColorVec4(L, idx, v)) {
        return ImGui::ColorConvertFloat4ToU32(v);
    }
    return 0;
}

// Parse combo items from either a 1-based Lua table of strings or a NUL-separated Lua string
static bool ParseComboItemsFromLua(lua_State* L, int idx, std::vector<std::string>& storage, std::vector<const char*>& ptrs) {
    storage.clear();
    ptrs.clear();
    if (lua_istable(L, idx)) {
        size_t n = lua_objlen(L, idx);
        storage.reserve(n);
        for (size_t i = 1; i <= n; ++i) {
            lua_rawgeti(L, idx, static_cast<int>(i));
            const char* s = lua_tostring(L, -1);
            storage.push_back(s ? s : "");
            lua_pop(L, 1);
        }
        ptrs.reserve(storage.size());
        for (const auto& s : storage) ptrs.push_back(s.c_str());
        return !ptrs.empty();
    }
    if (lua_type(L, idx) == LUA_TSTRING) {
        size_t len = 0;
        const char* raw = lua_tolstring(L, idx, &len);
        if (!raw || len == 0) return false;
        size_t pos = 0;
        while (pos < len) {
            const char* seg = raw + pos;
            size_t slen = std::strlen(seg);
            if (slen == 0 && pos > 0) break; // terminating double-NUL
            storage.emplace_back(seg, slen);
            pos += slen + 1;
        }
        ptrs.reserve(storage.size());
        for (const auto& s : storage) ptrs.push_back(s.c_str());
        return !ptrs.empty();
    }
    return false;
}

static bool ParseOptionalStringTable(lua_State* L, int idx, std::vector<std::string>& storage, std::vector<const char*>& ptrs, size_t min_len = 0) {
    storage.clear();
    ptrs.clear();
    if (lua_gettop(L) < idx || !lua_istable(L, idx)) return false;
    size_t n = lua_objlen(L, idx);
    size_t cap = (n > min_len) ? n : min_len;
    storage.reserve(cap);
    for (size_t i = 1; i <= n; ++i) {
        lua_rawgeti(L, idx, static_cast<int>(i));
        const char* s = lua_tostring(L, -1);
        storage.push_back(s ? s : "");
        lua_pop(L, 1);
    }
    while (storage.size() < min_len) {
        storage.emplace_back("");
    }
    ptrs.reserve(storage.size());
    for (const auto& s : storage) ptrs.push_back(s.c_str());
    return !ptrs.empty();
}

static bool IsIntegerPrintfFormat(const char* fmt) {
    if (!fmt) return false;
    for (const char* p = fmt; *p; ++p) {
        if (*p != '%') continue;
        ++p;
        if (*p == '%') continue;
        while (*p && (std::isdigit(static_cast<unsigned char>(*p)) || *p == '-' || *p == '+' || *p == ' ' || *p == '#' || *p == '.')) {
            ++p;
        }
        while (*p == 'h' || *p == 'l' || *p == 'z' || *p == 't') ++p;
        return (*p == 'd' || *p == 'i' || *p == 'u' || *p == 'x' || *p == 'X' || *p == 'o');
    }
    return false;
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
static ImGui::MarkdownConfig s_FWLMarkdownConfig;
static std::string s_GlobalMarkdownBaseDir;
static bool s_HasGlobalMarkdownBaseDir = false;
static const char* s_ActiveWindowAutoBaseDir = nullptr;
static const char* s_ActiveWindowExplicitBaseDir = nullptr;

static void RefreshEffectiveMarkdownBaseDir() {
    if (s_ActiveWindowExplicitBaseDir != nullptr) {
        s_FWLMarkdownConfig.baseDir = (s_ActiveWindowExplicitBaseDir[0] != '\0') ? s_ActiveWindowExplicitBaseDir : nullptr;
    } else if (s_HasGlobalMarkdownBaseDir) {
        s_FWLMarkdownConfig.baseDir = s_GlobalMarkdownBaseDir.empty() ? nullptr : s_GlobalMarkdownBaseDir.c_str();
    } else if (s_ActiveWindowAutoBaseDir != nullptr && s_ActiveWindowAutoBaseDir[0] != '\0') {
        s_FWLMarkdownConfig.baseDir = s_ActiveWindowAutoBaseDir;
    } else {
        s_FWLMarkdownConfig.baseDir = nullptr;
    }
}

ScopedMarkdownWindowBaseDir::ScopedMarkdownWindowBaseDir(const char* auto_base_dir, const char* explicit_base_dir)
    : m_prev_base_dir(s_FWLMarkdownConfig.baseDir)
{
    s_ActiveWindowAutoBaseDir = auto_base_dir;
    s_ActiveWindowExplicitBaseDir = explicit_base_dir;
    RefreshEffectiveMarkdownBaseDir();
}

ScopedMarkdownWindowBaseDir::~ScopedMarkdownWindowBaseDir() {
    s_ActiveWindowAutoBaseDir = nullptr;
    s_ActiveWindowExplicitBaseDir = nullptr;
    if (s_HasGlobalMarkdownBaseDir) {
        s_FWLMarkdownConfig.baseDir = s_GlobalMarkdownBaseDir.empty() ? nullptr : s_GlobalMarkdownBaseDir.c_str();
    } else {
        s_FWLMarkdownConfig.baseDir = m_prev_base_dir;
    }
}

static ImGui::TimedTooltip::TooltipConfig& GetFWLTooltipConfig() {
    if (!s_FWLTooltipConfigInitialized) {
        s_FWLTooltipConfig = ImGui::TimedTooltip::GetCurrentTooltipConfig();
        s_FWLTooltipConfigInitialized = true;
        ImGui::TimedTooltip::SetDefaultTooltipConfig(&s_FWLTooltipConfig);
    }
    return s_FWLTooltipConfig;
}

void UpdateImGeeMarkdownFonts() {
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
    int top = lua_gettop(L);
    const char* raw = luaL_checkstring(L, 1);
    float wrap_width = (top >= 2 && lua_isnumber(L, 2)) ? static_cast<float>(lua_tonumber(L, 2)) : 0.0f;
    float font_scale = (top >= 3 && lua_isnumber(L, 3)) ? static_cast<float>(lua_tonumber(L, 3)) : 0.0f;
    ImGuiID hash_id = ImGui::GetID("##FWL_MD_CALC");
    ImVec2 sz = ImGui::Markdown::CalcTextSize(hash_id, 0, std::string(raw ? raw : ""), wrap_width, font_scale);
    lua_pushnumber(L, sz.x);
    lua_pushnumber(L, sz.y);
    return 2;
}

static int LuaMarkdownDelayedTooltipMD(lua_State* L) {
    int top = lua_gettop(L);
    const char* md = luaL_checkstring(L, 1);
    float wrap_width = (top >= 2 && lua_isnumber(L, 2)) ? static_cast<float>(lua_tonumber(L, 2)) : 400.0f;
    int delay_frames = (top >= 3 && lua_isnumber(L, 3)) ? static_cast<int>(lua_tointeger(L, 3)) : 30;
    bool allowed     = (top >= 4 && !lua_isnil(L, 4)) ? (lua_toboolean(L, 4) != 0) : true;
    bool shown = ImGui::Markdown::DelayedTooltipMD(md ? md : "", wrap_width, delay_frames, allowed);
    lua_pushboolean(L, shown);
    return 1;
}

static int LuaMarkdownTextLink(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::LinkClickResult res = ImGui::TextMDLink(s);
    // Preserve unconsumed click-to-toggle state unless caller passed a bare URL handled elsewhere
    lua_pushboolean(L, res.clicked);
    return 1;
}

static int LuaMarkdownSetBaseDir(lua_State* L) {
    if (lua_gettop(L) < 1 || lua_isnil(L, 1)) {
        s_GlobalMarkdownBaseDir.clear();
        s_HasGlobalMarkdownBaseDir = false;
    } else {
        const char* dir = luaL_checkstring(L, 1);
        s_GlobalMarkdownBaseDir = dir ? dir : "";
        s_HasGlobalMarkdownBaseDir = true;
    }
    RefreshEffectiveMarkdownBaseDir();
    return 0;
}

static int LuaMarkdownGetBaseDir(lua_State* L) {
    if (s_FWLMarkdownConfig.baseDir && s_FWLMarkdownConfig.baseDir[0] != '\0') {
        lua_pushstring(L, s_FWLMarkdownConfig.baseDir);
    } else if (s_HasGlobalMarkdownBaseDir && !s_GlobalMarkdownBaseDir.empty()) {
        lua_pushstring(L, s_GlobalMarkdownBaseDir.c_str());
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int LuaMarkdownSetWindowBaseDir(lua_State* L) {
    if (!lua_islightuserdata(L, 1)) {
        return luaL_error(L, "SetWindowBaseDir: expected floating window handle as argument 1");
    }
    flwnd::FloatingWindow* wnd = flwnd::GetValidFloatingWindow(lua_touserdata(L, 1));
    auto* fwlWnd = dynamic_cast<flwnd::FlyWithLuaWindow*>(wnd);
    if (!fwlWnd) {
        return 0;
    }
    if (lua_gettop(L) < 2 || lua_isnil(L, 2)) {
        fwlWnd->clearExplicitMarkdownBaseDir();
        if (s_ActiveWindowExplicitBaseDir != nullptr) {
            s_ActiveWindowExplicitBaseDir = nullptr;
            RefreshEffectiveMarkdownBaseDir();
        }
    } else {
        const char* dir = luaL_checkstring(L, 2);
        fwlWnd->setExplicitMarkdownBaseDir(dir ? dir : "");
        if (s_ActiveWindowAutoBaseDir != nullptr || s_ActiveWindowExplicitBaseDir != nullptr) {
            s_ActiveWindowExplicitBaseDir = fwlWnd->getExplicitMarkdownBaseDir().c_str();
            RefreshEffectiveMarkdownBaseDir();
        }
    }
    return 0;
}

static int LuaMarkdownGetWindowBaseDir(lua_State* L) {
    if (!lua_islightuserdata(L, 1)) {
        lua_pushnil(L);
        return 1;
    }
    flwnd::FloatingWindow* wnd = flwnd::GetValidFloatingWindow(lua_touserdata(L, 1));
    auto* fwlWnd = dynamic_cast<flwnd::FlyWithLuaWindow*>(wnd);
    if (!fwlWnd) {
        lua_pushnil(L);
        return 1;
    }
    if (fwlWnd->hasExplicitMarkdownBaseDir()) {
        if (fwlWnd->getExplicitMarkdownBaseDir().empty()) lua_pushnil(L);
        else lua_pushstring(L, fwlWnd->getExplicitMarkdownBaseDir().c_str());
    } else if (!fwlWnd->getAutoMarkdownBaseDir().empty()) {
        lua_pushstring(L, fwlWnd->getAutoMarkdownBaseDir().c_str());
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int LuaMarkdownSetShowImageCaptions(lua_State* L) {
    s_FWLMarkdownConfig.showImageCaptions = (lua_gettop(L) < 1) ? true : (lua_toboolean(L, 1) != 0);
    return 0;
}

static int LuaMarkdownGetShowImageCaptions(lua_State* L) {
    lua_pushboolean(L, s_FWLMarkdownConfig.showImageCaptions);
    return 1;
}

static int LuaMarkdownSetEnableImageLightbox(lua_State* L) {
    s_FWLMarkdownConfig.enableImageLightbox = (lua_gettop(L) < 1) ? true : (lua_toboolean(L, 1) != 0);
    return 0;
}

static int LuaMarkdownGetEnableImageLightbox(lua_State* L) {
    lua_pushboolean(L, s_FWLMarkdownConfig.enableImageLightbox);
    return 1;
}

static int LuaMarkdownSetEnableTableTooltips(lua_State* L) {
    s_FWLMarkdownConfig.enableTableTooltips = (lua_gettop(L) < 1) ? true : (lua_toboolean(L, 1) != 0);
    return 0;
}

static int LuaMarkdownGetEnableTableTooltips(lua_State* L) {
    lua_pushboolean(L, s_FWLMarkdownConfig.enableTableTooltips);
    return 1;
}

static int LuaMarkdownSetShowCodeCopyButton(lua_State* L) {
    s_FWLMarkdownConfig.showCodeCopyButton = (lua_gettop(L) < 1) ? true : (lua_toboolean(L, 1) != 0);
    return 0;
}

static int LuaMarkdownGetShowCodeCopyButton(lua_State* L) {
    lua_pushboolean(L, s_FWLMarkdownConfig.showCodeCopyButton);
    return 1;
}

static int LuaMarkdownConfigure(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "base_dir");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_getfield(L, 1, "baseDir");
    }
    if (lua_isstring(L, -1)) {
        s_GlobalMarkdownBaseDir = lua_tostring(L, -1);
        s_HasGlobalMarkdownBaseDir = true;
        RefreshEffectiveMarkdownBaseDir();
    }
    lua_pop(L, 1);

    auto readBoolOpt = [&](const char* k1, const char* k2, bool& target) {
        lua_getfield(L, 1, k1);
        if (lua_isnil(L, -1) && k2) {
            lua_pop(L, 1);
            lua_getfield(L, 1, k2);
        }
        if (lua_isboolean(L, -1)) {
            target = (lua_toboolean(L, -1) != 0);
        }
        lua_pop(L, 1);
    };
    readBoolOpt("show_image_captions", "showImageCaptions", s_FWLMarkdownConfig.showImageCaptions);
    readBoolOpt("enable_image_lightbox", "enableImageLightbox", s_FWLMarkdownConfig.enableImageLightbox);
    readBoolOpt("enable_table_tooltips", "enableTableTooltips", s_FWLMarkdownConfig.enableTableTooltips);
    readBoolOpt("show_code_copy_button", "showCodeCopyButton", s_FWLMarkdownConfig.showCodeCopyButton);
    return 0;
}

static int LuaMarkdownClearImageCache(lua_State* L) {
    (void)L;
#if defined(IMGUI_MARKDOWN_IMAGE_SERVICE_H)
    ImGui::MarkdownImageService::ClearTextureCache();
#endif
    return 0;
}

// Markdown Tag Registry & Dynamic Callback Management
static std::unordered_map<std::string, int> s_LuaCustomTagRefs;
static lua_State* s_CurrentLuaState = nullptr;

static int LuaRegisterMarkdownStyleTag(lua_State* L) {
    const char* raw_tag = luaL_checkstring(L, 1);
    const char* style = luaL_checkstring(L, 2);
    if (!raw_tag || raw_tag[0] == '\0') {
        lua_pushboolean(L, false);
        return 1;
    }
    std::string tag_name = raw_tag;
    for (char& c : tag_name) c = (char)tolower((unsigned char)c);

    // Clean up any existing Lua callback ref for this tag
    auto it = s_LuaCustomTagRefs.find(tag_name);
    if (it != s_LuaCustomTagRefs.end()) {
        if (s_CurrentLuaState && it->second != LUA_NOREF) {
            luaL_unref(s_CurrentLuaState, LUA_REGISTRYINDEX, it->second);
        }
        s_LuaCustomTagRefs.erase(it);
    }

    ImGui::RegisterMarkdownStyleTag(tag_name, style ? style : "");
    lua_pushboolean(L, true);
    return 1;
}

static int LuaRegisterMarkdownTagAlias(lua_State* L) {
    const char* raw_tag = luaL_checkstring(L, 1);
    const char* tmpl = luaL_checkstring(L, 2);
    if (!raw_tag || raw_tag[0] == '\0') {
        lua_pushboolean(L, false);
        return 1;
    }
    std::string tag_name = raw_tag;
    for (char& c : tag_name) c = (char)tolower((unsigned char)c);

    // Clean up any existing Lua callback ref for this tag
    auto it = s_LuaCustomTagRefs.find(tag_name);
    if (it != s_LuaCustomTagRefs.end()) {
        if (s_CurrentLuaState && it->second != LUA_NOREF) {
            luaL_unref(s_CurrentLuaState, LUA_REGISTRYINDEX, it->second);
        }
        s_LuaCustomTagRefs.erase(it);
    }

    ImGui::RegisterMarkdownTagAlias(tag_name, tmpl ? tmpl : "");
    lua_pushboolean(L, true);
    return 1;
}

static int LuaRegisterMarkdownTag(lua_State* L) {
    const char* raw_tag = luaL_checkstring(L, 1);
    if (!raw_tag || raw_tag[0] == '\0') {
        lua_pushboolean(L, false);
        return 1;
    }
    std::string tag_name = raw_tag;
    for (char& c : tag_name) c = (char)tolower((unsigned char)c);

    if (lua_isfunction(L, 2)) {
        auto it = s_LuaCustomTagRefs.find(tag_name);
        if (it != s_LuaCustomTagRefs.end() && it->second != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, it->second);
        }

        lua_pushvalue(L, 2);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        s_LuaCustomTagRefs[tag_name] = ref;
        s_CurrentLuaState = L;

        ImGui::RegisterMarkdownTag(tag_name, [tag_name](const std::string& inner_text, const std::string& param) {
            if (!s_CurrentLuaState) return;
            auto fit = s_LuaCustomTagRefs.find(tag_name);
            if (fit == s_LuaCustomTagRefs.end() || fit->second == LUA_NOREF) return;

            lua_rawgeti(s_CurrentLuaState, LUA_REGISTRYINDEX, fit->second);
            if (!lua_isfunction(s_CurrentLuaState, -1)) {
                lua_pop(s_CurrentLuaState, 1);
                return;
            }
            lua_pushlstring(s_CurrentLuaState, inner_text.c_str(), inner_text.length());
            lua_pushlstring(s_CurrentLuaState, param.c_str(), param.length());
            if (lua_pcall(s_CurrentLuaState, 2, 0, 0) != 0) {
                lua_pop(s_CurrentLuaState, 1);
            }
        });
        lua_pushboolean(L, true);
        return 1;
    } else if (lua_isstring(L, 2)) {
        const char* def = lua_tostring(L, 2);
        std::string s_def = def ? def : "";

        auto it = s_LuaCustomTagRefs.find(tag_name);
        if (it != s_LuaCustomTagRefs.end()) {
            if (s_CurrentLuaState && it->second != LUA_NOREF) {
                luaL_unref(s_CurrentLuaState, LUA_REGISTRYINDEX, it->second);
            }
            s_LuaCustomTagRefs.erase(it);
        }

        if (s_def.find("%s") != std::string::npos) {
            ImGui::RegisterMarkdownTagAlias(tag_name, s_def);
        } else {
            ImGui::RegisterMarkdownStyleTag(tag_name, s_def);
        }
        lua_pushboolean(L, true);
        return 1;
    } else {
        return luaL_error(L, "RegisterTag expects a style string, template string, or callback function as argument 2");
    }
}

// ----------------------------------------------------------------------------
// 2. TimedTooltip Extension Lua Bindings
// ----------------------------------------------------------------------------

static int LuaTimedTooltipText(lua_State* L) {
    std::string s = FormatLuaString(L);
    bool shown = ImGui::TimedTooltip::Text("%s", s.c_str());
    lua_pushboolean(L, shown);
    return 1;
}

static int LuaTimedTooltipTextMD(lua_State* L) {
    std::string s = FormatLuaString(L);
    bool shown = ImGui::TimedTooltip::TextMD(s);
    lua_pushboolean(L, shown);
    return 1;
}

static int LuaTimedTooltipTextUnformatted(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    bool shown = ImGui::TimedTooltip::TextUnformatted(s ? s : "");
    lua_pushboolean(L, shown);
    return 1;
}

static int LuaTimedTooltipTextMDUnformatted(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    bool shown = ImGui::TimedTooltip::TextMDUnformatted(s ? s : "");
    lua_pushboolean(L, shown);
    return 1;
}

static int LuaTimedTooltipTextLink(lua_State* L) {
    std::string s = FormatLuaString(L);
    bool shown = ImGui::TimedTooltip::TextLinkContrib("%s", s.c_str());
    lua_pushboolean(L, shown);
    return 1;
}

static int LuaTimedTooltipTextMDLink(lua_State* L) {
    std::string s = FormatLuaString(L);
    bool shown = ImGui::TimedTooltip::TextMDLink("%s", s.c_str());
    lua_pushboolean(L, shown);
    return 1;
}

static int LuaTimedTooltipTextProxy(lua_State* L) {
    ImGuiID id = (lua_type(L, 1) == LUA_TNUMBER)
                     ? static_cast<ImGuiID>(lua_tonumber(L, 1))
                     : ImGui::GetID(luaL_checkstring(L, 1));
    bool is_hovered = lua_toboolean(L, 2) != 0;
    std::string s = FormatLuaStringFrom(L, 3);
    bool shown = ImGui::TimedTooltip::TextProxy(id, is_hovered, "%s", s.c_str());
    lua_pushboolean(L, shown);
    return 1;
}

static int LuaTimedTooltipTextMDProxy(lua_State* L) {
    ImGuiID id = (lua_type(L, 1) == LUA_TNUMBER)
                     ? static_cast<ImGuiID>(lua_tonumber(L, 1))
                     : ImGui::GetID(luaL_checkstring(L, 1));
    bool is_hovered = lua_toboolean(L, 2) != 0;
    std::string s = FormatLuaStringFrom(L, 3);
    bool shown = ImGui::TimedTooltip::TextMDProxy(id, is_hovered, "%s", s.c_str());
    lua_pushboolean(L, shown);
    return 1;
}

static int LuaTimedTooltipToggleOnClick(lua_State* L) {
    bool enable = (lua_gettop(L) >= 1 && !lua_isnil(L, 1)) ? (lua_toboolean(L, 1) != 0) : true;
    ImGui::TimedTooltip::ToggleOnClick(enable);
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

static int LuaTimedTooltipSetGracePeriod(lua_State* L) {
    int cycles = static_cast<int>(luaL_checkinteger(L, 1));
    auto& cfg = GetFWLTooltipConfig();
    cfg.grace_period_cycles = cycles;
    return 0;
}

static int LuaTimedTooltipSetWiggleLatch(lua_State* L) {
    bool enable = (lua_gettop(L) >= 1) ? (lua_toboolean(L, 1) != 0) : true;
    auto& cfg = GetFWLTooltipConfig();
    cfg.enable_wiggle_latch = enable;
    return 0;
}

static int LuaTimedTooltipSetDefaultWrapWidth(lua_State* L) {
    float w = static_cast<float>(luaL_checknumber(L, 1));
    auto& cfg = GetFWLTooltipConfig();
    cfg.default_wrap_width = w;
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

static int LuaTimedTooltipSetColors(lua_State* L) {
    auto& cfg = GetFWLTooltipConfig();
    ImVec4 c(0.0f, 0.0f, 0.0f, 0.0f);
    if (ParseLuaColorVec4(L, 1, c)) cfg.bg_color = c;
    if (ParseLuaColorVec4(L, 2, c)) cfg.border_color = c;
    if (ParseLuaColorVec4(L, 3, c)) cfg.text_color = c;
    return 0;
}

static int LuaTimedTooltipSetFontScalePolicy(lua_State* L) {
    ImGui::TimedTooltip::TooltipFontScalePolicy policy = ImGui::TimedTooltip::TooltipFontScale_Damped;
    if (lua_type(L, 1) == LUA_TSTRING) {
        std::string s = lua_tostring(L, 1);
        for (char& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        if (s == "fixed" || s == "fixedchrome" || s == "fixed_chrome") {
            policy = ImGui::TimedTooltip::TooltipFontScale_FixedChrome;
        } else if (s == "linear" || s == "linearinherit" || s == "linear_inherit") {
            policy = ImGui::TimedTooltip::TooltipFontScale_LinearInherit;
        } else {
            policy = ImGui::TimedTooltip::TooltipFontScale_Damped;
        }
    } else {
        int raw = static_cast<int>(luaL_checkinteger(L, 1));
        if (raw >= 0 && raw <= 2) {
            policy = static_cast<ImGui::TimedTooltip::TooltipFontScalePolicy>(raw);
        }
    }
    ImGui::TimedTooltip::SetDefaultFontScalePolicy(policy);
    GetFWLTooltipConfig().font_scale_policy = policy;
    return 0;
}

static int LuaTimedTooltipGetFontScalePolicy(lua_State* L) {
    lua_pushinteger(L, static_cast<int>(GetFWLTooltipConfig().font_scale_policy));
    return 1;
}

static int LuaTimedTooltipSetFontScaleMultiplier(lua_State* L) {
    float mult = static_cast<float>(luaL_checknumber(L, 1));
    GetFWLTooltipConfig().font_scale_multiplier = mult;
    return 0;
}

static int LuaTimedTooltipSetFontScale(lua_State* L) {
    float scale = (lua_gettop(L) >= 1 && !lua_isnil(L, 1)) ? static_cast<float>(luaL_checknumber(L, 1)) : 0.0f;
    GetFWLTooltipConfig().font_scale = scale;
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
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    const bool arg_is_num = (lua_type(L, 2) == LUA_TNUMBER);
    bool val = arg_is_num ? (lua_tointeger(L, 2) != 0) : (lua_toboolean(L, 2) != 0);

    float w = 0.0f, h = 0.0f;
    int arg_idx = 3;
    if (top >= arg_idx && lua_type(L, arg_idx) == LUA_TNUMBER) {
        w = static_cast<float>(lua_tonumber(L, arg_idx++));
        if (top >= arg_idx && lua_type(L, arg_idx) == LUA_TNUMBER) {
            h = static_cast<float>(lua_tonumber(L, arg_idx++));
        }
    }

    std::vector<std::string> tip_storage;
    std::vector<const char*> tip_ptrs;
    bool has_tips = false;
    if (top >= arg_idx && lua_istable(L, arg_idx)) {
        has_tips = ParseOptionalStringTable(L, arg_idx, tip_storage, tip_ptrs, 2);
        arg_idx++;
    } else if (top >= arg_idx + 1 && lua_type(L, arg_idx) == LUA_TSTRING && lua_type(L, arg_idx + 1) == LUA_TSTRING) {
        tip_storage.push_back(lua_tostring(L, arg_idx));
        tip_storage.push_back(lua_tostring(L, arg_idx + 1));
        tip_ptrs.push_back(tip_storage[0].c_str());
        tip_ptrs.push_back(tip_storage[1].c_str());
        has_tips = true;
        arg_idx += 2;
    }

    bool allow_wheel = true;
    if (top >= arg_idx && !lua_isnil(L, arg_idx)) {
        allow_wheel = (lua_toboolean(L, arg_idx) != 0);
        arg_idx++;
    } else if (top >= arg_idx && lua_isnil(L, arg_idx)) {
        arg_idx++;
    }

    ImGuiMouseCursor hover_cursor = ImGuiMouseCursor_Hand;
    if (top >= arg_idx && lua_type(L, arg_idx) == LUA_TNUMBER) {
        hover_cursor = static_cast<ImGuiMouseCursor>(lua_tointeger(L, arg_idx));
    }

    bool changed = ImGui::ToggleButton(label, &val, ImVec2(w, h), has_tips ? tip_ptrs.data() : nullptr, allow_wheel, hover_cursor);
    lua_pushboolean(L, changed);
    if (arg_is_num) {
        lua_pushinteger(L, val ? 1 : 0);
    } else {
        lua_pushboolean(L, val);
    }
    return 2;
}

static int LuaToggleButtons(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    const bool arg_is_bool = (lua_type(L, 2) == LUA_TBOOLEAN);
    int state = arg_is_bool ? ((lua_toboolean(L, 2) != 0) ? 1 : 0) : static_cast<int>(luaL_checkinteger(L, 2));
    const char* off_label = luaL_checkstring(L, 3);
    const char* on_label  = luaL_checkstring(L, 4);

    float w = 0.0f, h = 0.0f;
    int arg_idx = 5;
    if (top >= arg_idx && lua_type(L, arg_idx) == LUA_TNUMBER) {
        w = static_cast<float>(lua_tonumber(L, arg_idx++));
        if (top >= arg_idx && lua_type(L, arg_idx) == LUA_TNUMBER) {
            h = static_cast<float>(lua_tonumber(L, arg_idx++));
        }
    }

    std::vector<std::string> tip_storage;
    std::vector<const char*> tip_ptrs;
    bool has_tips = false;
    if (top >= arg_idx && lua_istable(L, arg_idx)) {
        has_tips = ParseOptionalStringTable(L, arg_idx, tip_storage, tip_ptrs, 2);
        arg_idx++;
    } else if (top >= arg_idx && lua_isnil(L, arg_idx)) {
        arg_idx++;
    }

    bool allow_wheel = true;
    if (top >= arg_idx && !lua_isnil(L, arg_idx)) {
        allow_wheel = (lua_toboolean(L, arg_idx) != 0);
        arg_idx++;
    } else if (top >= arg_idx && lua_isnil(L, arg_idx)) {
        arg_idx++;
    }

    ImGuiMouseCursor hover_cursor = ImGuiMouseCursor_Hand;
    if (top >= arg_idx && lua_type(L, arg_idx) == LUA_TNUMBER) {
        hover_cursor = static_cast<ImGuiMouseCursor>(lua_tointeger(L, arg_idx));
    }

    int hovered_item = -1;
    bool changed = ImGui::ToggleButtons(label, &state, off_label, on_label, ImVec2(w, h),
                                        has_tips ? tip_ptrs.data() : nullptr, &hovered_item,
                                        allow_wheel, hover_cursor);
    lua_pushboolean(L, changed);
    if (arg_is_bool) {
        lua_pushboolean(L, state != 0);
    } else {
        lua_pushinteger(L, state);
    }
    lua_pushinteger(L, hovered_item);
    return 3;
}

static int LuaSetDefaultToggleButtonScale(lua_State* L) {
    float scale = static_cast<float>(luaL_checknumber(L, 1));
    ImGui::SetDefaultToggleButtonScale(scale);
    return 0;
}

static int LuaSetDefaultToggleButtonColors(lua_State* L) {
    ImU32 col_on    = ParseLuaColor(L, 1);
    ImU32 col_off   = ParseLuaColor(L, 2);
    ImU32 col_thumb = ParseLuaColor(L, 3);
    ImGui::SetDefaultToggleButtonColors(col_on, col_off, col_thumb);
    return 0;
}

static int LuaAdjustItemOnMouseWheel(lua_State* L) {
    int top = lua_gettop(L);
    float val   = static_cast<float>(luaL_checknumber(L, 1));
    float v_min = static_cast<float>(luaL_checknumber(L, 2));
    float v_max = static_cast<float>(luaL_checknumber(L, 3));
    float step  = (top >= 4 && lua_isnumber(L, 4))
                      ? static_cast<float>(lua_tonumber(L, 4)) : 0.01f;
    bool set_cursor = (top >= 5 && !lua_isnil(L, 5)) ? (lua_toboolean(L, 5) != 0) : true;
    bool changed = ImGui::AdjustItemOnMouseWheel(&val, v_min, v_max, step, set_cursor);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaToggleItemOnMouseWheel(lua_State* L) {
    int top = lua_gettop(L);
    const bool arg_is_num = (lua_type(L, 1) == LUA_TNUMBER);
    bool val            = arg_is_num ? (lua_tointeger(L, 1) != 0) : (lua_toboolean(L, 1) != 0);
    bool wrap           = (top >= 2 && !lua_isnil(L, 2)) ? (lua_toboolean(L, 2) != 0) : false;
    bool only_over_box  = (top >= 3 && !lua_isnil(L, 3)) ? (lua_toboolean(L, 3) != 0) : true;
    bool consume        = (top >= 4 && !lua_isnil(L, 4)) ? (lua_toboolean(L, 4) != 0) : true;
    ImGuiMouseCursor hover_cursor = (top >= 5 && lua_type(L, 5) == LUA_TNUMBER)
                                        ? static_cast<ImGuiMouseCursor>(lua_tointeger(L, 5))
                                        : ImGuiMouseCursor_ResizeNS;
    bool changed = ImGui::ToggleItemOnMouseWheel(&val, wrap, only_over_box, consume, hover_cursor);
    lua_pushboolean(L, changed);
    if (arg_is_num) {
        lua_pushinteger(L, val ? 1 : 0);
    } else {
        lua_pushboolean(L, val);
    }
    return 2;
}

static int LuaCycleItemOnMouseWheel(lua_State* L) {
    int top = lua_gettop(L);
    // Boolean overload: CycleItemOnMouseWheel(bool_val, [wrap], [only_over_box], [consume], [hover_cursor])
    if (lua_type(L, 1) == LUA_TBOOLEAN) {
        return LuaToggleItemOnMouseWheel(L);
    }

    int idx = static_cast<int>(luaL_checkinteger(L, 1));

    // Range overload: (idx, min_idx, max_idx, [wrap], [natural_list_order], [consume], [hover_cursor])
    if (top >= 3 && lua_type(L, 3) == LUA_TNUMBER) {
        int min_idx = static_cast<int>(luaL_checkinteger(L, 2));
        int max_idx = static_cast<int>(luaL_checkinteger(L, 3));
        bool wrap   = (top >= 4 && !lua_isnil(L, 4)) ? (lua_toboolean(L, 4) != 0) : false;
        bool nat    = (top >= 5 && !lua_isnil(L, 5)) ? (lua_toboolean(L, 5) != 0) : true;
        bool cons   = (top >= 6 && !lua_isnil(L, 6)) ? (lua_toboolean(L, 6) != 0) : true;
        ImGuiMouseCursor cur = (top >= 7 && lua_type(L, 7) == LUA_TNUMBER)
                                   ? static_cast<ImGuiMouseCursor>(lua_tointeger(L, 7))
                                   : ImGuiMouseCursor_ResizeNS;
        bool changed = ImGui::CycleItemOnMouseWheel(&idx, min_idx, max_idx, wrap, nat, cons, cur);
        lua_pushboolean(L, changed);
        lua_pushinteger(L, idx);
        return 2;
    }

    // Count overload: (idx, count, [wrap], [natural_list_order], [consume], [hover_cursor])
    int count = static_cast<int>(luaL_checkinteger(L, 2));
    bool wrap = (top >= 3 && !lua_isnil(L, 3)) ? (lua_toboolean(L, 3) != 0) : false;
    bool nat  = (top >= 4 && !lua_isnil(L, 4)) ? (lua_toboolean(L, 4) != 0) : true;
    bool cons = (top >= 5 && !lua_isnil(L, 5)) ? (lua_toboolean(L, 5) != 0) : true;
    ImGuiMouseCursor cur = (top >= 6 && lua_type(L, 6) == LUA_TNUMBER)
                               ? static_cast<ImGuiMouseCursor>(lua_tointeger(L, 6))
                               : ImGuiMouseCursor_ResizeNS;

    // Support 1-based indexing if caller passes 1..count
    bool one_based = (idx >= 1 && idx <= count);
    int zero_idx = one_based ? (idx - 1) : idx;
    bool changed = ImGui::CycleItemOnMouseWheel(&zero_idx, count, wrap, nat, cons, cur);
    int result_idx = one_based ? (zero_idx + 1) : zero_idx;

    lua_pushboolean(L, changed);
    lua_pushinteger(L, result_idx);
    return 2;
}

static int LuaMouseWheelCheckbox(lua_State* L) {
    int top = lua_gettop(L);
    const char* label     = luaL_checkstring(L, 1);
    const bool arg_is_num = (lua_type(L, 2) == LUA_TNUMBER);
    bool val              = arg_is_num ? (lua_tointeger(L, 2) != 0) : (lua_toboolean(L, 2) != 0);
    bool wrap             = (top >= 3 && !lua_isnil(L, 3)) ? (lua_toboolean(L, 3) != 0) : false;
    bool only_over_box    = (top >= 4 && !lua_isnil(L, 4)) ? (lua_toboolean(L, 4) != 0) : true;
    ImGuiMouseCursor cur  = (top >= 5 && lua_type(L, 5) == LUA_TNUMBER)
                                ? static_cast<ImGuiMouseCursor>(lua_tointeger(L, 5))
                                : ImGuiMouseCursor_ResizeNS;
    bool changed = ImGui::MouseWheelCheckbox(label, &val, wrap, only_over_box, cur);
    lua_pushboolean(L, changed);
    if (arg_is_num) {
        lua_pushinteger(L, val ? 1 : 0);
    } else {
        lua_pushboolean(L, val);
    }
    return 2;
}

static int LuaMouseWheelSlider(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float val         = static_cast<float>(luaL_checknumber(L, 2));
    float v_min       = static_cast<float>(luaL_checknumber(L, 3));
    float v_max       = static_cast<float>(luaL_checknumber(L, 4));
    const char* fmt   = (top >= 5 && !lua_isnil(L, 5)) ? luaL_checkstring(L, 5) : "%.3f";
    ImGuiSliderFlags flags = (top >= 6 && !lua_isnil(L, 6)) ? static_cast<ImGuiSliderFlags>(luaL_checkinteger(L, 6)) : 0;

    if (IsIntegerPrintfFormat(fmt)) {
        int iv   = static_cast<int>(std::round(val));
        int imin = static_cast<int>(std::round(v_min));
        int imax = static_cast<int>(std::round(v_max));
        float step = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<float>(luaL_checknumber(L, 7)) : 1.0f;
        bool changed = ImGui::MouseWheelSlider(label, &iv, imin, imax, fmt, flags, step);
        lua_pushboolean(L, changed);
        lua_pushinteger(L, iv);
        return 2;
    }

    float step = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<float>(luaL_checknumber(L, 7)) : 0.01f;
    bool changed = ImGui::MouseWheelSlider(label, &val, v_min, v_max, fmt, flags, step);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaMouseWheelSliderInt(lua_State* L) {
    int top = lua_gettop(L);
    const char* label      = luaL_checkstring(L, 1);
    int val                = static_cast<int>(luaL_checkinteger(L, 2));
    int v_min              = static_cast<int>(luaL_checkinteger(L, 3));
    int v_max              = static_cast<int>(luaL_checkinteger(L, 4));
    const char* fmt        = (top >= 5 && !lua_isnil(L, 5)) ? luaL_checkstring(L, 5) : "%d";
    ImGuiSliderFlags flags = (top >= 6 && !lua_isnil(L, 6)) ? static_cast<ImGuiSliderFlags>(luaL_checkinteger(L, 6)) : 0;
    float step             = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<float>(luaL_checknumber(L, 7)) : 1.0f;
    bool changed = ImGui::MouseWheelSlider(label, &val, v_min, v_max, fmt, flags, step);
    lua_pushboolean(L, changed);
    lua_pushinteger(L, val);
    return 2;
}

static int LuaMouseWheelDrag(lua_State* L) {
    int top = lua_gettop(L);
    const char* label      = luaL_checkstring(L, 1);
    float val              = static_cast<float>(luaL_checknumber(L, 2));
    float v_speed          = (top >= 3 && !lua_isnil(L, 3)) ? static_cast<float>(luaL_checknumber(L, 3)) : 1.0f;
    float v_min            = (top >= 4 && !lua_isnil(L, 4)) ? static_cast<float>(luaL_checknumber(L, 4)) : 0.0f;
    float v_max            = (top >= 5 && !lua_isnil(L, 5)) ? static_cast<float>(luaL_checknumber(L, 5)) : 0.0f;
    const char* fmt        = (top >= 6 && !lua_isnil(L, 6)) ? luaL_checkstring(L, 6) : "%.3f";
    ImGuiSliderFlags flags = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<ImGuiSliderFlags>(luaL_checkinteger(L, 7)) : 0;
    float step             = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<float>(luaL_checknumber(L, 8)) : 0.0f;

    if (IsIntegerPrintfFormat(fmt)) {
        int iv   = static_cast<int>(std::round(val));
        int imin = static_cast<int>(std::round(v_min));
        int imax = static_cast<int>(std::round(v_max));
        bool changed = ImGui::MouseWheelDrag(label, &iv, v_speed, imin, imax, fmt, flags, step);
        lua_pushboolean(L, changed);
        lua_pushinteger(L, iv);
        return 2;
    }

    bool changed = ImGui::MouseWheelDrag(label, &val, v_speed, v_min, v_max, fmt, flags, step);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaMouseWheelDragInt(lua_State* L) {
    int top = lua_gettop(L);
    const char* label      = luaL_checkstring(L, 1);
    int val                = static_cast<int>(luaL_checkinteger(L, 2));
    float v_speed          = (top >= 3 && !lua_isnil(L, 3)) ? static_cast<float>(luaL_checknumber(L, 3)) : 1.0f;
    int v_min              = (top >= 4 && !lua_isnil(L, 4)) ? static_cast<int>(luaL_checkinteger(L, 4)) : 0;
    int v_max              = (top >= 5 && !lua_isnil(L, 5)) ? static_cast<int>(luaL_checkinteger(L, 5)) : 0;
    const char* fmt        = (top >= 6 && !lua_isnil(L, 6)) ? luaL_checkstring(L, 6) : "%d";
    ImGuiSliderFlags flags = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<ImGuiSliderFlags>(luaL_checkinteger(L, 7)) : 0;
    float step             = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<float>(luaL_checknumber(L, 8)) : 0.0f;
    bool changed = ImGui::MouseWheelDrag(label, &val, v_speed, v_min, v_max, fmt, flags, step);
    lua_pushboolean(L, changed);
    lua_pushinteger(L, val);
    return 2;
}

static int LuaPercentSlider(lua_State* L) {
    int top = lua_gettop(L);
    const char* label      = luaL_checkstring(L, 1);
    float val              = static_cast<float>(luaL_checknumber(L, 2));
    float v_min            = (top >= 3 && !lua_isnil(L, 3)) ? static_cast<float>(luaL_checknumber(L, 3)) : 0.0f;
    float v_max            = (top >= 4 && !lua_isnil(L, 4)) ? static_cast<float>(luaL_checknumber(L, 4)) : 1.0f;
    const char* fmt        = (top >= 5 && !lua_isnil(L, 5)) ? luaL_checkstring(L, 5) : "%.0f%%";
    ImGuiSliderFlags flags = (top >= 6 && !lua_isnil(L, 6)) ? static_cast<ImGuiSliderFlags>(luaL_checkinteger(L, 6)) : 0;
    float step             = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<float>(luaL_checknumber(L, 7)) : 0.01f;
    bool changed = ImGui::PercentSlider(label, &val, v_min, v_max, fmt, flags, step);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaSliderWithReset(lua_State* L) {
    int top = lua_gettop(L);
    const char* label      = luaL_checkstring(L, 1);
    float val              = static_cast<float>(luaL_checknumber(L, 2));
    float v_min            = static_cast<float>(luaL_checknumber(L, 3));
    float v_max            = static_cast<float>(luaL_checknumber(L, 4));
    float def_val          = static_cast<float>(luaL_checknumber(L, 5));
    const char* fmt        = (top >= 6 && !lua_isnil(L, 6)) ? luaL_checkstring(L, 6) : "%.3f";
    ImGuiSliderFlags flags = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<ImGuiSliderFlags>(luaL_checkinteger(L, 7)) : 0;
    const char* tip        = (top >= 9 && !lua_isnil(L, 9)) ? luaL_checkstring(L, 9) : nullptr;

    if (IsIntegerPrintfFormat(fmt)) {
        int iv   = static_cast<int>(std::round(val));
        int imin = static_cast<int>(std::round(v_min));
        int imax = static_cast<int>(std::round(v_max));
        int idef = static_cast<int>(std::round(def_val));
        float step = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<float>(luaL_checknumber(L, 8)) : 1.0f;
        bool changed = ImGui::SliderWithReset(label, &iv, idef, imin, imax, fmt, flags, step, tip);
        lua_pushboolean(L, changed);
        lua_pushinteger(L, iv);
        return 2;
    }

    float step = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<float>(luaL_checknumber(L, 8)) : 0.01f;
    bool changed = ImGui::SliderWithReset(label, &val, def_val, v_min, v_max, fmt, flags, step, tip);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaSliderIntWithReset(lua_State* L) {
    int top = lua_gettop(L);
    const char* label      = luaL_checkstring(L, 1);
    int val                = static_cast<int>(luaL_checkinteger(L, 2));
    int v_min              = static_cast<int>(luaL_checkinteger(L, 3));
    int v_max              = static_cast<int>(luaL_checkinteger(L, 4));
    int def_val            = static_cast<int>(luaL_checkinteger(L, 5));
    const char* fmt        = (top >= 6 && !lua_isnil(L, 6)) ? luaL_checkstring(L, 6) : "%d";
    ImGuiSliderFlags flags = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<ImGuiSliderFlags>(luaL_checkinteger(L, 7)) : 0;
    float step             = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<float>(luaL_checknumber(L, 8)) : 1.0f;
    const char* tip        = (top >= 9 && !lua_isnil(L, 9)) ? luaL_checkstring(L, 9) : nullptr;
    bool changed = ImGui::SliderWithReset(label, &val, def_val, v_min, v_max, fmt, flags, step, tip);
    lua_pushboolean(L, changed);
    lua_pushinteger(L, val);
    return 2;
}

static int LuaPercentSliderWithReset(lua_State* L) {
    int top = lua_gettop(L);
    const char* label      = luaL_checkstring(L, 1);
    float val              = static_cast<float>(luaL_checknumber(L, 2));
    float def_val          = (top >= 3 && !lua_isnil(L, 3)) ? static_cast<float>(luaL_checknumber(L, 3)) : 1.0f;
    float v_min            = (top >= 4 && !lua_isnil(L, 4)) ? static_cast<float>(luaL_checknumber(L, 4)) : 0.0f;
    float v_max            = (top >= 5 && !lua_isnil(L, 5)) ? static_cast<float>(luaL_checknumber(L, 5)) : 1.0f;
    const char* fmt        = (top >= 6 && !lua_isnil(L, 6)) ? luaL_checkstring(L, 6) : "%.0f%%";
    ImGuiSliderFlags flags = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<ImGuiSliderFlags>(luaL_checkinteger(L, 7)) : 0;
    float step             = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<float>(luaL_checknumber(L, 8)) : 0.01f;
    const char* tip        = (top >= 9 && !lua_isnil(L, 9)) ? luaL_checkstring(L, 9) : nullptr;
    bool changed = ImGui::PercentSliderWithReset(label, &val, def_val, v_min, v_max, fmt, flags, step, tip);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaDragWithReset(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float val         = static_cast<float>(luaL_checknumber(L, 2));

    // Support both compact 3-arg (label, val, def_val) and full Lua DragFloat-style
    // (label, val, v_speed, v_min, v_max, def_val, [format], [flags], [wheel_step], [reset_tooltip])
    float v_speed = 1.0f;
    float v_min   = 0.0f;
    float v_max   = 0.0f;
    float def_val = 0.0f;
    int fmt_idx   = 7;
    if (top == 3) {
        def_val = static_cast<float>(luaL_checknumber(L, 3));
    } else if (top == 4) {
        def_val = static_cast<float>(luaL_checknumber(L, 3));
        v_speed = static_cast<float>(luaL_checknumber(L, 4));
    } else {
        v_speed = (top >= 3 && !lua_isnil(L, 3)) ? static_cast<float>(luaL_checknumber(L, 3)) : 1.0f;
        v_min   = (top >= 4 && !lua_isnil(L, 4)) ? static_cast<float>(luaL_checknumber(L, 4)) : 0.0f;
        v_max   = (top >= 5 && !lua_isnil(L, 5)) ? static_cast<float>(luaL_checknumber(L, 5)) : 0.0f;
        def_val = (top >= 6 && !lua_isnil(L, 6)) ? static_cast<float>(luaL_checknumber(L, 6)) : 0.0f;
    }

    const char* fmt        = (top >= fmt_idx && !lua_isnil(L, fmt_idx)) ? luaL_checkstring(L, fmt_idx) : "%.3f";
    ImGuiSliderFlags flags = (top >= fmt_idx + 1 && !lua_isnil(L, fmt_idx + 1)) ? static_cast<ImGuiSliderFlags>(luaL_checkinteger(L, fmt_idx + 1)) : 0;
    float step             = (top >= fmt_idx + 2 && !lua_isnil(L, fmt_idx + 2)) ? static_cast<float>(luaL_checknumber(L, fmt_idx + 2)) : 0.0f;
    const char* tip        = (top >= fmt_idx + 3 && !lua_isnil(L, fmt_idx + 3)) ? luaL_checkstring(L, fmt_idx + 3) : nullptr;

    if (IsIntegerPrintfFormat(fmt)) {
        int iv   = static_cast<int>(std::round(val));
        int imin = static_cast<int>(std::round(v_min));
        int imax = static_cast<int>(std::round(v_max));
        int idef = static_cast<int>(std::round(def_val));
        bool changed = ImGui::DragWithReset(label, &iv, idef, v_speed, imin, imax, fmt, flags, step, tip);
        lua_pushboolean(L, changed);
        lua_pushinteger(L, iv);
        return 2;
    }

    bool changed = ImGui::DragWithReset(label, &val, def_val, v_speed, v_min, v_max, fmt, flags, step, tip);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaDragIntWithReset(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int val           = static_cast<int>(luaL_checkinteger(L, 2));
    float v_speed     = 1.0f;
    int v_min         = 0;
    int v_max         = 0;
    int def_val       = 0;
    if (top == 3) {
        def_val = static_cast<int>(luaL_checkinteger(L, 3));
    } else if (top == 4) {
        def_val = static_cast<int>(luaL_checkinteger(L, 3));
        v_speed = static_cast<float>(luaL_checknumber(L, 4));
    } else {
        v_speed = (top >= 3 && !lua_isnil(L, 3)) ? static_cast<float>(luaL_checknumber(L, 3)) : 1.0f;
        v_min   = (top >= 4 && !lua_isnil(L, 4)) ? static_cast<int>(luaL_checkinteger(L, 4)) : 0;
        v_max   = (top >= 5 && !lua_isnil(L, 5)) ? static_cast<int>(luaL_checkinteger(L, 5)) : 0;
        def_val = (top >= 6 && !lua_isnil(L, 6)) ? static_cast<int>(luaL_checkinteger(L, 6)) : 0;
    }
    const char* fmt        = (top >= 7 && !lua_isnil(L, 7)) ? luaL_checkstring(L, 7) : "%d";
    ImGuiSliderFlags flags = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<ImGuiSliderFlags>(luaL_checkinteger(L, 8)) : 0;
    float step             = (top >= 9 && !lua_isnil(L, 9)) ? static_cast<float>(luaL_checknumber(L, 9)) : 0.0f;
    const char* tip        = (top >= 10 && !lua_isnil(L, 10)) ? luaL_checkstring(L, 10) : nullptr;

    bool changed = ImGui::DragWithReset(label, &val, def_val, v_speed, v_min, v_max, fmt, flags, step, tip);
    lua_pushboolean(L, changed);
    lua_pushinteger(L, val);
    return 2;
}

static int LuaMouseWheelCombo(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int current_item  = static_cast<int>(luaL_checkinteger(L, 2));

    std::vector<std::string> items_storage;
    std::vector<const char*> item_ptrs;
    if (!ParseComboItemsFromLua(L, 3, items_storage, item_ptrs)) {
        lua_pushboolean(L, false);
        lua_pushinteger(L, current_item);
        return 2;
    }

    int popup_max_h = (top >= 4 && lua_type(L, 4) == LUA_TNUMBER) ? static_cast<int>(lua_tointeger(L, 4)) : -1;
    bool wrap       = (top >= 5 && !lua_isnil(L, 5)) ? (lua_toboolean(L, 5) != 0) : false;

    std::vector<std::string> tip_storage;
    std::vector<const char*> tip_ptrs;
    bool has_tips = (top >= 6 && lua_istable(L, 6))
                    && ParseOptionalStringTable(L, 6, tip_storage, tip_ptrs, item_ptrs.size());

    ImGuiMouseCursor hover_cursor = (top >= 7 && lua_type(L, 7) == LUA_TNUMBER)
                                        ? static_cast<ImGuiMouseCursor>(lua_tointeger(L, 7))
                                        : ImGuiMouseCursor_ResizeNS;
    ImGuiMouseCursor popup_cursor = (top >= 8 && lua_type(L, 8) == LUA_TNUMBER)
                                        ? static_cast<ImGuiMouseCursor>(lua_tointeger(L, 8))
                                        : ImGuiMouseCursor_ResizeNS;

    bool changed = ImGui::MouseWheelCombo(label, &current_item, item_ptrs.data(), static_cast<int>(item_ptrs.size()),
                                          popup_max_h, wrap, has_tips ? tip_ptrs.data() : nullptr,
                                          hover_cursor, popup_cursor);
    lua_pushboolean(L, changed);
    lua_pushinteger(L, current_item);
    return 2;
}

static int LuaComboWithReset(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    int current_item  = static_cast<int>(luaL_checkinteger(L, 2));

    int default_item = 0;
    int items_idx    = 4;
    if (lua_istable(L, 3) || (lua_type(L, 3) == LUA_TSTRING && !lua_isnumber(L, 3))) {
        // Alternate ergonomic order: (label, current_item, items, default_item, ...)
        items_idx    = 3;
        default_item = (top >= 4 && lua_type(L, 4) == LUA_TNUMBER) ? static_cast<int>(lua_tointeger(L, 4)) : 0;
    } else {
        // Canonical ImGee C++ order: (label, current_item, default_item, items, ...)
        default_item = static_cast<int>(luaL_checkinteger(L, 3));
        items_idx    = 4;
    }

    std::vector<std::string> items_storage;
    std::vector<const char*> item_ptrs;
    if (!ParseComboItemsFromLua(L, items_idx, items_storage, item_ptrs)) {
        lua_pushboolean(L, false);
        lua_pushinteger(L, current_item);
        return 2;
    }

    int popup_max_h  = (top >= 5 && lua_type(L, 5) == LUA_TNUMBER) ? static_cast<int>(lua_tointeger(L, 5)) : -1;
    bool wrap        = (top >= 6 && !lua_isnil(L, 6)) ? (lua_toboolean(L, 6) != 0) : false;
    const char* rtip = (top >= 7 && lua_type(L, 7) == LUA_TSTRING) ? lua_tostring(L, 7) : nullptr;

    std::vector<std::string> tip_storage;
    std::vector<const char*> tip_ptrs;
    bool has_tips = (top >= 8 && lua_istable(L, 8))
                    && ParseOptionalStringTable(L, 8, tip_storage, tip_ptrs, item_ptrs.size());

    ImGuiMouseCursor hover_cursor = (top >= 9 && lua_type(L, 9) == LUA_TNUMBER)
                                        ? static_cast<ImGuiMouseCursor>(lua_tointeger(L, 9))
                                        : ImGuiMouseCursor_ResizeNS;
    ImGuiMouseCursor popup_cursor = (top >= 10 && lua_type(L, 10) == LUA_TNUMBER)
                                        ? static_cast<ImGuiMouseCursor>(lua_tointeger(L, 10))
                                        : ImGuiMouseCursor_ResizeNS;

    bool changed = ImGui::ComboWithReset(label, &current_item, default_item, item_ptrs.data(),
                                         static_cast<int>(item_ptrs.size()), popup_max_h, wrap,
                                         rtip, has_tips ? tip_ptrs.data() : nullptr,
                                         hover_cursor, popup_cursor);
    lua_pushboolean(L, changed);
    lua_pushinteger(L, current_item);
    return 2;
}

static int LuaSetMouseWheelAutoDefeat(lua_State* L) {
    bool enable = (lua_gettop(L) >= 1 && !lua_isnil(L, 1)) ? (lua_toboolean(L, 1) != 0) : true;
    ImGui::SetMouseWheelAutoDefeat(enable);
    lua_pushboolean(L, ImGui::GetMouseWheelAutoDefeat());
    return 1;
}

static int LuaGetMouseWheelAutoDefeat(lua_State* L) {
    lua_pushboolean(L, ImGui::GetMouseWheelAutoDefeat());
    return 1;
}

static int LuaSetDefaultComboArrowStyle(lua_State* L) {
    int raw = static_cast<int>(luaL_checkinteger(L, 1));
    if (raw >= 0 && raw <= 3) {
        ImGui::SetDefaultComboArrowStyle(static_cast<ImGuiComboArrowStyle>(raw));
    }
    return 0;
}

static int LuaGetDefaultComboArrowStyle(lua_State* L) {
    lua_pushinteger(L, static_cast<int>(ImGui::GetDefaultComboArrowStyle()));
    return 1;
}

static int LuaSetResetButtonUndoEnabled(lua_State* L) {
    bool enabled = (lua_gettop(L) >= 1 && !lua_isnil(L, 1)) ? (lua_toboolean(L, 1) != 0) : true;
    ImGui::SetResetButtonUndoEnabled(enabled);
    return 0;
}

static int LuaGetResetButtonUndoEnabled(lua_State* L) {
    lua_pushboolean(L, ImGui::GetResetButtonUndoEnabled());
    return 1;
}

static std::string s_FWLDefaultResetLabel;
static std::string s_FWLDefaultRestoreLabel;

static int LuaSetDefaultResetButtonLabel(lua_State* L) {
    int top = lua_gettop(L);
    const char* reset_ptr = nullptr;
    const char* restore_ptr = ImGui::GetDefaultRestoreButtonLabel();
    if (top >= 1 && !lua_isnil(L, 1)) {
        const char* r = luaL_checkstring(L, 1);
        s_FWLDefaultResetLabel = r ? r : "";
        reset_ptr = s_FWLDefaultResetLabel.empty() ? nullptr : s_FWLDefaultResetLabel.c_str();
    } else {
        s_FWLDefaultResetLabel.clear();
        reset_ptr = nullptr;
    }
    if (top >= 2) {
        if (!lua_isnil(L, 2)) {
            const char* alt = luaL_checkstring(L, 2);
            s_FWLDefaultRestoreLabel = alt ? alt : "";
            restore_ptr = s_FWLDefaultRestoreLabel.empty() ? nullptr : s_FWLDefaultRestoreLabel.c_str();
        } else {
            s_FWLDefaultRestoreLabel.clear();
            restore_ptr = nullptr;
        }
    }
    ImGui::SetDefaultResetButtonLabel(reset_ptr, restore_ptr);
    return 0;
}

static int LuaGetDefaultResetButtonLabel(lua_State* L) {
    const char* lbl = ImGui::GetDefaultResetButtonLabel();
    if (lbl) lua_pushstring(L, lbl);
    else lua_pushnil(L);
    return 1;
}

static int LuaSetDefaultRestoreButtonLabel(lua_State* L) {
    if (lua_gettop(L) >= 1 && !lua_isnil(L, 1)) {
        const char* alt = luaL_checkstring(L, 1);
        s_FWLDefaultRestoreLabel = alt ? alt : "";
        ImGui::SetDefaultRestoreButtonLabel(s_FWLDefaultRestoreLabel.empty() ? nullptr : s_FWLDefaultRestoreLabel.c_str());
    } else {
        s_FWLDefaultRestoreLabel.clear();
        ImGui::SetDefaultRestoreButtonLabel(nullptr);
    }
    return 0;
}

static int LuaGetDefaultRestoreButtonLabel(lua_State* L) {
    const char* lbl = ImGui::GetDefaultRestoreButtonLabel();
    if (lbl) lua_pushstring(L, lbl);
    else lua_pushnil(L);
    return 1;
}

static int LuaKnob(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float val        = static_cast<float>(luaL_checknumber(L, 2));
    float v_min      = (top >= 3 && !lua_isnil(L, 3)) ? static_cast<float>(luaL_checknumber(L, 3)) : 0.0f;
    float v_max      = (top >= 4 && !lua_isnil(L, 4)) ? static_cast<float>(luaL_checknumber(L, 4)) : 1.0f;
    const char* fmt  = (top >= 5 && !lua_isnil(L, 5)) ? luaL_checkstring(L, 5) : "%.2f";
    float radius     = (top >= 6 && !lua_isnil(L, 6)) ? static_cast<float>(luaL_checknumber(L, 6)) : 0.0f;
    float wheel_step = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<float>(luaL_checknumber(L, 7)) : 0.0f;
    ImGuiKnobFlags flags = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<ImGuiKnobFlags>(luaL_checkinteger(L, 8)) : ImGuiKnobFlags_None;

    bool changed = ImGui::Knob(label, &val, v_min, v_max, fmt, radius, wheel_step, flags);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaKnobWithReset(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    float val = static_cast<float>(luaL_checknumber(L, 2));
    float a   = static_cast<float>(luaL_checknumber(L, 3));
    float b   = static_cast<float>(luaL_checknumber(L, 4));

    float v_min = a;
    float v_max = b;
    float def_val = a;
    if (top >= 5 && lua_isnumber(L, 5)) {
        float c = static_cast<float>(lua_tonumber(L, 5));
        if (a < b) {
            // Lua SliderWithReset convention: (v_min, v_max, default_val)
            v_min   = a;
            v_max   = b;
            def_val = c;
        } else if (b < c) {
            // C++ ImGui::KnobWithReset convention: (default_val, v_min, v_max)
            def_val = a;
            v_min   = b;
            v_max   = c;
        } else {
            v_min   = a;
            v_max   = b;
            def_val = c;
        }
    }

    const char* fmt      = (top >= 6 && !lua_isnil(L, 6)) ? luaL_checkstring(L, 6) : "%.2f";
    float radius         = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<float>(luaL_checknumber(L, 7)) : 0.0f;
    float wheel_step     = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<float>(luaL_checknumber(L, 8)) : 0.0f;
    const char* tip      = (top >= 9 && !lua_isnil(L, 9)) ? luaL_checkstring(L, 9) : nullptr;
    ImGuiKnobFlags flags = (top >= 10 && !lua_isnil(L, 10)) ? static_cast<ImGuiKnobFlags>(luaL_checkinteger(L, 10)) : ImGuiKnobFlags_None;

    bool changed = ImGui::KnobWithReset(label, &val, def_val, v_min, v_max, fmt, radius, wheel_step, tip, flags);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaDimmerKnob(lua_State* L) {
    int top = lua_gettop(L);
    const char* label   = luaL_checkstring(L, 1);
    float val           = static_cast<float>(luaL_checknumber(L, 2));
    float v_min_active  = (top >= 3 && !lua_isnil(L, 3)) ? static_cast<float>(luaL_checknumber(L, 3)) : 0.05f;
    float v_max         = (top >= 4 && !lua_isnil(L, 4)) ? static_cast<float>(luaL_checknumber(L, 4)) : 1.0f;
    float v_default     = (top >= 5 && !lua_isnil(L, 5)) ? static_cast<float>(luaL_checknumber(L, 5)) : 1.0f;
    const char* fmt     = (top >= 6 && !lua_isnil(L, 6)) ? luaL_checkstring(L, 6) : "%.0f%%";
    float radius        = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<float>(luaL_checknumber(L, 7)) : 0.0f;
    float wheel_step    = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<float>(luaL_checknumber(L, 8)) : 0.02f;
    bool has_detente    = (top >= 9 && !lua_isnil(L, 9)) ? (lua_toboolean(L, 9) != 0) : true;
    float v_off         = (top >= 10 && !lua_isnil(L, 10)) ? static_cast<float>(luaL_checknumber(L, 10)) : 0.0f;
    ImGuiKnobFlags flags = (top >= 11 && !lua_isnil(L, 11)) ? static_cast<ImGuiKnobFlags>(luaL_checkinteger(L, 11)) : ImGuiKnobFlags_None;

    bool changed = ImGui::DimmerKnob(label, &val, v_min_active, v_max, v_default, fmt, radius, wheel_step, has_detente, v_off, flags);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaKnobPercent(lua_State* L) {
    int top = lua_gettop(L);
    const char* label    = luaL_checkstring(L, 1);
    float val            = static_cast<float>(luaL_checknumber(L, 2));
    float v_default      = (top >= 3 && !lua_isnil(L, 3)) ? static_cast<float>(luaL_checknumber(L, 3)) : 1.0f;
    float v_min          = (top >= 4 && !lua_isnil(L, 4)) ? static_cast<float>(luaL_checknumber(L, 4)) : 0.0f;
    float v_max          = (top >= 5 && !lua_isnil(L, 5)) ? static_cast<float>(luaL_checknumber(L, 5)) : 1.0f;
    const char* fmt      = (top >= 6 && !lua_isnil(L, 6)) ? luaL_checkstring(L, 6) : "%.0f%%";
    float radius         = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<float>(luaL_checknumber(L, 7)) : 0.0f;
    float wheel_step     = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<float>(luaL_checknumber(L, 8)) : 0.01f;
    ImGuiKnobFlags flags = (top >= 9 && !lua_isnil(L, 9)) ? static_cast<ImGuiKnobFlags>(luaL_checkinteger(L, 9)) : ImGuiKnobFlags_None;

    bool changed = ImGui::KnobEx(label, &val, v_min, v_max, v_default, fmt, radius, wheel_step, false, 0.0f, flags);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaKnobPercentWithReset(lua_State* L) {
    int top = lua_gettop(L);
    const char* label    = luaL_checkstring(L, 1);
    float val            = static_cast<float>(luaL_checknumber(L, 2));
    float v_default      = (top >= 3 && !lua_isnil(L, 3)) ? static_cast<float>(luaL_checknumber(L, 3)) : 1.0f;
    float radius         = (top >= 4 && !lua_isnil(L, 4)) ? static_cast<float>(luaL_checknumber(L, 4)) : 0.0f;
    float wheel_step     = (top >= 5 && !lua_isnil(L, 5)) ? static_cast<float>(luaL_checknumber(L, 5)) : 0.01f;
    const char* tip      = (top >= 6 && !lua_isnil(L, 6)) ? luaL_checkstring(L, 6) : nullptr;
    ImGuiKnobFlags flags = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<ImGuiKnobFlags>(luaL_checkinteger(L, 7)) : ImGuiKnobFlags_None;

    bool changed = ImGui::KnobWithReset(label, &val, v_default, 0.0f, 1.0f, "%.0f%%", radius, wheel_step, tip, flags);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    return 2;
}

static int LuaKnobEx(lua_State* L) {
    int top = lua_gettop(L);
    const char* label    = luaL_checkstring(L, 1);
    float val            = static_cast<float>(luaL_checknumber(L, 2));
    float v_min          = static_cast<float>(luaL_checknumber(L, 3));
    float v_max          = static_cast<float>(luaL_checknumber(L, 4));
    float v_default      = (top >= 5 && !lua_isnil(L, 5)) ? static_cast<float>(luaL_checknumber(L, 5)) : 1.0f;
    const char* fmt      = (top >= 6 && !lua_isnil(L, 6)) ? luaL_checkstring(L, 6) : "%.2f";
    float radius         = (top >= 7 && !lua_isnil(L, 7)) ? static_cast<float>(luaL_checknumber(L, 7)) : 0.0f;
    float wheel_step     = (top >= 8 && !lua_isnil(L, 8)) ? static_cast<float>(luaL_checknumber(L, 8)) : 0.0f;
    bool has_detente     = (top >= 9 && !lua_isnil(L, 9)) ? (lua_toboolean(L, 9) != 0) : false;
    float v_off          = (top >= 10 && !lua_isnil(L, 10)) ? static_cast<float>(luaL_checknumber(L, 10)) : 0.0f;
    ImGuiKnobFlags flags = (top >= 11 && !lua_isnil(L, 11)) ? static_cast<ImGuiKnobFlags>(luaL_checkinteger(L, 11)) : ImGuiKnobFlags_None;

    bool is_active = false;
    bool is_hovered = false;
    bool is_scrolling = false;
    bool changed = ImGui::KnobEx(label, &val, v_min, v_max, v_default, fmt, radius, wheel_step, has_detente, v_off, flags,
                                 &is_active, &is_hovered, &is_scrolling);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, val);
    lua_pushboolean(L, is_active);
    lua_pushboolean(L, is_hovered);
    lua_pushboolean(L, is_scrolling);
    return 5;
}

static int LuaResetButton(lua_State* L) {
    int top = lua_gettop(L);
    const char* str_id = luaL_checkstring(L, 1);
    bool is_mod        = (top >= 2 && !lua_isnil(L, 2)) ? (lua_toboolean(L, 2) != 0) : true;
    float width        = 0.0f;
    bool abutting      = true;
    const char* r_lbl  = nullptr;
    bool restore_mode  = false;
    const char* a_lbl  = nullptr;

    if (top >= 3 && lua_type(L, 3) == LUA_TBOOLEAN) {
        // Short Lua form: (str_id, is_modified, abutting, [reset_label], [is_restore_mode], [restore_label])
        abutting     = (lua_toboolean(L, 3) != 0);
        r_lbl        = (top >= 4 && !lua_isnil(L, 4)) ? lua_tostring(L, 4) : nullptr;
        restore_mode = (top >= 5 && !lua_isnil(L, 5)) ? (lua_toboolean(L, 5) != 0) : false;
        a_lbl        = (top >= 6 && !lua_isnil(L, 6)) ? lua_tostring(L, 6) : nullptr;
    } else if (top >= 3) {
        // Full C++ ImGee form: (str_id, is_modified, width, [abutting], [reset_label], [is_restore_mode], [restore_label])
        width        = lua_isnumber(L, 3) ? static_cast<float>(lua_tonumber(L, 3)) : 0.0f;
        abutting     = (top >= 4 && !lua_isnil(L, 4)) ? (lua_toboolean(L, 4) != 0) : false;
        r_lbl        = (top >= 5 && !lua_isnil(L, 5)) ? lua_tostring(L, 5) : nullptr;
        restore_mode = (top >= 6 && !lua_isnil(L, 6)) ? (lua_toboolean(L, 6) != 0) : false;
        a_lbl        = (top >= 7 && !lua_isnil(L, 7)) ? lua_tostring(L, 7) : nullptr;
    }

    bool clicked = ImGui::ResetButton(str_id, is_mod, width, abutting, r_lbl, restore_mode, a_lbl);
    lua_pushboolean(L, clicked);
    return 1;
}

static bool IsLikelyUrlTarget(const char* s) {
    if (!s || s[0] == '\0') return false;
    if (s[0] == '#') return false;
    if (std::strstr(s, "://") != nullptr || std::strncmp(s, "local:", 6) == 0 || std::strncmp(s, "file:", 5) == 0) {
        return true;
    }
    if (std::strchr(s, '/') != nullptr || std::strchr(s, '\\') != nullptr || std::strchr(s, '.') != nullptr) {
        return true;
    }
    ImVec4 dummy;
    if (ImGui::ParseMarkdownColor(s, dummy)) {
        return false;
    }
    return true;
}

static int LuaButtonLink(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    const char* url   = nullptr;
    float w = 0.0f, h = 0.0f;

    if (top >= 2 && lua_type(L, 2) == LUA_TSTRING && !lua_isnumber(L, 2)) {
        url = lua_tostring(L, 2);
        if (top >= 3 && lua_isnumber(L, 3)) w = static_cast<float>(lua_tonumber(L, 3));
        if (top >= 4 && lua_isnumber(L, 4)) h = static_cast<float>(lua_tonumber(L, 4));
    } else {
        if (top >= 2 && lua_isnumber(L, 2)) w = static_cast<float>(lua_tonumber(L, 2));
        if (top >= 3 && lua_isnumber(L, 3)) h = static_cast<float>(lua_tonumber(L, 3));
    }

    ImGui::LinkClickResult res = ImGui::ButtonLink(label, ImVec2(w, h));
    bool clicked = false;
    if (url && url[0] != '\0') {
        clicked = static_cast<bool>(res); // Explicitly mark click consumed for URL launch
        if (clicked) OpenBrowserURL(url);
    } else {
        clicked = res.clicked; // Preserve unconsumed click-to-toggle tooltip eligibility
    }
    lua_pushboolean(L, clicked);
    return 1;
}

static int LuaTextLink(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    const char* url   = nullptr;
    ImVec4 col_override(0.0f, 0.0f, 0.0f, 0.0f);
    bool has_col = false;

    if (top >= 2 && !lua_isnil(L, 2)) {
        if (lua_type(L, 2) == LUA_TSTRING && IsLikelyUrlTarget(lua_tostring(L, 2))) {
            url = lua_tostring(L, 2);
            has_col = ParseLuaColorVec4(L, 3, col_override);
        } else {
            has_col = ParseLuaColorVec4(L, 2, col_override);
            if (!has_col && lua_type(L, 2) == LUA_TSTRING) {
                url = lua_tostring(L, 2);
            }
        }
    }

    ImGui::LinkClickResult res = ImGui::TextLinkContrib(label, has_col ? &col_override : nullptr);
    bool clicked = false;
    if (url && url[0] != '\0') {
        clicked = static_cast<bool>(res);
        if (clicked) OpenBrowserURL(url);
    } else {
        clicked = res.clicked;
    }
    lua_pushboolean(L, clicked);
    return 1;
}

static int LuaSmallButtonLink(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    const char* url   = (top >= 2 && lua_type(L, 2) == LUA_TSTRING) ? lua_tostring(L, 2) : nullptr;
    ImGui::LinkClickResult res = ImGui::SmallButtonLink(label);
    bool clicked = false;
    if (url && url[0] != '\0') {
        clicked = static_cast<bool>(res);
        if (clicked) OpenBrowserURL(url);
    } else {
        clicked = res.clicked;
    }
    lua_pushboolean(L, clicked);
    return 1;
}

static int LuaExternalTextLink(lua_State* L) {
    const char* label = luaL_checkstring(L, 1);
    const char* url   = (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) ? luaL_checkstring(L, 2) : nullptr;
    bool clicked = ImGui::ExternalTextLink(label, url);
    lua_pushboolean(L, clicked);
    return 1;
}

static int LuaCheckboxLink(lua_State* L) {
    const char* label     = luaL_checkstring(L, 1);
    const bool arg_is_num = (lua_type(L, 2) == LUA_TNUMBER);
    bool val              = arg_is_num ? (lua_tointeger(L, 2) != 0) : (lua_toboolean(L, 2) != 0);
    bool changed          = ImGui::CheckboxLink(label, &val);
    lua_pushboolean(L, changed);
    if (arg_is_num) {
        lua_pushinteger(L, val ? 1 : 0);
    } else {
        lua_pushboolean(L, val);
    }
    return 2;
}

static int LuaRadioButtonLink(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    if (top >= 3 && lua_type(L, 2) == LUA_TNUMBER && lua_type(L, 3) == LUA_TNUMBER) {
        int v        = static_cast<int>(lua_tointeger(L, 2));
        int v_button = static_cast<int>(lua_tointeger(L, 3));
        bool changed = ImGui::RadioButtonLink(label, &v, v_button);
        lua_pushboolean(L, changed);
        lua_pushinteger(L, v);
        return 2;
    }
    bool active  = lua_toboolean(L, 2) != 0;
    bool clicked = ImGui::RadioButtonLink(label, active);
    lua_pushboolean(L, clicked);
    return 1;
}

static int LuaSelectableLink(lua_State* L) {
    int top = lua_gettop(L);
    const char* label = luaL_checkstring(L, 1);
    bool sel          = (top >= 2 && !lua_isnil(L, 2)) ? (lua_toboolean(L, 2) != 0) : false;
    ImGuiSelectableFlags flags = (top >= 3 && lua_isnumber(L, 3)) ? static_cast<ImGuiSelectableFlags>(lua_tointeger(L, 3)) : 0;
    float w = (top >= 4 && lua_isnumber(L, 4)) ? static_cast<float>(lua_tonumber(L, 4)) : 0.0f;
    float h = (top >= 5 && lua_isnumber(L, 5)) ? static_cast<float>(lua_tonumber(L, 5)) : 0.0f;
    bool clicked = ImGui::SelectableLink(label, &sel, flags, ImVec2(w, h));
    lua_pushboolean(L, clicked);
    lua_pushboolean(L, sel);
    return 2;
}

static int LuaMenuItemLink(lua_State* L) {
    int top = lua_gettop(L);
    const char* label    = luaL_checkstring(L, 1);
    const char* shortcut = (top >= 2 && lua_type(L, 2) == LUA_TSTRING) ? lua_tostring(L, 2) : nullptr;
    bool sel             = (top >= 3 && !lua_isnil(L, 3)) ? (lua_toboolean(L, 3) != 0) : false;
    bool enabled         = (top >= 4 && !lua_isnil(L, 4)) ? (lua_toboolean(L, 4) != 0) : true;
    ImGui::LinkClickResult res = ImGui::MenuItemLink(label, shortcut, &sel, enabled);
    lua_pushboolean(L, res.clicked);
    lua_pushboolean(L, sel);
    return 2;
}

static int LuaIcon(lua_State* L) {
    int top = lua_gettop(L);
    const char* name = luaL_checkstring(L, 1);
    float sz = 0.0f;
    int col_idx = 2;
    if (top >= 2 && lua_type(L, 2) == LUA_TNUMBER) {
        sz = static_cast<float>(lua_tonumber(L, 2));
        col_idx = 3;
    } else if (top >= 2 && lua_isnil(L, 2)) {
        col_idx = 3;
    }
    ImVec4 col(0.0f, 0.0f, 0.0f, 0.0f);
    bool has_col = ParseLuaColorVec4(L, col_idx, col);
    ImGui::Icon(name, sz, has_col ? &col : nullptr);
    return 0;
}

static int LuaIsVectorIcon(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    lua_pushboolean(L, ImGui::IsVectorIcon(name));
    return 1;
}

static int LuaNormalizeIconName(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);
    std::string norm = ImGui::NormalizeIconName(name);
    lua_pushlstring(L, norm.c_str(), norm.size());
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
    ImVec4 icon_col(0.0f, 0.0f, 0.0f, 0.0f);
    bool has_col = ParseLuaColorVec4(L, 6, icon_col);
    bool clicked = ImGui::IconButton(str_id, icon_name, ImVec2(w, h), tooltip, has_col ? &icon_col : nullptr);
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
    ImVec4 icon_col(0.0f, 0.0f, 0.0f, 0.0f);
    bool has_col = ParseLuaColorVec4(L, 7, icon_col);
    bool clicked = ImGui::ButtonIcon(label, icon_name, ImVec2(w, h), icon_on_right, tooltip, has_col ? &icon_col : nullptr);
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

static bool IsCustomBadgeColorArg(lua_State* L, int idx) {
    if (lua_gettop(L) < idx || lua_isnil(L, idx)) return false;
    int t = lua_type(L, idx);
    if (t == LUA_TTABLE) return true;
    if (t == LUA_TNUMBER && lua_tonumber(L, idx) > 10.0) return true;
    if (t == LUA_TSTRING) {
        const char* s = lua_tostring(L, idx);
        if (s && (s[0] == '#' || (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')))) return true;
    }
    return false;
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
    int top = lua_gettop(L);
    const char* text = luaL_checkstring(L, 1);
    if (!text) return 0;

    if (IsCustomBadgeColorArg(L, 2)) {
        ImU32 bg_col      = ParseLuaColor(L, 2);
        ImU32 fg_col      = (top >= 3 && IsCustomBadgeColorArg(L, 3)) ? ParseLuaColor(L, 3) : 0;
        float rounding    = (top >= 4 && lua_isnumber(L, 4)) ? static_cast<float>(lua_tonumber(L, 4)) : -1.0f;
        const char* iname = (top >= 5 && lua_type(L, 5) == LUA_TSTRING) ? lua_tostring(L, 5) : nullptr;
        float iw          = (top >= 6 && lua_isnumber(L, 6)) ? static_cast<float>(lua_tonumber(L, 6)) : 0.0f;
        float ih          = (top >= 7 && lua_isnumber(L, 7)) ? static_cast<float>(lua_tonumber(L, 7)) : 0.0f;
        ImGui::Badge(text, bg_col, fg_col, rounding, (ImTextureID)0, ImVec2(iw, ih), iname);
    } else {
        ImGui::ImGuiBadgeVariant variant = (top >= 2 && !lua_isnil(L, 2))
                                               ? ParseLuaBadgeVariant(L, 2)
                                               : ImGui::ImGuiBadgeVariant_Default;
        float rounding    = (top >= 3 && lua_isnumber(L, 3)) ? static_cast<float>(lua_tonumber(L, 3)) : -1.0f;
        const char* iname = (top >= 4 && lua_type(L, 4) == LUA_TSTRING) ? lua_tostring(L, 4) : nullptr;
        float iw          = (top >= 5 && lua_isnumber(L, 5)) ? static_cast<float>(lua_tonumber(L, 5)) : 0.0f;
        float ih          = (top >= 6 && lua_isnumber(L, 6)) ? static_cast<float>(lua_tonumber(L, 6)) : 0.0f;
        ImGui::Badge(text, variant, rounding, (ImTextureID)0, ImVec2(iw, ih), iname);
    }
    return 0;
}

static int LuaPill(lua_State* L) {
    int top = lua_gettop(L);
    const char* text = luaL_checkstring(L, 1);
    if (!text) return 0;

    if (IsCustomBadgeColorArg(L, 2)) {
        ImU32 bg_col      = ParseLuaColor(L, 2);
        ImU32 fg_col      = (top >= 3 && IsCustomBadgeColorArg(L, 3)) ? ParseLuaColor(L, 3) : 0;
        const char* iname = (top >= 4 && lua_type(L, 4) == LUA_TSTRING) ? lua_tostring(L, 4) : nullptr;
        float iw          = (top >= 5 && lua_isnumber(L, 5)) ? static_cast<float>(lua_tonumber(L, 5)) : 0.0f;
        float ih          = (top >= 6 && lua_isnumber(L, 6)) ? static_cast<float>(lua_tonumber(L, 6)) : 0.0f;
        ImGui::Pill(text, bg_col, fg_col, (ImTextureID)0, ImVec2(iw, ih), iname);
    } else {
        ImGui::ImGuiBadgeVariant variant = (top >= 2 && !lua_isnil(L, 2))
                                               ? ParseLuaBadgeVariant(L, 2)
                                               : ImGui::ImGuiBadgeVariant_Default;
        const char* iname = (top >= 3 && lua_type(L, 3) == LUA_TSTRING) ? lua_tostring(L, 3) : nullptr;
        float iw          = (top >= 4 && lua_isnumber(L, 4)) ? static_cast<float>(lua_tonumber(L, 4)) : 0.0f;
        float ih          = (top >= 5 && lua_isnumber(L, 5)) ? static_cast<float>(lua_tonumber(L, 5)) : 0.0f;
        ImGui::Pill(text, variant, (ImTextureID)0, ImVec2(iw, ih), iname);
    }
    return 0;
}

static int LuaBadgeNamed(lua_State* L) {
    int top = lua_gettop(L);
    const char* imageName = luaL_checkstring(L, 1);
    if (!imageName) return 0;

    const char* text = nullptr;
    int col_arg = 2;
    if (top >= 2 && lua_type(L, 2) == LUA_TSTRING && !IsCustomBadgeColorArg(L, 2)) {
        text = lua_tostring(L, 2);
        col_arg = 3;
    }

    if (top >= col_arg && IsCustomBadgeColorArg(L, col_arg)) {
        ImU32 bg_col   = ParseLuaColor(L, col_arg);
        int next_idx   = col_arg + 1;
        ImU32 fg_col   = 0;
        if (top >= next_idx && IsCustomBadgeColorArg(L, next_idx)) {
            fg_col = ParseLuaColor(L, next_idx++);
        } else if (top >= next_idx && lua_isnil(L, next_idx)) {
            next_idx++;
        }
        float rounding = (top >= next_idx && lua_isnumber(L, next_idx)) ? static_cast<float>(lua_tonumber(L, next_idx++)) : -1.0f;
        float iw       = (top >= next_idx && lua_isnumber(L, next_idx)) ? static_cast<float>(lua_tonumber(L, next_idx++)) : 0.0f;
        float ih       = (top >= next_idx && lua_isnumber(L, next_idx)) ? static_cast<float>(lua_tonumber(L, next_idx++)) : 0.0f;
        ImGui::BadgeNamed(imageName, text, bg_col, fg_col, rounding, ImVec2(iw, ih));
    } else {
        ImGui::BadgeNamed(imageName, text);
    }
    return 0;
}

static int LuaPillNamed(lua_State* L) {
    int top = lua_gettop(L);
    const char* imageName = luaL_checkstring(L, 1);
    if (!imageName) return 0;

    const char* text = nullptr;
    int col_arg = 2;
    if (top >= 2 && lua_type(L, 2) == LUA_TSTRING && !IsCustomBadgeColorArg(L, 2)) {
        text = lua_tostring(L, 2);
        col_arg = 3;
    }

    if (top >= col_arg && IsCustomBadgeColorArg(L, col_arg)) {
        ImU32 bg_col = ParseLuaColor(L, col_arg);
        int next_idx = col_arg + 1;
        ImU32 fg_col = 0;
        if (top >= next_idx && IsCustomBadgeColorArg(L, next_idx)) {
            fg_col = ParseLuaColor(L, next_idx++);
        } else if (top >= next_idx && lua_isnil(L, next_idx)) {
            next_idx++;
        }
        float iw = (top >= next_idx && lua_isnumber(L, next_idx)) ? static_cast<float>(lua_tonumber(L, next_idx++)) : 0.0f;
        float ih = (top >= next_idx && lua_isnumber(L, next_idx)) ? static_cast<float>(lua_tonumber(L, next_idx++)) : 0.0f;
        ImGui::PillNamed(imageName, text, bg_col, fg_col, ImVec2(iw, ih));
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

static int LuaGetMouseWheelH(lua_State* L) {
    bool consume = (lua_gettop(L) >= 1) ? (lua_toboolean(L, 1) != 0) : true;
    float delta = ImGui::GetMouseWheelH(consume);
    lua_pushnumber(L, delta);
    return 1;
}

static int LuaGetMouseWheelEither(lua_State* L) {
    bool consume = (lua_gettop(L) >= 1) ? (lua_toboolean(L, 1) != 0) : true;
    float delta = ImGui::GetMouseWheelEither(consume);
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
    bool wrap = false;
    bool natural_list_order = true;
    if (top >= arg_idx && !lua_isnil(L, arg_idx)) {
        wrap = (lua_toboolean(L, arg_idx) != 0);
        arg_idx++;
    }
    if (top >= arg_idx && !lua_isnil(L, arg_idx)) {
        natural_list_order = (lua_toboolean(L, arg_idx) != 0);
        arg_idx++;
    }

    bool changed = ImGui::SelectPulldown(label, &current_item, items, has_tooltips ? &tooltips : nullptr, width, anchor, align_mode, wrap, natural_list_order);
    lua_pushboolean(L, changed);
    lua_pushinteger(L, current_item);
    return 2;
}

static std::string s_FWLPulldownCaretDown;
static std::string s_FWLPulldownCaretUp;

static int LuaSetDefaultPulldownCaret(lua_State* L) {
    const char* down_ptr = nullptr;
    const char* up_ptr   = nullptr;
    if (lua_gettop(L) >= 1 && !lua_isnil(L, 1)) {
        const char* down = lua_tostring(L, 1);
        s_FWLPulldownCaretDown = down ? down : "";
        down_ptr = s_FWLPulldownCaretDown.empty() ? nullptr : s_FWLPulldownCaretDown.c_str();
    } else {
        s_FWLPulldownCaretDown.clear();
    }
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        const char* up = lua_tostring(L, 2);
        s_FWLPulldownCaretUp = up ? up : "";
        up_ptr = s_FWLPulldownCaretUp.empty() ? nullptr : s_FWLPulldownCaretUp.c_str();
    } else {
        s_FWLPulldownCaretUp.clear();
    }
    ImGui::SetDefaultPulldownCaret(down_ptr, up_ptr);
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
    ImGui::StyledTextWrapped("%s", s.c_str());
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

static int LuaBoldItalicText(lua_State* L) {
    std::string s = FormatLuaString(L);
    ImGui::BoldItalicText("%s", s.c_str());
    return 0;
}

static int LuaBoldItalicTextUnformatted(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    if (s) ImGui::BoldItalicTextUnformatted(s);
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

// ----------------------------------------------------------------------------
// 16. Two-Layer Zoomable Viewport (imgui_zoomable_viewport.h)
// ----------------------------------------------------------------------------

#if defined(IMGUI_ZOOMABLE_VIEWPORT_H)
struct ActiveLuaZoomViewport {
    ImGui::ZoomableViewportState* state = nullptr;
    int lua_table_ref = LUA_NOREF;
};

static std::unordered_map<uintptr_t, ImGui::ZoomableViewportState> s_TableViewportStates;
static std::unordered_map<std::string, ImGui::ZoomableViewportState> s_NamedViewportStates;
static std::vector<ActiveLuaZoomViewport> s_ActiveLuaZoomViewports;

static void SyncLuaTableToViewportState(lua_State* L, int idx, ImGui::ZoomableViewportState& st) {
    int abs_idx = (idx < 0) ? (lua_gettop(L) + idx + 1) : idx;
    if (!lua_istable(L, abs_idx)) return;

    auto readFloat = [&](const char* k, float& dst) {
        lua_getfield(L, abs_idx, k);
        if (lua_type(L, -1) == LUA_TNUMBER) dst = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
    };
    auto readBool = [&](const char* k, bool& dst) {
        lua_getfield(L, abs_idx, k);
        if (lua_isboolean(L, -1)) dst = (lua_toboolean(L, -1) != 0);
        lua_pop(L, 1);
    };

    readFloat("zoom", st.zoom);
    readFloat("default_zoom", st.default_zoom);
    readFloat("last_zoom", st.last_zoom);
    readFloat("min_zoom", st.min_zoom);
    readFloat("max_zoom", st.max_zoom);
    readFloat("wheel_step", st.wheel_step);
    readFloat("button_step", st.button_step);
    readBool("enable_ctrl_wheel", st.enable_ctrl_wheel);
    readBool("border", st.border);

    lua_getfield(L, abs_idx, "content_padding");
    if (lua_type(L, -1) == LUA_TNUMBER) {
        float pad = static_cast<float>(lua_tonumber(L, -1));
        st.content_padding = ImVec2(pad, pad);
    } else if (lua_istable(L, -1)) {
        lua_rawgeti(L, -1, 1);
        float px = lua_isnumber(L, -1) ? static_cast<float>(lua_tonumber(L, -1)) : -1.0f;
        lua_pop(L, 1);
        lua_rawgeti(L, -1, 2);
        float py = lua_isnumber(L, -1) ? static_cast<float>(lua_tonumber(L, -1)) : -1.0f;
        lua_pop(L, 1);
        st.content_padding = ImVec2(px, py);
    }
    lua_pop(L, 1);
}

static void SyncViewportStateToLuaTable(lua_State* L, int idx, const ImGui::ZoomableViewportState& st) {
    int abs_idx = (idx < 0) ? (lua_gettop(L) + idx + 1) : idx;
    if (!lua_istable(L, abs_idx)) return;
    lua_pushnumber(L, st.zoom);              lua_setfield(L, abs_idx, "zoom");
    lua_pushnumber(L, st.last_zoom);         lua_setfield(L, abs_idx, "last_zoom");
    lua_pushinteger(L, st.GetZoomPercent()); lua_setfield(L, abs_idx, "zoom_percent");
}

static ImGui::ZoomableViewportState* ResolveLuaViewportState(lua_State* L, int idx, const char* fallback_id) {
    int abs_idx = (idx < 0) ? (lua_gettop(L) + idx + 1) : idx;
    if (abs_idx >= 1 && abs_idx <= lua_gettop(L)) {
        if (lua_istable(L, abs_idx)) {
            uintptr_t key = reinterpret_cast<uintptr_t>(lua_topointer(L, abs_idx));
            ImGui::ZoomableViewportState& st = s_TableViewportStates[key];
            SyncLuaTableToViewportState(L, abs_idx, st);
            return &st;
        }
        if (lua_type(L, abs_idx) == LUA_TSTRING) {
            const char* id = lua_tostring(L, abs_idx);
            return &s_NamedViewportStates[id ? id : "##default_zoom_vp"];
        }
    }
    return &s_NamedViewportStates[fallback_id ? fallback_id : "##default_zoom_vp"];
}

static int LuaBeginZoomedViewport(lua_State* L) {
    int top = lua_gettop(L);
    const char* str_id = luaL_checkstring(L, 1);
    ImGui::ZoomableViewportState* st = nullptr;
    int tbl_ref = LUA_NOREF;
    int size_idx = 3;

    if (top >= 2 && lua_istable(L, 2)) {
        st = ResolveLuaViewportState(L, 2, str_id);
        lua_pushvalue(L, 2);
        tbl_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        size_idx = 3;
    } else if (top >= 2 && lua_type(L, 2) == LUA_TNUMBER) {
        st = &s_NamedViewportStates[str_id ? str_id : "##default_zoom_vp"];
        size_idx = 2;
    } else {
        st = &s_NamedViewportStates[str_id ? str_id : "##default_zoom_vp"];
        size_idx = 3;
    }

    float w = 0.0f, h = 0.0f;
    ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar;
    if (top >= size_idx) {
        if (lua_istable(L, size_idx)) {
            lua_rawgeti(L, size_idx, 1); w = static_cast<float>(lua_tonumber(L, -1)); lua_pop(L, 1);
            lua_rawgeti(L, size_idx, 2); h = static_cast<float>(lua_tonumber(L, -1)); lua_pop(L, 1);
            if (top >= size_idx + 1 && lua_isnumber(L, size_idx + 1)) {
                flags = static_cast<ImGuiWindowFlags>(lua_tointeger(L, size_idx + 1));
            }
        } else if (lua_isnumber(L, size_idx)) {
            w = static_cast<float>(lua_tonumber(L, size_idx));
            if (top >= size_idx + 1 && lua_isnumber(L, size_idx + 1)) {
                h = static_cast<float>(lua_tonumber(L, size_idx + 1));
            }
            if (top >= size_idx + 2 && lua_isnumber(L, size_idx + 2)) {
                flags = static_cast<ImGuiWindowFlags>(lua_tointeger(L, size_idx + 2));
            }
        }
    }

    bool visible = ImGui::BeginZoomedViewport(str_id, *st, ImVec2(w, h), flags);
    if (tbl_ref != LUA_NOREF && lua_istable(L, 2)) {
        SyncViewportStateToLuaTable(L, 2, *st);
    }
    ActiveLuaZoomViewport active;
    active.state = st;
    active.lua_table_ref = tbl_ref;
    s_ActiveLuaZoomViewports.push_back(active);

    lua_pushboolean(L, visible);
    lua_pushnumber(L, st->zoom);
    return 2;
}

static int LuaEndZoomedViewport(lua_State* L) {
    if (!s_ActiveLuaZoomViewports.empty()) {
        ActiveLuaZoomViewport active = s_ActiveLuaZoomViewports.back();
        s_ActiveLuaZoomViewports.pop_back();
        if (active.state) {
            ImGui::EndZoomedViewport(*active.state);
            if (active.lua_table_ref != LUA_NOREF) {
                lua_rawgeti(L, LUA_REGISTRYINDEX, active.lua_table_ref);
                if (lua_istable(L, -1)) {
                    SyncViewportStateToLuaTable(L, -1, *active.state);
                }
                lua_pop(L, 1);
                luaL_unref(L, LUA_REGISTRYINDEX, active.lua_table_ref);
            }
        } else {
            ImGui::EndZoomedViewport();
        }
    } else {
        ImGui::EndZoomedViewport();
    }
    return 0;
}

static int LuaZoomViewportControls(lua_State* L) {
    int top = lua_gettop(L);
    const char* id_suffix = (top >= 2 && lua_type(L, 2) == LUA_TSTRING) ? lua_tostring(L, 2) : "##imgee_zoom_ctrl";
    ImGui::ZoomableViewportState* st = ResolveLuaViewportState(L, 1, id_suffix);
    bool changed = ImGui::ZoomViewportControls(*st, id_suffix);
    if (top >= 1 && lua_istable(L, 1)) {
        SyncViewportStateToLuaTable(L, 1, *st);
    }
    lua_pushboolean(L, changed);
    lua_pushnumber(L, st->zoom);
    return 2;
}

static int LuaSetViewportZoom(lua_State* L) {
    float target = static_cast<float>(luaL_checknumber(L, 2));
    ImGui::ZoomableViewportState* st = ResolveLuaViewportState(L, 1, "##default_zoom_vp");
    bool changed = ImGui::SetViewportZoom(*st, target);
    if (lua_istable(L, 1)) SyncViewportStateToLuaTable(L, 1, *st);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, st->zoom);
    return 2;
}

static int LuaStepViewportZoom(lua_State* L) {
    float delta = static_cast<float>(luaL_checknumber(L, 2));
    ImGui::ZoomableViewportState* st = ResolveLuaViewportState(L, 1, "##default_zoom_vp");
    bool changed = ImGui::StepViewportZoom(*st, delta);
    if (lua_istable(L, 1)) SyncViewportStateToLuaTable(L, 1, *st);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, st->zoom);
    return 2;
}

static int LuaResetViewportZoom(lua_State* L) {
    bool allow_undo = (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) ? (lua_toboolean(L, 2) != 0) : false;
    ImGui::ZoomableViewportState* st = ResolveLuaViewportState(L, 1, "##default_zoom_vp");
    bool changed = ImGui::ResetViewportZoom(*st, allow_undo);
    if (lua_istable(L, 1)) SyncViewportStateToLuaTable(L, 1, *st);
    lua_pushboolean(L, changed);
    lua_pushnumber(L, st->zoom);
    return 2;
}
#endif // IMGUI_ZOOMABLE_VIEWPORT_H

// ----------------------------------------------------------------------------
// 17. Standalone DocumentViewer & Turnkey ImgDocWindow (imgui_doc_viewer.h / ImgDocWindow.h)
// ----------------------------------------------------------------------------

#if defined(IMGUI_DOC_VIEWER_H)
static std::unordered_map<uintptr_t, ImGui::DocumentViewerState> s_TableDocViewerStates;
static std::unordered_map<std::string, ImGui::DocumentViewerState> s_NamedDocViewerStates;

static std::string GetCallerLuaScriptDir(lua_State* L) {
    for (int lvl = 1; lvl <= 4; ++lvl) {
        lua_Debug ar = {};
        if (lua_getstack(L, lvl, &ar) != 0 && lua_getinfo(L, "S", &ar) != 0) {
            if (ar.source != nullptr && ar.source[0] == '@') {
                std::string src(ar.source + 1);
                size_t sep = src.find_last_of("/\\");
                if (sep != std::string::npos) {
                    return src.substr(0, sep + 1);
                }
            }
        }
    }
    lua_getglobal(L, "SCRIPT_DIRECTORY");
    std::string fallback;
    if (lua_isstring(L, -1)) {
        fallback = lua_tostring(L, -1);
    }
    lua_pop(L, 1);
    return fallback;
}

static void SyncLuaTableToDocViewerState(lua_State* L, int idx, ImGui::DocumentViewerState& st) {
    int abs_idx = (idx < 0) ? (lua_gettop(L) + idx + 1) : idx;
    if (!lua_istable(L, abs_idx)) return;

    auto readFloat = [&](const char* k, float& dst) {
        lua_getfield(L, abs_idx, k);
        if (lua_type(L, -1) == LUA_TNUMBER) dst = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
    };
    auto readInt = [&](const char* k, int& dst) {
        lua_getfield(L, abs_idx, k);
        if (lua_type(L, -1) == LUA_TNUMBER) dst = static_cast<int>(lua_tointeger(L, -1));
        lua_pop(L, 1);
    };
    auto readBool = [&](const char* k, bool& dst) {
        lua_getfield(L, abs_idx, k);
        if (lua_isboolean(L, -1)) dst = (lua_toboolean(L, -1) != 0);
        lua_pop(L, 1);
    };

    readFloat("zoom", st.viewport.zoom);
    readFloat("default_zoom", st.viewport.default_zoom);
    readFloat("last_zoom", st.viewport.last_zoom);
    readFloat("min_zoom", st.viewport.min_zoom);
    readFloat("max_zoom", st.viewport.max_zoom);
    readFloat("wheel_step", st.viewport.wheel_step);
    readFloat("button_step", st.viewport.button_step);
    readBool("border", st.viewport.border);

    readInt("theme_index", st.theme_index);
    readBool("transparent_hud", st.transparent_hud);
    readFloat("window_opacity", st.window_opacity);
    readBool("dynamic_cursors_enabled", st.dynamic_cursors_enabled);
    readBool("is_open", st.is_open);
}

static void SyncDocViewerStateToLuaTable(lua_State* L, int idx, const ImGui::DocumentViewerState& st) {
    int abs_idx = (idx < 0) ? (lua_gettop(L) + idx + 1) : idx;
    if (!lua_istable(L, abs_idx)) return;
    lua_pushnumber(L, st.viewport.zoom);      lua_setfield(L, abs_idx, "zoom");
    lua_pushnumber(L, st.viewport.last_zoom); lua_setfield(L, abs_idx, "last_zoom");
    lua_pushinteger(L, static_cast<int>(std::round(st.viewport.zoom * 100.0f))); lua_setfield(L, abs_idx, "zoom_percent");
    lua_pushinteger(L, st.theme_index);       lua_setfield(L, abs_idx, "theme_index");
    lua_pushboolean(L, st.transparent_hud);   lua_setfield(L, abs_idx, "transparent_hud");
    lua_pushnumber(L, st.window_opacity);     lua_setfield(L, abs_idx, "window_opacity");
    lua_pushboolean(L, st.dynamic_cursors_enabled); lua_setfield(L, abs_idx, "dynamic_cursors_enabled");
    lua_pushboolean(L, st.is_open);           lua_setfield(L, abs_idx, "is_open");
    lua_pushinteger(L, st.active_toc_index + 1); lua_setfield(L, abs_idx, "active_toc_index");
    lua_pushnumber(L, st.last_scroll_y);      lua_setfield(L, abs_idx, "scroll_y");
}

static ImGui::DocumentViewerState* ResolveLuaDocViewerState(lua_State* L, int idx, const char* fallback_id) {
    int abs_idx = (idx < 0) ? (lua_gettop(L) + idx + 1) : idx;
    if (abs_idx >= 1 && abs_idx <= lua_gettop(L)) {
        if (lua_istable(L, abs_idx)) {
            uintptr_t key = reinterpret_cast<uintptr_t>(lua_topointer(L, abs_idx));
            ImGui::DocumentViewerState& st = s_TableDocViewerStates[key];
            SyncLuaTableToDocViewerState(L, abs_idx, st);
            return &st;
        }
        if (lua_type(L, abs_idx) == LUA_TSTRING) {
            const char* id = lua_tostring(L, abs_idx);
            return &s_NamedDocViewerStates[id ? id : "##default_doc_viewer"];
        }
    }
    return &s_NamedDocViewerStates[fallback_id ? fallback_id : "##default_doc_viewer"];
}

static int LuaDocumentViewer(lua_State* L) {
    int top = lua_gettop(L);
    const char* str_id = luaL_checkstring(L, 1);
    size_t md_len = 0;
    const char* md_text = luaL_checklstring(L, 2, &md_len);

    ImGui::DocumentViewerState* st = nullptr;
    bool has_state_tbl = false;
    int cfg_idx = 0;
    int size_idx = 0;

    if (top >= 3 && lua_istable(L, 3)) {
        st = ResolveLuaDocViewerState(L, 3, str_id);
        has_state_tbl = true;
        if (top >= 4 && lua_istable(L, 4)) {
            cfg_idx = 4;
            size_idx = 5;
        } else if (top >= 4 && lua_isnil(L, 4)) {
            size_idx = 5;
        } else {
            size_idx = 4;
        }
    } else if (top >= 3 && lua_isnil(L, 3)) {
        st = &s_NamedDocViewerStates[str_id ? str_id : "##default_doc_viewer"];
        if (top >= 4 && lua_istable(L, 4)) {
            cfg_idx = 4;
            size_idx = 5;
        } else {
            size_idx = 5;
        }
    } else {
        st = &s_NamedDocViewerStates[str_id ? str_id : "##default_doc_viewer"];
        size_idx = 3;
    }

    ImGui::DocumentViewerConfig cfg;
    cfg.baseDir = s_FWLMarkdownConfig.baseDir;
    std::string s_title, s_med_title, s_cmp_title, s_base_dir, s_pdf_path, s_footer_hint;

    if (cfg_idx > 0 && lua_istable(L, cfg_idx)) {
        auto readStr = [&](const char* k1, const char* k2, std::string& store, const char*& ptr) {
            lua_getfield(L, cfg_idx, k1);
            if (lua_isnil(L, -1) && k2) { lua_pop(L, 1); lua_getfield(L, cfg_idx, k2); }
            if (lua_type(L, -1) == LUA_TSTRING) {
                store = lua_tostring(L, -1);
                ptr = store.c_str();
            }
            lua_pop(L, 1);
        };
        auto readBool = [&](const char* k1, const char* k2, bool& dst) {
            lua_getfield(L, cfg_idx, k1);
            if (lua_isnil(L, -1) && k2) { lua_pop(L, 1); lua_getfield(L, cfg_idx, k2); }
            if (lua_isboolean(L, -1)) dst = (lua_toboolean(L, -1) != 0);
            lua_pop(L, 1);
        };
        auto readFloat = [&](const char* k1, const char* k2, float& dst) {
            lua_getfield(L, cfg_idx, k1);
            if (lua_isnil(L, -1) && k2) { lua_pop(L, 1); lua_getfield(L, cfg_idx, k2); }
            if (lua_type(L, -1) == LUA_TNUMBER) dst = static_cast<float>(lua_tonumber(L, -1));
            lua_pop(L, 1);
        };
        auto readInt = [&](const char* k1, const char* k2, int& dst) {
            lua_getfield(L, cfg_idx, k1);
            if (lua_isnil(L, -1) && k2) { lua_pop(L, 1); lua_getfield(L, cfg_idx, k2); }
            if (lua_type(L, -1) == LUA_TNUMBER) dst = static_cast<int>(lua_tointeger(L, -1));
            lua_pop(L, 1);
        };

        readStr("title", nullptr, s_title, cfg.title);
        readStr("medium_title", "mediumTitle", s_med_title, cfg.medium_title);
        readStr("compact_title", "compactTitle", s_cmp_title, cfg.compact_title);
        readStr("base_dir", "baseDir", s_base_dir, cfg.baseDir);
        readStr("external_pdf_path", "externalPdfPath", s_pdf_path, cfg.externalPdfPath);
        readStr("footer_hint", "footerHint", s_footer_hint, cfg.footerHint);

        readBool("show_title_icon", "showTitleIcon", cfg.showTitleIcon);
        readBool("enable_image_lightbox", "enableImageLightbox", cfg.enableImageLightbox);
        readBool("show_image_captions", "showImageCaptions", cfg.showImageCaptions);
        readBool("enable_table_tooltips", "enableTableTooltips", cfg.enableTableTooltips);
        readInt("max_toc_heading_level", "maxTocHeadingLevel", cfg.maxTocHeadingLevel);
        readFloat("toc_width", "tocWidth", cfg.tocWidth);
        readBool("scale_row2_with_zoom", "scaleRow2WithZoom", cfg.scaleRow2WithZoom);
        readBool("show_header_row1", "showHeaderRow1", cfg.showHeaderRow1);
        readBool("show_header_row2", "showHeaderRow2", cfg.showHeaderRow2);
        readBool("show_scroll_reset_button", "showScrollResetButton", cfg.showScrollResetButton);
        readBool("show_opacity_controls", "showOpacityControls", cfg.showOpacityControls);
        readBool("show_theme_picker", "showThemePicker", cfg.showThemePicker);
        readBool("show_footer", "showFooter", cfg.showFooter);
        readBool("show_footer_close_button", "showFooterCloseButton", cfg.showFooterCloseButton);
        readBool("show_close_button", "showCloseButton", cfg.showCloseButton);
    }

    float w = 0.0f, h = 0.0f;
    if (size_idx > 0 && top >= size_idx) {
        if (lua_istable(L, size_idx)) {
            lua_rawgeti(L, size_idx, 1); w = static_cast<float>(lua_tonumber(L, -1)); lua_pop(L, 1);
            lua_rawgeti(L, size_idx, 2); h = static_cast<float>(lua_tonumber(L, -1)); lua_pop(L, 1);
        } else if (lua_isnumber(L, size_idx)) {
            w = static_cast<float>(lua_tonumber(L, size_idx));
            if (top >= size_idx + 1 && lua_isnumber(L, size_idx + 1)) {
                h = static_cast<float>(lua_tonumber(L, size_idx + 1));
            }
        }
    }

    bool open = ImGui::DocumentViewer(str_id, md_text, md_len, *st, &cfg, ImVec2(w, h));
    if (has_state_tbl) {
        SyncDocViewerStateToLuaTable(L, 3, *st);
    }
    lua_pushboolean(L, open);
    return 1;
}

static int LuaParseMarkdownToc(lua_State* L) {
    size_t len = 0;
    const char* md = luaL_checklstring(L, 1, &len);
    int max_lvl = (lua_gettop(L) >= 2 && lua_isnumber(L, 2)) ? static_cast<int>(lua_tointeger(L, 2)) : 3;
    std::vector<ImGui::DocumentTocEntry> toc;
    std::string first_h1;
    ImGui::ParseMarkdownToc(md, len, toc, max_lvl, &first_h1);

    lua_newtable(L);
    for (size_t i = 0; i < toc.size(); ++i) {
        lua_newtable(L);
        lua_pushinteger(L, toc[i].level);                 lua_setfield(L, -2, "level");
        lua_pushstring(L, toc[i].title.c_str());          lua_setfield(L, -2, "title");
        lua_pushstring(L, toc[i].display_label.c_str());  lua_setfield(L, -2, "display_label");
        lua_pushstring(L, toc[i].slug.c_str());           lua_setfield(L, -2, "slug");
        lua_pushinteger(L, static_cast<lua_Integer>(toc[i].byte_offset + 1)); lua_setfield(L, -2, "byte_offset");
        lua_rawseti(L, -2, static_cast<int>(i + 1));
    }
    if (!first_h1.empty()) lua_pushstring(L, first_h1.c_str());
    else lua_pushnil(L);
    return 2;
}

static int LuaJumpToDocumentAnchor(lua_State* L) {
    const char* slug = luaL_checkstring(L, 2);
    ImGui::DocumentViewerState* st = ResolveLuaDocViewerState(L, 1, "##default_doc_viewer");
    bool ok = ImGui::JumpToDocumentAnchor(*st, slug);
    if (lua_istable(L, 1)) SyncDocViewerStateToLuaTable(L, 1, *st);
    lua_pushboolean(L, ok);
    return 1;
}

static int LuaJumpToDocumentHeading(lua_State* L) {
    int idx_1based = static_cast<int>(luaL_checkinteger(L, 2));
    ImGui::DocumentViewerState* st = ResolveLuaDocViewerState(L, 1, "##default_doc_viewer");
    bool ok = ImGui::JumpToDocumentHeading(*st, idx_1based - 1);
    if (lua_istable(L, 1)) SyncDocViewerStateToLuaTable(L, 1, *st);
    lua_pushboolean(L, ok);
    return 1;
}

static int LuaToggleDocumentScrollReset(lua_State* L) {
    ImGui::DocumentViewerState* st = ResolveLuaDocViewerState(L, 1, "##default_doc_viewer");
    bool changed = ImGui::ToggleDocumentScrollReset(*st);
    if (lua_istable(L, 1)) SyncDocViewerStateToLuaTable(L, 1, *st);
    lua_pushboolean(L, changed);
    return 1;
}

static int LuaApplyDocumentViewerTheme(lua_State* L) {
    int theme_idx = static_cast<int>(luaL_checkinteger(L, 1));
    bool trans = (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) ? (lua_toboolean(L, 2) != 0) : false;
    float opacity = (lua_gettop(L) >= 3 && lua_isnumber(L, 3)) ? static_cast<float>(lua_tonumber(L, 3)) : 0.80f;
    ImGui::ApplyDocumentViewerTheme(theme_idx, trans, opacity);
    return 0;
}

// --- Turnkey Prefab ImgDocWindow Bindings (imgui.DocWindow / float_wnd_create_doc) ---

static flwnd::FWLDocWindow* GetValidFWLDocWindow(lua_State* L, int idx = 1) {
    if (!lua_islightuserdata(L, idx)) return nullptr;
    flwnd::FloatingWindow* wnd = flwnd::GetValidFloatingWindow(lua_touserdata(L, idx));
    return dynamic_cast<flwnd::FWLDocWindow*>(wnd);
}

static int LuaDocWindowCreate(lua_State* L) {
    int top = lua_gettop(L);
    std::string script_dir = GetCallerLuaScriptDir(L);

    std::string title = "Documentation";
    std::string file_path;
    std::string raw_markdown;
    std::string base_dir;
    std::string fallback_markdown;
    std::string pdf_path;
    int width  = ImgDocWindow::kDefaultWidth;
    int height = ImgDocWindow::kDefaultHeight;
    int decoration = xplm_WindowDecorationSelfDecoratedResizable;
    bool cursors = (flywithlua::imgui_mouse_cursors != 0);
    bool init_visible = true;
    bool has_zoom = false; float init_zoom = 1.0f;
    bool has_theme = false; int init_theme = 0;
    bool has_opacity = false; float init_opacity = 0.80f;
    bool has_trans = false; bool init_trans = true;
    bool has_scale_on_zoom = false; bool scale_on_zoom = true;
    bool has_auto_reload = false; bool auto_reload = true;

    if (top >= 1 && lua_istable(L, 1)) {
        auto getStr = [&](const char* k1, const char* k2, std::string& dst) {
            lua_getfield(L, 1, k1);
            if (lua_isnil(L, -1) && k2) { lua_pop(L, 1); lua_getfield(L, 1, k2); }
            if (lua_type(L, -1) == LUA_TSTRING) dst = lua_tostring(L, -1);
            lua_pop(L, 1);
        };
        getStr("title", nullptr, title);
        getStr("file", "markdown_file", file_path);
        getStr("markdown", "text", raw_markdown);
        getStr("base_dir", "baseDir", base_dir);
        getStr("fallback", "fallback_markdown", fallback_markdown);
        getStr("pdf", "companion_pdf", pdf_path);

        lua_getfield(L, 1, "width");  if (lua_isnumber(L, -1)) width  = static_cast<int>(lua_tointeger(L, -1)); lua_pop(L, 1);
        lua_getfield(L, 1, "height"); if (lua_isnumber(L, -1)) height = static_cast<int>(lua_tointeger(L, -1)); lua_pop(L, 1);
        lua_getfield(L, 1, "decoration"); if (lua_isnumber(L, -1)) decoration = static_cast<int>(lua_tointeger(L, -1)); lua_pop(L, 1);
        lua_getfield(L, 1, "cursors"); if (lua_isboolean(L, -1)) cursors = (lua_toboolean(L, -1) != 0); lua_pop(L, 1);
        lua_getfield(L, 1, "visible"); if (lua_isboolean(L, -1)) init_visible = (lua_toboolean(L, -1) != 0); lua_pop(L, 1);

        lua_getfield(L, 1, "zoom");
        if (lua_isnumber(L, -1)) { has_zoom = true; init_zoom = static_cast<float>(lua_tonumber(L, -1)); }
        lua_pop(L, 1);

        lua_getfield(L, 1, "theme");
        if (lua_isnumber(L, -1)) { has_theme = true; init_theme = static_cast<int>(lua_tointeger(L, -1)); }
        lua_pop(L, 1);

        lua_getfield(L, 1, "opacity");
        if (lua_isnumber(L, -1)) { has_opacity = true; init_opacity = static_cast<float>(lua_tonumber(L, -1)); }
        lua_pop(L, 1);

        lua_getfield(L, 1, "transparent_hud");
        if (lua_isboolean(L, -1)) { has_trans = true; init_trans = (lua_toboolean(L, -1) != 0); }
        lua_pop(L, 1);

        lua_getfield(L, 1, "scale_window_on_zoom");
        if (lua_isboolean(L, -1)) { has_scale_on_zoom = true; scale_on_zoom = (lua_toboolean(L, -1) != 0); }
        lua_pop(L, 1);

        lua_getfield(L, 1, "auto_reload");
        if (lua_isboolean(L, -1)) { has_auto_reload = true; auto_reload = (lua_toboolean(L, -1) != 0); }
        lua_pop(L, 1);
    } else {
        if (top >= 1 && lua_type(L, 1) == LUA_TSTRING) title = lua_tostring(L, 1);
        if (top >= 2 && lua_type(L, 2) == LUA_TSTRING) file_path = lua_tostring(L, 2);
        if (top >= 3 && lua_type(L, 3) == LUA_TSTRING) pdf_path = lua_tostring(L, 3);
        if (top >= 4 && lua_isnumber(L, 4)) width = static_cast<int>(lua_tointeger(L, 4));
        if (top >= 5 && lua_isnumber(L, 5)) height = static_cast<int>(lua_tointeger(L, 5));
        if (top >= 6 && lua_isnumber(L, 6)) decoration = static_cast<int>(lua_tointeger(L, 6));
    }

    try {
        auto docWnd = std::make_shared<flwnd::FWLDocWindow>(title, width, height, decoration, cursors);
        if (!fallback_markdown.empty()) {
            docWnd->SetFallbackMarkdown(fallback_markdown);
        }
        if (!file_path.empty()) {
            docWnd->LoadMarkdownFileWithScriptDir(file_path, script_dir);
        } else if (!raw_markdown.empty()) {
            docWnd->SetMarkdownText(raw_markdown, base_dir.empty() ? script_dir : base_dir);
        }
        if (!pdf_path.empty()) {
            docWnd->SetCompanionPdfWithScriptDir(pdf_path, script_dir);
        }
        if (has_scale_on_zoom) docWnd->SetScaleWindowOnZoom(scale_on_zoom);
        if (has_auto_reload)   docWnd->SetAutoReloadOnDiskChange(auto_reload);
        if (has_theme)         docWnd->SetTheme(init_theme);
        if (has_trans)         docWnd->SetTransparentHud(init_trans);
        if (has_opacity)       docWnd->SetWindowOpacity(init_opacity);
        if (has_zoom)          docWnd->SetZoom(init_zoom, false);
        if (!init_visible)     docWnd->SetVisible(false);

        flwnd::RegisterManagedFloatingWindow(docWnd);
        lua_pushlightuserdata(L, docWnd.get());
        return 1;
    } catch (const std::exception& e) {
        flywithlua::logMsg(logToDevCon, std::string("FlyWithLua Error: Couldn't create DocWindow: ") + e.what());
        lua_pushnil(L);
        return 1;
    }
}

static int LuaDocWindowLoadFile(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    const char* path = luaL_checkstring(L, 2);
    if (!docWnd || !path) {
        lua_pushboolean(L, false);
        return 1;
    }
    bool ok = docWnd->LoadMarkdownFileWithScriptDir(path, GetCallerLuaScriptDir(L));
    lua_pushboolean(L, ok);
    return 1;
}

static int LuaDocWindowSetMarkdown(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    const char* md = luaL_checkstring(L, 2);
    if (!docWnd) return 0;
    std::string base_dir = (lua_gettop(L) >= 3 && lua_type(L, 3) == LUA_TSTRING)
                               ? std::string(lua_tostring(L, 3))
                               : GetCallerLuaScriptDir(L);
    docWnd->SetMarkdownText(md ? md : "", base_dir);
    return 0;
}

static int LuaDocWindowSetFallbackMarkdown(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    const char* md = luaL_checkstring(L, 2);
    if (docWnd && md) docWnd->SetFallbackMarkdown(md);
    return 0;
}

static int LuaDocWindowSetCompanionPdf(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    if (!docWnd) return 0;
    const char* pdf = (lua_gettop(L) >= 2 && lua_type(L, 2) == LUA_TSTRING) ? lua_tostring(L, 2) : "";
    docWnd->SetCompanionPdfWithScriptDir(pdf ? pdf : "", GetCallerLuaScriptDir(L));
    return 0;
}

static int LuaDocWindowReload(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    lua_pushboolean(L, docWnd ? docWnd->ReloadDocument() : false);
    return 1;
}

static int LuaDocWindowShow(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    if (docWnd) {
        docWnd->SetVisible(true);
        docWnd->bringToFront();
    }
    return 0;
}

static int LuaDocWindowHide(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    if (docWnd) docWnd->SetVisible(false);
    return 0;
}

static int LuaDocWindowToggle(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    if (!docWnd) {
        lua_pushboolean(L, false);
        return 1;
    }
    bool next_vis = !docWnd->GetVisible();
    docWnd->SetVisible(next_vis);
    if (next_vis) docWnd->bringToFront();
    lua_pushboolean(L, next_vis);
    return 1;
}

static int LuaDocWindowIsVisible(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    lua_pushboolean(L, docWnd ? docWnd->GetVisible() : false);
    return 1;
}

static int LuaDocWindowSetZoom(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    float z = static_cast<float>(luaL_checknumber(L, 2));
    bool resize = (lua_gettop(L) >= 3 && !lua_isnil(L, 3)) ? (lua_toboolean(L, 3) != 0) : false;
    if (docWnd) docWnd->SetZoom(z, resize);
    return 0;
}

static int LuaDocWindowGetZoom(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    lua_pushnumber(L, docWnd ? docWnd->GetZoom() : 1.0);
    return 1;
}

static int LuaDocWindowSetTheme(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    int idx = static_cast<int>(luaL_checkinteger(L, 2));
    if (docWnd) docWnd->SetTheme(idx);
    return 0;
}

static int LuaDocWindowGetTheme(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    lua_pushinteger(L, docWnd ? docWnd->GetTheme() : 0);
    return 1;
}

static int LuaDocWindowSetOpacity(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    float op = static_cast<float>(luaL_checknumber(L, 2));
    if (docWnd) docWnd->SetWindowOpacity(op);
    return 0;
}

static int LuaDocWindowGetOpacity(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    lua_pushnumber(L, docWnd ? docWnd->GetWindowOpacity() : 0.80);
    return 1;
}

static int LuaDocWindowSetTransparentHud(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    bool en = (lua_gettop(L) < 2) ? true : (lua_toboolean(L, 2) != 0);
    if (docWnd) docWnd->SetTransparentHud(en);
    return 0;
}

static int LuaDocWindowGetTransparentHud(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    lua_pushboolean(L, docWnd ? docWnd->GetTransparentHud() : false);
    return 1;
}

static int LuaDocWindowSetScaleWindowOnZoom(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    bool en = (lua_gettop(L) < 2) ? true : (lua_toboolean(L, 2) != 0);
    if (docWnd) docWnd->SetScaleWindowOnZoom(en);
    return 0;
}

static int LuaDocWindowGetScaleWindowOnZoom(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    lua_pushboolean(L, docWnd ? docWnd->GetScaleWindowOnZoom() : true);
    return 1;
}

static int LuaDocWindowSetAutoReload(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    bool en = (lua_gettop(L) < 2) ? true : (lua_toboolean(L, 2) != 0);
    if (docWnd) docWnd->SetAutoReloadOnDiskChange(en);
    return 0;
}

static int LuaDocWindowGetAutoReload(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    lua_pushboolean(L, docWnd ? docWnd->GetAutoReloadOnDiskChange() : true);
    return 1;
}

static int LuaDocWindowJumpToAnchor(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    const char* slug = luaL_checkstring(L, 2);
    lua_pushboolean(L, (docWnd && slug) ? docWnd->JumpToAnchor(slug) : false);
    return 1;
}

static int LuaDocWindowJumpToSection(lua_State* L) {
    auto* docWnd = GetValidFWLDocWindow(L, 1);
    int idx_1based = static_cast<int>(luaL_checkinteger(L, 2));
    lua_pushboolean(L, docWnd ? docWnd->JumpToSection(idx_1based - 1) : false);
    return 1;
}

static int LuaOpenBuiltInLuiGeeGuide(lua_State*) {
    flwnd::OpenBuiltInLuiGeeGuide();
    return 0;
}

static int LuaToggleBuiltInLuiGeeGuide(lua_State*) {
    flwnd::ToggleBuiltInLuiGeeGuide();
    return 0;
}
#endif // IMGUI_DOC_VIEWER_H

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
    s_CurrentLuaState = L;
    s_LuaCustomTagRefs.clear();
    s_GlobalMarkdownBaseDir.clear();
    s_HasGlobalMarkdownBaseDir = false;
    s_ActiveWindowAutoBaseDir = nullptr;
    s_ActiveWindowExplicitBaseDir = nullptr;
    s_FWLMarkdownConfig.baseDir = nullptr;
    s_FWLMarkdownConfig.showImageCaptions = false;
    s_FWLMarkdownConfig.enableImageLightbox = false;
    s_FWLMarkdownConfig.enableTableTooltips = true;
    s_FWLMarkdownConfig.showCodeCopyButton = true;
#if defined(IMGUI_ZOOMABLE_VIEWPORT_H)
    s_TableViewportStates.clear();
    s_NamedViewportStates.clear();
    s_ActiveLuaZoomViewports.clear();
#endif
#if defined(IMGUI_DOC_VIEWER_H)
    s_TableDocViewerStates.clear();
    s_NamedDocViewerStates.clear();
#endif

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
    lua_pushcfunction(L, LuaMarkdownDelayedTooltipMD);  lua_setfield(L, -2, "DelayedTooltipMD");
    lua_pushcfunction(L, LuaMarkdownDelayedTooltipMD);  lua_setfield(L, -2, "DelayedTooltip");
    lua_pushcfunction(L, LuaMarkdownTextLink);          lua_setfield(L, -2, "TextLink");
    lua_pushcfunction(L, LuaMarkdownTextLink);          lua_setfield(L, -2, "TextMDLink");
    lua_pushcfunction(L, LuaMarkdownSetBaseDir);             lua_setfield(L, -2, "SetBaseDir");
    lua_pushcfunction(L, LuaMarkdownGetBaseDir);             lua_setfield(L, -2, "GetBaseDir");
    lua_pushcfunction(L, LuaMarkdownSetWindowBaseDir);       lua_setfield(L, -2, "SetWindowBaseDir");
    lua_pushcfunction(L, LuaMarkdownGetWindowBaseDir);       lua_setfield(L, -2, "GetWindowBaseDir");
    lua_pushcfunction(L, LuaMarkdownSetShowImageCaptions);   lua_setfield(L, -2, "SetShowImageCaptions");
    lua_pushcfunction(L, LuaMarkdownGetShowImageCaptions);   lua_setfield(L, -2, "GetShowImageCaptions");
    lua_pushcfunction(L, LuaMarkdownSetEnableImageLightbox); lua_setfield(L, -2, "SetEnableImageLightbox");
    lua_pushcfunction(L, LuaMarkdownGetEnableImageLightbox); lua_setfield(L, -2, "GetEnableImageLightbox");
    lua_pushcfunction(L, LuaMarkdownSetEnableTableTooltips); lua_setfield(L, -2, "SetEnableTableTooltips");
    lua_pushcfunction(L, LuaMarkdownGetEnableTableTooltips); lua_setfield(L, -2, "GetEnableTableTooltips");
    lua_pushcfunction(L, LuaMarkdownSetShowCodeCopyButton);  lua_setfield(L, -2, "SetShowCodeCopyButton");
    lua_pushcfunction(L, LuaMarkdownGetShowCodeCopyButton);  lua_setfield(L, -2, "GetShowCodeCopyButton");
    lua_pushcfunction(L, LuaMarkdownConfigure);              lua_setfield(L, -2, "Configure");
    lua_pushcfunction(L, LuaMarkdownClearImageCache);        lua_setfield(L, -2, "ClearImageCache");

    lua_pushcfunction(L, LuaMarkdownText);              lua_setfield(L, -2, "text");
    lua_pushcfunction(L, LuaMarkdownTextWrapped);       lua_setfield(L, -2, "text_wrapped");
    lua_pushcfunction(L, LuaMarkdownTextUnformatted);   lua_setfield(L, -2, "text_unformatted");
    lua_pushcfunction(L, LuaMarkdownButtonMD);          lua_setfield(L, -2, "button");
    lua_pushcfunction(L, LuaMarkdownButtonMD);          lua_setfield(L, -2, "button_md");
    lua_pushcfunction(L, LuaMarkdownCalcTextSize);      lua_setfield(L, -2, "calc_text_size");
    lua_pushcfunction(L, LuaMarkdownCalcTextSize);      lua_setfield(L, -2, "calc_size");
    lua_pushcfunction(L, LuaMarkdownDelayedTooltipMD);  lua_setfield(L, -2, "delayed_tooltip_md");
    lua_pushcfunction(L, LuaMarkdownDelayedTooltipMD);  lua_setfield(L, -2, "delayed_tooltip");
    lua_pushcfunction(L, LuaMarkdownTextLink);          lua_setfield(L, -2, "text_link");
    lua_pushcfunction(L, LuaMarkdownTextLink);          lua_setfield(L, -2, "text_md_link");
    lua_pushcfunction(L, LuaMarkdownSetBaseDir);             lua_setfield(L, -2, "set_base_dir");
    lua_pushcfunction(L, LuaMarkdownGetBaseDir);             lua_setfield(L, -2, "get_base_dir");
    lua_pushcfunction(L, LuaMarkdownSetWindowBaseDir);       lua_setfield(L, -2, "set_window_base_dir");
    lua_pushcfunction(L, LuaMarkdownGetWindowBaseDir);       lua_setfield(L, -2, "get_window_base_dir");
    lua_pushcfunction(L, LuaMarkdownSetShowImageCaptions);   lua_setfield(L, -2, "set_show_image_captions");
    lua_pushcfunction(L, LuaMarkdownGetShowImageCaptions);   lua_setfield(L, -2, "get_show_image_captions");
    lua_pushcfunction(L, LuaMarkdownSetEnableImageLightbox); lua_setfield(L, -2, "set_enable_image_lightbox");
    lua_pushcfunction(L, LuaMarkdownGetEnableImageLightbox); lua_setfield(L, -2, "get_enable_image_lightbox");
    lua_pushcfunction(L, LuaMarkdownSetEnableTableTooltips); lua_setfield(L, -2, "set_enable_table_tooltips");
    lua_pushcfunction(L, LuaMarkdownGetEnableTableTooltips); lua_setfield(L, -2, "get_enable_table_tooltips");
    lua_pushcfunction(L, LuaMarkdownSetShowCodeCopyButton);  lua_setfield(L, -2, "set_show_code_copy_button");
    lua_pushcfunction(L, LuaMarkdownGetShowCodeCopyButton);  lua_setfield(L, -2, "get_show_code_copy_button");
    lua_pushcfunction(L, LuaMarkdownConfigure);              lua_setfield(L, -2, "configure");
    lua_pushcfunction(L, LuaMarkdownClearImageCache);        lua_setfield(L, -2, "clear_image_cache");

    lua_pushcfunction(L, LuaRegisterMarkdownStyleTag);  lua_setfield(L, -2, "RegisterStyleTag");
    lua_pushcfunction(L, LuaRegisterMarkdownStyleTag);  lua_setfield(L, -2, "register_style_tag");
    lua_pushcfunction(L, LuaRegisterMarkdownStyleTag);  lua_setfield(L, -2, "RegisterMarkdownStyleTag");
    lua_pushcfunction(L, LuaRegisterMarkdownStyleTag);  lua_setfield(L, -2, "register_markdown_style_tag");

    lua_pushcfunction(L, LuaRegisterMarkdownTagAlias);  lua_setfield(L, -2, "RegisterTagAlias");
    lua_pushcfunction(L, LuaRegisterMarkdownTagAlias);  lua_setfield(L, -2, "register_tag_alias");
    lua_pushcfunction(L, LuaRegisterMarkdownTagAlias);  lua_setfield(L, -2, "RegisterMarkdownTagAlias");
    lua_pushcfunction(L, LuaRegisterMarkdownTagAlias);  lua_setfield(L, -2, "register_markdown_tag_alias");

    lua_pushcfunction(L, LuaRegisterMarkdownTag);       lua_setfield(L, -2, "RegisterTag");
    lua_pushcfunction(L, LuaRegisterMarkdownTag);       lua_setfield(L, -2, "register_tag");
    lua_pushcfunction(L, LuaRegisterMarkdownTag);       lua_setfield(L, -2, "RegisterMarkdownTag");
    lua_pushcfunction(L, LuaRegisterMarkdownTag);       lua_setfield(L, -2, "register_markdown_tag");

    lua_pushcfunction(L, LuaSelectPulldown);            lua_setfield(L, -2, "SelectPulldown");
    lua_pushcfunction(L, LuaSelectPulldown);            lua_setfield(L, -2, "select_pulldown");
    lua_pushcfunction(L, LuaSelectPulldown);            lua_setfield(L, -2, "SelectPulldownMD");
    lua_pushcfunction(L, LuaSelectPulldown);            lua_setfield(L, -2, "select_pulldown_md");

    // Assign to imgui.Markdown and imgui.MD
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "Markdown");
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "MD");
    lua_pop(L, 1); // Pop Markdown table

    // 3. Build imgui.TimedTooltip table
    lua_newtable(L);
    lua_pushcfunction(L, LuaTimedTooltipText);                  lua_setfield(L, -2, "Text");
    lua_pushcfunction(L, LuaTimedTooltipTextMD);                lua_setfield(L, -2, "TextMD");
    lua_pushcfunction(L, LuaTimedTooltipTextUnformatted);       lua_setfield(L, -2, "TextUnformatted");
    lua_pushcfunction(L, LuaTimedTooltipTextMDUnformatted);     lua_setfield(L, -2, "TextMDUnformatted");
    lua_pushcfunction(L, LuaTimedTooltipTextMDUnformatted);     lua_setfield(L, -2, "TextUnformattedMD");
    lua_pushcfunction(L, LuaTimedTooltipTextLink);              lua_setfield(L, -2, "TextLink");
    lua_pushcfunction(L, LuaTimedTooltipTextMDLink);            lua_setfield(L, -2, "TextMDLink");
    lua_pushcfunction(L, LuaTimedTooltipTextProxy);             lua_setfield(L, -2, "TextProxy");
    lua_pushcfunction(L, LuaTimedTooltipTextMDProxy);           lua_setfield(L, -2, "TextMDProxy");
    lua_pushcfunction(L, LuaTimedTooltipToggleOnClick);         lua_setfield(L, -2, "ToggleOnClick");
    lua_pushcfunction(L, LuaTimedTooltipSetDelay);              lua_setfield(L, -2, "SetDelay");
    lua_pushcfunction(L, LuaTimedTooltipSetDelay);              lua_setfield(L, -2, "SetHoverDelay");
    lua_pushcfunction(L, LuaTimedTooltipSetMaxDisplay);         lua_setfield(L, -2, "SetMaxDisplay");
    lua_pushcfunction(L, LuaTimedTooltipSetMaxWidth);           lua_setfield(L, -2, "SetMaxWidth");
    lua_pushcfunction(L, LuaTimedTooltipSetMinWidth);           lua_setfield(L, -2, "SetMinWidth");
    lua_pushcfunction(L, LuaTimedTooltipSetDefaultWrapWidth);   lua_setfield(L, -2, "SetDefaultWrapWidth");
    lua_pushcfunction(L, LuaTimedTooltipSetDefaultWrapWidth);   lua_setfield(L, -2, "SetWrapWidth");
    lua_pushcfunction(L, LuaTimedTooltipSetGracePeriod);        lua_setfield(L, -2, "SetGracePeriod");
    lua_pushcfunction(L, LuaTimedTooltipSetWiggleLatch);        lua_setfield(L, -2, "SetWiggleLatch");
    lua_pushcfunction(L, LuaTimedTooltipSetColors);             lua_setfield(L, -2, "SetColors");
    lua_pushcfunction(L, LuaTimedTooltipSetFontScalePolicy);    lua_setfield(L, -2, "SetFontScalePolicy");
    lua_pushcfunction(L, LuaTimedTooltipSetFontScalePolicy);    lua_setfield(L, -2, "SetDefaultFontScalePolicy");
    lua_pushcfunction(L, LuaTimedTooltipGetFontScalePolicy);    lua_setfield(L, -2, "GetFontScalePolicy");
    lua_pushcfunction(L, LuaTimedTooltipGetFontScalePolicy);    lua_setfield(L, -2, "GetDefaultFontScalePolicy");
    lua_pushcfunction(L, LuaTimedTooltipSetFontScaleMultiplier);lua_setfield(L, -2, "SetFontScaleMultiplier");
    lua_pushcfunction(L, LuaTimedTooltipSetFontScale);          lua_setfield(L, -2, "SetFontScale");
    lua_pushcfunction(L, LuaTimedTooltipClear);                 lua_setfield(L, -2, "ClearActiveTooltip");
    lua_pushcfunction(L, LuaTimedTooltipClear);                 lua_setfield(L, -2, "Clear");

    lua_pushcfunction(L, LuaTimedTooltipText);                  lua_setfield(L, -2, "text");
    lua_pushcfunction(L, LuaTimedTooltipTextMD);                lua_setfield(L, -2, "text_md");
    lua_pushcfunction(L, LuaTimedTooltipTextUnformatted);       lua_setfield(L, -2, "text_unformatted");
    lua_pushcfunction(L, LuaTimedTooltipTextMDUnformatted);     lua_setfield(L, -2, "text_md_unformatted");
    lua_pushcfunction(L, LuaTimedTooltipTextMDUnformatted);     lua_setfield(L, -2, "text_unformatted_md");
    lua_pushcfunction(L, LuaTimedTooltipTextLink);              lua_setfield(L, -2, "text_link");
    lua_pushcfunction(L, LuaTimedTooltipTextMDLink);            lua_setfield(L, -2, "text_md_link");
    lua_pushcfunction(L, LuaTimedTooltipTextProxy);             lua_setfield(L, -2, "text_proxy");
    lua_pushcfunction(L, LuaTimedTooltipTextMDProxy);           lua_setfield(L, -2, "text_md_proxy");
    lua_pushcfunction(L, LuaTimedTooltipToggleOnClick);         lua_setfield(L, -2, "toggle_on_click");
    lua_pushcfunction(L, LuaTimedTooltipSetDelay);              lua_setfield(L, -2, "set_delay");
    lua_pushcfunction(L, LuaTimedTooltipSetDelay);              lua_setfield(L, -2, "set_hover_delay");
    lua_pushcfunction(L, LuaTimedTooltipSetMaxDisplay);         lua_setfield(L, -2, "set_max_display");
    lua_pushcfunction(L, LuaTimedTooltipSetMaxWidth);           lua_setfield(L, -2, "set_max_width");
    lua_pushcfunction(L, LuaTimedTooltipSetMinWidth);           lua_setfield(L, -2, "set_min_width");
    lua_pushcfunction(L, LuaTimedTooltipSetDefaultWrapWidth);   lua_setfield(L, -2, "set_default_wrap_width");
    lua_pushcfunction(L, LuaTimedTooltipSetDefaultWrapWidth);   lua_setfield(L, -2, "set_wrap_width");
    lua_pushcfunction(L, LuaTimedTooltipSetGracePeriod);        lua_setfield(L, -2, "set_grace_period");
    lua_pushcfunction(L, LuaTimedTooltipSetWiggleLatch);        lua_setfield(L, -2, "set_wiggle_latch");
    lua_pushcfunction(L, LuaTimedTooltipSetColors);             lua_setfield(L, -2, "set_colors");
    lua_pushcfunction(L, LuaTimedTooltipSetFontScalePolicy);    lua_setfield(L, -2, "set_font_scale_policy");
    lua_pushcfunction(L, LuaTimedTooltipSetFontScalePolicy);    lua_setfield(L, -2, "set_default_font_scale_policy");
    lua_pushcfunction(L, LuaTimedTooltipGetFontScalePolicy);    lua_setfield(L, -2, "get_font_scale_policy");
    lua_pushcfunction(L, LuaTimedTooltipGetFontScalePolicy);    lua_setfield(L, -2, "get_default_font_scale_policy");
    lua_pushcfunction(L, LuaTimedTooltipSetFontScaleMultiplier);lua_setfield(L, -2, "set_font_scale_multiplier");
    lua_pushcfunction(L, LuaTimedTooltipSetFontScale);          lua_setfield(L, -2, "set_font_scale");
    lua_pushcfunction(L, LuaTimedTooltipClear);                 lua_setfield(L, -2, "clear_active_tooltip");
    lua_pushcfunction(L, LuaTimedTooltipClear);                 lua_setfield(L, -2, "clear");

    // FontScalePolicy sub-table & constants inside TimedTooltip
    lua_newtable(L);
    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_Damped);        lua_setfield(L, -2, "Damped");
    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_FixedChrome);   lua_setfield(L, -2, "FixedChrome");
    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_LinearInherit); lua_setfield(L, -2, "LinearInherit");
    lua_setfield(L, -2, "FontScalePolicy");

    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_Damped);        lua_setfield(L, -2, "TooltipFontScale_Damped");
    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_FixedChrome);   lua_setfield(L, -2, "TooltipFontScale_FixedChrome");
    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_LinearInherit); lua_setfield(L, -2, "TooltipFontScale_LinearInherit");

    // Sub-namespaces: imgui.TimedTooltip.MD and imgui.TimedTooltip.Markdown
    lua_newtable(L);
    lua_pushcfunction(L, LuaTimedTooltipTextMD);                lua_setfield(L, -2, "Text");
    lua_pushcfunction(L, LuaTimedTooltipTextMDUnformatted);     lua_setfield(L, -2, "TextUnformatted");
    lua_pushcfunction(L, LuaTimedTooltipTextMDLink);            lua_setfield(L, -2, "TextLink");
    lua_pushcfunction(L, LuaTimedTooltipTextMDProxy);           lua_setfield(L, -2, "TextProxy");
    lua_pushcfunction(L, LuaTimedTooltipTextMD);                lua_setfield(L, -2, "text");
    lua_pushcfunction(L, LuaTimedTooltipTextMDUnformatted);     lua_setfield(L, -2, "text_unformatted");
    lua_pushcfunction(L, LuaTimedTooltipTextMDLink);            lua_setfield(L, -2, "text_link");
    lua_pushcfunction(L, LuaTimedTooltipTextMDProxy);           lua_setfield(L, -2, "text_proxy");

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

    // Expose TooltipFontScalePolicy on root imgui table too
    lua_newtable(L);
    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_Damped);        lua_setfield(L, -2, "Damped");
    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_FixedChrome);   lua_setfield(L, -2, "FixedChrome");
    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_LinearInherit); lua_setfield(L, -2, "LinearInherit");
    lua_setfield(L, -2, "TooltipFontScalePolicy");

    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_Damped);        lua_setfield(L, -2, "TooltipFontScale_Damped");
    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_FixedChrome);   lua_setfield(L, -2, "TooltipFontScale_FixedChrome");
    lua_pushinteger(L, ImGui::TimedTooltip::TooltipFontScale_LinearInherit); lua_setfield(L, -2, "TooltipFontScale_LinearInherit");

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
    lua_pushcfunction(L, LuaMarkdownDelayedTooltipMD);  lua_setfield(L, -2, "DelayedTooltipMD");
    lua_pushcfunction(L, LuaMarkdownDelayedTooltipMD);  lua_setfield(L, -2, "delayed_tooltip_md");
    lua_pushcfunction(L, LuaMarkdownTextLink);          lua_setfield(L, -2, "TextMDLink");
    lua_pushcfunction(L, LuaMarkdownTextLink);          lua_setfield(L, -2, "text_md_link");
    lua_pushcfunction(L, LuaMarkdownTextLink);          lua_setfield(L, -2, "TextMarkdownLink");
    lua_pushcfunction(L, LuaMarkdownTextLink);          lua_setfield(L, -2, "text_markdown_link");
    lua_pushcfunction(L, LuaMarkdownCalcTextSize);      lua_setfield(L, -2, "CalcMarkdownSize");
    lua_pushcfunction(L, LuaMarkdownCalcTextSize);      lua_setfield(L, -2, "calc_markdown_size");

    lua_pushcfunction(L, LuaRegisterMarkdownStyleTag);  lua_setfield(L, -2, "RegisterMarkdownStyleTag");
    lua_pushcfunction(L, LuaRegisterMarkdownStyleTag);  lua_setfield(L, -2, "register_markdown_style_tag");
    lua_pushcfunction(L, LuaRegisterMarkdownStyleTag);  lua_setfield(L, -2, "RegisterStyleTag");
    lua_pushcfunction(L, LuaRegisterMarkdownStyleTag);  lua_setfield(L, -2, "register_style_tag");

    lua_pushcfunction(L, LuaRegisterMarkdownTagAlias);  lua_setfield(L, -2, "RegisterMarkdownTagAlias");
    lua_pushcfunction(L, LuaRegisterMarkdownTagAlias);  lua_setfield(L, -2, "register_markdown_tag_alias");
    lua_pushcfunction(L, LuaRegisterMarkdownTagAlias);  lua_setfield(L, -2, "RegisterTagAlias");
    lua_pushcfunction(L, LuaRegisterMarkdownTagAlias);  lua_setfield(L, -2, "register_tag_alias");

    lua_pushcfunction(L, LuaRegisterMarkdownTag);       lua_setfield(L, -2, "RegisterMarkdownTag");
    lua_pushcfunction(L, LuaRegisterMarkdownTag);       lua_setfield(L, -2, "register_markdown_tag");
    lua_pushcfunction(L, LuaRegisterMarkdownTag);       lua_setfield(L, -2, "RegisterTag");
    lua_pushcfunction(L, LuaRegisterMarkdownTag);       lua_setfield(L, -2, "register_tag");

    // 6. ImGee Widgets directly on imgui table
    lua_pushcfunction(L, LuaToggleButton);                   lua_setfield(L, -2, "ToggleButton");
    lua_pushcfunction(L, LuaToggleButton);                   lua_setfield(L, -2, "toggle_button");
    lua_pushcfunction(L, LuaToggleButtons);                  lua_setfield(L, -2, "ToggleButtons");
    lua_pushcfunction(L, LuaToggleButtons);                  lua_setfield(L, -2, "toggle_buttons");
    lua_pushcfunction(L, LuaSetDefaultToggleButtonScale);    lua_setfield(L, -2, "SetDefaultToggleButtonScale");
    lua_pushcfunction(L, LuaSetDefaultToggleButtonScale);    lua_setfield(L, -2, "set_default_toggle_button_scale");
    lua_pushcfunction(L, LuaSetDefaultToggleButtonColors);   lua_setfield(L, -2, "SetDefaultToggleButtonColors");
    lua_pushcfunction(L, LuaSetDefaultToggleButtonColors);   lua_setfield(L, -2, "set_default_toggle_button_colors");

    lua_pushcfunction(L, LuaAdjustItemOnMouseWheel);         lua_setfield(L, -2, "AdjustItemOnMouseWheel");
    lua_pushcfunction(L, LuaAdjustItemOnMouseWheel);         lua_setfield(L, -2, "adjust_item_on_mouse_wheel");
    lua_pushcfunction(L, LuaCycleItemOnMouseWheel);          lua_setfield(L, -2, "CycleItemOnMouseWheel");
    lua_pushcfunction(L, LuaCycleItemOnMouseWheel);          lua_setfield(L, -2, "cycle_item_on_mouse_wheel");
    lua_pushcfunction(L, LuaToggleItemOnMouseWheel);         lua_setfield(L, -2, "ToggleItemOnMouseWheel");
    lua_pushcfunction(L, LuaToggleItemOnMouseWheel);         lua_setfield(L, -2, "toggle_item_on_mouse_wheel");
    lua_pushcfunction(L, LuaMouseWheelCheckbox);             lua_setfield(L, -2, "MouseWheelCheckbox");
    lua_pushcfunction(L, LuaMouseWheelCheckbox);             lua_setfield(L, -2, "mouse_wheel_checkbox");

    lua_pushcfunction(L, LuaMouseWheelSlider);               lua_setfield(L, -2, "MouseWheelSlider");
    lua_pushcfunction(L, LuaMouseWheelSlider);               lua_setfield(L, -2, "mouse_wheel_slider");
    lua_pushcfunction(L, LuaMouseWheelSlider);               lua_setfield(L, -2, "MouseWheelSliderFloat");
    lua_pushcfunction(L, LuaMouseWheelSlider);               lua_setfield(L, -2, "mouse_wheel_slider_float");
    lua_pushcfunction(L, LuaMouseWheelSliderInt);            lua_setfield(L, -2, "MouseWheelSliderInt");
    lua_pushcfunction(L, LuaMouseWheelSliderInt);            lua_setfield(L, -2, "mouse_wheel_slider_int");

    lua_pushcfunction(L, LuaMouseWheelDrag);                 lua_setfield(L, -2, "MouseWheelDrag");
    lua_pushcfunction(L, LuaMouseWheelDrag);                 lua_setfield(L, -2, "mouse_wheel_drag");
    lua_pushcfunction(L, LuaMouseWheelDrag);                 lua_setfield(L, -2, "MouseWheelDragFloat");
    lua_pushcfunction(L, LuaMouseWheelDrag);                 lua_setfield(L, -2, "mouse_wheel_drag_float");
    lua_pushcfunction(L, LuaMouseWheelDragInt);              lua_setfield(L, -2, "MouseWheelDragInt");
    lua_pushcfunction(L, LuaMouseWheelDragInt);              lua_setfield(L, -2, "mouse_wheel_drag_int");

    lua_pushcfunction(L, LuaPercentSlider);                  lua_setfield(L, -2, "PercentSlider");
    lua_pushcfunction(L, LuaPercentSlider);                  lua_setfield(L, -2, "percent_slider");
    lua_pushcfunction(L, LuaPercentSlider);                  lua_setfield(L, -2, "SliderPercent");
    lua_pushcfunction(L, LuaPercentSlider);                  lua_setfield(L, -2, "slider_percent");

    lua_pushcfunction(L, LuaSliderWithReset);                lua_setfield(L, -2, "SliderWithReset");
    lua_pushcfunction(L, LuaSliderWithReset);                lua_setfield(L, -2, "slider_with_reset");
    lua_pushcfunction(L, LuaSliderWithReset);                lua_setfield(L, -2, "ResettableSlider");
    lua_pushcfunction(L, LuaSliderWithReset);                lua_setfield(L, -2, "resettable_slider");
    lua_pushcfunction(L, LuaSliderWithReset);                lua_setfield(L, -2, "MouseWheelSliderWithReset");
    lua_pushcfunction(L, LuaSliderWithReset);                lua_setfield(L, -2, "mouse_wheel_slider_with_reset");

    lua_pushcfunction(L, LuaSliderIntWithReset);             lua_setfield(L, -2, "SliderIntWithReset");
    lua_pushcfunction(L, LuaSliderIntWithReset);             lua_setfield(L, -2, "slider_int_with_reset");
    lua_pushcfunction(L, LuaSliderIntWithReset);             lua_setfield(L, -2, "ResettableSliderInt");
    lua_pushcfunction(L, LuaSliderIntWithReset);             lua_setfield(L, -2, "resettable_slider_int");

    lua_pushcfunction(L, LuaPercentSliderWithReset);         lua_setfield(L, -2, "PercentSliderWithReset");
    lua_pushcfunction(L, LuaPercentSliderWithReset);         lua_setfield(L, -2, "percent_slider_with_reset");
    lua_pushcfunction(L, LuaPercentSliderWithReset);         lua_setfield(L, -2, "SliderPercentWithReset");
    lua_pushcfunction(L, LuaPercentSliderWithReset);         lua_setfield(L, -2, "slider_percent_with_reset");
    lua_pushcfunction(L, LuaPercentSliderWithReset);         lua_setfield(L, -2, "ResettablePercentSlider");
    lua_pushcfunction(L, LuaPercentSliderWithReset);         lua_setfield(L, -2, "resettable_percent_slider");
    lua_pushcfunction(L, LuaPercentSliderWithReset);         lua_setfield(L, -2, "ResettableSliderPercent");
    lua_pushcfunction(L, LuaPercentSliderWithReset);         lua_setfield(L, -2, "resettable_slider_percent");

    lua_pushcfunction(L, LuaDragWithReset);                  lua_setfield(L, -2, "DragWithReset");
    lua_pushcfunction(L, LuaDragWithReset);                  lua_setfield(L, -2, "drag_with_reset");
    lua_pushcfunction(L, LuaDragWithReset);                  lua_setfield(L, -2, "ResettableDrag");
    lua_pushcfunction(L, LuaDragWithReset);                  lua_setfield(L, -2, "resettable_drag");
    lua_pushcfunction(L, LuaDragWithReset);                  lua_setfield(L, -2, "MouseWheelDragWithReset");
    lua_pushcfunction(L, LuaDragWithReset);                  lua_setfield(L, -2, "mouse_wheel_drag_with_reset");

    lua_pushcfunction(L, LuaDragIntWithReset);               lua_setfield(L, -2, "DragIntWithReset");
    lua_pushcfunction(L, LuaDragIntWithReset);               lua_setfield(L, -2, "drag_int_with_reset");
    lua_pushcfunction(L, LuaDragIntWithReset);               lua_setfield(L, -2, "ResettableDragInt");
    lua_pushcfunction(L, LuaDragIntWithReset);               lua_setfield(L, -2, "resettable_drag_int");

    lua_pushcfunction(L, LuaMouseWheelCombo);                lua_setfield(L, -2, "MouseWheelCombo");
    lua_pushcfunction(L, LuaMouseWheelCombo);                lua_setfield(L, -2, "mouse_wheel_combo");
    lua_pushcfunction(L, LuaComboWithReset);                 lua_setfield(L, -2, "ComboWithReset");
    lua_pushcfunction(L, LuaComboWithReset);                 lua_setfield(L, -2, "combo_with_reset");
    lua_pushcfunction(L, LuaComboWithReset);                 lua_setfield(L, -2, "ResettableCombo");
    lua_pushcfunction(L, LuaComboWithReset);                 lua_setfield(L, -2, "resettable_combo");
    lua_pushcfunction(L, LuaComboWithReset);                 lua_setfield(L, -2, "MouseWheelComboWithReset");
    lua_pushcfunction(L, LuaComboWithReset);                 lua_setfield(L, -2, "mouse_wheel_combo_with_reset");

    lua_pushcfunction(L, LuaResetButton);                    lua_setfield(L, -2, "ResetButton");
    lua_pushcfunction(L, LuaResetButton);                    lua_setfield(L, -2, "reset_button");

    lua_pushcfunction(L, LuaSetMouseWheelAutoDefeat);        lua_setfield(L, -2, "SetMouseWheelAutoDefeat");
    lua_pushcfunction(L, LuaSetMouseWheelAutoDefeat);        lua_setfield(L, -2, "set_mouse_wheel_auto_defeat");
    lua_pushcfunction(L, LuaGetMouseWheelAutoDefeat);        lua_setfield(L, -2, "GetMouseWheelAutoDefeat");
    lua_pushcfunction(L, LuaGetMouseWheelAutoDefeat);        lua_setfield(L, -2, "get_mouse_wheel_auto_defeat");

    lua_pushcfunction(L, LuaSetDefaultComboArrowStyle);      lua_setfield(L, -2, "SetDefaultComboArrowStyle");
    lua_pushcfunction(L, LuaSetDefaultComboArrowStyle);      lua_setfield(L, -2, "set_default_combo_arrow_style");
    lua_pushcfunction(L, LuaGetDefaultComboArrowStyle);      lua_setfield(L, -2, "GetDefaultComboArrowStyle");
    lua_pushcfunction(L, LuaGetDefaultComboArrowStyle);      lua_setfield(L, -2, "get_default_combo_arrow_style");

    lua_pushcfunction(L, LuaSetResetButtonUndoEnabled);      lua_setfield(L, -2, "SetResetButtonUndoEnabled");
    lua_pushcfunction(L, LuaSetResetButtonUndoEnabled);      lua_setfield(L, -2, "set_reset_button_undo_enabled");
    lua_pushcfunction(L, LuaGetResetButtonUndoEnabled);      lua_setfield(L, -2, "GetResetButtonUndoEnabled");
    lua_pushcfunction(L, LuaGetResetButtonUndoEnabled);      lua_setfield(L, -2, "get_reset_button_undo_enabled");

    lua_pushcfunction(L, LuaSetDefaultResetButtonLabel);     lua_setfield(L, -2, "SetDefaultResetButtonLabel");
    lua_pushcfunction(L, LuaSetDefaultResetButtonLabel);     lua_setfield(L, -2, "set_default_reset_button_label");
    lua_pushcfunction(L, LuaGetDefaultResetButtonLabel);     lua_setfield(L, -2, "GetDefaultResetButtonLabel");
    lua_pushcfunction(L, LuaGetDefaultResetButtonLabel);     lua_setfield(L, -2, "get_default_reset_button_label");

    lua_pushcfunction(L, LuaSetDefaultRestoreButtonLabel);   lua_setfield(L, -2, "SetDefaultRestoreButtonLabel");
    lua_pushcfunction(L, LuaSetDefaultRestoreButtonLabel);   lua_setfield(L, -2, "set_default_restore_button_label");
    lua_pushcfunction(L, LuaSetDefaultRestoreButtonLabel);   lua_setfield(L, -2, "SetDefaultRedoButtonLabel");
    lua_pushcfunction(L, LuaSetDefaultRestoreButtonLabel);   lua_setfield(L, -2, "set_default_redo_button_label");
    lua_pushcfunction(L, LuaGetDefaultRestoreButtonLabel);   lua_setfield(L, -2, "GetDefaultRestoreButtonLabel");
    lua_pushcfunction(L, LuaGetDefaultRestoreButtonLabel);   lua_setfield(L, -2, "get_default_restore_button_label");
    lua_pushcfunction(L, LuaGetDefaultRestoreButtonLabel);   lua_setfield(L, -2, "GetDefaultRedoButtonLabel");
    lua_pushcfunction(L, LuaGetDefaultRestoreButtonLabel);   lua_setfield(L, -2, "get_default_redo_button_label");

    // ComboArrowStyle sub-table and flat constants
    lua_newtable(L);
    lua_pushinteger(L, ImGuiComboArrowStyle_DownWhenClosed);  lua_setfield(L, -2, "DownWhenClosed");
    lua_pushinteger(L, ImGuiComboArrowStyle_Standard);        lua_setfield(L, -2, "Standard");
    lua_pushinteger(L, ImGuiComboArrowStyle_RightWhenClosed); lua_setfield(L, -2, "RightWhenClosed");
    lua_pushinteger(L, ImGuiComboArrowStyle_UpWhenClosed);    lua_setfield(L, -2, "UpWhenClosed");
    lua_pushinteger(L, ImGuiComboArrowStyle_StaticDown);      lua_setfield(L, -2, "StaticDown");
    lua_setfield(L, -2, "ComboArrowStyle");

    lua_pushinteger(L, ImGuiComboArrowStyle_DownWhenClosed);  lua_setfield(L, -2, "ComboArrowStyle_DownWhenClosed");
    lua_pushinteger(L, ImGuiComboArrowStyle_Standard);        lua_setfield(L, -2, "ComboArrowStyle_Standard");
    lua_pushinteger(L, ImGuiComboArrowStyle_RightWhenClosed); lua_setfield(L, -2, "ComboArrowStyle_RightWhenClosed");
    lua_pushinteger(L, ImGuiComboArrowStyle_UpWhenClosed);    lua_setfield(L, -2, "ComboArrowStyle_UpWhenClosed");
    lua_pushinteger(L, ImGuiComboArrowStyle_StaticDown);      lua_setfield(L, -2, "ComboArrowStyle_StaticDown");

    lua_pushinteger(L, ImGuiComboArrowStyle_DownWhenClosed);  lua_setfield(L, -2, "ImGuiComboArrowStyle_DownWhenClosed");
    lua_pushinteger(L, ImGuiComboArrowStyle_Standard);        lua_setfield(L, -2, "ImGuiComboArrowStyle_Standard");
    lua_pushinteger(L, ImGuiComboArrowStyle_RightWhenClosed); lua_setfield(L, -2, "ImGuiComboArrowStyle_RightWhenClosed");
    lua_pushinteger(L, ImGuiComboArrowStyle_UpWhenClosed);    lua_setfield(L, -2, "ImGuiComboArrowStyle_UpWhenClosed");
    lua_pushinteger(L, ImGuiComboArrowStyle_StaticDown);      lua_setfield(L, -2, "ImGuiComboArrowStyle_StaticDown");

    // Avionics Rotary Knobs & Dimmer Dials (imgui_knobs.h)
    lua_pushcfunction(L, LuaKnob);                      lua_setfield(L, -2, "Knob");
    lua_pushcfunction(L, LuaKnob);                      lua_setfield(L, -2, "knob");
    lua_pushcfunction(L, LuaKnob);                      lua_setfield(L, -2, "RotaryDial");
    lua_pushcfunction(L, LuaKnob);                      lua_setfield(L, -2, "rotary_dial");

    lua_pushcfunction(L, LuaKnobWithReset);             lua_setfield(L, -2, "KnobWithReset");
    lua_pushcfunction(L, LuaKnobWithReset);             lua_setfield(L, -2, "knob_with_reset");
    lua_pushcfunction(L, LuaKnobWithReset);             lua_setfield(L, -2, "ResettableKnob");
    lua_pushcfunction(L, LuaKnobWithReset);             lua_setfield(L, -2, "resettable_knob");

    lua_pushcfunction(L, LuaDimmerKnob);                lua_setfield(L, -2, "DimmerKnob");
    lua_pushcfunction(L, LuaDimmerKnob);                lua_setfield(L, -2, "dimmer_knob");

    lua_pushcfunction(L, LuaKnobPercent);               lua_setfield(L, -2, "KnobPercent");
    lua_pushcfunction(L, LuaKnobPercent);               lua_setfield(L, -2, "knob_percent");
    lua_pushcfunction(L, LuaKnobPercent);               lua_setfield(L, -2, "PercentKnob");
    lua_pushcfunction(L, LuaKnobPercent);               lua_setfield(L, -2, "percent_knob");

    lua_pushcfunction(L, LuaKnobPercentWithReset);      lua_setfield(L, -2, "PercentKnobWithReset");
    lua_pushcfunction(L, LuaKnobPercentWithReset);      lua_setfield(L, -2, "percent_knob_with_reset");
    lua_pushcfunction(L, LuaKnobPercentWithReset);      lua_setfield(L, -2, "KnobPercentWithReset");
    lua_pushcfunction(L, LuaKnobPercentWithReset);      lua_setfield(L, -2, "knob_percent_with_reset");
    lua_pushcfunction(L, LuaKnobPercentWithReset);      lua_setfield(L, -2, "ResettablePercentKnob");
    lua_pushcfunction(L, LuaKnobPercentWithReset);      lua_setfield(L, -2, "resettable_percent_knob");
    lua_pushcfunction(L, LuaKnobPercentWithReset);      lua_setfield(L, -2, "ResettableKnobPercent");
    lua_pushcfunction(L, LuaKnobPercentWithReset);      lua_setfield(L, -2, "resettable_knob_percent");

    lua_pushcfunction(L, LuaKnobEx);                    lua_setfield(L, -2, "KnobEx");
    lua_pushcfunction(L, LuaKnobEx);                    lua_setfield(L, -2, "knob_ex");

    // KnobFlags sub-table and flat constants
    lua_newtable(L);
    lua_pushinteger(L, ImGuiKnobFlags_None);            lua_setfield(L, -2, "None");
    lua_pushinteger(L, ImGuiKnobFlags_NoInput);         lua_setfield(L, -2, "NoInput");
    lua_pushinteger(L, ImGuiKnobFlags_NoDetente);       lua_setfield(L, -2, "NoDetente");
    lua_pushinteger(L, ImGuiKnobFlags_ShowTicks);       lua_setfield(L, -2, "ShowTicks");
    lua_pushinteger(L, ImGuiKnobFlags_ValueBelow);      lua_setfield(L, -2, "ValueBelow");
    lua_pushinteger(L, ImGuiKnobFlags_NoValueText);     lua_setfield(L, -2, "NoValueText");
    lua_pushinteger(L, ImGuiKnobFlags_CursorHand);      lua_setfield(L, -2, "CursorHand");
    lua_setfield(L, -2, "KnobFlags");

    lua_pushinteger(L, ImGuiKnobFlags_None);            lua_setfield(L, -2, "KnobFlags_None");
    lua_pushinteger(L, ImGuiKnobFlags_NoInput);         lua_setfield(L, -2, "KnobFlags_NoInput");
    lua_pushinteger(L, ImGuiKnobFlags_NoDetente);       lua_setfield(L, -2, "KnobFlags_NoDetente");
    lua_pushinteger(L, ImGuiKnobFlags_ShowTicks);       lua_setfield(L, -2, "KnobFlags_ShowTicks");
    lua_pushinteger(L, ImGuiKnobFlags_ValueBelow);      lua_setfield(L, -2, "KnobFlags_ValueBelow");
    lua_pushinteger(L, ImGuiKnobFlags_NoValueText);     lua_setfield(L, -2, "KnobFlags_NoValueText");
    lua_pushinteger(L, ImGuiKnobFlags_CursorHand);      lua_setfield(L, -2, "KnobFlags_CursorHand");

    lua_pushinteger(L, ImGuiKnobFlags_None);            lua_setfield(L, -2, "ImGuiKnobFlags_None");
    lua_pushinteger(L, ImGuiKnobFlags_NoInput);         lua_setfield(L, -2, "ImGuiKnobFlags_NoInput");
    lua_pushinteger(L, ImGuiKnobFlags_NoDetente);       lua_setfield(L, -2, "ImGuiKnobFlags_NoDetente");
    lua_pushinteger(L, ImGuiKnobFlags_ShowTicks);       lua_setfield(L, -2, "ImGuiKnobFlags_ShowTicks");
    lua_pushinteger(L, ImGuiKnobFlags_ValueBelow);      lua_setfield(L, -2, "ImGuiKnobFlags_ValueBelow");
    lua_pushinteger(L, ImGuiKnobFlags_NoValueText);     lua_setfield(L, -2, "ImGuiKnobFlags_NoValueText");
    lua_pushinteger(L, ImGuiKnobFlags_CursorHand);      lua_setfield(L, -2, "ImGuiKnobFlags_CursorHand");

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

    lua_pushcfunction(L, LuaIcon);                      lua_setfield(L, -2, "Icon");
    lua_pushcfunction(L, LuaIcon);                      lua_setfield(L, -2, "icon");
    lua_pushcfunction(L, LuaIsVectorIcon);              lua_setfield(L, -2, "IsVectorIcon");
    lua_pushcfunction(L, LuaIsVectorIcon);              lua_setfield(L, -2, "is_vector_icon");
    lua_pushcfunction(L, LuaNormalizeIconName);         lua_setfield(L, -2, "NormalizeIconName");
    lua_pushcfunction(L, LuaNormalizeIconName);         lua_setfield(L, -2, "normalize_icon_name");

    lua_pushcfunction(L, LuaIconButton);                lua_setfield(L, -2, "IconButton");
    lua_pushcfunction(L, LuaIconButton);                lua_setfield(L, -2, "icon_button");
    lua_pushcfunction(L, LuaButtonIcon);                lua_setfield(L, -2, "ButtonIcon");
    lua_pushcfunction(L, LuaButtonIcon);                lua_setfield(L, -2, "button_icon");

    lua_pushcfunction(L, LuaButtonLink);                lua_setfield(L, -2, "ButtonLink");
    lua_pushcfunction(L, LuaButtonLink);                lua_setfield(L, -2, "button_link");
    lua_pushcfunction(L, LuaSmallButtonLink);           lua_setfield(L, -2, "SmallButtonLink");
    lua_pushcfunction(L, LuaSmallButtonLink);           lua_setfield(L, -2, "small_button_link");
    lua_pushcfunction(L, LuaTextLink);                  lua_setfield(L, -2, "TextLink");
    lua_pushcfunction(L, LuaTextLink);                  lua_setfield(L, -2, "text_link");
    lua_pushcfunction(L, LuaExternalTextLink);          lua_setfield(L, -2, "ExternalTextLink");
    lua_pushcfunction(L, LuaExternalTextLink);          lua_setfield(L, -2, "external_text_link");
    lua_pushcfunction(L, LuaCheckboxLink);              lua_setfield(L, -2, "CheckboxLink");
    lua_pushcfunction(L, LuaCheckboxLink);              lua_setfield(L, -2, "checkbox_link");
    lua_pushcfunction(L, LuaRadioButtonLink);           lua_setfield(L, -2, "RadioButtonLink");
    lua_pushcfunction(L, LuaRadioButtonLink);           lua_setfield(L, -2, "radio_button_link");
    lua_pushcfunction(L, LuaSelectableLink);            lua_setfield(L, -2, "SelectableLink");
    lua_pushcfunction(L, LuaSelectableLink);            lua_setfield(L, -2, "selectable_link");
    lua_pushcfunction(L, LuaMenuItemLink);              lua_setfield(L, -2, "MenuItemLink");
    lua_pushcfunction(L, LuaMenuItemLink);              lua_setfield(L, -2, "menu_item_link");
    lua_pushcfunction(L, LuaAttachItemCursor);          lua_setfield(L, -2, "AttachItemCursor");
    lua_pushcfunction(L, LuaAttachItemCursor);          lua_setfield(L, -2, "attach_item_cursor");
    lua_pushcfunction(L, LuaAttachItemCursor);          lua_setfield(L, -2, "ShowLinkCursorOnHover");
    lua_pushcfunction(L, LuaAttachItemCursor);          lua_setfield(L, -2, "show_link_cursor_on_hover");
    lua_pushcfunction(L, LuaInputTextGetSelection);     lua_setfield(L, -2, "GetInputTextSelection");
    lua_pushcfunction(L, LuaInputTextGetSelection);     lua_setfield(L, -2, "get_input_text_selection");
    lua_pushcfunction(L, LuaInputTextSetSelection);     lua_setfield(L, -2, "SetInputTextSelection");
    lua_pushcfunction(L, LuaInputTextSetSelection);     lua_setfield(L, -2, "set_input_text_selection");
    lua_pushcfunction(L, LuaGetMouseWheel);             lua_setfield(L, -2, "GetMouseWheel");
    lua_pushcfunction(L, LuaGetMouseWheel);             lua_setfield(L, -2, "get_mouse_wheel");
    lua_pushcfunction(L, LuaGetMouseWheel);             lua_setfield(L, -2, "GetMouseWheelV");
    lua_pushcfunction(L, LuaGetMouseWheel);             lua_setfield(L, -2, "get_mouse_wheel_v");
    lua_pushcfunction(L, LuaGetMouseWheelH);            lua_setfield(L, -2, "GetMouseWheelH");
    lua_pushcfunction(L, LuaGetMouseWheelH);            lua_setfield(L, -2, "get_mouse_wheel_h");
    lua_pushcfunction(L, LuaGetMouseWheelEither);       lua_setfield(L, -2, "GetMouseWheelEither");
    lua_pushcfunction(L, LuaGetMouseWheelEither);       lua_setfield(L, -2, "get_mouse_wheel_either");
    lua_pushcfunction(L, LuaGetMouseWheelEither);       lua_setfield(L, -2, "GetMouseWheelAny");
    lua_pushcfunction(L, LuaGetMouseWheelEither);       lua_setfield(L, -2, "get_mouse_wheel_any");

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

    // BadgeVariant sub-table and flat constants
    lua_newtable(L);
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Default);      lua_setfield(L, -2, "Default");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Primary);      lua_setfield(L, -2, "Primary");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Success);      lua_setfield(L, -2, "Success");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Warning);      lua_setfield(L, -2, "Warning");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Danger);       lua_setfield(L, -2, "Danger");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Info);         lua_setfield(L, -2, "Info");
    lua_setfield(L, -2, "BadgeVariant");

    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Default);      lua_setfield(L, -2, "BadgeVariant_Default");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Primary);      lua_setfield(L, -2, "BadgeVariant_Primary");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Success);      lua_setfield(L, -2, "BadgeVariant_Success");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Warning);      lua_setfield(L, -2, "BadgeVariant_Warning");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Danger);       lua_setfield(L, -2, "BadgeVariant_Danger");
    lua_pushinteger(L, ImGui::ImGuiBadgeVariant_Info);         lua_setfield(L, -2, "BadgeVariant_Info");

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
    lua_pushcfunction(L, LuaSelectPulldown);             lua_setfield(L, -2, "SelectPulldownMD");
    lua_pushcfunction(L, LuaSelectPulldown);             lua_setfield(L, -2, "select_pulldown_md");
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

    lua_pushcfunction(L, LuaBoldItalicText);             lua_setfield(L, -2, "BoldItalicText");
    lua_pushcfunction(L, LuaBoldItalicText);             lua_setfield(L, -2, "bold_italic_text");
    lua_pushcfunction(L, LuaBoldItalicTextUnformatted);  lua_setfield(L, -2, "BoldItalicTextUnformatted");
    lua_pushcfunction(L, LuaBoldItalicTextUnformatted);  lua_setfield(L, -2, "bold_italic_text_unformatted");

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
    lua_pushinteger(L, (int)ImGui::ImFontRole::BoldItalic);      lua_setfield(L, -2, "BoldItalic");
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
    lua_pushinteger(L, (int)ImGui::ImFontRole::BoldItalic);      lua_setfield(L, -2, "ImFontRole_BoldItalic");
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

#if defined(IMGUI_ZOOMABLE_VIEWPORT_H)
    // 16. Two-Layer Zoomable Viewport
    lua_pushcfunction(L, LuaBeginZoomedViewport);        lua_setfield(L, -2, "BeginZoomedViewport");
    lua_pushcfunction(L, LuaBeginZoomedViewport);        lua_setfield(L, -2, "begin_zoomed_viewport");
    lua_pushcfunction(L, LuaEndZoomedViewport);          lua_setfield(L, -2, "EndZoomedViewport");
    lua_pushcfunction(L, LuaEndZoomedViewport);          lua_setfield(L, -2, "end_zoomed_viewport");
    lua_pushcfunction(L, LuaZoomViewportControls);       lua_setfield(L, -2, "ZoomViewportControls");
    lua_pushcfunction(L, LuaZoomViewportControls);       lua_setfield(L, -2, "zoom_viewport_controls");
    lua_pushcfunction(L, LuaSetViewportZoom);            lua_setfield(L, -2, "SetViewportZoom");
    lua_pushcfunction(L, LuaSetViewportZoom);            lua_setfield(L, -2, "set_viewport_zoom");
    lua_pushcfunction(L, LuaStepViewportZoom);           lua_setfield(L, -2, "StepViewportZoom");
    lua_pushcfunction(L, LuaStepViewportZoom);           lua_setfield(L, -2, "step_viewport_zoom");
    lua_pushcfunction(L, LuaResetViewportZoom);          lua_setfield(L, -2, "ResetViewportZoom");
    lua_pushcfunction(L, LuaResetViewportZoom);          lua_setfield(L, -2, "reset_viewport_zoom");
#endif

#if defined(IMGUI_DOC_VIEWER_H)
    // 17. Standalone DocumentViewer & Turnkey ImgDocWindow
    lua_pushcfunction(L, LuaDocumentViewer);             lua_setfield(L, -2, "DocumentViewer");
    lua_pushcfunction(L, LuaDocumentViewer);             lua_setfield(L, -2, "document_viewer");
    lua_pushcfunction(L, LuaParseMarkdownToc);           lua_setfield(L, -2, "ParseMarkdownToc");
    lua_pushcfunction(L, LuaParseMarkdownToc);           lua_setfield(L, -2, "parse_markdown_toc");
    lua_pushcfunction(L, LuaJumpToDocumentAnchor);       lua_setfield(L, -2, "JumpToDocumentAnchor");
    lua_pushcfunction(L, LuaJumpToDocumentAnchor);       lua_setfield(L, -2, "jump_to_document_anchor");
    lua_pushcfunction(L, LuaJumpToDocumentHeading);      lua_setfield(L, -2, "JumpToDocumentHeading");
    lua_pushcfunction(L, LuaJumpToDocumentHeading);      lua_setfield(L, -2, "jump_to_document_heading");
    lua_pushcfunction(L, LuaToggleDocumentScrollReset);  lua_setfield(L, -2, "ToggleDocumentScrollReset");
    lua_pushcfunction(L, LuaToggleDocumentScrollReset);  lua_setfield(L, -2, "toggle_document_scroll_reset");
    lua_pushcfunction(L, LuaApplyDocumentViewerTheme);   lua_setfield(L, -2, "ApplyDocumentViewerTheme");
    lua_pushcfunction(L, LuaApplyDocumentViewerTheme);   lua_setfield(L, -2, "apply_document_viewer_theme");
    lua_pushcfunction(L, LuaDocWindowCreate);            lua_setfield(L, -2, "CreateDocWindow");
    lua_pushcfunction(L, LuaDocWindowCreate);            lua_setfield(L, -2, "create_doc_window");

    // DocumentViewerTheme enum sub-table
    lua_newtable(L);
    lua_pushinteger(L, ImGui::DocumentViewerTheme_Charcoal); lua_setfield(L, -2, "Charcoal");
    lua_pushinteger(L, ImGui::DocumentViewerTheme_Rusty);    lua_setfield(L, -2, "Rusty");
    lua_pushinteger(L, ImGui::DocumentViewerTheme_Dark);     lua_setfield(L, -2, "Dark");
    lua_pushinteger(L, ImGui::DocumentViewerTheme_Classic);  lua_setfield(L, -2, "Classic");
    lua_pushinteger(L, ImGui::DocumentViewerTheme_Light);    lua_setfield(L, -2, "Light");
    lua_setfield(L, -2, "DocumentViewerTheme");

    // imgui.DocViewer sub-namespace
    lua_newtable(L);
    lua_pushcfunction(L, LuaDocumentViewer);             lua_setfield(L, -2, "Render");
    lua_pushcfunction(L, LuaDocumentViewer);             lua_setfield(L, -2, "render");
    lua_pushcfunction(L, LuaDocumentViewer);             lua_setfield(L, -2, "Show");
    lua_pushcfunction(L, LuaDocumentViewer);             lua_setfield(L, -2, "show");
    lua_pushcfunction(L, LuaParseMarkdownToc);           lua_setfield(L, -2, "ParseToc");
    lua_pushcfunction(L, LuaParseMarkdownToc);           lua_setfield(L, -2, "parse_toc");
    lua_pushcfunction(L, LuaJumpToDocumentAnchor);       lua_setfield(L, -2, "JumpToAnchor");
    lua_pushcfunction(L, LuaJumpToDocumentAnchor);       lua_setfield(L, -2, "jump_to_anchor");
    lua_pushcfunction(L, LuaJumpToDocumentHeading);      lua_setfield(L, -2, "JumpToHeading");
    lua_pushcfunction(L, LuaJumpToDocumentHeading);      lua_setfield(L, -2, "jump_to_heading");
    lua_pushcfunction(L, LuaToggleDocumentScrollReset);  lua_setfield(L, -2, "ToggleScrollReset");
    lua_pushcfunction(L, LuaToggleDocumentScrollReset);  lua_setfield(L, -2, "toggle_scroll_reset");
    lua_pushcfunction(L, LuaApplyDocumentViewerTheme);   lua_setfield(L, -2, "ApplyTheme");
    lua_pushcfunction(L, LuaApplyDocumentViewerTheme);   lua_setfield(L, -2, "apply_theme");
    lua_setfield(L, -2, "DocViewer");

    // imgui.DocWindow sub-namespace (Prefab ImgDocWindow wrapper)
    lua_newtable(L);
    lua_pushcfunction(L, LuaDocWindowCreate);                 lua_setfield(L, -2, "Create");
    lua_pushcfunction(L, LuaDocWindowCreate);                 lua_setfield(L, -2, "create");
    lua_pushcfunction(L, LuaDocWindowLoadFile);               lua_setfield(L, -2, "LoadFile");
    lua_pushcfunction(L, LuaDocWindowLoadFile);               lua_setfield(L, -2, "load_file");
    lua_pushcfunction(L, LuaDocWindowSetMarkdown);            lua_setfield(L, -2, "SetMarkdown");
    lua_pushcfunction(L, LuaDocWindowSetMarkdown);            lua_setfield(L, -2, "set_markdown");
    lua_pushcfunction(L, LuaDocWindowSetFallbackMarkdown);    lua_setfield(L, -2, "SetFallbackMarkdown");
    lua_pushcfunction(L, LuaDocWindowSetFallbackMarkdown);    lua_setfield(L, -2, "set_fallback_markdown");
    lua_pushcfunction(L, LuaDocWindowSetCompanionPdf);        lua_setfield(L, -2, "SetCompanionPdf");
    lua_pushcfunction(L, LuaDocWindowSetCompanionPdf);        lua_setfield(L, -2, "set_companion_pdf");
    lua_pushcfunction(L, LuaDocWindowReload);                 lua_setfield(L, -2, "Reload");
    lua_pushcfunction(L, LuaDocWindowReload);                 lua_setfield(L, -2, "reload");
    lua_pushcfunction(L, LuaDocWindowShow);                   lua_setfield(L, -2, "Show");
    lua_pushcfunction(L, LuaDocWindowShow);                   lua_setfield(L, -2, "show");
    lua_pushcfunction(L, LuaDocWindowHide);                   lua_setfield(L, -2, "Hide");
    lua_pushcfunction(L, LuaDocWindowHide);                   lua_setfield(L, -2, "hide");
    lua_pushcfunction(L, LuaDocWindowToggle);                 lua_setfield(L, -2, "Toggle");
    lua_pushcfunction(L, LuaDocWindowToggle);                 lua_setfield(L, -2, "toggle");
    lua_pushcfunction(L, LuaDocWindowIsVisible);              lua_setfield(L, -2, "IsVisible");
    lua_pushcfunction(L, LuaDocWindowIsVisible);              lua_setfield(L, -2, "is_visible");
    lua_pushcfunction(L, LuaDocWindowSetZoom);                lua_setfield(L, -2, "SetZoom");
    lua_pushcfunction(L, LuaDocWindowSetZoom);                lua_setfield(L, -2, "set_zoom");
    lua_pushcfunction(L, LuaDocWindowGetZoom);                lua_setfield(L, -2, "GetZoom");
    lua_pushcfunction(L, LuaDocWindowGetZoom);                lua_setfield(L, -2, "get_zoom");
    lua_pushcfunction(L, LuaDocWindowSetTheme);               lua_setfield(L, -2, "SetTheme");
    lua_pushcfunction(L, LuaDocWindowSetTheme);               lua_setfield(L, -2, "set_theme");
    lua_pushcfunction(L, LuaDocWindowGetTheme);               lua_setfield(L, -2, "GetTheme");
    lua_pushcfunction(L, LuaDocWindowGetTheme);               lua_setfield(L, -2, "get_theme");
    lua_pushcfunction(L, LuaDocWindowSetOpacity);             lua_setfield(L, -2, "SetOpacity");
    lua_pushcfunction(L, LuaDocWindowSetOpacity);             lua_setfield(L, -2, "set_opacity");
    lua_pushcfunction(L, LuaDocWindowGetOpacity);             lua_setfield(L, -2, "GetOpacity");
    lua_pushcfunction(L, LuaDocWindowGetOpacity);             lua_setfield(L, -2, "get_opacity");
    lua_pushcfunction(L, LuaDocWindowSetTransparentHud);      lua_setfield(L, -2, "SetTransparentHud");
    lua_pushcfunction(L, LuaDocWindowSetTransparentHud);      lua_setfield(L, -2, "set_transparent_hud");
    lua_pushcfunction(L, LuaDocWindowGetTransparentHud);      lua_setfield(L, -2, "GetTransparentHud");
    lua_pushcfunction(L, LuaDocWindowGetTransparentHud);      lua_setfield(L, -2, "get_transparent_hud");
    lua_pushcfunction(L, LuaDocWindowSetScaleWindowOnZoom);   lua_setfield(L, -2, "SetScaleWindowOnZoom");
    lua_pushcfunction(L, LuaDocWindowSetScaleWindowOnZoom);   lua_setfield(L, -2, "set_scale_window_on_zoom");
    lua_pushcfunction(L, LuaDocWindowGetScaleWindowOnZoom);   lua_setfield(L, -2, "GetScaleWindowOnZoom");
    lua_pushcfunction(L, LuaDocWindowGetScaleWindowOnZoom);   lua_setfield(L, -2, "get_scale_window_on_zoom");
    lua_pushcfunction(L, LuaDocWindowSetAutoReload);          lua_setfield(L, -2, "SetAutoReload");
    lua_pushcfunction(L, LuaDocWindowSetAutoReload);          lua_setfield(L, -2, "set_auto_reload");
    lua_pushcfunction(L, LuaDocWindowGetAutoReload);          lua_setfield(L, -2, "GetAutoReload");
    lua_pushcfunction(L, LuaDocWindowGetAutoReload);          lua_setfield(L, -2, "get_auto_reload");
    lua_pushcfunction(L, LuaDocWindowJumpToAnchor);           lua_setfield(L, -2, "JumpToAnchor");
    lua_pushcfunction(L, LuaDocWindowJumpToAnchor);           lua_setfield(L, -2, "jump_to_anchor");
    lua_pushcfunction(L, LuaDocWindowJumpToSection);          lua_setfield(L, -2, "JumpToSection");
    lua_pushcfunction(L, LuaDocWindowJumpToSection);          lua_setfield(L, -2, "jump_to_section");
    lua_pushcfunction(L, LuaOpenBuiltInLuiGeeGuide);          lua_setfield(L, -2, "OpenLuiGeeGuide");
    lua_pushcfunction(L, LuaOpenBuiltInLuiGeeGuide);          lua_setfield(L, -2, "open_luigee_guide");
    lua_pushcfunction(L, LuaToggleBuiltInLuiGeeGuide);        lua_setfield(L, -2, "ToggleLuiGeeGuide");
    lua_pushcfunction(L, LuaToggleBuiltInLuiGeeGuide);        lua_setfield(L, -2, "toggle_luigee_guide");
    lua_setfield(L, -2, "DocWindow");

    lua_pushcfunction(L, LuaOpenBuiltInLuiGeeGuide);          lua_setfield(L, -2, "OpenLuiGeeGuide");
    lua_pushcfunction(L, LuaOpenBuiltInLuiGeeGuide);          lua_setfield(L, -2, "open_luigee_guide");
    lua_pushcfunction(L, LuaToggleBuiltInLuiGeeGuide);        lua_setfield(L, -2, "ToggleLuiGeeGuide");
    lua_pushcfunction(L, LuaToggleBuiltInLuiGeeGuide);        lua_setfield(L, -2, "toggle_luigee_guide");

    lua_register(L, "float_wnd_create_doc", LuaDocWindowCreate);
    lua_getfield(L, -1, "DocWindow");
    lua_setglobal(L, "DocWindow");
    lua_getfield(L, -1, "DocViewer");
    lua_setglobal(L, "DocViewer");
#endif

    // 7. Global convenience aliases (matching imgee-global.h in C++)
    lua_getfield(L, -1, "TimedTooltip");
    lua_setglobal(L, "TimedTooltip");

    lua_getfield(L, -1, "Markdown");
    lua_setglobal(L, "Markdown");

    lua_getfield(L, -1, "MD");
    lua_setglobal(L, "MD");

    lua_pop(L, 1); // Pop imgui table
}

void ClearImGeeMarkdownTextures() {
#if defined(IMGUI_MARKDOWN_EXT_H) && !defined(IMGEE_DISABLE_MARKDOWN) && !defined(CONTRIB_DISABLE_MARKDOWN)
    ImGui::MarkdownImageService::ClearTextureCache();
#endif
}

} // namespace flywithlua
