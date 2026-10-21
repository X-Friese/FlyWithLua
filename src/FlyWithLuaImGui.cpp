/*
 *   ImGui lifecycle (shared ImgFontAtlas) for FlyWithLua.
 *
 *   See FlyWithLuaImGui.h and ImgWindow/docs/Basic-Usage-Guide.md.
 */
#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "FlyWithLua.h"
#include "FlyWithLuaImGui.h"
#include "FloatingWindows/FWLIntegration.h"
#include "ImgWindow.h"

ImFont* customFont1 = nullptr;
ImFont* customFont2 = nullptr;
ImFont* customFont3 = nullptr;
ImFont* customFont4 = nullptr;
ImFont* customFont5 = nullptr;
ImFont* customFont6 = nullptr;
ImFont* customFont7 = nullptr;
ImFont* customFont8 = nullptr;
ImFont* customFont9 = nullptr;

namespace {

const char* const kCustomFontDir = "./Resources/plugins/FlyWithLua/Custom_Fonts/";

static std::unordered_map<std::string, ImFont*> sFontRegistry;
static std::array<ImFont*, 10> sLegacyFonts = {};
static ImFont* sDefaultFont = nullptr;
static std::vector<std::string> sRegisteredNames;

static std::string ToLower(std::string_view str) {
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

static void RegisterFontInternal(const std::string& name, ImFont* font, bool addToNameList = true) {
    if (font == nullptr || name.empty()) {
        return;
    }
    std::string lowerKey = ToLower(name);
    bool isNew = (sFontRegistry.find(lowerKey) == sFontRegistry.end());
    sFontRegistry[lowerKey] = font;
    if (isNew && addToNameList) {
        sRegisteredNames.push_back(name);
    }
}

static std::string GetFontStem(const std::string& filename) {
    size_t last_slash = filename.find_last_of("/\\");
    std::string name = (last_slash != std::string::npos) ? filename.substr(last_slash + 1) : filename;
    if (name.size() > 4) {
        std::string ext = name.substr(name.size() - 4);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (ext == ".ttf" || ext == ".otf") {
            name = name.substr(0, name.size() - 4);
        }
    }
    return name;
}

// Load one TTF from the FlyWithLua Custom_Fonts folder into the shared atlas.
// A missing/unreadable font yields nullptr instead of asserting inside ImGui.
ImFont* LoadCustomFont(const char* fileName, float sizePixels)
{
    if (fileName == nullptr || fileName[0] == '\0') {
        return nullptr;
    }
    std::string fileStr(fileName);
    std::string path;
    if (fileStr.find('/') != std::string::npos || fileStr.find('\\') != std::string::npos) {
        path = fileStr;
    } else {
        path = std::string(kCustomFontDir) + fileName;
    }

    if (!std::ifstream(path))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Custom font not found: " + path);
        return nullptr;
    }
    ImFont* font = ImgWindow::sFontAtlas->AddFontFromFileTTF(path.c_str(), sizePixels);
    if (font == nullptr)
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Could not load custom font: " + path);
    }
    return font;
}

struct FontTriplet {
    ImFont* size1 = nullptr;
    ImFont* size2 = nullptr;
    ImFont* size3 = nullptr;
};

static FontTriplet LoadFontTriplet(const std::string& filename, float s1, float s2, float s3) {
    FontTriplet triplet;
    triplet.size1 = LoadCustomFont(filename.c_str(), s1);
    triplet.size2 = LoadCustomFont(filename.c_str(), s2);
    triplet.size3 = LoadCustomFont(filename.c_str(), s3);

    std::string stem = GetFontStem(filename);
    int sz1 = static_cast<int>(s1);
    int sz2 = static_cast<int>(s2);
    int sz3 = static_cast<int>(s3);

    // Register by stem + size (e.g. "Roboto-Regular-16")
    RegisterFontInternal(stem + "-" + std::to_string(sz1), triplet.size1, true);
    RegisterFontInternal(stem + "-" + std::to_string(sz2), triplet.size2, true);
    RegisterFontInternal(stem + "-" + std::to_string(sz3), triplet.size3, true);

    // Register by filename + size (e.g. "Roboto-Regular.ttf-16")
    RegisterFontInternal(filename + "-" + std::to_string(sz1), triplet.size1, false);
    RegisterFontInternal(filename + "-" + std::to_string(sz2), triplet.size2, false);
    RegisterFontInternal(filename + "-" + std::to_string(sz3), triplet.size3, false);

    // Register base stem & filename pointing to primary size (size1)
    RegisterFontInternal(stem, triplet.size1, true);
    RegisterFontInternal(filename, triplet.size1, false);

    return triplet;
}

} // namespace

namespace flywithlua {

ImFont* GetFontByName(const std::string& name) {
    auto it = sFontRegistry.find(ToLower(name));
    if (it != sFontRegistry.end()) {
        return it->second;
    }
    return nullptr;
}

ImFont* GetFontByIndex(int index) {
    if (index >= 1 && index <= 9) {
        return sLegacyFonts[index];
    }
    return nullptr;
}

ImFont* GetDefaultFont() {
    if (sDefaultFont != nullptr) {
        return sDefaultFont;
    }
    if (ImgWindow::sFontAtlas && ImgWindow::sFontAtlas->getAtlas()) {
        ImFontAtlas* atlas = ImgWindow::sFontAtlas->getAtlas();
        if (atlas->Fonts.Size > 0) {
            return atlas->Fonts[0];
        }
    }
    return nullptr;
}

std::vector<std::string> GetRegisteredFontNames() {
    return sRegisteredNames;
}

void ClearFontRegistry() {
    sFontRegistry.clear();
    sLegacyFonts.fill(nullptr);
    sDefaultFont = nullptr;
    sRegisteredNames.clear();
}

} // namespace flywithlua

bool InitializeImGui()
{
    // Exactly one atlas for the whole plugin. If one is already published
    // (e.g. Enable without a matching Disable) leave it alone: adding the fonts
    // a second time would only duplicate them.
    if (ImgWindow::sFontAtlas)
    {
        return true;
    }

    flywithlua::ClearFontRegistry();

    ImgWindow::sFontAtlas = std::make_shared<ImgFontAtlas>();
    if (!ImgWindow::sFontAtlas || !ImgWindow::sFontAtlas->getAtlas())
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Could not create the shared ImGui font atlas.");
        ImgWindow::sFontAtlas.reset();
        return false;
    }

    ImFontAtlas* atlas = ImgWindow::sFontAtlas->getAtlas();

    ImFont* fontProggyBitmap = nullptr;
    ImFont* fontProggyVector = nullptr;

    if (flywithlua::proggy_clean_default == 1)
    {
        fontProggyBitmap = atlas->AddFontDefaultBitmap(); // Load embedded bitmap font (legacy).
        fontProggyVector = atlas->AddFontDefaultVector(); // Load embedded scalable font.
    } else {
        // Same default fonts, in the same order, as the legacy per-window setup.
        fontProggyVector = atlas->AddFontDefaultVector(); // Load embedded scalable font.
        fontProggyBitmap = atlas->AddFontDefaultBitmap(); // Load embedded bitmap font (legacy).
    }

    sDefaultFont = ImgWindow::sFontAtlas->AddFontDefault(); // Load embedded default font.

    RegisterFontInternal("default", sDefaultFont, true);
    RegisterFontInternal("ProggyClean", fontProggyBitmap, true);
    RegisterFontInternal("ProggyForever", fontProggyVector, true);

    const float font_size1 = static_cast<float>(flywithlua::imgui_font_size1 > 0 ? flywithlua::imgui_font_size1 : 13);
    const float font_size2 = static_cast<float>(flywithlua::imgui_font_size2 > 0 ? flywithlua::imgui_font_size2 : 16);
    const float font_size3 = static_cast<float>(flywithlua::imgui_font_size3 > 0 ? flywithlua::imgui_font_size3 : 20);

    // Determine the 3 configured primary fonts (from fwl_prefs.ini or bundled Roboto defaults)
    std::string fontFile1 = (flywithlua::use_custom_fonts == 1 && !flywithlua::imgui_custom_font1.empty())
                                ? flywithlua::imgui_custom_font1 : "Roboto-Light.ttf";
    std::string fontFile2 = (flywithlua::use_custom_fonts == 1 && !flywithlua::imgui_custom_font2.empty())
                                ? flywithlua::imgui_custom_font2 : "Roboto-Regular.ttf";
    std::string fontFile3 = (flywithlua::use_custom_fonts == 1 && !flywithlua::imgui_custom_font3.empty())
                                ? flywithlua::imgui_custom_font3 : "Roboto-Medium.ttf";

    FontTriplet triplet1 = LoadFontTriplet(fontFile1, font_size1, font_size2, font_size3);
    FontTriplet triplet2 = LoadFontTriplet(fontFile2, font_size1, font_size2, font_size3);
    FontTriplet triplet3 = LoadFontTriplet(fontFile3, font_size1, font_size2, font_size3);

    // Populate legacy global pointers
    customFont1 = triplet1.size1;
    customFont2 = triplet1.size2;
    customFont3 = triplet1.size3;

    customFont4 = triplet2.size1;
    customFont5 = triplet2.size2;
    customFont6 = triplet2.size3;

    customFont7 = triplet3.size1;
    customFont8 = triplet3.size2;
    customFont9 = triplet3.size3;

    // Populate legacy 1..9 lookup array
    sLegacyFonts[1] = customFont1;
    sLegacyFonts[2] = customFont2;
    sLegacyFonts[3] = customFont3;
    sLegacyFonts[4] = customFont4;
    sLegacyFonts[5] = customFont5;
    sLegacyFonts[6] = customFont6;
    sLegacyFonts[7] = customFont7;
    sLegacyFonts[8] = customFont8;
    sLegacyFonts[9] = customFont9;

    // Register string indices "1".."9" and legacy aliases "Font1-1" etc.
    for (int i = 1; i <= 9; ++i) {
        RegisterFontInternal(std::to_string(i), sLegacyFonts[i], false);
    }
    RegisterFontInternal("Font1-1", customFont1, false);
    RegisterFontInternal("Font1-2", customFont2, false);
    RegisterFontInternal("Font1-3", customFont3, false);
    RegisterFontInternal("Font1",   customFont1, false);

    RegisterFontInternal("Font2-1", customFont4, false);
    RegisterFontInternal("Font2-2", customFont5, false);
    RegisterFontInternal("Font2-3", customFont6, false);
    RegisterFontInternal("Font2",   customFont4, false);

    RegisterFontInternal("Font3-1", customFont7, false);
    RegisterFontInternal("Font3-2", customFont8, false);
    RegisterFontInternal("Font3-3", customFont9, false);
    RegisterFontInternal("Font3",   customFont7, false);

    // If custom fonts were configured, ensure standard bundled Roboto fonts are ALSO loaded and accessible
    if (flywithlua::GetFontByName("Roboto-Light") == nullptr) {
        LoadFontTriplet("Roboto-Light.ttf", font_size1, font_size2, font_size3);
    }
    if (flywithlua::GetFontByName("Roboto-Regular") == nullptr) {
        LoadFontTriplet("Roboto-Regular.ttf", font_size1, font_size2, font_size3);
    }
    if (flywithlua::GetFontByName("Roboto-Medium") == nullptr) {
        LoadFontTriplet("Roboto-Medium.ttf", font_size1, font_size2, font_size3);
    }

    // Auto-discover any additional .ttf or .otf fonts placed in the Custom_Fonts folder
    std::error_code ec;
    if (std::filesystem::exists(kCustomFontDir, ec) && std::filesystem::is_directory(kCustomFontDir, ec)) {
        for (const auto& entry : std::filesystem::directory_iterator(kCustomFontDir, ec)) {
            if (entry.is_regular_file(ec)) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
                if (ext == ".ttf" || ext == ".otf") {
                    std::string filename = entry.path().filename().string();
                    std::string stem = entry.path().stem().string();
                    if (flywithlua::GetFontByName(stem) == nullptr) {
                        LoadFontTriplet(filename, font_size1, font_size2, font_size3);
                    }
                }
            }
        }
    }

    // Note: the GPU upload happens lazily, when the first window draws.
    return true;
}

void TeardownImGui()
{
    // Step 0: close every floating window first, so none can draw against a dead atlas.
    flwnd::deinitFloatingWindowSupport();

    // Step 1: delete custom loaded textures (ImgWindow::DeleteTexture()).
    // flwnd::deinitFloatingWindowSupport() in Step 0 queues all tracked ImTextureIDs
    // for deletion via ImgWindow::DeleteTexture(), which is flushed to VRAM in Step 4.

    // Clear legacy font pointers
    customFont1 = customFont2 = customFont3 = nullptr;
    customFont4 = customFont5 = customFont6 = nullptr;
    customFont7 = customFont8 = customFont9 = nullptr;

    // Clear modern named registry
    flywithlua::ClearFontRegistry();

#if defined(IMGUI_VERSION_NUM) && (IMGUI_VERSION_NUM >= 19200)
    // Step 2: CRITICAL - sever ImGui's internal reference FIRST, while the atlas is still
    // alive. Reversing steps 2 and 3 leaves io.Fonts dangling (use-after-free).
    if (ImGui::GetCurrentContext() != nullptr)
    {
        ImGui::GetIO().Fonts = NULL;
    }
#endif

    // Step 3: release the shared pointer, which deletes the atlas.
    if (ImgWindow::sFontAtlas)
    {
        ImgWindow::sFontAtlas.reset();
    }

    // Step 4: flush the deferred deletion queue so the atlas (and any custom
    // textures) are purged from VRAM now; no further flight loop will do it.
    ImgWindow::Shutdown();
}
